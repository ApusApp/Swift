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

#ifndef __SWIFT_BASE_LOGGING_H__
#define __SWIFT_BASE_LOGGING_H__

#include <functional>

#include "swift/base/LogStream.h"
#include "swift/base/Timestamp.h"
#include "swift/base/noncopyable.hpp"

namespace swift {

class TimeZone;

class Logger
{
public:
    enum LogSeverity
    {
        LS_TRACE,
        LS_DEBUG,
        LS_INFO,
        LS_WARN,
        LS_ERROR,
        LS_FATAL,
        LS_NUMBER_LOG_SERVERITY,
    };

    // compile time calculation of name of source file
    class SourceFile
    {
    public:
        template<int length>
        inline SourceFile (const char (&file_name)[length])
            : name_ (file_name)
            , size_ (length - 1)
        {
            // strrchr ("a/b/c.cc", '/') return "/c.cc"
            const char* p = ::strrchr (name_, '/');
            if (nullptr != p) {
                name_ = p + 1;
                size_ -= static_cast<int>(name_ - file_name);
            }
        }

        explicit SourceFile (const char* file_name)
            : name_ (file_name)
            , size_ (0)
        {
            const char* p = ::strrchr (file_name, '/');
            if (nullptr != p) {
                name_ = p + 1;
            }

            size_ = static_cast<int>(::strlen (name_));
        }

        ~SourceFile () {}

    public:
        const char* name_;
        int size_;
    };

    Logger (const SourceFile& file, int line);
    Logger (const SourceFile& file, int line, LogSeverity log_serverity);
    Logger (const SourceFile& file, int line, LogSeverity log_serverity, const char* func_name);
    Logger (const SourceFile& file, int line, bool is_abort);
    ~Logger ();

    inline LogStream& GetStream ()
    {
        return logger_impl_.stream_;
    }

public:
    static LogSeverity GetLogSeverity ();
    static void SetLogSeverity (LogSeverity log_serverity);
    static void SetOutput (std::function<void (const char*, int)>&& func);
    static void SetFlush (std::function<void (void)>&& func);
    static void SetTimeZone (const TimeZone& tz);

private:
    class LoggerImpl : swift::noncopyable
    {
    public:
        LoggerImpl (Logger::LogSeverity log_serverity,
                    const int old_errno,
                    const SourceFile& file,
                    const int line);
        ~LoggerImpl () {}
        void FormatTime ();
        void Finish ();

    public:
        Timestamp time_;
        LogStream stream_;
        LogSeverity log_severity_;
        int line_;
        SourceFile file_name_;
    };

    LoggerImpl logger_impl_;
};

extern Logger::LogSeverity g_log_severity;

// static public
inline Logger::LogSeverity Logger::GetLogSeverity ()
{
    return g_log_severity;
}

#define LOG_TRACE if (swift::Logger::GetLogSeverity () <= swift::Logger::LS_TRACE) \
    swift::Logger (__FILE__, __LINE__, swift::Logger::LS_TRACE, __func__).GetStream ()

#define LOG_DEBUG if (swift::Logger::GetLogSeverity () <= swift::Logger::LS_DEBUG) \
    swift::Logger (__FILE__, __LINE__, swift::Logger::LS_DEBUG, __func__).GetStream ()

#define LOG_INFO if (swift::Logger::GetLogSeverity () <= swift::Logger::LS_INFO) \
    swift::Logger (__FILE__, __LINE__).GetStream ()

#define LOG_WARN swift::Logger (__FILE__, __LINE__, swift::Logger::LS_WARN).GetStream ()
#define LOG_ERROR swift::Logger (__FILE__, __LINE__, swift::Logger::LS_ERROR).GetStream ()
#define LOG_FATAL swift::Logger (__FILE__, __LINE__, swift::Logger::LS_FATAL).GetStream ()
#define LOG_SYSERR swift::Logger (__FILE__, __LINE__, false).GetStream ()
#define LOG_SYSFATAL swift::Logger (__FILE__, __LINE__, true).GetStream ()

// CHECK_NOTNULL get from glog/logging.h

// Check that the input is non NULL.  This very useful in constructor
// initializer lists.
#define CHECK_NOTNULL(val) swift::CheckNotNull (__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull (const Logger::SourceFile& file, int line, const char *names, T* t) {
    if (t == NULL) {
        Logger (file, line, Logger::LS_FATAL).GetStream () << names;
    }
    return t;
}

} // namespace swift

#endif // __SWIFT_BASE_LOGGING_H__
