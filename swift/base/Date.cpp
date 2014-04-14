#include <swift/base/Date.h>
#include <stdio.h> // snprintf
#include <time.h>

namespace swift {
namespace detail {

char require_32_bit_integer_at_least[sizeof(int) >= sizeof(int32_t) ? 1 : -1];

// algorithm and explanation see:
// http://www.faqs.org/faqs/calendars/faq/part2/
// http://blog.csdn.net/Solstice

int CalculateJulianDayNumber (int year, int month, int day)
{
    (void) require_32_bit_integer_at_least; // no warning please
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
		
    return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
}

struct Date::YearMonthDay CalculateYearMonthDay (int julianDayNumber)
{
    int a = julianDayNumber + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - ((b * 146097) / 4);
    int d = (4 * c + 3) / 1461;
    int e = c - ((1461 * d) / 4);
    int m = (5 * e + 2) / 153;
    Date::YearMonthDay ymd;
    ymd.day = e - ((153 * m + 2) / 5) + 1;
    ymd.month = m + 3 - 12 * (m / 10);
    ymd.year = b * 100 + d - 4800 + (m / 10);
    return ymd;
}

} // namespace detail

// public
const int Date::kJulianDayOf1970_01_01 = detail::CalculateJulianDayNumber (1970, 1, 1);

// public
Date::Date (int y, int m, int d) : julianDayNumber_ (detail::CalculateJulianDayNumber (y, m, d))
{
}

// public
Date::Date (const struct tm& t)
    : julianDayNumber_ (detail::CalculateJulianDayNumber (
    t.tm_year + 1900,
    t.tm_mon + 1,
    t.tm_mday))
{
}

// private
struct Date::YearMonthDay Date::GetYearMonthDay () const
{
    return detail::CalculateYearMonthDay (julianDayNumber_);
}

// public
std::string Date::ToString () const
{
    char buf[16] = {'\0'};
    struct YearMonthDay ymd (GetYearMonthDay ());
    int size = snprintf (buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
    return std::string (buf, size);
}

} // namespace swift
