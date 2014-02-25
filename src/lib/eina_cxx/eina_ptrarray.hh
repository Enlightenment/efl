#ifndef EINA_PTRARRAY_HH_
#define EINA_PTRARRAY_HH_

#include <Eina.h>
#include <eina_clone_allocators.hh>
#include <eina_lists_auxiliary.hh>
#include <eina_type_traits.hh>

#include <memory>
#include <iterator>
#include <cstdlib>
#include <cassert>

namespace efl { namespace eina {

struct _ptr_array_iterator_base
{
  _ptr_array_iterator_base() : _ptr(0) {}
  _ptr_array_iterator_base(void** ptr)
    : _ptr(ptr)
  {}

  void** _ptr;
};

template <typename T>
struct _ptr_array_iterator : protected _ptr_array_iterator_base
{
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef std::ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;

  _ptr_array_iterator() {}
  explicit _ptr_array_iterator(void** ptr)
    : _ptr_array_iterator_base(ptr)

  {
  }
  _ptr_array_iterator(_ptr_array_iterator<typename remove_cv<value_type>::type> const& other)
    : _ptr_array_iterator_base(static_cast<_ptr_array_iterator_base const&>(other))
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
    std::auto_ptr<value_type> p1(p);
    push_back(p1);
  }
  void push_back(std::auto_ptr<T>& p)
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
    std::auto_ptr<value_type> p(pv);
    return insert(i, p);
  }
  iterator insert(iterator i, std::auto_ptr<value_type>& p)
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
    return *static_cast<pointer>(this->_impl._array->data[size()-1]);
  }
  value_type const& back() const
  {
    return const_cast<ptr_array<T, CloneAllocator>&>(*this).back();
  }
  value_type& front()
  {
    return *static_cast<pointer>(this->_impl._array->data[0]);
  }
  value_type const& front() const
  {
    return const_cast<ptr_array<T, CloneAllocator>&>(*this).front();
  }

  const_reference operator[](size_type index) const
  {
    pointer data = static_cast<pointer>
      (this->_impl._array->data[index]);
    return *data;
  }
  reference operator[](size_type index)
  {
    return const_cast<reference>
      (const_cast<ptr_array<T, CloneAllocator>const&>(*this)[index]);
  }

  const_iterator begin() const
  {
    return const_iterator(this->_impl._array->data);
  }
  const_iterator end() const
  {
    return const_iterator(this->_impl._array->data + size());
  }
  iterator begin()
  {
    return iterator(this->_impl._array->data);
  }
  iterator end()
  {
    return iterator(this->_impl._array->data + size());
  }
  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(end());
  }
  reverse_iterator rbegin()
  {
    return reverse_iterator(begin());
  }
  reverse_iterator rend()
  {
    return reverse_iterator(end());
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
  eina::iterator<T> ibegin()
  {
    return eina::iterator<T>( ::eina_array_iterator_new(this->_impl._array) );
  }
  eina::iterator<T> iend()
  {
    return eina::iterator<T>();
  }
  eina::iterator<T const> ibegin() const
  {
    return eina::iterator<T const>( ::eina_array_iterator_new(this->_impl._array) );
  }
  eina::iterator<T const> iend() const
  {
    return eina::iterator<T const>();
  }
  eina::iterator<T const> cibegin() const
  {
    return ibegin();
  }
  eina::iterator<T const> ciend() const
  {
    return iend();
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
