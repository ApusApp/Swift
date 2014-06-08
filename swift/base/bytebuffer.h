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

#ifndef __SWIFT_BASE_BYTE_BUFFER_H__
#define __SWIFT_BASE_BYTE_BUFFER_H__

#include <string>

#include "swift/base/noncopyable.hpp"
#include "swift/base/byteorderhelper.h"

namespace swift {
    
class ByteBuffer : swift::noncopyable
{
    typedef ByteOrderHelper::ByteOrder ByteOrder;
public:
    class ReadPosition
    {
        friend class ByteBuffer;
        ReadPosition (size_t start, int version) 
            : start_ (start)
            , version_ (version)
        {
        }

        size_t start_;
        int version_;
    };
public:
    // Note: the default byte's order is ByteOrder::BYTE_ORDER_HOST
    ByteBuffer ();
    explicit ByteBuffer (const char *buffer);
    explicit ByteBuffer (ByteOrder byte_order);
    ByteBuffer (const char *buffer, size_t length);
    ByteBuffer (const char *buffer, 
                size_t length, 
                ByteOrder byte_order);
    
    ~ByteBuffer ();

    //movable
    ByteBuffer (ByteBuffer&&) = delete;
    ByteBuffer& operator= (ByteBuffer&&) = delete;

    size_t Length () const
    {
        return end_ - start_;
    }

    size_t Capacity () const
    {
        return size_ - start_;
    }

    const char* Data () const
    {
        return buffer_ + start_;
    }

    ByteOrder Order () const
    {
        return byte_order_;
    }

    ReadPosition GetReadPosition () const
    {
        return ReadPosition (start_, version_);
    }

    // if the version in new position isn't equal to version_, return false. 
    bool SetReadPosition (const ReadPosition& position);

    std::string ToString () const
    {
        return std::string (buffer_ + start_, end_ - start_);
    }

    // Read a next value from the buffer. Return false if 
    // there isn't enough data left for the specified type.
    bool ReadUInt8 (uint8_t *val)
    {
        if (nullptr == val) { return false;}
        return ReadBytes (reinterpret_cast<char*>(val), 1);
    }

    bool ReadUInt16 (uint16_t *val);
    bool ReadUInt32 (uint32_t *val);
    bool ReadUInt64 (uint64_t *val);
    bool ReadBytes (char *val, size_t len);

    // Appends next len bytes to val, if there is less than len return false.
    bool ReadString (std::string *val, size_t len);

    // Write value to the buffer. auto resizes the buffer when it is necessary.
    void WriteUInt8 (uint8_t val)
    {
        WriteBytes (reinterpret_cast<const char*>(&val), 1);
    }

    void WriteUInt16 (uint16_t val);
    void WriteUInt32 (uint32_t val);
    void WriteUInt64 (uint64_t val);

    void WriteString (const std::string& val)
    {
        WriteBytes (val.c_str (), val.size ());
    }

    void WriteBytes (const char* val, size_t len);

    // Resize the buffer to the specified size.
    // This invalidates any remembered seek positions.
    void Resize (size_t size);

    // Reserves the given number of bytes and return a char* that can be
    // written into. Useful for functions that require a char* buffer and
    // not a ByteBuffer.
    char* ReserveWriteBuffer (size_t len);

    // Moves current position size bytes forward. Return false if there is
    // less than size bytes left in the buffer. Consume doesn't permanently
    // remove data. so remembered read positions are still valid after this call.
    bool Consume (size_t size);

    // Clears the contents of the buffer. After this, Length() will be 0.
    void Clear ();

    void Swap (ByteBuffer& other);

private:
    void Initialize (const char *buffer);

private:
    char* buffer_;
    size_t size_;
    size_t start_;
    size_t end_;
    int version_;
    ByteOrder byte_order_;

private:
    static const int kDefaultSize = 4096;
};

} // namespace swift

#endif // __SWIFT_BASE_BYTE_BUFFER_H__
