#include <iostream>

#include <swift/base/Timestamp.h>
#include <swift/base/MurmurHash3.h>
#include <swift/base/Date.h>
#include <swift/base/LinkedHashMap.h>

#include <gtest/gtest.h>

class test_LinkedHashMap : public testing::Test
{
public:
    test_LinkedHashMap () {}
    ~test_LinkedHashMap () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_LinkedHashMap, All)
{
    swift::LinkedHashMap<int, int> lhm;
    lhm.Set (100, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    lhm.Set (101, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    lhm.Set (102, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    lhm.Set (103, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    ASSERT_TRUE (100 == *lhm.Get (100, swift::LinkedHashMap<int, int>::MM_LAST));
    ASSERT_TRUE (100 == lhm.LastKey ());
    ASSERT_TRUE (100 == lhm.LastValue ());


    swift::LinkedHashMap<int, int> bigMap (32768);
    bigMap.Set (100, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    bigMap.Set (101, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    bigMap.Set (102, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    bigMap.Set (103, 100, swift::LinkedHashMap<int, int>::MM_FIRST);

    lhm.Get (100, swift::LinkedHashMap<int, int>::MM_FIRST);
    ASSERT_TRUE (100 == lhm.FirstKey ());
    ASSERT_TRUE (100 == lhm.FirstValue ());
    ASSERT_TRUE (100 == *lhm.Get (100, swift::LinkedHashMap<int, int>::MM_LAST));
    ASSERT_TRUE (100 == lhm.LastKey ());
    ASSERT_TRUE (100 == lhm.LastValue ());

    ASSERT_TRUE (100 == *lhm.Get (100, swift::LinkedHashMap<int, int>::MM_CURRENT));
    ASSERT_TRUE (100 == lhm.LastKey ());
    ASSERT_TRUE (100 == lhm.LastValue ());
}

int main (int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS ();
}