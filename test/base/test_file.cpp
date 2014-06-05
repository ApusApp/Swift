#include <gtest/gtest.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <memory>

#include <swift/base/file.h>
#include <swift/base/stringpiece.h>

class test_File : public testing::Test
{
public:
    test_File () {}
    ~test_File () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_File, Open)
{
    char buf = 'a';
    swift::File f;
    EXPECT_EQ (-1, f.GetFd ());
    EXPECT_TRUE (f.Open ("/etc/hosts", O_RDONLY));
    EXPECT_NE (-1, f.GetFd ());
    EXPECT_EQ (1, ::read (f.GetFd (), &buf, 1));
    EXPECT_NE (buf, 'a');
    f.Close ();
    EXPECT_EQ (-1, f.GetFd ());
}

TEST_F (test_File, Release)
{
    swift::File in (STDOUT_FILENO, false);
    EXPECT_EQ (STDOUT_FILENO, in.Release ());
    EXPECT_EQ (-1, in.Release ());
}

namespace {
    void ExpectWouldBlock (ssize_t r) 
    {
        int savedErrno = errno;
        EXPECT_EQ (-1, r);
        EXPECT_EQ (EAGAIN, savedErrno);
    }

    void ExpectOK (ssize_t r) 
    {
        //int savedErrno = errno;
        EXPECT_LE (0, r);
    }
}  // namespace

TEST_F (test_File, IsOwnsFd)
{
    char buf = 'x';
    int p[2];
    ExpectOK (::pipe (p));
    int flags = ::fcntl (p[0], F_GETFL);
    ExpectOK (flags);
    ExpectOK (::fcntl (p[0], F_SETFL, flags | O_NONBLOCK));
    ExpectWouldBlock (::read (p[0], &buf, 1));
    {
        swift::File f (p[1]);
        EXPECT_EQ (p[1], f.GetFd ());
    }
    // Ensure that moving the file doesn't close it
    {
        swift::File f (p[1]);
        EXPECT_EQ (p[1], f.GetFd ());
        swift::File f1 (std::move (f));
        EXPECT_EQ (-1, f.GetFd ());
        EXPECT_EQ (p[1], f1.GetFd ());
    }
    ExpectWouldBlock (::read (p[0], &buf, 1));  // not closed
    {
        swift::File f (p[1], true);
        EXPECT_EQ (p[1], f.GetFd ());
    }
    ssize_t r = ::read (p[0], &buf, 1);  // eof
    ExpectOK (r);
    EXPECT_EQ (0, r);
    ::close (p[0]);
}

namespace {
    bool UnlinkFile (swift::File& f)
    {
        if (f) {
            char buf[32] = { '\0' };
            char name[2048] = { '0' };
            snprintf (buf, sizeof (buf), "/proc/self/fd/%d", f.GetFd ());
            if (-1 != ::readlink (buf, name, sizeof(name)-1)) {
                f.Close ();
                return 0 == ::unlink (name);
            }
        }

        return false;
    }
}

TEST_F (test_File, Truthy)
{
    {
        swift::File f = swift::File::Temporary ();
        SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT{
            UnlinkFile (f);
        };

        EXPECT_TRUE (bool (f));
        if (!f) {
            EXPECT_FALSE (true);
        }

        EXPECT_NE (-1, f.GetFd ());
    }
    
    EXPECT_FALSE (bool (swift::File ()));
    if (swift::File ()) {
        EXPECT_TRUE (false);
    }

    if (swift::File not_open = swift::File ()) {
        EXPECT_TRUE (false);
    }
}

TEST_F (test_File, Others)
{
    swift::File f;
    {
        f = swift::File::Temporary ();
        SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT{
            UnlinkFile (f);
        };

        EXPECT_NE (-1, f.GetFd ());
        EXPECT_EQ (0, f.GetFileSize ());
        EXPECT_EQ (0, f.GetPosition ());

        std::string buf = "abcdefghijklmnopqrstuvwxyz";
        EXPECT_EQ (buf.size (), f.Append (buf.c_str (), buf.size ()));
        EXPECT_EQ (buf.size (), f.GetFileSize ());
        EXPECT_EQ (0, f.GetPosition ());
        EXPECT_TRUE (f.Truncate (0));
        EXPECT_EQ (0, f.GetFileSize ());
        EXPECT_EQ (buf.size (), f.Write (buf.c_str (), buf.size ()));
        EXPECT_EQ (buf.size (), f.GetPosition ());
        EXPECT_EQ (buf.size (), f.GetFileSize ());
        EXPECT_EQ (buf.size (), f.Append (buf.c_str (), buf.size ()));
        EXPECT_EQ (2 * buf.size (), f.GetFileSize ());

        std::unique_ptr<char> p (new char[buf.size () + 1]);
        memset (p.get (), 0, buf.size () + 1);
        EXPECT_TRUE (f.SetPosition (0));
        EXPECT_EQ (f.Read (p.get (), buf.size ()), buf.size());
        swift::StringPiece s (p.get ());
        EXPECT_EQ (s.size (), buf.size ());
        EXPECT_EQ (s.ToString (), buf);
        f.FlushData ();
        f.Flush ();
        EXPECT_TRUE (f.Truncate (s.size ()));
        EXPECT_EQ (s.size (), f.GetFileSize ());
        EXPECT_EQ (s.size (), f.PWrite (s.data (), s.size (), f.GetPosition ()));
        EXPECT_EQ (s.size (), f.GetPosition ());
        memset (p.get (), 0, s.size () + 1);
        EXPECT_EQ (5, f.PRead (p.get (), 5, 5));
        s.Set (p.get ());
        EXPECT_EQ (5, s.size ());
        EXPECT_EQ (s.ToString (), buf.substr (5, 5));

        int tmp_fd = f.GetFd ();
        {
            swift::File tmp;
            f.Swap (tmp);
            EXPECT_EQ (0, f.Write (buf.c_str (), buf.size ()));
            EXPECT_EQ (0, f.PWrite (buf.c_str (), buf.size (), 0));
            EXPECT_EQ (0, f.PRead (p.get (), 5, 5));
            EXPECT_EQ (0, f.Read (p.get (), 5));

            int fd = tmp.Release ();
            EXPECT_EQ (tmp_fd, fd);
            EXPECT_TRUE (tmp.Close ());
        }
        
        swift::File tmp (tmp_fd, true);
        swift::Swap (tmp, f);
        EXPECT_EQ (tmp_fd, f.GetFd ());
        swift::File dup_file = f.Dup ();
        EXPECT_NE (dup_file.GetFd (), f.GetFd ());
        EXPECT_TRUE (dup_file.Truncate (10));
        EXPECT_EQ (10, f.GetFileSize ());
    }

    EXPECT_EQ (0, f.GetFileSize ());
}

TEST_F (test_File, Lock)
{
    swift::File f = swift::File::Temporary ();
    SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT{
        UnlinkFile (f);
    };

    EXPECT_TRUE (f.try_lock ());
    EXPECT_TRUE (f.try_lock ());
    f.unlock ();
    EXPECT_TRUE (f.try_lock_shared ());
    f.unlock_shared ();
    f.lock ();
    f.unlock ();

    // TO DO
    // multi process access a file with lock
}
