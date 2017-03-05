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

#include "swift/base/file.h"
#include "swift/net/httpclient/httpclient.h"
#include "swift/net/httpclient/easycurlpool.h"

namespace swift {

    #ifndef ScopeHolder
    typedef typename EasyCurlPool::EasyCurlHolder ScopeHolder;
    #endif

    const ReceiveHandler HttpClient::kBodyAndHeaderHandler(&HttpClient::BodyHandler,
                                                           &HttpClient::HeaderHandler);
    const ReceiveHandler HttpClient::kHeaderHandler(static_cast<ReceiveHandler::ReceiveHandlerType>(0x0),
                                                    &HttpClient::HeaderHandler);

    HttpClient::HttpClient()
    {

    }

    HttpClient::~HttpClient()
    {

    }

    // static private
    void HttpClient::BodyHandler(Response *resp, const char *data, const size_t size)
    {
        if (resp && data) {
            resp->SetBody(data, size);
        }
    }

    // static private
    void HttpClient::HeaderHandler(Response *resp, const char *data, const size_t size)
    {
        if (resp && 0 != size && data) {
            size_t i = 0;
            const char *str = data;
            const char *tmp = str;
            while (('\r' != *tmp) && (':' != *tmp++)) {
                ++i;
            }

            if (i != size - 2) {
                resp->AddHeader(std::move(std::string(str, i)),
                                std::move(std::string(tmp + 1, size-i-4)));
            }
        }
    }

    // public
    int HttpClient::Get(const Request* req, Response* resp,
                        const File* file,
                        size_t size /*size=std::numeric_limits<size_t>::max()*/,
                        size_t offset /*offset=0*/) const
    {
        if (nullptr == file) {
            return 0;
        }

        ScopeHolder holder(EasyCurlPool::Instance().Get());
        EasyCurl &curl = holder.GetEasyCurl();
        if (resp) {
            DownloadBuffer buffer(file, size, offset);
            curl.SetReceiveHandler(&kBodyAndHeaderHandler, resp, &buffer);
        }

        return curl.SendRequest(req, HTTP_METHOD_GET);
    }

    // public
    int HttpClient::Get(const Request* req, Response* resp, char* buf, size_t size) const
    {
        if (nullptr == req || nullptr == resp || nullptr == buf || size <= 0) {
            return 0;
        }

        ScopeHolder holder(EasyCurlPool::Instance().Get());
        EasyCurl &curl = holder.GetEasyCurl();

        DownloadBuffer buffer(buf, size, 0);
        curl.SetReceiveHandler(&kBodyAndHeaderHandler, resp, &buffer);


        return curl.SendRequest(req, HTTP_METHOD_GET);
    }

    // public
    int HttpClient::Put(const Request* req, Response* resp,
                        const File* file,
                        size_t size /*size=std::numeric_limits<size_t>::max()*/,
                        size_t offset /*offset=0*/) const
    {
        if (nullptr == file || file->GetFd() < 0) {
            return -1;
        }

        ScopeHolder holder(EasyCurlPool::Instance().Get());
        EasyCurl &curl = holder.GetEasyCurl();
        if (resp) {
            curl.SetReceiveHandler(&kBodyAndHeaderHandler, resp, nullptr);
        }

        size_t file_left_size = file->GetFileSize() - offset;
        size_t upload_size = size > file_left_size ? file_left_size : size;
        UploadBuffer buffer(file, upload_size, offset);
        curl.SetUploadBuf(&buffer, HTTP_METHOD_PUT);
        return curl.SendRequest(req, HTTP_METHOD_PUT);
    }

    // private
    int HttpClient::Do(const HttpMethod& method, const Request* req, Response* resp) const
    {
        ScopeHolder holder(EasyCurlPool::Instance().Get());
        EasyCurl &curl = holder.GetEasyCurl();
        if (resp) {
            curl.SetReceiveHandler(&kBodyAndHeaderHandler, resp, nullptr);
        }

        if (method == HTTP_METHOD_POST || method == HTTP_METHOD_PUT) {
            const char* buf = req->GetData();
            size_t size = req->GetSize();
            if (buf && size > 0) {
                UploadBuffer buffer(buf, size);
                curl.SetUploadBuf(&buffer, method);
            }
            else {
                curl.SetUploadBuf(static_cast<UploadBuffer*>(0x0L), method);
            }
        }

        return curl.SendRequest(req, method);
    }

    // public
    std::shared_ptr<Response> HttpClient::Get(const Request* req, const File* file) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Get(req, resp.get(), file);
        return resp;
    }

    // public
    std::shared_ptr<Response> HttpClient::Put(const Request* req, const File* file) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Put(req, resp.get(), file);
        return resp;
    }

} // namespace swift
