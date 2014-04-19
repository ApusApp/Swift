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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

#include "swift/base/Logging.h"
#include "swift/base/TimeZone.h"
#include "swift/base/Timestamp.h"
#include "swift/base/ThisThread.h"

namespace swift {

__thread char t_errno_buf[512];
__thread char t_time[32];
__thread time_t t_last_second;

namespace detail {
    inline const char* StrError_R (int saved_errno)
    {
        return ::strerror_r (saved_errno, swift::t_errno_buf, sizeof(swift::t_errno_buf));
    }

    inline Logger::LogLevel InitLogLevel ()
    {
        if (::getenv ("SWIFT_LOG_TRACE")) {
            return swift::Logger::LL_TRACE;
        }
        else if (::getenv ("SWIFT_LOG_DEBUG")) {
            return swift::Logger::LL_DEBUG;
        }
        
        return swift::Logger::LL_INFO;
    }

    const char* log_level_name[swift::Logger::LL_NUMBER_LOG_LEVELS] = {
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
    };

    // helper class for known string length at compile time
    class T
    {
    public:
        T (const char* str, unsigned int len)
            :str_ (str)
            ,length_ (len)
        {
            assert (::strlen (str) == length_);
        }

        ~T () {}

    public:
        const char* str_;
        const unsigned int length_;
    };
} // namespace detail

inline LogStream& operator<< (LogStream& s, const detail::T& v)
{
    s.Append (v.str_, v.length_);
    return s;
}

inline LogStream& operator<< (LogStream& s, const Logger::SourceFile& f)
{
    s.Append (f.name_, f.size_);
    return s;
}

// public 
Logger::LogLevel g_log_level = detail::InitLogLevel ();

std::function<void (const char*, int)> g_output = std::move ([](const char* msg, int len) {
    ::fwrite (msg, 1, len, stdout);
});

std::function<void (void)> g_flush = std::move ([](void) {
    ::fflush (stdout);
});

TimeZone g_log_time_zone;

// private
Logger::LoggerImpl::LoggerImpl (Logger::LogLevel level, 
                                const int old_errno, 
                                const SourceFile& file, 
                                const int line)
    : time_ (Timestamp::Now ())
    , stream_ ()
    , level_ (level)
    , line_ (line)
    , file_name_ (file)
{
    FormatTime ();
    ThisThread::GetTid ();
    stream_ << detail::T (ThisThread::TidToString (), 6);
    stream_ << detail::T (detail::log_level_name[level], 6);
    if (0 != old_errno) {
        stream_ << detail::StrError_R (old_errno) << " (errno=" << old_errno << ") ";
    }
}

// public
void Logger::LoggerImpl::FormatTime ()
{
    int64_t micro_seconds_since_epoch = time_.MicroSecondsSinceEpoch ();
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(micro_seconds_since_epoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_last_second) {
        t_last_second = seconds;
        struct tm localtime;
        if (g_log_time_zone.Valid ()) {
            localtime = g_log_time_zone.ToLocalTime (seconds);
        }
        else {
            ::gmtime_r (&seconds, &localtime);
        }

        int length = snprintf (t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d", 
                               localtime.tm_year + 1900,
                               localtime.tm_mon + 1,
                               localtime.tm_mday,
                               localtime.tm_hour,
                               localtime.tm_min,
                               localtime.tm_sec);
        assert (17 == length);
        (void) length;
    }

    if (g_log_time_zone.Valid ()) {
        Format ft (".%06d ", microseconds);
        assert (9 == ft.Length ());
        stream_ << detail::T (t_time, 17) << detail::T (ft.Data (), 9);
    }
}

// public
void Logger::LoggerImpl::Finish ()
{
    stream_ << " - " << file_name_ << ':' << line_ << '\n';
}

// public
Logger::Logger (const SourceFile& file, int line)
    : logger_impl_ (LL_INFO, 0, file, line)
{

}

// public
Logger::Logger (const SourceFile& file, int line, LogLevel level)
    : logger_impl_ (level, 0, file, line)
{

}

// public
Logger::Logger (const SourceFile& file, int line, LogLevel level, const char* func_name)
    : logger_impl_ (level, 0, file, line)
{
    logger_impl_.stream_ << func_name << ' ';
}

// public
Logger::Logger (const SourceFile& file, int line, bool is_abort)
: logger_impl_ (is_abort ? LL_FATAL : LL_ERROR, errno, file, line)
{

}

// public
Logger::~Logger ()
{
    logger_impl_.Finish ();
    const LogStream::BufferType& buf (GetStream ().Buffer ());
    g_output (buf.Data (), buf.Length ());
    if (LL_FATAL == logger_impl_.level_) {
        g_flush ();
        ::abort ();
    }
}

// static public
void Logger::SetLogLevel (LogLevel level)
{
    g_log_level = level;
}

// static public
void Logger::SetOutput (std::function<void (const char*, int)>&& func)
{
    g_output = std::move (func);
}

// static public
void Logger::SetFlush (std::function<void (void)>&& func)
{
    g_flush = std::move (func);
}

// static public
void Logger::SetTimeZone (const TimeZone& tz)
{
    g_log_time_zone = tz;
}

} // namespace swift
