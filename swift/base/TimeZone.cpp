#include <swift/base/TimeZone.h>
#include <swift/base/Date.h>
#include <swift/base/noncopyable.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

namespace swift {
namespace detail {

struct Transition 
{
	Transition (time_t gmt,
				time_t local,
				int index) 
		: gmtTime (gmt)
		, localtime (local)
		, localtimeIndex (index) 
	{
	} 

	time_t gmtTime;
	time_t localtime;
	int localtimeIndex;
};

struct Compare
{
	Compare (bool gmt) : compareGmt (gmt) {}

	bool operator() (const Transition& lhs, const Transition& rhs) const
	{
		if (compareGmt) {
			return lhs.gmtTime < rhs.gmtTime;
		}
		else {
			return lhs.localtime < rhs.localtime;
		}
	}

	bool Equal (const Transition& lhs, const Transition& rhs) const
	{
		if (compareGmt) {
			return lhs.gmtTime == rhs.gmtTime;
		}
		else {
			return lhs.localtime == rhs.localtime;
		}
	}

	bool compareGmt;
};

struct Localtime
{
	Localtime (time_t offset, 
				bool dst, 
				int arrb)
		: gmtOffset (offset)
		, isDst (dst)
		, arrbIndex (arrb)
	{
	}

	time_t gmtOffset;
	bool isDst;
	int arrbIndex;
};

inline void FillHMS (unsigned int seconds, struct tm* utc)
{
	utc->tm_sec = seconds % 60;
	unsigned int minutes = seconds / 60;
	utc->tm_min = minutes % 60;
	utc->tm_hour = minutes / 60;
}

class File : swift::noncopyable
{
public:
	File (const char* fileName) : fp_ (::fopen (fileName, "rb")) {}
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
		if (n != ::fread (buf, 1, n, fp_)) {
			throw std::logic_error ("no enough data to read");
		}

		return std::string (buf, n);
	}

	int32_t ReadInt32 ()
	{
		int32_t ret = 0;
		if (sizeof(int32_t) != ::fread (&ret, 1, sizeof(int32_t), fp_)) {
			throw std::logic_error ("bad read int32_t data");
		}

		return ret;
	}

	uint8_t ReadUInt8 ()
	{
		uint8_t ret = 0;
		if (sizeof(uint8_t) != ::fread (&ret, 1, sizeof(uint8_t), fp_)) {
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
	std::vector<detail::Transition> transitions;
	std::vector<detail::Localtime> localtimes;
	std::vector<std::string> names;
	std::string abbreviation;
};

namespace detail {

// zone file in the directory of /usr/share/zoneinfo/ etc. localtime
// localtime -> /etc/localtime
bool ReadTimeZoneFile (const char* zoneFile, struct TimeZone::Data* data)
{
	File f (zoneFile);
	if (f.Valid ()) {
		try {
			// read head
			if ("TZif" != f.ReadBytes (4)) {
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

				data->localtimes.push_back (Localtime (gmtoff, isdst, abbrind));
			}

			for (int i = 0; i < timecnt; ++i)
			{
				int localIdx = localtimes[i];
				time_t localtime = trans[i] + data->localtimes[localIdx].gmtOffset;
				data->transitions.push_back (Transition (trans[i], localtime, localIdx));
			}

			data->abbreviation = f.ReadBytes (charcnt);

			// leapcnt
			for (int i = 0; i < leapcnt; ++i) {
				// int32_t leaptime = f.ReadInt32 ();
				// int32_t cumleap = f.ReadInt32 ();
			}
			// FIXME
			(void) isstdcnt;
			(void) isgmtcnt;
		}
		catch (std::logic_error& e) {
			fprintf (stderr, "%s\n", e.what ());
		}
	}
} // ReadTimeZoneFile

const Localtime* FindLocalTime (const TimeZone::Data& data,
								const Transition& trans,
								const Compare& comp)
{
	const Localtime* localtime = nullptr;
	if (data.transitions.empty () || comp (trans, data.transitions.front ())) {
		// FIXME: should be first non dst time zone
		localtime = &data.localtimes.front ();
	}
	else {
		// lower_bound: 
		// Returns an iterator pointing to the first element in the range 
		// [data.transitions.begin (), data.transitions.end ()) which does not compare less than trans.
		std::vector<Transition>::const_iterator transI = std::lower_bound (data.transitions.begin (),
																			data.transitions.end (),
																			trans,
																			comp);
		if (transI != data.transitions.end ()) {
			if (!comp.Equal (trans, *transI)) {
				assert (transI != data.transitions.begin ());
				--transI;
			}

			localtime = &data.localtimes[transI->localtimeIndex];
		}
		else {
			// FIXME: use TZ-env
			localtime = &data.localtimes[data.transitions.back ().localtimeIndex];
		}
	}

	return localtime;
} // FindLocalTime

} // namespace detail

using namespace swift;

// public
TimeZone::TimeZone (const char* zoneFile) 
	: data_ (std::make_shared<TimeZone::Data> ())
{
	if (!detail::ReadTimeZoneFile (zoneFile, data_.get ())) {
		data_.reset ();
	}
}

// public
TimeZone::TimeZone (int eastOfUtc, const char* tzname)
	: data_ (std::make_shared<TimeZone::Data> ())
{
	data_->localtimes.push_back (detail::Localtime (eastOfUtc, false, 0));
	data_->abbreviation = std::move (std::string (tzname));
}

// public
struct tm TimeZone::ToLocalTime (time_t secondsSinceEpoch) const
{
	struct tm localtime;
	::bzero (&localtime, sizeof (localtime));
	assert (nullptr != data_.get ());
	detail::Transition trans (secondsSinceEpoch, 0, 0);
	const detail::Localtime* local = detail::FindLocalTime (*data_, trans, detail::Compare (true));
	if (local) {
		time_t localSeconds = secondsSinceEpoch + local->gmtOffset;
		::gmtime_r (&localSeconds, &localtime);
		localtime.tm_isdst = local->isDst;
		localtime.tm_gmtoff = local->gmtOffset;
		localtime.tm_zone = &data_->abbreviation[local->arrbIndex];
	}

	return localtime;
}

// public
time_t swift::TimeZone::FromLocalTime (const struct tm& t) const
{

}

} // namespace swift
