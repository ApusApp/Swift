#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <condition_variable>
#include <swift/base/threadlocal.h>
#include <swift/base/thisthread.h>
#include <swift/base/timestamp.h>
#include <swift/base/noncopyable.hpp>


class test_ThreadLocal : public testing::Test
{
public:
    test_ThreadLocal() {}
    ~test_ThreadLocal() {}

    virtual void SetUp(void) {}
    virtual void TearDown (void) {}
};

namespace {

struct TestData
{
    TestData(std::mutex *m, std::condition_variable *c, int *unref, int n,
             swift::UnrefHandler handler = nullptr)
        : mu(m)
        , cv(c)
        , unref(unref)
        , total(n)
        , started(0)
        , completed(0)
        , do_write(false)
        , tls1(handler)
        , tls2(nullptr) {}

    std::mutex *mu;
    std::condition_variable *cv;
    int *unref;
    int total;
    int started;
    int completed;
    bool do_write;
    swift::ThreadLocalPtr tls1;
    swift::ThreadLocalPtr *tls2;
};

class IDChecker : public swift::ThreadLocalPtr
{
public:
    static uint32_t PeekId() { return Instance()->PeekId(); }
};
} // anonymous namespace

TEST(test_ThreadLocal, UniqueId)
{
    std::mutex mu;
    std::condition_variable cv;

    uint32_t n = IDChecker::PeekId();

    EXPECT_EQ(IDChecker::PeekId(), 0u + n);
    // New ThreadLocal instance bumps id by 1
    {
        // Id used 0
        TestData p1(&mu, &cv, nullptr, 1u + n);
        EXPECT_EQ(IDChecker::PeekId(), 1u + n);
        // Id used 1
        TestData p2(&mu, &cv, nullptr, 1u + n);
        EXPECT_EQ(IDChecker::PeekId(), 2u + n);
        // Id used 2
        TestData p3(&mu, &cv, nullptr, 1u + n);
        EXPECT_EQ(IDChecker::PeekId(), 3u + n);
        // Id used 3
        TestData p4(&mu, &cv, nullptr, 1u + n);
        EXPECT_EQ(IDChecker::PeekId(), 4u + n);
    }
    // id 3, 2, 1, 0 are in the free queue in order
    EXPECT_EQ(IDChecker::PeekId(), 0u + n);

    // pick up 0
    TestData p1(&mu, &cv, nullptr, 1u + n);
    EXPECT_EQ(IDChecker::PeekId(), 1u + n);
    // pick up 1
    TestData *p2 = new TestData(&mu, &cv, nullptr, 1u + n);
    EXPECT_EQ(IDChecker::PeekId(), 2u + n);
    // pick up 2
    TestData p3(&mu, &cv, nullptr, 1u + n);
    EXPECT_EQ(IDChecker::PeekId(), 3u + n);
    // return up 1
    delete p2;
    EXPECT_EQ(IDChecker::PeekId(), 1u + n);
    // Now we have 3, 1 in queue
    // pick up 1
    TestData p4(&mu, &cv, nullptr, 1u + n);
    EXPECT_EQ(IDChecker::PeekId(), 3u + n);
    // pick up 3
    TestData p5(&mu, &cv, nullptr, 1u + n);
    // next new id
    EXPECT_EQ(IDChecker::PeekId(), 4u + n);
    // After exit, id sequence in queue:
    // 3, 1, 2, 0
}

TEST(test_ThreadLocal, ConcurrentReadWrite)
{
    uint32_t n = IDChecker::PeekId();
    EXPECT_EQ(IDChecker::PeekId(), 0u + n);

    swift::ThreadLocalPtr tls2;
    std::mutex mu1;
    std::condition_variable cv1;
    TestData p1(&mu1, &cv1, nullptr, 16);
    p1.tls2 = &tls2;

    std::mutex mu2;
    std::condition_variable cv2;
    TestData p2(&mu2, &cv2, nullptr, 16);
    p2.do_write = true;
    p2.tls2 = &tls2;

    auto func = [](TestData *ptr) {
        assert (nullptr != ptr);
        int own = 0;
        {
            std::unique_lock<std::mutex> lock (*(ptr->mu));
            own = ++(ptr->started);
            ptr->cv->notify_all();

            while (ptr->started != ptr->total) {
                ptr->cv->wait(lock);
            }
        }

        if (ptr->do_write) {
            own += 8192;
        }

        EXPECT_TRUE(nullptr == ptr->tls1.Get());
        EXPECT_TRUE(nullptr == ptr->tls2->Get());

        swift::Timestamp start = swift::Timestamp::Now();
        ptr->tls1.Reset(reinterpret_cast<int*>(own));
        ptr->tls2->Reset(reinterpret_cast<int*>(own + 1));
        while (swift::TimeDifference(swift::Timestamp::Now(), start) < 1) {
            for (unsigned i = 0; i < 100000; ++i) {
                EXPECT_TRUE(ptr->tls1.Get() == reinterpret_cast<int*>(own));
                EXPECT_TRUE(ptr->tls2->Get() == reinterpret_cast<int*>(own + 1));

                if (ptr->do_write) {
                    ptr->tls1.Reset(reinterpret_cast<int*>(own));
                    ptr->tls2->Reset(reinterpret_cast<int*>(own + 1));
                }
            }
        }

        std::unique_lock<std::mutex> lock(*(ptr->mu));
        ++(ptr->completed);
        ptr->cv->notify_all();
    };

    // Initiate 2 instnaces: one keeps writing and one keeps reading.
    // The read instance should not see data from the write instance.
    // Each thread local copy of the value are also different from each
    // other.
    std::vector<std::thread> ths;
    for (int i = 0; i < p1.total; ++i) {
        ths.push_back(std::move(std::thread(func, &p1)));
    }

    for (int i = 0; i < p2.total; ++i) {
        ths.push_back(std::move(std::thread(func, &p2)));
    }

    {
        std::unique_lock<std::mutex> lock(mu1);
        while (p1.completed != p1.total) {
            cv1.wait(lock);
        }
    }

    {
        std::unique_lock<std::mutex> lock(mu2);
        while (p2.completed != p2.total) {
            cv2.wait(lock);
        }
    }


    for (auto && i : ths) {
        i.join();
    }

    EXPECT_EQ(IDChecker::PeekId(), 3u + n);
}

TEST(test_ThreadLocal, SequentialReadWrite)
{
    uint32_t n = IDChecker::PeekId();
    EXPECT_EQ(IDChecker::PeekId(), 0u + n);
    std::mutex mu;
    std::condition_variable cv;
    TestData p(&mu, &cv, nullptr, 1);
    swift::ThreadLocalPtr tls2;
    p.tls2 = &tls2;

    auto func = [](TestData *ptr) {
        EXPECT_TRUE(ptr->tls1.Get() == nullptr);
        ptr->tls1.Reset(reinterpret_cast<int*>(1));
        EXPECT_TRUE(ptr->tls1.Get() == reinterpret_cast<int*>(1));
        ptr->tls1.Reset(reinterpret_cast<int*>(2));
        EXPECT_TRUE(ptr->tls1.Get() == reinterpret_cast<int*>(2));

        EXPECT_TRUE(ptr->tls2->Get() == nullptr);
        ptr->tls2->Reset(reinterpret_cast<int*>(1));
        EXPECT_TRUE(ptr->tls2->Get() == reinterpret_cast<int*>(1));
        ptr->tls2->Reset(reinterpret_cast<int*>(2));
        EXPECT_TRUE(ptr->tls2->Get() == reinterpret_cast<int*>(2));
        ++(ptr->completed);
        std::unique_lock<std::mutex> lock (*ptr->mu);
        ptr->cv->notify_all();
    };

    std::vector<std::thread> ths;
    for (int iter = 0; iter < 1024; ++iter) {
        EXPECT_EQ(IDChecker::PeekId(), 1u + n);
        // Another new thread, read/write should not see value from previous thread
        ths.push_back(std::move(std::thread (func, &p)));

        while (p.completed != iter + 1) {
            std::unique_lock<std::mutex> lock (mu);
            cv.wait(lock);
        }

        EXPECT_EQ(IDChecker::PeekId(), 1u + n);
    }

    for (auto && i : ths) {
        i.join();
    }
}

TEST(test_ThreadLocal, Unref)
{
    uint32_t n = IDChecker::PeekId();
    EXPECT_EQ(IDChecker::PeekId(), 0u + n);
    auto unref_func = [](void *ptr) {
        TestData *data = static_cast<TestData*>(ptr);
        data->mu->lock();
        ++(*data->unref);
        data->mu->unlock();
    };

    // Case 0: no unref triggered if ThreadLocalPtr is never accessed
    auto func0 = [](TestData *data) {
        std::unique_lock<std::mutex> lock(*data->mu);
        ++(data->started);
        data->cv->notify_all();
        while (data->started != data->total) {
            data->cv->wait(lock);
        }
    };

    for (int i = 1; i <= 128; i += i) {
        std::mutex mu;
        std::condition_variable cv;
        int unref_count = 0;
        TestData data(&mu, &cv, &unref_count, i, unref_func);
        std::vector<std::thread> ths;
        for (int i = 0; i < data.total; ++i) {
            ths.push_back(std::move(std::thread(func0, &data)));
        }

        for (auto&& t : ths) {
            t.join();
        }
        EXPECT_EQ(unref_count, 0);
    }

    // Case 1: unref triggered by thread exit
    auto func1 = [](TestData * data) {
        {
            std::unique_lock<std::mutex> lock(*data->mu);
            ++(data->started);
            data->cv->notify_all();
            while (data->started != data->total) {
                data->cv->wait(lock);
            }
        }

        EXPECT_TRUE(data->tls2->Get() == nullptr);
        EXPECT_TRUE(data->tls1.Get() == nullptr);
        data->tls1.Reset(data);
        data->tls2->Reset(data);

        data->tls1.Reset(data);
        data->tls2->Reset(data);
    };

    for (int i = 1; i <= 128; i += i) {
        std::mutex mu;
        std::condition_variable cv;
        int unref_count = 0;
        swift::ThreadLocalPtr tls2(unref_func);
        TestData data(&mu, &cv, &unref_count, i, unref_func);
        data.tls2 = &tls2;

        std::vector<std::thread> ths;
        for (int i = 0; i < data.total; ++i) {
            ths.push_back(std::move(std::thread(func1, &data)));
        }

        for (auto&& t : ths) {
            t.join();
        }

        EXPECT_EQ(unref_count, 2 * data.total);
    }

    // case 2: unref triggered by ThreadLocal instance destruction
    auto func2 = [](TestData *data) {
        {
            std::unique_lock<std::mutex> lock(*data->mu);
            ++(data->started);
            data->cv->notify_all();
            while (data->started != data->total) {
                data->cv->wait(lock);
            }
        }
        EXPECT_TRUE(nullptr == data->tls1.Get());
        EXPECT_TRUE(nullptr == data->tls2->Get());

        data->tls1.Reset(data);
        data->tls2->Reset(data);

        data->tls1.Reset(data);
        data->tls2->Reset(data);

        std::unique_lock<std::mutex> lock(*data->mu);
        ++(data->completed);
        data->cv->notify_all();
        while (data->completed != 0) {
            data->cv->wait(lock);
        }
    };

    for (int i = 1; i <= 128; i += i) {
        std::mutex mu;
        std::condition_variable cv;
        int unref_count = 0;
        TestData  data(&mu, &cv, &unref_count, i, unref_func);
        data.tls2 = new swift::ThreadLocalPtr(unref_func);

        std::vector<std::thread> ths;
        for (int i = 0; i < data.total; ++i) {
            ths.push_back(std::move(std::thread(func2, &data)));
        }

        {
            std::unique_lock<std::mutex> lock(mu);
            while (data.completed != data.total) {
                cv.wait(lock);
            }
        }

        delete data.tls2;
        data.tls2 = nullptr;
        EXPECT_EQ(unref_count, data.total);

        // signal to exit
        mu.lock();
        data.completed = 0;
        cv.notify_all();
        mu.unlock();

        for (auto&& i : ths) {
            i.join();
        }

        EXPECT_EQ(unref_count, 2 * data.total);
    }
}

TEST(test_ThreadLocal, Swap)
{
    swift::ThreadLocalPtr tls;
    tls.Reset(reinterpret_cast<void*>(1));
    EXPECT_EQ(reinterpret_cast<int64_t>(tls.Swap(nullptr)), 1);
    EXPECT_TRUE(tls.Swap(reinterpret_cast<void*>(2)) == nullptr);
    EXPECT_EQ(reinterpret_cast<int64_t>(tls.Get()), 2);
    EXPECT_EQ(reinterpret_cast<int64_t>(tls.Swap(reinterpret_cast<void*>(3))), 2);
}

TEST(test_ThreadLocal, CompareAndSwap)
{
    swift::ThreadLocalPtr tls;
    EXPECT_TRUE(nullptr == tls.Swap(reinterpret_cast<void*>(1)));
    void *expected = reinterpret_cast<void*>(1);
    EXPECT_TRUE(tls.CompareAndSwap(reinterpret_cast<void*>(2), expected));
    expected = reinterpret_cast<void*>(1000);
    EXPECT_TRUE(!tls.CompareAndSwap(reinterpret_cast<void*>(2), expected));
    EXPECT_EQ(expected, reinterpret_cast<void*>(2));

    expected = reinterpret_cast<void*>(2);
    EXPECT_TRUE(tls.CompareAndSwap(reinterpret_cast<void*>(3), expected));
    EXPECT_EQ(tls.Get(), reinterpret_cast<void*>(3));
}

TEST(test_ThreadLocal, Scrape)
{
    auto unref_func = [](void *ptr) {
        assert(nullptr != ptr);
        TestData *data = static_cast<TestData*>(ptr);
        data->mu->lock();
        ++(*data->unref);
        data->mu->unlock();
    };

    auto func = [](TestData *data) {
        assert(nullptr != data);
        ASSERT_TRUE(nullptr == data->tls1.Get());
        ASSERT_TRUE(nullptr == data->tls2->Get());

        data->tls1.Reset(data);
        data->tls2->Reset(data);

        data->tls1.Reset(data);
        data->tls2->Reset(data);

        std::unique_lock<std::mutex> lock(*data->mu);
        ++(data->completed);
        data->cv->notify_all();

        while (data->completed != 0) {
            data->cv->wait(lock);
        }
    };

    for (int i = 1; i <= 128; i += i) {
        std::mutex mu;
        std::condition_variable cv;
        int unref_count = 0;
        TestData data(&mu, &cv, &unref_count, i, unref_func);
        data.tls2 = new swift::ThreadLocalPtr(unref_func);
        std::vector<std::thread> ths;
        for (int i = 0; i < data.total; ++i) {
            ths.push_back(std::move(std::thread(func, &data)));
        }

        {
            std::unique_lock<std::mutex> lock(*data.mu);
            while (data.completed != data.total) {
                cv.wait(lock);
            }
        }
        EXPECT_EQ(unref_count, 0);

        // Scrape all thread local data. No unref at thread exit
        // or ThreadLocalPtr destruction
        std::vector<void*> ptrs;
        data.tls1.Scrape(&ptrs, nullptr);
        data.tls2->Scrape(&ptrs, nullptr);
        delete data.tls2;

        // Signal to exit
        mu.lock();
        data.completed = 0;
        cv.notify_all();
        mu.unlock();

        for (auto&& i : ths) {
            i.join();
        }

        EXPECT_EQ(unref_count, 0);
    }
}

class LocalData : swift::noncopyable
{
public:
    LocalData ()
    {
        std::cout << "Thread is = " << swift::thisthread::GetTid ()
                  << "\tConstructor " << this << std::endl;
    }

    ~LocalData ()
    {
        std::cout << "Thread is = " << swift::thisthread::GetTid ()
                  << "\tDestructor " << this
                  << "\tname = " << name_ << std::endl;
    }

    const std::string &GetName () const
    {
        return name_;
    }

    void SetName (const std::string &name)
    {
        name_ = name;
    }

private:
    std::string name_;
};

struct Tls
{
    swift::ThreadLocal<LocalData> tls_d1;
    swift::ThreadLocal<LocalData> tls_d2;
};



static inline void Dsiplay(const Tls* t)
{
    std::cout << "Thread id = " << swift::thisthread::GetTid()
              << "\ttls_d1 " << t->tls_d1.Get()
              << "\ttls_d1 name " << t->tls_d1.Get()->GetName() << std::endl;
    std::cout << "Thread id = " << swift::thisthread::GetTid()
              << "\ttls_d2 " << t->tls_d2.Get()
              << "\ttls_d2 name " << t->tls_d2.Get()->GetName() << std::endl;
}

void TestFunc(const Tls* t)
{
    std::cout << "Before..." << std::endl;
    Dsiplay(t);
    t->tls_d1.Get()->SetName ("son_thread_tls_d1");
    t->tls_d2.Get()->SetName ("son_thread_tls_d2");
    std::cout << "After..." << std::endl;
    Dsiplay(t);
}

TEST(test_ThreadLocal, ThreadLocal)
{
    Tls t;
    std::cout << "Main one..." << std::endl;
    t.tls_d1->SetName("main_thread_tls_d1");
    Dsiplay(&t);

    std::thread t1(TestFunc, &t);
    t1.join();

    (*t.tls_d2).SetName("main_thread_tls_d2");
    Dsiplay(&t);

    std::cout << "Main two..." << std::endl;
    std::thread t2(TestFunc, &t);
    t2.join();

    std::cout << "Main Reset..." << std::endl;
    LocalData *d = new LocalData;
    d->SetName("New Data");
    t.tls_d2.Reset(d);
    Dsiplay(&t);

    t.tls_d2.Reset(nullptr);
    t.tls_d2.Reset(nullptr);
    EXPECT_TRUE(t.tls_d2->GetName().empty());
}

