#include <string.h>
#include <memory>
#include <gtest/gtest.h>

#include <swift/base/stringpiece.h>

class test_StringPiece : public testing::Test
{
public:
    test_StringPiece () {}
    ~test_StringPiece () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_StringPiece, All)
{
    std::string str = "abcdefg";
    {
        swift::StringPiece sp (str);
        ASSERT_TRUE (sp.size () == str.size ());
        ASSERT_EQ (sp.data (), sp.c_str ());
        ASSERT_EQ (sp.data (), str.c_str ());
        ASSERT_TRUE (0 == memcmp (sp.data (), str.data (), str.size ()));
        ASSERT_EQ (sp.length (), sp.size ());
        ASSERT_EQ (sp.size (), str.size ());
        ASSERT_EQ (sp.empty (), false);

        sp.Set (str.c_str ());
        ASSERT_EQ (sp.size (), str.size ());
        ASSERT_TRUE (0 == memcmp (sp.data (), str.data (), str.size ()));

        sp.Set (str.data (), str.size ());
        ASSERT_EQ (sp.size (), str.size ());
        ASSERT_TRUE (0 == memcmp (sp.data (), str.data (), str.size ()));
        for (size_t i = 0; i < str.size (); ++i) {
            ASSERT_EQ (sp[i], str[i]);
        }

        ASSERT_EQ (sp.ToString (), str);
        sp.RemovePrefix (1);
        ASSERT_EQ (sp.ToString (), "bcdefg");
        ASSERT_EQ (sp.size (), str.size () - 1);
        sp.RemoveSuffix (1);
        ASSERT_EQ (sp.ToString (), "bcdef");
        ASSERT_EQ (sp.size (), str.size () - 2);

        sp.Set (nullptr);
        ASSERT_EQ (sp.data (), nullptr);
        ASSERT_TRUE (sp.size () == 0);
        ASSERT_TRUE (sp.ToString ().empty () == true);
        ASSERT_EQ (sp.compare (sp), 0);

        sp.Set (str.c_str ());
        swift::StringPiece s1 ("abcdefg");
        ASSERT_EQ (sp.compare (s1), 0);
        s1.RemovePrefix (2);
        ASSERT_TRUE (sp.compare (s1) < 0);
        s1.Set ("abcdefghi");
        ASSERT_TRUE (sp.compare (s1) < 0);
        s1.Set (nullptr);
        ASSERT_TRUE (sp.compare (s1) > 0);

        s1.Set ("abcdefghi");
        swift::StringPiece s2 = s1;
        ASSERT_EQ (s2.capacity (), s1.size ());
        ASSERT_EQ (s2.max_size (), s1.max_size ());

        ASSERT_EQ (s2.StartWith ("abc"), true);
        ASSERT_EQ (s2.StartWith ("cb"), false);
        ASSERT_EQ (s2.EndWith ("ghi"), true);
        ASSERT_EQ (s2.EndWith ("ohg"), false);

        std::string tmp;
        s2.CopyToString (&tmp);
        ASSERT_EQ (s2.ToString (), tmp);
        tmp.clear ();
        s2.AppendToString (&tmp);
        ASSERT_EQ (s2.ToString (), tmp);
        std::string tmp2 = tmp;
        s2.clear ();
        s2.AppendToString (&tmp);
        ASSERT_EQ (tmp2, tmp);

        s2.CopyToString (&tmp);
        ASSERT_EQ (s2.empty (), true);
        ASSERT_EQ (tmp.empty (), true);
    }

    {
        swift::StringPiece sp (str.c_str (), str.size ());
        swift::StringPiece sp1 (str.c_str ());
        ASSERT_EQ (sp, sp1);
    }

    {
        swift::StringPiece sp;
        swift::StringPiece sp1 (str.begin (), str.end ());
        sp = sp1;
        ASSERT_EQ (sp, sp1);
        ASSERT_EQ (sp.size (), str.size ());
        ASSERT_EQ (sp.ToString (), str);

        std::unique_ptr<std::string::value_type> ptr (new std::string::value_type[str.size () + 1]);
        memset (ptr.get (), '\0', str.size () + 1);
        sp.copy (ptr.get (), str.size ());
        ASSERT_EQ (sp.ToString (), std::string (ptr.get ()));
        ptr.reset ();

        {
            swift::StringPiece::const_iterator spit = sp.begin ();
            std::string::const_iterator sit = str.begin ();
            for (; spit != sp.end () && sit != str.end (); ++spit, ++sit) {
                ASSERT_EQ (*spit, *sit);
            }
        }
        
        {
            swift::StringPiece::const_reverse_iterator spit = sp.rbegin ();
            std::string::const_reverse_iterator sit = str.rbegin ();
            for (; spit != sp.rend () && sit != str.rend (); ++spit, ++sit) {
                ASSERT_EQ (*spit, *sit);
            }
        }

        ASSERT_EQ (sp.find ('c'), str.find ('c'));
        ASSERT_EQ (sp.find ('x'), swift::StringPiece::npos);
        ASSERT_EQ (sp.find ('c', sp.size ()), swift::StringPiece::npos);
        ASSERT_EQ (sp.find ('a', sp.size () + 10), swift::StringPiece::npos);
        ASSERT_EQ (sp.find (swift::StringPiece ("xx")), swift::StringPiece::npos);
        ASSERT_EQ (sp.find (swift::StringPiece ("def")), str.find ("def"));
        
        ASSERT_EQ (sp.rfind ('c'), sp.find ('c'));
        ASSERT_EQ (sp.rfind (swift::StringPiece ("def")), str.find ("def"));
    }

    {
        std::string s = "abcdabcdeabcdefghijklmnop123123321";
        swift::StringPiece sp (s);
        ASSERT_EQ (sp.find_first_of ('a'), 0);
        ASSERT_EQ (sp.find_first_of ('a', 4), 4);
        ASSERT_EQ (sp.find_first_of (swift::StringPiece ()), swift::StringPiece::npos);
        ASSERT_EQ (sp.find_first_of (swift::StringPiece ("123")), s.find_first_of ("123"));

        ASSERT_EQ (sp.find_first_not_of ("abcd", 4), 8);
        ASSERT_EQ (sp.find_first_not_of ('a', 4), 5);
        ASSERT_EQ (sp.find_first_not_of ("a", 4), 5);
        ASSERT_EQ (sp.find_first_not_of (swift::StringPiece ()), 0);
        ASSERT_EQ (sp.find_first_not_of ("abcd"), s.find_first_not_of ("abcd"));

        swift::StringPiece substr = sp.substr (3, 4);
        ASSERT_EQ (substr, "dabc");

        ASSERT_EQ (sp.find_last_of ('1'), s.size () - 1);
        ASSERT_EQ (sp.find_last_of ('x'), swift::StringPiece::npos);
        ASSERT_EQ (sp.find_last_of (swift::StringPiece ()), swift::StringPiece::npos);
        ASSERT_EQ (sp.find_last_of ("123"), s.find_last_of ("123"));
        ASSERT_EQ (sp.find_last_not_of ('1'), sp.size () - 2);
        ASSERT_EQ (sp.find_last_not_of ("1"), sp.size () - 2);
        ASSERT_EQ (sp.find_last_not_of (swift::StringPiece ()), sp.size () - 1);
        ASSERT_EQ (sp.find_last_not_of ("321"), s.find_last_not_of ("321"));
        std::cout << sp << std::endl;
        ASSERT_TRUE (sp == sp);
        ASSERT_FALSE (sp != sp);
        ASSERT_FALSE (sp > sp);
        ASSERT_TRUE (sp >= sp);
        ASSERT_FALSE (sp < sp);
        ASSERT_TRUE (sp <= sp);
    }
    
}