#ifndef EINA_ACCESSOR_HH_
#define EINA_ACCESSOR_HH_

#include <Eina.h>
#include <eina_error.hh>

#include <memory>
#include <iterator>
#include <cstdlib>
#include <cassert>

/**
 * @addtogroup Eina_Cxx_Content_Access_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Accessor_Group Accessor
 * @ingroup Eina_Cxx_Content_Access_Group
 *
 * @brief These classes manage accessor on containers.
 *
 * These classes allow to access elements of a container in a
 * generic way, without knowing which container is used (like
 * iterators in the C++ STL). Accessors allows random access (that is, any
 * element in the container). For sequential access, see
 * @ref Eina_Cxx_Iterator_Group.
 *
 * @{
 */

/**
 * Wraps an native Eina_Accessor and provide random access to data structures.
 */
template <typename T>
struct accessor
{
  typedef unsigned int key_type; /**< Type for accessor key. */
  typedef T mapped_type; /**< Type for accessor mapped elements. */
  typedef T value_type; /**< Type for accessor elements. Same as @ref mapped_type. */
  typedef std::size_t size_type; /**< Type for size information used in the accessor. */

  /**
   * @brief Default constructor. Creates an empty accessor.
   */
  accessor() : _impl(0) {}

  /**
   * @brief Create an accessor object that wraps the given Eina accessor.
   * @param impl Native @c Eina_Accessor to be wrapped.
   *
   * This constructor creates an accessor object that wraps the given
   * Eina_Accessor and provides access to the data pointed by it.
   *
   * @warning It is important to note that the created accessor object
   * gains ownership of the given handle, deallocating it at destruction
   * time.
   */
  explicit accessor(Eina_Accessor* impl)
    : _impl(impl)
  {
    assert(_impl != 0);
  }

  /**
   * @brief Copy constructor. Creates a copy of the given accessor object.
   * @param other Other accessor object.
   *
   * This constructor clones the internal @c Eina_Accessor of the given
   * accessor object, so that the newly created object can be used
   * freely.
   */
  accessor(accessor<T> const& other)
    : _impl(eina_accessor_clone(other._impl))
  {}

  /**
   * @brief Assignment Operator. Replace the current content.
   * @param other Other accessor object.
   * @throw <tt>eina::system_error</tt> if the Eina accessor could not be cloned.
   *
   * This operator replaces the current native Eina accessor by a copy
   * of the native accessor inside the given object.
   */
  accessor<T>& operator=(accessor<T> const& other)
  {
    eina_accessor_free(_impl);
    _impl = eina_accessor_clone(other._impl);
    if(!_impl)
       EFL_CXX_THROW(eina::system_error(efl::eina::get_error_code(), "Error cloning accessor"));
    return *this;
  }

  /**
   * @brief Destructor. Free the internal @c Eina_Acessor.
   */
  ~accessor()
  {
    eina_accessor_free(_impl);
  }

  /**
   * @brief Retrieve the data of the accessor at a given position.
   * @param i The position of the element.
   * @return Constant reference to the retrieved data.
   * @throw <tt>eina::system_error</tt> if the given element could not be retrieved.
   *
   * This operator retrieves a constant reference to the element at the
   * given position. If the element could not be retrieved an
   * <tt>eina::system_error</tt> is thrown.
   */
  mapped_type& operator[](size_type i) const
  {
    assert(_impl != 0);
    void* p;
    if(!eina_accessor_data_get(_impl, i, &p))
      {
        eina::error_code ec = efl::eina::get_error_code();
        EFL_CXX_THROW(eina::system_error(ec, "EFL Eina Error"));
      }
    return *static_cast<mapped_type*>(p);
  }

  /**
   * @brief Get the handle for the wrapped @c Eina_Accessor.
   * @return Internal handle for the native Eina accessor.
   *
   * This member function returns the native @c Eina_Accessor handle
   * that is wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  Eina_Accessor* native_handle() const;

  /**
   * @brief Swap content between both objects.
   * @param other Other accessor object.
   *
   * This member function swaps the internal @c Eina_Acessor with the
   * given accessor object.
   */
  void swap(accessor<T>& other)
  {
    std::swap(_impl, other._impl);
  }
  /**
   * @brief Cast to @c boolean based on the wrapped @c Eina_Accessor.
   * @return @c true if the wrapped handle is not @c NULL, @c false otherwise.
   *
   * Boolean typecast overload for easy validation of the accessor
   * object. Returns @c false if it does not have an internal
   * @c Eina_Accessor, i.e. if the current handle is not @c NULL.
   */
  explicit operator bool() const
  {
    return native_handle() ? &accessor<T>::native_handle : 0 ;
  }
private:

  /**
   * @internal
   * Member variable for storing the native Eina_Accessor pointer.
   */
  Eina_Accessor* _impl;
};

/**
 * @brief Swap the contents of the two accessor objects.
 * @param lhs First accessor object.
 * @param rhs Second accessor object.
 */
template <typename U>
void swap(accessor<U>& lhs, accessor<U>& rhs)
{
  lhs.swap(rhs);
}

/**
 * @}
 */

/**
 * @defgroup Eina_Cxx_Accessor_Iterator_Group Accessor Iterator
 * @ingroup Eina_Cxx_Content_Access_Group
 *
 * @{
 */

/**
 * Random access iterator for <tt>eina::accessor</tt>.
 */
template <typename T>
struct accessor_iterator
{
  typedef T value_type; /**< Type of the elements. */
  typedef value_type* pointer; /**< Pointer to element type. */
  typedef value_type& reference; /**< Reference to element type. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two @ref accessor_iterators */
  typedef std::random_access_iterator_tag iterator_category; /**< Defines the iterator as being a random access iterator. */


  /**
   * @brief Creates an @c accessor_iterator to the given <tt>eina::accessor</tt>.
   * @param a <tt>eina::accessor</tt> object.
   * @param pos Initial position of the iterator (Default = @c 0).
   *
   * This constructor creates an @c accessor_iterator for the given
   * <tt>eina::accessor</tt> object. The position initially pointed by
   * the iterator can be supplied via the @p pos argument, by default
   * it is the first position (index @c 0).
   */
  accessor_iterator(accessor<T> const& a, unsigned int pos = 0u)
    : _accessor(a), _index(pos)
  {}

  /**
   * @brief Move the iterator forward by @p i positions.
   * @param i Number of positions to move.
   * @return The @c accessor_iterator itself.
   */
  accessor_iterator<T>& operator+=(difference_type i)
  {
    _index += i;
    return *this;
  }

  /**
   * @brief Move the iterator back by @p i positions.
   * @param i Number of positions to move.
   * @return The @c accessor_iterator itself.
   */
  accessor_iterator<T>& operator-=(difference_type i)
  {
    _index -= i;
    return *this;
  }

  /**
   * @brief Get the element @p i positions away from the current element.
   * @param i Position relative to the current element.
   * @return Reference to the element @p i positions away from the
   *         element currently pointed by the @c accessor_iterator.
   */
  value_type& operator[](difference_type i)
  {
    return _accessor[_index + i];
  }

  /**
   * @brief Move the iterator to the next position.
   * @return The @c accessor_iterator itself.
   *
   * This operator increments the iterator, making it point to the
   * position right after the current one.
   * At the end, it returns a reference to itself.
   */
  accessor_iterator<T>& operator++()
  {
    ++_index;
    return *this;
  }

  /**
   * @brief Move the iterator to the previous position.
   * @return The @c accessor_iterator itself.
   *
   * This operator decrements the iterator, making it point to the
   * position right before the current one.
   * At the end, it returns a reference to itself.
   */
  accessor_iterator<T>& operator--()
  {
    --_index;
    return *this;
  }

  /**
   * @brief Move the iterator to the next position.
   * @return A copy of the @c accessor_iterator before the change.
   *
   * This operator increments the iterator, making it point to the
   * position right after the current one.
   * At the end, it returns a copy of the @c accessor_iterator before
   * the change.
   */
  accessor_iterator<T>& operator++(int)
  {
    accessor_iterator<T> tmp(*this);
    ++*this;
    return tmp;
  }

  /**
   * @brief Move the iterator to the previous position.
   * @return A copy of the @c accessor_iterator before the change.
   *
   * This operator decrements the iterator, making it point to the
   * position right before the current one.
   * At the end, it returns a copy of the @c accessor_iterator before
   * the change.
   */
  accessor_iterator<T>& operator--(int)
  {
    accessor_iterator<T> tmp(*this);
    --*this;
    return tmp;
  }

  /**
   * @brief Get a reference to the element currently pointed by the @c accessor_iterator.
   * @return Reference to the current element.
   */
  value_type& operator*() const
  {
    return _accessor[_index];
  }

  /**
   * @brief Return a pointer to the current element, which member will be accessed.
   * @return Pointer to the element currently pointed by the @c accessor_iterator.
   */
  pointer operator->() const
  {
    return &**this;
  }

  /**
   * @brief Swap content with the given @c accessor_iterator.
   * @param other Another @c accessor_iterator of the same type.
   */
  void swap(accessor_iterator<T>& other)
  {
    std::swap(_index, other._index);
    std::swap(_accessor, other._accessor);
  }

private:
  accessor<T> _accessor; /**< @internal  */
  unsigned int _index; /**< @internal */

  /**
   * @brief Check if @p lhs and @p rhs point to the same position.
   * @param lhs @c accessor_iterator at the left side of the expression.
   * @param rhs @c accessor_iterator at the right side of the expression.
   * @return @c true if both @p lhs and @p rhs point to the same position.
   */
  template <typename U>
  friend bool operator==(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index == rhs._index;
  }

  /**
   * @brief Get the distance between two <tt>accessor_iterator</tt>s.
   * @param lhs @c accessor_iterator at the left side of the expression.
   * @param rhs @c accessor_iterator at the right side of the expression.
   * @return The number of elements between @p lhs and @p rhs.
   */
  template <typename U>
  friend typename accessor_iterator<U>::difference_type
  operator-(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index - rhs._index;
  }

  /**
   * @brief Get an @c accessor_iterator moved @p rhs positions forward.
   * @param lhs @c accessor_iterator object.
   * @param rhs Number of positions relative to the current element.
   * @return Copy of @p lhs moved @p rhs positions forward.
   */
  template <typename U>
  friend
  accessor_iterator<U> operator+(accessor_iterator<U> lhs
                                 , typename accessor_iterator<U>::difference_type rhs)
  {
    lhs._index += rhs;
    return lhs;
  }

  /**
   * @brief Get an @c accessor_iterator moved @p lhs positions forward.
   * @param lhs Number of positions relative to the current element.
   * @param rhs @c accessor_iterator object.
   * @return Copy of @p rhs moved @p lhs positions forward.
   */
  template <typename U>
  friend
  accessor_iterator<U> operator+(typename accessor_iterator<U>::difference_type lhs
                               , accessor_iterator<U> rhs)
  {
    return rhs + lhs;
  }

  /**
   * @brief Check if @p lhs points to a position before the position pointed by @p rhs.
   * @param lhs @c accessor_iterator at the left side of the expression.
   * @param rhs @c accessor_iterator at the right side of the expression.
   * @return @c true if @p lhs points to a position before the position
   *         pointed by @p rhs, @c false otherwise.
   */
  template <typename U>
  friend bool operator<(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index < rhs._index;
  }

  /**
   * @brief Check if the position pointed by @p lhs is the same or is before the one pointed by @p rhs.
   * @param lhs @c accessor_iterator at the left side of the expression.
   * @param rhs @c accessor_iterator at the right side of the expression.
   * @return @c true if the position pointed by @p lhs is the same or is
   *            before the position pointed by @p rhs, @c false otherwise.
   */
  template <typename U>
  friend bool operator<=(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
  {
    return lhs._index <= rhs._index;
  }
};

/**
 * @brief Check if the position pointed by @p lhs is the same or is after the one pointed by @p rhs.
 * @param lhs @c accessor_iterator at the left side of the expression.
 * @param rhs @c accessor_iterator at the right side of the expression.
 * @return @c true if the position pointed by @p lhs is the same or is
 *            after the position pointed by @p rhs, @c false otherwise.
 */
template <typename U>
bool operator>=(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
{
  return !(lhs < rhs);
}

/**
 * @brief Check if @p lhs points to a position after the position pointed by @p rhs.
 * @param lhs @c accessor_iterator at the left side of the expression.
 * @param rhs @c accessor_iterator at the right side of the expression.
 * @return @c true if @p lhs points to a position after the position
 *         pointed by @p rhs, @c false otherwise.
 */
template <typename U>
bool operator>(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
{
  return !(lhs <= rhs);
}

/**
 * @brief Check if @p lhs and @p rhs point to different positions.
 * @param lhs @c accessor_iterator at the left side of the expression.
 * @param rhs @c accessor_iterator at the right side of the expression.
 * @return @c true if @p lhs and @p rhs point to different positions.
 */
template <typename U>
bool operator!=(accessor_iterator<U> const& lhs, accessor_iterator<U> const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Swap content between two <tt>accessor_iterator</tt>s.
 * @param lhs First @c accessor_iterator.
 * @param rhs Second @c accessor_iterator.
 */
template <typename U>
void swap(accessor_iterator<U>& lhs, accessor_iterator<U>& rhs)
{
  lhs.swap(rhs);
}

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
