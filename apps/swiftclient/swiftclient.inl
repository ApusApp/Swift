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

#ifndef __APPS_SWIFT_CLIENT_SWIFT_CLIENT_INL__
#define __APPS_SWIFT_CLIENT_SWIFT_CLIENT_INL__

#include <swift/base/stringutil.h>

// public
SwiftClient& SwiftClient::operator=(const SwiftClient& rhs)
{
    port_ = rhs.GetPort();
    host_ = rhs.GetHost();
    return *this;
}

// public
bool SwiftClient::operator== (const SwiftClient& rhs) const
{
    return host_ == rhs.GetHost() && port_ == rhs.GetPort();
}

// public
const std::string& SwiftClient::GetHost() const
{
    return host_;
}

// public
void SwiftClient::SetHost(const std::string& host)
{
    assert(!host.empty());
    host_ = host;
}

// public
void SwiftClient::SetHost(std::string&& host)
{
    assert(!host.empty());
    host_ = std::move(host);
}

// public
const int SwiftClient::GetPort() const
{
    return port_;
}

// public
void SwiftClient::SetPort(int port)
{
    assert(port_ > 0 || port_ < 65536);
    port_ = port;
}

// static public
SwiftClient::info_map_type SwiftClient::GetInfo(const std::string& url,
                                                const header_map_type* headers)
{
    return std::move(GetInfo(std::move(std::string(url)), headers));
}

// static public
SwiftClient::info_map_type SwiftClient::Download(const std::string& url,
                                                 const header_map_type* headers,
                                                 std::string& body)
{
    return std::move(Download(std::move(std::string(url)), headers, body));
}

// static public
const std::string& SwiftClient::GetSwiftApiVersion()
{
    return kApiVersion_;
}

// static public
void SwiftClient::AddToken(header_map_type& headers, const std::string& token)
{
    if (!token.empty()) {
        headers["X-Auth-Token"] = token;
    }
}

// static public
size_t SwiftClient::GetLastModifyTime(const info_map_type& info)
{
    if (!info.empty()) {
        auto it = info.find("X-Timestamp");
        if (it != info.end()) {
            swift::StringUtil::FromString<size_t>(it->second.c_str());
        }
    }

    return 0;
}

// static public
size_t SwiftClient::GetContentLength(const info_map_type& info)
{
    if (!info.empty()) {
        auto it = info.find("Content-Length");
        if (it != info.end()) {
            return swift::StringUtil::FromString<size_t>(it->second.c_str());
        }
    }

    return 0;
}

// public
SwiftClient::info_map_type SwiftClient::GetInfo(const Object *obj,
                                                const header_map_type* headers) const
{
    if (nullptr == obj || !obj->IsValid()) {
        return std::move(info_map_type());
    }

    return SwiftClient::GetInfo(std::move(Url(obj->GetUri(), nullptr)), headers);
}

// public
SwiftClient::info_map_type SwiftClient::Download(const Object* obj,
                                                 const header_map_type* headers,
                                                 std::string& body) const
{
    if (nullptr == obj || !obj->IsValid()) {
        return std::move(info_map_type());
    }

    return SwiftClient::Download(std::move(Url(obj->GetUri(), nullptr)), headers, body);
}


#endif // __APPS_SWIFT_CLIENT_SWIFT_CLIENT_INL__