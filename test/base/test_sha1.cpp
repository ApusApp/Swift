/*
 * Copyright (c) 2014 ApusApp
 *
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
#include <swift/base/sha1.h>
#include <gtest/gtest.h>
#include <openssl/sha.h>
#include <swift/base/stringpiece.h>

class test_Sha1 : public testing::Test
{
public:
    test_Sha1();
    ~test_Sha1();
};

std::string SSL_Sha1Sum(const void* data, size_t size)
{
    char tmp[3] = {'\0'};
    char buf[41] = {'\0'};
    swift::detail::Sha1Digest digest;
    SHA1(reinterpret_cast<const unsigned char*>(data), size, digest.digest);
    for (unsigned i = 0; i < sizeof(digest.digest); ++i) {
        snprintf(tmp, sizeof(tmp), "%02x", digest.digest[i]);
        strcat(buf, tmp);
    }

    return std::string(buf, sizeof(buf) - 1);
}

TEST(test_Sha1, All)
{
    swift::StringPiece str("abcdefghijklmnopqrstuvwxyz0123456789");
    std::string sha1;
    std::string ssl_sha1 = SSL_Sha1Sum(str.data(), str.length());
    swift::Sha1::Sha1Sum(str.data(), str.length(), &sha1);
    EXPECT_EQ(sha1, ssl_sha1);

    swift::Sha1 sha;
    EXPECT_FALSE(sha.Valid());

    sha.Update(str);
    sha.Final();
    EXPECT_EQ(sha.ToString(), ssl_sha1);
    sha.Reset();
    EXPECT_FALSE(sha.Valid());
    EXPECT_TRUE(sha.ToString().empty());

    sha.Update(str.data(), str.size());
    sha.Final();
    EXPECT_TRUE(sha.Valid());
    EXPECT_EQ(sha.ToString(), ssl_sha1);

    sha.Reset();
    for (unsigned i = 0; i < str.length(); ++i) {
        char c = str[i];
        sha.Update(&c, 1);
    }
    sha.Final();
    EXPECT_EQ(sha.ToString(), ssl_sha1);

    swift::Sha1 sha1_sec;
    sha1_sec.Update(str);
    sha1_sec.Final();
    EXPECT_EQ(sha1_sec, sha);
    EXPECT_TRUE(sha1_sec == sha);

    sha.Reset();
    sha.Update("abcdefghijklmnopqrstuvwxyz", 26);
    sha.Update(swift::StringPiece ("0123456789", 10));
    sha.Final();
    EXPECT_EQ(sha1_sec, sha);
    EXPECT_TRUE(sha1_sec == sha);
    EXPECT_EQ(sha.ToString(), ssl_sha1);

    std::string s;
    swift::Sha1::Sha1Sum(str, &s);
    EXPECT_EQ(s, sha1);
}