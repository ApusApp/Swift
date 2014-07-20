/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SWIFT_BASE_MEMORY_MAP_H__
#define __SWIFT_BASE_MEMORY_MAP_H__

#include "swift/base/stringpiece.h"
#include "swift/base/noncopyable.hpp"
#include "swift/base/file.h"
#include "swift/base/logging.h"

namespace swift {

class MemoryMapping : swift::noncopyable
{
public:
    // Lock the pages in memory
    enum class LockMode
    {
        LOCK_MODE_TRY_LOCK,
        LOCK_MODE_LOCK,
    };

    template<typename T>
    struct Buffer
    {
        T* buf = nullptr;
        size_t length = 0;
    };

    struct Options
    {
        Options() : page_size(0)
            , shared(true)
            , prefault(false)
            , readable(true)
            , writable(false)
            , grow(false)
            , address(nullptr) { }

        inline Options& SetPageSize(off_t val)
        {
            page_size = val;
            return *this;
        }

        inline Options& SetShared(bool val)
        {
            shared = val;
            return *this;
        }

        inline Options& SetPrefault(bool val)
        {
            prefault = val;
            return *this;
        }

        inline Options& SetReadable(bool val)
        {
            readable = val;
            return *this;
        }

        inline Options& SetWritable(bool val)
        {
            writable = val;
            return *this;
        }

        inline Options& SetGrow(bool val)
        {
            grow = val;
            return *this;
        }

        off_t page_size;
        bool shared;
        bool prefault;
        bool readable;
        bool writable;
        bool grow;
        void* address;
    }; // Options

    static inline Options Writable()
    {
        return Options().SetWritable(true).SetGrow(true);
    }

    enum class AnonymousType
    {
        ANONYMOUS_TYPE = 0,
    };

    MemoryMapping(AnonymousType type,
                  off_t length,
                  Options opt = Options());

    explicit MemoryMapping(File file,
                           off_t offset = 0,
                           off_t length = -1,
                           Options opt = Options());

    explicit MemoryMapping(const char* name,
                           off_t offset = 0,
                           off_t length = -1,
                           Options opt = Options());

    explicit MemoryMapping(int fd,
                           off_t offset = 0,
                           off_t length = -1,
                           Options opt = Options());

    ~MemoryMapping ();

    inline const StringPiece GetData() const
    {
        return data_;
    }

    // Lock the pages in memory
    bool mlock(LockMode lock);

    // Unlock the pages
    // If dontneed is true, the kernel is instructed to release these pages
    // (per madvise(MADV_DONTNEED))
    void munlock(bool dontneed = false);

    // Hint that these pages will be scanned linearly.
    // madvise(MADV_SEQUENTIAL)
    void HintLinearScan();

    // Advise the kernel about memory access.
    void Advise(int advice) const;

    inline bool IsLocked() const
    {
        return locked_;
    }

    inline int GetFd() const
    {
        return file_.GetFd();
    }

    template<typename T>
    Buffer<T> AsWritableBuffer() const
    {
        DCHECK(options_.writable);
        Buffer<T> buffer;
        buffer.length = data_.length() / sizeof(T);
        buffer.buf = reinterpret_cast<T*>(const_cast<char*>(data_.data()));
        return buffer;
    }

    template<typename T>
    Buffer<T> AsReadableBuffer() const
    {
        Buffer<T> buffer;
        buffer.length = data_.length() / sizeof(T);
        buffer.buf = reinterpret_cast<T*>(const_cast<char*>(data_.data()));
        return buffer;
    }

public:
    static void AlignedForwardMemcpy(void* dst, const void* src, size_t size);
    static void MMapFileCopy(const char* src, const char* dest, mode_t mode = 0655);

private:
    MemoryMapping () { };

    void Init(off_t offset, off_t length);

private:
    File file_;
    void* map_start_;
    off_t map_length_;
    Options options_;
    bool locked_;
    StringPiece data_;
};
} // namespace swift

#endif // __SWIFT_BASE_MEMORY_MAP_H__

