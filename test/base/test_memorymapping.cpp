#include <gtest/gtest.h>
#include <cstdlib>

#include <swift/base/memorymapping.h>

class test_MemoryMapping : testing::Test
{
public:
    test_MemoryMapping() { }
    ~test_MemoryMapping() { }
};

static const double kPI = 3.1415926;

TEST(test_MemoryMapping, Basic)
{
    swift::File f = swift::File::Temporary();
    {
        swift::MemoryMapping m(f.GetFd(), 0, sizeof(double), swift::MemoryMapping::Writable());
        double volatile* d = m.AsWritableBuffer<double>().buf;
        *d = 37 * kPI;
    }

    {
        swift::MemoryMapping m(f.GetFd(), 0, 3);
        EXPECT_EQ(0, m.AsReadableBuffer<int>().length);
    }

    {
        swift::MemoryMapping m(f.GetFd(), 0, sizeof(double));
        const double volatile* d = m.AsReadableBuffer<double>().buf;
        EXPECT_EQ(*d, 37 * kPI);
    }

    std::string s = "hello world";
    EXPECT_EQ(s.size(), f.Write(s.data(), s.size()));
    {
        swift::MemoryMapping m(f.GetFd());
        EXPECT_EQ(s, m.GetData().data());
    }
    {
        swift::MemoryMapping m(f.GetFd(), 1, 2);
        EXPECT_EQ('e', m.GetData().data()[0]);
        EXPECT_EQ('l', m.GetData().data()[1]);
        EXPECT_EQ(2, m.GetData().size());
    }
    {
        swift::MemoryMapping m(f.GetFd());
        EXPECT_EQ(s, m.GetData().data());
    }

    // Anonymous
    {
        swift::MemoryMapping::Options opt;
        opt.SetWritable(true);
        opt.SetShared(true);
        opt.SetGrow(false);
        opt.SetPrefault(false);
        opt.SetShared(false);
        opt.SetPageSize(::sysconf(_SC_PAGESIZE));
        // Anonymous mmap will set the file to 0
        swift::MemoryMapping m(swift::MemoryMapping::AnonymousType::ANONYMOUS_TYPE,
                               sizeof(int),
                               opt);
        EXPECT_EQ(-1, m.GetFd());
        EXPECT_EQ(sizeof(int), m.GetData().size());
        swift::MemoryMapping::Buffer<int> buffer = m.AsWritableBuffer<int>();
        EXPECT_NE(buffer.buf, nullptr);
        (*buffer.buf)++;
        EXPECT_EQ(1, *buffer.buf);
    }
}

TEST(test_MemoryMapping, Lock)
{
    swift::File f = swift::File::Temporary();
    swift::MemoryMapping m(swift::File(f.GetFd()));
    EXPECT_TRUE(m.mlock(swift::MemoryMapping::LockMode::LOCK_MODE_LOCK));
    EXPECT_TRUE(m.IsLocked());
    EXPECT_EQ(0, m.GetData().size());
    // Can lock mutil times at the same process
    EXPECT_TRUE(m.mlock(swift::MemoryMapping::LockMode::LOCK_MODE_TRY_LOCK));
    m.munlock();
    EXPECT_TRUE(m.mlock(swift::MemoryMapping::LockMode::LOCK_MODE_TRY_LOCK));
    m.munlock(true);
}

TEST(test_MemoryMapping, DoublyMapped)
{
    swift::File f = swift::File::Temporary();
    swift::MemoryMapping mw(f.GetFd(), 0, sizeof(double), swift::MemoryMapping::Writable());
    swift::MemoryMapping mr(f.GetFd(), 0, sizeof(double));

    double volatile* dw = mw.AsWritableBuffer<double>().buf;
    const double volatile* dr = mr.AsReadableBuffer<double>().buf;
    EXPECT_NE(dw, dr);
    *dw = 42 * kPI;
    EXPECT_EQ(*dr, 42 * kPI);
    *dw = 43 * kPI;
    EXPECT_EQ(*dr, 43 * kPI);
}

TEST(test_MemoryMapping, Copy)
{
    swift::File f;
    EXPECT_TRUE(f.Open("test_MemoryMapping_01"));
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(sizeof(i), f.PWrite(reinterpret_cast<const char*>(&i), sizeof(i), sizeof(i) * i));
    }
    f.Close();

    swift::MemoryMapping::MMapFileCopy("test_MemoryMapping_01", "test_MemoryMapping_02");
    swift::MemoryMapping m("test_MemoryMapping_02", 0, sizeof(int) * 100);
    swift::MemoryMapping::Buffer<int> buffer = m.AsReadableBuffer<int>();
    EXPECT_EQ(buffer.length, 100);
    int* buf = buffer.buf;
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(i, buf[i]);
    }

    system("rm -rf test_MemoryMapping_0*");
}