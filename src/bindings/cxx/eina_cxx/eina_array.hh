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
#ifndef EINA_ARRAY_HH_
#define EINA_ARRAY_HH_

#include <Eo.h>
#include <eina_ptrarray.hh>
#include <eina_eo_concrete_fwd.hh>

#include <iostream>
#include <initializer_list>
#include <memory>
#include <cstring>
#include <iterator>
#include <algorithm>
#include <vector>

namespace efl { namespace eina {

/**
 * @internal
 *
 * Iterator over an @c Eina_Array whose elements are strings.
 *
 * The array holds each string itself, a @c char*, where an array of objects
 * would hold a pointer to the object. So there is no @c T stored to refer to,
 * and dereferencing yields one built from the pointer rather than a reference
 * to one. That is why this exists rather than reusing the pointer-array
 * iterator, which reads the @c char* as if it were a @c T* and hands back
 * whatever the bytes of the string happen to look like.
 */
template <typename T>
struct _string_array_iterator
{
  typedef std::ptrdiff_t difference_type;
  typedef std::forward_iterator_tag iterator_category;
  typedef T value_type;
  typedef T reference;
  typedef void pointer;

  _string_array_iterator() : _ptr(nullptr) {}
  explicit _string_array_iterator(void** ptr) : _ptr(ptr) {}

  reference operator*() const
  {
    char const* s = static_cast<char const*>(*_ptr);
    return s ? T(s) : T();
  }

  _string_array_iterator<T>& operator++() { ++_ptr; return *this; }
  _string_array_iterator<T> operator++(int) { _string_array_iterator<T> t(*this); ++*this; return t; }
  _string_array_iterator<T>& operator--() { --_ptr; return *this; }
  _string_array_iterator<T> operator--(int) { _string_array_iterator<T> t(*this); --*this; return t; }

  void** native_handle() const { return _ptr; }

  friend inline bool operator==(_string_array_iterator<T> lhs, _string_array_iterator<T> rhs)
  { return lhs._ptr == rhs._ptr; }
  friend inline bool operator!=(_string_array_iterator<T> lhs, _string_array_iterator<T> rhs)
  { return !(lhs == rhs); }
  friend inline _string_array_iterator<T> operator+(_string_array_iterator<T> lhs, difference_type n)
  { lhs._ptr += n; return lhs; }
  friend inline _string_array_iterator<T> operator-(_string_array_iterator<T> lhs, difference_type n)
  { lhs._ptr -= n; return lhs; }
  friend inline difference_type operator-(_string_array_iterator<T> lhs, _string_array_iterator<T> rhs)
  { return lhs._ptr - rhs._ptr; }

private:
  void** _ptr;
};

/**
 * @internal
 *
 * True for element types that are the string itself rather than a pointer to
 * an object.
 */
template <typename T>
struct _is_string_element
  : std::integral_constant
    <bool
     , std::is_constructible<T, char const*>::value
       && !::efl::eo::is_eolian_object<T>::value>
{};


template <typename T, typename CloneAllocator = default_clone_allocator_placeholder, typename Enable = void>
class array : ptr_array<T, typename std::conditional
                      <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
                       , heap_no_copy_allocator, CloneAllocator>::type>
{
  typedef ptr_array<T, typename std::conditional
                   <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
                    , heap_no_copy_allocator, CloneAllocator>::type> _base_type;
public:
  typedef typename _base_type::value_type value_type; /**< The type of each element. */
  typedef typename _base_type::reference reference; /**< Type for a reference to an element. */
  typedef typename _base_type::const_reference const_reference; /**< Type for a constant reference to an element. */
  typedef typename _base_type::const_iterator const_iterator; /**< Type for a iterator for this container. */
  typedef typename _base_type::iterator iterator; /**< Type for a constant iterator for this container. */
  typedef typename _base_type::pointer pointer; /**< Type for a pointer to an element. */
  typedef typename _base_type::const_pointer const_pointer; /**< Type for a constant pointer for an element. */
  typedef typename _base_type::size_type size_type; /**< Type for size information used in the array. */
  typedef typename _base_type::difference_type difference_type; /**< Type to represent the distance between two iterators. */
  typedef typename _base_type::clone_allocator_type clone_allocator_type; /** Type for the clone allocator. */

  typedef typename _base_type::reverse_iterator reverse_iterator; /**< Type for reverse iterator for this container. */
  typedef typename _base_type::const_reverse_iterator const_reverse_iterator; /**< Type for reverse iterator for this container. */

  typedef typename _base_type::native_handle_type native_handle_type;

  using _base_type::_base_type;
  using _base_type::clear;
  using _base_type::size;
  using _base_type::empty;
  using _base_type::get_clone_allocator;
  using _base_type::push_back;
  using _base_type::pop_back;
  using _base_type::insert;
  using _base_type::erase;
  using _base_type::assign;
  using _base_type::back;
  using _base_type::front;
  using _base_type::operator[];
  using _base_type::begin;
  using _base_type::end;
  using _base_type::rbegin;
  using _base_type::rend;
  using _base_type::cbegin;
  using _base_type::cend;
  using _base_type::crbegin;
  using _base_type::crend;
  using _base_type::ibegin;
  using _base_type::iend;
  using _base_type::cibegin;
  using _base_type::ciend;
  using _base_type::swap;
  using _base_type::max_size;
  using _base_type::release_native_handle;
  using _base_type::native_handle;

  friend bool operator==(array<T, CloneAllocator> const& lhs, array<T, CloneAllocator> const& rhs)
  {
    return static_cast<_base_type const&>(lhs) == static_cast<_base_type const&>(rhs);
  }
};

/**
 * @internal
 */
template <typename T>
struct _ptr_eo_array_iterator : _ptr_array_iterator<Eo>
{
  typedef _ptr_array_iterator<Eo> _base_type;
  typedef _ptr_eo_array_iterator<T> _self_type;
  typedef typename remove_cv<T>::type value_type;
  typedef value_type* pointer;
  typedef value_type& reference;

  _base_type& _base() { return *this; }
  _base_type const& _base() const { return *this; }
  
  _ptr_eo_array_iterator(_base_type const& base) : _base_type(base) {}
  _ptr_eo_array_iterator() {}
  explicit _ptr_eo_array_iterator(void** ptr)
    : _base_type(ptr)
  {
  }
  _ptr_eo_array_iterator(_ptr_eo_array_iterator<value_type> const& other)
    : _base_type(static_cast<_base_type const&>(other))
  {
  }
  _self_type& operator=(_self_type const& other)
  {
    this->_base_type::operator=(other);
    return *this;
  }
  _self_type& operator++()
  {
    ++_base();
    return *this;
  }
  _self_type operator++(int)
  {
    _self_type tmp(*this);
    ++_base();
    return tmp;
  }
  _self_type& operator--()
  {
    --_base();
    return *this;
  }
  _self_type operator--(int)
  {
    _self_type tmp(*this);
    --_base();
    return tmp;
  }
  reference operator*() const
  {
    return *static_cast<pointer>(static_cast<void*>(native_handle()));
  }
  pointer operator->() const
  {
    return &**this;
  }

  using _base_type::native_handle;
};

/**
 * @internal
 */
struct _eo_array_access_traits : _ptr_array_access_traits
{
  template <typename T>
  struct iterator
  {
    typedef _ptr_eo_array_iterator<T> type;
  };
  template <typename T>
  struct const_iterator : iterator<T const> {};

  template <typename T>
  static T& back(Eina_Array* array)
  {
    return *static_cast<T*>(static_cast<void*>(array->data[size<T>(array)-1]));
  }
  template <typename T>
  static T const& back(Eina_Array const* array)
  {
    return _eo_array_access_traits::back<T>(const_cast<Eina_Array*>(array));
  }
  template <typename T>
  static T& front(Eina_Array* array)
  {
    return *static_cast<T*>(static_cast<void*>(array->data[0]));
  }
  template <typename T>
  static T const& front(Eina_Array const* array)
  {
    return _eo_array_access_traits::front<T>(const_cast<Eina_Array*>(array));
  }
  template <typename T>
  static typename iterator<T>::type begin(Eina_Array* array)
  {
    return _ptr_eo_array_iterator<T>(array->data);
  }
  template <typename T>
  static typename iterator<T>::type end(Eina_Array* array)
  {
    return _ptr_eo_array_iterator<T>(array->data + size<T>(array));
  }
  template <typename T>
  static typename const_iterator<T>::type begin(Eina_Array const* array)
  {
    return _eo_array_access_traits::begin<T>(const_cast<Eina_Array*>(array));
  }
  template <typename T>
  static typename const_iterator<T>::type end(Eina_Array const* array)
  {
    return _eo_array_access_traits::end<T>(const_cast<Eina_Array*>(array));
  }
  template <typename T>
  static std::reverse_iterator<typename iterator<T>::type> rbegin(Eina_Array* array)
  {
    return std::reverse_iterator<_ptr_eo_array_iterator<T> >(_eo_array_access_traits::end<T>(array));
  }
  template <typename T>
  static std::reverse_iterator<typename iterator<T>::type> rend(Eina_Array* array)
  {
    return std::reverse_iterator<_ptr_eo_array_iterator<T> >(_eo_array_access_traits::begin<T>(array));
  }
  template <typename T>
  static std::reverse_iterator<typename const_iterator<T>::type> rbegin(Eina_Array const* array)
  {
    return std::reverse_iterator<_ptr_eo_array_iterator<T>const>(_eo_array_access_traits::end<T>(array));
  }
  template <typename T>
  static std::reverse_iterator<typename const_iterator<T>::type> rend(Eina_Array const* array)
  {
    return std::reverse_iterator<_ptr_eo_array_iterator<T>const>(_eo_array_access_traits::begin<T>(array));
  }
  template <typename T>
  static typename const_iterator<T>::type cbegin(Eina_Array const* array)
  {
    return _eo_array_access_traits::begin<T>(array);
  }
  template <typename T>
  static typename const_iterator<T>::type cend(Eina_Array const* array)
  {
    return _eo_array_access_traits::end<T>(array);
  }
  template <typename T>
  static std::reverse_iterator<typename const_iterator<T>::type> crbegin(Eina_Array const* array)
  {
    return _eo_array_access_traits::rbegin<T>(array);
  }
  template <typename T>
  static std::reverse_iterator<typename const_iterator<T>::type> crend(Eina_Array const* array)
  {
    return _eo_array_access_traits::rend<T>(array);
  }
};
    
template <typename T, typename CloneAllocator>
class array<T, CloneAllocator, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type>
  : ptr_array<Eo, typename std::conditional
             <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
              , eo_clone_allocator, CloneAllocator>::type>
{
  typedef ptr_array<Eo, typename std::conditional
                   <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
                    , eo_clone_allocator, CloneAllocator>::type> _base_type;
  typedef array<T, CloneAllocator> _self_type;
public:
  typedef T value_type; /**< The type of each element. */
  typedef T& reference; /**< Type for a reference to an element. */
  typedef T const& const_reference; /**< Type for a constant reference to an element. */
  typedef _ptr_eo_array_iterator<T const> const_iterator; /**< Type for a iterator for this container. */
  typedef _ptr_eo_array_iterator<T> iterator; /**< Type for a constant iterator for this container. */
  typedef T* pointer; /**< Type for a pointer to an element. */
  typedef T const* const_pointer; /**< Type for a constant pointer for an element. */
  typedef typename _base_type::size_type size_type; /**< Type for size information used in the array. */
  typedef typename _base_type::difference_type difference_type; /**< Type to represent the distance between two iterators. */
  typedef typename _base_type::clone_allocator_type clone_allocator_type; /** Type for the clone allocator. */

  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for reverse iterator for this container. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /**< Type for reverse iterator for this container. */

  typedef typename _base_type::native_handle_type native_handle_type; /**< Type for the native handle of the container. */

  explicit array(native_handle_type handle)
    : _base_type(handle) {}
  array(clone_allocator_type alloc) : _base_type(alloc) {}
  array() {}
  array(size_type n, const_reference t)
  {
    while(n--)
      push_back(t);
  }
  template <typename InputIterator>
  array(InputIterator i, InputIterator const& j
           , clone_allocator_type const& alloc = clone_allocator_type()
           , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
    : _base_type(alloc)
  {
    while(i != j)
      {
        push_back(*i);
        ++i;
      }
  }
  array(array<T, CloneAllocator> const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  template <typename CloneAllocator1>
  array(array<T, CloneAllocator1>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  array<T, CloneAllocator>& operator=(array<T, CloneAllocator>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }
  
  using _base_type::clear;
  using _base_type::size;
  using _base_type::empty;
  using _base_type::get_clone_allocator;
  using _base_type::pop_back;

  void push_back(const_reference w)
  {
    _base_type::push_back(this->_new_clone(*w._eo_ptr()));
  }
  iterator insert(iterator i, value_type const& t)
  {
    return _base_type::insert(i, this->_new_clone(*t._eo_ptr()));
  }
  iterator insert(iterator i, size_t n, value_type const& t)
  {
    return _base_type::insert(i, n, this->_new_clone(*t._eo_ptr()));
  }
  template <typename InputIterator>
  iterator insert(iterator p, InputIterator i, InputIterator j
                  , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    size_type index = p.native_handle() - this->_impl._array->data;
    while(i != j)
      {
        p = insert(p, *i);
        ++p;
        ++i;
      }
    return iterator(this->_impl._array->data + index);
  }
  iterator erase(iterator p)
  {
    return _base_type::erase(p);
  }
  iterator erase(iterator i, iterator j)
  {
    return _base_type::erase(i, j);
  }
  template <typename InputIterator>
  void assign(InputIterator i, InputIterator j
              , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    clear();
    insert(end(), i, j);
  }

  void assign(size_type n, value_type const& t)
  {
    clear();
    insert(end(), n, t);
  }
  reference front()
  {
    return *static_cast<pointer>(static_cast<void*>(&native_handle()->data[0]));
  }
  reference back()
  {
    return *static_cast<pointer>(static_cast<void*>(&native_handle()->data[eina_array_count(native_handle())-1]));
  }
  const_reference front() const { return const_cast<_self_type*>(this)->front(); }
  const_reference back() const { return const_cast<_self_type*>(this)->back(); }

  const_reference operator[](size_type n) const
  {
    return *static_cast<pointer>(static_cast<void*>(&native_handle()->data[n]));
  }
  reference operator[](size_type n)
  {
    return *static_cast<pointer>(static_cast<void*>(&native_handle()->data[n]));
  }

  iterator begin()
  {
    return iterator(&native_handle()->data[0]);
  }
  iterator end()
  {
    return iterator(&native_handle()->data[ ::eina_array_count(native_handle())]);
  }
  const_iterator begin() const
  {
    return const_cast< _self_type*>(this)->begin();
  }
  const_iterator end() const
  {
    return const_cast< _self_type*>(this)->end();
  }
  reverse_iterator rbegin()
  {
    return reverse_iterator(end());
  }
  reverse_iterator rend()
  {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rbegin() const
  {
    return reverse_iterator(end());
  }
  const_reverse_iterator rend() const
  {
    return reverse_iterator(begin());
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
  eina::iterator<value_type> ibegin()
  {
    return _eo_array_access_traits::ibegin<value_type>(this->_impl._array);
  }
  eina::iterator<value_type> iend()
  {
    return _eo_array_access_traits::iend<value_type>(this->_impl._array);
  }

  eina::iterator<value_type const> ibegin() const
  {
    return _eo_array_access_traits::ibegin<value_type>(this->_impl._array);
  }

  eina::iterator<value_type const> iend() const
  {
    return _eo_array_access_traits::iend<value_type>(this->_impl._array);
  }
  eina::iterator<value_type const> cibegin() const
  {
    return _eo_array_access_traits::cibegin<value_type>(this->_impl._array);
  }
  eina::iterator<value_type const> ciend() const
  {
    return _eo_array_access_traits::ciend<value_type>(this->_impl._array);
  }
  
  using _base_type::swap;
  using _base_type::max_size;
  using _base_type::release_native_handle;
  using _base_type::native_handle;

  friend bool operator==(array<T, CloneAllocator> const& lhs, array<T, CloneAllocator> const& rhs)
  {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }
};

template <typename T, typename CloneAllocator>
bool operator!=(array<T, CloneAllocator> const& lhs, array<T, CloneAllocator> const& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Enable = void>
class range_array : range_ptr_array<T>
{
  typedef range_ptr_array<T> _base_type;
public:
  typedef typename _base_type::value_type value_type;
  typedef typename _base_type::reference reference;
  typedef typename _base_type::const_reference const_reference;
  typedef typename _base_type::const_iterator const_iterator;
  typedef typename _base_type::iterator iterator;
  typedef typename _base_type::pointer pointer;
  typedef typename _base_type::const_pointer const_pointer;
  typedef typename _base_type::size_type size_type;
  typedef typename _base_type::difference_type difference_type;

  typedef typename _base_type::reverse_iterator reverse_iterator;
  typedef typename _base_type::const_reverse_iterator const_reverse_iterator;

  typedef typename _base_type::native_handle_type native_handle_type;

  using _base_type::_base_type;

  using _base_type::size;
  using _base_type::empty;
  using _base_type::back;
  using _base_type::front;
  using _base_type::begin;
  using _base_type::end;
  using _base_type::rbegin;
  using _base_type::rend;
  using _base_type::cbegin;
  using _base_type::cend;
  using _base_type::crbegin;
  using _base_type::crend;
  using _base_type::release_native_handle;
  using _base_type::native_handle;

};

/**
 * @ingroup Eina_Cxx_Containers_Group
 *
 * Owning array of strings.
 *
 * The counterpart of @ref range_array for strings, for the same reason: an
 * @c Eina_Array of objects holds pointers to them, an @c Eina_Array of strings
 * holds each string, and only the first arrangement can be read by
 * dereferencing. Where the range only reads, this one owns: every string that
 * goes in is copied, and every string still in it is freed when it goes.
 *
 * It does not derive from @ref ptr_array the way the generic array does. That
 * one reads an element by reinterpreting the array's slot as a @c T*, which
 * works for an Eo wrapper because a wrapper is a pointer and nothing else. A
 * @c string_view is a pointer and a length, so there is nothing in the array to
 * reinterpret, and elements come back by value instead.
 *
 * The clone allocator parameter is accepted for symmetry and not used: strings
 * are copied with @c strdup and released with @c free.
 */
template <typename T, typename CloneAllocator>
class array<T, CloneAllocator, typename std::enable_if<_is_string_element<T>::value>::type>
{
  typedef array<T, CloneAllocator> _self_type;
public:
  typedef T value_type; /**< The type of each element. */
  /** By value: the array holds strings, not @c T objects, so there is nothing
   * to hand back a reference to. */
  typedef T reference;
  typedef T const_reference;
  typedef _string_array_iterator<T> iterator;
  typedef _string_array_iterator<T> const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef ::Eina_Array* native_handle_type;

  array() : _array(::eina_array_new(1)) {}

  /** Takes over an array the caller was holding, and the strings in it. */
  explicit array(native_handle_type handle)
    : _array(handle ? handle : ::eina_array_new(1))
  {}

  array(std::initializer_list<const char*> strings)
    : _array(::eina_array_new(strings.size() ? strings.size() : 1))
  {
     for (const char* str : strings)
       _push(str);
  }

  template <typename InputIterator>
  array(InputIterator i, InputIterator const& j
        , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
    : _array(::eina_array_new(1))
  {
     for (; i != j; ++i)
       push_back(*i);
  }

  array(_self_type const& other)
    : _array(::eina_array_new(other.size() ? other.size() : 1))
  {
     for (size_type n = 0u; n != other.size(); ++n)
       _push(other._raw(n));
  }

  array(_self_type&& other) noexcept
    : _array(other._array)
  {
     other._array = nullptr;
  }

  _self_type& operator=(_self_type const& other)
  {
     if (this != &other)
       {
          _self_type copy(other);
          swap(copy);
       }
     return *this;
  }

  _self_type& operator=(_self_type&& other) noexcept
  {
     if (this != &other)
       {
          _dispose();
          _array = other._array;
          other._array = nullptr;
       }
     return *this;
  }

  ~array() { _dispose(); }

  size_type size() const { return _array ? ::eina_array_count(_array) : 0u; }
  bool empty() const { return size() == 0u; }
  size_type max_size() const { return -1; }

  void clear()
  {
     _free_elements();
     if (_array)
       ::eina_array_clean(_array);
  }

  void push_back(value_type const& v) { _push(v.data(), v.size()); }
  void push_back(const char* str) { _push(str); }

  void pop_back()
  {
     if (!empty())
       ::free(::eina_array_pop(_array));
  }

  value_type operator[](size_type index) const { return _at(index); }
  value_type front() const { return _at(0); }
  value_type back() const { return _at(size() - 1); }

  iterator begin() const { return iterator(_data()); }
  iterator end() const { return iterator(_data() + size()); }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }
  const_reverse_iterator crbegin() const { return rbegin(); }
  const_reverse_iterator crend() const { return rend(); }

  /** Inserts before @p p. Iterators are invalidated, as for a vector. */
  iterator insert(iterator p, value_type const& v)
  {
     size_type const index = static_cast<size_type>(p.native_handle() - _data());
     std::vector<char*> kept = _take();
     kept.insert(kept.begin() + index, _dup(v.data(), v.size()));
     _rebuild(kept);
     return begin() + static_cast<difference_type>(index);
  }

  template <typename InputIterator>
  iterator insert(iterator p, InputIterator i, InputIterator j
                  , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
     size_type const index = static_cast<size_type>(p.native_handle() - _data());
     std::vector<char*> kept = _take();
     size_type at = index;
     for (; i != j; ++i, ++at)
       {
          value_type const v = *i;
          kept.insert(kept.begin() + at, _dup(v.data(), v.size()));
       }
     _rebuild(kept);
     return begin() + static_cast<difference_type>(index);
  }

  iterator erase(iterator p) { return erase(p, p + 1); }

  iterator erase(iterator i, iterator j)
  {
     size_type const first = static_cast<size_type>(i.native_handle() - _data());
     size_type const last = static_cast<size_type>(j.native_handle() - _data());
     std::vector<char*> kept = _take();
     for (size_type n = first; n != last; ++n)
       ::free(kept[n]);
     kept.erase(kept.begin() + first, kept.begin() + last);
     _rebuild(kept);
     return begin() + static_cast<difference_type>(first);
  }

  template <typename InputIterator>
  void assign(InputIterator i, InputIterator j
              , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
     clear();
     for (; i != j; ++i)
       push_back(*i);
  }

  void swap(_self_type& other) { std::swap(_array, other._array); }

  native_handle_type native_handle() const { return _array; }

  /** Hands the array and its strings to the caller; this one is left empty. */
  native_handle_type release_native_handle()
  {
     native_handle_type released = _array;
     _array = ::eina_array_new(1);
     return released;
  }

  friend bool operator==(_self_type const& lhs, _self_type const& rhs)
  {
     return lhs.size() == rhs.size()
       && std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  friend bool operator!=(_self_type const& lhs, _self_type const& rhs)
  {
     return !(lhs == rhs);
  }

private:
  void** _data() const { return _array ? _array->data : nullptr; }
  char const* _raw(size_type index) const { return static_cast<char const*>(_array->data[index]); }

  value_type _at(size_type index) const
  {
     char const* s = _raw(index);
     return s ? T(s) : T();
  }

  static char* _dup(char const* s, size_type n)
  {
     if (!s)
       return nullptr;
     char* copy = static_cast<char*>(::malloc(n + 1));
     if (copy)
       {
          ::memcpy(copy, s, n);
          copy[n] = '\0';
       }
     return copy;
  }

  void _push(char const* s) { ::eina_array_push(_array, s ? ::strdup(s) : nullptr); }
  void _push(char const* s, size_type n) { ::eina_array_push(_array, _dup(s, n)); }

  /** Empties the Eina_Array into a vector without freeing the strings. */
  std::vector<char*> _take()
  {
     std::vector<char*> kept;
     kept.reserve(size());
     for (size_type n = 0u; n != size(); ++n)
       kept.push_back(static_cast<char*>(_array->data[n]));
     ::eina_array_clean(_array);
     return kept;
  }

  void _rebuild(std::vector<char*> const& kept)
  {
     for (char* s : kept)
       ::eina_array_push(_array, s);
  }

  void _free_elements()
  {
     if (!_array)
       return;
     for (size_type n = 0u; n != size(); ++n)
       ::free(_array->data[n]);
  }

  void _dispose()
  {
     if (!_array)
       return;
     _free_elements();
     ::eina_array_free(_array);
     _array = nullptr;
  }

  ::Eina_Array* _array;
};

/**
 * @ingroup Eina_Cxx_Containers_Group
 *
 * Range over an @c Eina_Array of strings.
 *
 * Eolian gives every @c array<string> this type, in both directions. Going out
 * it can be built from a brace list; coming back, each element is constructed
 * from the @c char* the array holds. The generic range would have read that
 * @c char* as a @c T* and dereferenced it, so reading a returned string array
 * never worked.
 */
template <typename T>
class range_array<T, typename std::enable_if<_is_string_element<T>::value>::type>
{
public:
  typedef T value_type;
  /** By value: the array holds strings, not @c T objects, so there is nothing
   * to hand back a reference to. */
  typedef T reference;
  typedef T const_reference;
  typedef _string_array_iterator<T> iterator;
  typedef _string_array_iterator<T> const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef ::Eina_Array* native_handle_type;

  range_array(native_handle_type array = nullptr)
    : _array(array)
  {}

  /**
   * @brief Build a range over a brace list of strings.
   *
   * @code
   * model.filter_patterns_set({"*.h", "*.hh"});
   * @endcode
   *
   * Otherwise this takes an @c Eina_Array the caller has to build by hand --
   * eina_array_new, a push per element, and a free afterwards -- which is four
   * lines of C in the middle of C++ to say a list of two globs.
   *
   * The array and copies of the strings are owned by the range and released
   * when the last copy of it goes, so the usual case of passing a brace list
   * straight to a setter cleans up at the end of the full expression. Copying
   * shares that ownership; a range built the other way, from an @c Eina_Array
   * somebody else owns, holds nothing and frees nothing, exactly as before.
   *
   * Setters that take an array copy what they keep -- Efl.Io.Model's
   * filter_patterns says so in as many words -- so the temporary outliving the
   * call is not required, only the call itself.
   */
  range_array(std::initializer_list<const char*> strings)
    : _array(_build(strings))
    , _owned(_array, &range_array::_release)
  {
  }

  size_type size() const { return _array ? ::eina_array_count(_array) : 0u; }
  bool empty() const { return size() == 0u; }

  value_type operator[](size_type index) const { return _at(index); }
  value_type front() const { return _at(0); }
  value_type back() const { return _at(size() - 1); }

  iterator begin() const { return iterator(_data()); }
  iterator end() const { return iterator(_data() + size()); }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }
  const_reverse_iterator crbegin() const { return rbegin(); }
  const_reverse_iterator crend() const { return rend(); }

  native_handle_type native_handle() const { return _array; }

  native_handle_type release_native_handle()
  {
     native_handle_type released = _array;
     _array = nullptr;
     _owned.reset();
     return released;
  }

  void swap(range_array<T>& other)
  {
     std::swap(_array, other._array);
     std::swap(_owned, other._owned);
  }

private:
  value_type _at(size_type index) const
  {
     char const* s = static_cast<char const*>(_array->data[index]);
     return s ? T(s) : T();
  }

  void** _data() const { return _array ? _array->data : nullptr; }

private:
  /** Built before the base is constructed, so that the range and the storage
   * it owns come into being in the right order. */
  static ::Eina_Array* _build(std::initializer_list<const char*> strings)
  {
     ::Eina_Array* array = ::eina_array_new(strings.size() ? strings.size() : 1);

     for (const char* s : strings)
       ::eina_array_push(array, s ? ::strdup(s) : nullptr);

     return array;
  }

  static void _release(::Eina_Array* array)
  {
     unsigned int i;
     ::Eina_Array_Iterator it;
     void* item;

     EINA_ARRAY_ITER_NEXT(array, i, item, it)
       ::free(item);

     ::eina_array_free(array);
  }

  /** Null unless this range built the array itself; see the brace-list
   * constructor. */
  ::Eina_Array* _array;
  /** Null unless this range built the array itself; see the brace-list
   * constructor. */
  std::shared_ptr< ::Eina_Array> _owned;
};

template <typename T>
bool operator==(range_array<T, typename std::enable_if<_is_string_element<T>::value>::type> const& lhs
                , range_array<T, typename std::enable_if<_is_string_element<T>::value>::type> const& rhs)
{
  return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
bool operator!=(range_array<T, typename std::enable_if<_is_string_element<T>::value>::type> const& lhs
                , range_array<T, typename std::enable_if<_is_string_element<T>::value>::type> const& rhs)
{
  return !(lhs == rhs);
}

template <typename T>
class range_array<T, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value>::type>
  : range_ptr_array<Eo>
{
  typedef range_ptr_array<Eo> _base_type;
  typedef range_array<T> _self_type;
public:
  typedef T value_type;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef _ptr_eo_array_iterator<value_type const> const_iterator;
  typedef _ptr_eo_array_iterator<value_type> iterator;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef typename _base_type::native_handle_type native_handle_type;

  explicit range_array(native_handle_type handle)
    : _base_type(handle) {}
  range_array() {}
  range_array(range_array<T> const& other)
    : _base_type(other.native_handle())
  {
  }
  range_array<T>& operator=(range_array<T>const& other)
  {
    _base_type::_handle = other._handle;
    return *this;
  }
  range_array& operator=(range_array&& other) = default;
  range_array(range_array&& other) = default;
  
  using _base_type::size;
  using _base_type::empty;

  reference front()
  {
    return _eo_array_access_traits::front<value_type>(native_handle());
  }
  reference back()
  {
    return _eo_array_access_traits::back<value_type>(native_handle());
  }
  const_reference front() const { return const_cast<_self_type*>(this)->front(); }
  const_reference back() const { return const_cast<_self_type*>(this)->back(); }
  iterator begin()
  {
    return _eo_array_access_traits::begin<value_type>(native_handle());
  }
  iterator end()
  {
    return _eo_array_access_traits::end<value_type>(native_handle());
  }
  const_iterator begin() const
  {
    return const_cast<_self_type*>(this)->begin();
  }
  const_iterator end() const
  {
    return const_cast<_self_type*>(this)->end();
  }
  const_iterator cbegin() const
  {
    return begin();
  }
  const_iterator cend() const
  {
    return end();
  }
  reverse_iterator rbegin()
  {
    return _eo_array_access_traits::rbegin<value_type>(native_handle());
  }
  reverse_iterator rend()
  {
    return _eo_array_access_traits::rend<value_type>(native_handle());
  }
  const_reverse_iterator rbegin() const
  {
    return const_cast<_self_type*>(this)->rbegin();
  }
  const_reverse_iterator rend() const
  {
    return const_cast<_self_type*>(this)->rend();
  }
  const_reverse_iterator crbegin() const
  {
    return rbegin();
  }
  const_reverse_iterator crend() const
  {
    return rend();
  }
  using _base_type::swap;
  using _base_type::release_native_handle;
  using _base_type::native_handle;

  friend bool operator==(range_array<T> const& rhs, range_array<T> const& lhs)
  {
    return rhs.size() == lhs.size() && std::equal(rhs.begin(), rhs.end(), lhs.begin());
  }
};

template <typename T>
using crange_array = range_array<typename std::add_const<T>::type>;

} }

#endif
