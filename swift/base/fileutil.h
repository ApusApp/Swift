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

// fileutil is stolen from fb's folly
#ifndef __SWIFT_BASE_FILE_UTIL_H__
#define __SWIFT_BASE_FILE_UTIL_H__

#include <unistd.h>
#include <limits>
#include <fcntl.h>
#include <cassert>
#include <cerrno>
#include <sys/stat.h>
#include <sys/uio.h>

#include "swift/base/scopeguard.h"

namespace swift {
namespace detail {

// Wrap call to f (args) in loop to retry on EINTR
template <class Func, class... Args>
ssize_t WrapFuncT (Func f, Args... args)
{
    ssize_t ret = 0;
    do {
        ret = f (args...);
    } while (-1 == ret && EINTR == errno);

    return ret;
}

inline void Increase (ssize_t n) {}

inline void Increase (ssize_t n, off_t& offset)
{
    offset += n;
}

// Wrap call to read/pread/write/pwrite(fd, buf, size, offset?) to retry on
// incomplete reads / writes. The variadic argument magic is there to support
// an additional argument (offset) for pread / pwrite; see the Increase() functions
// above which do nothing if the offset is not present and increment it if it is.
template <class Func, class... Offset>
ssize_t WrapFileOpFuncT (Func f,
                         int fd, 
                         void *buf, 
                         size_t size, 
                         Offset... offset)
{
    ssize_t ret = -1;
    ssize_t total_bytes = 0;
    char* b = static_cast<char*>(buf);

    do {
        ret = f (fd, b, size, offset...);
        if (-1 == ret) {
            if (EINTR == errno || EAGAIN == errno) {
                continue;
            }

            return ret;
        }

        total_bytes += ret;
        b += ret;
        size -= ret;
        Increase (ret, offset...);
    } while (0 != ret && size); // 0 means EOF

    return total_bytes;
}

// Wrap call to readv/preadv/writev/pwritev(fd, iov, size, offset?) to
// retry on incomplete reads / writes.
template <class Func, class... Offset>
ssize_t WrapFileOpVFuncT (Func f,
                          int fd,
                          iovec *iov,
                          int size,
                          Offset... offset)
{
    ssize_t ret = -1;
    ssize_t total_bytes = 0;

    do {
        ret = f (fd, iov, size, offset...);
        if (-1 == ret) {
            if (EINTR == errno || EAGAIN == errno) {
                continue;
            }

            return ret;
        }

        if (0 == ret) {
            break; // EOF
        }

        total_bytes += ret;
        Increase (ret, offset...);
        while (0 != ret && 0 != size) {
            if (ret >= iov->iov_len) {
                ret -= iov->iov_len;
                ++iov;
                --size;
            }
            else {
                iov->iov_base = static_cast<char*>(iov->iov_base) + ret;
                iov->iov_len -= ret;
                ret = 0;
            }
        } 
    } while (size);

    return total_bytes;
}
} // namespace detail


// Convenience wrappers around some commonly used system calls. The no *Full
// wrappers retry on EINTR. The *Full wrappers retry on EINTR and EAGAIN, also loop
// until all data is written. Note that *Full wrappers weaken the thread
// semantics of underlying system calls.
int Open (const char *file_name, 
          int flags = O_RDWR | O_LARGEFILE | O_CREAT, 
          mode_t mode = 0666);
int Close (int fd);
int Dup (int fd);
int Dup2 (int old_fd, int new_fd);
int Fsync (int fd);
int Fdatasync (int fd);
int Ftrucate (int fd, off_t length);
int Truncate (const char *path, off_t length);
int Flock (int fd, int operation);
int Shutdown (int fd, int how);

ssize_t Read (int fd, void *buf, size_t length);
ssize_t PRead (int fd, void *buf, size_t length, off_t offset);
ssize_t Readv (int fd, const iovec *iov, int length);

ssize_t Write (int fd, const void *buf, size_t length);
ssize_t PWrite (int fd, void *buf, size_t length, off_t offset);
ssize_t Writev (int fd, const iovec *iov, int length);

// Wrapper around read() (and pread()) that, in addition to retrying on
// EINTR and EAGAIN, will loop until all data is read.
//
// This wrapper is only useful for blocking file descriptors (for non-blocking
// file descriptors, you have to be prepared to deal with incomplete reads
// anyway), and only exists because POSIX allows read() to return an incomplete
// read if interrupted by a signal (instead of returning -1 and setting errno
// to EINTR).
//
// Note that this wrapper weakens the thread safety of read(): the file pointer
// is shared between threads, but the system call is atomic. If multiple
// threads are reading from a file at the same time, you don't know where your
// data came from in the file, but you do know that the returned bytes were
// contiguous. You can no longer make this assumption if using ReadFull().
// You should probably use pread() when reading from the same file descriptor
// from multiple threads simultaneously, anyway.
//
// Note that readvFull and preadvFull require iov to be non-const, unlike
// readv and preadv.  The contents of iov after these functions return is unspecified.
ssize_t ReadFull (int fd, void *buf, size_t length);
ssize_t PReadFull (int fd, void *buf, size_t length, off_t offset);
ssize_t ReadvFull (int fd, iovec *iov, int length);
ssize_t PReadvFull (int fd, iovec *iov, int length, off_t offset);


// Similar to readFull and preadFull above, wrappers around write() and
// pwrite() that loop until all data is written.
//
// Generally, the write() / pwrite() system call may always write fewer bytes
// than requested, just like read(). In certain cases (such as when writing to
// a pipe), POSIX provides stronger guarantees, but not in the general case.
// For example, Linux (even on a 64-bit platform) won't write more than 2GB in
// one write() system call.
//
// Note that WritevFull and PWritevFull require iov to be non-const, unlike
// writev and pwritev. The contents of iov after these functions return is unspecified.
ssize_t WriteFull (int fd, const void *buf, size_t length);
ssize_t PWriteFull (int fd, const void *buf, size_t length, off_t offset);
ssize_t WritevFull (int fd, iovec *iov, int length);
ssize_t PWritevFull (int fd, iovec *iov, int length, off_t offset);


// Read entire file (if num_bytes is defaulted) or no more than
// num_bytes (otherwise) into container *out. The container is assumed
// to be contiguous, with element size equal to 1, and offer size(),
// reserve(), and random access (e.g. std::vector<char>, std::string).
//
// Returns: true on success or false on failure. In the latter case
// errno will be set appropriately by the failing system primitive.
template <class Container>
bool ReadFile (const char *file_name,
               Container& out,
               size_t num_bytes = std::numeric_limits<size_t>::max ())
{
    static_assert(sizeof(out[0]) == 1,
        "ReadFile: only containers with byte-sized elements accepted");
    assert (file_name);

    const int fd = ::open (file_name, O_RDONLY);
    if (-1 == fd) {
        return false;
    }

    size_t size = 0;
    SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT {
        assert (out.size () >= size);
        out.resize (size);
        Close (fd);
    };

    // Get file size
    struct stat buf;
    if (-1 == ::fstat (fd, &buf)) {
        return false;
    }

    // Some files (notably under /proc and /sys on Linux) lie about
    // their size, so treat the size advertised by fstat under advise
    // but don't rely on it. In particular, if the size is zero, we
    // should attempt to read stuff. If not zero, we'll attempt to read
    // one extra byte.
    const size_t initial_alloc = 1024 * 4;
    out.resize (std::min (buf.st_size > 0 ? static_cast<size_t>(buf.st_size + 1) : initial_alloc, 
                          num_bytes));
    while (size < out.size ()) {
        size_t n = ReadFull (fd, &out[size], out.size () - size);
        if (-1 == n) {
            return false;
        }
        size += n;
        if (size < out.size ()) {
            break;
        }

        // Ew, allocate more memory. Use exponential growth to avoid
        // quadratic behavior. Cap size to num_bytes.
        out.resize (std::min (out.size () * 3 / 2, num_bytes));
    }

    return true;
}
} // namespace swift

#endif // __SWIFT_BASE_FILE_UTIL_H__
