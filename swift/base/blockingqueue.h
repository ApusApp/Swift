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
#ifndef __SWIFT_BASE_BLOCKING_QUEUE_H__
#define __SWIFT_BASE_BLOCKING_QUEUE_H__

#include <deque>
#include <mutex>
#include <assert.h>
#include <condition_variable>

#include "swift/base/noncopyable.hpp"

namespace swift {

template <typename T>
class BlockingQueue : swift::noncopyable
{
    typedef std::lock_guard<std::mutex> LockGuard;
public:
    BlockingQueue () : mutex_ (), cond_ (), queue_ ()
    {

    }

    ~BlockingQueue ()
    {

    }

    void Put (const T& task)
    {
        LockGuard lock (mutex_);
        queue_.push_back (task);
        cond_.notify_one ();
    }

    void Put (T&& task)
    {
        LockGuard lock (mutex_);
        queue_.push_back (std::move (task));
        cond_.notify_one ();
    }

    T Take ()
    {
        std::unique_lock<std::mutex> lock (mutex_);
        cond_.wait (lock, [this]{return !queue_.empty ();});
        assert (!queue_.empty ());
        T front (std::move (queue_.front ()));
        queue_.pop_front ();

        return front;
    }

    size_t Size () const
    {
        LockGuard lock (mutex_);
        return queue_.size ();
    }

    bool IsEmpty () const
    {
        LockGuard lock (mutex_);
        return queue_.empty ();
    }

    void Clear ()
    {
        LockGuard lock (mutex_);
        queue_.clear ();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    std::deque<T> queue_;
};

} // namespace swift

#endif // __SWIFT_BASE_BLOCKING_QUEUE_H__
