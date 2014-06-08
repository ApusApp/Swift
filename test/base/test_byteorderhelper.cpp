#include <gtest/gtest.h>

#include <swift/base/byteorderhelper.h>

class test_ByteOrderHelper : public testing::Test
{
public:
    test_ByteOrderHelper () {}
    ~test_ByteOrderHelper () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_ByteOrderHelper, Set)
{
    uint8_t buf[8] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
    swift::ByteOrderHelper::Set8 (buf, 0, 0xfb);
    swift::ByteOrderHelper::Set8 (buf, 1, 0x12);
    EXPECT_EQ (0xfb, buf[0]);
    EXPECT_EQ (0x12, buf[1]);
    swift::ByteOrderHelper::SetBigEndian16 (buf, 0x1234);
    EXPECT_EQ (0x12, buf[0]);
    EXPECT_EQ (0x34, buf[1]);
    swift::ByteOrderHelper::SetLittleEndian16 (buf, 0x1234);
    EXPECT_EQ (0x34, buf[0]);
    EXPECT_EQ (0x12, buf[1]);
    swift::ByteOrderHelper::SetBigEndian32 (buf, 0x12345678);
    EXPECT_EQ (0x12, buf[0]);
    EXPECT_EQ (0x34, buf[1]);
    EXPECT_EQ (0x56, buf[2]);
    EXPECT_EQ (0x78, buf[3]);
    swift::ByteOrderHelper::SetLittleEndian32 (buf, 0x12345678);
    EXPECT_EQ (0x78, buf[0]);
    EXPECT_EQ (0x56, buf[1]);
    EXPECT_EQ (0x34, buf[2]);
    EXPECT_EQ (0x12, buf[3]);
    swift::ByteOrderHelper::SetBigEndian64 (buf, 0x0123456789abcdefULL);
    EXPECT_EQ (0x01, buf[0]);
    EXPECT_EQ (0x23, buf[1]);
    EXPECT_EQ (0x45, buf[2]);
    EXPECT_EQ (0x67, buf[3]);
    EXPECT_EQ (0x89, buf[4]);
    EXPECT_EQ (0xab, buf[5]);
    EXPECT_EQ (0xcd, buf[6]);
    EXPECT_EQ (0xef, buf[7]);
    swift::ByteOrderHelper::SetLittleEndian64 (buf, 0x0123456789abcdefULL);
    EXPECT_EQ (0xef, buf[0]);
    EXPECT_EQ (0xcd, buf[1]);
    EXPECT_EQ (0xab, buf[2]);
    EXPECT_EQ (0x89, buf[3]);
    EXPECT_EQ (0x67, buf[4]);
    EXPECT_EQ (0x45, buf[5]);
    EXPECT_EQ (0x23, buf[6]);
    EXPECT_EQ (0x01, buf[7]);
}

TEST_F (test_ByteOrderHelper, Get) {
    uint8_t buf[8];
    buf[0] = 0x01u;
    buf[1] = 0x23u;
    buf[2] = 0x45u;
    buf[3] = 0x67u;
    buf[4] = 0x89u;
    buf[5] = 0xabu;
    buf[6] = 0xcdu;
    buf[7] = 0xefu;
    EXPECT_EQ (0x01u, swift::ByteOrderHelper::Get8 (buf, 0));
    EXPECT_EQ (0x23u, swift::ByteOrderHelper::Get8 (buf, 1));
    EXPECT_EQ (0x0123u, swift::ByteOrderHelper::GetBigEndian16 (buf));
    EXPECT_EQ (0x2301u, swift::ByteOrderHelper::GetLittleEndian16 (buf));
    EXPECT_EQ (0x01234567u, swift::ByteOrderHelper::GetBigEndian32 (buf));
    EXPECT_EQ (0x67452301u, swift::ByteOrderHelper::GetLittleEndian32 (buf));
    EXPECT_EQ (0x0123456789abcdefULL, swift::ByteOrderHelper::GetBigEndian64 (buf));
    EXPECT_EQ (0xefcdab8967452301ULL, swift::ByteOrderHelper::GetLittleEndian64 (buf));
}

TEST_F (test_ByteOrderHelper, NetworkAndHost) {
    uint16_t v16 = 1;
    uint32_t v32 = 1;
    uint64_t v64 = 1;

    EXPECT_EQ (v16, swift::ByteOrderHelper::NetworkToHost16 (swift::ByteOrderHelper::HostToNetwork16 (v16)));
    EXPECT_EQ (v32, swift::ByteOrderHelper::NetworkToHost32 (swift::ByteOrderHelper::HostToNetwork32 (v32)));
    EXPECT_EQ (v64, swift::ByteOrderHelper::NetworkToHost64 (swift::ByteOrderHelper::HostToNetwork64 (v64)));

    if (swift::ByteOrderHelper::IsBigEndianHost ()) {
        // The host is the network (big) endian.
        EXPECT_EQ (v16, swift::ByteOrderHelper::HostToNetwork16 (v16));
        EXPECT_EQ (v32, swift::ByteOrderHelper::HostToNetwork32 (v32));
        EXPECT_EQ (v64, swift::ByteOrderHelper::HostToNetwork64 (v64));

        // GetBigEndian converts big endian to little endian here.
        EXPECT_EQ (v16 >> 8, swift::ByteOrderHelper::GetBigEndian16 (&v16));
        EXPECT_EQ (v32 >> 24, swift::ByteOrderHelper::GetBigEndian32 (&v32));
        EXPECT_EQ (v64 >> 56, swift::ByteOrderHelper::GetBigEndian64 (&v64));
    }
    else {
        // The host is little endian.
        EXPECT_NE (v16, swift::ByteOrderHelper::HostToNetwork16 (v16));
        EXPECT_NE (v32, swift::ByteOrderHelper::HostToNetwork32 (v32));
        EXPECT_NE (v64, swift::ByteOrderHelper::HostToNetwork64 (v64));

        // GetBigEndian converts little endian to big endian here.
        EXPECT_EQ (swift::ByteOrderHelper::GetBigEndian16 (&v16), swift::ByteOrderHelper::HostToNetwork16 (v16));
        EXPECT_EQ (swift::ByteOrderHelper::GetBigEndian32 (&v32), swift::ByteOrderHelper::HostToNetwork32 (v32));
        EXPECT_EQ (swift::ByteOrderHelper::GetBigEndian64 (&v64), swift::ByteOrderHelper::HostToNetwork64 (v64));

        // GetBigEndian converts little endian to big endian here.
        EXPECT_EQ (v16 << 8, swift::ByteOrderHelper::GetBigEndian16 (&v16));
        EXPECT_EQ (v32 << 24, swift::ByteOrderHelper::GetBigEndian32 (&v32));
        EXPECT_EQ (v64 << 56, swift::ByteOrderHelper::GetBigEndian64 (&v64));
    }
}
