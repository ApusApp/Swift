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

#include "swift/base/file.h"
#include "swift/base/likely.h"

#include <cassert>

namespace swift {

// public
File::File (int fd, bool owns_fd /*= false*/)
    : fd_ (fd)
    , is_owns_ (owns_fd)
{
    assert (fd > 0);
}

// public
File::File (File&& other)
    : fd_ (other.fd_)
    , is_owns_ (other.is_owns_)
{
    other.Release ();
}

// public
File& File::operator= (File&& other)
{
    Close ();
    Swap (other);
    return *this;
}

// public
bool File::Open (const char *file_name, 
                 int flags /*= O_RDWR | O_LARGEFILE | O_CREAT*/, 
                 mode_t mode /*= 0666*/)
{
    fd_ = fileutil::Open (file_name, flags, mode);
    return (fd_ > 0) ? true : false;
}

// public
void File::Swap (File& other)
{
    std::swap (fd_, other.fd_);
    std::swap (is_owns_, other.is_owns_);
}

// public
int File::Release ()
{
    int released = fd_;
    fd_ = -1;
    is_owns_ = false;
    return released;
}

// public
bool File::Close ()
{
    int ret = is_owns_ ? fileutil::Close (fd_) : 0;
    Release ();
    return (0 == ret);
}

// public
File File::Dup () const
{
    File ret_file;
    if (-1 != fd_) {
        int fd = fileutil::Dup (fd_);
        if (LIKELY (-1 != fd)) {
            File f (fd, true);
            ret_file.Swap (f);
        }        
    }

    return ret_file;
}

// static public
File File::Temporary ()
{
    FILE *tmp_file = ::tmpfile ();
    SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT{
        if (nullptr != tmp_file) {
            ::fclose (tmp_file);
        }
    };

    File ret_file;
    if (nullptr != tmp_file) {
        int fd = fileutil::Dup (::fileno (tmp_file));
        if (LIKELY (-1 != fd)) {
            File f (fd, true);
            ret_file.Swap (f);
        }
    }
    
    return ret_file;
}

} // namespace swift
