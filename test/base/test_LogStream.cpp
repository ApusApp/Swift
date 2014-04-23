#include <iostream>
#include <limits>
#include <stdlib.h>
#include <gtest/gtest.h>

#include <swift/base/LogStream.h>

class test_LogStream : public testing::Test
{
public:
    test_LogStream () {}
    ~test_LogStream () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_LogStream, Booleans)
{
    swift::LogStream os;
    const swift::LogStream::BufferType& buf = os.Buffer ();
    ASSERT_TRUE (buf.ToString () == "");
    os << false;
    ASSERT_TRUE (buf.ToString () == "0");
    os << true;
    ASSERT_TRUE (buf.ToString () == "01");
    os << '\n';
    ASSERT_TRUE (buf.ToString () == "01\n");
    os.ResetBuffer ();
    os << true << false << '\n' << -1;
    ASSERT_TRUE (buf.ToString () == "10\n-1");
}

TEST_F (test_LogStream, Integer)
{
    swift::LogStream os;
    const swift::LogStream::BufferType& buf = os.Buffer ();
    int n = std::numeric_limits<int>::max ();
    os << n;
    char tmp[32] = {'\0'};
    int size = snprintf (tmp, sizeof(tmp), "%d", n);
    ASSERT_TRUE (buf.ToString () == std::string (tmp, size));
}