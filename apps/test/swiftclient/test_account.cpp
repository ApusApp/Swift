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
#include <swiftclient/account.h>

class test_Account : public testing::Test
{
public:
    test_Account() {}
    ~test_Account() {}

    virtual void SetUp (void)
    {
    }

    virtual void TearDown (void)
    {

    }
};

TEST(test_Account, Account)
{
    std::string name("account");
    Account account(name);
    ASSERT_EQ(name, account.GetName());

    Account account1(account);
    ASSERT_EQ(account1.GetName(), account.GetName());
    ASSERT_EQ(account, account1);

    Account account2(std::move(account1));
    ASSERT_EQ(account2.GetName(), account.GetName());
    ASSERT_EQ(account, account2);
    ASSERT_TRUE(account1.GetName().empty());

    Account account3(std::move(account2));
    ASSERT_EQ(account3.GetName(), account.GetName());
    ASSERT_EQ(account1, account2);
    ASSERT_TRUE(account2.GetName().empty());

    account1 = std::move(account3);
    ASSERT_EQ(account1.GetName(), account.GetName());
    ASSERT_EQ(account3, account2);
    ASSERT_TRUE(!account1.GetName().empty());

    account2.SetName(std::move(account1.GetName()));
    ASSERT_EQ(account2.GetName(), account.GetName());
    ASSERT_EQ(account3, account1);
    ASSERT_TRUE(!account2.GetName().empty());

}

