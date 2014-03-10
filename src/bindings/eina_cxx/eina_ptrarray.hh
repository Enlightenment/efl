#ifndef EINA_PTRARRAY_HH_
#define EINA_PTRARRAY_HH_

#include <Eina.h>
#include <eina_clone_allocators.hh>
#include <eina_type_traits.hh>
#include <eina_range_types.hh>

#include <memory>
#include <iterator>
#include <cstdlib>
#include <cassert>

namespace efl { namespace eina {

template <typename T>
struct _ptr_array_iterator
{
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef std::ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;

  _ptr_array_iterator() : _ptr(0) {}
  explicit _ptr_array_iterator(void** ptr)
    : _ptr(ptr)

  {
  }
  _ptr_array_iterator(_ptr_array_iterator<typename remove_cv<value_type>::type> const& other)
    : _ptr(other._ptr)
  {
  }
  _ptr_array_iterator<T>& operator++()
  {
    ++_ptr;
    return *this;
  }
  _ptr_array_iterator<T> operator++(int)
  {
    _ptr_array_iterator<T> tmp(*this);
    ++*this;
    return tmp;
  }
  _ptr_array_iterator<T>& operator--()
  {
    --_ptr;
    return *this;
  }
  _ptr_array_iterator<T> operator--(int)
  {
    _ptr_array_iterator<T> tmp(*this);
    --*this;
    return tmp;
  }
  reference operator*() const
  {
    return *static_cast<pointer>(*_ptr);
  }
  pointer operator->() const
  {
    return &**this;
  }
  void** native_handle() const
  {
    return _ptr;
  }
private:
  template <typename U>
  friend struct _ptr_array_iterator;
  friend inline bool operator==(_ptr_array_iterator<T> lhs, _ptr_array_iterator<T> rhs)
  {
    return lhs._ptr == rhs._ptr;
  }
  friend inline bool operator!=(_ptr_array_iterator<T> lhs, _ptr_array_iterator<T> rhs)
  {
    return !(lhs == rhs);
  }
  friend inline _ptr_array_iterator<T> operator+(_ptr_array_iterator<T> lhs
                                                 , difference_type size)
  {
    lhs._ptr += size;
    return lhs;
  }
  friend inline _ptr_array_iterator<T> operator-(_ptr_array_iterator<T> lhs
                                                 , difference_type size)
  {
    lhs._ptr -= size;
    return lhs;
  }
  friend inline difference_type operator-(_ptr_array_iterator<T> lhs
                                          , _ptr_array_iterator<T> rhs)
  {
    return lhs._ptr - rhs._ptr;
  }

  void** _ptr;
};

struct _ptr_array_access_traits {

template <typename T>

struct iterator
{
  typedef _ptr_array_iterator<T> type;
};
template <typename T>
struct const_iterator : iterator<T const>
{
};
template <typename T>
struct native_handle
{
  typedef Eina_Array* type;
};
template <typename T>
struct const_native_handle
{
  typedef Eina_Array const* type;
};
template <typename T>
static Eina_Array* native_handle_from_const(Eina_Array const* array)
{
  return const_cast<Eina_Array*>(array);
}
template <typename T>
static T& back(Eina_Array* array)
{
  return *static_cast<T*>(array->data[size<T>(array)-1]);
}
template <typename T>
static T const& back(Eina_Array const* array)
{
  return _ptr_array_access_traits::back<T>(const_cast<Eina_Array*>(array));
}
template <typename T>
static T& front(Eina_Array* array)
{
  return *static_cast<T*>(array->data[0]);
}
template <typename T>
static T const& front(Eina_Array const* array)
{
  return _ptr_array_access_traits::front<T>(const_cast<Eina_Array*>(array));
}
template <typename T>
static T& index(Eina_Array* array, std::size_t index)
{
  return *static_cast<T*>(array->data[index]);
}
template <typename T>
static T const& index(Eina_Array const* array, std::size_t index)
{
  return _ptr_array_access_traits::index<T>(const_cast<Eina_Array*>(array), index);
}
template <typename T>
static _ptr_array_iterator<T> begin(Eina_Array* array)
{
  return _ptr_array_iterator<T>(array->data);
}
template <typename T>
static _ptr_array_iterator<T> end(Eina_Array* array)
{
  return _ptr_array_iterator<T>(array->data + size<T>(array));
}
template <typename T>
static _ptr_array_iterator<T> begin(Eina_Array const* array)
{
  return _ptr_array_access_traits::begin<T>(const_cast<Eina_Array*>(array));
}
template <typename T>
static _ptr_array_iterator<T> end(Eina_Array const* array)
{
  return _ptr_array_access_traits::end<T>(const_cast<Eina_Array*>(array));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T> > rbegin(Eina_Array* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T> >(_ptr_array_access_traits::begin<T>(array));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T> > rend(Eina_Array* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T> >(_ptr_array_access_traits::end<T>(array));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T const> > rbegin(Eina_Array const* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T const> >(_ptr_array_access_traits::begin<T>(const_cast<Eina_Array*>(array)));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T const> > rend(Eina_Array const* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T const> >(_ptr_array_access_traits::end<T>(const_cast<Eina_Array*>(array)));
}
template <typename T>
static _ptr_array_iterator<T const> cbegin(Eina_Array const* array)
{
  return _ptr_array_access_traits::begin<T>(array);
}
template <typename T>
static _ptr_array_iterator<T const> cend(Eina_Array const* array)
{
  return _ptr_array_access_traits::end<T>(array);
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T const> > crbegin(Eina_Array const* array)
{
  return _ptr_array_access_traits::rbegin<T>(array);
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T const> > crend(Eina_Array const* array)
{
  return _ptr_array_access_traits::rend<T>(array);
}
template <typename T>
static eina::iterator<T> ibegin(Eina_Array* array)
{
  return eina::iterator<T>( ::eina_array_iterator_new(array) );
}
template <typename T>
static eina::iterator<T> iend(Eina_Array* array)
{
  return eina::iterator<T>();
}
template <typename T>
static eina::iterator<T const> ibegin(Eina_Array const* array)
{
  return eina::iterator<T const>( ::eina_array_iterator_new(array) );
}
template <typename T>
static eina::iterator<T const> iend(Eina_Array const* array)
{
  return eina::iterator<T const>();
}
template <typename T>
static eina::iterator<T const> cibegin(Eina_Array const* array)
{
  return _ptr_array_access_traits::ibegin<T>(array);
}
template <typename T>
static eina::iterator<T const> ciend(Eina_Array const* array)
{
  return _ptr_array_access_traits::iend<T>(array);
}
template <typename T>
static std::size_t size(Eina_Array const* array)
{
  return eina_array_count(array);
}
template <typename T>
static bool empty(Eina_Array const* array)
{
  return size<T>(array) == 0u;
}

};

template <typename T, typename Allocator>
struct ptr_array;

template <typename T>
struct range_ptr_array : _range_template<T, _ptr_array_access_traits>
{
  typedef _range_template<T, _ptr_array_access_traits> _base_type;
  typedef typename _base_type::value_type value_type;

  range_ptr_array(Eina_Array* array)
    : _base_type(array)
  {}
  template <typename Allocator>
  range_ptr_array(ptr_array<value_type, Allocator>& array)
    : _base_type(array.native_handle())
  {}
  value_type& operator[](std::size_t index) const
  {
    return _ptr_array_access_traits::index<T>(this->native_handle(), index);
  }
};

template <typename T, typename CloneAllocator>
struct _ptr_array_common_base
{
  typedef CloneAllocator clone_allocator_type;

  _ptr_array_common_base(CloneAllocator clone_allocator)
    : _impl(clone_allocator)
  {}
  _ptr_array_common_base(Eina_Array* _array)
    : _impl(_array)
  {}
  _ptr_array_common_base() {}

  CloneAllocator& _get_clone_allocator()
  {
    return _impl;
  }
  CloneAllocator const& _get_clone_allocator() const
  {
    return _impl;
  }
  void _delete_clone(T const* p)
  {
    _get_clone_allocator().deallocate_clone(p);
  }
  T* _new_clone(T const& a)
  {
    return _get_clone_allocator().allocate_clone(a);
  }

  struct _ptr_array_impl : CloneAllocator
  {
    _ptr_array_impl() : _array( ::eina_array_new(32u) ) {}
    _ptr_array_impl(CloneAllocator allocator)
      : clone_allocator_type(allocator), _array( ::eina_array_new(32u)) {}

    Eina_Array* _array;
  };  

  _ptr_array_impl _impl;

private:
  _ptr_array_common_base(_ptr_array_common_base const& other);
  _ptr_array_common_base& operator=(_ptr_array_common_base const& other);
};

template <typename T, typename CloneAllocator = heap_no_copy_allocator>
class ptr_array : protected _ptr_array_common_base<T, CloneAllocator>
{
  typedef _ptr_array_common_base<T, CloneAllocator> _base_type;
public:
  typedef T value_type;
  typedef T& reference;
  typedef T const& const_reference;
  typedef _ptr_array_iterator<T const> const_iterator;
  typedef _ptr_array_iterator<T> iterator;
  typedef T* pointer;
  typedef T const* const_pointer;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef CloneAllocator clone_allocator_type;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  ptr_array() {}
  ptr_array(size_type n, const_reference t)
  {
    while(n--)
      push_back(t);
  }
  template <typename InputIterator>
  ptr_array(InputIterator i, InputIterator const& j
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
  ptr_array(ptr_array<T, CloneAllocator> const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  template <typename CloneAllocator1>
  ptr_array(ptr_array<T, CloneAllocator1>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  ~ptr_array()
  {
    clear();
  }
  ptr_array<T, CloneAllocator>& operator=(ptr_array<T, CloneAllocator>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }
  
  void clear()
  {
    for(iterator first = begin(), last = end(); first != last; ++first)
      this->_delete_clone(&*first);
    eina_array_flush(this->_impl._array);
  }
  std::size_t size() const
  {
    return eina_array_count(this->_impl._array);
  }
  bool empty() const
  {
    return size() == 0u;
  }
  clone_allocator_type get_clone_allocator() const
  {
    return clone_allocator_type(this->_get_clone_allocator());
  }
  void push_back(const_reference a)
  {
    push_back(this->_new_clone(a));
  }
  void push_back(pointer p)
  {
    std::unique_ptr<value_type> p1(p);
    push_back(p1);
  }
  void push_back(std::unique_ptr<T>& p)
  {
    if(eina_array_push(this->_impl._array, p.get()))
      p.release();
    else
      throw std::bad_alloc();
  }
  void pop_back()
  {
    eina_array_pop(this->_impl._array);
  }
  iterator insert(iterator i, value_type const& t)
  {
    return insert(i, this->_new_clone(t));
  }  
  iterator insert(iterator i, pointer pv)
  {
    std::unique_ptr<value_type> p(pv);
    return insert(i, p);
  }
  iterator insert(iterator i, std::unique_ptr<value_type>& p)
  {
    std::size_t j
      = i.native_handle() - this->_impl._array->data
      , size = this->size();
    if(eina_array_push(this->_impl._array, p.get()))
      {
        if(size - j)
        {
          memmove(
                  this->_impl._array->data + j + 1
                  , this->_impl._array->data + j
                  , (size - j)*sizeof(void*));
          // PRE: Q:[j, size) = [j+1, size+1)
          pointer* data = static_cast<pointer*>
            (static_cast<void*>(this->_impl._array->data));
          data[j] = p.get();
        }
        p.release();
        return iterator(this->_impl._array->data + j);
      }
    else
      throw std::bad_alloc();
  }  
  iterator insert(iterator i, size_t n, value_type const& t)
  {
    iterator r = i;
    if(n--)
      r = insert(i, t);
    while(n--)
      insert(i, t);
    return r;
  }
  iterator insert(iterator i, size_t n, pointer p)
  {
    iterator r = i;
    if(n--)
      r = insert(i, p);
    while(n--)
      insert(i, this->_new_clone(p));
    return r;
  }
  template <typename InputIterator>
  iterator insert(iterator p, InputIterator i, InputIterator j
                  , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    size_type index = p.native_handle() - this->_impl._array->data;
    while(i != j)
      {
        p = insert(p, this->_new_clone(*i));
        ++p;
        ++i;
      }
    return iterator(this->_impl._array->data + index);
  }
  iterator erase(iterator q)
  {
    size_type size = this->size()
      , i = q.native_handle() - this->_impl._array->data;
    memmove(q.native_handle()
            , q.native_handle() + 1
            , (size - i - 1)*sizeof(void*));
    eina_array_pop(this->_impl._array);
    return q;
  }
  iterator erase(iterator i, iterator j)
  {
    size_type size = this->size()
      , distance = std::distance(i, j);
    memmove(i.native_handle()
            , j.native_handle()
            , (size - distance)*sizeof(void*));
    while(distance--)
      eina_array_pop(this->_impl._array);
    return i;
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
  value_type& back()
  {
    return _ptr_array_access_traits::back<T>(this->_impl._array);
  }
  value_type const& back() const
  {
    return _ptr_array_access_traits::back<T>(this->_impl._array);
  }
  value_type& front()
  {
    return _ptr_array_access_traits::front<T>(this->_impl._array);
  }
  value_type const& front() const
  {
    return _ptr_array_access_traits::front<T>(this->_impl._array);
  }
  const_reference operator[](size_type index) const
  {
    return _ptr_array_access_traits::index<T>(this->_impl._array, index);
  }
  reference operator[](size_type index)
  {
    return _ptr_array_access_traits::index<T>(this->_impl._array, index);
  }
  const_iterator begin() const
  {
    return _ptr_array_access_traits::begin<T>(this->_impl._array);
  }
  const_iterator end() const
  {
    return _ptr_array_access_traits::end<T>(this->_impl._array);
  }
  iterator begin()
  {
    return _ptr_array_access_traits::begin<T>(this->_impl._array);
  }
  iterator end()
  {
    return _ptr_array_access_traits::end<T>(this->_impl._array);
  }
  const_reverse_iterator rbegin() const
  {
    return _ptr_array_access_traits::rbegin<T>(this->_impl._array);
  }
  const_reverse_iterator rend() const
  {
    return _ptr_array_access_traits::rend<T>(this->_impl._array);
  }
  reverse_iterator rbegin()
  {
    return _ptr_array_access_traits::rbegin<T>(this->_impl._array);
  }
  reverse_iterator rend()
  {
    return _ptr_array_access_traits::rend<T>(this->_impl._array);
  }
  const_iterator cbegin() const
  {
    return _ptr_array_access_traits::cbegin<T>(this->_impl._array);
  }
  const_iterator cend() const
  {
    return _ptr_array_access_traits::cend<T>(this->_impl._array);
  }
  const_reverse_iterator crbegin() const
  {
    return _ptr_array_access_traits::crbegin<T>(this->_impl._array);
  }
  const_reverse_iterator crend() const
  {
    return _ptr_array_access_traits::crend<T>(this->_impl._array);
  }
  eina::iterator<T> ibegin()
  {
    return _ptr_array_access_traits::ibegin<T>(this->_impl._array);
  }
  eina::iterator<T> iend()
  {
    return _ptr_array_access_traits::iend<T>(this->_impl._array);
  }
  eina::iterator<T const> ibegin() const
  {
    return _ptr_array_access_traits::ibegin<T>(this->_impl._array);
  }
  eina::iterator<T const> iend() const
  {
    return _ptr_array_access_traits::iend<T>(this->_impl._array);
  }
  eina::iterator<T const> cibegin() const
  {
    return _ptr_array_access_traits::cibegin<T>(this->_impl._array);
  }
  eina::iterator<T const> ciend() const
  {
    return _ptr_array_access_traits::ciend<T>(this->_impl._array);
  }
  void swap(ptr_array<T, CloneAllocator>& other)
  {
    std::swap(this->_impl._array, other._impl._array);
  }
  size_type max_size() const { return -1; }

  Eina_Array* native_handle()
  {
    return this->_impl._array;
  }
  Eina_Array const* native_handle() const
  {
    return this->_impl._array;
  }
};

template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator==(ptr_array<T, CloneAllocator1> const& lhs, ptr_array<T, CloneAllocator2> const& rhs)
{
  return lhs.size() == rhs.size()
    && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator!=(ptr_array<T, CloneAllocator1> const& lhs, ptr_array<T, CloneAllocator2> const& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename CloneAllocator>
void swap(ptr_array<T, CloneAllocator>& lhs, ptr_array<T, CloneAllocator>& rhs)
{
  lhs.swap(rhs);
}

} }

#endif
