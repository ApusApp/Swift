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

#include <swift/net/httpclient/httpclient.h>
#include <gtest/gtest.h>
#include <swift/base/stringpiece.h>
#include <swift/base/file.h>

class test_HttpClient : public testing::Test
{
public:
    test_HttpClient();
    ~test_HttpClient();

    virtual void SetUp (void)
    {
    }

    virtual void TearDown (void)
    {

    }
};

TEST(test_HttpClient, Get)
{
    swift::Request req;
    swift::File file;
    file.Open("/tmp/get.txt");
    req.SetUrl(std::move(std::string("http://www.iqiyi.com")));

    swift::Response resp;
    swift::HttpClient client;

    int code = client.Get(&req, &resp, &file);
    ASSERT_EQ(code, resp.GetStatusCode());
    long size = resp.ContentLength();
    ASSERT_EQ(file.GetFileSize(), size);

    resp.Reset();
    code = client.Get(&req, &resp);
    ASSERT_EQ(code, resp.GetStatusCode());
    size = resp.ContentLength();
    ASSERT_EQ(resp.GetBody().size(), size);
}

