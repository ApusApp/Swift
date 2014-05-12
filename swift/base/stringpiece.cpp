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
#include <limits.h>
#include <algorithm>
#include <string.h>
#include <ostream>

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
    inline void BuildLookupTable (const StringPiece& characters_wanted, bool* table) 
    {
        const size_t length = characters_wanted.length ();
        const char* const data = characters_wanted.data ();
        for (size_t i = 0; i < length; ++i) {
            table[static_cast<unsigned char>(data[i])] = true;
        }
    }

}  // namespace

namespace stringpiecedetail {

void CopyToString (const StringPiece& self, std::string* target)
{
    if (self.empty ()) {
        target->clear ();
    }
    else {
        target->assign (self.data (), self.size ());
    }
}

void AppendToString (const StringPiece& self, std::string* target)
{
    if (!self.empty ()) {
        target->append (self.data (), self.size ());
    }
}

size_t Copy (const StringPiece& self, char* buf, size_t n, size_t pos)
{
    size_t ret = std::min (self.size () - pos, n);
    memcpy (buf, self.data () + pos, ret * sizeof(typename StringPiece::value_type));
    return ret;
}

size_t Find (const StringPiece& self, const StringPiece& str, size_t pos)
{
    if (pos > self.size ()) {
        return StringPiece::npos;
    }

    StringPiece::const_iterator result =
        std::search (self.begin () + pos, self.end (), str.begin (), str.end ());
    const size_t xpos = static_cast<size_t>(result - self.begin ());
    return ((xpos + str.size ()) <= self.size ()) ? xpos : StringPiece::npos;
}

size_t Find (const StringPiece& self, char c, size_t pos)
{
    if (pos >= self.size ()) {
        return StringPiece::npos;
    }

    typename StringPiece::const_iterator result =
        std::find (self.begin () + pos, self.end (), c);
    return result != self.end () ?
           static_cast<size_t>(result - self.begin ()) : 
           StringPiece::npos;
}

size_t RFind (const StringPiece& self, const StringPiece& str, size_t pos)
{
    if (self.size () < str.size ()) {
        return StringPiece::npos;
    }

    if (str.empty ()) {
        return StringPiece::npos;
    }

    typename StringPiece::const_iterator last =
        self.begin () + std::min (self.size () - str.size (), pos) + str.size ();
    typename StringPiece::const_iterator result =
        std::find_end (self.begin (), last, str.begin (), str.end ());
    return result != last ?
           static_cast<size_t>(result - self.begin ()) :
           StringPiece::npos;
}

size_t RFind (const StringPiece& self, char c, size_t pos)
{
    if (0 == self.size ()) {
        return StringPiece::npos;
    }

    for (size_t i = std::min (pos, self.size () - 1); ; --i) {
        if (self.data ()[i] == c) {
            return i;
        }

        if (0 == i) {
            break;
        }
    }

    return StringPiece::npos;
}

size_t FindFirstOf (const StringPiece& self, const StringPiece& str, size_t pos)
{
    if (0 == self.size () || 0 == str.size ()) {
        return StringPiece::npos;
    }

    if (1 == str.size ()) {
        return Find (self, str.data ()[0], pos);
    }

    bool lookup[UCHAR_MAX + 1] = { false };
    BuildLookupTable (str, lookup);
    for (size_t i = pos; i < self.size (); ++i) {
        if (lookup[static_cast<unsigned char>(self.data ()[i])]) {
            return i;
        }
    }

    return StringPiece::npos;
}

size_t FindFirstNotOf (const StringPiece& self, const StringPiece& str, size_t pos)
{
    if (self.empty ()) {
        return StringPiece::npos;
    }

    if (str.empty ()) {
        return 0;
    }

    if (1 == str.size ()) {
        return FindFirstNotOf (self, str.data ()[0], pos);
    }

    bool lookup[UCHAR_MAX + 1] = { false };
    BuildLookupTable (str, lookup);
    for (size_t i = pos; i < self.size (); ++i) {
        if (!lookup[static_cast<unsigned char>(self.data ()[i])]) {
            return i;
        }
    }

    return StringPiece::npos;
}

size_t FindFirstNotOf (const StringPiece& self, char c, size_t pos)
{
    if (self.empty ()) {
        return StringPiece::npos;
    }

    for (; pos < self.size (); ++pos) {
        if (self.data ()[pos] != c) {
            return pos;
        }
    }

    return StringPiece::npos;
}

size_t FindLastOf (const StringPiece& self, const StringPiece& str, size_t pos)
{
    if (0 == self.size () || 0 == str.size ()) {
        return StringPiece::npos;
    }

    if (1 == str.size ()) {
        return RFind (self, str.data ()[0], pos);
    }

    bool lookup[UCHAR_MAX + 1] = { false };
    BuildLookupTable (str, lookup);
    for (size_t i = std::min (pos, self.size () - 1);; --i) {
        if (lookup[static_cast<unsigned char>(self.data ()[i])]) {
            return i;
        }

        if (0 == i) {
            break;
        }
    }

    return StringPiece::npos;
}

size_t FindLastOf (const StringPiece& self, char c, size_t pos)
{
    if (self.empty ()) {
        return StringPiece::npos;
    }

    for (size_t i = std::min (pos, self.size () - 1);; --i) {
        if (self.data ()[i] == c) {
            return i;
        }

        if (0 == i) {
            break;
        }
    }

    return StringPiece::npos;
}

size_t FindLastNotOf (const StringPiece& self, const StringPiece& str, size_t pos)
{
    if (self.empty ()) {
        return StringPiece::npos;
    }

    size_t i = std::min (pos, self.size () - 1);
    if (str.empty ()) {
        return i;
    }

    if (1 == str.size ()) {
        return FindLastNotOf (self, str.data ()[0], pos);
    }

    bool lookup[UCHAR_MAX + 1] = { false };
    BuildLookupTable (str, lookup);
    for (; ; --i) {
        if (!lookup[static_cast<unsigned char>(self.data ()[i])]) {
            return i;
        }

        if (0 == i) {
            break;
        }
    }

    return StringPiece::npos;
}

size_t FindLastNotOf (const StringPiece& self, char c, size_t pos)
{
    if (0 == self.size ()) {
        return StringPiece::npos;
    }

    for (size_t i = std::min (pos, self.size () - 1); ; --i) {
        if (self.data ()[i] != c) {
            return i;
        }

        if (0 == i) {
            break;
        }
    }

    return StringPiece::npos;
}

StringPiece SubStr (const StringPiece& self, size_t pos, size_t n)
{
    if (pos > self.size ()) {
        pos = self.size ();
    }

    if (n > (self.size () - pos)) {
        n = self.size () - pos;
    }

    return StringPiece (self.data () + pos, n);
}
} // namespace stringpiecedetail

const StringPiece::size_type StringPiece::npos = std::string::npos;

bool operator== (const StringPiece& lhs, const StringPiece& rhs)
{
    if (lhs.size () != rhs.size ()) {
        return false;
    }

    return 0 == StringPiece::WordMemcmp (lhs.data (), rhs.data (), lhs.size ());
}

std::ostream& operator<< (std::ostream& out, const StringPiece& piece)
{
    out.write (piece.data (), static_cast<std::streamsize>(piece.size ()));
    return out;
}

} // namespace swift
