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
#include <assert.h>

#include "swift/base/noncopyable.hpp"

namespace swift {

template <typename T>
class ThreadLocal : swift::noncopyable
{
public:
    ThreadLocal () : key_ (0)
    {
        int err = pthread_key_create (&key_, OnThreadExit);
        assert (err == 0); (void) err;
    }

    ~ThreadLocal ()
    {
        int err = pthread_key_delete (key_);
        assert (err == 0); (void) err;
    }

    T* Get () const
    {
        T* ptr = static_cast<T*>(pthread_getspecific (key_));
        if (nullptr != ptr) {
            return ptr;
        }

        return MakeTls ();
    }

    T* operator-> () const {
        return Get ();
    }

    T& operator* () const {
        return *Get ();
    }

    void Reset (T* t = nullptr)
    {
        T* ptr = Get ();
        if (t != ptr) {
            if (nullptr != ptr) {
                delete ptr;
            }
            
            int err = pthread_setspecific (key_, t);
            assert (err == 0); (void) err;
        }
    }

    // movable
    ThreadLocal (ThreadLocal&&) = default;
    ThreadLocal& operator= (ThreadLocal&&) = default;

private:
    static void OnThreadExit (void *obj)
    {
        T* ptr = static_cast<T*>(obj);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        if (nullptr != ptr) {
            delete ptr;
        }
    }

    // Tls: Thread Local Storage
    T* MakeTls () const
    {
        T* ptr = new T ();
        int err = pthread_setspecific (key_, ptr);
        assert (err == 0); (void) err;
        return ptr;
    }

private:
    mutable pthread_key_t key_;
};

} // namespace swift
#endif // __SWIFT_BASE_THREAD_LOCAL_H__
