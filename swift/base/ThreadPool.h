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

#ifndef __SWIFT_BASE_THREAD_POOL_H__
#define __SWIFT_BASE_THREAD_POOL_H__

#include <mutex>
#include <list>
#include <atomic>
#include <functional>
#include <condition_variable>

#include "swift/base/noncopyable.hpp"

namespace swift {

class ThreadPool : swift::noncopyable
{
    class Worker;
public:
    typedef std::function<void (void)> Task;

public:
    explicit ThreadPool (int threads_number = 4);

    // blocks until all tasks are complete (TasksRemaining () == 0)
    // You should not call schedule while in the destructor
    ~ThreadPool ();

    // must call this function at first use
    void Start ();

    // blocks until all tasks are complete (TasksRemaining () == 0)
    // does not prevent new tasks from being scheduled so could wait forever.
    // Also, new tasks could be scheduled after this returns.
    void Join ();

    // task will be copied a few times so make sure it's relatively cheap
    void Schedule (const Task& task);

    void Schedule (Task&& task);

    // Helpers that wrap schedule and std::bind.
    // Functor and args will be copied a few times so make sure it's relatively cheap
    template<typename F, typename A>
    void Schedule (F f, A a) 
    { 
        Schedule (std::move (std::bind (f, a))); 
    }

    template<typename F, typename A, typename B>
    void Schedule (F f, A a, B b) 
    { 
        Schedule (std::move (std::bind (f, a, b)));
    }

    template<typename F, typename A, typename B, typename C>
    void Schedule (F f, A a, B b, C c) 
    { 
        Schedule (std::move (std::bind (f, a, b, c)));
    }

    template<typename F, typename A, typename B, typename C, typename D>
    void Schedule (F f, A a, B b, C c, D d) 
    { 
        Schedule (std::move (std::bind (f, a, b, c, d)));
    }

    template<typename F, typename A, typename B, typename C, typename D, typename E>
    void Schedule (F f, A a, B b, C c, D d, E e) 
    { 
        Schedule (std::move (std::bind (f, a, b, c, d, e)));
    }

    int TasksRemaining () const
    { 
        return tasks_remaining_; 
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;

    std::list<Worker*> free_workers_;   //used as LIFO stack (always front)
    std::list<Task> tasks_;             //used as FIFO queue (push_back, pop_front)
    std::atomic<int> tasks_remaining_;  // in queue + currently processing
    int threads_number_;                // only used for sanity checking. could be removed in the future.

    // should only be called by a worker from the worker's thread
    void TaskDone (Worker* worker);

    friend class Worker;
};

} // namespace swift
#endif //__SWIFT_BASE_THREAD_POOL_H__