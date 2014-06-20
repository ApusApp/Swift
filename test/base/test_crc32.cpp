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

    swift::StringPiece input ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    uint32_t c = 0;
    for (size_t i = 0; i < input.size (); ++i) {
        char cc = input[i];
        c = swift::Crc32::UpdateCrc32 (c, &cc, 1);
    }
    EXPECT_EQ (0x171A3F5FU, c);

    std::string s ("123");
    n = swift::Crc32::ComputeCrc32 (s.data (), s.length ());
    EXPECT_EQ (0x884863D2, n);

    EXPECT_EQ(swift::Crc32::ComputeCrc32 ("ccc", 3),
              swift::Crc32::UpdateCrc32 (swift::Crc32::ComputeCrc32 ("c", 1), "cc", 2));
}