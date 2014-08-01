/*
 * Copyright 2014 Facebook, Inc.
 *
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

#ifndef __SWIFT_BASE_RANDOM_INL_H__
#define __SWIFT_BASE_RANDOM_INL_H__

#ifndef __SWIFT_BASE_RANDOM_H__
#error This file may only be include from random.h
#endif

#include <array>

namespace swift {
namespace detail {

template<typename RandomNumberGenerator>
struct StateSize
{
    // A sane default
    static constexpr size_t value = 512;
};

template<typename RandomNumberGenerator>
constexpr size_t StateSize<RandomNumberGenerator>::value;

template <typename UIntType, UIntType a, UIntType c, UIntType m>
struct StateSize<std::linear_congruential_engine<UIntType, a, c, m>>
{
    // From the standard [rand.eng.lcong], this is ceil(log2(m) / 32) + 3,
    // which is the same as ceil(ceil(log2(m) / 32) + 3, and
    // ceil(log2(m)) <= std::numeric_limits<UIntType>::digits
    static constexpr size_t value =
        (std::numeric_limits<UIntType>::digits + 31) / 32 + 3;
};

template <typename UIntType, UIntType a, UIntType c, UIntType m>
constexpr size_t
StateSize<std::linear_congruential_engine<UIntType, a, c, m>>::value;

template <typename UIntType, size_t w, size_t n, size_t m, size_t r,
          UIntType a, size_t u, UIntType d, size_t s,
          UIntType b, size_t t,
          UIntType c, size_t l, UIntType f>
struct StateSize<std::mersenne_twister_engine<UIntType, w, n, m, r,
                                              a, u, d, s, b, t, c, l, f>>
{
    static constexpr size_t value =
        std::mersenne_twister_engine<UIntType, w, n, m, r,
                                     a, u, d, s, b, t, c, l, f>::state_size;
};

template <typename UIntType, size_t w, size_t n, size_t m, size_t r,
          UIntType a, size_t u, UIntType d, size_t s,
          UIntType b, size_t t,
          UIntType c, size_t l, UIntType f>
constexpr size_t
StateSize<std::mersenne_twister_engine<UIntType, w, n, m, r,
                                       a, u, d, s, b, t, c, l, f>>::value;

#if USE_SIMD_PRNG

template <typename UIntType, size_t m, size_t pos1, size_t sl1, size_t sl2,
          size_t sr1, size_t sr2, uint32_t msk1, uint32_t msk2, uint32_t msk3,
          uint32_t msk4, uint32_t parity1, uint32_t parity2, uint32_t parity3,
          uint32_t parity4>
struct StateSize<__gnu_cxx::simd_fast_mersenne_twister_engine<
    UIntType, m, pos1, sl1, sl2, sr1, sr2, msk1, msk2, msk3, msk4,
    parity1, parity2, parity3, parity4>>
{
    static constexpr size_t value =
        __gnu_cxx::simd_fast_mersenne_twister_engine<
            UIntType, m, pos1, sl1, sl2, sr1, sr2,
            msk1, msk2, msk3, msk4,
            parity1, parity2, parity3, parity4>::state_size;
};

template <typename UIntType, size_t m, size_t pos1, size_t sl1, size_t sl2,
          size_t sr1, size_t sr2, uint32_t msk1, uint32_t msk2, uint32_t msk3,
          uint32_t msk4, uint32_t parity1, uint32_t parity2, uint32_t parity3,
          uint32_t parity4>
constexpr size_t
StateSize<__gnu_cxx::simd_fast_mersenne_twister_engine<
    UIntType, m, pos1, sl1, sl2, sr1, sr2, msk1, msk2, msk3, msk4,
    parity1, parity2, parity3, parity4>>::value;

#endif

template <typename UIntType, size_t w, size_t s, size_t r>
struct StateSize<std::subtract_with_carry_engine<UIntType, w, s, r>>
{
    // [rand.eng.sub]: r * ceil(w / 32)
    static constexpr size_t value = r * ((w + 31) / 32);
};

template <typename UIntType, size_t w, size_t s, size_t r>
constexpr size_t
StateSize<std::subtract_with_carry_engine<UIntType, w, s, r>>::value;

template <typename RandomNumberGenerator>
struct SeedData
{
    SeedData()
    {
        Random::SecureRandom(seed_data.begin(), seed_data.size() * sizeof(uint32_t));
    }

    static constexpr size_t state_size = StateSize<RandomNumberGenerator>::value;
    std::array<uint32_t, state_size> seed_data;
};

} // namespace detail

// static public
template<typename RandomNumberGenerator>
void Random::Seed(Random::ValidRNG<RandomNumberGenerator>& rng)
{
    detail::SeedData<RandomNumberGenerator> sd;
    std::seed_seq s(std::begin(sd.seed_data), std::end(sd.seed_data));
    rng.seed(s);
}

// static public
template<typename RandomNumberGenerator>
auto Random::Create() -> Random::ValidRNG<RandomNumberGenerator>
{
    detail::SeedData<RandomNumberGenerator> sd;
    std::seed_seq s(std::begin(sd.seed_data), std::end(sd.seed_data));
    return RandomNumberGenerator(s);
}

} // namespace swift

#endif // __SWIFT_BASE_RANDOM_INL_H__
