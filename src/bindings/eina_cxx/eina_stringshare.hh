#ifndef _EINA_STRINGSHARE_HH
#define _EINA_STRINGSHARE_HH

#include <Eina.h>
#include <eina_type_traits.hh>

#include <cstring>
#include <stdexcept>

namespace efl { namespace eina {

struct steal_stringshare_ref_t {};
steal_stringshare_ref_t const steal_stringshare_ref = {};

struct stringshare
{
  typedef char value_type;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::ptrdiff_t difference_type;
  typedef std::size_t size_type;

  stringshare()
    : _string( ::eina_stringshare_add("") )
  {}
  stringshare(const char* str)
    : _string( ::eina_stringshare_add(str) )
  {
  }
  stringshare(char* str, steal_stringshare_ref_t)
    : _string( str )
  {
  }
  template <typename InputIterator>
  stringshare(InputIterator i, InputIterator j
              , typename eina::enable_if
              <!eina::is_integral<InputIterator>::value
              && !eina::is_contiguous_iterator<InputIterator>::value
              >::type* = 0)
  {
    std::string tmp;
    while(i != j)
      {
        tmp.push_back(*i);
        ++i;
      }
    _string = ::eina_stringshare_add(tmp.c_str());
  }
  template <typename ContiguousMemoryIterator>
  stringshare(ContiguousMemoryIterator i, ContiguousMemoryIterator j
              , typename eina::enable_if
              <eina::is_contiguous_iterator<ContiguousMemoryIterator>::value>::type* = 0)
    : _string( ::eina_stringshare_add_length(&*i, j - i) )
  {
  }

  ~stringshare()
  {
    ::eina_stringshare_del(_string);
  }

  stringshare(stringshare const& other)
    : _string( eina_stringshare_ref(other._string) )
  {}
  stringshare& operator=(stringshare const& other)
  {
    ::eina_stringshare_refplace(&_string, other._string);
    return *this;
  }
  stringshare& operator=(const char* c_string)
  {
    ::eina_stringshare_replace(&_string, c_string);
    return *this;
  }
  
  const_iterator begin() const
  {
    return _string;
  }
  const_iterator end() const
  {
    return _string + size();
  }

  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }

  const_iterator cbegin() const
  {
    return begin();
  }
  const_iterator cend() const
  {
    return end();
  }
  const_reverse_iterator crbegin() const
  {
    return rbegin();
  }
  const_reverse_iterator crend() const
  {
    return rend();
  }

  size_type size() const
  {
    return eina_stringshare_strlen(_string);
  }

  size_type length() const
  {
    return size();
  }
  size_type max_size() const
  {
    return -1;
  }
  bool empty() const
  {
    return _string[0] == 0;
  }
  const_reference operator[](size_type i) const
  {
    return _string[i];
  }
  const_reference at(size_type i) const
  {
    if(i < size())
      return (*this)[i];
    else
      throw std::out_of_range("");
  }
  const_reference back() const
  {
    return _string[size()-1];
  }
  const_reference front() const
  {
    return _string[0];
  }

  void swap(stringshare& other)
  {
    std::swap(_string, other._string);
  }

  const char* c_str() const
  {
    return _string; 
  }
  const char* data() const
  {
    return _string; 
  }
  
private:
  Eina_Stringshare* _string;
};

template <>
struct is_contiguous_iterator<stringshare::const_iterator> : true_type {};

inline bool operator==(stringshare const& lhs, stringshare const& rhs)
{
  return lhs.c_str() == rhs.c_str();
}

inline bool operator!=(stringshare const& lhs, stringshare const& rhs)
{
  return !(lhs == rhs);
}

inline bool operator==(stringshare const& lhs, const char* rhs)
{
  return lhs.c_str() == rhs || std::strcmp(lhs.c_str(), rhs) == 0;
}

inline bool operator!=(stringshare const& lhs, const char* rhs)
{
  return !(lhs == rhs);
}

inline bool operator==(const char* lhs, stringshare const& rhs)
{
  return rhs == lhs;
}

inline bool operator!=(const char* lhs, stringshare const& rhs)
{
  return !(lhs == rhs);
}

} }

#endif
