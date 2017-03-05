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

#ifndef __SWIFT_NET_HTTP_CLIENT_EASY_CURL_H__
#define __SWIFT_NET_HTTP_CLIENT_EASY_CURL_H__

#include <stdint.h>
#include <curl/curl.h>

#include "swift/base/noncopyable.hpp"


namespace swift {

class File;
class Request;
class Response;

struct UploadBuffer : swift::noncopyable
{
    UploadBuffer(const char* buf, size_t size, size_t start_pos=0)
        : buf_(buf), file_(nullptr), size_(size), start_pos_(start_pos)
    {
    }

    UploadBuffer(const File* file, size_t size, size_t start_pos=0)
        : buf_(nullptr), file_(file), size_(size), start_pos_(start_pos)
    {
    }

    const char* buf_;
    const File* file_;
    size_t size_;
    size_t start_pos_;
};

struct DownloadBuffer : swift::noncopyable
{
    DownloadBuffer(char* buf, size_t size, size_t start_pos=0)
        : buf_(buf), file_(nullptr), size_(size), start_pos_(start_pos)
    {
    }

    DownloadBuffer(const File* file, size_t size, size_t start_pos=0)
        : buf_(nullptr), file_(file), size_(size), start_pos_(start_pos)
    {
    }

    char* buf_;
    const File* file_;
    size_t size_;
    size_t start_pos_;
};


struct ReceiveHandler : swift::noncopyable
{
    typedef void (*ReceiveHandlerType)(Response* resp, const char* data, const size_t size);

    ReceiveHandler (const ReceiveHandlerType body_handler, const ReceiveHandlerType header_handler)
        : body_handler_(body_handler)
        , header_handler_(header_handler)
    {
    }

    const ReceiveHandlerType body_handler_;
    const ReceiveHandlerType header_handler_;
};

enum HttpMethod
{
    HTTP_METHOD_INVALID,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_POST,
    HTTP_METHOD_COPY,
};

class EasyCurl : swift::noncopyable {
    class Middleware
    {
    public:
        enum OperateType
        {
            OPERATE_TYPE_HEADER,
            OPERATE_TYPE_BODY,
            OPERATE_TYPE_NULL,
        };

        Middleware(Response* resp, const ReceiveHandler* handler)
            : buffer_(nullptr)
            , response_(resp)
            , handler_(handler)
        {
        }

        Middleware(Response* resp, const ReceiveHandler* handler, DownloadBuffer* buf)
            : buffer_(buf)
            , response_(resp)
            , handler_(handler)
        {
        }

        ~Middleware()
        {
            if (response_) {
                response_ = nullptr;
            }

            if (handler_) {
                handler_ = nullptr;
            }

            if (buffer_) {
                buffer_ = nullptr;
            }
        }

        inline void SetResponseStatusCode(int code);
        size_t Write(const OperateType& type, const char* data, const size_t size);

    private:
        size_t Write(const char* data, const size_t size);

    private:
        DownloadBuffer* buffer_;
        Response* response_;
        const ReceiveHandler* handler_;
    };

public:
    EasyCurl();
    ~EasyCurl();

    inline void Reset();
    inline void SetUrl(const char* url);
    inline void SetPort(const long port);
    inline void SetHeader(const Request* req);
    inline void SetReadTimeout(size_t timeout);
    inline void SetConnectTimeout(size_t timeout);
    inline void SetMethod(const HttpMethod& method);
    inline void SetReceiveHandler(const ReceiveHandler* handler, Response* resp, DownloadBuffer* buf);
    inline void SetUploadBuf(UploadBuffer* buf, const HttpMethod& method);
    inline int SendRequest(const Request* req, const HttpMethod& method);

    inline static void GlobalInit();
    inline static void GlobalCleanUp();

private:
    inline void Init();
    inline void Destroy();
    inline static size_t BodyHandler(void *data, size_t size, size_t nmemb, void *user_data);
    inline static size_t HeaderHandler(void *data, size_t size, size_t nmemb, void *user_data);
    inline static size_t EmptyHandler(void *data, size_t size, size_t nmemb, void *user_data);
    static size_t UploadHandler(void *data, size_t size, size_t nmemb, void *user_data);
    static size_t UploadFileHandler(void *data, size_t size, size_t nmemb, void *user_data);

private:
    CURL* curl_;
    curl_slist* header_;
    Middleware* middleware_;
};

} // namespace swift

#include "swift/net/httpclient/easycurl.inl"

#endif //__SWIFT_NET_HTTP_CLIENT_EASY_CURL_H__
