#include <gtest/gtest.h>
#include <openssl/md5.h>

#include <swift/base/md5.h>
#include <swift/base/stringpiece.h>

class test_MD5 : public testing::Test
{
public:
    test_MD5 () {}
    ~test_MD5 () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

// static public
std::string Md5 (const void* str, const size_t size)
{
    assert (0 != str && size > 0);
    unsigned char md[16];
    char tmp[3] = { '\0' };
    char buf[33] = { '\0' };

    MD5 (reinterpret_cast<const unsigned char*>(str), size, md);
    for (int i = 0; i < 16; ++i) {
        snprintf (tmp, sizeof (tmp), "%02x", md[i]);
        strcat (buf, tmp);
    }

    return std::string (buf, sizeof (buf)-1);
}

TEST_F (test_MD5, All)
{
    swift::MD5 md5;
    EXPECT_FALSE (md5.Valid ());

    std::string val = "abcdefghijklmnopqrstuvwxyz0123456789";
    md5.Update (val.data (), val.size ());
    md5.Final ();
    std::string md5_result = std::move (md5.ToString ());
    std::string result = std::move (Md5 (val.data (), val.size ()));
    std::string out;
    swift::MD5::Md5Sum (val.data (), val.size (), out);
    EXPECT_EQ (md5_result, out);
    EXPECT_EQ (md5_result, result);
    out.clear ();
    swift::MD5::Md5Sum (swift::StringPiece (val.data (), val.size ()), out);
    EXPECT_EQ (out, result);
    EXPECT_TRUE (md5.Valid ());

    md5.Reset ();
    EXPECT_FALSE (md5.Valid ());
    std::string str1 = "abcdefghijklmnopqrstuvwxyz";
    std::string str2 = "0123456789";
    md5.Update (nullptr, val.size ());
    EXPECT_FALSE (md5.Valid ());
    md5.Update (str1.data (), str1.size ());
    md5.Update (str2.data (), str2.size ());
    md5.Final ();
    EXPECT_EQ (md5.ToString (), result);

    swift::MD5 other;
    other.Update (val.c_str (), val.size ());
    other.Final ();
    ASSERT_TRUE (other == md5);
    ASSERT_FALSE (other != md5);

    other.Reset ();
    EXPECT_FALSE (other.Valid ());
    other.Update ("abc", 0);
    EXPECT_TRUE (other.ToString ().empty ());
    other.Final ();
    EXPECT_TRUE (other.ToString ().empty ());
}
