#include <iostream>
#include <limits>
#include <stdlib.h>
#include <inttypes.h>
#include <gtest/gtest.h>

#include <swift/base/experimental/logstream.h>

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
    os << std::numeric_limits<int>::max ();
    char tmp[32] = {'\0'};
    snprintf (tmp, sizeof(tmp), "%d", std::numeric_limits<int>::max ());
    ASSERT_TRUE (buf.ToString () == std::string (tmp));
    os.ResetBuffer ();
    ASSERT_TRUE (buf.ToString () == "");
    os << -12306 << +12306;
    ASSERT_TRUE (buf.ToString () == std::string ("-1230612306"));
    os << 0 << -1 << 1 << 0 << 1;
    ASSERT_TRUE (buf.ToString () == std::string ("-12306123060-1101"));
    os.ResetBuffer ();
    int n = 0;
    os << n << n << n;
    ASSERT_TRUE (buf.ToString () == std::string ("000"));
    os.ResetBuffer ();
    os << std::numeric_limits<int64_t>::max ();
    snprintf (tmp, sizeof(tmp), "%" PRId64, std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (buf.ToString () == std::string (tmp));

    os.ResetBuffer ();
    os << std::numeric_limits<uint64_t>::max ();
    snprintf (tmp, sizeof(tmp), "%" PRIu64, std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (buf.ToString () == std::string (tmp));

    os.ResetBuffer ();
    os << std::numeric_limits<uint64_t>::min ();
    snprintf (tmp, sizeof(tmp), "%" PRIu64, std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (buf.ToString () == std::string (tmp));
}

TEST_F (test_LogStream, Float)
{
    swift::LogStream os;
    const swift::LogStream::BufferType& buf = os.Buffer ();
    os << 3.14159267;
    ASSERT_TRUE (buf.ToString () == std::string ("3.14159267"));
    os << -3.14159267;
    ASSERT_TRUE (buf.ToString () == std::string ("3.14159267-3.14159267"));
    os.ResetBuffer ();
    os << 0.0000;
    ASSERT_TRUE (buf.ToString () == std::string ("0"));
    os << -0.10;
    ASSERT_TRUE (buf.ToString () == std::string ("0-0.1"));
    os << 0.0005;
    ASSERT_TRUE (buf.ToString () == std::string ("0-0.10.0005"));
    os << 1234.567;
    ASSERT_TRUE (buf.ToString () == std::string ("0-0.10.00051234.567"));
}

TEST_F (test_LogStream, VoidAndString)
{
    swift::LogStream os;
    const swift::LogStream::BufferType& buf = os.Buffer ();

    os << static_cast<void*>(0);
    ASSERT_TRUE (buf.Length () == 0);

    void* p = nullptr;
    os << p;
    ASSERT_TRUE (buf.Length () == 0);

    os << reinterpret_cast<void*>(9999);
    ASSERT_TRUE (buf.ToString () == std::string ("0x270F"));
    os.ResetBuffer ();

    os << "Hello World!";
    ASSERT_TRUE (buf.ToString () == std::string ("Hello World!"));

    os << "";
    ASSERT_TRUE (buf.ToString () == std::string ("Hello World!"));
    os.ResetBuffer ();

    std::string str = "apusapp.com";
    os.Append (str.c_str (), str.size ());
    ASSERT_TRUE (buf.ToString () == str);

    char *ptr = nullptr;
    os << ptr;
    ASSERT_TRUE (buf.ToString () == str);

    unsigned char* uptr = nullptr;
    os << uptr;
    ASSERT_TRUE (buf.ToString () == str);
}

TEST_F (test_LogStream, Format)
{
    swift::LogStream os;
    const swift::LogStream::BufferType& buf = os.Buffer ();

    os << swift::Format ("%04d", 1);
    ASSERT_TRUE (buf.ToString () == std::string ("0001"));
    os.ResetBuffer ();

    os << swift::Format ("%4.3f", 123.06);
    ASSERT_TRUE (buf.ToString () == std::string ("123.060"));
    os.ResetBuffer ();

    os << swift::Format ("%2.1f", 12.306) << swift::Format ("%d", 10001);
    ASSERT_TRUE (buf.ToString () == std::string ("12.310001"));
}

TEST_F (test_LogStream, LongString)
{
    swift::LogStream os;
    const swift::LogStream::BufferType& buf = os.Buffer ();

    std::string v = "123456789 ";
    int size = static_cast<int>(swift::detail::kSmallBuffer / v.size ());
    for (int i = 0; i < size; ++i) {
        os << v;
        ASSERT_TRUE (buf.Length () == static_cast<int>(v.size () * (i + 1)));
        ASSERT_TRUE (buf.AvailSize () == swift::detail::kSmallBuffer - static_cast<int>(v.size () * (i + 1)));
    }

    int left = static_cast<int>(swift::detail::kSmallBuffer % v.size ());
    ASSERT_TRUE (buf.AvailSize () == left);
    ASSERT_TRUE (buf.AvailSize () == 6);

    os << "abcde";
    ASSERT_TRUE (buf.AvailSize () == 1);

    os << "ss";
    ASSERT_TRUE (buf.AvailSize () == 1);

    os << "q";
    ASSERT_TRUE (buf.AvailSize () == 1);
}