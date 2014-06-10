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

#ifndef __SWIFT_BASE_CRC32_H__
#define __SWIFT_BASE_CRC32_H__

#include <stddef.h>
#include <stdint.h>

namespace swift {

class Crc32
{
public:
    // Return the Crc32 of concat(A, data[0,len-1]) where initial is the
    // Crc32 of some string A. UpdateCrc32 () is often used to maintain the
    // Crc32 of a stream of data. for the first call, initial should be 0.
    uint32_t UpdateCrc32 (uint32_t initial, const void *data, size_t len);

    // Return the crc32c of data[0,len-1]
    inline uint32_t ComputeCrc32 (const void *data, size_t len) 
    {
        return UpdateCrc32 (0, data, len);
    }
};

} // namespace swift
#endif // __SWIFT_BASE_CRC32_H__
