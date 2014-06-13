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

#ifndef __SWIFT_BASE_STRING_PIECE_H__
#define __SWIFT_BASE_STRING_PIECE_H__

#include <stddef.h>
#include <assert.h>
#include <functional>

#include <iosfwd>
#include <string>

namespace swift {

class StringPiece;

namespace stringpiecedetail {

void CopyToString (const StringPiece& self, std::string* target);
void AppendToString (const StringPiece& self, std::string* target);
size_t Copy (const StringPiece& self, char* buf, size_t n, size_t pos);
size_t Find (const StringPiece& self, const StringPiece& str, size_t pos);
size_t Find (const StringPiece& self, char c, size_t pos);
size_t RFind (const StringPiece& self, const StringPiece& str, size_t pos);
size_t RFind (const StringPiece& self, char c, size_t pos);
size_t FindFirstOf (const StringPiece& self, const StringPiece& str, size_t pos);
size_t FindFirstNotOf (const StringPiece& self, const StringPiece& str, size_t pos);
size_t FindFirstNotOf (const StringPiece& self, char c, size_t pos);
size_t FindLastOf (const StringPiece& self, const StringPiece& str, size_t pos);
size_t FindLastOf (const StringPiece& self, char c, size_t pos);
size_t FindLastNotOf (const StringPiece& self, const StringPiece& str, size_t pos);
size_t FindLastNotOf (const StringPiece& self, char c, size_t pos);
StringPiece SubStr (const StringPiece& self, size_t pos, size_t n);
} // namespace stringpiecedetail 

class StringPiece
{
public:
    // Standard STL container boilerplate.
    typedef typename std::string::size_type size_type;
    typedef typename std::string::value_type value_type;
    typedef const value_type& const_reference;
    typedef const value_type& reference;
    typedef const value_type* pointer;
    typedef const value_type* const_iterator;
    typedef ptrdiff_t difference_type;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos;
    
public:
    StringPiece () : ptr_ (nullptr), length_ (0)
    {

    }

    StringPiece (const value_type* str)
        : ptr_ (str)
        , length_ ((nullptr == str) ? 0 : std::string::traits_type::length (str))
    {

    }

    StringPiece (const std::string& str) : ptr_ (str.data ()), length_ (str.size ())
    {

    }

    StringPiece (const value_type* str, size_type len) : ptr_ (str), length_ (len)
    {

    }

    StringPiece (const typename std::string::const_iterator& begin,
                 const typename std::string::const_iterator& end)
        : ptr_ ((end > begin) ? &(*begin) : nullptr)
        , length_ ((end > begin) ? static_cast<size_type>(end - begin) : 0)
    {

    }

    ~StringPiece ()
    {
        clear ();
    }

    const value_type* data () const
    {
        return ptr_;
    }

    const value_type* c_str () const
    {
        return ptr_;
    }

    size_type length () const
    {
        return length_;
    }

    size_type size () const
    {
        return length_;
    }

    bool empty () const
    {
        return 0 == length_;
    }

    void clear ()
    {
        ptr_ = nullptr;
        length_ = 0;
    }

    void Set (const value_type* str)
    {
        ptr_ = str;
        length_ = (nullptr == str) ? 0 : std::string::traits_type::length (str);
    }

    void Set (const value_type* str, size_type len)
    {
        ptr_ = str;
        length_ = len;
    }

    value_type operator[] (size_type i) const
    {
        assert (i >= 0 && i < length_);
        return ptr_[i];
    }

    void RemoveSuffix (size_type n)
    {
        assert (n >= 0 && n <= length_);
        length_ -= n;
    }

    void RemovePrefix (size_type n)
    {
        assert (n >= 0 && n <= length_);
        ptr_ += n;
        length_ -= n;
    }

    int compare (const StringPiece& rhs) const
    {
        int n = WordMemcmp (ptr_, rhs.ptr_, (length_ < rhs.length_) ? length_ : rhs.length_);
        if (0 == n) {
            if (length_ < rhs.length_) {
                n = -1;
            }
            else if (length_ > rhs.length_) {
                n = +1;
            }
        }

        return n;
    }

    std::string ToString () const
    {
        return empty () ? std::string () : std::string (data (), size ());
    }

    const_iterator begin () const
    {
        return ptr_;
    }

    const_iterator end () const
    {
        return ptr_ + length_;
    }

    const_reverse_iterator rbegin () const
    {
        return const_reverse_iterator (ptr_ + length_);
    }

    const_reverse_iterator rend () const
    {
        return const_reverse_iterator (ptr_);
    }

    size_type max_size () const
    {
        return length_;
    }

    size_type capacity () const
    {
        return length_;
    }

    size_type copy (value_type* buf, 
                    size_type n, 
                    size_type pos = 0) const
    {
        return stringpiecedetail::Copy (*this, buf, n, pos);
    }

    size_type find (const StringPiece& str, size_type pos = 0) const
    {
        return stringpiecedetail::Find (*this, str, pos);
    }

    size_type find (value_type c, size_type pos = 0) const
    {
        return stringpiecedetail::Find (*this, c, pos);
    }

    size_type rfind (const StringPiece& str, size_type pos = StringPiece::npos) const
    {
        return stringpiecedetail::RFind (*this, str, pos);
    }

    size_type rfind (value_type c,size_type pos = StringPiece::npos)
    {
        return stringpiecedetail::RFind (*this, c, pos);
    }

    size_type find_first_of (const StringPiece& str, size_type pos = 0)
    {
        return stringpiecedetail::FindFirstOf (*this, str, pos);
    }

    size_type find_first_of (value_type c, size_type pos = 0)
    {
        return this->find (c, pos);
    }

    size_type find_first_not_of (const StringPiece& str, size_type pos = 0)
    {
        return stringpiecedetail::FindFirstNotOf (*this, str, pos);
    }

    size_type find_first_not_of (value_type c, size_type pos = 0)
    {
        return stringpiecedetail::FindFirstNotOf (*this, c, pos);
    }

    size_type find_last_of (const StringPiece& str, size_type pos = StringPiece::npos)
    {
        return stringpiecedetail::FindLastOf (*this, str, pos);
    }

    size_type find_last_of (value_type c, size_type pos = StringPiece::npos)
    {
        return stringpiecedetail::FindLastOf (*this, c, pos);
    }

    size_type find_last_not_of (const StringPiece& str, size_type pos = StringPiece::npos)
    {
        return stringpiecedetail::FindLastNotOf (*this, str, pos);
    }

    size_type find_last_not_of (value_type c, size_type pos = StringPiece::npos)
    {
        return stringpiecedetail::FindLastNotOf (*this, c, pos);
    }

    StringPiece substr (size_type pos, size_type n = StringPiece::npos)
    {
        assert (n >= 0 && n <= size ());
        return stringpiecedetail::SubStr (*this, pos, n);
    }

    bool StartWith (const StringPiece& bsp) const
    {
        return (length_ >= bsp.length_) &&
               (0 == WordMemcmp (ptr_, bsp.ptr_, bsp.length_));
    }

    bool EndWith (const StringPiece& bsp) const
    {
        return (length_ >= bsp.length_) &&
               (0 == WordMemcmp (ptr_ + (length_ - bsp.length_), bsp.ptr_, bsp.length_));
    }

    void CopyToString (std::string* target) const
    {
        stringpiecedetail::CopyToString (*this, target);
    }

    void AppendToString (std::string* target) const
    {
        stringpiecedetail::AppendToString (*this, target);
    }

    uint32_t Hash () const {
        // Taken from fbi/nstring.h:
        //    Quick and dirty bernstein hash...fine for short ascii strings
        uint32_t hash = 5381;
        for (size_t ix = 0; ix < size (); ix++) {
            hash = ((hash << 5) + hash) + ptr_[ix];
        }
        return hash;
    }

public:
    static inline int WordMemcmp (const value_type* p1,
                                  const value_type* p2,
                                  size_type n)
    {
        return std::string::traits_type::compare (p1, p2, n);
    }

private:
    const value_type* ptr_;
    size_type length_;
}; // StringPiece

bool operator== (const StringPiece& lhs, const StringPiece& rhs);

inline bool operator!= (const StringPiece& lhs, const StringPiece& rhs)
{
    return !(lhs == rhs);
}

inline bool operator< (const StringPiece& lhs, const StringPiece& rhs)
{
    const int r = StringPiece::WordMemcmp (lhs.data (), 
                                           rhs.data (), 
                                           (lhs.size () < rhs.size () ? lhs.size () : rhs.size ()));
    return ((r < 0) || ((r == 0) && (lhs.size () < rhs.size ())));
}

inline bool operator> (const StringPiece& lhs, const StringPiece& rhs) 
{
    return rhs < lhs;
}

inline bool operator<= (const StringPiece& lhs, const StringPiece& rhs) 
{
    return !(lhs > rhs);
}

inline bool operator>= (const StringPiece& lhs, const StringPiece& rhs) 
{
    return !(lhs < rhs);
}

std::ostream& operator<< (std::ostream& out, const StringPiece& piece);

} // namespace swift

namespace std {

template <>
struct hash <swift::StringPiece> {
    size_t operator() (const swift::StringPiece& str) const {
        return static_cast<size_t>(str.Hash ());
    }
};
} // namespace std


#endif // __SWIFT_BASE_STRING_PIECE_H__
