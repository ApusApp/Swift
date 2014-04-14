#ifndef __SWIFT_BASE_DATE_H__
#define __SWIFT_BASE_DATE_H__

#include <string>

struct tm;

namespace swift {
    
class Date
{
public:
    struct YearMonthDay
    {
        int year;   // [1900..2500]
        int month;  // [1..12]
        int day;    // [1..31]
    };

public:
    // Constructor an invalid Date.
    Date () : julianDayNumber_ (0) {}

    //
    // Constructor a YYYY-MM-DD Date.
    //
    // 1 <= month <= 12
    Date (int year, int month, int day);

    //
    // Constructor a Date from Julian Day Number.
    //
    explicit Date (int julianDayNum) : julianDayNumber_ (julianDayNum) {}

    //
    // Constructor a Date from struct tm
    //
    explicit Date (const struct tm&);

    inline void Swap (Date& that)
    {
        std::swap (julianDayNumber_, that.julianDayNumber_);
    }

    inline bool Valid () const 
    { 
        return julianDayNumber_ > 0; 
    }

    // Converts to YYYY-MM-DD format.
    std::string ToString () const;

    inline int Year () const
    {
        return GetYearMonthDay ().year;
    }

    inline int Month () const
    {
        return GetYearMonthDay ().month;
    }

    inline int Day () const
    {
        return GetYearMonthDay ().day;
    }

    // [0, 1, 2, 3, 4, 5, 6] => [Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday]
    inline int WeekDay () const
    {
        return (julianDayNumber_ + 1) % kDaysPerWeek;
    }

    inline int GetJulianDayNumber () const 
    { 
        return julianDayNumber_; 
    }

private:
    struct YearMonthDay GetYearMonthDay () const;

private:
    int julianDayNumber_;

private:
    static const int kDaysPerWeek = 7;

public:
    static const int kJulianDayOf1970_01_01;
};

inline bool operator< (const Date& lhs, const Date& rhs)
{
    return lhs.GetJulianDayNumber () < rhs.GetJulianDayNumber ();
}

inline bool operator== (const Date& lhs, const Date& rhs)
{
    return lhs.GetJulianDayNumber () == rhs.GetJulianDayNumber ();
}

} // end of namespace swift

#endif // __SWIFT_BASE_DATE_H__
