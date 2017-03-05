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

#include <swift/base/file.h>
#include <swift/base/singleton.hpp>
#include <swift/base/experimental/logging.h>
#include <swift/net/httpclient/httpclient.h>

#include "swiftclient.h"


const std::string SwiftClient::kApiVersion_("v1");

// static public
int SwiftClient::Download(const std::string& url, const header_map_type& headers, const std::string& file)
{
    swift::Request req;
    req.SetUrl(url);
    req.AddHeader(headers);

    swift::File f;
    f.Open(file.data());
    swift::Response resp;
    return swift::Singleton<swift::HttpClient>::Instance().Get(&req, &resp, &f);
}

SwiftClient::info_map_type SwiftClient::Download(const std::string& url,
                                                 const header_map_type* headers,
                                                 char* buf,
                                                 size_t size,
                                                 size_t offset)
{
    char tmp[32] = {'\0'};
    swift::Request req;
    swift::Response resp;
    req.SetUrl(url);
    if (nullptr != headers) {
        req.AddHeader(*headers);
    }

    int len = snprintf(tmp, sizeof(tmp), "bytes=%ld-%ld", offset, offset+size-1);
    req.AddHeader(std::move(std::string("Range")),
                  std::move(std::string(tmp, static_cast<size_t>(len))));

    int status = swift::Singleton<swift::HttpClient>::Instance().Get(&req, &resp, buf, size);
    if (status == swift::HttpCode::HTTP_PARTIAL_CONTENT) {
        return std::move(resp.GetHeaders());
    }

    LOG_ERROR << "GET " << url << " Return status=" << status;

    return std::move(info_map_type());
}

// static public
int SwiftClient::Upload(const std::string& url, const header_map_type& headers, const std::string& file)
{
    swift::Request req;
    req.SetUrl(url);
    req.AddHeader(headers);

    swift::File f;
    f.Open(file.c_str(), O_RDONLY);
    swift::Response resp;
    return swift::Singleton<swift::HttpClient>::Instance().Put(&req, &resp, &f);
}

// static public
SwiftClient::info_map_type SwiftClient::GetInfo(std::string&& url,
                                                const header_map_type* headers)
{
    if (!url.empty()) {
        swift::Request req;
        swift::Response resp;
        req.SetUrl(std::move(url));
        if (headers) {
            req.AddHeader(*headers);
        }

        int status = swift::Singleton<swift::HttpClient>::Instance().Head(&req, &resp);
        if (status == swift::HttpCode::HTTP_OK) {
            return std::move(resp.GetHeaders());
        }

        LOG_ERROR << "HEAD " << url << " Return status=" << status;
    }

    return std::move(SwiftClient::info_map_type());
}

// static public
SwiftClient::info_map_type SwiftClient::Download(std::string&& url,
                                                 const SwiftClient::header_map_type* headers,
                                                 std::string& body)
{
    if (!url.empty()) {
        swift::Request req;
        req.SetUrl(std::move(url));
        if (headers) {
            req.AddHeader(*headers);
        }

        swift::Response resp;
        int status = swift::Singleton<swift::HttpClient>::Instance().Head(&req, &resp);
        if (status == swift::HttpCode::HTTP_OK) {
            body = std::move(resp.GetBody());
            return std::move(resp.GetHeaders());
        }
    }

    return std::move(SwiftClient::info_map_type());
}

void UrlAddQueryString(std::string& url, const SwiftClient::query_map_type* query)
{
    if (!url.empty() && query && !query->empty()) {
        char buf[1100] = {'\0'};
        url.append("?");
        size_t length = query->size();
        for (auto &it : *query) {
            int size = 0;
            length -= 1;
            if (0 == length) {
                size = snprintf(buf, sizeof(buf), "%s=%s", it.first.c_str(), it.second.c_str());
            } else {
                size = snprintf(buf, sizeof(buf), "%s=%s&", it.first.c_str(), it.second.c_str());
            }
            url.append(buf, static_cast<size_t>(size));
        }
    }
}

// private
std::string SwiftClient::Url(const std::string* account,
                             const std::string* container,
                             const std::string* object,
                             const query_map_type* query) const
{
    std::string uri;
    char buf[1025] = {'\0'};

    // account container object
    if ((account && !account->empty())
        && (container && !container->empty())
        && (object && !object->empty())) {
        int size = snprintf(buf, sizeof(buf), "http://%s:%d/%s/%s/%s/%s",
                            host_.c_str(), port_, kApiVersion_.c_str(),
                            account->c_str(), container->c_str(), object->c_str());
        uri = std::move(std::string(buf, static_cast<size_t>(size)));
    }

    // account container null
    if (uri.empty()
        && (account && !account->empty())
        && (container && !container->empty())
        && !object) {
        int size = snprintf(buf, sizeof(buf), "http://%s:%d/%s/%s/%s",
                            host_.c_str(), port_, kApiVersion_.c_str(),
                            account->c_str(), container->c_str());
        uri = std::move(std::string(buf, static_cast<size_t>(size)));
    }

    // account null null
    if (uri.empty()
        && (account && !account->empty())
        && !container && !object) {
        int size = snprintf(buf, sizeof(buf), "http://%s:%d/%s/%s",
                            host_.c_str(), port_, kApiVersion_.c_str(),
                            account->c_str());
        uri = std::move(std::string(buf, static_cast<size_t>(size)));
    }

    UrlAddQueryString(uri, query);

    return std::move(uri);
}

// public
std::string SwiftClient::Url(const std::string& path, const query_map_type* query) const
{
    if (path.empty()) {
        return std::move(std::string());
    }

    char buf[1025] = {'\0'};
    int size = snprintf(buf, sizeof(buf), "http://%s:%d/%s/%s", host_.c_str(),
                        port_,
                        kApiVersion_.c_str(),
                        path.c_str());
    std::string uri(buf, static_cast<size_t>(size));

    UrlAddQueryString(uri, query);

    return std::move(uri);
}
