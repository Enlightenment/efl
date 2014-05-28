#ifndef EINA_ACCESSOR_HH_
#define EINA_ACCESSOR_HH_

#include <Eina.h>
#include <eina_error.hh>

#include <memory>
#include <iterator>
#include <cstdlib>
#include <cassert>

namespace efl { namespace eina {

template <typename T>
struct accessor
{
  typedef unsigned int key_type;
  typedef T mapped_type;
  typedef T value_type;
  typedef std::size_t size_type;

  accessor() : _impl(0) {}
  explicit accessor(Eina_Accessor* impl)
    : _impl(impl)
  {
    assert(_impl != 0);
  }
  accessor(accessor<T> const& other)
    : _impl(eina_accessor_clone(other._impl))
  {}
  accessor<T>& operator=(accessor<T> const& other)
  {
    eina_accessor_free(_impl);
    _impl = eina_accessor_clone(other._impl);
    if(!_impl)
      throw eina::system_error(efl::eina::get_error_code(), "Error cloning accessor");
    return *this;
  }
  ~accessor()
  {
    eina_accessor_free(_impl);
  }
  
  mapped_type& operator[](size_type i) const
  {
    assert(_impl != 0);
    void* p;
    if(!eina_accessor_data_get(_impl, i, &p))
      {
        eina::error_code ec = efl::eina::get_error_code();
        throw eina::system_error(ec, "EFL Eina Error");
      }
    return *static_cast<mapped_type*>(p);
  }

  Eina_Accessor* native_handle() const;

  void swap(accessor<T>& other)
  {
    std::swap(_impl, other._impl);
  }
  explicit operator bool() const
  {
    return native_handle() ? &accessor<T>::native_handle : 0 ;
  }
private:
  Eina_Accessor* _impl;
};

template <typename U>
void swap(accessor<U>& lhs, accessor<U>& rhs)
{
  lhs.swap(rhs);
}

template <typename T>
struct accessor_iterator
{
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef std::ptrdiff_t difference_type;
  typedef std::random_access_iterator_tag iterator_category;

  accessor_iterator(accessor<T> const& a, unsigned int pos = 0u)
    : _accessor(a), _index(pos)
  {}

  accessor_iterator<T>& operator+=(difference_type i)
  {
    _index += i;
    return *this;
  }
  accessor_iterator<T>& operator-=(difference_type i)
  {
    _index -= i;
    return *this;
  }
  value_type& operator[](difference_type i)
  {
    return _accessor[_index + i];
  }
  accessor_iterator<T>& operator++()
  {
    ++_index;
    return *this;
  }
  accessor_iterator<T>& operator--()
  {
    --_index;
    return *this;
  }
  accessor_iterator<T>& operator++(int)
  {
    accessor_iterator<T> tmp(*this);
    ++*this;
    return tmp;
  }
  accessor_iterator<T>& operator--(int)
  {
    accessor_iterator<T> tmp(*this);
    --*this;
    return tmp;
  }
  value_type& operator*() const
  {
    return _accessor[_index];
  }
  pointer operator->() const
  {
    return &**this;
  }
  void swap(accessor_iterator<T>& other)
  {
    std::swap(_index, other._index);
    std::swap(_accessor, other._accessor);
  }
private:
  accessor<T> _accessor;
  unsigned int _index;

  template <typename U>
  friend bool operator==(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index == rhs._index;
  }

  template <typename U>
  friend typename accessor_iterator<U>::difference_type
  operator-(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index - rhs._index;
  }

  template <typename U>
  friend
  accessor_iterator<U> operator+(accessor_iterator<U> lhs
                                 , typename accessor_iterator<U>::difference_type rhs)
  {
    lhs._index += rhs;
    return lhs;
  }

  template <typename U>
  friend
  accessor_iterator<U> operator+(typename accessor_iterator<U>::difference_type lhs
                               , accessor_iterator<U> rhs)
  {
    return rhs + lhs;
  }

  template <typename U>
  friend bool operator<(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index < rhs._index;
  }

  template <typename U>
  friend bool operator<=(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index <= rhs._index;
  }
};

template <typename U>
bool operator>=(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
{
  return !(lhs < rhs);
}

template <typename U>
bool operator>(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
{
  return !(lhs <= rhs);
}

template <typename U>
bool operator!=(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
{
  return !(lhs == rhs);
}

template <typename U>
void swap(accessor_iterator<U>& lhs, accessor_iterator<U>& rhs)
{
  lhs.swap(rhs);
}

} }

#endif
