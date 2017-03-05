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

#include <string>
#include <cassert>

class Account
{
public:
    Account() = default;

    ~Account() = default;

    Account(const std::string& name) : name_(name)
    {

    }

    Account(std::string&& name) : name_(std::move(name))
    {

    }

    Account(const Account&) = default;

    Account(Account&&) = default;

    Account& operator=(const Account&) = default;

    Account& operator=(Account&&) = default;

    inline bool operator==(const Account& rhs) const
    {
        return name_ == rhs.GetName();
    }

    inline std::string&& GetName()
    {
        return std::move(name_);
    }

    inline const std::string& GetName() const
    {
        return name_;
    }

    inline void SetName(std::string&& name)
    {
        assert(!name.empty());
        name_ = std::move(name);
    }

    inline void SetName(const std::string& name)
    {
        assert(!name.empty());
        name_ = name;
    }

    inline const std::string& GetUri() const
    {
        return name_;
    }

    inline bool IsValid() const
    {
        return !name_.empty();
    }

private:
    std::string name_;
};
