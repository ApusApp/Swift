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

#ifndef __SWIFT_NET_HTTP_CLIENT_HTTP_CLIENT_H__
#define __SWIFT_NET_HTTP_CLIENT_HTTP_CLIENT_H__

#include <cstdint>
#include <cstddef>
#include <memory>
#include <limits>

#include "swift/base/noncopyable.hpp"
#include "swift/net/httpclient/easycurl.h"
#include "swift/net/httpclient/httpcode.hpp"
#include "swift/net/httpclient/request.hpp"
#include "swift/net/httpclient/response.hpp"


namespace swift {

class File;
struct ReceiveHandler;

class HttpClient : swift::noncopyable
{
public:
    HttpClient();
    ~HttpClient();

    inline int Get(const Request* req, Response* resp) const;
    inline int Head(const Request* req, Response* resp) const;
    inline int Copy(const Request* req, Response* resp) const;
    inline int Delete(const Request* req, Response* resp) const;
    inline int Put(const Request* req, Response* resp) const;
    inline int Post(const Request* req, Response* resp) const;
    int Get(const Request* req, Response* resp,
            const File* file, size_t size=std::numeric_limits<size_t>::max(), size_t offset=0) const;
    int Put(const Request* req, Response* resp,
            const File* file, size_t size=std::numeric_limits<size_t>::max(), size_t offset=0) const;

    int Get(const Request* req, Response* resp, char* buf, size_t size) const;

    inline std::shared_ptr<Response> Get(const Request* req) const;
    inline std::shared_ptr<Response> Head(const Request* req) const;
    inline std::shared_ptr<Response> Copy(const Request* req) const;
    inline std::shared_ptr<Response> Delete(const Request* req) const;
    inline std::shared_ptr<Response> Put(const Request* req) const;
    inline std::shared_ptr<Response> Post(const Request* req) const;
    std::shared_ptr<Response> Get(const Request* req, const File* file) const;
    std::shared_ptr<Response> Put(const Request* req, const File* file) const;

private:
    int Do(const HttpMethod& method, const Request* req, Response* resp) const;

private:
    static const ReceiveHandler kHeaderHandler;
    static const ReceiveHandler kBodyAndHeaderHandler;
    static void HeaderHandler(Response* resp, const char* data, const size_t size);
    static void BodyHandler(Response* resp, const char* data, const size_t size);
};

}

#include "swift/net/httpclient/httpclient.inl"

#endif // __SWIFT_NET_HTTP_CLIENT_HTTP_CLIENT_H__