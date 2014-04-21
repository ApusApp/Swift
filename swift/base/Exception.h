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

#ifndef __SWIFT_BASE_EXCEPTION_H__
#define __SWIFT_BASE_EXCEPTION_H__

#include <string>
#include <exception>

namespace swift {

class Exception : public std::exception
{
public:
    explicit Exception (const char* msg);
    explicit Exception (const std::string& msg);
    explicit Exception (std::string&& msg);

    virtual ~Exception () throw ();

    const char* GetStackTrace () const throw ();

    // Returns a C-style character string describing the general cause of the current error. 
    virtual const char* what () const throw ();

private:
    void InitStackTrace ();

private:
    std::string msg_;
    std::string stack_;
};
} // namespace swift

#endif // __SWIFT_BASE_EXCEPTION_H__
