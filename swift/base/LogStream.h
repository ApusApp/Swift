#ifndef __SWIFT_BASE_LOG_STREAM_H__
#define __SWIFT_BASE_LOG_STREAM_H__

#include <string.h>
#include <string>
#include <assert.h>

#include "swift/base/noncopyable.hpp"

namespace swift {
namespace detail {

const int kSmallBuffer = 4096;
const int kLargeBuffer = 4096 * 1024;

template <int SIZE>
class FixedBuffer : swift::noncopyable
{
public:
    FixedBuffer () : current_ (data_) {}

    ~FixedBuffer () {}

    void Append (const char* /*restrict*/ buf, size_t len)
    {
        if (static_cast<size_t>(AvailSize ()) > len) {
            memcpy (current_, buf, len);
            current_ += len;
        }
    }

    // return available size
    int AvailSize () const
    {
        return static_cast<int>(End () - current_);
    }

    const char* Data () const
    {
        return data_;
    }

    int Length () const
    {
        return static_cast<int>(current_ - data_);
    }

    char* Current ()
    {
        return current_;
    }

    void Add (size_t len)
    {
        current_ += len;
    }

    void Reset ()
    {
        current_ = data_;
    }

    void Bzero ()
    {
        ::bzero (data_, sizeof(data_));
    }

    std::string ToString () const
    {
        return std::string (data_, Length ());
    }

private:
    const char* End () const
    {
        return data_ + sizeof(data_);
    }

    char data_[SIZE];
    char *current_;
};

} // end of namespace detail

class LogStream : swift::noncopyable
{
    typedef LogStream self;
public:
    typedef detail::FixedBuffer<detail::kSmallBuffer> BufferType;

    LogStream () : buffer_ () {}
    ~LogStream () {}

    self& operator<< (bool v)
    {
        buffer_.Append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<< (short v);
    self& operator<< (unsigned short v);
    self& operator<< (int v);
    self& operator<< (unsigned int v);
    self& operator<< (long v);
    self& operator<< (unsigned long v);
    self& operator<< (long long v);
    self& operator<< (unsigned long long v);
    self& operator<< (const void* v);
    self& operator<< (float v);
    self& operator<< (double v);

    self& operator<< (char v)
    {
        buffer_.Append (&v, 1);
        return *this;
    }

    self& operator<< (unsigned char v)
    {
        return operator<< (static_cast<char>(v));
    }

    self& operator<< (const char* str)
    {
        if (str) {
            buffer_.Append (str, strlen (str));
        }
        else {
            buffer_.Append ("(null)", 6);
        }
        return *this;
    }

    self& operator<< (const unsigned char* str)
    {
        return operator<< (reinterpret_cast<const char*>(str));
    }

    self& operator<< (const std::string& str)
    {
        buffer_.Append (str.c_str (), str.size ());
        return *this;
    }

    void Append (const char* data, int len) 
    { 
        buffer_.Append (data, len); 
    }

    const BufferType& Buffer () const 
    { 
        return buffer_; 
    }

    void ResetBuffer() 
    { 
        buffer_.Reset (); 
    }

private:
    void StaticCheck ();

    template<typename T>
    void FormatInteger (T);

    BufferType buffer_;

    static const int kMaxNumericSize = 32;
}; // end of LogStream

class Format
{
public:
    template<typename T>
    Format (const char* fmt, T val);

    const char* Data() const 
    { 
        return buf_; 
    }

    int Length() const 
    {
        return length_;
    }

private:
    char buf_[32];
    int length_;
};

inline LogStream& operator<< (LogStream& s, const Format& fmt)
{
    s.Append (fmt.Data (), fmt.Length ());
    return s;
}

} // end of namespace swift

#endif // __SWIFT_BASE_LOG_STREAM_H__
