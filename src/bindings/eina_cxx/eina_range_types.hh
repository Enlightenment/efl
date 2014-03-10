#ifndef EINA_RANGE_TYPES_HH_
#define EINA_RANGE_TYPES_HH_

namespace efl { namespace eina {

template <typename T, typename Traits>
struct _const_range_template
{
  typedef typename Traits::template const_iterator<T>::type const_iterator;
  typedef typename Traits::template iterator<T>::type iterator;
  typedef T value_type;
  typedef T& reference;
  typedef T const& const_reference;
  typedef T* pointer;
  typedef T const* const_pointer;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef typename Traits::template const_native_handle<T>::type native_handle_type;
  typedef _const_range_template<T, Traits> _self_type;

  _const_range_template(native_handle_type handle)
    : _handle(handle) {}

  native_handle_type native_handle() const { return _handle; }
  value_type const& back() const
  {
    return Traits::template back<value_type>(_handle);
  }
  value_type const& front() const
  {
    return Traits::template front<value_type>(_handle);
  }
  const_iterator begin() const
  {
    return cbegin();
  }
  const_iterator end() const
  {
    return cend();
  }
  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator(Traits::template begin<value_type>(_handle));
  }
  const_reverse_iterator crend() const
  {
    return const_reverse_iterator(Traits::template rend<value_type>(_handle));
  }
  const_iterator cbegin() const
  {
    return Traits::template cbegin<value_type>(_handle);
  }
  const_iterator cend() const
  {
    return Traits::template cend<value_type>(_handle);
  }
  const_reverse_iterator rbegin()
  {
    return crbegin();
  }
  const_reverse_iterator rend()
  {
    return crend();
  }
  bool empty() const
  {
    return Traits::template empty<value_type>(_handle);
  }
  size_type size() const
  {
    return Traits::template size<value_type>(_handle);
  }
  void swap(_self_type& other)
  {
    std::swap(_handle, other._handle);
  }
protected:
  native_handle_type _handle;  
};

template <typename T, typename Traits>
void swap(_const_range_template<T, Traits>& lhs, _const_range_template<T, Traits>& rhs)
{
  lhs.swap(rhs);
}

template <typename T, typename Traits>
struct _mutable_range_template : _const_range_template<T, Traits>
{
  typedef T value_type;
  typedef typename Traits::template iterator<T>::type iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef typename Traits::template native_handle<T>::type native_handle_type;
  typedef _const_range_template<T, Traits> _base_type;

  _mutable_range_template(native_handle_type handle)
    : _base_type(handle) {}

  native_handle_type native_handle() const
  {
    return Traits::template native_handle_from_const<T>(_base_type::native_handle());
  }
  value_type& back() const
  {
    return Traits::template back<value_type>(native_handle());
  }
  value_type& front() const
  {
    return Traits::template front<value_type>(native_handle());
  }
  iterator begin() const
  {
    return Traits::template begin<value_type>(native_handle());
  }
  iterator end() const
  {
    return Traits::template end<value_type>(native_handle());
  }
  reverse_iterator rbegin() const
  {
    return Traits::template rbegin<value_type>(native_handle());
  }
  reverse_iterator rend() const
  {
    return Traits::template rend<value_type>(native_handle());
  }
protected:
  using _base_type::_handle;
};

template <typename T, typename Traits>
struct _range_template : private std::conditional
  <std::is_const<T>::value
   , _const_range_template<typename std::remove_const<T>::type, Traits>
   , _mutable_range_template<T, Traits> >::type
{
  typedef std::integral_constant<bool, !std::is_const<T>::value> is_mutable;
  typedef typename std::remove_const<T>::type value_type;
  typedef typename std::conditional<is_mutable::value, _mutable_range_template<value_type, Traits>
                                    , _const_range_template<value_type, Traits> >::type _base_type;
  typedef typename Traits::template native_handle<T>::type native_handle_type;

  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef typename Traits::template const_iterator<T>::type const_iterator;
  typedef typename _base_type::const_reverse_iterator const_reverse_iterator;
  typedef typename Traits::template iterator<T>::type iterator;
  typedef typename _base_type::reverse_iterator reverse_iterator;
  typedef typename _base_type::size_type size_type;
  typedef typename _base_type::difference_type difference_type;

  _range_template(native_handle_type handle)
    : _base_type(handle)
  {}

  using _base_type::native_handle;
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
  using _base_type::empty;
  using _base_type::size;
protected:
  using _base_type::_handle;
};

} }

#endif
