/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "swift/base/memorymapping.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <system_error>
#include <cassert>

#include "swift/base/logging.h"

namespace swift {
namespace {

// define FLAGS_mlock_chunk_size
DEFINE_int64(mlock_chunk_size, 1 << 20,  // 1MB
             "Maximum bytes to mlock/munlock/munmap at once "
             "(will be rounded up to PAGESIZE)");

off_t MemOpChunkSize(off_t length, off_t page_size)
{
    if (0 >= FLAGS_mlock_chunk_size) {
        return length;
    }

    off_t chunk_size = FLAGS_mlock_chunk_size;
    off_t n = chunk_size % page_size;
    if (0 != n) {
        chunk_size += (page_size - n);
    }

    return chunk_size;
}

//
// Run @func in chunks over the buffer @mem of @buf_size length.
//
// Return:
// - success: true + amount_succeeded == buf_size (func success on whole buffer)
// - failure: false + amount_succeeded == nr bytes on which func succeeded.
//
bool MemOpInChunks(std::function<int(void*, size_t)> func,
                   void* mem,
                   size_t buf_size,
                   off_t page_size,
                   size_t& amount_succeeded)
{
    // unmap/mlock/munlock take a kernel semaphore and block other threads from
    // doing other memory operations. If the size of the buffer is big the
    // semaphore can be down for seconds (for benchmarks see
    // http://kostja-osipov.livejournal.com/42963.html).  Doing the operations in
    // chunks breaks the locking into intervals and lets other threads do memory
    // operations of their own.
    size_t chunk_size = MemOpChunkSize(buf_size, page_size);
    char* addr = static_cast<char*>(mem);
    amount_succeeded = 0;

    while (amount_succeeded < buf_size) {
        size_t size = std::min(chunk_size, buf_size - amount_succeeded);
        if (0 != func(addr + amount_succeeded, size)) {
            return false;
        }
        amount_succeeded += size;
    }

    return true;
}

} // anonymous namespace

// public
MemoryMapping::~MemoryMapping ()
{
    if (map_length_) {
        size_t amount_succeeded = 0;
        if (!MemOpInChunks(::munmap,
                           map_start_,
                           map_length_,
                           options_.page_size,
                           amount_succeeded)) {
            PLOG(FATAL) << "munmap error at "
                        << map_length_
                        << " and "
                        << amount_succeeded;
        }
    }
}

// public
MemoryMapping::MemoryMapping(File file,
                             off_t offset /*= 0*/,
                             off_t length /*= -1*/,
                             Options opt /*= Options()*/)
    : file_(std::move(file))
    , map_start_(nullptr)
    , map_length_(0)
    , options_(std::move(opt))
    , locked_(false)
    , data_()
{
    CHECK(file_);
    Init(offset, length);
}

// public
MemoryMapping::MemoryMapping(int fd,
                             off_t offset /*= 0*/,
                             off_t length /*= -1*/,
                             Options opt /*= Options()*/)
    : MemoryMapping(File(fd), offset, length, opt)
{

}

// public
MemoryMapping::MemoryMapping(const char* name,
                             off_t offset /*= 0*/,
                             off_t length /*= -1*/,
                             Options opt /*= Options()*/)
    : file_()
    , map_start_(nullptr)
    , map_length_(0)
    , options_(std::move(opt))
    , locked_(false)
    , data_()
{
    PCHECK(true == file_.Open(name)) << "Open file " << name << " error";
    Init(offset, length);
}

// public
MemoryMapping::MemoryMapping(AnonymousType type,
                             off_t length,
                             Options opt)
    : file_()
    , map_start_(nullptr)
    , map_length_(0)
    , options_(std::move(opt))
    , locked_(false)
    , data_()
{
    Init(0, length);
    (void)type;
}

// public
void MemoryMapping::Init(off_t offset, off_t length)
{
    const bool grow = options_.grow;
    const bool anonymous = !file_;
    off_t& page_size = options_.page_size;
    bool auto_extend = false;
    struct stat st;

    CHECK(!(grow && anonymous));
    if (!anonymous) {
        CHECK_ERR(fstat(file_.GetFd(), &st));
    } else {
        DCHECK(!file_);
        DCHECK_EQ(0, offset);
        // CHECK_EQ(0, page_size);
        CHECK_GE(length, 0);
    }

    if (0 == page_size) {
        page_size = ::sysconf(_SC_PAGESIZE);
    }

    CHECK_GT(page_size, 0);
    CHECK_EQ(page_size & (page_size - 1), 0);
    CHECK_GE(offset, 0);

    size_t skip_start = offset % page_size;
    offset -= skip_start;
    map_length_ = length;
    if (-1 != map_length_) {
        map_length_ += skip_start;
        map_length_ = (map_length_ + page_size - 1) / page_size * page_size;
    }

    off_t remaining = anonymous ? length : st.st_size - offset;
    if (-1 == map_length_) {
        length = map_length_ = remaining;
    } else {
        if (length > remaining) {
            if (grow) {
                if (!auto_extend) {
                    PCHECK(true == file_.Truncate(offset + length))
                        << "Truncate failed, couldn't grow file to "
                        << offset + length;
                    remaining = length;
                } else {
                    remaining = map_length_;
                }
            } else {
                length = remaining;
            }
        }

        if (map_length_ > remaining) {
            map_length_ = remaining;
        }
    }

    if (0 == length) {
        map_length_ = 0;
        map_start_ = nullptr;
    } else {
        int flags = options_.shared ? MAP_SHARED : MAP_PRIVATE;
        if (anonymous) {
            flags |= MAP_ANONYMOUS;
        }
        if (options_.prefault) {
            flags |= MAP_POPULATE;
        }

        int prot = PROT_NONE;
        if (options_.readable || options_.writable) {
            prot = ((options_.readable ? PROT_READ : 0) |
                    (options_.writable ? PROT_WRITE : 0));
        }

        unsigned char* start = static_cast<unsigned char*>(
            mmap(options_.address, map_length_, prot, flags, file_.GetFd(), offset));
        PCHECK(start != MAP_FAILED)
            << "mmap error, offset = " << offset
            << " length = " << map_length_;
        map_start_ = start;
        data_.Set(reinterpret_cast<const char*>(start + skip_start),
                  static_cast<size_t>(length));
    }
}

// public
bool MemoryMapping::mlock(LockMode lock)
{
    size_t amount_succeeded = 0;
    locked_ = MemOpInChunks(::mlock,
                            map_start_,
                            map_length_,
                            options_.page_size,
                            amount_succeeded);
    if (locked_) {
        return true;
    }

    if (LockMode::LOCK_MODE_TRY_LOCK == lock && (EPERM == errno || ENOMEM == errno)) {
        PLOG(WARNING) << "mlock failed at "
                      << map_length_
                      << " and "
                      << amount_succeeded;
    } else {
        PLOG(FATAL) << "mlock failed at "
                    << map_length_
                    << " and "
                    << amount_succeeded;
    }

    // unlock it back
    if (!MemOpInChunks(::munlock,
                       map_start_,
                       amount_succeeded,
                       options_.page_size,
                       amount_succeeded)) {
        PLOG(WARNING) << "munlock()";
    }

    return false;
}

// public
void MemoryMapping::munlock(bool dontneed /*= false*/)
{
    if (!locked_) {
        return;
    }

    size_t amount_succeeded = 0;
    if (!MemOpInChunks(::munlock,
                       map_start_,
                       map_length_,
                       options_.page_size,
                       amount_succeeded)) {
        PLOG(WARNING) << "munlock()";
    }

    if (map_length_ && dontneed &&
        ::madvise(map_start_, map_length_, MADV_DONTNEED)) {
        PLOG(WARNING) << "madvise()";
    }

    locked_ = false;
}

// public
void MemoryMapping::HintLinearScan()
{
    Advise(MADV_SEQUENTIAL);
}

// public
void MemoryMapping::Advise(int advice) const
{
    if (map_length_ && ::madvise(map_start_, map_length_, advice)) {
        PLOG(WARNING) << "madvise()";
    }
}

// static public
void MemoryMapping::AlignedForwardMemcpy(void *dst, const void *src, size_t size)
{
    assert(reinterpret_cast<uintptr_t>(src) % alignof(unsigned long) == 0);
    assert(reinterpret_cast<uintptr_t>(dst) % alignof(unsigned long) == 0);

    auto srcl = static_cast<const unsigned long*>(src);
    auto dstl = static_cast<unsigned long*>(dst);
    while (size >= sizeof(unsigned long)) {
        *dstl++ = *srcl++;
        size -= sizeof(unsigned long);
    }

    auto srcc = reinterpret_cast<const unsigned char*>(srcl);
    auto dstc = reinterpret_cast<unsigned char*>(dstl);
    while (size != 0) {
        *dstc++ = *srcc++;
        --size;
    }
}

// static public
void MemoryMapping::MMapFileCopy(const char *src,
                                 const char *dest,
                                 mode_t mode /*= 0655*/)
{
    MemoryMapping src_map(src);
    src_map.HintLinearScan();
    File file;
    PCHECK(true == file.Open(dest, O_RDWR | O_CREAT | O_TRUNC, mode))
        << "Open file " << dest << " error";
    MemoryMapping dest_map(file.GetFd(),
                           0,
                           src_map.GetData().size(),
                           MemoryMapping::Writable());
    MemoryMapping::Buffer<char> buffer = dest_map.AsWritableBuffer<char>();
    AlignedForwardMemcpy(buffer.buf,
                         src_map.GetData().data(),
                         src_map.data_.length());
}

} // namespace swift
