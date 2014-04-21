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

#include <execinfo.h> // backtrace and backtrace_symbols func

#include "swift/base/Exception.h"

namespace swift {

// public
Exception::Exception (const char* msg) : msg_ (msg)
{
    InitStackTrace ();
}

// public
Exception::Exception (const std::string& msg) : msg_ (msg)
{
    InitStackTrace ();
}

// public
Exception::Exception (std::string&& msg) : msg_ (std::move (msg))
{
    InitStackTrace ();
}

// public
Exception::~Exception () throw ()
{

}

// public
const char* Exception::GetStackTrace () const throw ()
{
    return stack_.c_str ();
}

// public
const char* Exception::what () const throw ()
{
    return msg_.c_str ();
}

// private
void Exception::InitStackTrace ()
{
    const int size = 256;
    void* buf[size];
    
    int n = ::backtrace (buf, size);
    char** stacks = ::backtrace_symbols (buf, n);
    if (stacks) {
        for (int i = 0; i < n; ++i) {
            stack_.append (stacks[i]);
            stack_.push_back ('\n');
        }
        free (stacks);
        stacks = nullptr;
    }

    // another implement can use abi::__cxa_demangle
}

} // namespace swift
