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

    inline void Update(const void* data, size_t length)
    {
        if (data && length) {
            detail::Sha1Update(&context_,
                               reinterpret_cast<const unsigned char*>(data),
                               static_cast<uint32_t>(length));
            complete_ = true;
        }
    }

    void Update(const StringPiece& str);

    inline void Final()
    {
        if (complete_) {
            detail::Sha1Final(&context_, &digest_);
        }
    }

    inline void Reset()
    {
        digest_.Init();
        memset(&context_, 0, sizeof(context_));
        detail::Sha1Init(&context_);
        complete_ = false;
    }

    std::string ToString() const;

    inline bool Valid() const
    {
        return complete_;
    }

    inline bool operator== (const Sha1& rhs) const
    {
        if (!complete_ && !rhs.complete_) {
            return true;
        }

        return (0 == memcmp(digest_.digest,
                            rhs.digest_.digest,
                            sizeof(digest_.digest)));
    }

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
    static inline void Sha1Sum(const void* data,
                               size_t length,
                               std::string* sha1)
    {
        if (data && length) {
            detail::ComputeSha1(data, static_cast<uint32_t>(length), sha1);
        }
    }

private:
    bool complete_;
    detail::Sha1Digest digest_;
    detail::Sha1Context context_;
};

} // namespace swift

#endif // __SWIFT_BASE_SHA1_H__
