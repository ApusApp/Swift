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

// The original file was copied from sqlite, and was in the public domain.

/*
* This code implements the MD5 message-digest algorithm.
* The algorithm is due to Ron Rivest.  This code was
* written by Colin Plumb in 1993, no copyright is claimed.
* This code is in the public domain; do with it what you wish.
*
* Equivalent code is available from RSA Data Security, Inc.
* This code has been tested against that, and is equivalent,
* except that you don't need to include two pages of legalese
* with every copy.
*
* To compute the message digest of a chunk of bytes, declare an
* MD5Context structure, pass it to MD5Init, call MD5Update as
* needed on buffers full of bytes, and then call MD5Final, which
* will fill a supplied 16-byte array with the digest.
*/

#include <cstring>
#include <stdlib.h>
#include <assert.h>

#include "swift/base/md5.h"
#include "swift/base/stringpiece.h"

namespace {
struct Context
{
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
};

// Note: this code is harmless on little-endian machines.
static void ByteReverse (unsigned char* buf, unsigned longs)
{
    uint32_t t = 0;
    do {
        t = (uint32_t)((unsigned)buf[3] << 8 | buf[2]) << 16 |
                       ((unsigned)buf[1] << 8 | buf[0]);
        *(uint32_t *)buf = t;
        buf += 4;
    } while (--longs);
}

// The four core functions - F1 is optimized somewhat

//#define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) \
    (w += f(x, y, z) + data, w = w << s | w >> (32 - s), w += x)

// The core of the MD5 algorithm, this alters an existing MD5 has to
// reflect the addition of 16 longwords of new data. MD5Update blocks
// the data and converts bytes into longwords for this routine.
static void MD5Transform (uint32_t buf[4], const uint32_t in[16])
{
    register uint32_t a;
    register uint32_t b;
    register uint32_t c;
    register uint32_t d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[ 0] + 0xd76aa478,  7);
    MD5STEP(F1, d, a, b, c, in[ 1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[ 2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[ 3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[ 4] + 0xf57c0faf,  7);
    MD5STEP(F1, d, a, b, c, in[ 5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[ 6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[ 7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[ 8] + 0x698098d8,  7);
    MD5STEP(F1, d, a, b, c, in[ 9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122,  7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[ 1] + 0xf61e2562,  5);
    MD5STEP(F2, d, a, b, c, in[ 6] + 0xc040b340,  9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[ 0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[ 5] + 0xd62f105d,  5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453,  9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[ 4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[ 9] + 0x21e1cde6,  5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6,  9);
    MD5STEP(F2, c, d, a, b, in[ 3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[ 8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905,  5);
    MD5STEP(F2, d, a, b, c, in[ 2] + 0xfcefa3f8,  9);
    MD5STEP(F2, c, d, a, b, in[ 7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[ 5] + 0xfffa3942,  4);
    MD5STEP(F3, d, a, b, c, in[ 8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[ 1] + 0xa4beea44,  4);
    MD5STEP(F3, d, a, b, c, in[ 4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[ 7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6,  4);
    MD5STEP(F3, d, a, b, c, in[ 0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[ 3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[ 6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[ 9] + 0xd9d4d039,  4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[ 2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[ 0] + 0xf4292244,  6);
    MD5STEP(F4, d, a, b, c, in[ 7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[ 5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3,  6);
    MD5STEP(F4, d, a, b, c, in[ 3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[ 1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[ 8] + 0x6fa87e4f,  6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[ 6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[ 4] + 0xf7537e82,  6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[ 2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[ 9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

} // namespace

namespace swift {
namespace detail {

// MD5 stands for Message Digest algorithm 5.
// MD5 is a robust hash function, designed for cyptography, but often used
// for file checksums.  The code is complex and slow, but has few
// collisions.
// See Also:
//   http://en.wikipedia.org/wiki/MD5

// These functions perform MD5 operations. The simplest call is MD5Sum() to
// generate the MD5 sum of the given data.
//
// You can also compute the MD5 sum of data incrementally by making multiple
// calls to MD5Update():
//   MD5Context ctx; // intermediate MD5 data: do not use
//   MD5Init(&ctx);
//   MD5Update(&ctx, data1, length1);
//   MD5Update(&ctx, data2, length2);
//   ...
//
//   MD5Digest digest; // the result of the computation
//   MD5Final(&digest, &ctx);
//
// You can call MD5DigestToString () to generate a string of the digest.

// Initializes the given MD5 context structure for subsequent calls to MD5Update ().
static inline void MD5Init (MD5Context* context)
{
    struct Context *ctx = reinterpret_cast<struct Context*>(context);
    ctx->buf[ 0] = 0x67452301;
    ctx->buf[ 1] = 0xefcdab89;
    ctx->buf[ 2] = 0x98badcfe;
    ctx->buf[ 3] = 0x10325476;
    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

// For the given buffer of data as a StringPiece, updates the given MD5
// context with the sum of the data. You can call this any numbers of times
// during the computation, except that MD5Init () must have been called first.
static void MD5Update (MD5Context* context, const StringPiece& data)
{
    struct Context *ctx = reinterpret_cast<struct Context*>(context);
    const unsigned char *buf = reinterpret_cast<const unsigned char*>(data.data ());
    size_t len = data.size ();

    uint32_t t = ctx->bits[0];
    ctx->bits[0] = t + ((uint32_t)len << 3);
    if (ctx->bits[0] < t) {
        ctx->bits[1]++;
    }
    ctx->bits[1] += static_cast<uint32_t>(len >> 29);

    t = (t >> 3) & 0x3f;
    if (t) {
        unsigned char *p = static_cast<unsigned char*>(ctx->in) + t;
        t = 64 - t;
        if (len < t) {
            memcpy (p, buf, len);
            return;
        }

        memcpy (p, buf, t);
        ByteReverse (ctx->in, 16);
        MD5Transform (ctx->buf, reinterpret_cast<const uint32_t*>(ctx->in));
        buf += t;
        len -= t;
    }

    // Process data in 64-byte chunks
    while (len >= 64) {
        memcpy (ctx->in, buf, 64);
        ByteReverse (ctx->in, 16);
        MD5Transform (ctx->buf, reinterpret_cast<const uint32_t*>(ctx->in));
        buf += 64;
        len -= 64;
    }

    // Handle any remaining bytes of data.
    memcpy (ctx->in, buf, len);
}

// MD5IntermediateFinal () generates a digest without finalizing the MD5
// operation. Can be used to generte digests for the input seen thus far,
// without affecting the digest generated for the entire input.
static void MD5Final (MD5Digest* digest, MD5Context* context)
{
    struct Context *ctx = reinterpret_cast<struct Context*>(context);

    // Compute number of bytes mod 64
    unsigned count = (ctx->bits[0] >> 3) & 0x3f;

    // Set the first char of padding to 0x80.
    // This is safe since there is always at least one byte free.
    unsigned char *p = ctx->in + count;
    *p++ = 0x80;

    // Bytes of padding needed to make 64 bytes
    count = 64 - 1 - count;

    // pad out to 56 mod 64
    if (count < 8) {
        memset (p, 0, count);
        ByteReverse (ctx->in, 16);
        MD5Transform (ctx->buf, reinterpret_cast<const uint32_t*>(ctx->in));
        memset (ctx->in, 0, 56);
    }
    else {
        memset (p, 0, count - 8);
    }

    ByteReverse (ctx->in, 14);

    // Append length in bits and transform
    ((uint32_t *)ctx->in)[14] = ctx->bits[0];
    ((uint32_t *)ctx->in)[15] = ctx->bits[1];

    MD5Transform (ctx->buf, reinterpret_cast<const uint32_t*>(ctx->in));
    ByteReverse ((unsigned char *)ctx->buf, 4);
    memcpy (digest->digest, ctx->buf, 16);
    memset (ctx, 0, sizeof (*ctx));
}

// Computes the MD5 sum of the given data buffer with the given length.
// The given 'digest' structure will be filled with the result data.
static inline void MD5Sum (const void* data, size_t length, MD5Digest* digest)
{
    MD5Context ctx;
    MD5Init (&ctx);
    MD5Update (&ctx, StringPiece (reinterpret_cast<const char*>(data), length));
    MD5Final (digest, &ctx);
}

// Converts a digest into human-readable hexadecimal.
static std::string MD5DigestToString (const MD5Digest& digest)
{
    static char const zEncode[] = "0123456789abcdef";

    std::string ret;
    ret.resize (32);

    int j = 0;
    for (int i = 0; i < 16; i++) {
        int a = digest.digest[i];
        ret[j++] = zEncode[(a >> 4) & 0xf];
        ret[j++] = zEncode[a & 0xf];
    }

    return ret;
}

// Returns the MD5 (in hexadecimal) of a string.
static inline std::string MD5String (const StringPiece& str)
{
    MD5Digest digest;
    MD5Sum (str.data (), str.length (), &digest);
    return MD5DigestToString (digest);
}

// Finalizes the MD5 operation and fills the buffer with the digest
static inline void MD5IntermediateFinal (MD5Digest* digest, const MD5Context* context)
{
    MD5Context context_copy;
    memcpy (&context_copy, context, sizeof (context_copy));
    MD5Final (digest, &context_copy);
}

} // namespace detail

// public
MD5::MD5 () : complete_ (false), digest_ ()
{
    bzero (&context_, sizeof (context_));
    detail::MD5Init (&context_);
}

// public
MD5::~MD5 ()
{
    complete_ = false;
}

// public
void MD5::Update (const void* data, size_t length)
{
    if (data && length) {
        detail::MD5Update (&context_, StringPiece (reinterpret_cast<const char*>(data), length));
        complete_ = true;
    }
}

// public
void MD5::Update (const StringPiece& str)
{
    if (!str.empty ()) {
        detail::MD5Update (&context_, str);
        complete_ = true;
    }
}

// public
void MD5::Final ()
{
    if (complete_) {
        detail::MD5Final (&digest_, &context_);
    }
}

// public
void MD5::Reset ()
{
    digest_.Init ();
    bzero (&context_, sizeof (context_));
    detail::MD5Init (&context_);
    complete_ = false;
}

// public
std::string MD5::ToString () const
{
    std::string ret;
    if (complete_) {
        ret = std::move (detail::MD5DigestToString (digest_));
    }

    return ret;
}

// public
bool MD5::operator== (const MD5& rhs) const
{
    if (!complete_ && !rhs.complete_) {
        return true;
    }

    return (0 == memcmp(digest_.digest, rhs.digest_.digest, sizeof(digest_.digest)));
}

// static public
void MD5::Md5Sum (const StringPiece& str, std::string& out)
{
    if (!str.empty ()) {
        out = std::move (detail::MD5String (str));
    }
}

// static public
void MD5::Md5Sum (const void* data, size_t length, std::string& out)
{
    if (data && length) {
        const char* p = reinterpret_cast<const char*>(data);
        StringPiece str (p, length);
        out = std::move (detail::MD5String (str));
    }
}

} // namespace swift
