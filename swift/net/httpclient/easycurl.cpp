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
} // namespace swift
