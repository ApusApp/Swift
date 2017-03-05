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


#ifndef __SWIFT_NET_HTTP_CLIENT_REQUEST_HPP__
#define __SWIFT_NET_HTTP_CLIENT_REQUEST_HPP__

#include <map>
#include <string>
#include <cassert>

#include "swift/base/noncopyable.hpp"

namespace swift {

class Request : swift::noncopyable {

public:
    Request() : size_(0), read_timeout_(30), connect_timeout_(3)
        , data_(nullptr), url_(), headers_()
    {
        headers_.insert(std::move(
                std::make_pair(std::move(std::string("User-Agent")),
                               std::move(std::string("SwiftCli/1.0")))));
    }

    ~Request()
    {
        if (nullptr != data_) {
            data_ = nullptr;
        }

        size_ = 0;
    }

    inline void SetReadTimeout(int timeout /* seconds */)
    {
        assert(timeout >= 0);
        read_timeout_ = timeout;
    }

    inline int GetReadTimeout() const
    {
        return read_timeout_;
    }

    inline void SetConnectTimeout(int timeout /* seconds */)
    {
        assert(timeout >= 0);
        connect_timeout_ = timeout;
    }

    inline int GetConnectTimeout() const
    {
        return connect_timeout_;
    }

    inline size_t GetSize() const
    {
        return size_;
    }

    inline const char* GetData() const
    {
        return data_;
    }

    inline void SetData(const char* data, size_t size)
    {
        assert(nullptr != data);
        assert(size > 0);
        data_ = data;
        size_ = size;
    }

    inline void SetUrl(const std::string& url)
    {
        assert(!url.empty());
        url_ = url;
    }

    inline void SetUrl(std::string&& url)
    {
        assert(!url.empty());
        url_ = std::move(url);
    }

    const char* GetUrl() const
    {
        return url_.data();
    }

    inline const std::map<std::string, std::string>& GetHeaders() const
    {
        return headers_;
    }

    inline void AddHeader(const std::string& name, const std::string& value)
    {
        assert(!name.empty() || !value.empty());
        std::map<std::string, std::string>::iterator it = headers_.find(name);
        if (it != headers_.end()) {
            it->second = value;
        }
        else {
            headers_.insert(std::move(std::make_pair(name, value)));
        }
    }

    inline void AddHeader(std::string&& name, std::string&& value)
    {
        assert(!name.empty() || !value.empty());
        std::map<std::string, std::string>::iterator it = headers_.find(name);
        if (it != headers_.end()) {
            it->second = std::move(value);
        }
        else {
            headers_.insert(std::move(std::make_pair(std::move(name), std::move(value))));
        }
    }

    void AddHeader(const std::map<std::string, std::string>& headers)
    {
        for (auto &it : headers) {
            AddHeader(it.first, it.second);
        }
    }

private:
    size_t size_;
    int read_timeout_;
    int connect_timeout_;
    const char* data_;
    std::string url_;
    std::map<std::string, std::string> headers_;
};
} // namespace swift
#endif //__SWIFT_NET_HTTP_CLIENT_REQUEST_HPP__
