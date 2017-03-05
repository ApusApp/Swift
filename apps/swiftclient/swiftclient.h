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

#ifndef __APPS_SWIFT_CLIENT_SWIFT_CLIENT_H__
#define __APPS_SWIFT_CLIENT_SWIFT_CLIENT_H__

#include <memory>
#include <map>

#include "swiftclient/object.h"
#include "swiftclient/container.h"
#include "swiftclient/account.h"

class SwiftClient
{
public:
    typedef std::map<std::string, std::string> header_map_type;
    typedef header_map_type info_map_type;
    typedef header_map_type query_map_type;

public:
    SwiftClient() = default;
    SwiftClient(const std::string& host, const int port)
        : port_(port)
        , host_(host)
    {
        assert(!host_.empty());
        assert(port_ > 0 || port_ < 65536);
    }

    SwiftClient(std::string&& host, int port)
            : port_(port)
            , host_(std::move(host))
    {
        assert(!host_.empty());
        assert(port_ > 0 || port_ < 65536);
    }

    SwiftClient(SwiftClient&&) = default;
    SwiftClient(const SwiftClient&) = default;
    SwiftClient& operator=(SwiftClient&&) = default;
    inline SwiftClient& operator=(const SwiftClient& rhs);
    inline bool operator== (const SwiftClient& rhs) const;
    inline const std::string& GetHost() const;
    inline void SetHost(const std::string& host);
    inline void SetHost(std::string&& host);
    inline const int GetPort() const;
    inline void SetPort(int port);


    inline info_map_type GetInfo(const Object* obj, const header_map_type* headers) const;
    inline info_map_type Download(const Object* obj, const header_map_type* headers, std::string& body) const;

    std::string Url(const std::string* account,
                    const std::string* container = nullptr,
                    const std::string* object = nullptr,
                    const query_map_type* query = nullptr) const;

    std::string Url(const std::string& path, const query_map_type* query = nullptr) const;

public:
    static info_map_type GetInfo(std::string&& url, const header_map_type* headers);
    inline static info_map_type GetInfo(const std::string& url, const header_map_type* headers);

    static info_map_type Download(std::string&& url, const header_map_type* headers, std::string& body);
    inline static info_map_type Download(const std::string& url, const header_map_type* headers, std::string& body);

    // Like pread
    static info_map_type Download(const std::string& url,
                                  const header_map_type* headers,
                                  char* buf, size_t size, size_t offset = 0);

    static int Download(const std::string& url, const header_map_type& headers, const std::string& file);
    static int Upload(const std::string& url, const header_map_type& headers, const std::string& file);

    inline static const std::string& GetSwiftApiVersion();
    inline static void AddToken(header_map_type& headers, const std::string& token);
    inline static size_t GetLastModifyTime(const info_map_type& info);
    inline static size_t GetContentLength(const info_map_type& info);

private:
    int port_;
    std::string host_;

    static const std::string kApiVersion_;
};

#include "swiftclient/swiftclient.inl"

#endif // __APPS_SWIFT_CLIENT_SWIFT_CLIENT_H__