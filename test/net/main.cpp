#include <gtest/gtest.h>
#include <swift/base/stacktrace.h>

int main (int argc, char* argv[])
{
    testing::InitGoogleTest (&argc, argv);
    swift::StackTrace::InitStackTraceHandler ();

    return RUN_ALL_TESTS ();
}