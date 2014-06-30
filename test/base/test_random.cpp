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