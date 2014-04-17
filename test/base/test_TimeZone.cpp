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

struct tm GetTime (int year, 
                   int month, 
                   int day,
                   int hour, 
                   int minute, 
                   int secondes)
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

time_t GetGmt (int year, 
               int month, 
               int day, 
               int hour, 
               int minu, 
               int sec)
{
    struct tm gmt = GetTime (year, month, day, hour, minu, sec);
    return timegm (&gmt);
}

time_t GetGmt (const char* str)
{
    struct tm gmt = GetTime (str);
    return timegm (&gmt);
}

struct TestCase
{
    const char* gmt;
    const char* local;
    bool isdst;
};

void Test (const swift::TimeZone& tz, const TestCase& tc)
{
    time_t gmt = GetGmt (tc.gmt);
    {
        struct tm local = tz.ToLocalTime (gmt);
        char buf[256] = {'0'};
        ::strftime (buf, sizeof(buf), "%F %T%z(%Z)", &local);
        if (::strcmp (buf, tc.local) != 0
            || tc.isdst != local.tm_isdst) {
            printf ("WRONG: ");
        }

        printf ("%s -> %s\n", tc.gmt, buf);
    }

    {
        struct tm local = GetTime (tc.local);
        local.tm_isdst = tc.isdst;
        time_t result = tz.FromLocalTime (local);
        if (result != gmt) {
            struct tm temp = tz.ToLocalTime (result);
            char buf[256] = {'0'};
            ::strftime (buf, sizeof(buf), "%F %T%z(%Z)", &temp);

            printf ("WRONG FromLocalTime: %ld %ld %s\n",
                    static_cast<long>(gmt),
                    static_cast<long>(result),
                    buf);
        }
    }
}

void TestNewYork ()
{
    swift::TimeZone tz ("/usr/share/zoneinfo/America/New_York");
    TestCase cases[] = {
        { "2006-03-07 00:00:00", "2006-03-06 19:00:00-0500(EST)", false },
        { "2006-04-02 06:59:59", "2006-04-02 01:59:59-0500(EST)", false },
        { "2006-04-02 07:00:00", "2006-04-02 03:00:00-0400(EDT)", true  },
        { "2006-05-01 00:00:00", "2006-04-30 20:00:00-0400(EDT)", true  },
        { "2006-05-02 01:00:00", "2006-05-01 21:00:00-0400(EDT)", true  },
        { "2006-10-21 05:00:00", "2006-10-21 01:00:00-0400(EDT)", true  },
        { "2006-10-29 05:59:59", "2006-10-29 01:59:59-0400(EDT)", true  },
        { "2006-10-29 06:00:00", "2006-10-29 01:00:00-0500(EST)", false },
        { "2006-10-29 06:59:59", "2006-10-29 01:59:59-0500(EST)", false },
        { "2006-12-31 06:00:00", "2006-12-31 01:00:00-0500(EST)", false },
        { "2007-01-01 00:00:00", "2006-12-31 19:00:00-0500(EST)", false },

        { "2007-03-07 00:00:00", "2007-03-06 19:00:00-0500(EST)", false },
        { "2007-03-11 06:59:59", "2007-03-11 01:59:59-0500(EST)", false },
        { "2007-03-11 07:00:00", "2007-03-11 03:00:00-0400(EDT)", true  },
        { "2007-05-01 00:00:00", "2007-04-30 20:00:00-0400(EDT)", true  },
        { "2007-05-02 01:00:00", "2007-05-01 21:00:00-0400(EDT)", true  },
        { "2007-10-31 05:00:00", "2007-10-31 01:00:00-0400(EDT)", true  },
        { "2007-11-04 05:59:59", "2007-11-04 01:59:59-0400(EDT)", true  },
        { "2007-11-04 06:00:00", "2007-11-04 01:00:00-0500(EST)", false },
        { "2007-11-04 06:59:59", "2007-11-04 01:59:59-0500(EST)", false },
        { "2007-12-31 06:00:00", "2007-12-31 01:00:00-0500(EST)", false },
        { "2008-01-01 00:00:00", "2007-12-31 19:00:00-0500(EST)", false },

        { "2009-03-07 00:00:00", "2009-03-06 19:00:00-0500(EST)", false },
        { "2009-03-08 06:59:59", "2009-03-08 01:59:59-0500(EST)", false },
        { "2009-03-08 07:00:00", "2009-03-08 03:00:00-0400(EDT)", true  },
        { "2009-05-01 00:00:00", "2009-04-30 20:00:00-0400(EDT)", true  },
        { "2009-05-02 01:00:00", "2009-05-01 21:00:00-0400(EDT)", true  },
        { "2009-10-31 05:00:00", "2009-10-31 01:00:00-0400(EDT)", true  },
        { "2009-11-01 05:59:59", "2009-11-01 01:59:59-0400(EDT)", true  },
        { "2009-11-01 06:00:00", "2009-11-01 01:00:00-0500(EST)", false },
        { "2009-11-01 06:59:59", "2009-11-01 01:59:59-0500(EST)", false },
        { "2009-12-31 06:00:00", "2009-12-31 01:00:00-0500(EST)", false },
        { "2010-01-01 00:00:00", "2009-12-31 19:00:00-0500(EST)", false },

        { "2010-03-13 00:00:00", "2010-03-12 19:00:00-0500(EST)", false },
        { "2010-03-14 06:59:59", "2010-03-14 01:59:59-0500(EST)", false },
        { "2010-03-14 07:00:00", "2010-03-14 03:00:00-0400(EDT)", true  },
        { "2010-05-01 00:00:00", "2010-04-30 20:00:00-0400(EDT)", true  },
        { "2010-05-02 01:00:00", "2010-05-01 21:00:00-0400(EDT)", true  },
        { "2010-11-06 05:00:00", "2010-11-06 01:00:00-0400(EDT)", true  },
        { "2010-11-07 05:59:59", "2010-11-07 01:59:59-0400(EDT)", true  },
        { "2010-11-07 06:00:00", "2010-11-07 01:00:00-0500(EST)", false },
        { "2010-11-07 06:59:59", "2010-11-07 01:59:59-0500(EST)", false },
        { "2010-12-31 06:00:00", "2010-12-31 01:00:00-0500(EST)", false },
        { "2011-01-01 00:00:00", "2010-12-31 19:00:00-0500(EST)", false },

        { "2011-03-01 00:00:00", "2011-02-28 19:00:00-0500(EST)", false },
        { "2011-03-13 06:59:59", "2011-03-13 01:59:59-0500(EST)", false },
        { "2011-03-13 07:00:00", "2011-03-13 03:00:00-0400(EDT)", true  },
        { "2011-05-01 00:00:00", "2011-04-30 20:00:00-0400(EDT)", true  },
        { "2011-05-02 01:00:00", "2011-05-01 21:00:00-0400(EDT)", true  },
        { "2011-11-06 05:59:59", "2011-11-06 01:59:59-0400(EDT)", true  },
        { "2011-11-06 06:00:00", "2011-11-06 01:00:00-0500(EST)", false },
        { "2011-11-06 06:59:59", "2011-11-06 01:59:59-0500(EST)", false },
        { "2011-12-31 06:00:00", "2011-12-31 01:00:00-0500(EST)", false },
        { "2012-01-01 00:00:00", "2011-12-31 19:00:00-0500(EST)", false },

    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        Test (tz, cases[i]);
    }
}


TEST_F (test_TimeZone, All)
{
    swift::TimeZone z;
    ASSERT_TRUE (z.Valid () == false);

    TestNewYork ();
}