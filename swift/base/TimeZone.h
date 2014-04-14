#ifndef __SWIFT_BASE_TIME_ZONE_H__
#define __SWIFT_BASE_TIME_ZONE_H__

#include <memory>
#include <time.h>

namespace swift {

// TimeZone for 1970~2030
class TimeZone
{
public:
    explicit TimeZone (const char* zonefile);
    TimeZone (int eastOfUtc, const char* tzname);  // a fixed timezone
    TimeZone () {}  // an invalid timezone
    bool Valid () const
    {
        // 'explicit operator bool() const' in C++11
        return static_cast<bool>(data_);
    }

    struct tm ToLocalTime (time_t secondsSinceEpoch) const;
    time_t FromLocalTime (const struct tm& t) const;

    // gmtime(3)
    static struct tm ToUtcTime (time_t secondsSinceEpoch, bool yday = false);

    // timegm(3)
    static time_t FromUtcTime (const struct tm& t);

    // year in [1900..2500], month in [1..12], day in [1..31]
    static time_t FromUtcTime (int year, int month, int day,
                                int hour, int minute, int seconds);

    struct Data;

private:
    std::shared_ptr<Data> data_;
}; // end of TimeZone

} // end of namespace swift

#endif // __SWIFT_BASE_TIME_ZONE_H__