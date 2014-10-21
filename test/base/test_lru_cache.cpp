#include <gtest/gtest.h>
#include <iostream>
#include <sys/time.h>
#include <swift/base/lru_cache.hpp>
#include <thread>

class test_LruCache : public testing::Test
{
public:
  test_LruCache() {}
  ~test_LruCache() {}
};

TEST_F (test_Crc32, All)
{
    //
    // base case:
    //
    LruCache<int, int> lru_cache(3);
    lru_cache.Set(1, 1);
    lru_cache.Set(2, 2);
    lru_cache.Set(3, 3);

    // order: 3, 2, 1

    int value = 0;
    lru_cache.Get(1, value);
    EXPECT_EQ(value, 1);

    // order 1, 3, 2  erase 2
    lru_cache.Set(4, 4);
    ASSERT_FALSE(lru_cache.Get(2, value));

    // benchmark
    LruCache<int, int> lru_cache_bench(2000);

    size_t thread_num = 4;
    size_t run_times = 1000;

    auto run_func = [&](LruCache& lru_cache, size_t run_times) {
      int value;
      for (size_t i = 0; i < run_times; ++i) {
        if (i % 2 == 0) {
          lru_cache.Set(i, i);
        } else {
          lru_cache.Get(i - 1, value);
        }
      }
    };


    timeval t1, t2;
    gettimeofday(&t1, NULL);

    std::vector<std::thread> thread_pool;
    for (size_t i = 0; i < thread_num; ++i) {
      thread_pool.push_back(std::thread(run_func, lru_cache_bench, run_times));
    }

    for (auto& t : thread_pool) {
      t.join();
    }

    gettimeofday(&t2, NULL);
    time_t time_cost = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    double time_in_sec = double(time_cost) / 1000000;
    double total_num = double(thread_num * run_times);
    double qps = total_num / time_in_sec;
    std::cout << "lru_cache: thread_num[" << thread_num << "] qps:" << qps << std::endl;
}