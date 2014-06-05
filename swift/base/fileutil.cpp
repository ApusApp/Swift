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

#include "swift/base/fileutil.h"

#include <sys/file.h>
#include <sys/socket.h>

namespace swift {
namespace fileutil {

int Open (const char *file_name, 
          int flags /*= O_RDWR | O_LARGEFILE | O_CREAT*/, 
          mode_t mode /*= 0666*/)
{
    return detail::WrapFuncT (open, file_name, flags, mode);
}

int Close (int fd)
{
    int ret = close (fd);
    // Ignore EINTR. On Linux, close() may only return EINTR after the file
    // descriptor has been closed, so you must not retry close() on EINTR --
    // in the best case, you'll get EBADF, and in the worst case, you'll end up
    // closing a different file (one opened from another thread).
    //
    // Interestingly enough, the Single Unix Specification says that the state
    // of the file descriptor is unspecified if close returns EINTR.  In that
    // case, the safe thing to do is also not to retry close() -- leaking a file
    // descriptor is probably better than closing the wrong file.
    return (-1 == ret && EINTR == errno) ? 0 : ret;
}

int Dup (int fd)
{
    return detail::WrapFuncT (dup, fd);
}

int Dup2 (int old_fd, int new_fd)
{
    return detail::WrapFuncT (dup2, old_fd, new_fd);
}

int Fsync (int fd)
{
    return detail::WrapFuncT (fsync, fd);
}

int Fdatasync (int fd)
{
    return detail::WrapFuncT (fdatasync, fd);
}

int Ftrucate (int fd, off_t length)
{
    return detail::WrapFuncT (ftruncate, fd, length);
}

int Truncate (const char *path, off_t length)
{
    return detail::WrapFuncT (truncate, path, length);
}

int Flock (int fd, int operation)
{
    return detail::WrapFuncT (flock, fd, operation);
}

int Shutdown (int fd, int how)
{
    return detail::WrapFuncT (shutdown, fd, how);
}

ssize_t Read (int fd, void *buf, size_t length)
{
    return detail::WrapFuncT (read, fd, buf, length);
}

ssize_t PRead (int fd, void *buf, size_t length, off_t offset)
{
    return detail::WrapFuncT (pread, fd, buf, length, offset);
}

ssize_t Readv (int fd, const iovec *iov, int length)
{
    return detail::WrapFuncT (readv, fd, iov, length);
}

ssize_t Write (int fd, const void *buf, size_t length)
{
    return detail::WrapFuncT (write, fd, buf, length);
}

ssize_t PWrite (int fd, void *buf, size_t length, off_t offset)
{
    return detail::WrapFuncT (pwrite, fd, buf, length, offset);
}

ssize_t Writev (int fd, const iovec *iov, int length)
{
    return detail::WrapFuncT (writev, fd, iov, length);
}

ssize_t ReadFull (int fd, void *buf, size_t length)
{
    return detail::WrapFileOpFuncT (read, fd, buf, length);
}

ssize_t PReadFull (int fd, void *buf, size_t length, off_t offset)
{
    return detail::WrapFileOpFuncT (pread, fd, buf, length, offset);
}

ssize_t ReadvFull (int fd, iovec *iov, int length)
{
    return detail::WrapFileOpVFuncT (readv, fd, iov, length);
}

ssize_t PReadvFull (int fd, iovec *iov, int length, off_t offset)
{
    return detail::WrapFileOpVFuncT (preadv, fd, iov, length, offset);
}

ssize_t WriteFull (int fd, const void *buf, size_t length)
{
    return detail::WrapFileOpFuncT (write, fd, const_cast<void*>(buf), length);
}

ssize_t PWriteFull (int fd, const void *buf, size_t length, off_t offset)
{
    return detail::WrapFileOpFuncT (pwrite, fd, const_cast<void*>(buf), length, offset);
}

ssize_t WritevFull (int fd, iovec *iov, int length)
{
    return detail::WrapFileOpVFuncT (writev, fd, iov, length);
}

ssize_t PWritevFull (int fd, iovec *iov, int length, off_t offset)
{
    return detail::WrapFileOpVFuncT (pwritev, fd, iov, length, offset);
}

} // fileutil
} // namespace swift
