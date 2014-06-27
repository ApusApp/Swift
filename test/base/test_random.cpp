#include <swift/base/random.h>

#include <gtest/gtest.h>
#include <algorithm>
#include <thread>
#include <vector>

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
    int count = 1024;
    std::vector<uint32_t> seeds(count);
    std::vector<std::thread> ths;
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

void TestFunc()
{
#ifdef f
    uint32_t n = f();
    n = f(0);
    EXPECT_TRUE(n == 0);
    n = f(1);  // [0, 1)
    EXPECT_TRUE(n == 0);

    int count = 1024;
    std::vector<uint32_t> values(count);
    std::vector<std::thread> ths;
    for (int i = 0; i < count; ++i) {
        ths.push_back(std::move(std::thread([i, &values]{
            values[i] = f(100, 1000);  // [100, 1000)
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
    n = f(1, prng);  // [0, 1)
    EXPECT_TRUE(n == 0);
#endif
}

TEST(test_Random, RandUInt64)
{
    #define f swift::Random::RandUInt64
    TestFunc();
    #undef f
}

TEST(test_Random, RandUInt32)
{
    #define f swift::Random::RandUInt32
    TestFunc();
    #undef f
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