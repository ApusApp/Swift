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

#ifndef __SWIFT_BASE_SCOPE_GUARD_H__
#define __SWIFT_BASE_SCOPE_GUARD_H__

#include <functional>
#include <new>
#include <cstddef>

namespace swift {

/**
 * ScopeGuard is a general implementation of the "Initialization is
 * Resource Acquisition" idiom.  Basically, it guarantees that a function
 * is executed upon leaving the current scope unless otherwise told.
 *
 * The MakeScopeGuard () function is used to create a new ScopeGuard object.
 * It can be instantiated with a lambda function, a std::function<void()>,
 * a functor, or a void(*)() function pointer.
 *
 *
 * Usage example: Add a friend to memory if it is also added to the db.
 *
 * void User::AddFriend (User& newFriend) {
 *   // add the friend to memory
 *   friends_.push_back (&newFriend);
 *
 *   // If the db insertion that follows fails, we should
 *   // remove it from memory.
 *   // (You could also declare this as "auto guard = MakeScopeGuard (...)")
 *   swift::ScopeGuard guard = swift::MakeScopeGuard([&] { friends_.pop_back(); });
 *
 *   // this will throw an exception upon error, which
 *   // makes the ScopeGuard execute UserCont::pop_back()
 *   // once the Guard's destructor is called.
 *   db_->AddFriend(GetName (), newFriend.GetName ());
 *
 *   // an exception was not thrown, so don't execute
 *   // the Guard.
 *   guard.Dismiss ();
 * }
 * // For more, to see unit test
 *
 * Stolen from:
 *   Andrei's and Petru Marginean's CUJ article:
 *     http://drdobbs.com/184403758
 *   and the loki library:
 *     http://loki-lib.sourceforge.net/index.php?n=Idioms.ScopeGuardPointer
 *   and triendl.kj article:
 *     http://www.codeproject.com/KB/cpp/scope_guard.aspx
 */
 
namespace detail {

class ScopeGuardBase
{
public:
    inline void Dismiss () noexcept
    {
        dismissed_ = true;
    }

protected:
    ScopeGuardBase () : dismissed_ (false)
    {
    }

    ScopeGuardBase (ScopeGuardBase&& other)
        : dismissed_ (other.dismissed_)
    {
        other.dismissed_ = true;
    }

protected:
    bool dismissed_;
};

template <typename FuncT>
class ScopeGuardImpl : public ScopeGuardBase
{
public:
    explicit ScopeGuardImpl (const FuncT& func)
        : function_ (func)
    {
    }

    explicit ScopeGuardImpl (FuncT&& func)
        : function_ (std::move (func))
    {
    }

    ScopeGuardImpl (ScopeGuardImpl&& other)
        : ScopeGuardBase (std::move (other))
        , function_ (std::move (other.function_))
    {
        other.dismissed_ = true;
    }

    ~ScopeGuardImpl ()
    {
        if (!dismissed_) {
            function_ ();
        }
    }

private:
    void* operator new(size_t) = delete;

private:
    FuncT function_;
};

// Internal use for the macro SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT below
enum class ScopeGuardOnExit {};

template <typename FuncT>
ScopeGuardImpl<typename std::decay<FuncT>::type>
operator+ (detail::ScopeGuardOnExit, FuncT&& func) {
    return ScopeGuardImpl<typename std::decay<FuncT>::type> (
        std::forward<FuncT> (func));
}

} // namespace detail

template <typename FuncT>
detail::ScopeGuardImpl<typename std::decay<FuncT>::type>
MakeScopeGuard (FuncT&& func) {
    return detail::ScopeGuardImpl<typename std::decay<FuncT>::type> (
        std::forward<FuncT> (func));
}


typedef detail::ScopeGuardBase&& ScopeGuard;

} // namespace swift

#ifndef SWIFT_ANONYMOUS_VARIABLES
#define SWIFT_ANONYMOUS_VARIABLES_IMPL(s1, s2) s1##s2
#define SWIFT_ANONYMOUS_VARIABLES(str) SWIFT_ANONYMOUS_VARIABLES_IMPL(str, __LINE__)
#endif
#ifndef SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT
#define SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT \
    auto SWIFT_ANONYMOUS_VARIABLES(SCOPE_GUARD_EXIT_STATE) \
    = ::swift::detail::ScopeGuardOnExit () + [&]() noexcept

#endif

#endif // __SWIFT_BASE_SCOPE_GUARD_H__