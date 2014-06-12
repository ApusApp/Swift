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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cxxabi.h>
#include <signal.h>
#include <execinfo.h>
#include <assert.h>
#include <memory>

#include "swift/base/stacktrace.h"

namespace swift {

namespace {

void PrintStackTraceByLine (const char *program_name, 
                            const char *symbol, 
                            void *frame)
{
    if (symbol) {
        fprintf (stderr, "%s ", symbol);
    }

    const int line_max = 256;
    char cmd[line_max] = { '\0' };
    snprintf (cmd, sizeof(cmd),
              "addr2line %p -e %s -f -C 2>&1",
              frame, program_name);
    FILE *f = popen (cmd, "r");
    if (nullptr != f) {
        char line[line_max] = { '\0' };
        while (fgets (line, sizeof(line), f)) {
            line[strlen (line) - 1] = '\0';
            fprintf (stderr, "%s\t", line);
        }
        pclose (f);
    }
    else {
        fprintf (stderr, " %p", frame);
    }

    fprintf (stderr, "\n");
}
} // namespace

// static public
bool StackTrace::GetExecutableName (std::string* name)
{
    assert (nullptr != name);
    char buf[1024] = { '\0' };
    char link[64] = { '\0' };
    
    snprintf (link, sizeof(link), "/proc/%d/exe", getpid ());
    ssize_t n = readlink (link, buf, sizeof(buf));
    if (-1 == n) {
        return false;
    }
    else {
        buf[n] = '\0';
        name->clear ();
        name->append (buf);
    }

    return true;
}

// static public
void StackTrace::PrintStack (int first_frames_to_skip /*= 0*/)
{
    void* buf[100];
    int size = ::backtrace (buf, 100);
    std::unique_ptr<char*, void (*)(void*)> stacks{
        ::backtrace_symbols (buf, size),
        std::free
    };

    if (nullptr != stacks.get ()) {
        std::string program_name;
        bool running = GetExecutableName (&program_name);
        for (int i = first_frames_to_skip; i < size && running; ++i) {
            fprintf (stderr, "#%-2d  ", i - first_frames_to_skip);
            PrintStackTraceByLine (program_name.c_str (), stacks.get ()[i], buf[i]);
        }
    }
}

// static private
void StackTrace::StackTraceHandler (int signal_num)
{
    // reset to default handler
    signal (signal_num, SIG_DFL);

    fprintf (stderr, "Received signal %d (%s)\n", signal_num, strsignal (signal_num));
    PrintStack (3);

    // re-signal to default handler
    raise (signal_num);
}

// static public
void StackTrace::InitStackTraceHandler ()
{
    signal (SIGILL, StackTraceHandler);
    signal (SIGSEGV, StackTraceHandler);
    signal (SIGBUS, StackTraceHandler);
    signal (SIGABRT, StackTraceHandler);
}

} // namespace swift
