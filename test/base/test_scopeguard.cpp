#include "swift/base/scopeguard.h"
#include <gtest/gtest.h>
#include <functional>
#include <vector>

using std::vector;

class test_ScopeGuard : public testing::Test
{
public:
    test_ScopeGuard () {}
    ~test_ScopeGuard () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

int ReturnInt ()
{
    return 1;
}

class TestFunctor
{
public:
    explicit TestFunctor (int* ptr) : ptr_ (ptr)
    {

    }

    void operator() ()
    {
        ++*ptr_;
    }

private:
    int *ptr_;
};

TEST_F (test_ScopeGuard, DifferentWaysToBind)
{
    {
        swift::ScopeGuard guard = swift::MakeScopeGuard (ReturnInt);
        (void)guard;
    }

    vector<int> v;
    void (vector<int>::*push_back) (const int&) = &vector<int>::push_back;
    v.push_back (1);
    {
        swift::ScopeGuard guard = swift::MakeScopeGuard (std::bind (
            &vector<int>::pop_back, &v));
        (void)guard;
    }
    EXPECT_EQ (0, v.size ());

    {
        swift::ScopeGuard guard = swift::MakeScopeGuard (std::bind (
            push_back, &v, 2));
        (void)guard;
    }
    EXPECT_EQ (1, v.size ());

    {
        // v pass by lvalue
        swift::ScopeGuard guard = swift::MakeScopeGuard (std::bind (
            push_back, v, 2));
        (void)guard;
    }
    EXPECT_EQ (1, v.size ());

    {
        // v pass by ref
        swift::ScopeGuard guard = swift::MakeScopeGuard (std::bind (
            push_back, std::ref (v), 3));
        (void)guard;
    }
    EXPECT_EQ (2, v.size ());

    {
        // v pass into by ref
        swift::ScopeGuard guard = swift::MakeScopeGuard ([&v]() {
            v.push_back (4);
        });
        (void)guard;
    }
    EXPECT_EQ (3, v.size ());

    {
        // lambda with a copy of v
        swift::ScopeGuard guard = swift::MakeScopeGuard ([v]() mutable {
            v.push_back (5);
        });
        (void)guard;
    }
    EXPECT_EQ (3, v.size ());

    // test class object
    int n = 0;
    {
        TestFunctor f (&n);
        swift::ScopeGuard guard = swift::MakeScopeGuard (f);
        (void)guard;
    }
    EXPECT_EQ (1, n);

    {
        swift::ScopeGuard guard = swift::MakeScopeGuard (TestFunctor (&n));
        (void)guard;
    }
    EXPECT_EQ (2, n);

    // use auto instead of ScopeGuard
    {
        auto guard = swift::MakeScopeGuard (TestFunctor (&n));
        (void)guard;
    }
    EXPECT_EQ (3, n);

    {
        const auto& guard = swift::MakeScopeGuard (TestFunctor (&n));
        (void)guard;
    }
    EXPECT_EQ (4, n);
}

/**
* Add an integer to a vector if it was inserted into the
* db successfully. Here is a schematic of how you would accomplish
* this with scope guard.
*/
void TestUndoAction (bool failure) {
    vector<int64_t> v;
    { // defines a "mini" scope

        // be optimistic and insert this into memory
        v.push_back (1);

        // The guard is triggered to undo the insertion unless Dismiss() is called.
        swift::ScopeGuard guard = swift::MakeScopeGuard ([&] { v.pop_back (); });

        // Do some action; Use the failure argument to pretend
        // if it failed or succeeded.

        // if there was no failure, dismiss the undo guard action.
        if (!failure) {
            guard.Dismiss ();
        }
    } // all stack allocated in the mini-scope will be destroyed here.

    if (failure) {
        EXPECT_EQ (0, v.size ()); // the action failed => undo insertion
    }
    else {
        EXPECT_EQ (1, v.size ()); // the action succeeded => keep insertion
    }
}

TEST_F (test_ScopeGuard, UndoAction) {
    TestUndoAction (true);
    TestUndoAction (false);
}

TEST_F (test_ScopeGuard, SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT_) {
    int x = 0;
    {
        SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT{
            ++x;
        };
        EXPECT_EQ (0, x);
    }

    EXPECT_EQ (1, x);
}
