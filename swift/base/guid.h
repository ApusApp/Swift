/*
 * Copyright (c) 2014 ApusApp
 *
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

#ifndef __SWIFT_BASE_GUID_H__
#define __SWIFT_BASE_GUID_H__

#include <stdint.h>
#include <string>
#include <cassert>

namespace swift {

class Guid
{
public:
    Guid();
    Guid(const std::string& guid);
    ~Guid() {};

    Guid(const Guid& guid);
    Guid& operator=(const Guid& rhs);

    inline bool IsValid() const
    {
        return (0 != bytes_[0]) || (0 != bytes_[1]);
    }

    inline std::string ToString() const
    {
        assert(true == IsValid());
        std::string guid;
        RandomDataToGuidString(bytes_, guid);
        return guid;
    }

    bool operator== (const Guid& rhs) const
    {
        return (bytes_[0] == rhs.bytes_[0]) && (bytes_[1] == rhs.bytes_[1]);
    }

public:
    // Generate a 128 bit random GUID of the form: "%08X-%04X-%04X-%04X-%012llX".
    static bool Generate(std::string& guid);

    // Returns true if the input string conforms to the GUID format.
    static bool IsValid(const std::string& guid);

    static bool RandomDataToGuidString(const uint64_t bytes[2], std::string& guid);

private:
    uint64_t bytes_[2];

private:
    static const char* kHexChars;
};
} // namespace swift

#endif // __SWIFT_BASE_GUID_H__
