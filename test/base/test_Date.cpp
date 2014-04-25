#include <stdlib.h>
#include <time.h>
#include <gtest/gtest.h>
#include <iostream>
#include <swift/base/Date.h>

class test_Date : public testing::Test
{
public:
    test_Date () {}
    ~test_Date () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_Date, All)
{
    swift::Date date;
    ASSERT_FALSE (date.Valid ());

    swift::Date::YearMonthDay ymd;
    ymd.year = 2014;
    ymd.month = 4;
    ymd.day = 25;

    swift::Date dt (ymd.year, ymd.month, ymd.day);
    date.Swap (dt);
    ASSERT_TRUE (date.Valid ());
    ASSERT_FALSE (dt.Valid ());

    ASSERT_TRUE (2014 == date.Year ());
    ASSERT_TRUE (4 == date.Month ());
    ASSERT_TRUE (25 == date.Day ());
    // Friday == 5
    ASSERT_TRUE (5 == date.WeekDay ());

    struct tm stm;
    time_t tt = time (nullptr);
    ::gmtime_r (&tt, &stm);
    dt = swift::Date (stm);
    ASSERT_TRUE (dt.Year () == date.Year ());
    ASSERT_TRUE (dt.Month () == date.Month ());
    ASSERT_TRUE (dt.Day () == date.Day ());
    ASSERT_TRUE (dt == date);
    ASSERT_TRUE (swift::Date (2014, 4, 24) < date);
    swift::Date::YearMonthDay ymd_tmp = dt.GetYearMonthDay ();
    ASSERT_TRUE (ymd_tmp.year == ymd.year);
    ASSERT_TRUE (ymd_tmp.month == ymd.month);
    ASSERT_TRUE (ymd_tmp.day == ymd.day);
    ASSERT_TRUE (date.ToString () == "2014-04-25");

    swift::Date dtt (date.GetJulianDayNumber ());
    ASSERT_TRUE (dtt == date);
    ASSERT_TRUE (dtt.Year () == date.Year ());
    ASSERT_TRUE (dtt.Month () == date.Month ());
    ASSERT_TRUE (dtt.Day () == date.Day ());
}