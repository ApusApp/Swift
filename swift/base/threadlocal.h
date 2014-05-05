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

#ifndef __SWIFT_BASE_THREAD_LOCAL_H__
#define __SWIFT_BASE_THREAD_LOCAL_H__

#include <pthread.h>

#include "swift/base/noncopyable.hpp"

namespace swift {

template <typename T>
class ThreadLocal : swift::noncopyable
{
public:
    ThreadLocal () : key_ (0)
    {
        pthread_key_create (&key, OnThreadExit);
    }

    ~ThreadLocal ()
    {
        pthread_key_delete (key_);
    }

    T* get () const
    {
        return static_cast<T*>(pthread_getspecific (key_));
    }

    void reset (T* t)
    {
        delete get ();
        pthread_setspecific (key_, t);
    }

private:
    static void OnThreadExit (void *obj)
    {
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete static_cast<T*>(obj);
    }

private:
    pthread_key_t key_;
};

} // namespace swift
#endif // __SWIFT_BASE_THREAD_LOCAL_H__
