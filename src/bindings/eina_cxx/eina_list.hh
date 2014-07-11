#ifndef EINA_LIST_HH_
#define EINA_LIST_HH_

#include <Eo.h>

#include <eina_ptrlist.hh>

namespace efl { namespace eina {

/**
 * @internal
 */
template <typename T>
struct _ptr_eo_list_iterator : _ptr_list_iterator<Eo>
{
  typedef _ptr_list_iterator<Eo> _base_type;
  typedef _ptr_eo_list_iterator<T> _self_type;
  typedef typename remove_cv<T>::type value_type;
  typedef value_type* pointer;
  typedef value_type& reference;

  _base_type& _base() { return *this; }
  _base_type const& _base() const { return *this; }
  
  _ptr_eo_list_iterator(_base_type const& base) : _base_type(base) {}
  _ptr_eo_list_iterator() {}
  explicit _ptr_eo_list_iterator(Eina_List* list, Eina_List* node)
    : _base_type(list, node)
  {
  }
  _ptr_eo_list_iterator(_ptr_eo_list_iterator<value_type> const& other)
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
    void* data = reinterpret_cast<void*>(&this->_node->data);
    return *static_cast<pointer>(data);
  }
  pointer operator->() const
  {
    return &**this;
  }

  using _base_type::native_handle;
};

template <typename T, typename CloneAllocator = default_clone_allocator_placeholder, typename Enable = void>
class list : ptr_list<T, typename std::conditional
                      <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
                       , heap_no_copy_allocator, CloneAllocator>::type>
{
  typedef ptr_list<T, typename std::conditional
                   <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
                    , heap_no_copy_allocator, CloneAllocator>::type> _base_type;
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
  typedef typename _base_type::clone_allocator_type clone_allocator_type;

  typedef typename _base_type::reverse_iterator reverse_iterator;
  typedef typename _base_type::const_reverse_iterator const_reverse_iterator;

  using _base_type::_base_type;
  using _base_type::clear;
  using _base_type::size;
  using _base_type::empty;
  using _base_type::get_clone_allocator;
  using _base_type::push_back;
  using _base_type::push_front;
  using _base_type::pop_back;
  using _base_type::pop_front;
  using _base_type::insert;
  using _base_type::erase;
  using _base_type::assign;
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
  using _base_type::ibegin;
  using _base_type::iend;
  using _base_type::cibegin;
  using _base_type::ciend;
  using _base_type::swap;
  using _base_type::max_size;
  using _base_type::native_handle;
  using _base_type::accessor;
};

template <typename T, typename CloneAllocator>
class list<T, CloneAllocator, typename std::enable_if<std::is_base_of<efl::eo::base, T>::value>::type>
  : ptr_list<Eo, typename std::conditional
             <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
              , eo_clone_allocator, CloneAllocator>::type>
{
  typedef ptr_list<Eo, typename std::conditional
                   <std::is_same<CloneAllocator, default_clone_allocator_placeholder>::value
                    , eo_clone_allocator, CloneAllocator>::type> _base_type;
  typedef list<T, CloneAllocator> _self_type;
public:
  typedef T value_type;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef _ptr_eo_list_iterator<value_type const> const_iterator;
  typedef _ptr_eo_list_iterator<value_type> iterator;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef typename _base_type::clone_allocator_type clone_allocator_type;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  explicit list(Eina_List* handle)
    : _base_type(handle) {}
  list(clone_allocator_type alloc) : _base_type(alloc) {}
  list() {}
  list(size_type n, const_reference t)
  {
    while(n--)
      push_back(t);
  }
  template <typename InputIterator>
  list(InputIterator i, InputIterator const& j
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
  list(list<T, CloneAllocator> const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }
  ~list()
  {
  }
  list<T, CloneAllocator>& operator=(list<T, CloneAllocator>const& other)
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
  using _base_type::pop_front;

  void push_back(const_reference w)
  {
    this->_base_type::push_back(* ::eo_ref(w._eo_ptr()));
  }
  void push_front(const_reference w)
  {
    this->_base_type::push_front(* ::eo_ref(w._eo_ptr()));
  }
  iterator insert(iterator i, const_reference v)
  {
    return this->_base_type::insert(i, * ::eo_ref(v._eo_ptr()));
  }
  iterator insert(iterator i, size_t n, const_reference v)
  {
    return this->_base_type::insert(i, n, * ::eo_ref(v._eo_ptr()));
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
        insert(p, *i);
        ++i;
      }
    return r;
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
    return *static_cast<pointer>(static_cast<void*>(&native_handle()->data));
  }
  reference back()
  {
    return *static_cast<pointer>(static_cast<void*>(&eina_list_last(native_handle())->data));
  }
  const_reference front() const { return const_cast<_self_type*>(this)->front(); }
  const_reference back() const { return const_cast<_self_type*>(this)->back(); }
  iterator begin()
  {
    return iterator(native_handle(), native_handle());
  }
  iterator end()
  {
    return iterator(native_handle(), nullptr);
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
    return reverse_iterator(end());
  }
  reverse_iterator rend()
  {
    return reverse_iterator(begin());
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
  using _base_type::max_size;
  using _base_type::native_handle;

  friend bool operator==(list<T, CloneAllocator> const& rhs, list<T, CloneAllocator> const& lhs)
  {
    return rhs.size() == lhs.size() && std::equal(rhs.begin(), rhs.end(), lhs.begin());
  }
};

template <typename T, typename CloneAllocator>
bool operator!=(list<T, CloneAllocator> const& rhs, list<T, CloneAllocator> const& lhs)
{
  return !(rhs == lhs);
}
    
} }

#endif
