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

#include "swift/net/httpclient/easycurlpool.h"

namespace swift {

    const std::unique_ptr<EasyCurlPool> EasyCurlPool::kPool(new EasyCurlPool);

    // public
    EasyCurlPool::EasyCurlHandlerSPtrType EasyCurlPool::Get()
    {
        {
            ReadLockGuard lock(lock_);
            size_t size = handlers_.size();
            for (size_t i = 0; i < size; ++i) {
                size_t n = std::atomic_fetch_add(&rider_, static_cast<size_t>(1));
                if (n >= size) {
                    std::atomic_store(&rider_, static_cast<size_t>(0));
                    n = 0;
                }

                EasyCurlHandlerSPtrType &conn = handlers_[n];
                if (conn->UsedCompareExchange(false, true)) {
                    return conn;
                }
            }
        }

        EasyCurlHandlerSPtrType handler(new EasyCurlHandler);
        {
            WriteLockGuard lock(lock_);
            if (handlers_.size() < kMax) {
                handlers_.push_back(handler);
                handler->SetUsed(true);
            }
        }

        return handler;
    }

    EasyCurlPool::~EasyCurlPool()
    {
        EasyCurl::GlobalCleanUp();
    }

    EasyCurlPool::EasyCurlPool() : lock_(), rider_(0), handlers_()
    {
        EasyCurl::GlobalInit();
    }

} // namespace swift
