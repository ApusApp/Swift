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

#include <thread>
#include <assert.h>

#include "swift/base/threadpool.h"
#include "swift/base/exception.h"
#include "swift/base/logging.h"

namespace swift {
namespace detail {

// This is based on haskell's MVar synchronization primitive:
// http://www.haskell.org/ghc/docs/latest/html/libraries/base/Control-Concurrent-MVar.html
//
// It is a thread-safe queue that can hold at most one object.
// You can also think of it as a box that can be either full or empty.
template <typename T>
class MVar
{
    typedef std::unique_lock<std::recursive_mutex> ScopedLock;
public:
    enum State
    {
        EMPTY = 0,
        FULL,
    };

    MVar () : state_ (EMPTY), value_ (), mutex_ (), condition_ ()
    {

    }

    MVar (const T& val) : state_ (FULL), value_ (val), mutex_ (), condition_ ()
    {

    }

    MVar (T&& val) : state_ (FULL), value_ (std::move (val)), mutex_ (), condition_ ()
    {

    }

    // puts val into the MVar and returns true or returns false if full
    // never blocks
    bool TryPut (const T& val)
    {
        // intentionally repeat test before and after lock
        if (FULL == state_) {
            return false;
        }

        ScopedLock lock (mutex_);
        if (FULL == state_) {
            return false;
        }
        state_ = FULL;
        value_ = val;

        // unblock threads waiting to 'Take'
        condition_.notify_all ();

        return true;
    }

    bool TryPut (T&& val)
    {
        // intentionally repeat test before and after lock
        if (FULL == state_) {
            return false;
        }

        ScopedLock lock (mutex_);
        if (FULL == state_) {
            return false;
        }
        state_ = FULL;
        value_ = std::move (val);

        // unblock threads waiting to 'Take'
        condition_.notify_all ();

        return true;
    }

    // put val into the MVar will block if the MVar is already full
    void Put (const T& val)
    {
        ScopedLock lock (mutex_);
        while (!TryPut (val)) {
            // unlocks lock while waiting and relocks before returning
            condition_.wait (lock);
        }
    }

    void Put (T&& val)
    {
        ScopedLock lock (mutex_);
        while (!TryPut (std::move (val))) {
            // unlocks lock while waiting and relocks before returning
            condition_.wait (lock);
        }
    }

    // takes val out of the MVar and returns true or returns false if empty
    // never blocks
    bool TryTake (T& out)
    {
        // intentionally repeat test before and after lock
        if (EMPTY == state_) {
            return false;
        }

        ScopedLock lock (mutex_);
        if (EMPTY == state_) {
            return false;
        }

        state_ = EMPTY;
        out = std::move (value_);
        value_ = nullptr;

        // unblock threads waiting to 'Put'
        condition_.notify_all ();

        return true;
    }

    // takes val out of the MVar
    // will block if the MVar is empty
    T Take ()
    {
        T ret = T ();
        ScopedLock lock (mutex_);
        while (!TryTake (ret)) {
            // unlocks lock while waiting and relocks before returning
            condition_.wait (lock);
        }

        return ret;
    }

    // Note: this is fast because there is no locking, but state could
    // change before you get a chance to act on it.
    // Mainly useful for sanity checks / asserts.
    State GetState ()
    {
        return state_;
    }

private:
    State state_;
    T value_;
    std::recursive_mutex mutex_;
    std::condition_variable_any condition_;
};

} // namespace detail

class ThreadPool::Worker : swift::noncopyable
{
public:
    explicit Worker (ThreadPool& owner)
        : owner_ (owner)
        , is_done_ (true)
        , thread_ (std::bind (&Worker::Loop, this))
    {
    }

    // destructor will block until current operation is completed
    // Acts as a "join" on this thread
    ~Worker ()
    {
        task_.Put (ThreadPool::Task ());
        thread_.join ();
    }

    void SetTask (const ThreadPool::Task& t)
    {
        assert (nullptr != t);
        assert (is_done_);
        is_done_ = false;
        task_.Put (t);
    }

    void SetTask (ThreadPool::Task&& t)
    {
        assert (nullptr != t);
        assert (is_done_);
        is_done_ = false;
        task_.Put (std::move (t));
    }

private:
    void Loop ()
    {
        while (true) {
            ThreadPool::Task t = std::move (task_.Take ());
            if (nullptr == t) {
                break; // ends the thread
            }

            try {
                t ();
            }
            catch (Exception& ex) {
                LOG(ERROR) << "Unhandled Exception: " << ex.what () << "\n";
                LOG(ERROR) << "BackStack: " << ex.GetStackTrace () << "\n";
            }
            catch (std::exception& ex) {
                LOG(ERROR) << "Unhandled std::exception: " << ex.what () << "\n";
            }
            catch (...) {
                LOG(ERROR) << "Unhandled non-exception in worker thread\n";
            }

            is_done_ = true;
            owner_.TaskDone (this);
        }
    }

private:
    ThreadPool& owner_;
    detail::MVar<ThreadPool::Task> task_;
    std::atomic<bool> is_done_;
    std::thread thread_;
};

// public
ThreadPool::ThreadPool (int threads_number /*= 4*/)
    : mutex_ ()
    , condition_ ()
    , free_workers_ ()
    , tasks_ ()
    , tasks_remaining_ (0)
    , threads_number_ (threads_number)
{
    assert (threads_number > 0);
}

// public
ThreadPool::~ThreadPool ()
{
    Join ();
    assert (tasks_.empty ());
    assert (static_cast<int>(free_workers_.size ()) == threads_number_);
    while (!free_workers_.empty ()) {
        delete free_workers_.front ();
        free_workers_.pop_front ();
    }
}

// public
void ThreadPool::Start ()
{
    assert (threads_number_ > 0);
    int count = threads_number_;
    std::lock_guard<std::mutex> lock (mutex_);
    while (count-- > 0) {
        Worker* worker = new Worker (*this);
        free_workers_.push_back (worker);
    }
}

// public
void ThreadPool::Join ()
{
    std::unique_lock<std::mutex> lock (mutex_);
    while (tasks_remaining_) {
        condition_.wait (lock);
    }
}

// public
void ThreadPool::Schedule (const Task& task)
{
    std::lock_guard<std::mutex> lock (mutex_);
    ++tasks_remaining_;
    if (!free_workers_.empty ()) {
        free_workers_.front ()->SetTask (task);
        free_workers_.pop_front ();
    }
    else {
        tasks_.push_back (task);
    }
}

//public
void ThreadPool::Schedule (Task&& task)
{
    std::lock_guard<std::mutex> lock (mutex_);
    ++tasks_remaining_;
    if (!free_workers_.empty ()) {
        free_workers_.front ()->SetTask (std::move (task));
        free_workers_.pop_front ();
    }
    else {
        tasks_.push_back (std::move (task));
    }
}

// private
void ThreadPool::TaskDone (Worker* worker)
{
    std::lock_guard<std::mutex> lock (mutex_);
    if (!tasks_.empty ()) {
        worker->SetTask (tasks_.front ());
        tasks_.pop_front ();
    }
    else {
        free_workers_.push_front (worker);
    }

    if (0 == --tasks_remaining_) {
        condition_.notify_all ();
    }
}

} // namespace swift