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

#ifndef __SWIFT_BASE_RANDOM_H__
#define __SWIFT_BASE_RANDOM_H__

#include <random>
#include <stdint.h>

#include "swift/base/threadlocal.h"

namespace swift {

class Random;
namespace detail {

// Thread local random number generator
class ThreadLocalPRNG
{
    friend class Random;
public:
    typedef uint32_t result_type;

    ThreadLocalPRNG();
    ~ThreadLocalPRNG() {}

    uint32_t operator()()
    {
        return GetImpl(local_);
    }

    static inline constexpr result_type min()
    {
        return std::numeric_limits<result_type>::min();
    }

    static inline constexpr result_type max()
    {
        return std::numeric_limits<result_type>::max();
    }

private:
    class LocalInstancePRNG;
    static result_type GetImpl(LocalInstancePRNG* local);

private:
    LocalInstancePRNG* local_;

    static swift::ThreadLocal<ThreadLocalPRNG::LocalInstancePRNG> kLocalInstance;
};
} // namespace detail

class Random
{
private:
    template<typename RandomNumberGenerator>
    using ValidRNG = typename std::enable_if<
        std::is_unsigned<typename std::result_RandomNumberGenerator&()>::type>::value,
        RandomNumberGenerator>::type;

public:
    // Return a good seed for a random number gnerator
    static uint32_t RandomNumberSeed();

    // Return a random uint32_t
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline uint32_t RandUInt32(ValidRRandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        return rrng.operator()();
    }

    // Return [0, max), if max = 0 return 0
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline uint32_t RandUInt32(uint32_t max,
                                      ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        if (0 == max) { return 0; }
        return std::uniform_int_distribution<uint32_t>(0, max - 1)(rrng);
    }

    // Return [min, max), if min == max return 0
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline uint32_t RandUInt32(uint32_t min,
                                      uint32_t max,
                                      ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        if (min == max) { return 0; }
        return std::uniform_int_distribution<uint32_t>(min, max - 1)(rrng);
    }

    // Return  a random uint64_t
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline uint64_t RandUInt64(ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        return ((uint64_t)rrng() << 32) | rrng();
    }

    // Return [0, max), if max = 0 return 0
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline uint64_t RandUInt64(uint64_t max,
                                      ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        if (0 == max) { return 0; }
        return std::uniform_int_distribution<uint64_t>(0, max - 1)(rrng);
    }

    // Return [min, max), if min == max return 0
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline uint64_t RandUInt64(uint64_t min,
                                      uint64_t max,
                                      ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        if (min == max) { return 0; }
        return std::uniform_int_distribution<uint64_t>(min, max - 1)(rrng);
    }

    // Return true 1/n of the time. If 0 == n, always return false
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline bool RandBool(uint32_t n,
                                ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        return (0 == n) ? false : (RandUInt32(n, rrng) == 0);
    }

    // Returns [0, 1)
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline double RandDouble01(ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        return std::generate_canonical<double,
                                       std::numeric_limits<double>::digits>(rrng);
    }

    // Returns a double in [min, max), if max == min, return 0
    template<typename RandomNumberGenerator = detail::ThreadLocalPRNG>
    static inline double RandDouble(double min,
                                    double max,
                                    ValidRNG<RandomNumberGenerator> rrng = RandomNumberGenerator())
    {
        if (std::fabs(max - min) < std::numeric_limits<double>::epsilon()) {
            return 0;
        }

        return std::uniform_real_distribution<double>(min, max)(rrng);
    }
};

} // namespace swift
#endif // __SWIFT_BASE_RANDOM_H__
