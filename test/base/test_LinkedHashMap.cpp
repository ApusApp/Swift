#include <iostream>
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

    ASSERT_TRUE (0 == lhm.Count ());
    lhm.Set (100, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    lhm.Set (101, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    lhm.Set (102, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    lhm.Set (103, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    ASSERT_TRUE (100 == *lhm.Get (100, swift::LinkedHashMap<int, int>::MM_LAST));
    ASSERT_TRUE (100 == lhm.LastKey ());
    ASSERT_TRUE (100 == lhm.LastValue ());
    int count = 0;
    for (auto it = lhm.Begin (); it != lhm.End (); ++it) {
        ++count;
    }
    ASSERT_TRUE (4 == count);

    auto it = lhm.Find (102);
    ASSERT_TRUE (102 == it.Key ());
    ASSERT_TRUE (100 == it.Value ());

    ASSERT_TRUE (lhm.Remove (102));
    ASSERT_FALSE (lhm.Remove (102));
    ASSERT_TRUE (nullptr == lhm.Get (102, swift::LinkedHashMap<int, int>::MM_LAST));

    swift::LinkedHashMap<int, int> bigMap (32768);
    bigMap.Set (100, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    bigMap.Set (101, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    bigMap.Set (102, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
    bigMap.Set (103, 100, swift::LinkedHashMap<int, int>::MM_FIRST);

    bigMap.Get (100, swift::LinkedHashMap<int, int>::MM_FIRST);
    ASSERT_TRUE (100 == bigMap.FirstKey ());
    ASSERT_TRUE (100 == bigMap.FirstValue ());
    ASSERT_TRUE (100 == *bigMap.Get (100, swift::LinkedHashMap<int, int>::MM_LAST));
    ASSERT_TRUE (100 == bigMap.LastKey ());
    ASSERT_TRUE (100 == bigMap.LastValue ());

    ASSERT_TRUE (100 == *bigMap.Get (100, swift::LinkedHashMap<int, int>::MM_CURRENT));
    ASSERT_TRUE (100 == bigMap.LastKey ());
    ASSERT_TRUE (100 == bigMap.LastValue ());
}

