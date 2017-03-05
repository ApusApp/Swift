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

#pragma once

#include <stdio.h>
#include <string>
#include <cassert>

class Container
{
public:
    Container() = default;

    ~Container() = default;

    Container(const std::string& account, const std::string& name)
        : account_(account)
        , name_(name)
    {

    }

    Container(std::string&& account, std::string&& name)
        : account_(std::move(account))
        , name_(std::move(name))
    {

    }

    Container(Container&&) = default;

    Container(const Container&) = default;

    Container& operator=(Container&&) = default;

    Container& operator=(const Container& rhs)
    {
        account_ = rhs.GetAccount();
        name_ = rhs.GetName();
        return *this;
    }

    inline bool operator== (const Container& rhs) const
    {
        return (name_ == rhs.GetName() && account_ == rhs.GetAccount());
    }

    inline const std::string& GetName() const
    {
        return name_;
    }

    inline std::string&& GetName()
    {
        return std::move(name_);
    }

    inline void SetName(const std::string& name)
    {
        assert(!name.empty());
        name_ = name;
    }

    inline void SetName(std::string&& name)
    {
        assert(!name.empty());
        name_ = std::move(name);
    }

    inline const std::string& GetAccount() const
    {
        return account_;
    }

    inline std::string&& GetAccount()
    {
        return std::move(account_);
    }

    inline void SetAccount(const std::string& account)
    {
        assert(!account.empty());
        account_ = account;
    }

    inline void SetAccount(std::string&& account)
    {
        assert(!account.empty());
        account_ = std::move(account);
    }

    inline std::string GetUri() const
    {
        if (!IsValid()) {
            return std::move(std::string());
        }

        char buf[512] = {'\0'};
        int size = snprintf(buf, sizeof(buf), "%s/%s", account_.c_str(), name_.c_str());
        return std::move(std::string(buf, size));
    }

    inline bool IsValid() const
    {
        return (!account_.empty() && !name_.empty());
    }

private:
    std::string account_;
    std::string name_;
};
