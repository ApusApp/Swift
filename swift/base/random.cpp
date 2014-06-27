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
#include <random>
#include <algorithm>

#if __GNUC_PREREQ(4, 8)
#include <ext/random>
#define USE_SIMD_PRNG
#endif

#include "swift/base/thisthread.h"

namespace swift {

namespace detail {

std::atomic<uint32_t> seed_input(0);

class ThreadLocalPRNG::LocalInstancePRNG
{
#ifdef USE_SIMD_PRNG
    typedef __gnu_cxx::sfmt19937 RNG;
#else
    typedef std::mt19937 RNG;
#endif

static RNG MakeRng()
{
    std::array<int, RNG::state_size> seed_data;
    std::random_device rdev;  // /dev/urandom
    std::generate_n(seed_data.data(), seed_data.size(), std::ref(rdev));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    return RNG(seq);
}

public:
    LocalInstancePRNG() : rng_(std::move(MakeRng())) {}

public:
    RNG rng_;
};

swift::ThreadLocal<ThreadLocalPRNG::LocalInstancePRNG> ThreadLocalPRNG::kLocalInstance;

ThreadLocalPRNG::ThreadLocalPRNG() : local_(kLocalInstance.Get()) {};

// static private
uint32_t ThreadLocalPRNG::GetImpl(LocalInstancePRNG* local)
{
    assert(nullptr != local);
    return local->rng_();
}

} // namespace detail

uint32_t Random::RandomNumberSeed()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return 51551u * (detail::seed_input++)
         + 61631u * static_cast<uint32_t>(swift::thisthread::GetTid())
         + 64997u * static_cast<uint32_t>(tv.tv_sec)
         + 111857u * static_cast<uint32_t>(tv.tv_usec);
}

} // namespace swift
