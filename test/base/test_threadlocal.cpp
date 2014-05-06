#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <swift/base/threadlocal.h>
#include <swift/base/thisthread.h>
#include <swift/base/noncopyable.hpp>

class test_ThreadLocal : public testing::Test
{
public:
    test_ThreadLocal () {}
    ~test_ThreadLocal () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

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

    const std::string& GetName () const
    {
        return name_;
    }

    void SetName (const std::string& name)
    {
        name_ = name;
    }

private:
    std::string name_;
};

swift::ThreadLocal<LocalData> tls_d1;
swift::ThreadLocal<LocalData> tls_d2;

static inline void Dsiplay ()
{
    std::cout << "Thread id = " << swift::thisthread::GetTid ()
              << "\ttls_d1 " << tls_d1.Get ()
              << "\ttls_d1 name " << tls_d1.Get ()->GetName () << std::endl;
    std::cout << "Thread id = " << swift::thisthread::GetTid ()
              << "\ttls_d2 " << tls_d2.Get ()
              << "\ttls_d2 name " << tls_d2.Get ()->GetName () << std::endl;
}

void TestFunc ()
{
    std::cout << "Before..." << std::endl;
    Dsiplay ();
    tls_d1.Get ()->SetName ("son_thread_tls_d1");
    tls_d2.Get ()->SetName ("son_thread_tls_d2");
    std::cout << "After..." << std::endl;
    Dsiplay ();
}

TEST_F (test_ThreadLocal, All)
{
    std::cout << "Main one..." << std::endl;
    tls_d1->SetName ("main_thread_tls_d1");
    Dsiplay ();

    std::thread t1 (TestFunc);
    t1.join ();

    (*tls_d2).SetName ("main_thread_tls_d2");
    Dsiplay ();

    std::cout << "Main two..." << std::endl;
    std::thread t2 (TestFunc);
    t2.join ();

    std::cout << "Main Reset..." << std::endl;
    LocalData *d = new LocalData;
    d->SetName ("New Data");
    tls_d2.Reset (d);
    Dsiplay ();

    tls_d2.Reset (nullptr);
    tls_d2.Reset (nullptr);
    ASSERT_TRUE (tls_d2->GetName ().empty ());
}