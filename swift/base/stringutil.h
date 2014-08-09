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

#ifndef __SWIFT_BASE_STRING_UTIL_H__
#define __SWIFT_BASE_STRING_UTIL_H__

#include <string>
#include <cassert>
#include <vector>
#include <algorithm>
#include <sstream>
#include <set>
#include <unordered_set>
#include <unordered_map>

namespace swift {
class StringUtil
{
public:
    static inline char* AsArray(std::string* str)
    {
        // Do not use const_cast<char*>(str->data())
        assert(nullptr != str);
        return str->empty() ? nullptr : &*(str->begin());
    }

    static inline bool StartWithPrefix(const char* str,
                                       const char* prefix)
    {
        const char* s = str;
        const char* p = prefix;
        while (*p) {
            if (*p != *s) {
                return false;
            }
            ++p;
            ++s;
        }

        return true;
    }

    static inline bool StartWithPrefix(const std::string& str,
                                       const std::string& prefix)
    {
        return (str.size() >= prefix.size()) &&
               (0 == str.compare(0, prefix.size(), prefix));
    }

    static inline bool EndWithSuffix(const std::string& str,
                                     const std::string& suffix)
    {
        return (str.size() >= suffix.size()) &&
               (0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix));
    }

    static inline std::string StripPrefix(const std::string& str,
                                          const std::string& prefix)
    {
        return StartWithPrefix(str, prefix) ?
               std::move(str.substr(prefix.size())) :
               str;
    }

    static inline std::string StripSuffix(const std::string& str,
                                          const std::string& suffix)
    {
        return EndWithSuffix(str, suffix) ?
               std::move(str.substr(0, str.size() - suffix.size())) :
               str;
    }

    static inline uint32_t Count(const std::string* str, char ch)
    {
        assert(nullptr != str);
        uint32_t count = 0;
        std::string::const_iterator end = str->end();
        for (std::string::const_iterator it = str->begin(); it != end; ++it) {
            if (ch == *it) {
                ++count;
            }
        }

        return count;
    }

    static inline void ToLower(std::string* str)
    {
        assert(nullptr != str);
        std::string::iterator end = str->end();
        for (std::string::iterator it = str->begin(); it != end; ++it) {
            if (('A' <= *it) && (*it <= 'Z')) {
                *it += ('a' - 'A');
            }
        }
    }

    static inline void ToUpper(std::string* str)
    {
        assert(nullptr != str);
        std::string::iterator end = str->end();
        for (std::string::iterator it = str->begin(); it != end; ++it) {
            if (('a' <= *it) && (*it <= 'z')) {
                *it += ('A' - 'a');
            }
        }
    }

    //
    // Replaces any occurrence of the character 'remove' with the character 'replace_with'
    // returns reference of the 'str'
    //
    static std::string& Strip(std::string& str,
                              const char* remove,
                              char replace_with);

    //
    // Removes characters in " \t\r\n" from head or tail of the 'str'
    // returns reference of the 'str'
    //
    static inline std::string& Trim(std::string& str,
                                    bool left = true,
                                    bool right = true)
    {
        static const std::string delimiter = " \t\r\n";
        if (left) {
            str.erase(0, str.find_first_not_of(delimiter));
        }
        if (right) {
            str.erase(str.find_last_not_of(delimiter) + 1);
        }

        return str;
    }

    //
    // Removes all space characters
    // returns reference of the 'str'
    //
    static inline std::string& TrimSpaces(std::string& str)
    {
        std::string::iterator it = std::remove_if(str.begin(),
                                                  str.end(),
                                                  ::isspace);
        str.resize(it - str.begin());
        return str;
    }

    //
    // Removes all 'ch' from 'str'
    // returns reference of the 'str'
    //
    static inline std::string& TrimAll(std::string& str, char ch)
    {
        std::string::iterator it = std::remove_if(
            str.begin(), str.end(), std::move([&ch](const char& c)->bool {
                return (ch == c);
            }));

        str.resize(it - str.begin());

        return str;
    }

    //
    // Removes all character in 'filter' from 'str'
    // returns reference of the 'str'
    //
    static std::string& TrimAll(std::string& str, const char* filter);

    //
    // Replace the 'old_str' pattern with the 'new_str' pattern in a string,
    // and append the result to 'out_str'.  If replace_all is false,
    // it only replaces the first instance of 'old_str'.
    static void Replace(const std::string& str,
                        const std::string& old_str,
                        const std::string& new_str,
                        bool replace_all,
                        std::string& out_str);

    //
    // Replace the first instance of 'old_str' in the 'str' with 'new_str', if it is exists,
    // if 'replace_all' is true, replace all 'old_str' in the 'str'.
    // return a new string, regardless of whether the replacement happened or not.
    //
    static inline std::string Replace(const std::string& str,
                                      const std::string& old_str,
                                      const std::string& new_str,
                                      bool replace_all)
    {
        std::string ret;
        Replace(str, old_str, new_str, replace_all, ret);
        return std::move(ret);
    }

    //
    // Split a string using one or more character delimiters, presented
    // as a nul-terminated c string. Append the components to 'result'.
    // If there are consecutive delimiters, this function will return
    // corresponding empty strings.
    //
    // If 'str' is the empty string, yields an empty string as the only value.
    //
    static void Split(const std::string* str,
                      const char* delimiter,
                      std::vector<std::string>* result);

    static inline void Split(const std::string* str,
                             const char delimiter,
                             std::vector<std::string>* result)
    {
        Split(str, &delimiter, result);
    }

    void SplitToSet(const std::string* str,
                    const char* delimiter,
                    std::set<std::string>* result);
    void SplitToHashSet(const std::string* str,
                        const char* delimiter,
                        std::unordered_set<std::string>* result);
    void SplitToHashMap(const std::string* str,
                        const char* delimiter,
                        std::unordered_map<std::string, std::string>* result);


    //
    // Split a string using one or more byte delimiters, presented
    // as a nul-terminated c string. Append the components to 'result'.
    // If there are consecutive delimiters, this function will return corresponding
    // empty strings. If you want to drop the empty strings, try Split().
    //
    // If 'str' is the empty string, yields an empty string as the only value.
    //
    static void SplitAllowEmpty(const std::string* str,
                                const char* delimiter,
                                std::vector<std::string>* result);
    static void SplitToSetAllowEmpty(const std::string* str,
                                     const char* delimiter,
                                     std::set<std::string>* result);
    static void SplitToHashSetAllowEmpty(const std::string* str,
                                         const char* delimiter,
                                         std::unordered_set<std::string>* result);
    //
    // The even-positioned (0-based) components become the keys for the
    // odd-positioned components that follow them. When there is an odd
    // number of components, the value for the last key will be unchanged
    // if the key was already present in the hash table, or will be the
    // empty string if the key is a newly inserted key.
    //
    static void SplitToHashMapAllowEmpty(const std::string* str,
                                         const char* delimiter,
                                         std::unordered_map<std::string, std::string>* result);

    //
    // Concatenate a vector of strings into a C++ string, using the C-string
    // 'delimiter' as s separator bwtween components. if 'delimiter' is NULL,
    // concatenate the vector of strings into a string directly.
    // If string is empty in vector, skip it and no append 'delimiter'.
    // Note. 'result' will be clear at first
    //
    static void Join(const std::vector<std::string>* components,
                     const char* delimiter,
                     std::string* result);

    static inline std::string Join(const std::vector<std::string>* components,
                                   const char* delimiter)
    {
        if (components && !components->empty()) {
            std::string result;
            Join(components, delimiter, &result);
            return std::move(result);
        }

        return std::string();
    }

    //
    // Convert to string
    //
    template<typename T>
    static inline bool ToString(const T& val, std::string* str)
    {
        assert(nullptr != str);
        std::ostringstream oss;
        oss << std::boolalpha << val;
        *str = std::move(oss.str());
        return !oss.fail();
    }

    template<typename T>
    static inline bool FromString(const std::string& str, T* val)
    {
        assert(nullptr != val);
        std::istringstream iss(str);
        iss >> std::boolalpha >> *val;
        return !iss.fail();
    }

    template<typename T>
    static inline std::string ToString(const T& val)
    {
        std::string ret;
        ToString(val, &ret);
        return std::move(ret);
    }

    template<typename T>
    static inline T FromString(const std::string& str)
    {
        T val;
        FromString(str, &val);
        return val;
    }

    template<typename T>
    static inline T FromString(const std::string& str,
                               const T& default_value)
    {
        T val(default_value);
        FromString(str, &val);
        return val;
    }

    //
    // Efficient Integer to String Conversions, by Matthew Wilson.
    // Convert integer to string
    // Returns the valid length of content
    //
    template<typename T>
    static size_t FastIntegerToBuffer(char buf[], const T& value);

    //
    // uintptr_t:
    // Integer type capable of holding a value converted from a void pointer and then be
    // converted back to that type with a value that compares equal to the original pointer.
    //
    // Returns the valid length of content
    //
    static size_t FastIntegerToHex(char buf[], uintptr_t value);
};

template<typename T>
size_t StringUtil::FastIntegerToBuffer(char buf[], const T& value)
{
    static const char digits[] = "9876543210123456789";
    static_assert(sizeof(digits) == 20, "sizeof(digits) must equal to 20");
    static const char* zero = digits + 9;

    T i = value;
    char* p = buf;
    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

} // namespace swift

#endif // __SWIFT_BASE_STRING_UTIL_H__

