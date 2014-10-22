/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "swift/base/fileutil.h"
#include "swift/base/stringpiece.h"

#include <deque>
#include <vector>
#include <string>
#include <string.h>
#include <gtest/gtest.h>

class Reader
{
public:
    Reader (off_t offset, std::string&& str, std::deque<ssize_t> spec)
        : offset_ (offset)
        , str_ (std::move (str))
        , data_ (str_.data (), str_.size ())
        , spec_ (std::move (spec))
    {
    }

    // write-like
    ssize_t operator()(int fd, void* buf, size_t count)
    {
        ssize_t n = NextSize ();
        if (n <= 0) {
            return n;
        }

        if (n > static_cast<ssize_t>(count)) {
            return 0;
        }

        memcpy (buf, data_.data (), n);
        data_.RemovePrefix (n);

        return n;
    }

    // pwrite-like
    ssize_t operator()(int fd, void* buf, size_t count, off_t offset)
    {
        EXPECT_EQ (offset_, offset);
        return operator() (fd, buf, count);
    }

    // writev-like
    ssize_t operator()(int fd, const iovec* iov, int count)
    {
        ssize_t n = NextSize ();
        if (n <= 0) {
            return n;
        }

        ssize_t remaining = n;
        for (; count != 0 && remaining != 0; ++iov, --count) {
            ssize_t len = std::min (remaining, ssize_t(iov->iov_len));
            memcpy (iov->iov_base, data_.data (), len);
            data_.RemovePrefix (len);
            remaining -= len;
        }

        if (remaining != 0) {
            return 0;
        }

        return n;
    }

    // pwritev-like
    ssize_t operator()(int fd, const iovec* iov, int count, off_t offset)
    {
        EXPECT_EQ (offset_, offset);
        return operator() (fd, iov, count);
    }

    const std::deque<ssize_t> spec () const { return spec_; }

private:
    ssize_t NextSize ()
    {
        if (spec_.empty ()) {
            return 0;
        }

        ssize_t n = spec_.front ();
        spec_.pop_front ();
        if (n <= 0) {
            if (-1 == n) {
                errno = EIO;
            }
            spec_.clear ();
        }
        else {
            offset_ += n;
        }

        return n;
    }

    off_t offset_;
    std::string str_;
    swift::StringPiece data_;
    std::deque<ssize_t> spec_;
};

class test_FileUtil : public testing::Test
{
public:
    test_FileUtil ()
        : in_ ("1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
    {
        EXPECT_EQ (62, in_.size ());

        readers_.emplace_back (0, ReaderFactory ({ 0 }));
        readers_.emplace_back (62, ReaderFactory ({ 62 }));
        readers_.emplace_back (62, ReaderFactory ({ 62, -1 }));  // error after end (not called)
        readers_.emplace_back (61, ReaderFactory ({ 61, 0 }));
        readers_.emplace_back (-1, ReaderFactory ({ 61, -1 }));  // error before end
        readers_.emplace_back (62, ReaderFactory ({ 31, 31 }));
        readers_.emplace_back (62, ReaderFactory ({ 1, 10, 20, 10, 1, 20 }));
        readers_.emplace_back (61, ReaderFactory ({ 1, 10, 20, 10, 20, 0 }));
        readers_.emplace_back (41, ReaderFactory ({ 1, 10, 20, 10, 0 }));
        readers_.emplace_back (-1, ReaderFactory ({ 1, 10, 20, 10, 20, -1 }));
    }

    ~test_FileUtil () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }

protected:
    Reader ReaderFactory (std::deque<ssize_t> spec)
    {
        return Reader (42, std::move (in_.ToString ()), std::move (spec));
    }

    swift::StringPiece in_;
    std::vector<std::pair<size_t, Reader>> readers_;
};

TEST_F (test_FileUtil, Read)
{
    for (auto& p : readers_) {
        std::string out (in_.size (), '\0');
        EXPECT_EQ (p.first,
            swift::detail::WrapFileOpFuncT (p.second, 0, &out[0], out.size ()));
        if (-1 != static_cast<int>(p.first)) {
            EXPECT_EQ (in_.substr (0, p.first), out.substr (0, p.first));
        }
    }
}

TEST_F (test_FileUtil, PRead)
{
    for (auto& p : readers_) {
        std::string out (in_.size (), '\0');
        EXPECT_EQ (p.first,
            swift::detail::WrapFileOpFuncT (p.second, 0, &out[0], out.size (), off_t (42)));
        if (static_cast<int>(p.first) != -1) {
            EXPECT_EQ (in_.substr (0, p.first), out.substr (0, p.first));
        }
    }
}

class IovecBuffers
{
public:
    explicit IovecBuffers (std::initializer_list<size_t> sizes)
    {
        iov_.reserve (sizes.size ());
        for (auto& s : sizes) {
            buffers_.push_back (std::string (s, '\0'));
        }

        for (auto& b : buffers_) {
            iovec iov;
            iov.iov_base = &b[0];
            iov.iov_len = b.size ();
            iov_.push_back (iov);
        }
    }

    std::vector<iovec> GetIov () const
    {
        return iov_;
    }

    size_t Size () const
    {
        size_t s = 0;
        for (auto& b : buffers_) {
            s += b.size ();
        }

        return s;
    }

    std::string Join () const
    {
        std::string str;
        for (auto& b : buffers_) {
            str.append (b);
        }
        return str;
    }

private:
    std::vector<std::string> buffers_;
    std::vector<iovec> iov_;
};

TEST_F (test_FileUtil, Readv)
{
    for (auto& p : readers_) {
        IovecBuffers buf ({ 12, 19, 31 });
        ASSERT_EQ (62, buf.Size ());

        auto iov = buf.GetIov ();
        EXPECT_EQ (p.first, 
            swift::detail::WrapFileOpVFuncT (p.second, 0, iov.data (), iov.size ()));
        if (-1 != static_cast<int>(p.first)) {
            EXPECT_EQ (in_.substr (0, p.first), buf.Join ().substr (0, p.first));
        }
    }
}

TEST_F (test_FileUtil, PReadv) {
    for (auto& p : readers_) {
        IovecBuffers buf ({ 12, 19, 31 });
        ASSERT_EQ (62, buf.Size ());

        auto iov = buf.GetIov ();
        EXPECT_EQ (p.first,
            swift::detail::WrapFileOpVFuncT (p.second, 0, iov.data (), iov.size (), off_t (42)));
        if (-1 != static_cast<int>(p.first)) {
            EXPECT_EQ (in_.substr (0, p.first), buf.Join ().substr (0, p.first));
        }
    }
}

TEST_F (test_FileUtil, ReadFile)
{
    const std::string temp_file = "/tmp/test_file_util_file___";
    const std::string empty_file = "/tmp/test_file_util_empty_file___";

    SCOPE_GUARD_VARIABLES_AUTO_RUNNING_ON_EXIT{
        unlink (temp_file.c_str ());
        unlink (empty_file.c_str ());
    };

    FILE *f = fopen (empty_file.c_str (), "wb");
    EXPECT_NE (nullptr, f);
    EXPECT_EQ (0, fclose (f));
    f = fopen (temp_file.c_str (), "wb");
    EXPECT_NE (nullptr, f);
    EXPECT_EQ (3, fwrite ("bar", 1, 3, f));
    EXPECT_EQ (0, fclose (f));

    {
        std::string contents;
        EXPECT_TRUE (swift::fileutil::ReadFile (empty_file.c_str (), contents));
        EXPECT_EQ (contents, "");
        EXPECT_TRUE (swift::fileutil::ReadFile (temp_file.c_str (), contents, 0));
        EXPECT_EQ (contents, "");
        EXPECT_TRUE (swift::fileutil::ReadFile (temp_file.c_str (), contents, 2));
        EXPECT_EQ (contents, "ba");
        EXPECT_TRUE (swift::fileutil::ReadFile (temp_file.c_str (), contents));
        EXPECT_EQ (contents, "bar");
    }

    {
        std::vector<unsigned char> contents;
        EXPECT_TRUE (swift::fileutil::ReadFile (empty_file.c_str (), contents));
        EXPECT_EQ (std::vector<unsigned char> (), contents);
        EXPECT_TRUE (swift::fileutil::ReadFile (temp_file.c_str (), contents, 0));
        EXPECT_EQ (std::vector<unsigned char> (), contents);
        EXPECT_TRUE (swift::fileutil::ReadFile (temp_file.c_str (), contents, 2));
        EXPECT_EQ (std::vector<unsigned char> ({ 'b', 'a' }), contents);
        EXPECT_TRUE (swift::fileutil::ReadFile (temp_file.c_str (), contents));
        EXPECT_EQ (std::vector<unsigned char> ({ 'b', 'a', 'r' }), contents);
    }
}