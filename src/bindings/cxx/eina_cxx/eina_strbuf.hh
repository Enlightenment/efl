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
#ifndef EINA_CXX_STRBUF_HH
#define EINA_CXX_STRBUF_HH

#include <Eina.h>
#include <eina_type_traits.hh>
#include <eina_throw.hh>
#include <eina_error.hh>

#include <cstring>

// FIXME: Needs doc, I guess :)

namespace efl { namespace eina {

template <typename strbuf_type, typename basic_type>
struct _strbuf_view_trait
{
   typedef basic_type value_type;
   typedef value_type& reference;
   typedef value_type* pointer;
   typedef value_type const& const_reference;
   typedef value_type const* const_pointer;
   typedef const_pointer const_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
   typedef std::ptrdiff_t difference_type;
   typedef std::size_t size_type;

   _strbuf_view_trait() = delete;

   /** Create a new view from another _strbuf_view_trait */
   _strbuf_view_trait(_strbuf_view_trait const& other)
      : _sb(other._sb)
   {
   }

   /** Create a new view from an existing Eina_Strbuf */
   _strbuf_view_trait(Eina_Strbuf const* sb)
      : _sb(const_cast<Eina_Strbuf *>(sb))
   {
   }

   /**
    * @brief Tells whether this object contains a real Eina_Strbuf or not
    * @return true if wrapping an Eina_Strbuf, false if contains nullptr.
    */
   bool empty() const
   {
      return (_sb != nullptr);
   }

   /**
    * @brief Get the contained C string
    * @return A C-style string (const char*)
    */
   const char * c_str() const
   {
      return ::eina_strbuf_string_get(_sb);
   }

   /**
    * @brief Convert to a string
    * @return A std::string copy
    */
   operator std::string() const
   {
      return std::string(c_str());
   }

   /**
    * @brief Get the size of the string.
    * @return Number of characters in the string.
    */
   size_type size() const
   {
     return eina_strbuf_length_get(_sb);
   }

   /**
    * @brief Alias to @ref size() const.
    */
   size_type length() const
   {
     return size();
   }

   /**
    * @brief Get a constant iterator pointing to the first character of the string.
    * @return Constant iterator to the initial position of the string.
    *
    * This member function returns a constant iterator pointing to the
    * first character of the string. If the string is empty the iterator
    * is equal to the one returned by @ref end() const.
    */
   const_iterator begin() const
   {
      return ::eina_strbuf_string_get(_sb);
   }

   /**
    * @brief Get a constant iterator to the position following the last character of the string.
    * @return Constant iterator to the final position of the string.
    *
    * This member function returns an constant iterator to the position
    * following the last character in the string. If the string is empty
    * the iterator is equal to the one returned by @ref begin().
    *
    * @note Note that attempting to access this position causes undefined
    * behavior.
    */
   const_iterator end() const
   {
      return begin() + size();
   }

   /**
    * @brief Get a constant reverse iterator pointing to the reverse begin of the string.
    * @return Constant reverse iterator pointing to the reverse begin of the string.
    *
    * This member function returns a constant reverse iterator pointing
    * to the last character of the string. If the string is empty the
    * returned reverse iterator is the same as the one returned by
    * @ref rend() const.
    */
   const_reverse_iterator rbegin() const
   {
      return const_reverse_iterator(end());
   }

   /**
    * @brief Get a constant reverse iterator pointing to the reverse end of the string.
    * @return Constant reverse iterator pointing to the reverse end of the string.
    *
    * This member function returns a constant reverse iterator pointing
    * to the position before the first character of the string. If the
    * string is empty the returned iterator is the same as the one
    * returned by @ref rbegin() const.
    *
    * @note Note that attempting to access this position causes undefined
    * behavior.
    */
   const_reverse_iterator rend() const
   {
      return const_reverse_iterator(begin());
   }

   /**
    * @brief Get a constant iterator to the position following the last character of the string.
    * @return Constant iterator to the final position of the string.
    *
    * This member function works just like @ref end() const. But it is
    * granted to always return a constant iterator.
    */
   const_iterator cend() const
   {
      return end();
   }

   /**
    * @brief Get a constant reverse iterator pointing to the reverse begin of the string.
    * @return Constant reverse iterator pointing to the reverse begin of the string.
    *
    * This member function works just like @ref rbegin() const. But it is
    * granted to always return a constant reverse iterator.
    */
   const_reverse_iterator crbegin() const
   {
      return rbegin();
   }

   /**
    * @brief Get a constant reverse iterator pointing to the reverse end of the string.
    * @return Constant reverse iterator pointing to the reverse end of the string.
    *
    * This member function works just like @ref rend() const. But it is
    * granted to always return a constant reverse iterator.
    */
   const_reverse_iterator crend() const
   {
      return rend();
   }

   /**
    * @brief Get the maximum number of characters a string can hold.
    * @return Maximum number of characters a string can hold.
    */
   size_type max_size() const
   {
      return static_cast<size_type>(-1);
   }

   typedef strbuf_type* native_handle_type;
   native_handle_type native_handle() const
   {
      return _sb;
   }

protected:
   strbuf_type *_sb;
};


/** A read-only view of an existing Eina_Strbuf. */
typedef _strbuf_view_trait<const Eina_Strbuf, const char> strbuf_view;

static struct _strbuf_empty_t {} _strbuf_empty;


/** Allocates and manages an Eina_Strbuf, this is a mutable object. */
struct strbuf : public _strbuf_view_trait<Eina_Strbuf, char>
{
   strbuf(const char *str = nullptr)
      : _strbuf_view_trait(::eina_strbuf_new())
   {
      if (!_sb) EFL_CXX_THROW(std::make_error_code(std::errc::not_enough_memory));
      eina_strbuf_append(_sb, str);
   }

   explicit strbuf(_strbuf_empty_t)
      : _strbuf_view_trait(nullptr)
   {
   }

   explicit strbuf(Eina_Strbuf* sb) = delete;

   strbuf(Eina_Strbuf const* sb)
      : strbuf(eina_strbuf_string_get(sb))
   {
   }

   strbuf(Eina_Strbuf const& sb)
      : strbuf(eina_strbuf_string_get(&sb))
   {
   }

   strbuf(strbuf_view const& other)
      : strbuf(other.native_handle())
   {
   }

   strbuf(strbuf&& other)
      : strbuf(_strbuf_empty)
   {
      swap(other);
   }

   template <typename S>
   strbuf(S const& str)
      : strbuf(std::string(str).c_str())
   {
   }

   ~strbuf()
   {
      ::eina_strbuf_free(_sb);
   }

   void swap(strbuf& other)
   {
      std::swap(_sb, other._sb);
   }

   strbuf dup()
   {
      return strbuf(c_str());
   }

   strbuf& reset()
   {
      ::eina_strbuf_reset(_sb);
      return *this;
   }

   template <typename S>
   strbuf& append(S const& str)
   {
      ::eina_strbuf_append(_sb, std::string(str).c_str());
      return *this;
   }

   template <typename S, typename... Args>
   strbuf& append_printf(S const& fmt, Args... args)
   {
      ::eina_strbuf_append_printf(_sb, std::string(fmt).c_str(), args...);
      return *this;
   }

   template <typename S, typename... Args>
   strbuf& insert_printf(S const& fmt, size_t pos, Args... args)
   {
      ::eina_strbuf_insert_printf(_sb, std::string(fmt).c_str(), pos, args...);
      return *this;
   }

   template <typename S>
   strbuf& append_strftime(S const& fmt, struct tm const& time)
   {
      ::eina_strbuf_append_strftime(_sb, std::string(fmt).c_str(), &time);
      return *this;
   }

   template <typename S, typename... Args>
   strbuf& insert_strftime(S const& fmt, size_t pos, struct tm const& time)
   {
      ::eina_strbuf_insert_printf(_sb, std::string(fmt).c_str(), pos, time);
      return *this;
   }

   strbuf& trim()
   {
      ::eina_strbuf_trim(_sb);
      return *this;
   }

   strbuf& ltrim()
   {
      ::eina_strbuf_ltrim(_sb);
      return *this;
   }

   strbuf& rtrim()
   {
      ::eina_strbuf_rtrim(_sb);
      return *this;
   }

   // FIXME: add toupper!!
   strbuf& tolower()
   {
      ::eina_strbuf_tolower(_sb);
      return *this;
   }

   strbuf substr_get(size_t pos, size_t len)
   {
      strbuf sb(_strbuf_empty);
      sb._sb = ::eina_strbuf_substr_get(_sb, pos, len);
      return sb;
   }

   char * steal()
   {
      return ::eina_strbuf_string_steal(_sb);
   }

   template <typename S>
   strbuf& operator=(S const& other)
   {
      return reset().append(other);
   }

   template <typename S>
   strbuf& operator+=(S const& other)
   {
      return append(other);
   }
};


/** A writeable Eina_Strbuf wrapper, does not allocate or destroy the Eina_Strbuf. */
struct strbuf_wrapper : public strbuf
{
   strbuf_wrapper() = delete;
   strbuf_wrapper(strbuf_view const& other) = delete;

   explicit strbuf_wrapper(Eina_Strbuf* sb)
      : strbuf(_strbuf_empty)
   {
      _sb = sb;
   }

   strbuf_wrapper(Eina_Strbuf& sb)
      : strbuf_wrapper(&sb)
   {
   }

   ~strbuf_wrapper()
   {
      _sb = nullptr;
   }
};

} }

#endif
