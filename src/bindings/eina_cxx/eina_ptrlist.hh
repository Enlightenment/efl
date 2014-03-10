#ifndef EINA_LIST_HH_
#define EINA_LIST_HH_

#include <Eina.h>
#include <eina_clone_allocators.hh>
#include <eina_lists_auxiliary.hh>
#include <eina_type_traits.hh>
#include <eina_accessor.hh>

#include <memory>
#include <iterator>

namespace efl { namespace eina {

struct _ptr_list_iterator_base
{
  typedef std::ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;

  _ptr_list_iterator_base() : _list(0) {}
  _ptr_list_iterator_base(Eina_List* list, Eina_List* node)
    : _list(list), _node(node)
  {}

protected:
  Eina_List *_list, *_node;
};

template <typename T>
struct _ptr_list_iterator : _ptr_list_iterator_base
{
  typedef typename remove_cv<T>::type value_type;
  typedef value_type* pointer;
  typedef value_type& reference;

  _ptr_list_iterator() {}
  explicit _ptr_list_iterator(Eina_List* list, Eina_List* node)
    : _ptr_list_iterator_base(list, node)
  {
  }
  _ptr_list_iterator(_ptr_list_iterator<value_type> const& other)
    : _ptr_list_iterator_base(static_cast<_ptr_list_iterator_base const&>(other))
  {
  }

  _ptr_list_iterator<T>& operator++()
  {
    _node = eina_list_next(_node);
    return *this;
  }
  _ptr_list_iterator<T> operator++(int)
  {
    _ptr_list_iterator<T> tmp(*this);
    ++*this;
    return tmp;
  }
  _ptr_list_iterator<T>& operator--()
  {
    if(_node)
      _node = eina_list_prev(_node);
    else
      _node = eina_list_last(_list);
    return *this;
  }
  _ptr_list_iterator<T> operator--(int)
  {
    _ptr_list_iterator<T> tmp(*this);
    --*this;
    return tmp;
  }
  reference operator*() const
  {
    void* data = eina_list_data_get(_node);
    return *static_cast<pointer>(data);
  }
  pointer operator->() const
  {
    return &**this;
  }
  Eina_List* native_handle()
  {
    return _node;
  }
  Eina_List const* native_handle() const
  {
    return _node;
  }
  friend inline bool operator==(_ptr_list_iterator<T> lhs, _ptr_list_iterator<T> rhs)
  {
    return lhs._node == rhs._node;
  }
  friend inline bool operator!=(_ptr_list_iterator<T> lhs, _ptr_list_iterator<T> rhs)
  {
    return !(lhs == rhs);
  }
};

struct _ptr_list_access_traits {

template <typename T>
struct iterator
{
  typedef _ptr_list_iterator<T> type;
};
template <typename T>
struct const_iterator : iterator<T const> {};
template <typename T>
struct native_handle
{
  typedef Eina_List* type;
};
template <typename T>
struct const_native_handle
{
  typedef Eina_List const* type;
};
template <typename T>
static Eina_List* native_handle_from_const(Eina_List const* list)
{
  return const_cast<Eina_List*>(list);
}
template <typename T>
static T& back(Eina_List* list)
{
  return *static_cast<T*>(eina_list_data_get(eina_list_last(list)));
}
template <typename T>
static T const& back(Eina_List const* list)
{
  return _ptr_list_access_traits::back<T>(const_cast<Eina_List*>(list));
}
template <typename T>
static T& front(Eina_List* list)
{
  return *static_cast<T*>(eina_list_data_get(list));
}
template <typename T>
static T const& front(Eina_List const* list)
{
  return _ptr_list_access_traits::front<T>(const_cast<Eina_List*>(list));
}
template <typename T>
static _ptr_list_iterator<T> begin(Eina_List* list)
{
  return _ptr_list_iterator<T>(list, list);
}
template <typename T>
static _ptr_list_iterator<T> end(Eina_List* list)
{
  return _ptr_list_iterator<T>(list, 0);
}
template <typename T>
static _ptr_list_iterator<T const> begin(Eina_List const* list)
{
  return _ptr_list_access_traits::begin<T>(const_cast<Eina_List*>(list));
}
template <typename T>
static _ptr_list_iterator<T const> end(Eina_List const* list)
{
  return _ptr_list_access_traits::end<T>(const_cast<Eina_List*>(list));
}
template <typename T>
static std::reverse_iterator<_ptr_list_iterator<T> > rbegin(Eina_List* list)
{
  return std::reverse_iterator<_ptr_list_iterator<T> >(_ptr_list_access_traits::begin<T>(list));
}
template <typename T>
static std::reverse_iterator<_ptr_list_iterator<T> > rend(Eina_List* list)
{
  return std::reverse_iterator<_ptr_list_iterator<T> >(_ptr_list_access_traits::end<T>(list));
}
template <typename T>
static std::reverse_iterator<_ptr_list_iterator<T const> > rbegin(Eina_List const* list)
{
  return _ptr_list_access_traits::rbegin<T>(const_cast<Eina_List*>(list));
}
template <typename T>
static std::reverse_iterator<_ptr_list_iterator<T const> > rend(Eina_List const* list)
{
  return _ptr_list_access_traits::rend<T>(const_cast<Eina_List*>(list));
}
template <typename T>
static _ptr_list_iterator<T const> cbegin(Eina_List const* list)
{
  return _ptr_list_access_traits::begin<T>(list);
}
template <typename T>
static _ptr_list_iterator<T const> cend(Eina_List const* list)
{
  return _ptr_list_access_traits::end<T>(list);
}
template <typename T>
static std::reverse_iterator<_ptr_list_iterator<T const> > crbegin(Eina_List const* list)
{
  return _ptr_list_access_traits::rbegin<T>(list);
}
template <typename T>
static std::reverse_iterator<_ptr_list_iterator<T const> > crend(Eina_List const* list)
{
  return _ptr_list_access_traits::rend<T>(list);
}
template <typename T>
static eina::iterator<T> ibegin(Eina_List* list)
{
  return eina::iterator<T>( ::eina_list_iterator_new(list) );
}
template <typename T>
static eina::iterator<T> iend(Eina_List*)
{
  return eina::iterator<T>();
}
template <typename T>
static eina::iterator<T const> ibegin(Eina_List const* list)
{
  return eina::iterator<T const>( ::eina_list_iterator_new(list) );
}
template <typename T>
static eina::iterator<T const> iend(Eina_List const* list)
{
  return eina::iterator<T const>();
}
template <typename T>
static eina::iterator<T const> cibegin(Eina_List const* list)
{
  return _ptr_list_access_traits::ibegin<T>(list);
}
template <typename T>
static eina::iterator<T const> ciend(Eina_List const* list)
{
  return _ptr_list_access_traits::iend<T>(list);
}
template <typename T>
static std::size_t size(Eina_List const* list)
{
  return eina_list_count(list);
}
template <typename T>
static bool empty(Eina_List const* list)
{
  return _ptr_list_access_traits::size<T>(list) == 0u;
}

};

template <typename T>
struct _const_range_ptr_list
{
  typedef _ptr_list_iterator<T const> const_iterator;
  typedef const_iterator iterator;
  typedef T value_type;
  typedef T& reference;
  typedef T const& const_reference;
  typedef T* pointer;
  typedef T const* const_pointer;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef const_reverse_iterator reverse_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef Eina_List const* native_handle_type;
  typedef _const_range_ptr_list<T> _self_type;

  _const_range_ptr_list(native_handle_type list)
    : _list(list) {}

  native_handle_type native_handle() const { return _list; }
  value_type const& back() const
  {
    return _ptr_list_access_traits::back<value_type>(_list);
  }
  value_type const& front() const
  {
    return _ptr_list_access_traits::front<value_type>(_list);
  }
  const_iterator begin() const
  {
    return _ptr_list_access_traits::begin<value_type>(_list);
  }
  const_iterator end() const
  {
    return _ptr_list_access_traits::end<value_type>(_list);
  }
  const_reverse_iterator rbegin() const
  {
    return _ptr_list_access_traits::rbegin<value_type>(_list);
  }
  const_reverse_iterator rend() const
  {
    return _ptr_list_access_traits::rend<value_type>(_list);
  }
  const_iterator cbegin() const
  {
    return _ptr_list_access_traits::cbegin<value_type>(_list);
  }
  const_iterator cend() const
  {
    return _ptr_list_access_traits::cend<value_type>(_list);
  }
  const_reverse_iterator crbegin() const
  {
    return _ptr_list_access_traits::crbegin<value_type>(_list);
  }
  const_reverse_iterator crend() const
  {
    return _ptr_list_access_traits::crend<value_type>(_list);
  }
  void swap(_self_type& other)
  {
    std::swap(_list, other._list);
  }
  bool empty() const
  {
    return _ptr_list_access_traits::empty<T>(_list);
  }
  size_type size() const
  {
    return _ptr_list_access_traits::size<T>(_list);
  }

  native_handle_type _list;
};

template <typename T>
void swap(_const_range_ptr_list<T>& lhs, _const_range_ptr_list<T>& rhs)
{
  lhs.swap(rhs);
}

template <typename T>
struct _mutable_range_ptr_list : _const_range_ptr_list<T>
{
  typedef _const_range_ptr_list<T> _base_type;
  typedef T value_type;
  typedef _ptr_list_iterator<T> iterator;
  typedef T& reference;
  typedef T* pointer;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef typename _base_type::const_iterator const_iterator;
  typedef typename _base_type::const_reference const_reference;
  typedef typename _base_type::const_pointer const_pointer;
  typedef typename _base_type::const_reverse_iterator const_reverse_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef Eina_List* native_handle_type;
  typedef _mutable_range_ptr_list<T> _self_type;

  _mutable_range_ptr_list(native_handle_type list)
    : _base_type(list) {}

  native_handle_type native_handle() const
  {
    return const_cast<native_handle_type>(_base_type::native_handle()); 
  }
  value_type& back() const
  {
    return _ptr_list_access_traits::back<value_type>(native_handle());
  }
  value_type& front() const
  {
    return _ptr_list_access_traits::front<value_type>(native_handle());
  }
  iterator begin() const
  {
    return _ptr_list_access_traits::begin<value_type>(native_handle());
  }
  iterator end() const
  {
    return _ptr_list_access_traits::end<value_type>(native_handle());
  }
  reverse_iterator rbegin() const
  {
    return _ptr_list_access_traits::rbegin<value_type>(native_handle());
  }
  reverse_iterator rend() const
  {
    return _ptr_list_access_traits::rend<value_type>(native_handle());
  }
};

template <typename T, typename Allocator>
struct ptr_list;

template <typename T>
struct range_ptr_list : _range_template<T, _ptr_list_access_traits>
{
  typedef _range_template<T, _ptr_list_access_traits> _base_type;
  typedef typename _base_type::value_type value_type;
  typedef typename _base_type::native_handle_type native_handle_type;

  range_ptr_list(native_handle_type list)
    : _base_type(list)
  {}
  template <typename Allocator>
  range_ptr_list(ptr_list<value_type, Allocator>& list)
    : _base_type(list.native_handle())
  {}
};

template <typename T, typename CloneAllocator>
struct _ptr_list_common_base
{
  typedef CloneAllocator clone_allocator_type;

  _ptr_list_common_base(CloneAllocator clone_allocator)
    : _impl(clone_allocator)
  {}
  _ptr_list_common_base(Eina_List* _list)
    : _impl(_list)
  {}
  _ptr_list_common_base() {}

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

  struct _ptr_list_impl : CloneAllocator
  {
    _ptr_list_impl() : _list(0) {}
    _ptr_list_impl(CloneAllocator allocator)
      : clone_allocator_type(allocator), _list(0) {}

    Eina_List* _list;
  };  

  _ptr_list_impl _impl;

private:
  _ptr_list_common_base(_ptr_list_common_base const& other);
  _ptr_list_common_base& operator=(_ptr_list_common_base const& other);
};

template <typename T, typename CloneAllocator = heap_no_copy_allocator>
class ptr_list : protected _ptr_list_common_base<T, CloneAllocator>
{
  typedef _ptr_list_common_base<T, CloneAllocator> _base_type;
public:
  typedef T value_type;
  typedef T& reference;
  typedef T const& const_reference;
  typedef _ptr_list_iterator<T const> const_iterator;
  typedef _ptr_list_iterator<T> iterator;
  typedef T* pointer;
  typedef T const* const_pointer;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef CloneAllocator clone_allocator_type;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  ptr_list() {}
  ptr_list(size_type n, const_reference t)
  {
    while(n--)
      push_back(t);
  }
  template <typename InputIterator>
  ptr_list(InputIterator i, InputIterator const& j
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
  ptr_list(ptr_list<T, CloneAllocator> const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  template <typename CloneAllocator1>
  ptr_list(ptr_list<T, CloneAllocator1>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  ~ptr_list()
  {
    clear();
  }
  ptr_list<T, CloneAllocator>& operator=(ptr_list<T, CloneAllocator>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }
  
  void clear()
  {
    for(iterator first = begin(), last = end(); first != last; ++first)
      this->_delete_clone(&*first);
    eina_list_free(this->_impl._list);
    this->_impl._list = 0;
  }
  std::size_t size() const
  {
    return _ptr_list_access_traits::size<T>(this->_impl._list);
  }
  bool empty() const
  {
    return _ptr_list_access_traits::empty<T>(this->_impl._list);
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
    Eina_List* new_list = eina_list_append(this->_impl._list, p.get());
    if(new_list)
    {
      this->_impl._list = new_list;
      p.release();
    }
    else
      throw std::bad_alloc();
  }
  void push_front(const_reference a)
  {
    push_front(this->new_clone(a));
  }
  void push_front(pointer p)
  {
    std::unique_ptr<value_type> p1(p);
    push_front(p1);
  }
  void push_front(std::unique_ptr<T>& p)
  {
    Eina_List* new_list = eina_list_prepend(this->_impl._list, p.get());
    if(new_list)
    {
      this->_impl._list = new_list;
      p.release();
    }
    else
      throw std::bad_alloc();
  }
  void pop_back()
  {
    this->_impl._list = eina_list_remove_list(this->_impl._list, eina_list_last(this->_impl._list));
  }
  void pop_front()
  {
    this->_impl._list = eina_list_remove_list(this->_impl._list, this->_impl._list);
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
    this->_impl._list = _eina_list_prepend_relative_list
      (this->_impl._list, p.get(), i.native_handle());
    if(this->_impl._list)
      p.release();
    else
      throw std::bad_alloc();
    return iterator(this->_impl._list
                    , i.native_handle()
                    ? ::eina_list_prev(i.native_handle())
                    : ::eina_list_last(this->_impl._list)
                    );
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
    iterator r = p;
    if(i != j)
      {
        r = insert(p, *i);
        ++i;
      }
    while(i != j)
      {
        insert(p, this->_new_clone(*i));
        ++i;
      }
    return r;
  }
  iterator erase(iterator q)
  {
    if(q.native_handle())
      {
        iterator r(this->_impl._list, eina_list_next(q.native_handle()));
        this->_impl._list = eina_list_remove_list(this->_impl._list, q.native_handle());
        return r;
      }
    else
      return q;
  }
  iterator erase(iterator i, iterator j)
  {
    while(i != j)
      i = erase(i);
    if(j.native_handle())
      return j;
    else
      return end();
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
    return _ptr_list_access_traits::back<T>(this->_impl._list);
  }
  value_type const& back() const
  {
    return _ptr_list_access_traits::back<T>(this->_impl._list);
  }
  value_type& front()
  {
    return _ptr_list_access_traits::front<T>(this->_impl._list);
  }
  value_type const& front() const
  {
    return _ptr_list_access_traits::front<T>(this->_impl._list);
  }
  const_iterator begin() const
  {
    return _ptr_list_access_traits::cbegin<T>(this->_impl._list);
  }
  const_iterator end() const
  {
    return _ptr_list_access_traits::cend<T>(this->_impl._list);
  }
  iterator begin()
  {
    return _ptr_list_access_traits::begin<T>(this->_impl._list);
  }
  iterator end()
  {
    return _ptr_list_access_traits::end<T>(this->_impl._list);
  }
  const_reverse_iterator rbegin() const
  {
    return _ptr_list_access_traits::rbegin<T>(this->_impl._list);
  }
  const_reverse_iterator rend() const
  {
    return _ptr_list_access_traits::rend<T>(this->_impl._list);
  }
  reverse_iterator rbegin()
  {
    return _ptr_list_access_traits::rbegin<T>(this->_impl._list);
  }
  reverse_iterator rend()
  {
    return _ptr_list_access_traits::rend<T>(this->_impl._list);
  }
  const_iterator cbegin() const
  {
    return _ptr_list_access_traits::cbegin<T>(this->_impl._list);
  }
  const_iterator cend() const
  {
    return _ptr_list_access_traits::cend<T>(this->_impl._list);
  }
  const_reverse_iterator crbegin() const
  {
    return _ptr_list_access_traits::crbegin<T>(this->_impl._list);
  }
  const_reverse_iterator crend() const
  {
    return _ptr_list_access_traits::crend<T>(this->_impl._list);
  }
  eina::iterator<T> ibegin()
  {
    return _ptr_list_access_traits::ibegin<T>(this->_impl._list);
  }
  eina::iterator<T> iend()
  {
    return _ptr_list_access_traits::iend<T>(this->_impl._list);
  }
  eina::iterator<T const> ibegin() const
  {
    return _ptr_list_access_traits::ibegin<T>(this->_impl._list);
  }
  eina::iterator<T const> iend() const
  {
    return _ptr_list_access_traits::iend<T>(this->_impl._list);
  }
  eina::iterator<T const> cibegin() const
  {
    return _ptr_list_access_traits::cibegin<T>(this->_impl._list);
  }
  eina::iterator<T const> ciend() const
  {
    return _ptr_list_access_traits::ciend<T>(this->_impl._list);
  }
  void swap(ptr_list<T, CloneAllocator>& other)
  {
    std::swap(this->_impl._list, other._impl._list);
  }
  size_type max_size() const { return -1; }

  Eina_List* native_handle()
  {
    return this->_impl._list;
  }
  Eina_List const* native_handle() const
  {
    return this->_impl._list;
  }
  eina::accessor<T const> accessor() const
  {
    return eina::accessor<T const>(eina_list_accessor_new(this->_impl._list));
  }
  eina::accessor<T> accessor()
  {
    return eina::accessor<T>(eina_list_accessor_new(this->_impl._list));
  }
};

template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator==(ptr_list<T, CloneAllocator1> const& lhs, ptr_list<T, CloneAllocator2> const& rhs)
{
  return lhs.size() == rhs.size()
    && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator!=(ptr_list<T, CloneAllocator1> const& lhs, ptr_list<T, CloneAllocator2> const& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename CloneAllocator>
void swap(ptr_list<T, CloneAllocator>& lhs, ptr_list<T, CloneAllocator>& rhs)
{
  lhs.swap(rhs);
}

} }

#endif
