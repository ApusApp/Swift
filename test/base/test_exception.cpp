#include <stdio.h>
#include <iostream>
#include <gtest/gtest.h>
#include <swift/base/exception.h>

class test_Exception : public testing::Test
{
public:
    test_Exception () {}
    ~test_Exception () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

#define STR_MSG "g_test_msg"
static std::string g_test_msg = STR_MSG;

class TestCase
{
public:
    void TestThrowNewException ()
    {
        throw new swift::Exception (std::move (g_test_msg));
    }
};

void TestFunc (int n)
{
    TestCase tc;
    try {
        tc.TestThrowNewException ();
    }
    catch (std::exception* e) {
        ASSERT_TRUE (STR_MSG == std::string (e->what ()));
        ASSERT_TRUE (g_test_msg.empty ());
        ASSERT_TRUE (!std::string ((dynamic_cast<swift::Exception*>(e))->GetStackTrace ()).empty ());
        std::cout << (dynamic_cast<swift::Exception*>(e))->GetStackTrace () << std::endl;
        delete e;
    }
}

TEST_F (test_Exception, All)
{
    {
        swift::Exception ex ("");
        ASSERT_TRUE (true == std::string (ex.what ()).empty ());
        ASSERT_TRUE (true == ex.Messge ().empty ());
        ASSERT_TRUE (false == std::string (ex.GetStackTrace ()).empty ());

        swift::Exception e (nullptr);
        ASSERT_TRUE (true == std::string (e.what ()).empty ());
        ASSERT_TRUE (true == e.Messge ().empty ());
        ASSERT_TRUE (false == std::string (e.GetStackTrace ()).empty ());
    }
    
    {
        swift::Exception ex ("abc");
        ASSERT_TRUE ("abc" == std::string (ex.what ()));
        ASSERT_TRUE (false == std::string (ex.GetStackTrace ()).empty ());
        swift::Exception tmp = ex;
        ASSERT_TRUE ("abc" == std::string (tmp.what ()));
    }

    {
        std::string msg = "abc";
        swift::Exception ex (msg);
        ASSERT_TRUE (msg == std::string (ex.what ()));
    }
    
    TestFunc (5);
}

