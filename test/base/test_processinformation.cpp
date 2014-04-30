#include <gtest/gtest.h>
#include <iostream>
#include <swift/base/processinformation.h>

class test_ProcessInformation : public testing::Test
{
public:
    test_ProcessInformation () {}
    ~test_ProcessInformation () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_ProcessInformation, All)
{
    swift::ProcessInformation::InitializeSystemInformation ();
    swift::ProcessInformation pf;
    std::cout << "pid:\t" << pf.GetPidAsUint32 () << std::endl;
    ASSERT_TRUE (pf.GetPidAsUint32 () > 0);
    ASSERT_TRUE (pf.GetAddressSize () > 0);
    std::cout << "VirtualMemorySize:\t" << pf.GetVirtualMemorySize () << std::endl;
    std::cout << "ResidentSize:\t" << pf.GetResidentSize () << std::endl;
    std::cout << "pid:\t" << pf.GetPidAsString () << std::endl;
    std::cout << "OsType:\t" << pf.GetOsType () << std::endl;
    std::cout << "OsName:\t" << pf.GetOsName () << std::endl;
    std::cout << "OsVersion:\t" << pf.GetOsVersion () << std::endl;
    std::cout << "AddressSize:\t" << pf.GetAddressSize () << std::endl;
    std::cout << "MemorySizeMB:\t" << pf.GetMemorySizeMB () << std::endl;
    std::cout << "NumberPages:\t" << pf.GetNumberPages () << std::endl;
    std::cout << "NumberOfCores:\t" << pf.GetNumberOfCores () << std::endl;
    std::cout << "MaxOpenFiles:\t" << pf.GetMaxOpenFiles () << std::endl;
    std::cout << "Architecture:\t" << pf.GetArchitecture () << std::endl;
    std::cout << "NumaEnabled:\t" << pf.HasNumaEnabled () << std::endl;
    std::cout << "LibcVersion:\t" << pf.GetLibcVersion () << std::endl;
    std::cout << "KernelVersion:\t" << pf.GetKernelVersion () << std::endl;
    std::cout << "CpuFrequncy:\t" << pf.GetCpuFrequncy () << std::endl;
    std::cout << "VersionSignature:\t" << pf.GetVersionSignature () << std::endl;
    std::cout << "ParentProcessId:\t" << pf.GetParentProcessId () << std::endl;
    std::cout << "PageSize:\t" << pf.GetPageSize () << std::endl;
}