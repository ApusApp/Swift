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

#ifndef __SWIFT_BASE_FILE_H__
#define __SWIFT_BASE_FILE_H__

#include "swift/base/noncopyable.hpp"
#include "swift/base/fileutil.h"

namespace swift {

class File : swift::noncopyable
{
public:
    File () : fd_ (-1), is_owns_ (true)
    {
    }

    File (int fd, bool owns_fd = false);

    ~File ()
    {
        Close ();
    }

    // movable
    File (File&& other);

    File& operator= (File&& other);

    bool Open (const char *file_name,
               int flags = O_RDWR | O_LARGEFILE | O_CREAT,
               mode_t mode = 0666);

    size_t GetFileSize () const
    {
        if (fd_ < 0) {
            return 0;
        }
        struct stat statbuf;
        return (0 != ::fstat (fd_, &statbuf)) ? 0 : statbuf.st_size;
    }

    bool SetPosition (size_t position) const
    {
        if (fd_ < 0) {
            return false;
        }
            
        return (::lseek (fd_, static_cast<off_t>(position), SEEK_SET) == static_cast<off_t>(position));
    }

    size_t GetPosition () const
    {
        if (fd_ < 0) {
            return 0;
        }
        return static_cast<size_t> (::lseek (fd_, 0, SEEK_CUR));
    }

    void Flush () const
    {
        if (fd_ > 0) {
            fileutil::Fsync (fd_);
        }
    }

    void FlushData () const
    {
        if (fd_ > 0) {
            fileutil::Fdatasync (fd_);
        }
    }

    bool Truncate (size_t size) const
    {
        if (fd_ < 0) {
            return false;
        }

        return 0 == fileutil::Ftrucate (fd_, static_cast<off_t>(size));
    }

    size_t Read (char* buf, size_t size) const
    {
        if (fd_ < 0) {
            return 0;
        }

        return static_cast<size_t>(fileutil::ReadFull (fd_, buf, size));
    }

    size_t PRead (char* buf, size_t nbytes, size_t offset) const
    {
        if (fd_ < 0) {
            return 0;
        }

        return static_cast<size_t>(fileutil::PReadFull (fd_, 
                                                        buf, 
                                                        nbytes, 
                                                        static_cast<off_t>(offset)));
    }

    size_t Write (const char* buf, size_t size) const
    {
        if (fd_ < 0) {
            return 0;
        }

        return static_cast<size_t>(fileutil::WriteFull (fd_, buf, size));
    }

    size_t PWrite (const char* buf, size_t nbytes, size_t offset) const
    {
        if (fd_ < 0) {
            return 0;
        }

        return static_cast<size_t> (fileutil::PWriteFull (fd_, 
                                                          buf, 
                                                          nbytes, 
                                                          static_cast<off_t>(offset)));
    }

    size_t Append (const char* buf, const size_t size) const
    {
        if (fd_ < 0) {
            return 0;
        }

        off_t offset = ::lseek (fd_, 0, SEEK_END);
        if (offset < 0) {
            return 0;
        }

        return (size == PWrite (buf, size, static_cast<size_t>(offset))) ? size : 0;
    }


    explicit operator bool () const
    {
        return (-1 != fd_);
    }

    int GetFd () const
    {
        return fd_;
    }

    void Swap (File& other);

    // Returns and releases the file descriptor; no longer owned by this File.
    // Returns -1 if the File object didn't wrap a file.
    int Release ();

    bool Close ();

    // Duplicate file descriptor and return File that owns it.
    File Dup () const;

    // FLOCK (INTERPROCESS) LOCKS
    //
    // NOTE THAT THESE LOCKS ARE flock() LOCKS. That is, they may only be used
    // for inter-process synchronization -- an attempt to acquire a second lock
    // on the same file descriptor from the same process may succeed. Attempting
    // to acquire a second lock on a different file descriptor for the same file
    // should fail, but some systems might implement flock() using fcntl() locks,
    // in which case it will succeed.
    void lock ()
    {
        DoLock (LOCK_EX);
    }

    bool try_lock ()
    {
        return DoTryLock (LOCK_EX);
    }

    void unlock ()
    {
        fileutil::Flock (fd_, LOCK_UN);
    }

    void lock_shared ()
    {
        DoLock (LOCK_SH);
    }

    bool try_lock_shared ()
    {
        return DoTryLock (LOCK_SH);
    }

    void unlock_shared ()
    {
        unlock ();
    }

public:
    // Create and return a temporary, owned file (uses tmpfile()).
    static File Temporary ();

private:
    void DoLock (int op)
    {
        fileutil::Flock (fd_, op);
    }

    inline bool DoTryLock (int op)
    {
        int ret = fileutil::Flock (fd_, op | LOCK_NB);
        // flock returns EWOULDBLOCK if already locked
        if (-1 == ret || EWOULDBLOCK == errno) {
            return false;
        }

        return true;
    }

private:
    int fd_;
    bool is_owns_;
}; // File

inline void Swap (File& lhs, File& rhs)
{
    lhs.Swap (rhs);
}

} // namespace swift

#endif // __SWIFT_BASE_FILE_H__
