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

#include <string.h>
#include <algorithm>
#include <cassert>

#include "swift/base/bytebuffer.h"

namespace swift {

// public
ByteBuffer::ByteBuffer () 
    : buffer_ (nullptr)
    , size_ (kDefaultSize)
    , start_ (0)
    , end_ (0)
    , version_ (0)
    , byte_order_ (ByteOrder::BYTE_ORDER_HOST)
{
    Initialize (nullptr);
}

// public
ByteBuffer::~ByteBuffer ()
{
    if (nullptr != buffer_) {
        delete [] buffer_;
        buffer_ = nullptr;
    }
}

// public
ByteBuffer::ByteBuffer (const char *buffer, 
                        size_t length, 
                        ByteOrder byte_order)
    : buffer_ (nullptr)
    , size_ (strlen (buffer))
    , start_ (0)
    , end_ (0)
    , version_ (0)
    , byte_order_ (byte_order)
{
    assert (nullptr != buffer);
    assert (length == strlen (buffer));
    Initialize (buffer);
}

// public
ByteBuffer::ByteBuffer (const char *buffer, size_t length)
    : buffer_ (nullptr)
    , size_ (length)
    , start_ (0)
    , end_ (0)
    , version_ (0)
    , byte_order_ (ByteOrder::BYTE_ORDER_HOST)
{
    assert (nullptr != buffer);
    assert (length == strlen (buffer));
    Initialize (buffer);
}

// public
ByteBuffer::ByteBuffer (const char *buffer)
    : buffer_ (nullptr)
    , size_ (strlen (buffer))
    , start_ (0)
    , end_ (0)
    , version_ (0)
    , byte_order_ (ByteOrder::BYTE_ORDER_HOST)
{
    assert (nullptr != buffer);
    Initialize (buffer);
}

// public
ByteBuffer::ByteBuffer (ByteOrder byte_order)
    : buffer_ (nullptr)
    , size_ (kDefaultSize)
    , start_ (0)
    , end_ (0)
    , version_ (0)
    , byte_order_ (byte_order)
{
    Initialize (nullptr);
}

// private
void ByteBuffer::Initialize (const char *buffer)
{
    buffer_ = new char[size_];
    memset (buffer_, 0, size_);

    if (nullptr != buffer) {
        end_ = size_;
        memcpy (buffer_, buffer, end_);
    }
    else {
        end_ = 0;
    }
}

// public
bool ByteBuffer::SetReadPosition (const ByteBuffer::ReadPosition& position)
{
    if (position.version_ != version_) {
        return false;
    }

    start_ = position.start_;
    return true;
}

// public
bool ByteBuffer::ReadUInt16 (uint16_t *val)
{
    if (nullptr == val) {
        return false;
    }

    uint16_t v;
    if (ReadBytes (reinterpret_cast<char*>(&v), 2)) {
        *val = (ByteOrder::BYTE_ORDER_NETWORKER == byte_order_) ?
                ByteOrderHelper::NetworkToHost16 (v) :
                v;
        return true;
    }
    
    return false;
}

// public
bool ByteBuffer::ReadUInt32 (uint32_t *val)
{
    if (nullptr == val) {
        return false;
    }

    uint32_t v;
    if (ReadBytes (reinterpret_cast<char*>(&v), 4)) {
        *val = (ByteOrder::BYTE_ORDER_NETWORKER == byte_order_) ?
                ByteOrderHelper::NetworkToHost32 (v) :
                v;
        return true;
    }
    
    return false;
}

// public
bool ByteBuffer::ReadUInt64 (uint64_t *val)
{
    if (nullptr == val) {
        return false;
    }

    uint64_t v;
    if (ReadBytes (reinterpret_cast<char*>(&v), 8)) {
        *val = (ByteOrder::BYTE_ORDER_NETWORKER == byte_order_) ?
                ByteOrderHelper::NetworkToHost64 (v) :
                v;
        return true;
    }

    return false;
}

// public
bool ByteBuffer::ReadBytes (char *val, size_t len)
{
    if (len > Length ()) {
        return false;
    }
    else {
        memcpy (val, buffer_ + start_, len);
        start_ += len;
    }

    return true;
}

// public
bool ByteBuffer::ReadString (std::string *val, size_t len)
{
    if (nullptr == val || len > Length ()) {
        return false;
    }

    val->append (buffer_ + start_, len);
    start_ += len;

    return true;
}

// public
void ByteBuffer::WriteUInt16 (uint16_t val)
{
    uint16_t v = (ByteOrder::BYTE_ORDER_NETWORKER == byte_order_) ?
                  ByteOrderHelper::HostToNetwork16 (val) :
                  val;
    WriteBytes (reinterpret_cast<const char*>(&v), 2);
}

// public
void ByteBuffer::WriteUInt32 (uint32_t val)
{
    uint32_t v = (ByteOrder::BYTE_ORDER_NETWORKER == byte_order_) ?
                  ByteOrderHelper::HostToNetwork32 (val) :
                  val;
    WriteBytes (reinterpret_cast<const char*>(&v), 4);
}

// public
void ByteBuffer::WriteUInt64 (uint64_t val)
{
    uint64_t v = (ByteOrder::BYTE_ORDER_NETWORKER == byte_order_) ?
                  ByteOrderHelper::HostToNetwork64 (val) :
                  val;
    WriteBytes (reinterpret_cast<const char*>(&v), 8);
}

// public
void ByteBuffer::WriteBytes (const char* val, size_t len)
{
    memcpy (ReserveWriteBuffer (len), val, len);
}

// public
void ByteBuffer::Resize (size_t size)
{
    size_t len = std::min (end_ - start_, size);
    if (size < size_) {
        memmove (buffer_, buffer_ + start_, len);
    }
    else {
        size_ = std::max (size, 3 * size_ / 2);
        char *new_buffer = new char[size_];
        memset (new_buffer, 0, size_);
        memcpy (new_buffer, buffer_ + start_, len);
        delete [] buffer_;
        buffer_ = new_buffer;
    }

    start_ = 0;
    end_ = len;
    ++version_;
}

// public
char* ByteBuffer::ReserveWriteBuffer (size_t len)
{
    if (Length () + len > Capacity ()) {
        Resize (Length () + len);
    }

    char *start = buffer_ + end_;
    end_ += len;
    return start;
}

// public
bool ByteBuffer::Consume (size_t size)
{
    if (size > Length ()) {
        return false;
    }

    start_ += size;
    return true;
}

// public
void ByteBuffer::Clear ()
{
    memset (buffer_, 0, size_);
    start_ = end_ = 0;
    ++version_;
}

void ByteBuffer::Swap (ByteBuffer& other)
{
    using std::swap;
    swap (buffer_, other.buffer_);
    swap (size_, other.size_);
    swap (start_, other.start_);
    swap (end_, other.end_);
    swap (version_, other.version_);
    swap (byte_order_, other.byte_order_);
}

} // namespace swift
