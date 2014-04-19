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

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

#include "swift/base/TimeZone.h"
#include "swift/base/Date.h"
#include "swift/base/noncopyable.hpp"

namespace swift {
namespace detail {

struct Transition 
{
    Transition (time_t gmt,
                time_t local,
                int index) 
        : gmt_time_ (gmt)
        , localtime_ (local)
        , localtime_index_ (index) 
    {
    } 

    time_t gmt_time_;
    time_t localtime_;
    int localtime_index_;
};

struct Compare
{
    Compare (bool gmt) : compare_gmt_ (gmt) {}
    ~Compare () {}

    bool operator () (const Transition& lhs, const Transition& rhs) const
    {
        return compare_gmt_ ? (lhs.gmt_time_ < rhs.gmt_time_)
                            : (lhs.localtime_ < rhs.localtime_);
    }

    bool Equal (const Transition& lhs, const Transition& rhs) const
    {
        return compare_gmt_ ? (lhs.gmt_time_ == rhs.gmt_time_) 
                            : (lhs.localtime_ == rhs.localtime_);
    }

    bool compare_gmt_;
};

struct Localtime
{
    Localtime (time_t off_set, 
               bool dst, 
               int arrb)
        : gmt_off_set_ (off_set)
        , is_dst_ (dst)
        , arrb_index_ (arrb)
    {
    }

    time_t gmt_off_set_;
    bool is_dst_;
    int arrb_index_;
};

inline void FillHMS (unsigned int seconds, struct tm* utc)
{
    utc->tm_sec = seconds % 60;
    unsigned int minutes = static_cast<unsigned int>(seconds / 60);
    utc->tm_min = minutes % 60;
    utc->tm_hour = static_cast<int>(minutes / 60);
}

class File : swift::noncopyable
{
public:
    File (const char* filn_name) : fp_ (::fopen (filn_name, "rb")) {}
    ~File ()
    {
        if (fp_) {
            ::fclose (fp_);
            fp_ = nullptr;
        }
    }

    bool Valid () const
    {
        return nullptr != fp_;
    }

    std::string ReadBytes (int n)
    {
        char buf[n];
        ssize_t bytes = ::fread (buf, 1, n, fp_);
        if (n != static_cast<int>(bytes)) {
            throw std::logic_error ("no enough data to read");
        }

        return std::string (buf, n);
    }

    int32_t ReadInt32 ()
    {
        int32_t ret = 0;
        ssize_t bytes = ::fread (&ret, 1, sizeof(int32_t), fp_);
        if (sizeof(int32_t) != static_cast<int>(bytes)) {
            throw std::logic_error ("bad read int32_t data");
        }

        return static_cast<int32_t>(::be32toh (ret));
    }

    uint8_t ReadUInt8 ()
    {
        uint8_t ret = 0;
        ssize_t bytes = ::fread (&ret, 1, sizeof(uint8_t), fp_);
        if (sizeof(uint8_t) != static_cast<int>(bytes)) {
            throw std::logic_error ("bad read uint8_t data");
        }

        return ret;
    }

private:
    FILE* fp_;
}; // class File

} // namespace detail

const int kSecondsPerDay = 24 * 60 * 60;

struct TimeZone::Data 
{
    std::vector<detail::Transition> transitions_;
    std::vector<detail::Localtime> localtimes_;
    std::vector<std::string> names_;
    std::string abbreviation_;
};

namespace detail {

// zone file in the directory of /usr/share/zoneinfo/ etc. localtime
// localtime -> /etc/localtime
bool ReadTimeZoneFile (const char* zone_file, struct TimeZone::Data* data)
{
    File f (zone_file);
    if (f.Valid ()) {
        try {
            // read head
            std::string head = f.ReadBytes (4);
            if ("TZif" != head) {
                throw std::logic_error ("bad time zone file");
            }
            // read version
            f.ReadBytes (1);
            f.ReadBytes (15);

            int32_t isgmtcnt = f.ReadInt32 ();
            int32_t isstdcnt = f.ReadInt32 ();
            int32_t leapcnt = f.ReadInt32 ();
            int32_t timecnt = f.ReadInt32 ();
            int32_t typecnt = f.ReadInt32 ();
            int32_t charcnt = f.ReadInt32 ();

            std::vector<int32_t> trans;
            std::vector<int> localtimes;
            trans.reserve (timecnt);
            for (int i = 0; i < timecnt; ++i) {
                trans.push_back (f.ReadInt32 ());
            }

            for (int i = 0; i < timecnt; ++i) {
                uint8_t local = f.ReadUInt8 ();
                localtimes.push_back (local);
            }

            for (int i = 0; i < typecnt; ++i) {
                int32_t gmtoff = f.ReadInt32 ();
                uint8_t isdst = f.ReadUInt8 ();
                uint8_t abbrind = f.ReadUInt8 ();

                data->localtimes_.push_back (Localtime (gmtoff, isdst, abbrind));
            }

            for (int i = 0; i < timecnt; ++i) {
                int local_idx = localtimes[i];
                time_t localtime = trans[i] + data->localtimes_[local_idx].gmt_off_set_;
                data->transitions_.push_back (Transition (trans[i], localtime, local_idx));
            }
            
            data->abbreviation_ = f.ReadBytes (charcnt);

            // leapcnt
            //for (int i = 0; i < leapcnt; ++i) {
                // int32_t leaptime = f.ReadInt32 ();
                // int32_t cumleap = f.ReadInt32 ();
            //}

            // FIXME
            (void) leapcnt;
            (void) isstdcnt;
            (void) isgmtcnt;
        }
        catch (std::logic_error& e) {
            fprintf (stderr, "%s\n", e.what ());
        }
    }

    return true;
} // ReadTimeZoneFile

const Localtime* FindLocalTime (const TimeZone::Data& data,
                                const Transition& trans,
                                const Compare& comp)
{
    const Localtime* localtime = nullptr;
    if (data.transitions_.empty () || comp (trans, data.transitions_.front ())) {
        // FIXME: should be first non dst time zone
        localtime = &data.localtimes_.front ();
    }
    else {
        // lower_bound: 
        // Returns an iterator pointing to the first element in the range 
        // [data.transitions.begin (), data.transitions.end ()) which does not compare less than trans.
        std::vector<Transition>::const_iterator transI = std::lower_bound (data.transitions_.begin (),
                                                                           data.transitions_.end (),
                                                                           trans,
                                                                           comp);
        if (transI != data.transitions_.end ()) {
            if (!comp.Equal (trans, *transI)) {
                assert (transI != data.transitions_.begin ());
                --transI;
            }

            localtime = &data.localtimes_[transI->localtime_index_];
        }
        else {
            // FIXME: use TZ-env
            localtime = &data.localtimes_[data.transitions_.back ().localtime_index_];
        }
    }

    return localtime;
} // FindLocalTime

} // namespace detail

// public
TimeZone::TimeZone (const char* zone_file) 
    : data_ (std::make_shared<TimeZone::Data> ())
{
    if (!detail::ReadTimeZoneFile (zone_file, data_.get ())) {
        data_.reset ();
    }
}

// public
TimeZone::TimeZone (int east_of_utc, const char* tzname)
    : data_ (std::make_shared<TimeZone::Data> ())
{
    data_->localtimes_.push_back (detail::Localtime (east_of_utc, false, 0));
    data_->abbreviation_ = std::string (tzname);
}

// public
struct tm TimeZone::ToLocalTime (time_t seconds_since_epoch) const
{
    struct tm localtime;
    ::bzero (&localtime, sizeof (localtime));
    assert (nullptr != data_.get ());
    const Data& data (*data_);
    detail::Transition trans (seconds_since_epoch, 0, 0);

    const detail::Localtime* local = detail::FindLocalTime (data, trans, detail::Compare (true));
    if (local) {
        time_t local_seconds = seconds_since_epoch + local->gmt_off_set_;
        ::gmtime_r (&local_seconds, &localtime);
        localtime.tm_isdst = local->is_dst_;
        localtime.tm_gmtoff = local->gmt_off_set_;
        localtime.tm_zone = &data_->abbreviation_[local->arrb_index_];
    }

    return localtime;
}

// public
time_t TimeZone::FromLocalTime (const struct tm& t) const
{
    assert (nullptr != data_.get ());
    struct tm localtime = t;
    const Data& data (*data_);
    time_t seconds = ::timegm (&localtime);
    detail::Transition tran (0, seconds, 0);

    const detail::Localtime* local = detail::FindLocalTime (data, tran, detail::Compare (false));
    if (t.tm_isdst) {
        struct tm try_time = ToLocalTime (seconds - local->gmt_off_set_);
        if (!try_time.tm_isdst
            && try_time.tm_hour == localtime.tm_hour
            && try_time.tm_min == localtime.tm_min) {
            seconds -= 3600;
        }
    }

    return seconds - local->gmt_off_set_;
}

// public
time_t TimeZone::FromUtcTime (int year, 
                              int month, 
                              int day, 
                              int hour, 
                              int minute, 
                              int seconds)
{
    Date date (year, month, day);
    int seconds_in_day = hour * 3600 + minute * 60 + seconds;
    time_t days = date.GetJulianDayNumber () - Date::kJulianDayOf1970_01_01;

    return days * kSecondsPerDay + seconds_in_day;
}

// public
time_t TimeZone::FromUtcTime (const struct tm& t)
{
    return FromUtcTime (t.tm_year + 1900,
                        t.tm_mon + 1,
                        t.tm_yday,
                        t.tm_hour,
                        t.tm_min,
                        t.tm_sec);
}

// static public
struct tm TimeZone::ToUtcTime (time_t seconds_since_epoch, bool yday /*= false*/)
{
    struct tm utc;
    ::bzero (&utc, sizeof(utc));
    utc.tm_zone = "GMT";
    int seconds = static_cast<int>(seconds_since_epoch % kSecondsPerDay);
    int days = static_cast<int>(seconds_since_epoch / kSecondsPerDay);
    if (seconds < 0) {
        seconds += kSecondsPerDay;
        --days;
    }

    detail::FillHMS (seconds, &utc);
    Date date (days + Date::kJulianDayOf1970_01_01);
    Date::YearMonthDay ymd = date.GetYearMonthDay ();
    utc.tm_year = ymd.year - 1900;
    utc.tm_mon  = ymd.month - 1;
    utc.tm_mday = ymd.day;
    utc.tm_wday = date.WeekDay ();

    if (yday) {
        Date start_of_year (ymd.year, 1, 1);
        utc.tm_yday = date.GetJulianDayNumber () - start_of_year.GetJulianDayNumber ();
    }

    return utc;
}

} // namespace swift
