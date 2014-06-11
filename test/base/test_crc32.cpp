#include <gtest/gtest.h>
#include <string.h>

#include <swift/base/crc32.h>
#include <swift/base/stringpiece.h>

class test_Crc32 : public testing::Test
{
public:
    test_Crc32 () {}
    ~test_Crc32 () {}
};

TEST_F (test_Crc32, All)
{
    uint32_t n = swift::Crc32::ComputeCrc32 ("", 0);
    EXPECT_EQ (0, n);

    char buf[32];

    memset (buf, 0, sizeof(buf));
    ASSERT_EQ (0x8a9136aa, swift::Crc32::ComputeCrc32 (buf, sizeof(buf)));

    memset (buf, 0xff, sizeof(buf));
    ASSERT_EQ (0x62a8ab43, swift::Crc32::ComputeCrc32 (buf, sizeof(buf)));

    for (int i = 0; i < 32; i++) {
        buf[i] = i;
    }
    ASSERT_EQ (0x46dd794e, swift::Crc32::ComputeCrc32 (buf, sizeof(buf)));

    for (int i = 0; i < 32; i++) {
        buf[i] = 31 - i;
    }
    ASSERT_EQ (0x113fdb5c, swift::Crc32::ComputeCrc32 (buf, sizeof(buf)));

    unsigned char data[48] = {
        0x01, 0xc0, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x14,
        0x00, 0x00, 0x00, 0x18,
        0x28, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    ASSERT_EQ (0xd9963a56, swift::Crc32::ComputeCrc32 (reinterpret_cast<char*>(data), sizeof(data)));

    swift::StringPiece input ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    uint32_t c = 0;
    for (size_t i = 0; i < input.size (); ++i) {
        char cc = input[i];
        c = swift::Crc32::UpdateCrc32 (c, &cc, 1);
    }
    EXPECT_EQ (0x171A3F5FU, c);

    swift::StringPiece s ("123");
    n = swift::Crc32::ComputeCrc32 (s.data (), s.length ());
    EXPECT_EQ (0x884863D2, n);

}