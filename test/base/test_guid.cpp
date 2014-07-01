#include <gtest/gtest.h>
#include <swift/base/guid.h>

#include <thread>
#include <set>
#include <vector>
#include <mutex>

class test_Guid : public testing::Test
{
public:
    test_Guid();
    ~test_Guid();
};

TEST(test_Guid, Zeroes)
{
    uint64_t bytes[] = { 0, 0 };
    std::string guid;
    EXPECT_TRUE(swift::Guid::RandomDataToGuidString(bytes, guid));
    EXPECT_EQ(guid, "00000000-0000-0000-0000-000000000000");
}

TEST(test_Guid, Correctly)
{
    uint64_t bytes[] = { 0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL };
    std::string guid;
    EXPECT_TRUE(swift::Guid::RandomDataToGuidString(bytes, guid));
    EXPECT_EQ(guid, "01234567-89AB-CDEF-FEDC-BA9876543210");

    for (int i = 0; i < 100; ++i) {
        std::move(swift::Guid::Generate(guid));
        EXPECT_TRUE(swift::Guid::IsValid(guid));
    }
}

TEST(test_Guid, Uniqueness)
{
    std::string guid1;
    std::string guid2;
    for (int i = 0; i < 100; ++i) {
        swift::Guid::Generate(guid1);
        swift::Guid::Generate(guid2);
        EXPECT_NE(guid1, guid2);
        EXPECT_EQ(guid1.size(), 36);
        EXPECT_EQ(guid2.size(), 36);
    }

    std::mutex mu;
    std::set<std::string> ss;
    std::vector<std::thread> ths;
    for (int i = 0; i < 4; ++i) {
        ths.push_back(std::move(std::thread([&mu, &ss]{
            std::string guid;
            for (int i = 0; i < 10000; ++i) {
                EXPECT_TRUE(swift::Guid::Generate(guid));
                EXPECT_TRUE(swift::Guid::IsValid(guid));
                mu.lock();
                auto ret = ss.insert(std::move(guid));
                mu.unlock();
                EXPECT_NE(ret.second, false);
            }
        })));
    }

    for (auto&& i : ths) {
        i.join();
    }
}

TEST(test_Guid, Others)
{
    swift::Guid guid;
    EXPECT_TRUE(guid.IsValid());
    std::string s = guid.ToString();
    EXPECT_TRUE(false == s.empty());
    EXPECT_EQ(s.size(), 36);

    swift::Guid str_guid (s);
    EXPECT_TRUE(str_guid.IsValid());
    EXPECT_EQ(s, str_guid.ToString());

    s = "01234567-89AB-CDEF-FEDC-BA";
    swift::Guid g(s);
    EXPECT_FALSE(g.IsValid());

    s = "01234567-89AB-CDEF-HEDC-BA9876543210";
    swift::Guid gg(s);
    EXPECT_FALSE(gg.IsValid());

    s = "00000000-0000-0000-HEDC-BA9876543210";
    swift::Guid ggg(s);
    EXPECT_FALSE(ggg.IsValid());

    swift::Guid gggg = guid;
    EXPECT_EQ(gggg, guid);
}
