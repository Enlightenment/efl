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

namespace efl { namespace eina {

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
