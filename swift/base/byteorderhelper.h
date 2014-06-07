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

#ifndef __SWIFT_BASE_BYTE_ORDER_HELPER_H__
#define __SWIFT_BASE_BYTE_ORDER_HELPER_H__

#include <stdint.h>

namespace swift {

// int val = 0x12345678;
// big endian:                  little endian:
// hight address |0x78|         hight address |0x12|
//               |0x56|                       |0x34|
//               |0x34|                       |0x56|
// low address   |0x12|         low address   |0x78|
//
class ByteOrderHelper
{
public:
    enum class ByteOrder
    {
        BYTE_ORDER_HOST = 0,        // the native order of the host.
        BYTE_ORDER_NETWORKER = 1,   // network byte order (big-endian)
    };


public:
    inline static void Set8 (void* addr, size_t offset, uint8_t v) 
    {
        static_cast<uint8_t*>(addr)[offset] = v;
    }

    inline static uint8_t Get8 (const void* addr, size_t offset) 
    {
        return static_cast<const uint8_t*>(addr)[offset];
    }

    inline static void SetBigEndian16 (void* addr, uint16_t v) 
    {
        Set8 (addr, 0, static_cast<uint8_t>(v >> 8));
        Set8 (addr, 1, static_cast<uint8_t>(v >> 0));
    }

    inline static void SetBigEndian32 (void* addr, uint32_t v) 
    {
        Set8 (addr, 0, static_cast<uint8_t>(v >> 24));
        Set8 (addr, 1, static_cast<uint8_t>(v >> 16));
        Set8 (addr, 2, static_cast<uint8_t>(v >> 8));
        Set8 (addr, 3, static_cast<uint8_t>(v >> 0));
    }

    inline static void SetBigEndian64 (void* addr, uint64_t v) 
    {
        Set8 (addr, 0, static_cast<uint8_t>(v >> 56));
        Set8 (addr, 1, static_cast<uint8_t>(v >> 48));
        Set8 (addr, 2, static_cast<uint8_t>(v >> 40));
        Set8 (addr, 3, static_cast<uint8_t>(v >> 32));
        Set8 (addr, 4, static_cast<uint8_t>(v >> 24));
        Set8 (addr, 5, static_cast<uint8_t>(v >> 16));
        Set8 (addr, 6, static_cast<uint8_t>(v >> 8));
        Set8 (addr, 7, static_cast<uint8_t>(v >> 0));
    }

    inline static uint16_t GetBigEndian16 (const void* addr) 
    {
        return static_cast<uint16_t>((Get8 (addr, 0) << 8) |
                                     (Get8 (addr, 1) << 0));
    }

    inline static uint32_t GetBigEndian32 (const void* addr) 
    {
        return (static_cast<uint32_t>(Get8 (addr, 0)) << 24) |
               (static_cast<uint32_t>(Get8 (addr, 1)) << 16) |
               (static_cast<uint32_t>(Get8 (addr, 2)) << 8) |
               (static_cast<uint32_t>(Get8 (addr, 3)) << 0);
    }

    inline static uint64_t GetBigEndian64 (const void* addr) 
    {
        return (static_cast<uint64_t>(Get8 (addr, 0)) << 56) |
               (static_cast<uint64_t>(Get8 (addr, 1)) << 48) |
               (static_cast<uint64_t>(Get8 (addr, 2)) << 40) |
               (static_cast<uint64_t>(Get8 (addr, 3)) << 32) |
               (static_cast<uint64_t>(Get8 (addr, 4)) << 24) |
               (static_cast<uint64_t>(Get8 (addr, 5)) << 16) |
               (static_cast<uint64_t>(Get8 (addr, 6)) << 8) |
               (static_cast<uint64_t>(Get8 (addr, 7)) << 0);
    }

    inline static void SetLittleEndian16 (void* addr, uint16_t v) 
    {
        Set8 (addr, 0, static_cast<uint8_t>(v >> 0));
        Set8 (addr, 1, static_cast<uint8_t>(v >> 8));
    }

    inline static void SetLittleEndian32 (void* addr, uint32_t v) 
    {
        Set8 (addr, 0, static_cast<uint8_t>(v >> 0));
        Set8 (addr, 1, static_cast<uint8_t>(v >> 8));
        Set8 (addr, 2, static_cast<uint8_t>(v >> 16));
        Set8 (addr, 3, static_cast<uint8_t>(v >> 24));
    }

    inline static void SetLittleEndian64 (void* addr, uint64_t v) 
    {
        Set8 (addr, 0, static_cast<uint8_t>(v >> 0));
        Set8 (addr, 1, static_cast<uint8_t>(v >> 8));
        Set8 (addr, 2, static_cast<uint8_t>(v >> 16));
        Set8 (addr, 3, static_cast<uint8_t>(v >> 24));
        Set8 (addr, 4, static_cast<uint8_t>(v >> 32));
        Set8 (addr, 5, static_cast<uint8_t>(v >> 40));
        Set8 (addr, 6, static_cast<uint8_t>(v >> 48));
        Set8 (addr, 7, static_cast<uint8_t>(v >> 56));
    }

    inline static uint16_t GetLittleEndian16 (const void* addr) 
    {
        return static_cast<uint16_t>((Get8 (addr, 0) << 0) |
                                     (Get8 (addr, 1) << 8));
    }

    inline static uint32_t GetLittleEndian32 (const void* addr) 
    {
        return (static_cast<uint32_t>(Get8 (addr, 0)) << 0) |
               (static_cast<uint32_t>(Get8 (addr, 1)) << 8) |
               (static_cast<uint32_t>(Get8 (addr, 2)) << 16) |
               (static_cast<uint32_t>(Get8 (addr, 3)) << 24);
    }

    inline static uint64_t GetLittleEndian64 (const void* addr) 
    {
        return (static_cast<uint64_t>(Get8 (addr, 0)) << 0) |
               (static_cast<uint64_t>(Get8 (addr, 1)) << 8) |
               (static_cast<uint64_t>(Get8 (addr, 2)) << 16) |
               (static_cast<uint64_t>(Get8 (addr, 3)) << 24) |
               (static_cast<uint64_t>(Get8 (addr, 4)) << 32) |
               (static_cast<uint64_t>(Get8 (addr, 5)) << 40) |
               (static_cast<uint64_t>(Get8 (addr, 6)) << 48) |
               (static_cast<uint64_t>(Get8 (addr, 7)) << 56);
    }

    // Check whether the current host is big endian.
    inline static bool IsBigEndianHost ()
    {
        static const int number = 1;
        return 0 == *reinterpret_cast<const char*>(&number);
    }

    inline static uint16_t HostToNetwork16 (uint16_t n) 
    {
        uint16_t result;
        SetBigEndian16 (&result, n);
        return result;
    }

    inline static uint32_t HostToNetwork32 (uint32_t n) 
    {
        uint32_t result;
        SetBigEndian32 (&result, n);
        return result;
    }

    inline static uint64_t HostToNetwork64 (uint64_t n) 
    {
        uint64_t result;
        SetBigEndian64 (&result, n);
        return result;
    }

    inline static uint16_t NetworkToHost16 (uint16_t n) 
    {
        return GetBigEndian16 (&n);
    }

    inline static uint32_t NetworkToHost32 (uint32_t n) 
    {
        return GetBigEndian32 (&n);
    }

    inline static uint64_t NetworkToHost64 (uint64_t n) 
    {
        return GetBigEndian64 (&n);
    }
};

} // namespace swift

#endif // __SWIFT_BASE_BYTE_ORDER_HELPER_H__
