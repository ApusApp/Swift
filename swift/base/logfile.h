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

#ifndef __SWIFT_BASE_LOG_FILE_H__
#define __SWIFT_BASE_LOG_FILE_H__

#include <mutex>
#include <memory>
#include <string>
#include <time.h>

#include "swift/base/noncopyable.hpp"

namespace swift {

class LogFile : swift::noncopyable
{
public:
    LogFile (const std::string& file_name,
             size_t roll_size,
             bool thread_safe = true,
             int flush_interval = 3,
             int check_every_count = 1024);
    ~LogFile ();

    void Append (const char* log_line, int length);
    void Flush ();
    bool RollFile ();

private:
    void AppendUnlocked (const char* log_line, int length);
    static std::string GetLogFileName (const std::string& base_name, time_t* now);

private:
    const static int kRollPerSeconds = 60 * 60 * 24; // one day

private:
    std::unique_ptr<std::mutex> mutex_;
    const std::string name_;
    const size_t roll_size_;        // reached this size then write another file
    const int flush_interval_;      // flush interval
    const int check_every_count_;   // reached this number then flush data to disk
    int count_;                     // write count

    time_t start_of_period_;
    time_t last_roll_;
    time_t last_flush_;

    class AppendFile;
    std::unique_ptr<AppendFile> file_;
};
} // namespace swift
#endif // __SWIFT_BASE_LOG_FILE_H__