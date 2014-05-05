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

#ifndef __SWIFT_BASE_THIS_THREAD_H__
#define __SWIFT_BASE_THIS_THREAD_H__

#include <stdint.h>

#include "swift/base/likely.h"

namespace swift {
namespace thisthread {
    // internal
    extern __thread int t_cached_tid;
    extern __thread char t_tid_string[32];
    extern __thread const char* t_thread_name;

    void CacheTid ();
    bool IsMainThread ();
    inline int GetTid ()
    {
        if (UNLIKELY (0 == t_cached_tid)) {
            CacheTid ();
        }

        return t_cached_tid;
    }

    inline const char* TidToString ()
    {
        if (UNLIKELY (0 == t_cached_tid)) {
            CacheTid ();
        }

        return t_tid_string;
    }

    inline const char* GetName ()
    {
        return t_thread_name;
    }

} // namespace thisthread
} // namespace swift
#endif // __SWIFT_BASE_THIS_THREAD_H__
