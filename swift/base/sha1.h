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

#ifndef __SWIFT_BASE_SHA1_H__
#define __SWIFT_BASE_SHA1_H__

#include <string.h>
#include <string>

#include "swift/base/noncopyable.hpp"

namespace swift {
namespace detail {

struct Sha1Context
{
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
};

struct Sha1Digest
{
    Sha1Digest()
    {
        Init();
    }

    inline void Init()
    {
        memset(digest, 0, sizeof(digest));
    }

    unsigned char digest[20];
};

/**
 * @param [in] data
 * @param [in] length
 * @param [out] sha1
 */
void ComputeSha1(const void* data, uint32_t length, std::string* sha1);
void Sha1Init(Sha1Context* context);
void Sha1Update(Sha1Context* context,
                const unsigned char* data,
                uint32_t length);
void Sha1Final(Sha1Context* context, Sha1Digest* digest);
} // namespace detail

// Example:
//  Sha1 sha1;
//  sha1.Update ("abc", 3);
//  sha1.Update (StringPiece ("defg"));
//  sha1.Final ();
//  std::string result = sha1.ToString ();
// or
// std::string result;
// Sha1::Sha1Sum ("abcdefg", 7, &result);
// or
// std::string result;
// Sha1::Sha1Sum (StringPiece ("abcdefg"), &result);
class StringPiece;
class Sha1 : swift::noncopyable
{
public:
    Sha1();
    ~Sha1();

    void Update(const void* data, size_t length);
    void Update(const StringPiece& str);
    void Final();
    void Reset();
    std::string ToString() const;

    inline bool Valid() const
    {
        return complete_;
    }

    bool operator== (const Sha1& rhs) const;
    inline bool operator!= (const Sha1& rhs) const
    {
        return (*this == rhs) ? false : true;
    }

    // movable
    Sha1 (Sha1&&) = default;
    Sha1& operator=(Sha1&&) = default;
public:
    static void Sha1Sum(const StringPiece& str, std::string* out);
    /**
     * @param [in] data
     * @param [in] length
     * @param [out] sha1
     */
    static void Sha1Sum(const void* data, size_t length, std::string* sha1);
private:
    bool complete_;
    detail::Sha1Digest digest_;
    detail::Sha1Context context_;
};
} // namespace swift

#endif // __SWIFT_BASE_SHA1_H__
