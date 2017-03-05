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

#include <cstring>
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
        curl_easy_setopt(curl_, CURLOPT_PORT, port);
    }

    // public
    void EasyCurl::SetUrl(const char* url)
    {
        assert(0 != url);
        curl_easy_setopt(curl_, CURLOPT_URL, url);
    }

    // private
    void EasyCurl::Destroy()
    {
        if (header_) {
            curl_slist_free_all(header_);
            header_ = 0;
        }

        if (middleware_) {
            delete middleware_;
            middleware_ = 0;
        }
    }

    // private
    void EasyCurl::Init()
    {
        assert(0 != curl_);
        curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
        // set dns cache never time out
        curl_easy_setopt(curl_, CURLOPT_DNS_CACHE_TIMEOUT, 5L);
        // set tcp no delay
        curl_easy_setopt(curl_, CURLOPT_TCP_NODELAY, 0);
        // set receive time out
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
        // set connect time out
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 1L);
        // set http version HTTP/1.1
        curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, EasyCurl::EmptyHandler);
        curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, EasyCurl::EmptyHandler);
    }

    // public
    void EasyCurl::SetMethod(const HttpMethod& method)
    {
        assert(0 != curl_);
        switch(method) {
            case HTTP_METHOD_GET:
                curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
                break;
            case HTTP_METHOD_HEAD:
                curl_easy_setopt(curl_, CURLOPT_NOBODY, 1L);
                break;
            case HTTP_METHOD_PUT:
                curl_easy_setopt(curl_, CURLOPT_PUT, 1L);
                header_ = curl_slist_append(header_, "Expect: ");
                break;
            case HTTP_METHOD_DELETE:
                curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
            case HTTP_METHOD_POST:
                curl_easy_setopt(curl_, CURLOPT_POST, 1L);
                header_ = curl_slist_append(header_, "Expect: ");
                break;
            case HTTP_METHOD_COPY:
                curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "COPY");
                break;
            default:
                break;
        }
    }

    // public
    void EasyCurl::SetHeader(const Request *req)
    {
        assert(0 != curl_);
        if (nullptr != req) {
            for (auto &it : req->GetHeaders()) {
                std::string str = std::move(std::string(it.first + ":" + it.second));
                header_ = curl_slist_append(header_, str.c_str());
            }

            if (header_) {
                curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_);
            }
        }
    }

    // public
    void EasyCurl::SetReadTimeout(size_t timeout)
    {
        if (timeout > 0) {
            curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout);
        }
    }

    // public
    void EasyCurl::SetConnectTimeout(size_t timeout)
    {
        if (timeout > 0) {
            curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout);
        }
    }

    // public
    void EasyCurl::SetReceiveHandler(const ReceiveHandler* handler, Response* resp, DownloadBuffer* buf)
    {
        assert(0 != curl_);
        if (handler && resp) {
            middleware_ = new Middleware(resp, handler, buf);
            if (handler->body_handler_ && handler->header_handler_) {
                curl_easy_setopt(curl_, CURLOPT_WRITEDATA, middleware_);
                curl_easy_setopt(curl_, CURLOPT_HEADERDATA, middleware_);
                curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, EasyCurl::BodyHandler);
                curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, EasyCurl::HeaderHandler);
            }
            else if (handler->body_handler_ && !handler->header_handler_) {
                curl_easy_setopt(curl_, CURLOPT_WRITEDATA, middleware_);
                curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, EasyCurl::BodyHandler);
            }
            else if (!handler->body_handler_ && handler->header_handler_) {
                curl_easy_setopt(curl_, CURLOPT_HEADERDATA, middleware_);
                curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, EasyCurl::HeaderHandler);
            }
        }
    }

    // public
    int EasyCurl::SendRequest(const Request* req, const HttpMethod& method)
    {
        assert(0 != req);
        SetMethod(method);
        SetUrl(req->GetUrl());
        SetHeader(req);
        SetConnectTimeout(req->GetConnectTimeout());
        SetReadTimeout(req->GetReadTimeout());
        CURLcode ret = curl_easy_perform(curl_);
        int code = static_cast<int>(ret);
        if (CURLE_OK == ret) {
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &code);
            if (middleware_) {
                middleware_->SetResponseStatusCode(code);
            }
        }

        return code;
    }

    // public
    void EasyCurl::SetUploadBuf(UploadBuffer* buffer, const HttpMethod& method)
    {
        assert (0 != curl_);
        switch (method) {
            case HTTP_METHOD_PUT:
                curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
                if (buffer) {
                    // set read callback function
                    if (buffer->buf_) {
                        curl_easy_setopt(curl_, CURLOPT_READFUNCTION, EasyCurl::UploadHandler);
                        curl_easy_setopt(curl_, CURLOPT_INFILESIZE, static_cast<curl_off_t>(buffer->size_));
                    } else if (buffer->file_) {
                        curl_easy_setopt(curl_, CURLOPT_READFUNCTION, EasyCurl::UploadFileHandler);
                        // Content-Length
                        curl_easy_setopt(curl_, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(buffer->size_));
                    } else {
                        // TO DO
                        // throw exception ??
                        (void)buffer;
                    }
                    // set data object to pass to callback function
                    curl_easy_setopt(curl_, CURLOPT_READDATA, buffer);

                }
                else {
                    curl_easy_setopt(curl_, CURLOPT_INFILESIZE, 0L);
                }
                break;
            case HTTP_METHOD_POST:
                if (buffer && buffer->buf_ && buffer->size_ > 0) {
                    assert(0 != buffer->size_);
                    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, buffer->buf_);
                    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(buffer->size_));
                }
                else {
                    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, 0L);
                }
                break;
            default:
                break;
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
