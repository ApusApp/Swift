/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
    explicit Timestamp (const int64_t micro_seconds_since_epoch);

    Timestamp (const Timestamp& rhs);
    Timestamp& operator= (const Timestamp& rhs);

    void Swap (Timestamp& that)
    {
        std::swap (micro_seconds_since_epoch_, that.micro_seconds_since_epoch_);
    }

    std::string ToSecDotMicroString () const;
    std::string ToString () const;

    std::string ToFormattedString () const;

    bool Valid () const { return micro_seconds_since_epoch_ > 0; }

    // for internal usage.
    int64_t MicroSecondsSinceEpoch () const 
    { 
        return micro_seconds_since_epoch_; 
    }

    time_t SecondsSinceEpoch () const 
    { 
        return static_cast<time_t> (micro_seconds_since_epoch_ / kMicroSecondsPerSecond); 
    }

    static Timestamp Now ();

    static Timestamp Invalid ();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t micro_seconds_since_epoch_;
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
