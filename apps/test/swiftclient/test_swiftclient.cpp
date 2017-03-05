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

#include <iostream>
#include <gtest/gtest.h>
#include <swiftclient/swiftclient.h>

class test_SwiftClient : public testing::Test
{
public:
    test_SwiftClient() {}
    ~test_SwiftClient() {}

    virtual void SetUp (void)
    {
    }

    virtual void TearDown (void)
    {

    }
};

TEST_F(test_SwiftClient, Uri)
{
    std::string host("127.0.0.1");
    int port = 8080;
    std::string account("account");
    std::string container("container");
    std::string object("object");

    SwiftClient client;
    client.SetHost(host);
    client.SetPort(port);
    std::string uri = std::move(client.Url(&account));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account");

    uri = std::move(client.Url(&account, &container));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account/container");

    uri = std::move(client.Url(&account, &container, &object));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account/container/object");

    uri = std::move(client.Url(nullptr, &container, &object));
    ASSERT_TRUE(uri.empty());

    uri = std::move(client.Url(&account, nullptr, &object));
    ASSERT_TRUE(uri.empty());

    std::string empty;
    uri = std::move(client.Url(&account, &empty, &empty));
    ASSERT_TRUE(uri.empty());

    SwiftClient::query_map_type query;
    query["format"] = "json";
    uri = std::move(client.Url(&account, &container, &object, &query));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account/container/object?format=json");

    uri = std::move(client.Url(&account, &empty, &object, &query));
    ASSERT_TRUE(uri.empty());

    Object obj(account, container, object);
    std::string path = std::move(obj.GetUri());
    uri = std::move(client.Url(path, &query));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account/container/object?format=json");

    uri = std::move(client.Url(empty, &query));
    ASSERT_TRUE(uri.empty());

    uri = std::move(client.Url(path));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account/container/object");

    query["limit"] = "1000";
    uri = std::move(client.Url(obj.GetUri(), &query));
    ASSERT_EQ(uri, "http://127.0.0.1:8080/v1/account/container/object?format=json&limit=1000");
}