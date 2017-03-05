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

#ifndef __SWIFT_NET_HTTP_CLIENT_RESPONSE_HPP__
#define __SWIFT_NET_HTTP_CLIENT_RESPONSE_HPP__

#include <map>
#include <string>

#include "swift/base/noncopyable.hpp"
#include "swift/base/stringutil.h"

namespace swift {

class Response : swift::noncopyable {
public:
    Response() : status_code_(0), body_(), headers_()
    {
    }

    ~Response()
    {
        Reset();
    }

    Response(Response&&) = default;
    Response(const Response&) = default;
    Response& operator=(const Response&) = default;
    Response& operator=(Response&&) = default;

    inline int GetStatusCode() const
    {
        return status_code_;
    }

    inline void SetStatusCode(int status_code)
    {
        status_code_ = status_code;
    }

    inline const std::map<std::string, std::string>& GetHeaders() const
    {
        return headers_;
    }

    inline std::map<std::string, std::string>&& GetHeaders()
    {
        return std::move(headers_);
    }

    inline void AddHeader(const std::string& name, const std::string& value)
    {
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

    inline const std::string& GetBody() const
    {
        return body_;
    }

    inline std::string&& GetBody()
    {
        return std::move(body_);
    }

    inline void SetBody(const char* data, const size_t size)
    {
        body_.append(data, size);
    }

    inline void Reset()
    {
        status_code_ = 0;
        headers_.clear();
        body_.clear();
    }

    inline size_t ContentLength() const
    {
        if (!headers_.empty()) {
            auto it = headers_.find("Content-Length");
            if (it != headers_.end()) {
                return swift::StringUtil::FromString<size_t>(it->second.c_str());
            }
        }

        return 0;
    }

private:
    int status_code_;
    std::string body_;
    std::map<std::string, std::string> headers_;

}; // Response
} // namespace swift

#endif //__SWIFT_NET_HTTP_CLIENT_RESPONSE_HPP__
