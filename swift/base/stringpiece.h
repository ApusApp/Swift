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

#ifndef __SWIFT_BASE_STRING_PIECE_H__
#define __SWIFT_BASE_STRING_PIECE_H__

#include <stddef.h>

#include <iosfwd>
#include <string>

namespace swift {

template <typename __ST> 
class BasicStringPiece;

typedef BasicStringPiece<std::string> StringPiece;

namespace detail {

} // namespace detail 

template <typename __ST>
class BasicStringPiece
{
public:
    // Standard STL container boilerplate.
    typedef size_t size_type;
    typedef typename __ST::value_type value_type;
    typedef const value_type& const_reference;
    typedef const value_type& reference;
    typedef const value_type* pointer;
    typedef const value_type* const_iterator;
    typedef ptrdiff_t difference_type;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos;
    
public:
    BasicStringPiece () : ptr_ (nullptr), length_ (0)
    {

    }

    BasicStringPiece (const value_type* str) 
        : ptr_ (str)
        , length_ ((nullptr == str) ? 0 : __ST::traits_type::length (str))
    {

    }

    BasicStringPiece (const __ST& str) : ptr_ (str.data ()), length_ (str.size ())
    {

    }

private:
    value_type ptr_;
    size_type length_;
}; // BasicStringPiece

} // namespace swift

#endif // __SWIFT_BASE_STRING_PIECE_H__
