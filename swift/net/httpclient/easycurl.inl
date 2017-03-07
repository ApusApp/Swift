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

#ifndef __SWIFT_NET_HTTP_CLIENT_EASY_CURL_INL__
#define __SWIFT_NET_HTTP_CLIENT_EASY_CURL_INL__

#include <cassert>

#include "swift/net/httpclient/request.hpp"
#include "swift/net/httpclient/response.hpp"

namespace swift {

    // public
    void EasyCurl::Reset()
    {
        if (curl_) {
            Destroy();
            curl_easy_reset(curl_);
            Init();
        }
    }

    // public
    void EasyCurl::SetPort(const long port)
    {
        assert(0 != curl_);
        curl_easy_setopt(curl_, CURLOPT_PORT, port);
    }

    // public
    void EasyCurl::SetUrl(const char* url)
    {
        assert(0 != url);
        assert(0 != curl_);
        curl_easy_setopt(curl_, CURLOPT_URL, url);
    }

    // public
    void EasyCurl::SetReadTimeout(size_t timeout)
    {
        assert(0 != curl_);
        if (timeout > 0) {
            curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout);
        }
    }

    // public
    void EasyCurl::SetConnectTimeout(size_t timeout)
    {
        assert(0 != curl_);
        if (timeout > 0) {
            curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout);
        }
    }

    // static private
    size_t EasyCurl::BodyHandler(void *data, size_t size, size_t nmemb, void *user_data)
    {
        if (user_data && data && size * nmemb > 0) {
            return (reinterpret_cast<Middleware*>(user_data))->Write(Middleware::OPERATE_TYPE_BODY,
                                                                     reinterpret_cast<const char*>(data),
                                                                     size * nmemb);
        }

        return size * nmemb;
    }

    // static private
    size_t EasyCurl::HeaderHandler(void *data, size_t size, size_t nmemb, void *user_data)
    {
        if (user_data && data && size * nmemb > 0) {
            return (reinterpret_cast<Middleware*>(user_data))->Write(Middleware::OPERATE_TYPE_HEADER,
                                                                     reinterpret_cast<const char*>(data),
                                                                     size * nmemb);
        }

        return size * nmemb;
    }


    // static private
    size_t EasyCurl::EmptyHandler(void *data, size_t size, size_t nmemb, void *user_data)
    {
        return size * nmemb;
    }

    // public
    void EasyCurl::Middleware::SetResponseStatusCode(int code)
    {
        if (response_) {
            response_->SetStatusCode(code);
        }
    }

    // static public
    void EasyCurl::GlobalInit()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    // static public
    void EasyCurl::GlobalCleanUp()
    {
        curl_global_cleanup();
    }

} // namespace swift

#endif // __SWIFT_NET_HTTP_CLIENT_EASY_CURL_INL__
