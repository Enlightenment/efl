#ifndef EINA_INARRAY_HH_
#define EINA_INARRAY_HH_

#include <Eina.h>
#include <eina_type_traits.hh>
#include <eina_range_types.hh>

#include <iterator>
#include <cstring>
#include <cassert>

/**
 * @addtogroup Eina_Cxx_Containers_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Inline_Array_Group Inline Array
 * @ingroup Eina_Cxx_Containers_Group
 *
 * Inline array is a container that stores the data itself not pointers to data,
 * this means there is no memory fragmentation, also for small data types(such
 * as char, short, int, etc.) it's more memory efficient.
 *
 * @{
 */

/**
 * @internal
 * Access traits for eina::inarray.
 */
struct _inarray_access_traits {

  template <typename T>
  struct const_iterator
  {
    typedef T const* type; /**< Type for constant iterator. */
  };
  template <typename T>
  struct iterator
  {
    typedef T* type; /**< Type for iterator. */
  };
  template <typename T>
  struct const_native_handle
  {
    typedef Eina_Inarray const* type; /**< Type for constant native @c Eina_Inarray handle. */
  };
  template <typename T>
  struct native_handle
  {
    typedef Eina_Inarray* type; /**< Type for native @c Eina_Inarray handle. */
  };

  /**
   * @brief Get a non-constant native @c Eina_Inarray handle from a constant one.
   */
  template <typename T>
  static Eina_Inarray* native_handle_from_const(Eina_Inarray const* array)
  {
    return const_cast<Eina_Inarray*>(array);
  }

/**
 * @brief Get a reference to the last element of the given @c Eina_Inarray.
 */
template <typename T>
static T& back(Eina_Inarray* raw)
{
  assert(!_inarray_access_traits::empty<T>(raw));
  return *static_cast<T*>( ::eina_inarray_nth(raw, _inarray_access_traits::size<T>(raw)-1u));
}
/**
 * @brief Get a constant reference to the last element of the given @c Eina_Inarray.
 *
 * Version of @ref back(Eina_Inarray* raw) for const-qualified pointer
 * to @c Eina_Inarray. Get a constant reference to the last element
 * instead.
 */
template <typename T>
static T const& back(Eina_Inarray const* raw)
{
  return _inarray_access_traits::back<T>(const_cast<Eina_Inarray*>(raw));
}

/**
 * @brief Get a reference to the first element of the given @c Eina_Inarray.
 */
template <typename T>
static T& front(Eina_Inarray* raw)
{
  assert(!empty<T>(raw));
  return *static_cast<T*>( ::eina_inarray_nth(raw, 0u));
}

/**
 * @brief Get a constant reference to the first element of the given @c Eina_Inarray.
 *
 * Version of @ref front(Eina_Inarray* raw) for const-qualified
 * pointer to @c Eina_Inarray. Get a constant reference to the first
 * element instead.
 */
template <typename T>
static T const& front(Eina_Inarray const* raw)
{
  return _inarray_access_traits::front<T>(const_cast<Eina_Inarray*>(raw));
}

/**
 * @brief Get an iterator to the begin of the memory block of the given @c Eina_Inarray.
 */
template <typename T>
static T* begin(Eina_Inarray* raw)
{
  return !raw->members ? 0 : static_cast<T*>( ::eina_inarray_nth(raw, 0u));
}

/**
 * @brief Get an iterator to the end of the memory block of the given @c Eina_Inarray.
 */
template <typename T>
static T* end(Eina_Inarray* raw)
{
  return !raw->members ? 0
    : static_cast<T*>( ::eina_inarray_nth(raw, _inarray_access_traits::size<T>(raw) -1)) + 1;
}

/**
 * @brief Get a constant iterator to the begin of the memory block of the given @c Eina_Inarray.
 *
 * Version of @ref begin(Eina_Inarray* raw) for const-qualified
 * @c Eina_Inarray handles. Returns a constant iterator instead.
 */
template <typename T>
static T const* begin(Eina_Inarray const* raw)
{
  return _inarray_access_traits::begin<T>(const_cast<Eina_Inarray*>(raw));
}


/**
 * @brief Get a constant iterator to the end of the memory block of the given @c Eina_Inarray.
 *
 * Version of @ref end(Eina_Inarray* raw) const-qualified
 * @c Eina_Inarray. Returns a constant iterator instead.
 */
template <typename T>
static T const* end(Eina_Inarray const* raw)
{
  return _inarray_access_traits::end<T>(const_cast<Eina_Inarray*>(raw));
}

/**
 * @brief Get a constant reverse iterator pointing to the reverse begin of the given Eina_Inarray.
 *
 * Version of @ref rbegin(Eina_Inarray* raw) for const-qualified
 * Eina_Inarray handles. Returns a constant reverse iterator instead.
 */
template <typename T>
static std::reverse_iterator<T const*> rbegin(Eina_Inarray const* raw)
{
  return std::reverse_iterator<T const*>(_inarray_access_traits::end<T>(raw));
}

/**
 * @brief Get a constant reverse iterator pointing to the reverse end of the given Eina_Inarray.
 *
 * Version of @ref rend(Eina_Inarray* raw) to const-qualified
 * Eina_Inarray handles. Returns a constant reverse iterator instead.
 */
template <typename T>
static std::reverse_iterator<T const*> rend(Eina_Inarray const* raw)
{
  return std::reverse_iterator<T const*>(_inarray_access_traits::begin<T>(raw));
}

/**
 * @brief Get a reverse iterator pointing to the reverse begin of the given @c Eina_Inarray.
 */
template <typename T>
static std::reverse_iterator<T*> rbegin(Eina_Inarray* raw)
{
  return std::reverse_iterator<T*>(_inarray_access_traits::end<T>(raw));
}

/**
 * @brief Get a reverse iterator pointing to the reverse end of the given @c Eina_Inarray.
 */
template <typename T>
static std::reverse_iterator<T*> rend(Eina_Inarray* raw)
{
  return std::reverse_iterator<T*>(_inarray_access_traits::begin<T>(raw));
}

/**
 * @brief Get a constant iterator to the begin of the memory block of the given @c Eina_Inarray.
 *
 * Works like @ref begin(Eina_Inarray const* raw) but is granted to
 * return a constant iterator even for handles that are not
 * const-qualified.
 */
template <typename T>
static T const* cbegin(Eina_Inarray const* raw)
{
  return _inarray_access_traits::begin<T>(raw);
}

/**
 * @brief Get a constant iterator to the end of the memory block of the given @c Eina_Inarray.
 *
 * Works like @ref end(Eina_Inarray const* raw) but is granted to
 * return a constant iterator even for handles that are not
 * const-qualified.
 */
template <typename T>
static T const* cend(Eina_Inarray const* raw)
{
  return _inarray_access_traits::end<T>(raw);
}

/**
 * @brief Get a constant reverse iterator to the end of the memory block of the given @c Eina_Inarray.
 *
 * Works like @ref rbegin(Eina_Inarray const* raw) but is granted to
 * return a constant reverse iterator even for handles that are not
 * const-qualified.
 */
template <typename T>
static std::reverse_iterator<T const*> crbegin(Eina_Inarray const* raw)
{
  return _inarray_access_traits::rbegin<T const*>(raw);
}

/**
 * @brief Get a constant reverse iterator to the begin of the memory block of the given @c Eina_Inarray.
 *
 * Works like @ref rend(Eina_Inarray const* raw) but is granted to
 * return a constant reverse iterator even for handles that are not
 * const-qualified.
 */
template <typename T>
static std::reverse_iterator<T const*> crend(Eina_Inarray const* raw)
{
  return _inarray_access_traits::rend<T const*>(raw);
}

/**
 * @brief Check if the given Eina array is empty.
 * @return @c true if the given array is empty, @c false otherwise.
 *
 * This functions returns @c true if the given @c Eina_Inarray contains
 * no elements, otherwise it returns @c false.
 */
template <typename T>
static inline bool empty(Eina_Inarray const* raw)
{
  return _inarray_access_traits::size<T>(raw) == 0;
}

/**
 * @brief Get the size of the given @c Eina_Inarray.
 * @return Number of elements in the given array.
 *
 * This function returns the current number of elements inside of @p raw.
 */
template <typename T>
static inline std::size_t size(Eina_Inarray const* raw)
{
  return ::eina_inarray_count(raw);
}

/**
 * @brief Get a constant reference to the element at the given position.
 * @param raw Constant pointer to an @c Eina_Inarray.
 * @param i Position of the element.
 * @return Constant reference to the element.
 *
 * Version of @ref index() for const-qualified @c Eina_Inarray. Returns
 * a constant reference instead.
 */
template <typename T>
static T const& index(Eina_Inarray const* raw, std::size_t i)
{
  return *(_inarray_access_traits::begin<T>(raw) + i);
}

/**
 * @brief Get a reference to the element at the given position.
 * @param raw Pointer to a @c Eina_Inarray.
 * @param i Position of the element.
 * @return Reference to the element.
 *
 * This member function returns a reference to the element at position
 * @p i inside @p raw.
 */
template <typename T>
static T& index(Eina_Inarray* raw, std::size_t i)
{
  return *(_inarray_access_traits::begin<T>(raw) + i);
}


};

template <typename T>
class inarray;

/**
 * @ingroup Eina_Cxx_Range_Group
 *
 * Range class for @ref inarray.
 */
template <typename T>
struct range_inarray : _range_template<T, _inarray_access_traits>
{
  typedef _range_template<T, _inarray_access_traits> _base_type; /**< Type for the base class. */
  typedef typename std::remove_const<T>::type value_type; /**< The type of each element. */

  /**
   * @brief Creates a range from a native Eina inline array handle.
   */
  range_inarray(Eina_Inarray* array)
    : _base_type(array)
  {}

  /**
   * @brief Creates a range from a @c inarray object.
   */
  range_inarray(inarray<T>& array)
    : _base_type(array.native_handle())
  {}

  /**
   * @brief Get the element at the given position in the array.
   * @param index Position of the element.
   * @return Reference to the element at the given position.
   */
  value_type& operator[](std::size_t index) const
  {
    return _inarray_access_traits::index<T>(this->native_handle(), index);
  }
};

/**
 * Common inarray interface for every value type.
 */
struct _inarray_common_base
{
  typedef std::size_t size_type; /**< Type for size information used in the array. */
  typedef Eina_Inarray* native_handle_type; /** Type for the native @c Eina_Inarray handle. */
  typedef Eina_Inarray const* const_native_handle_type; /** Type for constant native @c Eina_Inarray handle. */

  /**
   * @brief Creates a new array object from a handle to a native @c Eina_Inarray.
   * @param array Handler to a native @c Eina_Inarray
   *
   * This constructor wraps a pre-allocated @c Eina_Inarray providing an
   * OO interface to it.
   *
   * @warning It is important to note that the created array object
   * gains ownership of the handle, deallocating it at destruction time.
   */
  explicit _inarray_common_base(native_handle_type array)
    : _array(array) {}

  /**
   * @brief Allocates a array with the given size for each element.
   * @param member_size Size of each element in the array.
   *
   * This constructor creates an inline array object with the given
   * size (in bytes) for each element. All allocated memory will be
   * released at destruction.
   */
  explicit _inarray_common_base(size_type member_size)
    : _array( ::eina_inarray_new(member_size, 0) )
  {
  }

  /**
   * @brief Release the inline array memory.
   *
   * This destructor release the internal native @c Eina_Inarray handle,
   * freeing allocated memory.
   */
  ~_inarray_common_base()
  {
    ::eina_inarray_free(_array);
  }

  /**
   * @brief Get the current size of the array.
   * @return Number of elements in the array.
   *
   * This member function returns the current number of elements inside
   * the inline array.
   */
  size_type size() const
  {
    return _inarray_access_traits::size<void>(_array);
  }

  /**
   * @brief Check if the array is empty.
   * @return @c true if the array is empty, @c false otherwise.
   *
   * This member function returns @c true if the array does not contain
   * any elements, otherwise it returns @c false.
   */
  bool empty() const
  {
    return _inarray_access_traits::empty<void>(_array);
  }

  /**
   * @brief Get the handle for the wrapped Eina_Inarray.
   * @return Internal handle for the native Eina inline array.
   *
   * This member function returns the native @c Eina_Inarray handle that
   * is wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  native_handle_type native_handle() { return _array; }

  /**
   * @brief Get a constant handle for the wrapped Eina_Inarray.
   * @return Constant handle for the native Eina inline array.
   *
   * Version of @ref native_handle() for const-qualified objects.
   * Return a constant handle instead.
   */
  const_native_handle_type native_handle() const { return _array; }

  /**
   * @internal
   * Member variable that holds the native @c Eina_Inarray handle.
   */
  native_handle_type _array;
private:
  /** Disabled copy constructor. */
  _inarray_common_base(_inarray_common_base const& other);
  /** Disabled assignment operator. */
  _inarray_common_base& operator=(_inarray_common_base const& other);
};

/**
 * Optimized specialization of the base inline array for POD types.
 */
template <typename T>
class _pod_inarray : _inarray_common_base
{
  typedef _inarray_common_base _base_type; /**< Type for the base class. */
public:
  typedef T value_type; /**< The type of each element. */
  typedef T& reference; /**< Type for a reference to an element. */
  typedef T const& const_reference; /**< Type for a constant reference to an element. */
  typedef T* pointer; /**< Type for a pointer to an element. */
  typedef T const* const_pointer; /**< Type for a constant pointer for an element. */

  /**
   * Type for a iterator for this container.
   * Defined as a @ref pointer for performance reasons.
   */
  typedef pointer iterator;

  /**
   * Type for a constant iterator for this container.
   * Defined as a  @ref const_pointer for performance reasons.
   */
  typedef const_pointer const_iterator;
  typedef std::size_t size_type; /**< Type for size information used in the array. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two iterators. */
  typedef _base_type::native_handle_type native_handle_type; /** Type for the native @c Eina_Inarray handle. */
  typedef _base_type::const_native_handle_type const_native_handle_type; /** Type for constant native @c Eina_Inarray handle. */

  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for reverse iterator of the array. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /** Type for constant reverse iterator of the array. */

  using _base_type::size; /**< Type for size information used in the array. */

  /**
   * @brief Check if the array is empty.
   * @return @c true if the array is empty, @c false otherwise.
   *
   * This member function returns @c true if the array does not contain
   * any elements, otherwise it returns @c false.
   */
   using _base_type::empty;
  using _base_type::native_handle; /** Type for the native @c Eina_Inarray handle. */


  /**
   * @brief Create a new object from a handle to a native @c Eina_Inarray.
   * @param array Handler to a native @c Eina_Inarray.
   *
   * This constructor wraps a pre-allocated @c Eina_Inarray providing an
   * OOP interface to it.
   *
   * @warning It is important to note that the created object gains
   * ownership of the handle, deallocating it at destruction time.
   */
  _pod_inarray(native_handle_type array)
    : _base_type(array) {}

  /**
   * @brief Default constructor. Create an empty array.
   *
   * This constructor creates an array object with no elements. Elements
   * are declarated as having the same size of the given template
   * typename argument.
   */
  _pod_inarray() : _base_type(sizeof(T))
  {
  }

  /**
   * @brief Construct an array object with @p n copies of @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   *
   * This constructor creates an inline array with @p n elements, each
   * one as a copy of @p t.
   */
  _pod_inarray(size_type n, value_type const& t) : _base_type(sizeof(T))
  {
    while(n--)
      push_back(t);
  }

  /**
   * @brief Create a inline array with elements from the given range.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   *
   * This constructor creates a inline array with copies of the elements
   * between @p i and @p j in the same order.
   *
   * @note The ending element (pointed by @p j) is not copied.
   */
  template <typename InputIterator>
  _pod_inarray(InputIterator i, InputIterator const& j
         , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
    : _base_type(sizeof(T))
  {
    while(i != j)
      {
        push_back(*i);
        ++i;
      }
  }

  /**
   * @brief Copy constructor. Creates a copy of the given inline array.
   * @param other Another inline array of the same type.
   *
   * This constructor creates an inline array containing a copy of each
   * element inside @p other in the same order.
   */
  _pod_inarray(_pod_inarray<T>const& other)
    : _base_type(sizeof(T))
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * Do nothing, the native @c Eina_Inarray is already released in the
   * base class destructor.
   */
  ~_pod_inarray()
  {
  }

  /**
   * @brief Replace the current content with the cotent of another array.
   * @param other Another inline array of the same type.
   *
   * This assignment operator replaces the content of the array by a
   * copy of the content of @p other. The array size is adjusted
   * accordingly and the newly copied elements keep their original order.
   */
  _pod_inarray<T>& operator=(_pod_inarray<T>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }

  /**
   * @brief Remove all the elements of the array.
   */
  void clear()
  {
    ::eina_inarray_flush(_array);
  }

  /**
   * @brief Add a copy of the given element at the end of the array.
   * @param value Element to be added at the end of the array.
   *
   * This member function allocates a new element at the end of the
   * inline array, the content of @p value is copied to the new element.
   */
  void push_back(T const& value)
  {
    size_type s = size();
    static_cast<void>(s);
    eina_inarray_push(_array, &value);
    assert(size() != s);
    assert(size() == s + 1u);
  }

  /**
   * @brief Remove the last element of the array.
   */
  void pop_back()
  {
    eina_inarray_pop(_array);
  }

  /**
   * @brief Insert a new element at the given position.
   * @param i Iterator pointing to the position where the new element will be inserted.
   * @param t Value to be copied to the new element.
   * @return Iterator pointing to the new element inserted.
   *
   * This member function inserts a copy of the element @p t at the
   * position @p i. The new element comes right before the element
   * originally pointed by @p i.
   *
   * At the end, a valid iterator pointing to the element just inserted
   * is returned.
   */
  iterator insert(iterator i, value_type const& t)
  {
    if(i != end())
    {
      T* q = static_cast<iterator>
        ( ::eina_inarray_alloc_at(_array, i - begin(), 1u));
      std::memcpy(q, &t, sizeof(t));
      return q;
    }
    else
    {
      push_back(t);
      return end()-1;
    }
  }

  /**
   * @brief Insert @p n copies of @p t at the given position.
   * @param i Iterator pointing to the position where the new elements will be inserted.
   * @param n Number of elements to be inserted.
   * @param t Value to be copied to each new inserted element.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts @p n new elements at position @p i
   * in the array, each one as a copy of @p t. The new elements come
   * right before the element originally pointed by @p i.
   *
   * At the end, a valid iterator pointing to the first element inserted
   * is returned.
   */
  iterator insert(iterator i, size_t n, value_type const& t)
  {
    T* q;
    if(i != end())
    {
      q = static_cast<iterator>
        ( ::eina_inarray_alloc_at(_array, i - &_array->members, n));
    }
    else
    {
      q = eina_inarray_grow(_array, n);
    }
    for(T* p = q; n; --n, ++p)
      std::memcpy(p, &t, sizeof(t));
    return q;
  }

  /**
   * @brief Insert the elements between the given range at the given position.
   * @param p Iterator pointing to the position where the new elements will be inserted.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts a copy of the elements between @p i
   * and @p j at the position @p p. The new elements come right before
   * the element originally pointed by @p p. Note that the element
   * pointed by @p j is not copied.
   *
   * At the end, a valid iterator pointing to the first element inserted
   * is returned.
   */
  template <typename InputIterator>
  iterator insert(iterator p, InputIterator i, InputIterator j
                  , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    size_type n = 0;
    while(i != j)
      {
        p = insert(p, *i);
        ++p;
        ++i;
        ++n;
      }
    return p - n;
  }

  /**
   * @brief Remove the element at the given position.
   * @param q Iterator pointing to the element to be removed.
   * @return Iterator pointing to the element after the removed one.
   *
   * This member function removes the element pointed by the iterator
   * @p q, reducing the array size by one. At the end, a valid iterator
   * pointing to the element right after the removed one is returned.
   */
  iterator erase(iterator q)
  {
    ::eina_inarray_remove_at(_array, q - begin());
    return q;
  }

  /**
   * @brief Remove the elements between the given range.
   * @param i Iterator pointing to the starting position to be removed.
   * @param j Iterator pointing to the ending position to be removed.
   *          The element pointed by this iterator is not removed.
   * @return Iterator pointing to the new position of the first
   *         non-removed element after the removed ones (i.e. the one
   *         originally pointed by @p j).
   *
   * This member function removes the elements between the iterators
   * @p i and @p j, including the element pointed by @p i but not the
   * element pointed by @j.
   */
  iterator erase(iterator i, iterator j)
  {
    while(i != j)
      {
        erase(--j);
      }
    return i;
  }

  /**
   * @brief Replace the content of the array by the elements in the given range.
   * @param i Iterator pointing to the beginning of the elements to be copied.
   * @param j Iterator pointing to the end of the elements to be copied.
   *          Note that the element pointed by j will NOT be copied.
   *
   * This member function replaces the current elements by copies of the
   * elements between the iterators @p i and @p j, including the element
   * pointed by @p i but not the one pointed by @p j. The size of the
   * array is adjusted accordingly and the newly copied elements remain
   * in their original order.
   */
  template <typename InputIterator>
  void assign(InputIterator i, InputIterator j
              , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0);

  /**
   * @brief Replace the content of the array by @p n copies @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   */
  void assign(size_type n, value_type const& t);

  /**
   * @brief Get a reference to the element at the given position.
   * @param i Position of the element.
   * @return Reference to the element at the ith position.
   */
  value_type& operator[](size_type i)
  {
    return *(begin() + i);
  }

  /**
   * @brief Get a constant reference to the element at the given position.
   * @param i Position of the element.
   * @return Constant reference to the element at the ith position.
   *
   * Version of @ref operator[](size_type i) for const-qualified inline
   * array objects. Returns a constant reference instead.
   */
  value_type const& operator[](size_type i) const
  {
    return const_cast<inarray<T>&>(*this)[i];
  }

  /**
   * @brief Get a reference to the last element.
   * @return Reference to the last element in the array.
   */
  value_type& back()
  {
    return _inarray_access_traits::back<value_type>(_array);
  }

  /**
   * @brief Get a constant reference to the last element.
   * @return Constant reference to the last element in the array.
   *
   * Version of @ref back() for const-qualified inline array objects.
   * Returns a constant reference instead.
   */
  value_type const& back() const
  {
    return _inarray_access_traits::back<value_type>(_array);
  }

  /**
   * @brief Get a reference to the first element.
   * @return Reference to the first element of the array.
   */
  value_type& front()
  {
    return _inarray_access_traits::front<value_type>(_array);
  }

  /**
   * @brief Get a constant reference to the first element.
   * @return Constant reference to the first element of the array.
   *
   * Version of @ref front() for const-qualified inline array objects.
   * Returns a constant reference instead.
   */
  value_type const& front() const
  {
    return _inarray_access_traits::front<value_type>(_array);
  }

  /**
   * @brief Get an iterator pointing to the first element of the array.
   * @return Iterator to the initial position of the array.
   *
   * This member function returns an iterator pointing to the first
   * element of the array. If the array is empty the returned iterator
   * is the same as the one returned by @ref end().
   */
  iterator begin()
  {
    return _inarray_access_traits::begin<value_type>(_array);
  }

  /**
   * @brief Get an iterator to the position following the last element of the array.
   * @return Iterator to the final position of the array.
   *
   * This member function returns an iterator to the position following
   * the last element in the array. If the array is empty the returned
   * iterator is the same as the one returned by @ref begin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  iterator end()
  {
    return _inarray_access_traits::end<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the array.
   * @return Constant iterator to the initial position of the array.
   *
   * Version of @ref begin() for const-qualified inline array objects.
   * Returns a constant iterator instead.
   */
  const_iterator begin() const
  {
    return _inarray_access_traits::begin<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the array.
   * @return Constant iterator to the final position of the array.
   *
   * Version of @ref end() for const-qualified inline array objects.
   * Returns a constant iterator instead.
   */
  const_iterator end() const
  {
    return _inarray_access_traits::end<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the array.
   * @return Constant reverse iterator pointing to the reverse begin of the array.
   *
   * Version of @ref rbegin() for const-qualified inline array objects.
   * Returns a constant reverse iterator instead.
   */
  const_reverse_iterator rbegin() const
  {
    return _inarray_access_traits::rbegin<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the array.
   * @return Constant reverse iterator pointing to the reverse end of the array.
   *
   * Version of @ref rend() for const-qualified inline array objects.
   * Returns a constant reverse iterator instead.
   */
  const_reverse_iterator rend() const
  {
    return _inarray_access_traits::rend<value_type>(_array);
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse begin of the array.
   * @return Reverse iterator pointing to the reverse begin of the array.
   *
   * This member function returns a reverse iterator pointing to the
   * last element of the array. If the array is empty the returned
   * reverse iterator is the same as the one returned by @ref rend().
   */
  reverse_iterator rbegin()
  {
    return _inarray_access_traits::rbegin<value_type>(_array);
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse end of the array.
   * @return Reverse iterator pointing to the reverse end of the array.
   *
   * This member function returns a reverse iterator pointing to the
   * position before the first element of the array. If the array is
   * empty the returned iterator is the same as the one returned by
   * @ref rbegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  reverse_iterator rend()
  {
    return _inarray_access_traits::rend<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the array.
   * @return Constant iterator to the initial position of the array.
   *
   * This member function works like @ref begin() const but is granted
   * to return a constant iterator even for arrays that are not
   * const-qualified.
   */
  const_iterator cbegin() const
  {
    return _inarray_access_traits::cbegin<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the array.
   * @return Constant iterator to the final position of the array.
   *
   * This member function works like @ref end() const  but is granted to
   * return a constant iterator even for arrays that are not
   * const-qualified.
   */
  const_iterator cend() const
  {
    return _inarray_access_traits::cend<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the array.
   * @return Constant reverse iterator pointing to the reverse begin of the array.
   *
   * This member function works like @ref rbegin() const but is granted
   * to return a constant reverse iterator even for arrays that are not
   * const-qualified.
   */
  const_reverse_iterator crbegin() const
  {
    return _inarray_access_traits::crbegin<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the array.
   * @return Constant reverse iterator pointing to the reverse end of the array.
   *
   * This member function works like @ref rend() const but is granted to
   * return a constant reverse iterator even for arrays that are not
   * const-qualified.
   */
  const_reverse_iterator crend() const
  {
    return _inarray_access_traits::crend<value_type>(_array);
  }

  /**
   * @brief Swap content between two inline arrays.
   * @param other Other inline array of the same type.
   */
  void swap(_pod_inarray<T>& other)
  {
    std::swap(_array, other._array);
  }

  /**
   * @brief Get the maximum number of elements a inline array can hold.
   * @return Maximum number of elements a inline array can hold.
   */
  size_type max_size() const { return -1; }

  /**
   * @brief Get a handle for the wrapped Eina_Inarray.
   * @return Handle for the native Eina inline array.
   *
   * This member function returns the native Eina_Inarray handle that is
   * wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  native_handle_type native_handle()
  {
    return this->_array;
  }

  /**
   * @brief Get a constant handle for the wrapped Eina_Inarray.
   * @return Constant handle for the native Eina inline array.
   *
   * Version of @ref native_handle() for const-qualified objects.Returns
   * a constant handle instead.
   *
   * @see native_handle()
   */
  const_native_handle_type native_handle() const
  {
    return this->_array;
  }
};

template <typename T>
class _nonpod_inarray : _inarray_common_base
{
  typedef _inarray_common_base _base_type; /**< Type for the base class. */
public:
  typedef T value_type; /**< The type of each element. */
  typedef T& reference; /**< Type for a reference to an element. */
  typedef T const& const_reference; /**< Type for a constant reference to an element. */
  typedef T* pointer; /**< Type for a pointer to an element. */
  typedef T const* const_pointer; /**< Type for a constant pointer for an element. */

  /**
   * Type for a iterator to this kind of inline array.
   * Defined as a  @ref pointer for performance reasons.
   */
  typedef pointer iterator;

  /**
   * Type for a constant iterator for this kind of inline array.
   * Defined as a  @ref const_pointer for performance reasons.
   */
  typedef const_pointer const_iterator;
  typedef std::size_t size_type; /**< Type for size information used in the array. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two iterators. */

  typedef std::reverse_iterator<iterator> reverse_iterator; /** Type for the native @c Eina_Inarray handle. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /** Type for constant native @c Eina_Inarray handle. */

  using _base_type::size;
  using _base_type::empty;

  /**
   * @brief Create a new object from a handle to a native Eina_Inarray.
   * @param array Handle to a native Eina_Inarray.
   *
   * This constructor wraps a pre-allocated Eina_Inarray providing an
   * OOP interface to it.
   *
   * @warning It is important to note that the created object gains
   * ownership of the handle, deallocating it at destruction time.
   */
  _nonpod_inarray(Eina_Inarray* array)
    : _base_type(array) {}

  /**
   * @brief Default constructor. Create an empty array.
   *
   * This constructor creates an array object with no elements. Elements
   * are declarated as having the same size of the given template
   * typename argument.
   */
  _nonpod_inarray() : _base_type(sizeof(T))
  {
  }

  /**
   * @brief Construct an array object with @p n copies of @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   *
   * This constructor creates an inline array with @p n elements, each
   * one as a copy of @p t.
   */
  _nonpod_inarray(size_type n, value_type const& t) : _base_type(sizeof(T))
  {
    while(n--)
      push_back(t);
  }

  /**
   * @brief Create a inline array coping the elements from the given range.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   *
   * This constructor creates a inline array with copies of the elements
   * between @p i and @p j in the same order. Note that the ending
   * element (pointed by @p j) is excluded.
   */
  template <typename InputIterator>
  _nonpod_inarray(InputIterator i, InputIterator const& j
         , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
    : _base_type(sizeof(T))
  {
    while(i != j)
      {
        push_back(*i);
        ++i;
      }
  }

  /**
   * @brief Copy constructor. Creates a copy of the given inline array.
   * @param other Another inline array of the same type.
   *
   * This constructor creates an inline array containing a copy of each
   * element inside @p other in the same order.
   */
  _nonpod_inarray(_nonpod_inarray<T>const& other)
    : _base_type(sizeof(T))
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * @brief Destructor of array for non-POD elements.
   *
   * Calls the destructor of each allocated element, before the base
   * class destructor releases their memory.
   */
  ~_nonpod_inarray()
  {
    for(T* first = static_cast<T*>(_array->members)
          , *last = first + _array->len; first != last; ++first)
      first->~T();
  }

  /**
   * @brief Replace current content with the cotent of another array.
   * @param other Another inline array of the same type.
   *
   * This assignment operator replaces the content of the array by a
   * copy of the content of the given array @p other. The array size is
   * adjusted accordingly and the newly copied elements keep their
   * original order.
   */
  _nonpod_inarray<T>& operator=(_nonpod_inarray<T>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }

  /**
   * @brief Remove all the elements of the array.
   */
  void clear()
  {
    for(T* first = static_cast<T*>(_array->members)
          , *last = first + _array->len; first != last; ++first)
      first->~T();
    ::eina_inarray_flush(_array);
  }

  /**
   * @brief Add a copy of the given element at the end of the array.
   * @param value Element to be added at the end of the array.
   *
   * This member function allocates a new element at the end of the
   * inline array, the content of @p value is copied to the new element.
   */
  void push_back(T const& value)
  {
    insert(end(), 1u, value);
  }

  /**
   * @brief Remove the last element of the array.
   */
  void pop_back()
  {
    T* elem = static_cast<T*>(_array->members) + _array->len - 1;
    elem->~T();
    eina_inarray_pop(_array);
  }

  /**
   * @brief Insert a new element at the given position.
   * @param i Iterator pointing to the position where the new element will be inserted.
   * @param t Value to be copied to the new element.
   * @return Iterator pointing to the new element inserted.
   *
   * This member function inserts a copy of the element @p t at the
   * position @p i. The new element comes right before the element
   * originally pointed by @p i.
   *
   * At the end, a valid iterator pointing to the element just inserted
   * is returned.
   */
  iterator insert(iterator i, value_type const& t)
  {
    return insert(i, 1u, t);
  }

  /**
   * @brief Insert @p n copies of @p t at the given position.
   * @param i Iterator pointing to the position where the new elements will be inserted.
   * @param n Number of elements to be inserted.
   * @param t Value to be copied to each new inserted element.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts @p n new elements at position @p i
   * in the array, each one as a copy of @p t. The new elements come
   * right before the element originally pointed by @p i.
   *
   * At the end, a valid iterator pointing to the first element inserted
   * is returned.
   */
  iterator insert(iterator i, size_t n, value_type const& t)
  {
    if(_array->max - _array->len >= n)
      {
        iterator end = static_cast<T*>(_array->members)
          + _array->len
          , last = end + n;
        _array->len += n;
        std::reverse_iterator<iterator>
          dest(last), src(end), src_end(i);
        for(;src != src_end; ++src)
          {
            if(dest.base() <= end)
                *dest++ = *src;
            else
              new (&*dest++) T(*src);
          }
        iterator j = i;
        for(size_type k = 0;k != n;++k)
          {
            if(j < end)
                *j = t;
            else
              new (&*j++) T(t);
          }
      }
    else
      {
        size_type index = i - static_cast<iterator>(_array->members);

        Eina_Inarray* old_array = eina_inarray_new(_array->member_size, 0);
        *old_array = *_array;
        _array->len = _array->max = 0;
        _array->members = 0;
        eina_inarray_resize(_array, old_array->len+n);
        _array->len = old_array->len+n;

        iterator old_first = static_cast<iterator>(old_array->members)
          , first = begin()
          , last = first + _array->len;
        i = index + begin();

        while(first != i)
          {
            new (&*first++) T(*old_first);
            old_first++->~T();
          }
        for(size_type j = 0;j != n;++j)
          new (&*first++) T(t);
        std::size_t diff = last - first;
        static_cast<void>(diff);
        assert(diff == _array->len - index - n);
        static_cast<void>(diff);
        while(first != last)
          {
            new (&*first++) T(*old_first);
            old_first++->~T();
          }
      }
    return i;
  }

  /**
   * @brief Insert the elements between the given range at the given position.
   * @param p Iterator pointing to the position where the new elements will be inserted.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts a copy of the elements between @p i
   * and @p j at the position @p p. The new elements come right before
   * the element originally pointed by @p p. Note that the element
   * pointed by @p j is not copied.
   */
  template <typename InputIterator>
  iterator insert(iterator p, InputIterator i, InputIterator j
                  , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    size_type n = 0;
    while(i != j)
      {
        p = insert(p, *i);
        ++p;
        ++i;
        ++n;
      }
    return p - n;
  }

  /**
   * @brief Remove the element at the given position.
   * @param q Iterator pointing to the element to be removed.
   * @return Iterator pointing to the element after the removed one.
   *
   * This member function removes the element pointed by the iterator
   * @p q, reducing the array size by one. At the end, a valid iterator
   * pointing to the element right after the removed one is returned.
   */
  iterator erase(iterator q)
  {
    return erase(q, q+1);
  }

  /**
   * @brief Remove the elements between the given range.
   * @param i Iterator pointing to the starting position to be removed.
   * @param j Iterator pointing to the ending position to be removed.
   *          The element pointed by this iterator is not removed.
   * @return Iterator pointing to the new position of the first
   *         non-removed element after the removed ones (i.e. the one
   *         originally pointed by j).
   *
   * This member function removes the elements between the iterators
   * @p i and @p j, including the element pointed by @p i but not the
   * element pointed by @j.
   */
  iterator erase(iterator i, iterator j)
  {
    iterator last = end();
    iterator k = i, l = j;
    while(l != last)
      *k++ = *l++;
    while(k != last)
      k++->~T();
    _array->len -= j - i;

    return i;
  }

  /**
   * @brief Replace the content of the array by the elements in the given range.
   * @param i Iterator pointing to the beginning of the elements to be copied.
   * @param j Iterator pointing to the end of the elements to be copied.
   *          Note that the element pointed by j will NOT be copied.
   *
   * This member function replaces the current elements by copies of the
   * elements between the iterators @p i and @p j, including the element
   * pointed by @p i but not the one pointed by @p j. The size of the
   * array is adjusted accordingly and the newly copied elements remain
   * in their original order.
   */
  template <typename InputIterator>
  void assign(InputIterator i, InputIterator j
              , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0);

  /**
   * @brief Replace the content of the array by @p n copies @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   */
  void assign(size_type n, value_type const& t);

  /**
   * @brief Get a reference to the element at the given position.
   * @param i Position of the element.
   * @return Reference to the element at the ith position.
   */
  value_type& operator[](size_type i)
  {
    return *(begin() + i);
  }


  /**
   * @brief Get a constant reference to the element at the given position.
   * @param i Position of the element.
   * @return Constant reference to the element at the ith position.
   *
   * Version of @ref operator[](size_type i) for const-qualified inline
   * array objects. Return a constant reference instead.
   */
  value_type const& operator[](size_type i) const
  {
    return const_cast<inarray<T>&>(*this)[i];
  }

  /**
   * @brief Get a reference to the last element.
   * @return Reference to the last element in the array.
   */
  value_type& back()
  {
    return _inarray_access_traits::back<value_type>(_array);
  }

  /**
   * @brief Get a constant reference to the last element.
   * @return Constant reference to the last element in the array.
   *
   * Version of @ref back() for const-qualified inline array objects.
   * Return a constant reference instead.
   */
  value_type const& back() const
  {
    return _inarray_access_traits::back<value_type>(_array);
  }

  /**
   * @brief Get a reference to the first element.
   * @return Reference to the first element of the array.
   */
  value_type& front()
  {
    return _inarray_access_traits::front<value_type>(_array);
  }

  /**
   * @brief Get a constant reference to the first element.
   * @return Constant reference to the first element of the array.
   *
   * Version of @ref front() for const-qualified inline array objects.
   * Return a constant reference instead.
   */
  value_type const& front() const
  {
    return _inarray_access_traits::front<value_type>(_array);
  }

  /**
   * @brief Get an iterator pointing to the first element of the array.
   * @return Iterator to the initial position of the array.
   *
   * This member function returns an iterator pointing to the first
   * element of the array. If the array is empty the returned iterator
   * is the same as the one returned by @ref end().
   */
  iterator begin()
  {
    return _inarray_access_traits::begin<value_type>(_array);
  }

  /**
   * @brief Get an iterator to the position following the last element of the array.
   * @return Iterator to the final position of the array.
   *
   * This member function returns an iterator to the position following
   * the last element in the array. If the array is empty the returned
   * iterator is the same as the one returned by @ref begin().
   * Note that attempting to access this position causes undefined
   * behavior.
   */
  iterator end()
  {
    return _inarray_access_traits::end<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the array.
   * @return Constant iterator to the initial position of the array.
   *
   * Version of @ref begin() for const-qualified inline array objects.
   * Returns a constant iterator instead.
   */
  const_iterator begin() const
  {
    return _inarray_access_traits::begin<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the array.
   * @return Constant iterator to the final position of the array.
   *
   * Version of @ref end() for const-qualified inline array objects.
   * Returns a constant iterator instead.
   */
  const_iterator end() const
  {
    return _inarray_access_traits::end<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the array.
   * @return Constant reverse iterator pointing to the reverse begin of the array.
   *
   * Version of @ref rbegin() for const-qualified inline array objects.
   * Returns a constant reverse iterator instead.
   */
  const_reverse_iterator rbegin() const
  {
    return _inarray_access_traits::rbegin<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the array.
   * @return Constant reverse iterator pointing to the reverse end of the array.
   *
   * Version of @ref rend() for const-qualified inline array objects.
   * Returns a constant reverse iterator instead.
   */
  const_reverse_iterator rend() const
  {
    return _inarray_access_traits::rend<value_type>(_array);
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse begin of the array.
   * @return Reverse iterator pointing to the reverse begin of the array.
   *
   * This member function returns a reverse iterator pointing to the
   * last element of the array. If the array is empty the returned
   * reverse iterator is the same as the one returned by @ref rend().
   */
  reverse_iterator rbegin()
  {
    return _inarray_access_traits::rbegin<value_type>(_array);
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse end of the array.
   * @return Reverse iterator pointing to the reverse end of the array.
   *
   * This member function returns a reverse iterator pointing to the
   * position before the first element of the array. If the array is
   * empty the returned iterator is the same as the one returned by
   * @ref rbegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  reverse_iterator rend()
  {
    return _inarray_access_traits::rend<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the array.
   * @return Constant iterator to the initial position of the array.
   *
   * This member function works like the constant overload of @ref begin()
   * but is granted to return a constant iterator even for arrays that
   * are not const-qualified.
   */
  const_iterator cbegin() const
  {
    return _inarray_access_traits::cbegin<value_type>(_array);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the array.
   * @return Constant iterator to the final position of the array.
   *
   * This member function works like the constant overload of @ref end()
   * but is granted to return a constant iterator even for arrays that
   * are not const-qualified.
   */
  const_iterator cend() const
  {
    return _inarray_access_traits::cend<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the array.
   * @return Constant reverse iterator pointing to the reverse begin of the array.
   *
   * This member function works like the constant overload of @ref rbegin()
   * but is granted to return a constant reverse iterator even for
   * arrays that are not const-qualified.
   */
  const_reverse_iterator crbegin() const
  {
    return _inarray_access_traits::crbegin<value_type>(_array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the array.
   * @return Constant reverse iterator pointing to the reverse end of the array.
   *
   * This member function works like the constant overload of @ref rend()
   * but is granted to return a constant reverse iterator even for
   * arrays that are not const-qualified.
   */
  const_reverse_iterator crend() const
  {
    return _inarray_access_traits::crend<value_type>(_array);
  }

  /**
   * @brief Swap content between two inline arrays.
   * @param other Other inline array of the same type.
   */
  void swap(_nonpod_inarray<T>& other)
  {
    std::swap(_array, other._array);
  }

  /**
   * @brief Get the maximum number of elements a inline array can hold.
   * @return Maximum number of elements a inline array can hold.
   */
  size_type max_size() const { return -1; }

  /**
   * @brief Get the handle for the wrapped Eina_Inarray.
   * @return Internal handle for the native Eina inline array.
   *
   * This member function returns the native Eina_Inarray handle that is
   * wrapped inside this object. It is important to take care when using
   * it, since the handle will be automatically release upon object
   * destruction.
   */
  Eina_Inarray* native_handle()
  {
    return this->_array;
  }

  /**
   * @brief Get the handle for the wrapped Eina_Inarray.
   * @return Internal handle for the native Eina inline array.
   *
   * This member function returns the native Eina_Inarray handle that is
   * wrapped inside this object. It is important to take care when using
   * it, since the handle will be automatically release upon object
   * destruction.
   */
  Eina_Inarray const* native_handle() const
  {
    return this->_array;
  }
};


/**
 * Inline array class. It provides an OOP interface to the
 * @c Eina_Inarray functions, and automatically take care of allocating
 * and deallocating resources using the RAII programming idiom.
 *
 * It also provides additional member functions to facilitate the access
 * to the array content, much like a STL vector.
 */
template <typename T>
class inarray : public eina::if_<eina::is_pod<T>, _pod_inarray<T>
                                 , _nonpod_inarray<T> >::type
{
  typedef typename eina::if_<eina::is_pod<T>, _pod_inarray<T>
                             , _nonpod_inarray<T> >::type _base_type; /**< Type for the base class. */
public:

  /**
   * @brief Create a new object from a handle to a native Eina_Inarray.
   * @param array Handle to a native Eina_Inarray.
   *
   * This constructor wraps a pre-allocated Eina_Inarray providing an
   * OOP interface to it.
   *
   * @warning It is important to note that the created object gains
   * ownership of the handle, deallocating it at destruction time.
   */
  inarray(Eina_Inarray* array)
    : _base_type(array) {}

  /**
   * @brief Default constructor. Creates an empty array.
   */
  inarray() : _base_type() {}

  /**
   * @brief Construct an array object with @p n copies of @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   *
   * This constructor creates an inline array with @p n elements, each
   * one as a copy of @p t.
   */
  inarray(typename _base_type::size_type n, typename _base_type::value_type const& t)
    : _base_type(n, t) {}

  /**
   * @brief Create a inline array with elements from the given range.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   *
   * This constructor creates a inline array with copies of the elements
   * between @p i and @p j in the same order.
   *
   * @note The ending element (pointed by @p j) is not copied.
   */
  template <typename InputIterator>
  inarray(InputIterator i, InputIterator const& j
          , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
    : _base_type(i, j)
  {}

};

/**
 * @brief Check if two inline arrays are equal.
 * @param lhs Inline array at the left side of the expression.
 * @param rhs Inline array at the right side of the expression.
 * @return @c true if the arrays are equals, @c false otherwise.
 *
 * This operator checks if the given inline arrays are equal. To be
 * considered equal both arrays need to have the same number of
 * elements, and each element in one array must be equal to the element
 * at the same position in the other array.
 */
template <typename T>
bool operator==(inarray<T> const& lhs, inarray<T> const& rhs)
{
  return lhs.size() == rhs.size() &&
    std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Check if two inline arrays are different.
 * @param lhs Inline array at the left side of the expression.
 * @param rhs Inline array at the right side of the expression.
 * @return @c true if the arrays are not equal , @c false otherwise.
 *
 * This operator returns the opposite of @ref operator==(inarray<T> const& lhs, inarray<T> const& rhs).
 */
template <typename T>
bool operator!=(inarray<T> const& lhs, inarray<T> const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Swap content between two inline arrays.
 * @param other Other inline array of the same type.
 */
template <typename T>
void swap(inarray<T>& lhs, inarray<T>& rhs)
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
