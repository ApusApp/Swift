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

#ifndef __APPS_SWIFT_CLIENT_OBJECT_H__
#define __APPS_SWIFT_CLIENT_OBJECT_H__

#include <stdio.h>
#include <string>
#include <cassert>

class Object
{
public:
    Object() = default;

    Object(const std::string& account, const std::string& container, const std::string& name)
        : account_(account)
        , container_(container)
        , name_(name)
    {

    }


    Object(std::string&& account, std::string&& container, std::string&& name)
        : account_(std::move(account))
        , container_(std::move(container))
        , name_(std::move(name))
    {

    }

    Object(Object&&) = default;
    Object(const Object&) = default;
    ~Object() = default;

    Object& operator=(Object&&) = default;
    Object& operator=(const Object& rhs)
    {
        account_ = rhs.GetAccount();
        container_ = rhs.GetContainer();
        name_ = rhs.GetName();
        return *this;
    }

    inline bool operator== (const Object& rhs) const
    {
        if (name_ == rhs.GetName()
            && container_ == rhs.GetContainer()
            && account_ == rhs.GetAccount()) {
            return true;
        }

        return false;
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

    inline const std::string& GetContainer() const
    {
        return container_;
    }

    inline std::string&& GetContainer()
    {
        return std::move(container_);
    }

    inline void SetContainer(const std::string& container)
    {
        assert(!container.empty());
        container_ = container;
    }

    inline void SetContainer(std::string&& container)
    {
        assert(!container.empty());
        container_ = std::move(container);
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

    inline bool IsValid() const
    {
        return (!account_.empty() && !container_.empty() && !name_.empty());
    }

    inline std::string GetUri() const
    {
        if (!IsValid()) {
            return std::move(std::string());
        }

        char buf[1025] = {'\0'};
        int size = snprintf(buf, sizeof(buf), "%s/%s/%s", account_.c_str(),
                            container_.c_str(), name_.c_str());
        return std::move(std::string(buf, size));
    }


private:
    std::string account_;
    std::string container_;
    std::string name_;
}; // Object

#endif // __APPS_SWIFT_CLIENT_OBJECT_H__
