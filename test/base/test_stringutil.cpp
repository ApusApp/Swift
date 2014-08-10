#include <gtest/gtest.h>
#include <swift/base/stringutil.h>

class test_StringUtil : public testing::Test
{
};

TEST(test_StringUtil, AsArray)
{
    std::string str;
    char* p = swift::StringUtil::AsArray(&str);
    EXPECT_EQ(nullptr, p);
    str = "abc";
    p = swift::StringUtil::AsArray(&str);
    EXPECT_FALSE(nullptr == p);
    EXPECT_EQ(*p, 'a');
    EXPECT_EQ(str.size(), strlen(p));
    p[0] = 'c';
    p[2] = 'a';
    EXPECT_EQ(str, "cba");
}

TEST(test_StringUtil, StartWithPrefix)
{
    std::string str("abc");
    EXPECT_EQ(true, swift::StringUtil::StartWithPrefix(str.c_str(), "a"));
    EXPECT_EQ(true, swift::StringUtil::StartWithPrefix(str.c_str(), "ab"));
    EXPECT_EQ(true, swift::StringUtil::StartWithPrefix(str.c_str(), "abc"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix(str.c_str(), "abd"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix(str.c_str(), "xxxxxx"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix(str.c_str(), ""));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix("", "xx"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix("", ""));

    EXPECT_EQ(true, swift::StringUtil::StartWithPrefix(str, "a"));
    EXPECT_EQ(true, swift::StringUtil::StartWithPrefix(str, "ab"));
    EXPECT_EQ(true, swift::StringUtil::StartWithPrefix(str, "abc"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix(str, "abd"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix(str, "xxxxxx"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix(str, ""));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix("", "xx"));
    EXPECT_EQ(false, swift::StringUtil::StartWithPrefix("", ""));
}

TEST(test_StringUtil, EndWithSuffix)
{
    std::string str("abc");
    EXPECT_EQ(true, swift::StringUtil::EndWithSuffix(str, "c"));
    EXPECT_EQ(true, swift::StringUtil::EndWithSuffix(str, "bc"));
    EXPECT_EQ(true, swift::StringUtil::EndWithSuffix(str, "abc"));
    EXPECT_EQ(false, swift::StringUtil::EndWithSuffix(str, "dbc"));
    EXPECT_EQ(false, swift::StringUtil::EndWithSuffix(str, "xxxxxx"));
    EXPECT_EQ(false, swift::StringUtil::EndWithSuffix(str, ""));
    EXPECT_EQ(false, swift::StringUtil::EndWithSuffix("", "xx"));
    EXPECT_EQ(false, swift::StringUtil::EndWithSuffix("", ""));
    EXPECT_EQ(true, swift::StringUtil::EndWithSuffix("abcdef", "f"));
}

TEST(test_StringUtil, Count)
{
    std::string str("aaaaaaaaa");
    EXPECT_EQ(str.size(), swift::StringUtil::Count(str.c_str(), 'a'));
    EXPECT_EQ(str.size(), swift::StringUtil::Count(str.c_str(), 'a', str.size() + 2));
    EXPECT_EQ(str.size() - 2, swift::StringUtil::Count(str.c_str() + 2, 'a', str.size()));
    EXPECT_EQ(str.size() - 2, swift::StringUtil::Count(str.c_str(), 'a', str.size() - 2));

    EXPECT_EQ(0, swift::StringUtil::Count("a", '\0', 10));
    EXPECT_EQ(0, swift::StringUtil::Count("", 's', 10));
    EXPECT_EQ(0, swift::StringUtil::Count("abcfes", 's', 5));
    EXPECT_EQ(0, swift::StringUtil::Count("sbcaef", 's', 0));
    EXPECT_EQ(1, swift::StringUtil::Count("sbcaef", 's', 1));
    EXPECT_EQ(1, swift::StringUtil::Count("abcsef", 's', 6));
    EXPECT_EQ(1, swift::StringUtil::Count("abcfes", 's', 10));
    EXPECT_EQ(1, swift::StringUtil::Count("sbcaef", 's', 10));
}

TEST(test_StringUtil, ToLower_ToUpper)
{
    std::string lower ("0123abcde()fgh@@ijkl&&mn**--==");
    std::string lower_1 ("0123abcde()fgh@@ijkl&&mn**--==");
    std::string upper ("0123ABCDE()FGH@@IJKL&&MN**--==");

    EXPECT_EQ(upper, swift::StringUtil::ToUpper(lower));
    EXPECT_EQ(lower_1, swift::StringUtil::ToLower(upper));
    lower.clear();
    EXPECT_EQ(lower, swift::StringUtil::ToLower(lower));
}

TEST(test_StringUtil, Strips)
{
    // Strip
    std::string str;
    EXPECT_EQ(std::string(), swift::StringUtil::Strip(str, "abc", 'x'));
    str = "abc";
    EXPECT_EQ("xxx", swift::StringUtil::Strip(str, "abc", 'x'));
    str = "abc";
    EXPECT_EQ("abc", swift::StringUtil::Strip(str, "xxx", 'x'));
    EXPECT_EQ("abc", swift::StringUtil::Strip(str, "", 'x'));
    swift::StringUtil::Strip(str, "b", '\0');
    EXPECT_EQ(str.size(), 3);
    EXPECT_EQ(str[1], '\0');
    EXPECT_EQ(str[2], 'c');
    EXPECT_EQ(1, strlen(str.c_str()));

    swift::StringUtil::Strip(str, "c", 'b');
    EXPECT_EQ(str.size(), 3);
    EXPECT_EQ(str[1], '\0');
    EXPECT_EQ(str[2], 'c');
    EXPECT_EQ(1, strlen(str.c_str()));

    // StripPrefix
    str = "abcdefgh";
    EXPECT_EQ(str, swift::StringUtil::StripPrefix(str, "xx"));
    EXPECT_EQ(str, swift::StringUtil::StripPrefix(str, ""));
    EXPECT_EQ("", swift::StringUtil::StripPrefix(str, str));
    EXPECT_EQ("bcdefgh", swift::StringUtil::StripPrefix(str, "a"));
    EXPECT_EQ("h", swift::StringUtil::StripPrefix(str, "abcdefg"));
    EXPECT_EQ("", swift::StringUtil::StripPrefix("", ""));
    EXPECT_EQ("", swift::StringUtil::StripPrefix("", "h"));

    // StripSuffix
    EXPECT_EQ(str, swift::StringUtil::StripSuffix(str, "xx"));
    EXPECT_EQ(str, swift::StringUtil::StripSuffix(str, ""));
    EXPECT_EQ("", swift::StringUtil::StripSuffix(str, str));
    EXPECT_EQ("abcdefg", swift::StringUtil::StripSuffix(str, "h"));
    EXPECT_EQ("a", swift::StringUtil::StripSuffix(str, "bcdefgh"));
    EXPECT_EQ("", swift::StringUtil::StripSuffix("", ""));
    EXPECT_EQ("", swift::StringUtil::StripSuffix("", "h"));
}

TEST(test_StringUtil, Trims)
{
    // Trim
    std::string str("a b  c");
    std::string str_1("\t \n \r a b  c  \r\t\n   ");
    EXPECT_EQ(str, swift::StringUtil::Trim(str_1));
    str_1 = "\t \n \r a b  c  \r\t\n   ";
    EXPECT_EQ(str, swift::StringUtil::Trim(str_1));
    str_1 = "\t \n \r a b  c  \r\t\n   ";
    EXPECT_EQ("a b  c  \r\t\n   ", swift::StringUtil::Trim(str_1, true, false));
    str_1 = "\t \n \r a b  c  \r\t\n   ";
    EXPECT_EQ("\t \n \r a b  c", swift::StringUtil::Trim(str_1, false, true));

    //TrimSpaces
    EXPECT_EQ("abc", swift::StringUtil::TrimSpaces(str_1));
}