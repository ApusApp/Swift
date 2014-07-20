#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include <swift/base/experimental/logfile.h>
#include <swift/base/experimental/logging.h>
#include <swift/base/timezone.h>
#include <swift/base/thisthread.h>

class test_LogFile : public testing::Test
{
public:
    test_LogFile () {}
    ~test_LogFile () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

void DebugFunc (const std::string& line)
{
    for (int i = 0; i < 6000; ++i) {
        LOG_DEBUG << line << i;
    }
}

void TraceFunc (const std::string& line)
{
    for (int i = 0; i < 10000; ++i) {
        LOG_TRACE << line << i;
    }
}

TEST_F (test_LogFile, All)
{
    std::unique_ptr<swift::LogFile> log_file;

    std::string log_path = "~/test_swift_log_file";
    log_file.reset (new swift::LogFile (::basename (log_path.c_str ()), 200 * 1000));
    swift::Logger::SetOutput (std::move ([&log_file] (const char* msg, int len) {
        log_file->Append (msg, len);
    }));

    swift::Logger::SetFlush (std::move ([&log_file] () {
        log_file->Flush ();
    }));

    swift::Logger::SetLogSeverity (swift::Logger::LS_TRACE);
    swift::TimeZone tz ("/usr/share/zoneinfo/Asia/Shanghai");
    swift::Logger::SetTimeZone (tz);

    std::string line = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    std::cout << "Main process id = " << getpid () << std::endl;
    std::vector<std::thread> thread_pool;
    thread_pool.push_back (std::thread ([&line] () {
        std::cout << "LOG_INFO thread id = " << swift::thisthread::GetTid () << std::endl;
        for (int i = 0; i < 2000; ++i) {
            LOG_INFO << line << i;
            usleep (100);
        }
    }));

    thread_pool.push_back (std::thread ([&line] () {
        std::cout << "LOG_DEBUG thread id = " << swift::thisthread::GetTid () << std::endl;
        DebugFunc (line);
    }));

    thread_pool.push_back (std::thread ([&line] () {
        std::cout << "LOG_ERROR thread id = " << swift::thisthread::GetTid () << std::endl;
        for (int i = 0; i < 5000; ++i) {
            LOG_ERROR << line << i;
            usleep (100);
        }
    }));

    thread_pool.push_back (std::thread ([&line] () {
        std::cout << "LOG_TRACE thread id = " << swift::thisthread::GetTid () << std::endl;
        TraceFunc (line);
    }));

    thread_pool.push_back (std::thread ([&line] () {
        std::cout << "LOG_WARN thread id = " << swift::thisthread::GetTid () << std::endl;
        for (int i = 0; i < 4000; ++i) {
            LOG_WARN << line << i;
            usleep (100);
        }
    }));

    for (auto &it : thread_pool) {
        if (it.joinable ()) {
            it.join ();
        }
    }

    const char *cmd = "rm -rf test_swift_log_file*";
    system (cmd);
}