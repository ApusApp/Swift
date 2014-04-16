#include <swift/base/TimeZone.h>

#include <gtest/gtest.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

class test_TimeZone : public testing::Test
{
public:
    test_TimeZone () {}
    ~test_TimeZone () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

struct tm GetTime (int year, int month, int day,
                   int hour, int minute, int secondes)
{
    struct tm gmt;
    ::bzero (&gmt, sizeof(gmt));
    gmt.tm_year = year - 1900;
    gmt.tm_mon  = month - 1;
    gmt.tm_mday = day;
    gmt.tm_hour = hour;
    gmt.tm_min  = minute;
    gmt.tm_sec  = secondes;

    return gmt;
}

struct tm GetTime (const char* str)
{
    struct tm gmt;
    ::bzero (&gmt, sizeof(gmt));
    ::strptime (str, "%F %T", &gmt);
    return gmt;
}

TEST_F (test_TimeZone, All)
{
    swift::TimeZone z;
    ASSERT_TRUE (z.Valid () == false);
}