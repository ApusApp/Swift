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

#include <stdio.h>
#include <assert.h>

#include "swift/base/experimental/logfile.h"
#include "swift/base/processinformation.h"

namespace swift {

class LogFile::AppendFile : swift::noncopyable
{
public:
    explicit AppendFile (const std::string& file_name)
        : fd_ (fopen (file_name.c_str (), "ae"))
        , written_bytes_ (0)
    {
        assert (nullptr != fd_);
        setbuffer (fd_, buf_, sizeof(buf_));
    }

    ~AppendFile ()
    {
        if (nullptr != fd_) {
            fclose (fd_);
        }
    }

    inline size_t Write (const char* log_line, size_t length)
    {
        return ::fwrite_unlocked (log_line, 1, length, fd_);
    }

    void Append (const char* log_line, size_t length)
    {
        size_t n = Write (log_line, length);
        size_t remain = length - n;
        while (remain > 0) {
            size_t size = Write (log_line + n, remain);
            if (0 == size) {
                //int err = ferror (fd_);
                //if (err) {
                //    // TO DO
                //}
                break;
            }

            n += size;
            remain = length - n;
        }

        written_bytes_ += length;
    }

    inline void Flush ()
    {
        fflush (fd_);
    }

    size_t GetWrittenBytes () const
    {
        return written_bytes_;
    }

private:
    FILE* fd_;
    char buf_[64 * 1024];
    size_t written_bytes_;
};

// public
LogFile::LogFile (const std::string& file_name,
                  size_t roll_size,
                  bool thread_safe /*= true*/,
                  int flush_interval /*= 3*/,
                  int check_every_count /*= 1024*/)
    : mutex_ (thread_safe ? new std::mutex : nullptr)
    , name_ (file_name)
    , roll_size_ (roll_size)
    , flush_interval_ (flush_interval)
    , check_every_count_ (check_every_count)
    , count_ (0)
    , start_of_period_ (0)
    , last_roll_ (0)
    , last_flush_ (0)
{
    assert (file_name.find ('/') == std::string::npos);
    RollFile ();
}

// public
LogFile::~LogFile ()
{

}

// public
void LogFile::Append (const char* log_line, int length)
{
    if (nullptr != mutex_.get ()) {
        std::lock_guard<std::mutex> lock (*mutex_);
        AppendUnlocked (log_line, length);
    }
    else {
         AppendUnlocked (log_line, length);
    }
}

// public
void LogFile::Flush ()
{
    if (nullptr != mutex_.get ()) {
        std::lock_guard<std::mutex> lock (*mutex_);
        file_->Flush ();
    }
    else {
        file_->Flush ();
    }
}

// public
bool LogFile::RollFile ()
{
    time_t now = 0;
    std::string file_name = std::move (GetLogFileName (name_, &now));
    time_t start = now / kRollPerSeconds * kRollPerSeconds;

    if (now > last_roll_) {
        last_roll_ = now;
        last_flush_ = now;
        start_of_period_ = start;
        file_.reset (new LogFile::AppendFile (file_name));
        return true;
    }

    return false;
}

// private
void LogFile::AppendUnlocked (const char* log_line, int length)
{
    file_->Append (log_line, length);
    if (file_->GetWrittenBytes () > roll_size_) {
        RollFile ();
    }
    else {
        ++count_;
        if (count_ > check_every_count_) {
            count_ = 0;
            time_t now = ::time (nullptr);
            time_t this_period = now / kRollPerSeconds * kRollPerSeconds;
            if (this_period != start_of_period_) {
                RollFile ();
            }
            else if (now - last_flush_ > flush_interval_) {
                last_flush_ = now;
                file_->Flush ();
            }
        }
    }
}

// static private
std::string LogFile::GetLogFileName (const std::string& base_name, time_t* now)
{
    std::string file_name;
    file_name.reserve (base_name.size () + 64);
    file_name = base_name;

    char time_buf[32] = {'\0'};
    struct tm tmp_time;
    *now = time (nullptr);
    gmtime_r (now, &tmp_time);
    strftime (time_buf, sizeof(time_buf), ".%Y%m%d-%H%M%S.", &tmp_time);
    file_name += time_buf;
    file_name += ProcessInformation::GetHostName ();

    char pid_buf[32] = {'\0'};
    snprintf (pid_buf, sizeof(pid_buf), ".%d", ProcessInformation::GetProcessId ());
    file_name += pid_buf;
    file_name += ".log";

    return file_name;
}

} // namespace swift
