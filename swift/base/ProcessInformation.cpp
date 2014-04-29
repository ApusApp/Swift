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

#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <gnu/libc-version.h>

#include <vector>

#include "swift/base/logging.h"
#include "swift/base/processinformation.h"

namespace swift {
namespace detail {

#define KLONG long
#define KLF "l"

class LinuxProc
{
public:
    LinuxProc (pid_t pid)
    {
        char file_name[128] = {'\0'};
        snprintf (file_name, sizeof(file_name), "/proc/%d/stat", static_cast<uint32_t>(pid));

        FILE* pfd = fopen (file_name, "r");
        if (nullptr == pfd) {
            LOG_ERROR << "Could not open [" << file_name << "], pid = " 
                      << static_cast<uint32_t>(pid) << "\n";

            // TO DO
            // throw a Exception
        }
        else {
            int found = 0;
            found = fscanf (pfd, "%d %127s %c "
                            "%d %d %d %d %d "
                            "%lu %lu %lu %lu %lu "
                            "%lu %lu %ld %ld "  /* utime stime cutime cstime */
                            "%ld %ld "
                            "%ld "
                            "%ld "
                            "%lu "  /* start_time */
                            "%lu "
                            "%ld " // rss
                            "%lu %" KLF "u %" KLF "u %" KLF "u %" KLF "u %" KLF "u "
                            /*
                                "%*s %*s %*s %*s "
                                "%"KLF"u %*lu %*lu "
                                "%d %d "
                                "%lu %lu"
                            */
                            ,
                            &pid_, commond_, &state_,
                            &ppid_, &pgroup_, &session_id_, &tty_, &tpgid_,
                            &flags_, &min_flt_, &cmin_flt_, &major_flt_, &cmajor_flt_,
                            &utime_, &stime_, &cutime_, &cstime_,
                            &priority_, &nice_,
                            &nlwp_,
                            &alarm_,
                            &start_time_,
                            &virtual_memory_size_,
                            &resident_set_size_,
                            &resident_set_size_rlim_, &start_code_, &end_code_, &start_stack_, &kstk_esp_, &kstk_eip_
                            /*
                                &_wchan,
                                &_exit_signal, &_processor,
                                &_rtprio, &_sched
                            */
                            );
            if (0 == found) {
                LOG_ERROR << "System error: reading proc information, pid = " 
                          << static_cast<uint32_t>(pid) << "\n";
            }

            fclose (pfd);

            assert (pid == pid_);
        }
    }

    inline unsigned long GetVirtualMemorySize () const
    {
        return virtual_memory_size_;
    }

    inline unsigned long GetResidentSize () const
    {
        return static_cast<unsigned> (resident_set_size_ * 4 * 1024);
    }

public:
    // The process ID
    int pid_;

    // The filename of the executable, in parentheses.
    // This is visible whether or not the executable is swapped out.
    char commond_[128];

    // One character from the string "RSDZTW" where R is running, 
    // S is sleeping in an interruptible wait, D is waiting in uninterruptible
    // disk sleep, Z is zombie, T is traced or stopped (on a signal), and W is paging.
    char state_;

    // The PID of the parent
    int ppid_;

    // The process group ID
    int pgroup_;

    // The session ID
    int session_id_;

    // The tty the process uses
    int tty_;

    // The process group ID of the process which 
    // currently owns the tty that the process is connected to.
    int tpgid_;

    // The  kernel flags word of the process. For bit meanings, 
    // see the PF_* defines in <linux/sched.h>.  Details depend on the kernel version.
    // %lu
    unsigned long flags_;

    // The number of minor faults the process has made which have not required loading a memory page from disk.
    // %lu
    unsigned long min_flt_;

    // The number of minor faults that the process
    // %lu
    unsigned long cmin_flt_;

    // The number of major faults the process has made which 
    // have required loading a memory page from disk
    // %lu
    unsigned long major_flt_;

    // The number of major faults the process
    // %lu
    unsigned long cmajor_flt_;

    // The number of jiffies that this process has been scheduled in user mode
    // %lu
    unsigned long utime_;

    // The number of jiffies that this process has been scheduled in kernel mode
    // %lu
    unsigned long stime_;

    // The number of jiffies that removed field
    // %ld
    long cutime_;

    long cstime_;

    long priority_;
    long nice_;

    // number of threads
    // %ld
    long nlwp_;

    // The time in jiffies before the next SIGALRM is sent to the process
    // due to an interval timer
    // %lu
    unsigned long alarm_;

    // The time in jiffies the process started after system boot
    // %lu
    unsigned long start_time_;

    // Virtual memory size in bytes
    // %lu
    unsigned long virtual_memory_size_;

    // Resident Set Size: number of pages the process has in real memory, 
    // minus 3 for administrative purposes. This is just the pages which
    // count  towards text, data, or stack space. This does not include pages 
    // which have not been demand-loaded in, or which are swapped out
    // %ld
    long resident_set_size_;

    // Current limit in bytes on the resident set size of the process
    // %lu
    unsigned long resident_set_size_rlim_;

    // The address above which program text can run
    // %lu
    unsigned long start_code_;

    // The address below which program text can run
    // %lu
    unsigned long end_code_;

    // The address of the start of the stack
    // %lu
    unsigned long start_stack_;

    // The current value of esp (stack pointer), as found in the 
    // kernel stack page for the process
    // %lu
    unsigned long kstk_esp_;

    // The current EIP (instruction pointer)
    // %lu
    unsigned long kstk_eip_;
}; // LinuxProc

class LinuxSystemHelper
{
public:
    LinuxSystemHelper () {}
    ~LinuxSystemHelper () {}

public:
    // Read the first 1023 bytes form a file
    static std::string ReadLineFromFile (const char* file_name)
    {
        assert (nullptr != file_name);
        FILE* pfd = nullptr;
        char fstr[1024] = {'\0'};

        pfd = fopen (file_name, "r");
        if (nullptr != pfd) {
            if (nullptr != fgets (fstr, 1023, pfd)) {
                fstr[strlen (fstr) < 1 ? 0 : strlen (fstr) - 1] = '\0';
            }

            fclose (pfd);
        }

        return fstr;
    }

    //Get some details about the CPU
    static void GetCpuInformation (int &proc_count, 
                                   std::string& freq,
                                   std::string& features)
    {
        FILE* pfd = nullptr;
        char fstr[1024] = {'\0'};
        proc_count = 0;

        pfd = fopen ("/proc/cpuinfo", "r");
        if (nullptr == pfd) {
            return;
        }

        while (nullptr != fgets (fstr, 1023, pfd) && !feof (pfd)) {
            // until the end of the file
            fstr[strlen (fstr) < 1 ? 0 : strlen (fstr) - 1] = '\0';
            if (0 == strncmp (fstr, "processor\t:", 11)) {
                ++proc_count;
                continue;
            }

            if (0 == strncmp (fstr, "cpu MHz\t\t:", 10)) {
                freq = fstr + 11;
                continue;
            }

            if (0 == strncmp (fstr, "flags\t\t:", 8)) {
                features = fstr + 9;
                continue;
            }
        }

        fclose (pfd);
    }

    //Determine Linux distro and version
    static void GetLinuxDistro (std::string& name, std::string& version)
    {
        FILE* pfd = nullptr;
        bool found = false;
        char buf[512] = {'\0'};
        
        // try lsb file first
        if (0 == access ("/etc/lsb-release", 0)) {
            pfd = fopen ("/etc/lsb-release", "r");
        }
        
        if (nullptr != pfd) {
            while (nullptr != fgets (buf, 511, pfd) && !feof (pfd)) {
                buf[strlen (buf) < 1 ? 0 : strlen (buf) - 1] = '\0';
                if (0 == strncmp (buf, "DISTRIB_ID", 10)) {
                    name = buf + 11;
                    continue;
                }

                if (0 == strncmp (buf, "DISTRIB_RELEASE", 15)) {
                    version = buf + 16;
                    continue;
                }

                if (!name.empty () && !version.empty ()) {
                    found = true;
                    break;
                }
            }

            fclose (pfd);
            if (found) {
                return;
            }
        }

        // not found
        // try known flat-text file locations
        std::vector <std::string> paths;
        found = false;
        paths.push_back ("/etc/system-release");
        paths.push_back ("/etc/redhat-release");
        paths.push_back ("/etc/gentoo-release");
        paths.push_back ("/etc/novell-release");
        paths.push_back ("/etc/gentoo-release");
        paths.push_back ("/etc/SuSE-release");
        paths.push_back ("/etc/SUSE-release");
        paths.push_back ("/etc/sles-release");
        paths.push_back ("/etc/debian_release");
        paths.push_back ("/etc/slackware-version");
        paths.push_back ("/etc/centos-release");

        std::vector<std::string>::iterator it = paths.begin ();
        for (; it != paths.end (); ++it) {
            if (0 == access (it->c_str (), 0)) {
                found = true;
                break;
            }
        }

        if (found) {
            pfd = fopen (it->c_str (), "r");
            if (nullptr == pfd) {
                bzero (buf, sizeof(buf));
                if (nullptr != fgets (buf, 511, pfd)) {
                    buf[strlen (buf) < 1 ? 0 : strlen (buf) - 1] = '\0';
                    name = buf;
                }

                fclose (pfd);
                version = "Kernel ";
                version += LinuxSystemHelper::ReadLineFromFile ("/proc/sys/kernel/osrelease");
            }
        }
    }

    //Get system memory total size
    static unsigned long long GetSystemMemorySize ()
    {
        std::string memory_info = LinuxSystemHelper::ReadLineFromFile ("/proc/meminfo");
        size_t line_off = 0;
        if (!memory_info.empty () 
            && (line_off = memory_info.find ("MemTotal")) != std::string::npos) {
            line_off = memory_info.substr (line_off).find (':') + 1;
            memory_info = memory_info.substr (line_off, memory_info.substr (line_off).find ("kB") - 1);
            line_off = 0;

            // trim whitespace and append 000 to replace kB
            while (isspace (memory_info.at (line_off))) {
                ++line_off;
            }
            memory_info = memory_info.substr (line_off);
            if (!memory_info.empty ()) {
                return strtoull (memory_info.c_str (), NULL, 10);
            }
        }

        return 0;
    }
};

} // namepsace detail

// public
ProcessInformation::ProcessInformation (const pid_t pid /*= GetProcessId ()*/)
    : pid_ (pid)
{
    InitializeSystemInformation ();
}

// public
ProcessInformation::~ProcessInformation ()
{

}

// public
int ProcessInformation::GetVirtualMemorySize ()
{
    detail::LinuxProc lp (pid_);
    return static_cast<int>(lp.GetVirtualMemorySize () / (1024.0 * 1024));
}

// public
int ProcessInformation::GetResidentSize ()
{
    detail::LinuxProc lp (pid_);
    return static_cast<int>(lp.GetResidentSize () / (1024.0 * 1024));
}

// static private
bool ProcessInformation::CheckNumaEnabled ()
{
    bool has_multiple_nodes = false;
    bool has_numa_maps = false;

    if (0 == access ("/sys/devices/system/node/node1", 0)) {
        has_multiple_nodes = true;
    }

    if (0 == access ("/proc/self/numa_maps", 0)) {
        has_numa_maps = true;
    }

    if (has_multiple_nodes && has_numa_maps) {
        // proc is populated with numa entries

        // read the second column of first line to determine numa state
        // ('default' = enabled, 'interleave' = disabled).  
        // Logic from version.cpp's warnings.
        std::string line = detail::LinuxSystemHelper::ReadLineFromFile ("/proc/self/numa_maps");
        size_t pos = line.find (' ');
        if (pos != std::string::npos && 
            line.substr (pos + 1, 10).find ("interleave") == std::string::npos) {
            // interleave not found; 
            return true;
        }
    }

    return false;
}

// static public
bool ProcessInformation::BlockCheckSupported ()
{
    return true;
}

// static public
bool ProcessInformation::BlockInMemory (const void* start)
{
    unsigned char x = 0;
    if (mincore (const_cast<void*>(AlignToStartOfPage (start)), 
                 GetPageSize (), 
                 &x)) {
        LOG_ERROR << "mincore failed: " << strerror (errno) << "\n";
        return 1;
    }

    return x & 0x1;
}

// static private
std::unique_ptr<ProcessInformation::SystemInformation> ProcessInformation::kSystemInfo;

// static private
std::unique_ptr<std::mutex> ProcessInformation::kSystemInformationLock = 
    std::move (std::unique_ptr<std::mutex> (new std::mutex));

// static public
void ProcessInformation::InitializeSystemInformation ()
{
    kSystemInformationLock->lock ();
    if (nullptr == kSystemInfo.get ()) {
        kSystemInfo = 
            std::move (std::unique_ptr<ProcessInformation::SystemInformation> (new SystemInformation));
    }
    kSystemInformationLock->unlock ();
}

// private
void ProcessInformation::SystemInformation::CollectSystemInformation ()
{
    int cpu_count;
    struct utsname uname_data;
    std::string distro_name;
    std::string distro_version;
    std::string cpu_frequncy;
    std::string cpu_features;

    std::string version_signature = 
        detail::LinuxSystemHelper::ReadLineFromFile ("/proc/version_signature");
    detail::LinuxSystemHelper::GetCpuInformation (cpu_count, cpu_frequncy, cpu_features);
    detail::LinuxSystemHelper::GetLinuxDistro (distro_name, distro_version);

    if (-1 == uname (&uname_data)) {
        LOG_ERROR << "Unable to collect detailed system information: "
                  << strerror (errno) << "\n";
    }

    os_type_ = "Linux";
    os_name_ = distro_name;
    os_version_ = distro_version;
    memory_size_ = detail::LinuxSystemHelper::GetSystemMemorySize ();
    address_size_ = (std::string (uname_data.machine).find ("x86_64") != std::string::npos ? 64 : 32);
    number_cores_ = cpu_count;
    page_size_ = static_cast<unsigned long long>(sysconf (_SC_PAGESIZE));
    cpu_arch_ = uname_data.machine;
    has_numa_ = CheckNumaEnabled ();
    libc_version_ = gnu_get_libc_version ();
    version_signature_ = version_signature;
    kernel_version_ = uname_data.release;
    cpu_frequncy_ = cpu_frequncy;
    cpu_features_ = cpu_features;
    number_pages_ = static_cast<unsigned int>(sysconf (_SC_PHYS_PAGES));
    max_open_files_ = static_cast<unsigned int>(sysconf (_SC_OPEN_MAX));
}

} // namespace swift
