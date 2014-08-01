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

#include "swift/base/random.h"

#include <atomic>
#include <unistd.h>
#include <sys/time.h>
#include <cassert>
#include <array>
#include <algorithm>

#include "swift/base/logging.h"
#include "swift/base/file.h"
#include "swift/base/likely.h"
#include "swift/base/singleton.hpp"

namespace swift {
namespace {

// Use Singleton ot keep it open for the durarion of the program
class RandomDevice : public Singleton<RandomDevice>
{
public:
    RandomDevice() : dev_()
    {
        PCHECK(true == dev_.Open("/dev/urandom"));
    }

    inline void Read(void* data, size_t size)
    {
        PCHECK(size == dev_.Read(static_cast<char*>(data), size));
    }

private:
    File dev_;
};

class BufferedRandomDevice
{
public:
    explicit BufferedRandomDevice(size_t buffer_size = kDefaultBufferSize)
        : buffer_size_(buffer_size)
        , buffer_(new unsigned char[buffer_size])
        , ptr_(buffer_.get() + buffer_size)
    {

    };

    ~BufferedRandomDevice() {};

    void Get(void* data, size_t size)
    {
        if (LIKELY(size <= Remaining())) {
            ::memcpy(data, ptr_, size);
            ptr_ += size;
        } else {
            GetSlow(static_cast<unsigned char*>(data), size);
        }
    }

private:
    void GetSlow(unsigned char* data, size_t size)
    {
        DCHECK_GT(size, Remaining());
        if (size >= buffer_size_) {
            // Just read directly
            RandomDevice::Instance().Read(data, size);
            return;
        }

        size_t copied = Remaining();
        ::memcpy(data, ptr_, copied);
        data += copied;
        size -= copied;

        // refill
        RandomDevice::Instance().Read(buffer_.get(), buffer_size_);
        ptr_ = buffer_.get();

        ::memcpy(data, ptr_, size);
        ptr_ += size;
    }

    inline size_t Remaining() const
    {
        return buffer_.get() + buffer_size_ - ptr_;
    }

public:
    static constexpr size_t kDefaultBufferSize = 128;

private:
    const size_t buffer_size_;
    std::unique_ptr<unsigned char[]> buffer_;
    unsigned char* ptr_;
}; // BufferedRandomDevice

ThreadLocal<BufferedRandomDevice> buffered_random_device;

} // anonymous namespace

namespace detail {

class ThreadLocalPRNG::LocalInstancePRNG
{
public:
    LocalInstancePRNG() : rng_(std::move(Random::Create())) {}
    ~LocalInstancePRNG() {}

public:
    Random::DefaultGenerator rng_;
};

ThreadLocal<ThreadLocalPRNG::LocalInstancePRNG> ThreadLocalPRNG::kLocalInstance;

ThreadLocalPRNG::ThreadLocalPRNG() : local_(kLocalInstance.Get())
{
    if (nullptr == local_) {
        local_ = InitLocal();
    }
}

// static private
ThreadLocalPRNG::LocalInstancePRNG* ThreadLocalPRNG::InitLocal()
{
    LocalInstancePRNG* prng = new LocalInstancePRNG;
    kLocalInstance.Reset(prng);
    return prng;
}

// static private
uint32_t ThreadLocalPRNG::GetImpl(LocalInstancePRNG* local)
{
    assert(nullptr != local);
    return local->rng_();
}

} // namespace detail

// static public
void Random::SecureRandom(void* data, size_t size)
{
    buffered_random_device->Get(data, size);
}

} // namespace swift
