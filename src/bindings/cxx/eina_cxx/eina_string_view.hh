/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EINA_STRING_VIEW_HH_
#define EINA_STRING_VIEW_HH_

#include <string>
#include <iterator>
#include <stdexcept>
#include <algorithm>

#include <eina_throw.hh>

namespace efl { namespace eina {

template <typename CharT, typename Traits = std::char_traits<CharT> >
class basic_string_view
{
public:
   // Types:
   typedef Traits traits_type;
   typedef CharT value_type;
   typedef CharT& reference;
   typedef CharT const& const_reference;
   typedef CharT* pointer;
   typedef CharT const* const_pointer;
   typedef const_pointer const_iterator;
   typedef const_iterator iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
   typedef std::ptrdiff_t difference_type;
   typedef std::size_t size_type;

   // Constants:
   static constexpr size_type npos = size_type(-1);

   // Constructors:
   constexpr basic_string_view() noexcept
     : _str(nullptr), _len(0)
   {}

   basic_string_view(basic_string_view<CharT, Traits> const& other) noexcept = default;
   basic_string_view(basic_string_view<CharT, Traits>&& other) noexcept = default;

   template<typename Allocator>
   basic_string_view(std::basic_string<CharT, Traits, Allocator> const& str) noexcept
     : _str(str.data()), _len(str.length())
   {}

   basic_string_view(CharT const* c_str)
       : _str(c_str), _len(Traits::length(c_str))
   {}

   basic_string_view(std::nullptr_t)
       : _str(NULL), _len(0)
   {}

//    basic_string_view(CharT const* c_str, size_type len) noexcept
//      : _str(c_str), _len(len)
//    {}

   // Assignment:
   basic_string_view<CharT, Traits>& operator=(basic_string_view<CharT, Traits> const& other) noexcept = default;
   basic_string_view<CharT, Traits>& operator=(basic_string_view<CharT, Traits>&& other) noexcept = default;

   // Iterators
   const_iterator begin() const noexcept { return _str; }
   const_iterator cbegin() const noexcept { return _str; }
   const_iterator end() const noexcept { return _str + _len; }
   const_iterator cend() const noexcept { return _str + _len; }
   const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
   const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
   const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
   const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

   // Capacity:
   size_type size() const noexcept { return _len; }
   size_type length() const noexcept { return _len; }
   bool empty() const noexcept { return _len == 0; }

   // Element access:
   const_reference operator[](size_type pos) const { return _str[pos]; }

   const_reference at(size_type pos) const
   {
      if (pos >= _len)
        EFL_CXX_THROW(std::out_of_range("efl::eina::basic_string_view::at"));
      return _str[pos];
   }

   const_reference front() const { return _str[0]; }
   const_reference back() const { return _str[_len-1]; }

   // Operations:
   CharT const* data() const noexcept { return _str; }

   size_type copy(CharT* s, size_type len, size_type pos = 0) const
   {
      Traits::copy(s, _str+pos, len);
      return len;
   }

   int compare(basic_string_view<CharT, Traits> const& s) const
   {
      const int cmp = Traits::compare(_str, s._str, (std::min)(_len, s._len));
      return cmp != 0 ? cmp : ( _len == s._len ? 0 : _len < s._len ? -1 : 1 );
   }

   int compare(size_type pos, size_type len, basic_string_view<CharT, Traits> const& s) const
   {
      const int cmp = Traits::compare(_str+pos, s._str, (std::min)(len, s._len));
      return cmp != 0 ? cmp : ( len == s._len ? 0 : len < s._len ? -1 : 1 );
   }

   int compare(size_type pos1, size_type len1, basic_string_view<CharT, Traits> const& s, size_type pos2, size_type len2) const
   {
      const int cmp = Traits::compare(_str+pos1, s._str+pos2, (std::min)(len1, len2));
      return cmp != 0 ? cmp : ( len1 == len2 ? 0 : len1 < len2 ? -1 : 1 );
   }

   bool starts_with(CharT c) const
   {
      return !empty() && Traits::eq(c, front());
   }

   bool starts_with(basic_string_view<CharT, Traits> const& s) const
   {
      return _len >= s._len && Traits::compare(_str, s._str, s._len) == 0;
   }

   bool ends_with(CharT c) const
   {
      return !empty() && Traits::eq(c, back());
   }

   bool ends_with(basic_string_view<CharT, Traits> const& s) const
   {
      return _len >= s._len && Traits::compare(_str + _len - s._len, s._str, s._len) == 0;
   }

   size_type find(basic_string_view<CharT, Traits> const& s) const
   {
      const_iterator iter = std::search(cbegin(), cend(), s.cbegin(), s.cend(), Traits::eq);
      return iter == cend () ? npos : std::distance(cbegin(), iter);
   }

   size_type find(basic_string_view<CharT, Traits> const& s, size_type pos) const
   {
      if (pos >= _len)
        return npos;
      const_iterator iter = std::search(cbegin()+pos, cend(), s.cbegin(), s.cend(), Traits::eq);
      return iter == cend () ? npos : std::distance(cbegin(), iter);
   }

   size_type find(CharT c) const
   {
      const_iterator iter = std::find_if(cbegin(), cend(), [=](CharT val) { return Traits::eq(c, val); });
      return iter == cend() ? npos : std::distance(cbegin(), iter);
   }

   size_type find(CharT c, size_type pos) const
   {
      if (pos >= _len)
        return npos;
      const_iterator iter = std::find_if(cbegin()+pos, cend(), [=](CharT val) { return Traits::eq(c, val); });
      return iter == cend() ? npos : std::distance(cbegin(), iter);
   }

   size_type rfind(basic_string_view<CharT, Traits> const& s) const
   {
      const_reverse_iterator iter = std::search(crbegin(), crend(), s.crbegin(), s.crend(), Traits::eq);
      return iter == crend() ? npos : reverse_distance(crbegin(), iter) - s.lenght();
   }

   size_type rfind(basic_string_view<CharT, Traits> const& s, size_type pos) const
   {
      if (pos >= _len)
        return npos;
      const_reverse_iterator iter = std::search(crbegin()+pos, crend(), s.crbegin(), s.crend(), Traits::eq);
      return iter == crend() ? npos : reverse_distance(crbegin(), iter) - s.lenght();
   }

   size_type rfind(CharT c) const
   {
      const_reverse_iterator iter = std::find_if(crbegin(), crend(), [&](CharT val) { return Traits::eq(c, val); });
      return iter == crend() ? npos : reverse_distance(crbegin(), iter);
   }

   size_type rfind(CharT c, size_type pos) const
   {
      pos = (pos >= _len) ? 0 : _len - pos - 1;
      const_reverse_iterator iter = std::find_if(crbegin()+pos, crend(), [&](CharT val) { return Traits::eq(c, val); });
      return iter == crend() ? npos : reverse_distance(crbegin(), iter);
   }

   size_type find_first_of(CharT c) const { return find(c); }
   size_type find_last_of (CharT c) const { return rfind(c); }

   size_type find_first_of(basic_string_view<CharT, Traits> const& s) const
   {
      const_iterator iter = std::find_first_of(cbegin(), cend(), s.cbegin(), s.cend(), Traits::eq);
      return iter == cend() ? npos : std::distance(cbegin(), iter);
   }

   size_type find_last_of(basic_string_view<CharT, Traits> const& s) const
   {
      const_reverse_iterator iter = std::find_first_of(crbegin(), crend(), s.cbegin(), s.cend(), Traits::eq);
      return iter == crend () ? npos : reverse_distance(crbegin(), iter);
   }

   size_type find_first_not_of(basic_string_view<CharT, Traits> const& s) const
   {
      const_iterator iter = find_not_of(cbegin(), cend(), s);
      return iter == cend() ? npos : std::distance(cbegin(), iter);
   }

   size_type find_first_not_of(CharT c) const
   {
      for (const_iterator iter = cbegin(); iter != cend(); ++iter)
        if (!Traits::eq(c, *iter))
          return std::distance(cbegin(), iter);
      return npos;
   }

   size_type find_last_not_of(basic_string_view<CharT, Traits> const& s) const
   {
      const_reverse_iterator iter = find_not_of(crbegin(), crend(), s);
      return iter == crend() ? npos : reverse_distance(crbegin(), iter);
   }

   size_type find_last_not_of(CharT c) const
   {
      for (const_reverse_iterator iter = crbegin(); iter != crend(); ++iter)
        if (!Traits::eq(c, *iter))
          return reverse_distance(crbegin(), iter);
      return npos;
   }

   // XXX: returning std::string instead of eina::string_view
   std::basic_string<CharT, Traits> substr(size_type pos, size_type len=npos) const
   {
      if (pos > size())
        EFL_CXX_THROW(std::out_of_range("efl::eina::string_view::substr"));
      if (len == npos || pos + len > size())
        len = size () - pos;
      return std::basic_string<CharT, Traits>(data() + pos, len);
   }

   // Conversions:
   std::basic_string<CharT, Traits> to_string() const
   {
      return std::basic_string<CharT, Traits>(_str, _len);
   }

   std::basic_string<CharT, Traits> str() const
   {
      return to_string();
   }

   CharT const* c_str() const noexcept { return data(); }

   template<typename Allocator>
   operator std::basic_string<CharT, Traits, Allocator>() const
   {
      return std::basic_string<CharT, Traits, Allocator>(_str, _len);
   }

   void swap(basic_string_view<CharT, Traits>& s)
   {
      std::swap(_str, s._str);
      std::swap(_len, s._len);
   }

   //  Comparison operators:
   friend inline bool operator==(basic_string_view<CharT, Traits> const& s1, basic_string_view<CharT, Traits> const& s2)
   {
      if (s1.size() != s2.size())
        return false;
      return s1.compare(s2) == 0;
   }

   friend inline bool operator!=(basic_string_view<CharT, Traits> const& s1, basic_string_view<CharT, Traits> const& s2)
   {
      return !(s1 == s2);
   }

   friend inline bool operator<(basic_string_view<CharT, Traits> const& s1, basic_string_view<CharT, Traits> const& s2)
   {
      return s1.compare(s2) < 0;
   }

   friend inline bool operator>(basic_string_view<CharT, Traits> const& s1, basic_string_view<CharT, Traits> const& s2)
   {
      return s2 < s1;
   }

   friend inline bool operator<=(basic_string_view<CharT, Traits> const& s1, basic_string_view<CharT, Traits> const& s2)
   {
       return !(s2 < s1);
   }

   friend inline bool operator>=(basic_string_view<CharT, Traits> const& s1, basic_string_view<CharT, Traits> const& s2)
   {
       return !(s1 < s2);
   }

private:
   template <typename Iterator>
   size_type reverse_distance(Iterator first, Iterator last) const
   {
     return _len - 1 - std::distance(first, last);
   }

   template <typename Iterator>
   Iterator find_not_of(Iterator first, Iterator last, basic_string_view<CharT, Traits>& s) const
   {
      for (; first != last; ++first)
        if (0 == Traits::find(s._str, s._len, *first))
          return first;
      return last;
   }

   const CharT* _str;
   size_type _len;
};

// Stream operation
template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, basic_string_view<CharT,Traits> const& s)
{
   return os << s.data();
}

template <typename CharT, typename Traits>
void swap(basic_string_view<CharT, Traits>& s1, basic_string_view<CharT, Traits>& s2)
{
   s1.swap(s2);
}

// Instantiations:
typedef basic_string_view<char> string_view;
typedef basic_string_view<wchar_t> wstring_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;

} }

#endif
