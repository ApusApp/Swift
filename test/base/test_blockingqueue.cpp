#include <iostream>
#include <thread>
#include <future>
#include <string>
#include <gtest/gtest.h>
#include <swift/base/blockingqueue.h>

class test_BlockingQueue : public testing::Test
{
public:
    test_BlockingQueue () {}
    ~test_BlockingQueue () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_BlockingQueue, Integer)
{
    swift::BlockingQueue<int> q;
    auto t1 = std::async (std::launch::async, [&q] () {
        for (int i = 0; i < 10; ++i) {
            q.Put (i);
        }
    });

    auto t2 = std::async (std::launch::async, [&q] () {
        while (q.Size ()) {
            ASSERT_TRUE (q.Take () < 10);
        }
    });

    auto t3 = std::async (std::launch::async, [&q] () {
        while (q.Size ()) {
            ASSERT_TRUE (q.Take () < 10);
        }
    });

    t1.wait ();
    t2.wait ();
    t3.wait ();
}

class Data
{
public:
    Data (const char* data) : length_ (0), data_ (nullptr)
    {
        if (nullptr != data) {
            length_ = strlen (data);
            data_ = new char[length_ + 1];
            memcpy (data_, data, length_);
            data_[length_] = '\0';
        }
    }

    ~Data ()
    {
        if (nullptr != data_) {
            delete [] data_;
            data_ = nullptr;
            length_ = 0;
        }
    }

    Data (const Data& rhs)
    {
        if (this != &rhs) {
            length_ = rhs.length_;
            if (length_) {
                data_ = new char[length_ + 1];
                memcpy (data_, rhs.data_, length_);
                data_[length_] = '\0';
            }
        }
    }

    Data (Data&& rhs)
    {
        if (this != &rhs) {
            length_ = rhs.length_;
            if (length_) {
                data_ = rhs.data_;
                rhs.data_ = nullptr;
                rhs.length_ = 0;
            }
        }
    }

    const Data& operator= (const Data& rhs)
    {
        if (this != &rhs) {
            if (data_) {
                delete [] data_;
            }
            length_ = rhs.length_;
            if (length_) {
                data_ = new char[length_ + 1];
                memcpy (data_, rhs.data_, length_);
                data_[length_] = '\0';
            }
        }
        return *this;
    }

    const Data& operator= (Data&& rhs)
    {
        if (this != &rhs) {
            if (data_) {
                delete [] data_;
            }
            length_ = rhs.length_;
            if (length_ && data_ != nullptr) {
                data_ = rhs.data_;
                rhs.data_ = nullptr;
                rhs.length_ = 0;
            }
        }

        return *this;
    }

    bool operator== (const Data& rhs) const
    {
        if (length_ != rhs.length_) {
            return false;
        }

        return 0 == memcmp (data_, rhs.data_, length_);
    }

    const char* GetData () const
    {
        return data_;
    }

    size_t GetSize () const
    {
        return length_;
    }

private:
    int length_;
    char *data_;
};

TEST_F (test_BlockingQueue, Structure)
{
    Data a ("xxxxx");
    Data d ("abcde");
    d = a;
    
    Data c = a;
    c = d;

    swift::BlockingQueue<Data> q;
    q.Put (d);
    ASSERT_EQ (q.Size (), 1);
    ASSERT_EQ (q.IsEmpty (), false);
    q.Clear ();
    ASSERT_EQ (q.IsEmpty (), true);
    q.Put (d);
    q.Put (std::move (a));
    Data r = q.Take ();
    ASSERT_EQ (q.Size (), 1);

    ASSERT_TRUE (d == r);
    ASSERT_TRUE (c == r);
    ASSERT_TRUE (memcmp (r.GetData (), "xxxxx", r.GetSize ()) == 0);

    r = q.Take ();
    ASSERT_EQ (a.GetData (), nullptr);
    ASSERT_EQ (a.GetSize (), 0);
    ASSERT_EQ (strlen (r.GetData ()), r.GetSize ());
}
