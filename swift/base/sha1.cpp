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

#include "swift/base/sha1.h"
#include <swift/base/stringpiece.h>

namespace swift {
namespace detail {

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

// blk0() and blk() perform the initial expand.
#ifdef ARCH_CPU_BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | \
    (rol(block->l[i], 8) & 0x00FF00FF))
#endif

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ \
    block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

// (R0 + R1), R2, R3, R4 are the different operations used in Sha1.
#define R0(v, w, x, y, z, i) \
    z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
    w = rol(w, 30);

#define R1(v, w, x, y, z, i) \
    z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
    w = rol(w, 30);

#define R2(v, w, x, y, z, i) \
    z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
    w = rol(w, 30);

#define R3(v, w, x, y, z, i) \
    z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
    w = rol(w, 30);

#define R4(v, w, x, y, z, i) \
    z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
    w = rol(w, 30);

static std::string Sha1DigestToString(const Sha1Digest* digest)
{
    static char const zEncode[] = "0123456789abcdef";
    std::string ret;
    ret.resize(40);

    int j = 0;
    int n = 0;
    for (int i = 0; i < 20; ++i) {
        n = digest->digest[i];
        ret[j++] = zEncode[(n >> 4) & 0xf];
        ret[j++] = zEncode[n & 0xf];
    }

    return ret;
}

static void Sha1Transform(uint32_t state[5], const uint8_t buffer[64])
{
    union CHAR64LONG16
    {
        uint8_t c[64];
        uint32_t l[16];
    };

    CHAR64LONG16 block[1];
    memcpy(block, buffer, 64);

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];

    R0(a,b,c,d,e,0); R0(e,a,b,c,d,1); R0(d,e,a,b,c,2); R0(c,d,e,a,b,3);
    R0(b,c,d,e,a,4); R0(a,b,c,d,e,5); R0(e,a,b,c,d,6); R0(d,e,a,b,c,7);
    R0(c,d,e,a,b,8); R0(b,c,d,e,a,9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    // Add the working vars back into context.state[].
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

// static
void Sha1Init(Sha1Context* context)
{
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

// static
void Sha1Update(Sha1Context* context, const unsigned char* data, uint32_t length)
{
    uint32_t i = 0;
    uint32_t j = context->count[0];
    context->count[0] += (length << 3);
    if (context->count[0] < j) {
        ++context->count[1];
    }
    context->count[1] += (length >> 29);
    j = (j >> 3) & 63;
    if ((j + length) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        Sha1Transform(context->state, context->buffer);
        for (; i + 63 < length; i += 64) {
            Sha1Transform(context->state, data + i);
        }
        j = 0;
    } else {
        i = 0;
    }

    assert(length >= i);
    memcpy(&context->buffer[j], &data[i], length - i);
}

// static
void Sha1Final(Sha1Context* context, Sha1Digest* digest)
{
    unsigned char final_count[8];

    for (uint32_t i = 0; i < 8; ++i) {
        final_count[i] = static_cast<unsigned char>((context->count[(i >= 4 ? 0 : 1)] >>
            ((3 - (i & 3)) * 8)) & 255);
    }

    unsigned char c = 0200;
    Sha1Update(context, &c, 1);
    while (448 != (context->count[0] & 504)) {
        c = 0000;
        Sha1Update(context, &c, 1);
    }

    Sha1Update(context, final_count, 8);
    for (uint32_t i = 0; i < 20; ++i) {
        digest->digest[i] = static_cast<unsigned char>((context->state[i >> 2] >>
            ((3 - (i & 3)) * 8)) & 255);
    }

    memset(context, '\0', sizeof(*context));
}

// static
void ComputeSha1(const void* data, uint32_t length, std::string* sha1)
{
    assert(nullptr != sha1);
    Sha1Digest digest;
    Sha1Context context;
    Sha1Init(&context);
    Sha1Update(&context, reinterpret_cast<const unsigned char*>(data), length);
    Sha1Final(&context, &digest);
    *sha1 = std::move(Sha1DigestToString(&digest));
}

} // namespace detail

// public
Sha1::Sha1() : complete_(false), digest_()
{
    detail::Sha1Init(&context_);
}

// public
Sha1::~Sha1()
{
    complete_ = false;
}

// public
void Sha1::Update(const StringPiece& str)
{
    Update(str.data(), str.length());
}

// public
std::string Sha1::ToString() const
{
    std::string ret;
    if (complete_) {
       ret = std::move(detail::Sha1DigestToString(&digest_));
    }
    return ret;
}

// static public
void Sha1::Sha1Sum(const StringPiece& str, std::string* out)
{
    assert(nullptr != out);
    if (!str.empty()) {
        detail::ComputeSha1(str.data(), static_cast<uint32_t>(str.size()), out);
    }
}

} // namespace swift