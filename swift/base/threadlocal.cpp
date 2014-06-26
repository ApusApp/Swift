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

#include "swift/base/threadlocal.h"
#include "swift/base/likely.h"

namespace swift {

std::mutex ThreadLocalPtr::StaticMeta::kLock_;
__thread ThreadLocalPtr::ThreadData* ThreadLocalPtr::StaticMeta::kTls_ = nullptr;

// public
ThreadLocalPtr::StaticMeta::StaticMeta () : next_instance_id_ (0)
{
    if (0 != pthread_key_create (&key_, &OnThreadExit)) {
        throw std::runtime_error ("pthread_key_create failed");
    }

    head_.next = &head_;
    head_.prev = &head_;
}

// public
uint32_t ThreadLocalPtr::StaticMeta::GetId ()
{
    std::lock_guard<std::mutex> lock (kLock_);
    if (free_instance_ids_.empty ()) {
        return next_instance_id_++;
    }

    uint32_t id = free_instance_ids_.back ();
    free_instance_ids_.pop_back ();
    return id;
}

// public
uint32_t ThreadLocalPtr::StaticMeta::PeekId () const
{
    std::lock_guard<std::mutex> lock (kLock_);
    if (!free_instance_ids_.empty ()) {
        return free_instance_ids_.back ();
    }

    return next_instance_id_;
}

// public
void* ThreadLocalPtr::StaticMeta::Get (uint32_t id)
{
    ThreadLocalPtr::ThreadData* data = GetThreadLocal ();
    if (UNLIKELY (id >= data->entries.size ())) {
        return nullptr;
    }

    return data->entries[id].ptr.load (std::memory_order_relaxed);
}

// public
void ThreadLocalPtr::StaticMeta::Reset (uint32_t id, void* ptr)
{
    assert (id >= 0);
    ThreadLocalPtr::ThreadData* data = GetThreadLocal ();
    if (UNLIKELY (id >= data->entries.size ())) {
        std::lock_guard<std::mutex> lock (kLock_);
        data->entries.resize (id + 1);
    }

    data->entries[id].ptr.store (ptr, std::memory_order_relaxed);
}

// public
void ThreadLocalPtr::StaticMeta::ReclaimId (uint32_t id)
{
    assert (id >= 0);
    std::lock_guard<std::mutex> lock (kLock_);
    UnrefHandler handler = GetHandler (id);
    for (ThreadLocalPtr::ThreadData* data = head_.next; data != &head_; data = data->next) {
        if (id < data->entries.size ()) {
            void *ptr = data->entries[id].ptr.exchange (nullptr, std::memory_order_relaxed);
            if (nullptr != ptr && nullptr != handler) {
                handler (ptr);
            }
        }
    }

    handler_map_[id] = nullptr;
    free_instance_ids_.push_back (id);
}

// public
void ThreadLocalPtr::StaticMeta::Scrape (uint32_t id,
                                         std::vector<void*>* ptrs,
                                         const void* replacement)
{
    std::lock_guard<std::mutex> lock (kLock_);
    for (ThreadLocalPtr::ThreadData *data = head_.next; data != &head_; data = data->next) {
        if (id < data->entries.size ()) {
            void *ptr = data->entries[id].ptr.exchange (
                const_cast<void*>(replacement), std::memory_order_relaxed);
            if (ptr != nullptr) {
                ptrs->push_back (ptr);
            }
        }
    }
}

// public
void* ThreadLocalPtr::StaticMeta::Swap (uint32_t id, void* ptr)
{
    assert (id >= 0);
    ThreadLocalPtr::ThreadData* data = GetThreadLocal ();
    if (UNLIKELY (id >= data->entries.size ())) {
        std::lock_guard<std::mutex> lock (kLock_);
        data->entries.resize (id + 1);
    }

    return data->entries[id].ptr.exchange (ptr, std::memory_order_relaxed);
}

// public
bool ThreadLocalPtr::StaticMeta::CompareAndSwap (uint32_t id, void* ptr, void*& expected)
{
    assert (id >= 0);
    ThreadLocalPtr::ThreadData* data = GetThreadLocal ();
    if (UNLIKELY (id >= data->entries.size ())) {
        std::lock_guard<std::mutex> lock (kLock_);
        data->entries.resize (id + 1);
    }

    return data->entries[id].ptr.compare_exchange_strong (expected, ptr,
           std::memory_order_relaxed, std::memory_order_relaxed);
}

// public
void ThreadLocalPtr::StaticMeta::SetHandler (uint32_t id, UnrefHandler handler)
{
    assert (id >= 0);
    std::lock_guard<std::mutex> lock (kLock_);
    handler_map_[id] = handler;
}

// private
UnrefHandler ThreadLocalPtr::StaticMeta::GetHandler (uint32_t id) const
{
    auto it = handler_map_.find (id);
    if (it == handler_map_.end ()) {
        return nullptr;
    }

    return it->second;
}

//private
void ThreadLocalPtr::StaticMeta::AddThreadData (ThreadData* data)
{
    data->next = &head_;
    data->prev = head_.prev;
    head_.prev->next = data;
    head_.prev = data;
}

// private
void ThreadLocalPtr::StaticMeta::RemoveThreadData (ThreadData* data)
{
    data->next->prev = data->prev;
    data->prev->next = data->next;
    data->next = data->prev = data;
}

// static private
void ThreadLocalPtr::StaticMeta::OnThreadExit (void* ptr)
{
    ThreadLocalPtr::ThreadData* data = static_cast<ThreadData*>(ptr);
    assert (nullptr != data);

    ThreadLocalPtr::StaticMeta* instance = Instance ();
    pthread_setspecific (instance->key_, nullptr);

    std::lock_guard<std::mutex> lock (kLock_);
    instance->RemoveThreadData (data);

    uint32_t id = 0;
    for (auto &it : data->entries) {
        void *p = it.ptr.load (std::memory_order_relaxed);
        if (nullptr != p) {
            UnrefHandler handler = instance->GetHandler (id);
            if (nullptr != handler) {
                handler (p);
            }
        }

        ++id;
    }

    delete data;
}

// static private
ThreadLocalPtr::ThreadData* ThreadLocalPtr::StaticMeta::GetThreadLocal ()
{
    ThreadLocalPtr::ThreadData* data = static_cast<ThreadLocalPtr::ThreadData*>(
        pthread_getspecific (Instance ()->key_));
    if (UNLIKELY (data == nullptr)) {
        auto instance = Instance ();
        data = new ThreadLocalPtr::ThreadData ();
        {
            std::lock_guard<std::mutex> lock (kLock_);
            instance->AddThreadData (data);
        }

        if (0 != pthread_setspecific (instance->key_, data)) {
            {
                std::lock_guard<std::mutex> lock (kLock_);
                instance->RemoveThreadData (data);
            }

            delete data;
            throw std::runtime_error ("pthread_setspecific failed");
        }
    }

    return data;
}

// public
ThreadLocalPtr::ThreadLocalPtr (UnrefHandler handler)
    : id_ (Instance ()->GetId ())
{
    if (nullptr != handler) {
        Instance ()->SetHandler (id_, handler);
    }
}

// public
ThreadLocalPtr::~ThreadLocalPtr ()
{
    Instance ()->ReclaimId (id_);
}

// public
void* ThreadLocalPtr::Get () const
{
    return Instance ()->Get (id_);
}

// public
void ThreadLocalPtr::Reset (void* ptr)
{
    Instance ()->Reset (id_, ptr);
}

// public
void* ThreadLocalPtr::Swap (void* ptr)
{
    return Instance ()->Swap (id_, ptr);
}

// public
bool ThreadLocalPtr::CompareAndSwap (void* ptr, void*& expected)
{
    return Instance ()->CompareAndSwap (id_, ptr, expected);
}

// public
void ThreadLocalPtr::Scrape (std::vector<void*>* ptrs, const void* replacement)
{
    Instance ()->Scrape (id_, ptrs, replacement);
}

// public
ThreadLocalPtr::StaticMeta* ThreadLocalPtr::Instance ()
{
    static ThreadLocalPtr::StaticMeta KInstance;
    return &KInstance;
}

} // swift
