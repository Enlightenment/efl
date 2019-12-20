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
#ifndef EINA_ITERATOR_HH_
#define EINA_ITERATOR_HH_

#include <Eina.h>

#include <eina_eo_concrete_fwd.hh>

#include <cstdlib>
#include <iterator>

/**
 * @addtogroup Eina_Cxx_Content_Access_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Iterator_Group Iterator
 * @ingroup Eina_Cxx_Content_Access_Group
 *
 * @{
 */

/**
 * Common implementations for iterators.
 */
template <typename T>
struct _common_iterator_base
{
private:
  typedef _common_iterator_base<T> self_type; /**< Type for the iterator instantiation itself. */
public:
  typedef typename std::remove_reference<T>::type const value_type; /**< Type for elements returned by the iterator. */
  typedef value_type* pointer; /**< Type for a pointer to an element. */
  typedef value_type& reference; /**< Type for a reference to an element. */
  typedef std::ptrdiff_t difference_type;  /**< Type to represent the distance between two iterators. */
  typedef std::input_iterator_tag iterator_category; /**< Defines the iterator as being an input iterator. */

  /**
   * @brief Default constructor. Creates an iterator that points to nothing.
   */
  _common_iterator_base() {}

  /**
   * @brief Creates a iterator wrapping the given native @c Eina_Iterator handle.
   * @param iterator Handle to a native @c Eina_Iterator.
   *
   * This constructor creates an iterator that wraps the given native
   * @c Eina_Iterator handle, providing a generic interface to it.
   *
   * @warning The created iterator object gains ownership of the handle
   * and will deallocate it at destruction time.
   */
  explicit _common_iterator_base(Eina_Iterator* iterator)
    : _iterator(iterator) {}

  /**
   * @brief Release the internal native Eina iterator handle.
   */
  ~_common_iterator_base()
  {
    if(_iterator)
      eina_iterator_free(_iterator);
  }

  /**
   * @brief Creates an iterator from another iterator of the same type.
   * @param other Another iterator of the same type.
   *
   * @warning The supplied iterator transfer its internal handle to the
   * new iterator, thus @p other will point to nothing after the call
   * of this constructor.
   */
  _common_iterator_base(self_type const& other)
    : _iterator(other._iterator)
  {
    other._iterator = 0;
  }

  /**
   * @brief Acquire the internal handle of the given iterator.
   * @param other Another iterator of the same type.
   * @return Reference for itself.
   *
   * @warning The supplied iterator transfer its internal handle to the
   * new iterator, thus @p other will point to nothing after the call
   * of this constructor.
   */
  _common_iterator_base& operator=(self_type const& other)
  {
    _iterator = other._iterator;
    other._iterator = 0;
    return *this;
  }


  /**
   * @brief Get the handle for the wrapped @c Eina_Iterator.
   * @return Internal handle for the native Eina iterator.
   *
   * This member function returns the native @c Eina_Iterator handle
   * that is wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  Eina_Iterator* native_handle() const
  {
    return _iterator;
  }

protected:
  /**
   * @internal
   */
  mutable Eina_Iterator* _iterator;

  /**
   * @brief Check if the iterators wrap the same handle.
   * @param lhs Iterator at the left side of the expression.
   * @param lhs Iterator at the right side of the expression.
   * @return @c true if both iterators wrap the same handle, @c false otherwise.
   */
  friend inline bool operator==(_common_iterator_base<T> const& lhs, _common_iterator_base<T> const& rhs)
  {
    return lhs._iterator == rhs._iterator;
  }

  /**
   * @brief Check if the iterators wrap the different handles.
   * @param lhs Iterator at the left side of the expression.
   * @param lhs Iterator at the right side of the expression.
   * @return @c true if the iterators wrap different handles, @c false otherwise.
   */
  friend inline bool operator!=(_common_iterator_base<T> const& lhs, _common_iterator_base<T> const& rhs)
  {
    return !(lhs == rhs);
  }
};

/**
 * C++ wrappers to the native @c Eina_Iterator.
 * It provides a generic interface to the @c Eina_Iterator functions, and
 * automatically take care of allocating a deallocating resources using
 * the RAII programming idiom.
 */
template <typename T, typename Enable = void>
struct iterator
  : _common_iterator_base<typename std::enable_if<! ::efl::eo::is_eolian_object<T>::value, T const>::type>
{
private:
  typedef _common_iterator_base<T const> base_type; /**< Type for the base class. */
  typename base_type::pointer _value; /**< @internal */
  typedef iterator<T> self_type; /**< Type for the specialized iterator itself. */
public:
  typedef typename base_type::value_type value_type; /**< Type for elements returned by the iterator. */
  typedef typename base_type::pointer pointer; /**< Type for a pointer to an element. */
  typedef typename base_type::reference reference; /**< Type for a reference to an element. */
  typedef typename base_type::difference_type difference_type; /**< Type to represent the distance between two iterators. */
  typedef typename base_type::iterator_category iterator_category; /**< Defines the iterator category as the same of the base class. */

  /**
   * @brief Creates a iterator wrapping the given native @c Eina_Iterator handle.
   *
   * This constructor creates an iterator that wraps the given native
   * @c Eina_Iterator handle, providing a generic interface to it.
   */
  explicit iterator(Eina_Iterator* iterator_ = 0)
    : base_type(iterator_)
    , _value(nullptr)
  {
    if(this->_iterator)
      ++*this;
  }


  /**
   * @brief Move the iterator to the next position.
   * @return The iterator itself.
   *
   * This operator increments the iterator, making it point to the
   * position right after the current one.
   * At the end, it returns a reference to itself.
   */
  self_type& operator++()
  {
    void* data;
    Eina_Bool r = ::eina_iterator_next(this->_iterator, &data);
    if(!r)
      {
         ::eina_iterator_free(this->_iterator);
         this->_iterator = 0;
      }
    _value = static_cast<pointer>(data);
    return *this;
  }

  /**
   * @brief Move the iterator to the next position.
   * @return The iterator itself.
   *
   * Works exactly like @ref operator++().
   */
  self_type& operator++(int)
  {
    return ++*this;
  }

  /**
   * @brief Get a reference to the element currently pointed by the iterator.
   * @return Reference to the current element.
   */
  value_type& operator*() const
  {
    return *_value;
  }

  /**
   * @brief Return a pointer to the current element, which member will be accessed.
   * @return Pointer to the element currently pointed by the iterator.
   */
  pointer operator->() const
  {
    return _value;
  }
};

template <typename T>
struct iterator<T, typename std::enable_if< ::efl::eo::is_eolian_object<T>::value, void>::type>
  : _common_iterator_base<Eo const>
{
private:
  typedef _common_iterator_base<Eo const> base_type; /**< Type for the base class. */
  typename base_type::pointer _value; /**< @internal */
  typedef iterator<T> self_type; /**< Type for the specialized iterator itself. */
public:
  typedef typename base_type::value_type value_type; /**< Type for elements returned by the iterator. */
  typedef typename base_type::pointer pointer; /**< Type for a pointer to an element. */
  typedef typename base_type::reference reference; /**< Type for a reference to an element. */
  typedef typename base_type::difference_type difference_type; /**< Type to represent the distance between two iterators. */
  typedef typename base_type::iterator_category iterator_category; /**< Defines the iterator category as the same of the base class. */

  /**
   * @brief Creates a iterator wrapping the given native @c Eina_Iterator handle.
   *
   * This constructor creates an iterator that wraps the given native
   * @c Eina_Iterator handle, providing a generic interface to it.
   */
  explicit iterator(Eina_Iterator* iterator_ = 0)
    : base_type(iterator_)
    , _value(nullptr)
  {
    if(this->_iterator)
      ++*this;
  }


  /**
   * @brief Move the iterator to the next position.
   * @return The iterator itself.
   *
   * This operator increments the iterator, making it point to the
   * position right after the current one.
   * At the end, it returns a reference to itself.
   */
  self_type& operator++()
  {
    void* data;
    Eina_Bool r = ::eina_iterator_next(this->_iterator, &data);
    if(!r)
      {
         ::eina_iterator_free(this->_iterator);
         this->_iterator = 0;
      }
    _value = static_cast<pointer>(data);
    return *this;
  }

  /**
   * @brief Move the iterator to the next position.
   * @return The iterator itself.
   *
   * Works exactly like @ref operator++().
   */
  self_type& operator++(int)
  {
    return ++**this;
  }

  /**
   * @brief Get a reference to the element currently pointed by the iterator.
   * @return Reference to the current element.
   */
  T const& operator*() const
  {
    // relies on layout compatibility between eo::concrete and Eo*
    return *reinterpret_cast<T const*>(&_value);
  }

  /**
   * @brief Return a pointer to the current element, which member will be accessed.
   * @return Pointer to the element currently pointed by the iterator.
   */
  T const* operator->() const
  {
    // relies on layout compatibility between eo::concrete and Eo*
    return reinterpret_cast<T const*>(&_value);
  }
};

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
