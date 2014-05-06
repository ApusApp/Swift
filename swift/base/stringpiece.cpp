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

#include "swift/base/stringpiece.h"

namespace swift {
namespace {
    // For each character in characters_wanted, sets the index corresponding
    // to the ASCII code of that character to 1 in table. This is used by
    // the find_.*_of methods below to tell whether or not a character is in
    // the lookup table in constant time.
    // The argument `table' must be an array that is large enough to hold all
    // the possible values of an unsigned char. Thus it should be be declared
    // as follows:
    // bool table[UCHAR_MAX + 1]
    inline void BuildLookupTable (const StringPiece& characters_wanted,
                                  bool* table) {
        const size_t length = characters_wanted.length ();
        const char* const data = characters_wanted.data ();
        for (size_t i = 0; i < length; ++i) {
            table[static_cast<unsigned char>(data[i])] = true;
        }
    }

}  // namespace

namespace detail {

} // namespace detail

} // namespace swift
