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

/**
 * @addtogroup Eina_Cxx_Containers_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Pointer_Array_Group Array of Pointers
 * @ingroup Eina_Cxx_Containers_Group
 *
 * @{
 */

/**
 * @internal
 */
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

/**
 * @internal
 */
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
  return std::reverse_iterator<_ptr_array_iterator<T> >(_ptr_array_access_traits::end<T>(array));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T> > rend(Eina_Array* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T> >(_ptr_array_access_traits::begin<T>(array));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T const> > rbegin(Eina_Array const* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T const> >(_ptr_array_access_traits::end<T>(const_cast<Eina_Array*>(array)));
}
template <typename T>
static std::reverse_iterator<_ptr_array_iterator<T const> > rend(Eina_Array const* array)
{
  return std::reverse_iterator<_ptr_array_iterator<T const> >(_ptr_array_access_traits::begin<T>(const_cast<Eina_Array*>(array)));
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
static eina::iterator<T> iend(Eina_Array*)
{
  return eina::iterator<T>();
}
template <typename T>
static eina::iterator<T const> ibegin(Eina_Array const* array)
{
  return eina::iterator<T const>( ::eina_array_iterator_new(array) );
}
template <typename T>
static eina::iterator<T const> iend(Eina_Array const*)
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
class ptr_array;

/**
 * @ingroup Eina_Cxx_Range_Group
 *
 * Range for @ref ptr_array.
 */
template <typename T>
struct range_ptr_array : _range_template<T, _ptr_array_access_traits>
{
  typedef _range_template<T, _ptr_array_access_traits> _base_type;  /**< Type for the base class. */
  typedef typename _base_type::value_type value_type;  /**< The type of each element. */

  /**
   * @brief Creates a range from a native Eina array handle.
   */
  range_ptr_array(Eina_Array* array)
    : _base_type(array)
  {}

  /**
   * @brief Creates a range from a @c ptr_array object.
   */
  template <typename Allocator>
  range_ptr_array(ptr_array<value_type, Allocator>& array)
    : _base_type(array.native_handle())
  {}

  /**
   * @brief Get the element at the given position in the array.
   * @param index Position of the element.
   * @return Reference to the element at the given position.
   */
  value_type& operator[](std::size_t index) const
  {
    return _ptr_array_access_traits::index<T>(this->native_handle(), index);
  }
};

/**
 * Common implementations for the ptr_array.
 */
template <typename T, typename CloneAllocator>
struct _ptr_array_common_base
{
  typedef CloneAllocator clone_allocator_type; /**< Type for the clone allocator. */

  /**
   * @brief Creates an array with the given clone allocator.
   */
  _ptr_array_common_base(CloneAllocator clone_allocator)
    : _impl(clone_allocator)
  {}

  /**
   * @brief Create a new object from a handle to a native Eina_Array.
   * @param array Handle to a native Eina_Array.
   *
   * This constructor wraps a pre-allocated Eina_Array providing an OOP
   * interface to it.
   *
   * @warning It is important to note that the created object gains
   * ownership of the handle, deallocating it at destruction time.
   */
  _ptr_array_common_base(Eina_Array* _array)
    : _impl(_array)
  {}

  /**
   * @brief Default constructor. Create an empty array.
   *
   * This constructor creates an array with no elements.
   */
  _ptr_array_common_base() {}

  /**
   * @internal
   * @brief Get the clone allocator of the array.
   * @return Reference to the clone allocator.
   */
  CloneAllocator& _get_clone_allocator()
  {
    return _impl;
  }

  /**
   * @internal
   * @brief Get the clone allocator of the array.
   * @return Constant reference to the clone allocator.
   *
   * Version of @ref _get_clone_allocator() for const-qualified arrays,
   * returns a constant reference instead.
   */
  CloneAllocator const& _get_clone_allocator() const
  {
    return _impl;
  }

  /**
   * @internal
   */
  void _delete_clone(T const* p)
  {
    _get_clone_allocator().deallocate_clone(p);
  }

  /**
   * @internal
   */
  T* _new_clone(T const& a)
  {
    return _get_clone_allocator().allocate_clone(a);
  }

  /**
   * @internal
   */
  struct _ptr_array_impl : CloneAllocator
  {
    _ptr_array_impl() : _array( ::eina_array_new(32u) ) {}
    _ptr_array_impl(CloneAllocator allocator)
      : clone_allocator_type(allocator), _array( ::eina_array_new(32u)) {}

    Eina_Array* _array;
  };

  /**
   * @internal
   */
  _ptr_array_impl _impl;

private:
  /** Disabled copy constructor. */
  _ptr_array_common_base(_ptr_array_common_base const& other);
  /** Disabled assignment operator. */
  _ptr_array_common_base& operator=(_ptr_array_common_base const& other);
};

/**
 * Array class. It provides an OOP interface to the @c Eina_Array
 * functions, and automatically take care of allocating and deallocating
 * resources using the RAII programming idiom.
 *
 * It also provides additional member functions to facilitate the access
 * to the array content, much like a STL vector.
 */
template <typename T, typename CloneAllocator = heap_no_copy_allocator>
class ptr_array : protected _ptr_array_common_base<T, CloneAllocator>
{
  typedef _ptr_array_common_base<T, CloneAllocator> _base_type; /**< Type for the base class. */
public:
  typedef T value_type; /**< The type of each element. */
  typedef T& reference; /**< Type for a reference to an element. */
  typedef T const& const_reference; /**< Type for a constant reference to an element. */
  typedef _ptr_array_iterator<T const> const_iterator; /**< Type for a iterator for this container. */
  typedef _ptr_array_iterator<T> iterator; /**< Type for a constant iterator for this container. */
  typedef T* pointer; /**< Type for a pointer to an element. */
  typedef T const* const_pointer; /**< Type for a constant pointer for an element. */
  typedef std::size_t size_type; /**< Type for size information used in the array. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two iterators. */
  typedef CloneAllocator clone_allocator_type; /** Type for the clone allocator. */

  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for reverse iterator for this container. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /**< Type for reverse iterator for this container. */

  /**
   * @brief Default constructor. Create an empty array.
   *
   * This constructor creates a @c ptr_array object with no elements.
   */
  ptr_array() {}

  /**
   * @brief Construct an array object with @p n copies of @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   *
   * This constructor creates a @c ptr_array with @p n elements, each
   * one as a copy of @p t.
   */
  ptr_array(size_type n, const_reference t)
  {
    while(n--)
      push_back(t);
  }

  /**
   * @brief Create an array with elements from the given range.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   * @param alloc Clone allocator to be used.
   *
   * This constructor creates a @c ptr_array with copies of the elements
   * between @p i and @p j in the same order.
   *
   * @note The ending element (pointed by @p j) is not copied.
   */
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

  /**
   * @brief Copy constructor. Creates a copy of the given @c ptr_array.
   * @param other Another @c ptr_array of the same type.
   *
   * This constructor creates a @c ptr_array containing a copy of each
   * element inside @p other in the same order.
   */
  ptr_array(ptr_array<T, CloneAllocator> const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * @brief Creates a copy of a @c ptr_array with a different clone allocator.
   * @param other Another @c ptr_array with a different clone allocator.
   *
   * This constructor creates a @c ptr_array containing a copy of each
   * element inside @p other in the same order, even if the given array
   * uses a different clone allocator.
   */
  template <typename CloneAllocator1>
  ptr_array(ptr_array<T, CloneAllocator1>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * @brief Destructor. Release all allocated elements.
   */
  ~ptr_array()
  {
    clear();
  }

  /**
   * @brief Replace the current content with the content of another array.
   * @param other Another @c ptr_array of the same type.
   *
   * This assignment operator replaces the content of the array by a
   * copy of the content of @p other. The array size is adjusted
   * accordingly and the newly copied elements keep their original order.
   */
  ptr_array<T, CloneAllocator>& operator=(ptr_array<T, CloneAllocator>const& other)
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
    for(iterator first = begin(), last = end(); first != last; ++first)
      this->_delete_clone(&*first);
    eina_array_flush(this->_impl._array);
  }

  /**
   * @brief Get the current size of the array.
   * @return Number of elements in the array.
   *
   * This member function returns the current number of elements inside
   * the array.
   */
  std::size_t size() const
  {
    return eina_array_count(this->_impl._array);
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
    return size() == 0u;
  }

  /**
   * @brief Get the clone allocator of the array.
   * @return Reference to the clone allocator.
   */
  clone_allocator_type get_clone_allocator() const
  {
    return clone_allocator_type(this->_get_clone_allocator());
  }

  /**
   * @brief Add a copy of the given element at the end of the array.
   * @param a Element to be added at the end of the array.
   *
   * This member function allocates a new element, as a copy of @p a,
   * and inserts it at the end of the array.
   */
  void push_back(const_reference a)
  {
    push_back(this->_new_clone(a));
  }

  /**
   * @brief Add the object pointed by @p p as a element at the end of the array.
   * @param p Pointer to a pre-allocated element to be inserted at the end of the array.
   *
   * This member function adds the object pointed by @p p as a element
   * at the end of the array. The array gains ownership of the pointer
   * and nothing is copied.
   *
   * @warning The array gains ownership of the given pointer and will
   * release the pointed object upon element destruction.
   */
  void push_back(pointer p)
  {
    std::unique_ptr<value_type> p1(p);
    push_back(p1);
  }


  /**
   * @brief Add the object pointed by @p p as a element at the end of the array.
   * @param p Reference to a @c unique_ptr pointing to a element to be inserted at the end of the array.
   *
   * This member function adds the object pointed by the given
   * @c unique_ptr as a element at the end of the array. The object
   * ownership is transferred to the array and nothing is copied.
   *
   * @warning The array gains ownership of the object managed by the
   * given @c unique_ptr and will release it upon element destruction.
   */
  void push_back(std::unique_ptr<T>& p)
  {
    if(eina_array_push(this->_impl._array, p.get()))
      p.release();
    else
      throw std::bad_alloc();
  }

  /**
   * @brief Remove the last element of the array.
   */
  void pop_back()
  {
    eina_array_pop(this->_impl._array);
  }

  /**
   * @brief Insert a copy of the given element at the given position.
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
    return insert(i, this->_new_clone(t));
  }

  /**
   * @brief Insert the object pointed by @p pv as a element at the given position.
   * @param i Iterator pointing to the position where the new element will be inserted.
   * @param pv Pointer to a pre-allocated object to be inserted to the array.
   * @return Iterator pointing to the new element inserted.
   *
   * This member function adds the object pointed by @p pv as a element
   * at the given position. The new element comes right before the
   * element originally pointed by @p i. The array gains ownership of
   * the pointer and nothing is copied.
   *
   * At the end, a valid iterator pointing to the element just inserted
   * is returned.
   *
   * @warning The array gains ownership of the given pointer and will
   * release the pointed object upon element destruction.
   */
  iterator insert(iterator i, pointer pv)
  {
    std::unique_ptr<value_type> p(pv);
    return insert(i, p);
  }

  /**
   * @brief Insert the object pointed by @p p as a element at the given position.
   * @param i Iterator pointing to the position where the new element will be inserted.
   * @param p Reference to a @c unique_ptr pointing to the element to be inserted in the array.
   * @return Iterator pointing to the new element inserted.
   *
   * This member function adds the object pointed by @p p as a element
   * at the given position. The new element comes right before the
   * element originally pointed by @p i. The object ownership is
   * transferred to the array and nothing is copied.
   *
   * At the end, a valid iterator pointing to the element just inserted
   * is returned.
   *
   * @warning The array gains ownership of the object managed by the
   * given @c unique_ptr and will release it upon element destruction.
   */
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
    iterator r = i;
    if(n--)
      r = insert(i, t);
    while(n--)
      insert(i, t);
    return r;
  }

  /**
   * @brief Insert the object pointed by @p p and <tt>n-1</tt> copies of it as elements at the given position.
   * @param i Iterator pointing to the position where the new elements will be inserted.
   * @param n Number of elements to be inserted.
   * @param p Pointer to a pre-allocated object to be inserted in the array.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts the object pointed by @p p and
   * <tt>n-1</tt> copies of it as elements at the given position. The
   * new elements come right before the element originally pointed by
   * @p i.
   *
   * At the end, a valid iterator pointing to the first element inserted
   * is returned.
   *
   * @warning The array gains ownership of the given pointer and will
   * release the pointed object upon element destruction.
   */
  iterator insert(iterator i, size_t n, pointer p)
  {
    iterator r = i;
    if(n--)
      r = insert(i, p);
    while(n--)
      insert(i, this->_new_clone(p));
    return r;
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
    size_type index = p.native_handle() - this->_impl._array->data;
    while(i != j)
      {
        p = insert(p, this->_new_clone(*i));
        ++p;
        ++i;
      }
    return iterator(this->_impl._array->data + index);
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
    size_type size = this->size()
      , i = q.native_handle() - this->_impl._array->data;
    memmove(q.native_handle()
            , q.native_handle() + 1
            , (size - i - 1)*sizeof(void*));
    eina_array_pop(this->_impl._array);
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
    size_type size = this->size()
      , distance = std::distance(i, j);
    memmove(i.native_handle()
            , j.native_handle()
            , (size - distance)*sizeof(void*));
    while(distance--)
      eina_array_pop(this->_impl._array);
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
              , typename eina::enable_if<!eina::is_integral<InputIterator>::value>::type* = 0)
  {
    clear();
    insert(end(), i, j);
  }

  /**
   * @brief Replace the content of the array by @p n copies @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   */
  void assign(size_type n, value_type const& t)
  {
    clear();
    insert(end(), n, t);
  }

  /**
   * @brief Get a reference to the last element.
   * @return Reference to the last element in the array.
   */
  value_type& back()
  {
    return _ptr_array_access_traits::back<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant reference to the last element.
   * @return Constant reference to the last element in the array.
   *
   * Version of @ref back() for const-qualified objects. Returns a
   * constant reference instead.
   */
  value_type const& back() const
  {
    return _ptr_array_access_traits::back<T>(this->_impl._array);
  }

  /**
   * @brief Get a reference to the first element.
   * @return Reference to the first element of the array.
   */
  value_type& front()
  {
    return _ptr_array_access_traits::front<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant reference to the first element.
   * @return Constant reference to the first element of the array.
   *
   * Version of @ref front() for const-qualified objects. Returns a
   * constant reference instead.
   */
  value_type const& front() const
  {
    return _ptr_array_access_traits::front<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant reference to the element at the given position.
   * @param index Position of the element in the array.
   * @return Constant reference to element at position @p index.
   *
   * Version of @ref operator[](size_type index) for const-qualified
   * objects. Returns a constant reference instead.
   */
  const_reference operator[](size_type index) const
  {
    return _ptr_array_access_traits::index<T>(this->_impl._array, index);
  }

  /**
   * @brief Get a reference to the element at the given position.
   * @param index Position of the element in the array.
   * @return Reference to element at position @p index.
   */
  reference operator[](size_type index)
  {
    return _ptr_array_access_traits::index<T>(this->_impl._array, index);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the array.
   * @return Constant iterator to the initial position of the array.
   *
   * Version of @ref begin() for const-qualified objects. Returns a
   * constant iterator instead.
   */
  const_iterator begin() const
  {
    return _ptr_array_access_traits::begin<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the array.
   * @return Constant iterator to the final position of the array.
   *
   * Version of @ref end() for const-qualified objects. Returns a
   * constant iterator instead.
   */
  const_iterator end() const
  {
    return _ptr_array_access_traits::end<T>(this->_impl._array);
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
    return _ptr_array_access_traits::begin<T>(this->_impl._array);
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
    return _ptr_array_access_traits::end<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the array.
   * @return Constant reverse iterator pointing to the reverse begin of the array.
   *
   * Version of @ref rbegin() for const-qualified objects. Returns a
   * constant reverse iterator instead.
   */
  const_reverse_iterator rbegin() const
  {
    return _ptr_array_access_traits::rbegin<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the array.
   * @return Constant reverse iterator pointing to the reverse end of the array.
   *
   * Version of @ref rend() for const-qualified objects. Returns a
   * constant reverse iterator instead.
   */
  const_reverse_iterator rend() const
  {
    return _ptr_array_access_traits::rend<T>(this->_impl._array);
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
    return _ptr_array_access_traits::rbegin<T>(this->_impl._array);
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
    return _ptr_array_access_traits::rend<T>(this->_impl._array);
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
    return _ptr_array_access_traits::cbegin<T>(this->_impl._array);
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
    return _ptr_array_access_traits::cend<T>(this->_impl._array);
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
    return _ptr_array_access_traits::crbegin<T>(this->_impl._array);
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
    return _ptr_array_access_traits::crend<T>(this->_impl._array);
  }

  /**
   * @brief Get an <tt>eina::iterator</tt> pointing to the first element of the array.
   * @return <tt>eina::iterator</tt> to the initial position of the array.
   *
   * This member function returns an <tt>eina::iterator</tt> pointing to
   * the first element of the array. If the array is empty the returned
   * iterator is the same as the one returned by @ref iend().
   */
  eina::iterator<T> ibegin()
  {
    return _ptr_array_access_traits::ibegin<T>(this->_impl._array);
  }

  /**
   * @brief Get an <tt>eina::iterator</tt> to the position following the last element of the array.
   * @return <tt>eina::iterator</tt> to the final position of the array.
   *
   * This member function returns an <tt>eina::iterator</tt> to the
   * position following the last element in the array. If the array is
   * empty the returned iterator is the same as the one returned by
   * @ref ibegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  eina::iterator<T> iend()
  {
    return _ptr_array_access_traits::iend<T>(this->_impl._array);
  }

  /**
   * @brief Get a constant <tt>eina::iterator</tt> pointing to the first element of the array.
   * @return Constant <tt>eina::iterator</tt> to the initial position of the array.
   *
   * Version of @ref ibegin() for const-qualified objects. Returns a
   * constant <tt>eina::iterator</tt> instead.
   */
  eina::iterator<T const> ibegin() const
  {
    return _ptr_array_access_traits::ibegin<T>(this->_impl._array);
  }

  /**
   * @brief Get an constant <tt>eina::iterator</tt> to the position following the last element of the array.
   * @return Constant <tt>eina::iterator</tt> to the final position of the array.
   *
   * Version of @ref iend() for const-qualified objects. Returns a
   * constant <tt>eina::iterator</tt> instead.
   */
  eina::iterator<T const> iend() const
  {
    return _ptr_array_access_traits::iend<T>(this->_impl._array);
  }

  /**
   * @brief Get an <tt>eina::iterator</tt> pointing to the first element of the array.
   * @return <tt>eina::iterator</tt> to the initial position of the array.
   *
   * This member function works like @ref ibegin() const but is granted
   * to return a constant iterator even for arrays that are not
   * const-qualified.
   */
  eina::iterator<T const> cibegin() const
  {
    return _ptr_array_access_traits::cibegin<T>(this->_impl._array);
  }

  /**
   * @brief Get an constant <tt>eina::iterator</tt> to the position following the last element of the array.
   * @return Constant <tt>eina::iterator</tt> to the final position of the array.
   *
   * This member function works like @ref iend() const but is granted to
   * return a constant iterator even for arrays that are not
   * const-qualified.
   */
  eina::iterator<T const> ciend() const
  {
    return _ptr_array_access_traits::ciend<T>(this->_impl._array);
  }

  /**
   * @brief Swap content between two arrays.
   * @param other Other @c ptr_array of the same type.
   */
  void swap(ptr_array<T, CloneAllocator>& other)
  {
    std::swap(this->_impl._array, other._impl._array);
  }

  /**
   * @brief Get the maximum number of elements @c ptr_array can hold.
   * @return Maximum number of elements a @c ptr_array can hold.
   */
  size_type max_size() const { return -1; }

  /**
   * @brief Get a handle for the wrapped Eina_Array.
   * @return Handle for the native Eina array.
   *
   * This member function returns the native Eina_Array handle that is
   * wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  Eina_Array* native_handle()
  {
    return this->_impl._array;
  }

  /**
   * @brief Get a constant handle for the wrapped Eina_Array.
   * @return Constant handle for the native Eina array.
   *
   * Version of @ref native_handle() for const-qualified objects.
   * Returns a constant handle instead.
   *
   * @see native_handle()
   */
  Eina_Array const* native_handle() const
  {
    return this->_impl._array;
  }
};

/**
 * @brief Check if both arrays are equal.
 * @param lhs @c ptr_array at the left side of the expression.
 * @param rhs @c ptr_array at the right side of the expression.
 * @return @c true if the arrays are equals, @c false otherwise.
 *
 * This operator checks if the given arrays are equal. To be considered
 * equal both arrays need to have the same number of elements, and each
 * element in one array must be equal to the element at the same
 * position in the other array.
 */
template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator==(ptr_array<T, CloneAllocator1> const& lhs, ptr_array<T, CloneAllocator2> const& rhs)
{
  return lhs.size() == rhs.size()
    && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Check if two arrays are different.
 * @param lhs @c ptr_array at the left side of the expression.
 * @param rhs @c ptr_array at the right side of the expression.
 * @return @c true if the arrays are not equal , @c false otherwise.
 *
 * This operator returns the opposite of
 * @ref operator==(ptr_array<T, CloneAllocator1> const& lhs, ptr_array<T, CloneAllocator2> const& rhs).
 */
template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator!=(ptr_array<T, CloneAllocator1> const& lhs, ptr_array<T, CloneAllocator2> const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Swap content between two arrays.
 * @param other Other @c ptr_array of the same type.
 */
template <typename T, typename CloneAllocator>
void swap(ptr_array<T, CloneAllocator>& lhs, ptr_array<T, CloneAllocator>& rhs)
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
