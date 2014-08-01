#include <swift/base/random.h>

#include <gtest/gtest.h>
#include <algorithm>
#include <thread>
#include <vector>
#include <exception>
#include <iostream>

class test_Random : public testing::Test
{
public:
    test_Random() {};
    ~test_Random() {};
};

TEST(test_Random, RandomSeed)
{
    uint32_t prev = 0;
    uint32_t seed = 0;
    for (int i = 0; i < 2048; ++i) {
        EXPECT_NE(prev, seed = swift::Random::RandomNumberSeed());
        prev = seed;
    }
}

TEST(test_Random, MultiThreadSeed)
{
    int count = 20;
    std::vector<uint32_t> seeds(count);
    std::vector<std::thread> ths;

    try {
        for (int i = 0; i < count; ++i) {
            ths.push_back(std::move(std::thread([i, &seeds]{
                seeds[i] = swift::Random::RandomNumberSeed();
            })));
        }

        for (auto&& i : ths) {
            i.join();
        }

        std::sort(seeds.begin(), seeds.end());
        for (int i = 0; i < count - 1; ++i) {
            EXPECT_LT(seeds[i], seeds[i + 1]);
        }
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

TEST(test_Random, RandUInt64)
{
    uint32_t n = swift::Random::RandUInt64();
    n = swift::Random::RandUInt64(0);
    EXPECT_TRUE(n == 0);
    n = swift::Random::RandUInt64(1);  // [0, 1)
    EXPECT_TRUE(n == 0);

    int count = 20;
    std::vector<uint32_t> values(count);
    std::vector<std::thread> ths;
    for (int i = 0; i < count; ++i) {
        ths.push_back(std::move(std::thread([i, &values]{
            values[i] = swift::Random::RandUInt64(100, 1000);  // [100, 1000)
        })));
    }

    for (auto&& i : ths) {
        i.join();
    }

    std::sort(values.begin(), values.end());
    for (int i = 0; i < count - 1; ++i) {
        EXPECT_LE(values[i], values[i + 1]);
        EXPECT_LT(values[i], 1000);
        EXPECT_GE(values[i], 100);
    }

    swift::detail::ThreadLocalPRNG prng;
    n = swift::Random::RandUInt64(1, prng);  // [0, 1)
    EXPECT_TRUE(n == 0);
}

TEST(test_Random, RandUInt32)
{
    uint32_t n = swift::Random::RandUInt32();
    n = swift::Random::RandUInt32(0);
    EXPECT_TRUE(n == 0);
    n = swift::Random::RandUInt32(1);  // [0, 1)
    EXPECT_TRUE(n == 0);

    int count = 20;
    std::vector<uint32_t> values(count);
    std::vector<std::thread> ths;
    for (int i = 0; i < count; ++i) {
        ths.push_back(std::move(std::thread([i, &values]{
            values[i] = swift::Random::RandUInt32(100, 1000);  // [100, 1000)
        })));
    }

    for (auto&& i : ths) {
        i.join();
    }

    std::sort(values.begin(), values.end());
    for (int i = 0; i < count - 1; ++i) {
        EXPECT_LE(values[i], values[i + 1]);
        EXPECT_LT(values[i], 1000);
        EXPECT_GE(values[i], 100);
    }

    swift::detail::ThreadLocalPRNG prng;
    n = swift::Random::RandUInt32(1, prng);  // [0, 1)
    EXPECT_TRUE(n == 0);
}

TEST(test_Random, RandBool)
{
    bool b = swift::Random::RandBool(0);
    EXPECT_EQ(b, false);

    std::vector<std::thread> ths;
    for (int i = 0; i < 20; ++i) {
        ths.push_back(std::move(std::thread([&i] {
            for (int j = 0; j < i * 1000; ++j) {
                swift::Random::RandBool(1000);
            }
        })));
    }

    for (auto&& t : ths) {
        t.join();
    }
}

TEST(test_Random, RandDouble)
{
    double v = swift::Random::RandDouble01();
    EXPECT_LT(v, 1);
    EXPECT_GE(v, 0);

    std::vector<std::thread> ths;
    for (int i = 0; i < 20; ++i) {
        ths.push_back(std::move(std::thread([&i] {
            for (int j = 0; j < i * 1000; ++j) {
                double v = swift::Random::RandDouble01();
                EXPECT_LT(v, 1);
                EXPECT_GE(v, 0);

                v = swift::Random::RandDouble(0, j);
                EXPECT_LE(v, j);
                EXPECT_GE(v, 0);
            }
        })));
    }

    for (auto&& t : ths) {
        t.join();
    }
}

TEST(test_Random, String)
{
    std::string s;
    swift::Random::RandomString(&s, 10);
    EXPECT_TRUE(!s.empty());
    EXPECT_EQ(s.size(), 10);
    for (size_t i = 0; i < s.size(); ++i)
    {
        printf("%02X", s[i]);
    }
    printf("\n");

    s.clear();
    swift::Random::RandomString(&s, 0);
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);

    std::string* p = nullptr;
    swift::Random::RandomString(p, 100);
    EXPECT_EQ(p, nullptr);

    EXPECT_EQ(sizeof(uint32_t), sizeof(swift::Random::SecureRandom<uint32_t>()));
    EXPECT_EQ(sizeof(int32_t),  sizeof(swift::Random::SecureRandom<int32_t>()));
    EXPECT_EQ(sizeof(uint64_t), sizeof(swift::Random::SecureRandom<uint64_t>()));
    EXPECT_EQ(sizeof(int64_t),  sizeof(swift::Random::SecureRandom<int64_t>()));
    EXPECT_EQ(sizeof(char),     sizeof(swift::Random::SecureRandom<char>()));
    EXPECT_EQ(sizeof(unsigned), sizeof(swift::Random::SecureRandom<unsigned>()));
    EXPECT_EQ(sizeof(unsigned char), sizeof(swift::Random::SecureRandom<unsigned char>()));
}

TEST(test_Random, StateSize)
{
    EXPECT_EQ(sizeof(uint_fast32_t) / 4 + 3,
        swift::detail::StateSize<std::minstd_rand0>::value);
    EXPECT_EQ(624, swift::detail::StateSize<std::mt19937>::value);
    #if USE_SIMD_PRNG
    EXPECT_EQ(624, swift::detail::StateSize<__gnu_cxx::sfmt19937>::value);
    #endif
    EXPECT_EQ(24, swift::detail::StateSize<std::ranlux24_base>::value);
}