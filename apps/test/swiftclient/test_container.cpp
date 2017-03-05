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

#include <gtest/gtest.h>
#include <swiftclient/container.h>

class test_Container : public testing::Test
{
public:
    test_Container() {}
    ~test_Container() {}

    virtual void SetUp (void)
    {
    }

    virtual void TearDown (void)
    {

    }
};

TEST_F(test_Container, Container)
{
    Container c(std::move(std::string("account")),
                std::move(std::string("container")));
    ASSERT_EQ(c.GetAccount(), "account");
    ASSERT_EQ(c.GetName(), "container");

    Container c2(c);
    ASSERT_EQ(c2.GetAccount(), c.GetAccount());
    ASSERT_EQ(c.GetName(), c2.GetName());
    ASSERT_EQ(c, c2);
    std::string account = std::move(c2.GetAccount());
    ASSERT_EQ(account, c.GetAccount());
    ASSERT_TRUE(c2.GetAccount().empty());

    c2.SetAccount(account);
    ASSERT_EQ(account, c2.GetAccount());
    Container c3(std::move(c2));
    ASSERT_EQ(c3.GetAccount(), c.GetAccount());
}