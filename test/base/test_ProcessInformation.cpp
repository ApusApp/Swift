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
    std::cout << pf.GetPidAsUint32 () << std::endl;
    ASSERT_TRUE (pf.GetPidAsUint32 () > 0);
    ASSERT_TRUE (pf.GetAddressSize () > 0);
    std::cout << pf.GetVirtualMemorySize () << std::endl;
    std::cout << pf.GetResidentSize () << std::endl;
    std::cout << pf.GetPidAsString () << std::endl;
    std::cout << pf.GetOsType () << std::endl;
    std::cout << pf.GetOsName () << std::endl;
    std::cout << pf.GetOsVersion () << std::endl;
    std::cout << pf.GetAddressSize () << std::endl;
    std::cout << pf.GetMemorySizeMB () << std::endl;
    std::cout << pf.GetNumberPages () << std::endl;
    std::cout << pf.GetNumberOfCores () << std::endl;
    std::cout << pf.GetMaxOpenFiles () << std::endl;
    std::cout << pf.GetArchitecture () << std::endl;
    std::cout << pf.HasNumaEnabled () << std::endl;
    std::cout << pf.GetLibcVersion () << std::endl;
    std::cout << pf.GetKernelVersion () << std::endl;
    std::cout << pf.GetCpuFrequncy () << std::endl;
    std::cout << pf.GetVersionSignature () << std::endl;
    std::cout << pf.GetParentProcessId () << std::endl;
    std::cout << pf.GetPageSize () << std::endl;
}