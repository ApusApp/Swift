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

#ifndef __SWIFT_BASE_MD5_H__
#define __SWIFT_BASE_MD5_H__

#include <string>
#include "swift/base/noncopyable.hpp"

namespace swift {
namespace detail {
// The output of an MD5 operation.
struct MD5Digest
{
    MD5Digest ()
    {
        Init ();
    }

    inline void Init ()
    {
        memset (digest, 0, sizeof(digest));
    }

    unsigned char digest[16];
};

// Used for storing intermediate data during an MD5 computation. Callers
// should not access the data.
typedef char MD5Context[88];

} // namespace detail

// Example:
//  MD5 md5;
//  md5.Update ("abc", 3);
//  md5.Update (StringPiece ("defg"));
//  md5.Final ();
//  std::string result = md5.ToString ();
// or
// std::string result;
// MD5::Md5Sum ("abcdefg", 7, result);
// or
// std::string result;
// MD5::Md5Sum (StringPiece ("abcdefg"), result);
class StringPiece;
class MD5 : swift::noncopyable
{
public:
    MD5 ();
    ~MD5 ();

    void Update (const void* data, size_t length);
    void Update (const StringPiece& str);
    void Final ();
    void Reset ();
    std::string ToString () const;

    inline bool Valid () const
    {
        return complete_;
    }

    bool operator== (const MD5& rhs) const;

    inline bool operator!= (const MD5& rhs) const
    {
        return (*this == rhs) ? false : true;
    }

    // movable
    MD5 (MD5&&) = default;
    MD5& operator= (MD5&&) = default;

public:
    static void Md5Sum (const StringPiece& str, std::string& out);
    static void Md5Sum (const void* data, size_t length, std::string& out);

private:
    bool complete_;
    detail::MD5Digest digest_;
    detail::MD5Context context_;
}; // MD5

} // namespace swift

#endif // __SWIFT_BASE_MD5_H__
