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

#ifndef __SWIFT_NET_HTTP_CLIENT_HTTP_CLIENT_INL__
#define __SWIFT_NET_HTTP_CLIENT_HTTP_CLIENT_INL__

namespace swift {

    // public
    int HttpClient::Head(const Request* req, Response* resp) const
    {
        return Do(HTTP_METHOD_HEAD, req, resp);
    }

    // public
    int HttpClient::Get(const Request* req, Response* resp) const
    {
        return Do(HTTP_METHOD_GET, req, resp);
    }

    // public
    int HttpClient::Copy(const Request* req, Response* resp) const
    {
        return Do(HTTP_METHOD_COPY, req, resp);
    }

    // public
    int HttpClient::Delete(const Request* req, Response* resp) const
    {
        return Do(HTTP_METHOD_DELETE, req, resp);
    }

    // public
    int HttpClient::Put(const Request* req, Response* resp) const
    {
        return Do(HTTP_METHOD_PUT, req, resp);
    }

    // public
    int HttpClient::Post(const Request* req, Response* resp) const
    {
        return Do(HTTP_METHOD_POST, req, resp);
    }

    // public
    std::shared_ptr<Response> HttpClient::Get(const Request* req) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Get(req, resp.get());
        return resp;
    }

    // public
    std::shared_ptr<Response> HttpClient::Head(const Request* req) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Head(req, resp.get());
        return resp;
    }

    // public
    std::shared_ptr<Response> HttpClient::Copy(const Request* req) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Copy(req, resp.get());
        return resp;
    }

    // public
    std::shared_ptr<Response> HttpClient::Delete(const Request* req) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Delete(req, resp.get());
        return resp;
    }

    // public
    std::shared_ptr<Response> HttpClient::Put(const Request* req) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Put(req, resp.get());
        return resp;
    }

    // public
    std::shared_ptr<Response> HttpClient::Post(const Request* req) const
    {
        std::shared_ptr<Response> resp = std::make_shared<Response>();
        Post(req, resp.get());
        return resp;
    }
}

#endif // __SWIFT_NET_HTTP_CLIENT_HTTP_CLIENT_INL__
