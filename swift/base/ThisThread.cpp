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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#include "swift/base/thisthread.h"

namespace swift {
namespace detail {
    inline pid_t GetTid ()
    {
        return static_cast<pid_t>(::syscall (SYS_gettid));
    }
}

namespace thisthread {
    __thread int t_cached_tid = 0;
    __thread char t_tid_string[32];
    __thread const char* t_thread_name = "unknown";

    void CacheTid ()
    {
        if (0 == t_cached_tid) {
            t_cached_tid = detail::GetTid ();
            int length = snprintf (t_tid_string, sizeof(t_tid_string), "%5d ", t_cached_tid);
            assert (6 == length); (void) length;
        }
    }

    bool IsMainThread ()
    {
        return GetTid () == ::getpid ();
    }

} // namespace thisthread

} // namespace swift
