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

#include <algorithm>
#include <limits>		// numeric_limits
#include <stdint.h>
#include <stdio.h>
#include <type_traits>	// for static_assert and is_arithmetic

#include "swift/base/LogStream.h"

namespace swift {
namespace detail {

static const char digits[] = "9876543210123456789";
static_assert (sizeof(digits) == 20, "sizeof(digits) must equal to 20");

static const char* zero = digits + 9;

const char digits_hex[] = "0123456789ABCDEF";
static_assert (sizeof(digits_hex) == 17, "sizeof(digitsHex) must equal to 17");

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
static size_t Convert (char buf[], const T value)
{
    T i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse (buf, p);

    return p - buf;
}

// uintptr_t: 
// Integer type capable of holding a value converted from a void pointer and then be 
// converted back to that type with a value that compares equal to the original pointer.
size_t ConvertHex (char buf[], uintptr_t value)
{
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = i % 16;
        i /= 16;
        *p++ = digits_hex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse (buf, p);

    return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;
} // namespace detail

// private
template <typename T>
void LogStream::FormatInteger (T v)
{
    if (buffer_.AvailSize () >= kMaxNumericSize) {
        size_t len = detail::Convert (buffer_.Current (), v);
        buffer_.Add (len);
    }
}

// public
LogStream& LogStream::operator<< (int v)
{
    FormatInteger (v);
    return *this;
}

// public
LogStream& LogStream::operator<< (unsigned int v)
{
    FormatInteger (v);
    return *this;
}

// public
LogStream& LogStream::operator<< (short v)
{
    *this << static_cast<int>(v);
    return *this;
}

// public
LogStream& LogStream::operator<< (unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

// public
LogStream& LogStream::operator<< (long v)
{
    FormatInteger (v);
    return *this;
}

// public
LogStream& LogStream::operator<< (unsigned long v)
{
    FormatInteger (v);
    return *this;
}

// public
LogStream& LogStream::operator<< (long long v)
{
    FormatInteger (v);
    return *this;
}

// public
LogStream& LogStream::operator<< (unsigned long long v)
{
    FormatInteger (v);
    return *this;
}

// public
LogStream& LogStream::operator<< (const void* v)
{
    // uintptr_t: 
    // Integer type capable of holding a value converted from a void pointer and then be 
    // converted back to that type with a value that compares equal to the original pointer.
    uintptr_t p = reinterpret_cast<uintptr_t>(v);
    if (buffer_.AvailSize () >= kMaxNumericSize) {
        char* buf = buffer_.Current ();
        buf[0] = '0';
        buf[1] = 'X';
        size_t len = detail::ConvertHex (buf + 2, p);
        buffer_.Add (len + 2);
    }

    return *this;
}

// public
LogStream& LogStream::operator<< (float v)
{
    *this << static_cast<double>(v);
    return *this;
}

// public
LogStream& LogStream::operator<< (double v)
{
    if (buffer_.AvailSize () >= kMaxNumericSize) {
        int len = snprintf (buffer_.Current (), kMaxNumericSize, "%0.12g", v);
        buffer_.Add (len);
    }
    return *this;
}

// private
void LogStream::StaticCheck ()
{
    static_assert (kMaxNumericSize - 10 > std::numeric_limits<double>::digits10, "");
    static_assert (kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10, "");
    static_assert (kMaxNumericSize - 10 > std::numeric_limits<long>::digits10, "");
    static_assert (kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10, "");
}

template<typename T>
Format::Format (const char* fmt, T val)
{
    // is_arithmetic: Determine whether T is an arithmetic type
    static_assert (std::is_arithmetic<T>::value == true, "typename T must be a arithmetic type");

    length_ = snprintf (buf_, sizeof(buf_), fmt, val);
    assert (static_cast<size_t>(length_) < sizeof(buf_));
}

// Explicit instantiations
template Format::Format (const char* fmt, char);
template Format::Format (const char* fmt, short);
template Format::Format (const char* fmt, unsigned short);
template Format::Format (const char* fmt, int);
template Format::Format (const char* fmt, unsigned int);
template Format::Format (const char* fmt, long);
template Format::Format (const char* fmt, unsigned long);
template Format::Format (const char* fmt, long long);
template Format::Format (const char* fmt, unsigned long long);
template Format::Format (const char* fmt, float);
template Format::Format (const char* fmt, double);

} // namespace swift
