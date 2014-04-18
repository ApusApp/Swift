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

#ifndef __SWIFT_BASE_NONCOPYABLE_HPP__
#define __SWIFT_BASE_NONCOPYABLE_HPP__

namespace swift {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

namespace noncopyable_  // protection from unintended ADL
{
    class noncopyable
    {
    protected:
        noncopyable () {}
        ~noncopyable () {}

    private:  // emphasize the following members are private
        noncopyable (const noncopyable&);
        const noncopyable& operator= (const noncopyable&);
    };
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace swift

#endif  // __SWIFT_BASE_NONCOPYABLE_HPP__
