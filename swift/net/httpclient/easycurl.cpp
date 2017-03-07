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

#include <stdio.h>
#include <cassert>
#include <string.h>

#include "swift/base/file.h"
#include "swift/net/httpclient/easycurl.h"

namespace swift {

    EasyCurl::EasyCurl() : curl_(0), header_(0), middleware_(0) {
        curl_ = curl_easy_init();
        assert(0 != curl_);
        Init();
    }

    EasyCurl::~EasyCurl() {
        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = 0;
        }

        Destroy();
    }

    // static public
    size_t EasyCurl::UploadFileHandler(void *data, size_t size, size_t nmemb, void *user_data)
    {
        size_t n = size * nmemb;
        if (n < 1) {
            return 0;
        }

        size_t total_read = 0;
        if (user_data && data) {
            UploadBuffer* buffer = reinterpret_cast<UploadBuffer*>(user_data);
            if (nullptr == buffer->file_ || buffer->size_ <= 0 || buffer->file_->GetFd() < 0) {
                return 0;
            }

            size_t read_bytes = 0;
            size_t copy_size = buffer->size_ > n ? n : buffer->size_;
            for (; total_read < copy_size; total_read += read_bytes) {
                read_bytes = buffer->file_->PRead(&(reinterpret_cast<char*>(data))[total_read],
                                                  copy_size - total_read,
                                                  buffer->start_pos_ + total_read);
                if (0 == read_bytes) {
                    // eof
                    break;
                } else if (-1 == static_cast<int>(read_bytes)) {
                    // error
                    return 0;
                }
            }

            buffer->start_pos_ += total_read;
            buffer->size_ -= total_read;
        }

        return total_read;
    }

    // private
    size_t EasyCurl::Middleware::Write(const char* data, const size_t size)
    {
        // write to file
        if (buffer_ && buffer_->file_ && buffer_->file_->GetFd() > 0 && size > 0) {
            size_t write_bytes = buffer_->file_->PWrite(data, size, buffer_->start_pos_);
            if (-1 == static_cast<int>(write_bytes)) {
                return 0;
            }
            buffer_->start_pos_ += size;
            return write_bytes;
        }

        // write to memory first
        if (buffer_ && buffer_->buf_ && !buffer_->file_ && buffer_->size_ > 0 && size > 0) {
            size_t len = buffer_->size_ > size ? size : buffer_->size_;
            char *buf = buffer_->buf_ + buffer_->start_pos_;
            void *dst = reinterpret_cast<void*>(buf);
            memcpy(dst, reinterpret_cast<const void*>(data), len);
            buffer_->start_pos_ += len;
            buffer_->size_ -= len;
            return len;
        }

        return 0;
    }

    // public
    size_t EasyCurl::Middleware::Write(const Middleware::OperateType& type,
                                       const char* data,
                                       const size_t size)
    {
        switch (type) {
            case OPERATE_TYPE_BODY:
                if (handler_ && handler_->body_handler_ && response_) {
                    if (buffer_ && response_) {
                        return Write(data, size);
                    }
                    else {
                        (*(handler_->body_handler_))(response_, data, size);
                    }
                }
                break;
            case OPERATE_TYPE_HEADER:
                if (handler_ && handler_->header_handler_ && response_) {
                    (*(handler_->header_handler_))(response_, data, size);
                }
                break;
            default:
                break;
        }

        return size;
    }

    // static private
    size_t EasyCurl::UploadHandler(void *data, size_t size, size_t nmemb, void *user_data)
    {
        if (size * nmemb < 1) {
            return 0;
        }

        if (user_data && data) {
            UploadBuffer* buffer = reinterpret_cast<UploadBuffer*>(user_data);
            if (nullptr == buffer->buf_ || buffer->size_ <= 0) {
                return 0;
            }

            size_t n = size * nmemb;
            size_t m = buffer->size_ > n ? n : buffer->size_;
            // copy data to send buffer
            if (buffer->buf_) {
                // memcpy(data, (const void *)buffer->buf_, m);
                const char *buf = buffer->buf_ + buffer->start_pos_;
                const void *src = reinterpret_cast<const void*>(buf);
                memcpy(data, src, m);
                // decrement length and increment data pointer
                buffer->size_ -= m;
                // buffer->buf_ += m;
                buffer->start_pos_ += m;
                // return copied size
                return m;
            }
        }

        return 0;
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

    // public
    int EasyCurl::SendRequest(const Request* req, const HttpMethod& method)
    {
        assert(0 != req);
        SetMethod(method);
        SetUrl(req->GetUrl());
        SetHeader(req);
        SetConnectTimeout(static_cast<size_t>(req->GetConnectTimeout()));
        SetReadTimeout(static_cast<size_t>(req->GetReadTimeout()));
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
    void EasyCurl::SetReceiveHandler(const ReceiveHandler* handler, Response* resp, DownloadBuffer* buf /*=nullptr*/)
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

} // namespace swift
