#include <gtest/gtest.h>
#include <swift/base/rwspinlock.h>

class test_RWSpinLock : public testing::Test
{
public:
    test_RWSpinLock () {}
    ~test_RWSpinLock () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_RWSpinLock, RWSpinLock)
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