#include <type_traits> // for static_assert
#include <sys/time.h>
#include <stdio.h>

#include "swift/base/Timestamp.h"

namespace swift {

static_assert (sizeof(Timestamp) == sizeof(int64_t), "sizeof(Timestamp) must equal to sizeof(int64_t)");

Timestamp::Timestamp () : micro_seconds_since_epoch_ (0)
{
}

Timestamp::Timestamp (const Timestamp& rhs)
{
    if (this != &rhs) {
        micro_seconds_since_epoch_ = rhs.micro_seconds_since_epoch_;
    }
}

Timestamp::Timestamp (const int64_t micro_seconds_since_epoch) 
    : micro_seconds_since_epoch_ (micro_seconds_since_epoch)
{

}

Timestamp::~Timestamp ()
{

}

Timestamp& Timestamp::operator= (const Timestamp& rhs)
{
    if (this != &rhs) {
        micro_seconds_since_epoch_ = rhs.micro_seconds_since_epoch_;
    }

    return *this;
}

// static public
Timestamp Timestamp::Now ()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return Timestamp (tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

// static public
Timestamp Timestamp::Invalid ()
{
    return Timestamp ();
}

// public
std::string Timestamp::ToSecDotMicroString () const
{
    char buf[32] = {0};
    int64_t seconds = static_cast<int64_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
    int64_t microseconds = micro_seconds_since_epoch_ % kMicroSecondsPerSecond;
    // PRId64 GCC compile with -D__STDC_FORMAT_MACROS
    int size = snprintf (buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return std::string (buf, size);
}

// public 
std::string Timestamp::ToString () const
{
    char buf[32] = {0};
    int size = snprintf (buf, sizeof(buf) - 1, "%" PRId64"", micro_seconds_since_epoch_);
    return std::string (buf, size);
}

// public
std::string Timestamp::ToFormattedString () const
{
    char buf[32] = {0};
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(micro_seconds_since_epoch_ % kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    int size = snprintf (buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
        microseconds);
    return std::string (buf, size);
}

} // namespace swift
