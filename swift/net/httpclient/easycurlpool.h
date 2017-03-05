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

#ifndef __SWIFT_NET_HTTP_CLIENT_EASY_CURL_POOL_H__
#define __SWIFT_NET_HTTP_CLIENT_EASY_CURL_POOL_H__

#include <mutex>
#include <atomic>
#include <vector>
#include <memory>

#include "swift/base/rwspinlock.h"
#include "swift/base/noncopyable.hpp"
#include "swift/net/httpclient/easycurl.h"

namespace swift {

class EasyCurlPool : swift::noncopyable
{
private:
    class EasyCurlHandler : swift::noncopyable
    {
    public:
        EasyCurlHandler () : curl_(), used_(false)
        {
        }

        inline ~EasyCurlHandler();
        inline EasyCurl& GetCurl();
        inline bool GetUsed() const;
        inline bool UsedCompareExchange(bool compare_value, bool used);
        inline void SetUsed(bool used);
        inline void Reset();

    private:
        EasyCurl curl_;
        std::atomic<bool> used_;
    };

    typedef RWTicketSpinLock64 LockType;
    typedef typename RWTicketSpinLock64::ReadHolder ReadLockGuard;
    typedef typename RWTicketSpinLock64::WriteHolder WriteLockGuard;

public:
    typedef std::shared_ptr<EasyCurlHandler> EasyCurlHandlerSPtrType;

    class EasyCurlHolder
    {
    public:
        EasyCurlHolder(const EasyCurlHandlerSPtrType& handler) : handler_(handler)
        {

        }

        inline ~EasyCurlHolder();
        inline EasyCurl& GetEasyCurl() const;
    private:
        const EasyCurlHandlerSPtrType handler_;
    };

public:
    static inline EasyCurlPool& Instance();
    inline void Clear();
    EasyCurlHandlerSPtrType Get();
    ~EasyCurlPool();

private:
    EasyCurlPool();

private:
    LockType lock_;
    std::atomic<size_t> rider_;
    std::vector<EasyCurlHandlerSPtrType> handlers_;
    static const int kMax = 100;
    static const std::unique_ptr<EasyCurlPool> kPool;
}; // ConnectionPool

} // namespace swift

#include "swift/net/httpclient/easycurlpool.inl"

#endif // __SWIFT_NET_HTTP_CLIENT_EASY_CURL_POOL_H__
