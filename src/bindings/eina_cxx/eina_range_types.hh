#ifndef EINA_RANGE_TYPES_HH_
#define EINA_RANGE_TYPES_HH_

/**
 * @addtogroup Eina_Cxx_Content_Access_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Range_Group Range
 * @ingroup Eina_Cxx_Content_Access_Group
 *
 * @{
 */

/**
 * @brief Range implementation for immutable collections.
 */
template <typename T, typename Traits>
struct _const_range_template
{
  typedef typename Traits::template const_iterator<T>::type const_iterator; /**< Type for constant iterator to the range. */
  typedef typename Traits::template iterator<T>::type iterator; /**< Type for iterator to the range. */
  typedef T value_type; /**< The type of each element. */
  typedef T& reference; /**< Type for a reference to an element. */
  typedef T const& const_reference; /**< Type for a constant reference to an element. */
  typedef T* pointer; /**< Type for a pointer to an element. */
  typedef T const* const_pointer; /**< Type for a constant pointer to an element. */
  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for reverse iterator to the range. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /**< Type for constant reverse iterator to the range. */
  typedef std::size_t size_type; /**< Type for size information. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two iterators. */

  typedef typename Traits::template const_native_handle<T>::type native_handle_type; /**< Type for the native handle of the container. */
  typedef _const_range_template<T, Traits> _self_type; /**< Type of the range itself.  */

  /**
   * @brief Creates a range object wrapping the given native container handle.
   */
  _const_range_template(native_handle_type handle)
    : _handle(handle) {}

  /**
   * @brief Get a constant handle for the native Eina container.
   * @return Constant handle for the native Eina container.
   */
  native_handle_type native_handle() const { return _handle; }

  /**
   * @brief Get a constant reference to the last element.
   * @return Constant reference to the last element of the range.
   */
  value_type const& back() const
  {
    return Traits::template back<value_type>(_handle);
  }

  /**
   * @brief Get a constant reference to the first element.
   * @return Constant reference to the first element of the range.
   */
  value_type const& front() const
  {
    return Traits::template front<value_type>(_handle);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the range.
   * @return Constant iterator to the initial position of the range.
   *
   * This member function returns a constant iterator pointing to the
   * first element of the range. If the range contains no elements the
   * returned iterator is the same as the one returned by @ref end() const.
   */
  const_iterator begin() const
  {
    return cbegin();
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the range.
   * @return Constant iterator to the final position of the range.
   *
   * This member function returns a constant iterator to the position
   * following the last element in the range. If the range contains no
   * elements the returned iterator is the same as the one returned by
   * @ref begin() const.
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  const_iterator end() const
  {
    return cend();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the range.
   * @return Constant reverse iterator pointing to the reverse begin of the range.
   *
   * This member function works like @ref rbegin() const but is granted
   * to return a constant reverse iterator even for a range to a mutable
   * collection.
   */
  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator(Traits::template rbegin<value_type>(_handle));
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the range.
   * @return Constant reverse iterator pointing to the reverse end of the range.
   *
   * This member function works like @ref rend() const but is granted to
   * return a constant reverse iterator even for range to a mutable
   * collection.
   */
  const_reverse_iterator crend() const
  {
    return const_reverse_iterator(Traits::template rend<value_type>(_handle));
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the range.
   * @return Constant iterator to the initial position of the range.
   *
   * This member function works like @ref begin() const but is granted
   * to return a constant iterator even for a range to a mutable
   * collection.
   */
  const_iterator cbegin() const
  {
    return Traits::template cbegin<value_type>(_handle);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the range.
   * @return Constant iterator to the final position of the range.
   *
   * This member function works like @ref end() const  but is granted to
   * return a constant iterator even for a range to a mutable collection.
   */
  const_iterator cend() const
  {
    return Traits::template cend<value_type>(_handle);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the range.
   * @return Constant reverse iterator pointing to the reverse begin of the range.
   *
   * This member function returns a constant reverse iterator pointing
   * to the last element of the range. If the range is empty the
   * returned reverse iterator is the same as the one returned by
   * @ref rend().
   */
  const_reverse_iterator rbegin()
  {
    return crbegin();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the range.
   * @return Constant reverse iterator pointing to the reverse end of the range.
   *
   * This member function returns a constant reverse iterator pointing
   * to the position before the first element of the range. If the range
   * is empty the returned iterator is the same as the one returned by
   * @ref rbegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  const_reverse_iterator rend()
  {
    return crend();
  }

  /**
   * @brief Check if the range does not contain any elements.
   * @return @c true if there is no elements in the range, @c false otherwise.
   *
   * This member function returns @c true if the range does not contain
   * any elements, otherwise it returns @c false.
   */
  bool empty() const
  {
    return Traits::template empty<value_type>(_handle);
  }

  /**
   * @brief Get the number of elements in the range.
   * @return Number of elements in the range.
   *
   * This member function returns the current number of elements in the
   * range.
   */
  size_type size() const
  {
    return Traits::template size<value_type>(_handle);
  }

  /**
   * @brief Swap content with another range of the same type.
   * @param other Another range of the same type.
   */
  void swap(_self_type& other)
  {
    std::swap(_handle, other._handle);
  }
protected:
  /**
   * @internal
   */
  native_handle_type _handle;
};

/**
 * @brief Swap content between two @ref _const_range_template.
 * @param lhs First @c _const_range_template object.
 * @param rhs Second @c _const_range_template object.
 */
template <typename T, typename Traits>
void swap(_const_range_template<T, Traits>& lhs, _const_range_template<T, Traits>& rhs)
{
  lhs.swap(rhs);
}

/**
 * @brief Range implementation for mutable collections.
 */
template <typename T, typename Traits>
struct _mutable_range_template : _const_range_template<T, Traits>
{
  typedef T value_type; /**< The type of each element. */
  typedef typename Traits::template iterator<T>::type iterator; /**< Type for a iterator to the range. */
  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for constant reverse iterator to the range. */
  typedef typename Traits::template native_handle<T>::type native_handle_type; /**< Type for the native handle of the container. */
  typedef _const_range_template<T, Traits> _base_type;  /**< Type for the base class. */

  /**
   * @brief Creates a range object wrapping the given native container handle.
   */
  _mutable_range_template(native_handle_type handle)
    : _base_type(handle) {}

  /**
   * @brief Get a constant handle for the native Eina container.
   * @return Constant handle for the native Eina container.
   */
  native_handle_type native_handle() const
  {
    return Traits::template native_handle_from_const<T>(_base_type::native_handle());
  }

  /**
   * @brief Get a reference to the last element.
   * @return Reference to the last element of the range.
   */
  value_type& back() const
  {
    return Traits::template back<value_type>(native_handle());
  }

  /**
   * @brief Get a reference to the first element.
   * @return Reference to the first element of the range.
   */
  value_type& front() const
  {
    return Traits::template front<value_type>(native_handle());
  }

  /**
   * @brief Get an iterator pointing to the first element of the range.
   * @return Iterator to the initial position of the range.
   *
   * This member function returns an iterator pointing to the first
   * element of the range. If the range contains no elements the
   * returned iterator is the same as the one returned by @ref end() const.
   */
  iterator begin() const
  {
    return Traits::template begin<value_type>(native_handle());
  }

  /**
   * @brief Get an iterator to the position following the last element of the range.
   * @return Iterator to the final position of the range.
   *
   * This member function returns an iterator to the position following
   * the last element in the range. If the range contains no elements
   * the returned iterator is the same as the one returned by
   * @ref begin() const.
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  iterator end() const
  {
    return Traits::template end<value_type>(native_handle());
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse begin of the range.
   * @return Reverse iterator pointing to the reverse begin of the range.
   *
   * This member function returns a reverse iterator pointing to the
   * last element of the range. If the range is empty the returned
   * reverse iterator is the same as the one returned by @ref rend().
   */
  reverse_iterator rbegin() const
  {
    return Traits::template rbegin<value_type>(native_handle());
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse end of the range.
   * @return Reverse iterator pointing to the reverse end of the range.
   *
   * This member function returns a reverse iterator pointing to the
   * position before the first element of the range. If the range is
   * empty the returned iterator is the same as the one returned by
   * @ref rbegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  reverse_iterator rend() const
  {
    return Traits::template rend<value_type>(native_handle());
  }
protected:
  /**
   * @internal
   */
  using _base_type::_handle;
};

/**
 * Range class.
 *
 * Provide objects for accessing and/or modifying elements inside a
 * container without modifying the container itself.
 */
template <typename T, typename Traits>
struct _range_template : private std::conditional
  <std::is_const<T>::value
   , _const_range_template<typename std::remove_const<T>::type, Traits>
   , _mutable_range_template<T, Traits> >::type
{
  typedef std::integral_constant<bool, !std::is_const<T>::value> is_mutable; /**< Type that specifies if the elements can be modified. */
  typedef typename std::remove_const<T>::type value_type; /**< The type of each element. */
  typedef typename std::conditional<is_mutable::value, _mutable_range_template<value_type, Traits>
                                    , _const_range_template<value_type, Traits> >::type _base_type; /**< Type for the base class. */
  typedef typename _base_type::native_handle_type native_handle_type; /**< Type for the native handle of the container. */

  typedef value_type& reference; /**< Type for a reference to an element. */
  typedef value_type const& const_reference; /**< Type for a constant reference to an element. */
  typedef value_type* pointer; /**< Type for a pointer to an element. */
  typedef value_type const* const_pointer; /**< Type for a constant pointer to an element. */
  typedef typename Traits::template const_iterator<T>::type const_iterator; /**< Type for constant iterator to the range. */
  typedef typename _base_type::const_reverse_iterator const_reverse_iterator; /**< Type for constant reverse iterator to the range. */
  typedef typename Traits::template iterator<T>::type iterator; /**< Type for iterator to the range. */
  typedef typename _base_type::reverse_iterator reverse_iterator; /**< Type for reverse iterator to the range. */
  typedef typename _base_type::size_type size_type; /**< Type for size information. */
  typedef typename _base_type::difference_type difference_type; /**< Type to represent the distance between two iterators. */

  /**
   * @brief Creates a range object wrapping the given native container handle.
   */
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
  using _base_type::swap;
protected:
  using _base_type::_handle;
};

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
