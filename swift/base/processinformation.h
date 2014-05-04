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

#ifndef __SWIFT_BASE_PROCESS_INFORMATION_H__
#define __SWIFT_BASE_PROCESS_INFORMATION_H__

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <mutex>
#include <memory>

namespace swift {
class ProcessInformation
{
public:
    ProcessInformation (const pid_t pid = GetProcessId ());
    ~ProcessInformation ();

    // @return mbytes
    int GetVirtualMemorySize ();

    // @return mbytes
    int GetResidentSize ();

    std::string GetExecutableName () const;

    inline uint32_t GetPidAsUint32 () const
    {
        return static_cast<uint32_t>(pid_);
    }

    inline std::string GetPidAsString () const
    {
        char buf[32] = {'\0'};
        snprintf (buf, sizeof(buf), "%d", static_cast<uint32_t>(pid_));
        return buf;
    }

    // Get the type of OS
    inline const std::string& GetOsType () const
    {
        return GetSystemInfo ().os_type_;
    }

    // Get Os version (Ubuntu, Red Hat ...)
    inline const std::string& GetOsName () const
    {
        return GetSystemInfo ().os_name_;
    }

    // Get Os version (e.g. 11.04, 11.10, 14.04 ...)
    inline const std::string& GetOsVersion () const
    {
        return GetSystemInfo ().os_version_;
    }

    // Get the CPU address size (e.g. 32, 36, 64)
    inline const unsigned int GetAddressSize () const
    {
        return GetSystemInfo ().address_size_;
    }

    // Get the total memory of system in MB
    inline const unsigned long long GetMemorySizeMB () const 
    {
        return GetSystemInfo ().memory_size_ / (1024 * 1024);
    }

    inline const unsigned long long GetNumberPages () const 
    {
        return GetSystemInfo ().number_pages_;
    }

    // Get the number of CPUs
    inline unsigned int GetNumberOfCores () const
    {
        return GetSystemInfo ().number_cores_;
    }

    inline unsigned int GetMaxOpenFiles () const
    {
        return GetSystemInfo ().max_open_files_;
    }

    // Get the CPU architecture (e.g. x86, x86_64)
    inline const std::string& GetArchitecture () const
    {
        return GetSystemInfo ().cpu_arch_;
    }

    // Determine if NUMA is enabled for this process
    inline bool HasNumaEnabled () const
    {
        return GetSystemInfo ().has_numa_;
    }

    inline const std::string& GetLibcVersion () const
    {
        return GetSystemInfo ().libc_version_;
    }

    inline const std::string& GetKernelVersion () const
    {
        return GetSystemInfo ().kernel_version_;
    }

    inline const std::string& GetCpuFrequncy () const
    {
        return GetSystemInfo ().cpu_frequncy_;
    }

    inline const std::string& GetCpuFeatures () const
    {
        return GetSystemInfo ().cpu_features_;
    }

    inline const std::string& GetVersionSignature () const
    {
        return GetSystemInfo ().version_signature_;
    }

public:
    // This function must call at least once
    static void InitializeSystemInformation ();

    inline static pid_t GetProcessId ()
    {
        return ::getpid ();
    }

    inline static pid_t GetParentProcessId ()
    {
        return ::getppid ();
    }

    // Get the system page size in bytes
    inline static long long GetPageSize ()
    {
        return kSystemInfo->page_size_;
    }

    static std::string GetHostName ();

    static bool BlockCheckSupported ();

    static bool BlockInMemory (const void* start);

    /**
    * @return a pointer aligned to the start of the page the provided pointer belongs to.
    *
    * NOTE requires BlockCheckSupported () == true
    */
    inline static const void* AlignToStartOfPage (const void* ptr)
    {
        return reinterpret_cast<const void*>(
               reinterpret_cast<unsigned long long>(ptr) & ~(GetPageSize () - 1));
    }

private:
    // Host and operating system information
    class SystemInformation
    {
    public:
        SystemInformation ()
            : address_size_ (0)
            , number_cores_ (0)
            , number_pages_ (0)
            , max_open_files_ (0)
            , memory_size_ (0)
            , page_size_ (0)
            , has_numa_ (false)
        {
            CollectSystemInformation ();
        }

        ~SystemInformation () {}

    private:
        void CollectSystemInformation ();

    public:
        std::string os_type_;
        std::string os_name_;
        std::string os_version_;
        std::string cpu_arch_;
        unsigned int address_size_;
        unsigned int number_cores_;
        unsigned int number_pages_;
        unsigned int max_open_files_;
        unsigned long long memory_size_;
        unsigned long long page_size_;
        bool has_numa_;
        std::string libc_version_;
        std::string kernel_version_;
        std::string cpu_frequncy_;
        std::string cpu_features_;
        std::string version_signature_;
    }; // SystemInformation

private:
    // Determine if the process is running with (cc)NUMA
    static bool CheckNumaEnabled ();

    static std::unique_ptr<std::mutex> kSystemInformationLock;
    static std::unique_ptr<ProcessInformation::SystemInformation> kSystemInfo;

    inline const SystemInformation& GetSystemInfo () const
    {
        return *kSystemInfo;
    }

private:
    pid_t pid_;
}; // ProcessInformation

} // namespace swift

#endif // __SWIFT_BASE_PROCESS_INFORMATION_H__