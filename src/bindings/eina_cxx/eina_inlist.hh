#ifndef EINA_INLIST_HH_
#define EINA_INLIST_HH_

#include <Eina.h>
#include <eina_lists_auxiliary.hh>
#include <eina_type_traits.hh>
#include <eina_accessor.hh>

#include <iterator>

namespace efl { namespace eina {

template <typename T>
struct _inlist_node
{
  EINA_INLIST;
  T object;
};

template <typename T>
_inlist_node<T>* _get_node(Eina_Inlist* l)
{
  return static_cast<_inlist_node<T>*>(static_cast<void*>(l));
}

template <typename T>
_inlist_node<T> const* _get_node(Eina_Inlist const* l)
{
  return const_cast<Eina_Inlist*>(l);
}

template <typename T>
Eina_Inlist* _get_list(_inlist_node<T>* n)
{
  if(n)
    return EINA_INLIST_GET(n);
  else
    return 0;
}

template <typename T>
Eina_Inlist const* _get_list(_inlist_node<T> const* n)
{
  return _get_list(const_cast<_inlist_node<T>*>(n));
}

template <typename T>
struct _inlist_iterator
{
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef std::ptrdiff_t difference_type;
  typedef std::bidirectional_iterator_tag iterator_category;

  _inlist_iterator() {}
  explicit _inlist_iterator(_inlist_node<T>* list, _inlist_node<T>* node)
    : _list(list), _node(node) {}

  _inlist_iterator<T>& operator++()
  {
    _node = _get_node<T>(_node->__in_list.next);
    return *this;
  }
  _inlist_iterator<T> operator++(int)
  {
    _inlist_iterator<T> tmp(*this);
    ++*this;
    return tmp;
  }
  _inlist_iterator<T>& operator--()
  {
    if(_node)
      _node = _get_node<T>(_node->__in_list.prev);
    else
      _node = _get_node<T>(_list->__in_list.last);
    return *this;
  }
  _inlist_iterator<T> operator--(int)
  {
    _inlist_iterator<T> tmp(*this);
    --*this;
    return tmp;
  }
  T const& operator*() const
  {
    return _node->object;
  }
  T const* operator->() const
  {
    return &_node->object;
  }
  _inlist_node<T>* native_handle()
  {
    return _node;
  }
  _inlist_node<T> const* native_handle() const
  {
    return _node;
  }
private:
  _inlist_node<T>* _list;
  _inlist_node<T>* _node;

  friend bool operator==(_inlist_iterator<T> lhs, _inlist_iterator<T> rhs)
  {
    return lhs._node == rhs._node;
  }
};

template <typename T>
bool operator!=(_inlist_iterator<T> lhs, _inlist_iterator<T> rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Allocator>
struct _inlist_common_base
{
  typedef typename Allocator::template rebind<_inlist_node<T> >::other node_allocator_type;
  typedef Allocator allocator_type;
  typedef _inlist_node<T> node_type;

  _inlist_common_base(Allocator allocator)
    : _impl(allocator) {}
  _inlist_common_base()
  {}
  ~_inlist_common_base()
  {
    clear();
  }

  void clear()
  {
    Eina_Inlist* p = _impl._list;
    Eina_Inlist* q;
    while(p)
      {
        q = p->next;

        _inlist_node<T>* node = _get_node<T>(p);
        node->~_inlist_node<T>();
        get_node_allocator().deallocate(node, 1);

        p = q;
      }
    _impl._list = 0;
  }
  node_allocator_type& get_node_allocator()
  {
    return _impl;
  }

  // For EBO
  struct _inlist_impl : node_allocator_type
  {
    _inlist_impl(Allocator allocator)
      : node_allocator_type(allocator), _list(0)
    {}
    _inlist_impl() : _list(0) {}

    Eina_Inlist* _list;
  };

  _inlist_impl _impl;
private:
  _inlist_common_base(_inlist_common_base const& other);
  _inlist_common_base& operator=(_inlist_common_base const& other);
};

template <typename T, typename Allocator = std::allocator<T> >
class inlist : protected _inlist_common_base<T, Allocator>
{
  typedef _inlist_common_base<T, Allocator> _base_type;
  typedef typename _base_type::node_type _node_type;
public:
  typedef typename _base_type::allocator_type allocator_type;
  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef _inlist_iterator<T const> const_iterator;
  typedef _inlist_iterator<T> iterator;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  using _base_type::clear;

  inlist() {}
  inlist(size_type n, value_type const& t)
  {
    while(n--)
      push_back(t);
  }
  template <typename InputIterator>
  inlist(InputIterator i, InputIterator const& j
         , allocator_type const& alloc = allocator_type()
         , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
    : _base_type(alloc)
  {
    while(i != j)
      {
        push_back(*i);
        ++i;
      }
  }
  inlist(inlist<T, Allocator>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  inlist<T, Allocator>& operator=(inlist<T, Allocator>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }
  size_type size() const
  {
    return ::eina_inlist_count(this->_impl._list);
  }
  bool empty() const
  {
    return this->_impl._list == 0;
  }
  allocator_type get_allocator() const
  {
    return allocator_type(this->get_node_allocator());
  }
  void push_back(T const& value)
  {
    _node_type* node ( this->get_node_allocator().allocate(1) );
    try
    {
      new (&node->object) T(value);
      // eina_inlist_append can't fail
      this->_impl._list = eina_inlist_append(this->_impl._list, _get_list(node));
    }
    catch(...)
    {
      this->get_node_allocator().deallocate(node, 1);
      throw;
    }
  }
  void push_front(T const& value)
  {
    _node_type* node ( this->get_node_allocator().allocate(1) );
    try
    {
      new (&node->object) T(value);
      // eina_inlist_prepend can't fail
      this->_impl._list = eina_inlist_prepend(this->_impl._list, _get_list(node));
    }
    catch(...)
    {
      this->get_node_allocator().deallocate(node, 1);
      throw;
    }
  }
  void pop_back()
  {
    this->_impl._list = eina_inlist_remove(this->_impl._list, this->_impl._list->last);
  }
  void pop_front()
  {
    this->_impl._list = eina_inlist_remove(this->_impl._list, this->_impl._list);
  }
  iterator insert(iterator i, value_type const& t)
  {
    _node_type* node ( this->get_node_allocator().allocate(1) );
    try
    {
      new (&node->object) T(t);
      // eina_inlist_prepend_relative can't fail
      this->_impl._list = _eina_inlist_prepend_relative
        (this->_impl._list, _get_list(node)
         , _get_list(i.native_handle()));
      return iterator(_get_node<T>(this->_impl._list), node);
    }
    catch(...)
    {
      this->get_node_allocator().deallocate(node, 1);
      throw;
    }
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

  template <typename InputIterator>
  iterator insert(iterator p, InputIterator i, InputIterator j
                  , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    iterator r = p;
    if(i != j)
      {
        value_type v = *i;
        r = insert(p, v);
        ++i;
      }
    while(i != j)
      {
        insert(p, *i);
        ++i;
      }
    return r;
  }

  iterator erase(iterator q)
  {
    if(q.native_handle())
      {
        iterator r(_get_node<T>(this->_impl._list), _get_node<T>(_get_list(q.native_handle())->next));
        this->_impl._list = eina_inlist_remove(this->_impl._list, _get_list(q.native_handle()));
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
    return _get_node<T>(this->_impl._list->last)->object;
  }
  value_type const& back() const
  {
    return const_cast<inlist<T, Allocator>&>(*this).back();
  }
  value_type& front()
  {
    return _get_node<T>(this->_impl._list)->object;
  }
  value_type const& front() const
  {
    return const_cast<inlist<T, Allocator>&>(*this).front();
  }
  const_iterator begin() const
  {
    return const_iterator(_get_node<T const>(this->_impl._list), _get_node<T const>(this->_impl._list));
  }
  const_iterator end() const
  {
    return const_iterator(_get_node<T const>(this->_impl._list), 0);
  }
  iterator begin()
  {
    return iterator(_get_node<T>(this->_impl._list), _get_node<T>(this->_impl._list));
  }
  iterator end()
  {
    return iterator(_get_node<T>(this->_impl._list), 0);
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
  void swap(inlist<T, Allocator>& other)
  {
    std::swap(this->_impl._list, other._impl._list);
  }
  size_type max_size() const { return -1; }

  Eina_Inlist* native_handle()
  {
    return this->_impl._list;
  }
  Eina_Inlist const* native_handle() const
  {
    return this->_impl._list;
  }
  eina::accessor<T const> accessor() const
  {
    return eina::accessor<T const>(eina_inlist_accessor_new(this->_impl._list));
  }
  eina::accessor<T> accessor()
  {
    return eina::accessor<T>(eina_inlist_accessor_new(this->_impl._list));
  }
};

template <typename T, typename Allocator1, typename Allocator2>
bool operator==(inlist<T, Allocator1> const& lhs, inlist<T, Allocator2> const& rhs)
{
  return lhs.size() == rhs.size() &&
    std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Allocator1, typename Allocator2>
bool operator!=(inlist<T, Allocator1> const& lhs, inlist<T, Allocator2> const& rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename Allocator>
void swap(inlist<T, Allocator>& lhs, inlist<T, Allocator>& rhs)
{
  lhs.swap(rhs);
}

} }

#endif
