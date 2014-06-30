#include <gtest/gtest.h>
#include <thread>

#include <swift/base/logging.h>
#include <swift/base/logfile.h>
#include <swift/base/threadpool.h>
#include <swift/base/timezone.h>

class test_Logging : public testing::Test
{
public:
    test_Logging () {}
    ~test_Logging () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

std::atomic<uint64_t> g_total_write_size;
FILE* g_file_descriptor = nullptr;
std::unique_ptr<swift::LogFile> g_log_file;

void LogBench (const char* buf, bool is_write_long_string = false)
{
    swift::Logger::SetOutput (std::move ([&] (const char* msg, int length) {
        g_total_write_size += length;
        if (g_file_descriptor) {
            fwrite (msg, 1, length, g_file_descriptor);
        }
        else if (g_log_file.get ()) {
            g_log_file->Append (msg, length);
        }
    }));

    swift::Timestamp start (swift::Timestamp::Now ());
    g_total_write_size = 0;

    int n = 1000 * 2000;
    std::string empty = " ";
    std::string long_str (3000, 'Y');
    long_str += " ";
    for (int i = 0; i < n; ++i) {
        LOG_INFO << "ABCDEFGHIJKLMNOPQRSTUVWXZ"
                 << (is_write_long_string ? long_str : empty)
                 << i;
    }

    swift::Timestamp end (swift::Timestamp::Now ());
    double seconds = swift::TimeDifference (end, start);
    // printf ("%12s: %f seconds, %lu bytes, %10.2f msg/s, %.2f MiB/s\n",
    //     buf, seconds, g_total_write_size.load (), n / seconds, g_total_write_size / seconds / (1024 * 1024));
    printf ("%12s: %f seconds, %llu bytes, %10.2f msg/s, %.2f MiB/s\n",
        buf, seconds, g_total_write_size.load (),
        static_cast<double>(n / seconds),
        static_cast<double>(g_total_write_size / seconds / (1024 * 1024)));
    g_total_write_size = 0;
}

TEST_F (test_Logging, All)
{
    LogBench ("nop", true);

    char buffer[1024 * 64] = {'\0'};
    g_file_descriptor = fopen ("/dev/null", "w");
    setbuffer (g_file_descriptor, buffer, sizeof(buffer));
    LogBench ("/dev/null", true);
    fclose (g_file_descriptor);
    g_file_descriptor = nullptr;

    g_file_descriptor = fopen ("/tmp/log", "w");
    setbuffer (g_file_descriptor, buffer, sizeof(buffer));
    LogBench ("/tmp/log");
    fclose (g_file_descriptor);
    g_file_descriptor = nullptr;

    g_log_file.reset (new swift::LogFile ("test_log_st", 500 * 1000 * 1000, false));
    LogBench ("test_log_st");

    g_log_file.reset (new swift::LogFile ("test_log_mt", 1000 * 1000 * 1000, true));
    auto f1 = std::thread ([] () {LogBench ("test_log_mt_th1");});
    auto f2 = std::thread ([] () {LogBench ("test_log_mt_th2");});
    LogBench ("test_log_mt");
    f1.join ();
    f2.join ();
    g_log_file.reset ();

    system ("rm -rf test_log_*");

    g_file_descriptor = stdout;
    sleep (1);
    swift::TimeZone beijing (8 * 3600, "CST");
    swift::Logger::SetTimeZone (beijing);
    LOG_TRACE << "trace CST";
    LOG_DEBUG << "debug CST";
    LOG_INFO << "Hello CST";
    LOG_WARN << "World CST";
    LOG_ERROR << "Error CST";

    sleep (1);
    swift::TimeZone newyork ("/usr/share/zoneinfo/America/New_York");
    swift::Logger::SetTimeZone (newyork);
    LOG_TRACE << "trace NYT";
    LOG_DEBUG << "debug NYT";
    LOG_INFO << "Hello NYT";
    LOG_WARN << "World NYT";
    LOG_ERROR << "Error NYT";
    g_file_descriptor = nullptr;
}