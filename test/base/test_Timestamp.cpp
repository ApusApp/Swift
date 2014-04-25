#include <gtest/gtest.h>

#include <swift/base/Timestamp.h>

class test_Timestamp : public testing::Test
{
public:
    test_Timestamp () {}
    ~test_Timestamp () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_Timestamp, All)
{
    swift::Timestamp ts;
    ASSERT_FALSE (ts.Valid ());

    swift::Timestamp now = swift::Timestamp::Now ();
    ASSERT_TRUE (now.Valid ());

    now.Swap (ts);
    ASSERT_FALSE (now.Valid ());
    ASSERT_TRUE (ts.Valid ());

    swift::Timestamp tt (ts.MicroSecondsSinceEpoch ());
    ASSERT_TRUE (tt == ts);
    ASSERT_TRUE (ts.ToString () == tt.ToString ());
    ASSERT_TRUE (ts.ToFormattedString () == tt.ToFormattedString ());
    ASSERT_TRUE (ts.ToFormattedString (false) == tt.ToFormattedString (false));
    ASSERT_TRUE (ts.ToSecDotMicroString () == tt.ToSecDotMicroString ());

    ASSERT_FALSE (ts < tt);
    ASSERT_TRUE (0 == swift::TimeDifference (ts, tt));

    swift::Timestamp add_time = swift::Timestamp::Invalid ();
    add_time = swift::AddTime (ts, 1000.0);
    ASSERT_FALSE (add_time < tt);
    ASSERT_TRUE (add_time > tt);
    ASSERT_TRUE (1000.0 == swift::TimeDifference (add_time, tt));
}