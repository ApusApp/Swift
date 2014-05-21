#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <gtest/gtest.h>
#include <swift/base/rwspinlock.h>

namespace {

template<typename RWSpinLockT>
class test_RWSpinLock : public testing::Test
{
public:
    typedef RWSpinLockT RWSpinLockType;

    test_RWSpinLock () {}
    ~test_RWSpinLock () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST (test_RWSpinLock, RWSpinLock)
{
    {
        swift::RWSpinLock lock;
        lock.lock ();
        ASSERT_TRUE (lock.try_lock_shared () == false);
        lock.unlock ();
        lock.lock_shared ();
        ASSERT_TRUE (lock.try_lock_shared () == true);
        lock.unlock_shared ();
        lock.unlock_shared ();
        lock.lock ();
        lock.unlock_and_lock_shared ();
        ASSERT_TRUE (lock.try_lock () == false);
    }
    {
        swift::RWSpinLock lock;
        EXPECT_TRUE (lock.try_lock_upgrade ());
        EXPECT_FALSE (lock.try_lock_shared ());
        EXPECT_FALSE (lock.try_lock ());
        EXPECT_FALSE (lock.try_lock_upgrade ());
        lock.unlock_upgrade ();
        lock.lock_shared ();
        EXPECT_FALSE (lock.try_lock ());
        EXPECT_TRUE (lock.try_lock_upgrade ());
        lock.unlock_upgrade ();
        lock.unlock_shared ();
        EXPECT_TRUE (lock.try_lock ());
        EXPECT_FALSE (lock.try_lock_upgrade ());
        lock.unlock_and_lock_upgrade ();
        EXPECT_FALSE (lock.try_lock_shared ());
        lock.unlock_upgrade_and_lock_shared ();
        lock.unlock_shared ();
        EXPECT_EQ (0, lock.bits ());
    }

    {
        swift::RWTicketSpinLockT<32> lock;
        lock.lock ();
        ASSERT_TRUE (lock.try_lock_shared () == false);
        lock.unlock ();
        lock.lock_shared ();
        ASSERT_TRUE (lock.try_lock_shared () == true);
        lock.unlock_shared ();
        lock.unlock_shared ();
        lock.lock ();
        lock.unlock_and_lock_shared ();
        ASSERT_TRUE (lock.try_lock () == false);
    }
    {
        swift::RWTicketSpinLock64 lock;
        lock.lock ();
        ASSERT_TRUE (lock.try_lock_shared () == false);
        lock.unlock ();
        lock.lock_shared ();
        ASSERT_TRUE (lock.try_lock_shared () == true);
        lock.unlock_shared ();
        lock.unlock_shared ();
        lock.lock ();
        lock.unlock_and_lock_shared ();
        ASSERT_TRUE (lock.try_lock () == false);
    }
}


using namespace swift;

const static int kMaxReaders = 50;
static std::atomic<bool> kStopThread;

typedef testing::Types<RWSpinLock
#ifdef RW_SPINLOCK_USE_X86_INTRINSIC_
    , RWTicketSpinLockT<32, true>,
    RWTicketSpinLockT<32, false>,
    RWTicketSpinLockT<64, true>,
    RWTicketSpinLockT<64, false>
#endif
> Implementations;

TYPED_TEST_CASE (test_RWSpinLock, Implementations);

TYPED_TEST (test_RWSpinLock, Writer_Wait_Readers)
{
    typedef typename TestFixture::RWSpinLockType RWSpinLockType;
    RWSpinLockType l;

    for (int i = 0; i < kMaxReaders; ++i) {
        EXPECT_TRUE (l.try_lock_shared ());
        EXPECT_FALSE (l.try_lock ());
    }

    for (int i = 0; i < kMaxReaders; ++i) {
        EXPECT_FALSE (l.try_lock ());
        l.unlock_shared ();
    }

    EXPECT_TRUE (l.try_lock ());
}

TYPED_TEST (test_RWSpinLock, Read_Holders)
{
    typedef typename TestFixture::RWSpinLockType RWSpinLockType;
    RWSpinLockType l;

    {
        typename RWSpinLockType::ReadHolder guard (&l);
        EXPECT_FALSE (l.try_lock ());
        EXPECT_TRUE (l.try_lock_shared ());
        l.unlock_shared ();

        EXPECT_FALSE (l.try_lock ());
    }

    EXPECT_TRUE (l.try_lock ());
    l.unlock ();
}

TYPED_TEST (test_RWSpinLock, Write_Holders)
{
    typedef typename TestFixture::RWSpinLockType RWSpinLockType;
    RWSpinLockType l;
    {
        typename RWSpinLockType::WriteHolder guard (&l);
        EXPECT_FALSE (l.try_lock ());
        EXPECT_FALSE (l.try_lock_shared ());
    }

    EXPECT_TRUE (l.try_lock_shared ());
    EXPECT_FALSE (l.try_lock ());
    l.unlock_shared ();
    EXPECT_TRUE (l.try_lock ());
}

template <typename RWSpinLockType>
static void Run (RWSpinLockType* lock)
{
    int64_t reads = 0;
    int64_t writes = 0;
    while (!kStopThread.load (std::memory_order_acquire)) {
        if (rand () % 10 == 0) { // write
            typename RWSpinLockType::WriteHolder guard (lock);
            ++writes;
        }
        else { // read
            typename RWSpinLockType::ReadHolder guard (lock);
            ++reads;
        }
    }
}


TYPED_TEST (test_RWSpinLock, ConcurrentTests)
{
    typedef typename TestFixture::RWSpinLockType RWSpinLockType;
    RWSpinLockType l;
    srand (time (nullptr));

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.push_back (std::thread (&Run<RWSpinLockType>, &l));
    }

    sleep (1);
    kStopThread.store (true, std::memory_order_release);

    for (auto& t : threads) {
        t.join ();
    }
}

TEST (test_RWSpinLock, concurrent_holder_test) {
    srand (time (nullptr));

    RWSpinLock lock;
    std::atomic<int64_t> reads (0);
    std::atomic<int64_t> writes (0);
    std::atomic<int64_t> upgrades (0);
    std::atomic<bool> stop (false);

    auto go = [&] {
        while (!stop.load (std::memory_order_acquire)) {
            auto r = (uint32_t)(rand ()) % 10;
            if (r < 3) {          // starts from write lock
                RWSpinLock::ReadHolder rg{
                    RWSpinLock::UpgradedHolder{
                        RWSpinLock::WriteHolder{ &lock } } };
                writes.fetch_add (1, std::memory_order_acq_rel);;
            }
            else if (r < 6) {   // starts from upgrade lock
                RWSpinLock::UpgradedHolder ug (&lock);
                if (r < 4) {
                    RWSpinLock::WriteHolder wg (std::move (ug));
                }
                else {
                    RWSpinLock::ReadHolder rg (std::move (ug));
                }
                upgrades.fetch_add (1, std::memory_order_acq_rel);;
            }
            else {
                RWSpinLock::ReadHolder rg{ &lock };
                reads.fetch_add (1, std::memory_order_acq_rel);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.push_back (std::thread (go));
    }

    sleep (5);
    stop.store (true, std::memory_order_release);

    for (auto& t : threads) t.join ();
}
} // namespace