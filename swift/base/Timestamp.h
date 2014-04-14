#ifndef __SWIFT_BASE_TIMESTAMP_H__
#define __SWIFT_BASE_TIMESTAMP_H__

#include <inttypes.h>
#include <string>
#include <boost/operators.hpp>

namespace swift {

class Timestamp : boost::less_than_comparable<Timestamp>
{
public:
    Timestamp ();
    ~Timestamp ();
    explicit Timestamp (const int64_t microSecondsSinceEpoch);

    Timestamp (const Timestamp& rhs);
    Timestamp& operator= (const Timestamp& rhs);

    void Swap (Timestamp& that)
    {
        std::swap (microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    std::string ToSecDotMicroString () const;
    std::string ToString () const;

    std::string ToFormattedString () const;

    bool Valid () const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t MicroSecondsSinceEpoch () const 
    { 
        return microSecondsSinceEpoch_; 
    }

    time_t SecondsSinceEpoch () const 
    { 
        return static_cast<time_t> (microSecondsSinceEpoch_ / kMicroSecondsPerSecond); 
    }

    static Timestamp Now ();

    static Timestamp Invalid ();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator< (const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.MicroSecondsSinceEpoch () < rhs.MicroSecondsSinceEpoch ();
}

inline bool operator== (const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.MicroSecondsSinceEpoch () == rhs.MicroSecondsSinceEpoch ();
}

/**
 * Gets time difference of two timestamps, result in seconds.
 *
 * @param high, low
 * @return (high-low) in seconds
 * @c double has 52-bit precision, enough for one-microsecond
 * resolution for next 100 years.
 */
inline double TimeDifference (const Timestamp& high, const Timestamp& low)
{
    int64_t diff = high.MicroSecondsSinceEpoch () - low.MicroSecondsSinceEpoch ();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

/**
 * Add @c seconds to given timestamp.
 *
 * @return timestamp+seconds as Timestamp
 */
inline Timestamp AddTime (const Timestamp& timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp (timestamp.MicroSecondsSinceEpoch () + delta);
}

} // end of namespace swift

#endif // __SWIFT_BASE_TIMESTAMP_H__
