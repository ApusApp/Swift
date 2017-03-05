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

#ifndef __SWIFT_NET_HTTP_CLIENT_EASY_CURL_POOL_INL__
#define __SWIFT_NET_HTTP_CLIENT_EASY_CURL_POOL_INL__

namespace swift {

    // static public
    EasyCurlPool& EasyCurlPool::Instance()
    {
        return *kPool;
    }

    // public
    void EasyCurlPool::Clear()
    {
        WriteLockGuard lock(lock_);
        handlers_.clear();
        rider_.store(0, std::memory_order_release);
    }

    // public
    EasyCurlPool::EasyCurlHolder::~EasyCurlHolder()
    {
        handler_->Reset();
        handler_->SetUsed(false);
    }

    // public
    EasyCurl& EasyCurlPool::EasyCurlHolder::GetEasyCurl() const
    {
        return handler_->GetCurl();
    }

    // public
    void EasyCurlPool::EasyCurlHandler::Reset()
    {
        curl_.Reset();
    }

    // public
    void EasyCurlPool::EasyCurlHandler::SetUsed(bool used)
    {
        used_.store(used, std::memory_order_release);
    }

    // public
    bool EasyCurlPool::EasyCurlHandler::GetUsed() const
    {
        return used_.load(std::memory_order_acquire);
    }

    // public
    bool EasyCurlPool::EasyCurlHandler::UsedCompareExchange(bool compare_value, bool used)
    {
        return std::atomic_compare_exchange_strong(&used_, &compare_value, used);
    }

    // public
    EasyCurl& EasyCurlPool::EasyCurlHandler::GetCurl()
    {
        return curl_;
    }

    EasyCurlPool::EasyCurlHandler::~EasyCurlHandler()
    {

    }
} // namespace swift

#endif // __SWIFT_NET_HTTP_CLIENT_EASY_CURL_POOL_INL__
