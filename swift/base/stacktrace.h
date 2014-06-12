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

#ifndef __SWIFT_BASE_STACK_TRACE_H__
#define __SWIFT_BASE_STACK_TRACE_H__

#include <string>

namespace swift {

class StackTrace
{
public:
    // get current program name
    static bool GetExecutableName (std::string* name);

    // init a signal handler to print callstack on the following signal:
    // SIGILL SIGSEGV SIGBUS SIGABRT
    static void InitStackTraceHandler ();

    static void PrintStack (int first_frames_to_skip = 0);

private:
    static void StackTraceHandler (int signal_num);
};
}
#endif // __SWIFT_BASE_STACK_TRACE_H__