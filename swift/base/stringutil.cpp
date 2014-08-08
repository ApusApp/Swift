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

#include "stringutil.h"

#include <string.h>
#include <memory>

namespace swift {

//
// Split a string using a character delimiter. Append the components to 'result'.
//
// Note: For multi-character delimiters, this routine will split on *ANY* of
// the characters in the string, not the entire string as a single delimiter.
//
template<typename ITERATOR>
static inline void SplitStringToIterator(const std::string* str,
                                         const char* delimiter,
                                         ITERATOR& result)
{
    // Optimize the common case where delimiter is a single character.
    if ('\0' != delimiter[0] && '\0' == delimiter[1]) {
        char c = delimiter[0];
        const char* p = str->data();
        const char* end = p + str->size();
        while (p != end) {
            if (*p == c) {
                ++p;
            } else {
                const char* start = p;
                while (++p != end && *p != c);
                *result++ = std::move(std::string(start, p - start));
            }
        }
        return;
    }

    std::string::size_type end = 0;
    std::string::size_type begin = str->find_first_not_of(delimiter);
    while (std::string::npos != begin) {
        end = str->find_first_of(delimiter, begin);
        if (std::string::npos == end) {
            *result++ = std::move(str->substr(begin));
            return;
        }
        *result++ = std::move(str->substr(begin, (end - begin)));
        begin = str->find_first_not_of(delimiter, end);
    }
}

//
// Split a string using a character delimiter. Append the components
// to 'result'.  If there are consecutive delimiters, this function
// will return corresponding empty strings. The string is split into
// at most the specified number of pieces greedily. This means that the
// last piece may possibly be split further. To split into as many pieces
// as possible, specify 0 as the number of pieces.
//
// If 'str' is the empty string, yields an empty string as the only value.
//
// If 'pieces' is negative for some reason, it returns the whole string
//
template<typename STRING_TYPE, typename ITERATOR>
static inline void SplitStringToIteratorAllowEmpty(const STRING_TYPE* str,
                                                   const char* delimiter,
                                                   int pieces,
                                                   ITERATOR& result)
{
    std::string::size_type begin = 0;
    std::string::size_type end = 0;
    for (int i = 0; (i < pieces) || (0 == pieces); ++i) {
        end = str->find_first_of(delimiter, begin);
        if (std::string::npos == end) {
            *result++ = std::move(str->substr(begin));
            return;
        }

        *result++ = std::move(str->substr(begin, (end - begin)));
        begin = end + 1;
    }
    *result++ = std::move(str->substr(begin));
}

// static public
std::string& StringUtil::Strip(std::string& str,
                               const char* remove,
                               char replace_with)
{
    assert(nullptr != remove);
    const char* start = str.c_str();
    const char* current = start;
    for (current = strpbrk(current, remove);
        nullptr != current;
        current = strpbrk(current + 1, remove)) {
            str[current - start] = replace_with;
    }
    return str;
}

// static public
std::string& StringUtil::TrimAll(std::string& str, const char* filter)
{
    assert(nullptr != filter);

    std::string tmp;
    tmp.reserve(str.size());
    const char* start = str.c_str();
    const char* end = start + str.size();
    for (const char* current = strpbrk(start, filter);
         nullptr != current;
         current = strpbrk(start, filter)) {
            if ((current - start) > 0) {
                tmp.append(start, current - start);
            }

            start = current + 1;
    }

    if (start != str.c_str()) {
        if (end != start) {
            tmp.append(start, end - start);
        }

        str = std::move(tmp);
    }

    return str;
}

// static public
void StringUtil::Replace(const std::string& str,
                         const std::string& old_str,
                         const std::string& new_str,
                         bool replace_all,
                         std::string& out_str)
{
    if (old_str.empty()) {
        out_str.append(str);  // if old_str empty, append the given string
        return;
    }

    std::string::size_type start_pos = 0;
    std::string::size_type pos = 0;
    do {
        pos = str.find(old_str, start_pos);
        if (std::string::npos == pos) {
            break;
        }
        out_str.append(str, start_pos, pos - start_pos);
        out_str.append(new_str);
        start_pos = pos + old_str.size();
    } while (replace_all);
    out_str.append(str, start_pos, str.length() - start_pos);
}

// static public
void StringUtil::Split(const std::string* str,
                       const char* delimiter,
                       std::vector<std::string>* result)
{
    if (str && delimiter && result) {
        std::back_insert_iterator<std::vector<std::string>> it(*result);
        SplitStringToIterator(str, delimiter, it);
    }
}

// static public
void StringUtil::SplitAllowEmpty(const std::string* str,
                                 const char* delimiter,
                                 std::vector<std::string>* result)
{
    if (str && delimiter && result) {
        std::back_insert_iterator<std::vector<std::string>> it(*result);
        SplitStringToIteratorAllowEmpty(str, delimiter, 0, it);
    }
}

// static public
void StringUtil::Join(const std::vector<std::string>* components,
                      const char* delimiter,
                      std::string* result)
{
    if (nullptr == result || nullptr == components) {
        return;
    }

    result->clear();
    size_t total_length = 0;
    size_t delimiter_length = (nullptr == delimiter) ? strlen(delimiter) : 0;
    const std::vector<std::string>& vec = *components;
    for (auto const& it : vec) {
        if (!it.empty()) {
            total_length += it.size();
            total_length += delimiter_length;
        }
    }

    total_length -= delimiter_length;
    if (total_length <= 0) {
        return;
    }

    result->reserve(total_length);

    // combine erverything
    bool first = true;
    for (auto const& it : vec) {
        if (!it.empty()) {
            if (first && delimiter_length) {
                first = false;
                result->append(delimiter, delimiter_length);
            }
            result->append(it.data(), it.size());
        }
    }
}

// static public
size_t StringUtil::FastIntegerToHex(char buf[], uintptr_t value)
{
    static const char digits_hex[] = "0123456789abcdef";
    static_assert (sizeof(digits_hex) == 17, "sizeof(digitsHex) must equal to 17");

    uintptr_t i = value;
    char* p = buf;
    do {
        int lsd = i % 16;
        i /= 16;
        *p++ = digits_hex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse (buf, p);

    return p - buf;
}

} // namespace swift
