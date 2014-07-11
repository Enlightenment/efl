#ifndef EINA_ARRAY_HH_
#define EINA_ARRAY_HH_

#include <Eo.h>
#include <eina_ptrarray.hh>
#include <eina_eo_base_fwd.hh>

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

template <typename T, typename CloneAllocator>
class array<T, CloneAllocator, typename std::enable_if<std::is_base_of<efl::eo::base, T>::value>::type>
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

  explicit array(Eina_Array* handle)
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
    return cend();
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
  using _base_type::max_size;
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
  
} }

#endif
