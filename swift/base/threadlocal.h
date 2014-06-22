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
#include <cassert>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>

#include "swift/base/noncopyable.hpp"

namespace swift {

typedef void (*UnrefHandler) (void *ptr);
class ThreadLocalPtr
{
public:
    explicit ThreadLocalPtr (UnrefHandler handler = nullptr);
    ~ThreadLocalPtr ();

    void* Get () const;
    void Reset (void *ptr);
    void* Swap (void *ptr);
    bool CompareAndSwap (void *ptr, void*& expected);
    void Scrape (std::vector<void*> *ptrs, const void *replacement);

protected:
    struct Entry
    {
        Entry () : ptr (nullptr) {}
        Entry (const Entry& other) : ptr (other.ptr.load (std::memory_order_relaxed)) {}
        std::atomic<void*> ptr;
    };

    struct ThreadData
    {
        ThreadData () : next (nullptr), prev (nullptr) {}
        std::vector<Entry> entries;
        ThreadData* next;
        ThreadData* prev;
    };

    class StaticMeta
    {
    public:
        StaticMeta ();

        uint32_t GetId ();
        uint32_t PeekId () const;
        void* Get (uint32_t id);
        void Reset (uint32_t id, void *ptr);
        void* Swap (uint32_t id, void *ptr);
        void ReclaimId (uint32_t id);
        void Scrape (uint32_t id, std::vector<void*> *ptrs, const void *replacement);
        bool CompareAndSwap (uint32_t id, void *ptr, void*& expected);
        void SetHandler (uint32_t id, UnrefHandler handler);

    private:
        UnrefHandler GetHandler (uint32_t id) const;
        void AddThreadData (ThreadData* data);
        void RemoveThreadData (ThreadData* data);

    private:
        static void OnThreadExit (void *ptr);
        static ThreadData* GetThreadLocal ();

    private:
        ThreadData head_;
        pthread_key_t key_;
        uint32_t next_instance_id_;
        std::vector<uint32_t> free_instance_ids_;
        std::unordered_map<uint32_t, UnrefHandler> handler_map_;

        static std::mutex kLock_;
        static __thread ThreadData* kTls_;
     }; // StaticMeta

     static StaticMeta* Instance ();

     const uint32_t id_;
};

template <typename T>
class ThreadLocal : swift::noncopyable
{
public:
    ThreadLocal () : tlp_ (OnThreadExit) { }
    ~ThreadLocal () {}

    T* Get () const
    {
        T* ptr = static_cast<T*>(tlp_.Get ());
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
        tlp_.Reset (t);
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
        tlp_.Reset (ptr);
        return ptr;
    }

private:
    mutable ThreadLocalPtr tlp_;
};
} // namespace swift
#endif // __SWIFT_BASE_THREAD_LOCAL_H__
