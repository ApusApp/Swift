#include <chrono>
#include <thread>
#include <iostream>
#include <gtest/gtest.h>

#include <swift/base/threadpool.h>

class test_ThreadPool : public testing::Test
{
public:
    test_ThreadPool () {}
    ~test_ThreadPool () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

int TestT (int a)
{
    std::this_thread::sleep_for (std::chrono::microseconds (500));
    return a * a;
}

int TestAdd (int a, int b)
{
    a += b;
    std::this_thread::sleep_for (std::chrono::microseconds (500));
    return a;
}

int TestSub (int a, int b, int c)
{
    a -= b;
    b -= c;
    std::this_thread::sleep_for (std::chrono::microseconds (500));
    return a - b;
}

int TestMultiply (int a, int b, int c, int d)
{
    std::this_thread::sleep_for (std::chrono::microseconds (500));
    return a * b * c * d;
}

int TestDivision (int a, int b, int c, int d, int e)
{
    std::this_thread::sleep_for (std::chrono::microseconds (500));
    return a / b / c / d / e;
}


TEST_F (test_ThreadPool, All)
{
    swift::ThreadPool pool (4);
    pool.Start ();

    for (int i = 0; i < 1000; ++i) {
        pool.Schedule (TestT, 3);
        pool.Schedule (TestAdd, 3, 5);
        pool.Schedule (TestSub, 3, 5, 7);
        pool.Schedule (TestMultiply, 3, 5, 7, 9);
        pool.Schedule (TestDivision, 3, 5, 7, 9, 11);
    }

    ASSERT_TRUE (pool.TasksRemaining () > 0);
    swift::ThreadPool::Task func;
    {
        func = std::move ([]() { TestAdd (100, 100); });
    }

    auto func_1 = std::move (func);
    pool.Schedule (func_1);
    ASSERT_TRUE (func == nullptr);
    func_1 ();
    pool.Schedule (std::move (func_1));
    ASSERT_TRUE (func_1 == nullptr);

    pool.Join ();
}