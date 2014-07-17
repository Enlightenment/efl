#ifndef EINA_PTRLIST_HH_
#define EINA_PTRLIST_HH_

#include <Eina.h>
#include <eina_clone_allocators.hh>
#include <eina_lists_auxiliary.hh>
#include <eina_type_traits.hh>
#include <eina_accessor.hh>
#include <eina_eo_base_fwd.hh>
#include <eina_iterator.hh>

#include <memory>
#include <iterator>
#include <type_traits>

/**
 * @addtogroup Eina_Cxx_Containers_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Pointer_List_Group List of Pointers
 * @ingroup Eina_Cxx_Containers_Group
 *
 * @{
 */

/**
 * @internal
 */
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

/**
 * @internal
 */
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

/**
 * @internal
 */
struct _ptr_list_access_traits
{
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
    return std::reverse_iterator<_ptr_list_iterator<T> >(_ptr_list_access_traits::end<T>(list));
  }
  template <typename T>
  static std::reverse_iterator<_ptr_list_iterator<T> > rend(Eina_List* list)
  {
    return std::reverse_iterator<_ptr_list_iterator<T> >(_ptr_list_access_traits::begin<T>(list));
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
  static eina::iterator<T const> iend(Eina_List const*)
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

template <typename T, typename Allocator>
class ptr_list;

/**
 * @ingroup Eina_Cxx_Range_Group
 *
 * Range class for @ref ptr_list.
 */
template <typename T>
struct range_ptr_list : _range_template<T, _ptr_list_access_traits>
{
  typedef _range_template<T, _ptr_list_access_traits> _base_type; /**< Type for the base class. */
  typedef typename _base_type::value_type value_type; /**< The type of each element. */
  typedef typename _base_type::native_handle_type native_handle_type; /** Type for the native Eina list handle. */

  /**
   * @brief Creates a range from a native Eina list handle.
   */
  range_ptr_list(native_handle_type list)
    : _base_type(list)
  {}

  /**
   * @brief Creates a range from a @c ptr_list object.
   */
  template <typename Allocator>
  range_ptr_list(ptr_list<value_type, Allocator>& list)
    : _base_type(list.native_handle())
  {}
};

/**
 * Common implementations for @c ptr_list.
 */
template <typename T, typename CloneAllocator>
struct _ptr_list_common_base
{
  typedef CloneAllocator clone_allocator_type; /**< Type for the clone allocator. */

  /**
   * @brief Creates an list with the given clone allocator.
   */
  _ptr_list_common_base(CloneAllocator clone_allocator)
    : _impl(clone_allocator)
  {}

  /**
   * @brief Create a new object from a handle to a native Eina_List.
   * @param _list Handle to a native Eina_List.
   *
   * This constructor wraps a pre-allocated Eina_List providing an OOP
   * interface to it.
   *
   * @warning It is important to note that the created object gains
   * ownership of the handle, deallocating it at destruction time.
   */
  _ptr_list_common_base(Eina_List* _list)
    : _impl(_list)
  {}

  /**
   * @brief Default constructor. Create an empty list.
   *
   * This constructor creates a list with no elements.
   */
  _ptr_list_common_base() {}

  /**
   * @internal
   */
  CloneAllocator& _get_clone_allocator()
  {
    return _impl;
  }

  /**
   * @internal
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
  struct _ptr_list_impl : CloneAllocator
  {
    _ptr_list_impl() : _list(0) {}
    _ptr_list_impl(Eina_List* list) : _list(list) {}
    _ptr_list_impl(CloneAllocator allocator)
      : clone_allocator_type(allocator), _list(0) {}

    Eina_List* _list;
  };

  /**
   * @internal
   */
  _ptr_list_impl _impl;

private:
  /** Disabled copy constructor. */
  _ptr_list_common_base(_ptr_list_common_base const& other);
  /** Disabled assignment operator. */
  _ptr_list_common_base& operator=(_ptr_list_common_base const& other);
};

/**
 * List class. It provides an OOP interface to the @c Eina_List
 * functions, and automatically take care of allocating and deallocating
 * resources using the RAII programming idiom.
 *
 * It also provides additional member functions to facilitate the access
 * to the list content, much like a STL list.
 */
template <typename T, typename CloneAllocator = heap_no_copy_allocator>
class ptr_list : protected _ptr_list_common_base<T, CloneAllocator>
{
  typedef _ptr_list_common_base<T, CloneAllocator> _base_type; /**< Type for the base class. */
public:
  typedef T value_type; /**< The type of each element. */
  typedef T& reference; /**< Type for a reference to an element. */
  typedef T const& const_reference; /**< Type for a constant reference to an element. */
  typedef _ptr_list_iterator<T const> const_iterator; /**< Type for a iterator for this container. */
  typedef _ptr_list_iterator<T> iterator; /**< Type for a constant iterator for this container. */
  typedef T* pointer; /**< Type for a pointer to an element. */
  typedef T const* const_pointer; /**< Type for a constant pointer for an element. */
  typedef std::size_t size_type; /**< Type for size information. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two iterators. */
  typedef CloneAllocator clone_allocator_type; /** Type for the clone allocator. */

  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for reverse iterator for this container. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /**< Type for reverse iterator for this container. */

  typedef std::unique_ptr<value_type, clone_allocator_deleter<clone_allocator_type> > _unique_ptr;

  /**
   * @brief Default constructor. Create an empty list.
   *
   * This constructor creates a @c ptr_list object with no elements.
   */
  ptr_list() {}

  ptr_list(Eina_List* handle) : _base_type(handle) {}
  
  ptr_list(clone_allocator_type alloc)
    : _base_type(alloc)
  {}
  
  /**
   * @brief Construct an list object with @p n copies of @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   *
   * This constructor creates an @c ptr_list with @p n elements, each
   * one as a copy of @p t.
   */
  ptr_list(size_type n, const_reference t)
  {
    while(n--)
      push_back(t);
  }

  /**
   * @brief Create a list with elements from the given range.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   * @param alloc Clone allocator to be used.
   *
   * This constructor creates a @c ptr_list with copies of the elements
   * between @p i and @p j in the same order.
   *
   * @note The ending element (pointed by @p j) is not copied.
   */
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

  /**
   * @brief Copy constructor. Creates a copy of the given @c ptr_list.
   * @param other Another @c ptr_list of the same type.
   *
   * This constructor creates a @c ptr_list containing a copy of each
   * element inside @p other in the same order.
   */
  ptr_list(ptr_list<T, CloneAllocator> const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * @brief Creates a copy of a @c ptr_list with a different clone allocator.
   * @param other Another @c ptr_list with a different clone allocator.
   *
   * This constructor creates a @c ptr_list containing a copy of each
   * element inside @p other in the same order, even if the given list
   * uses a different clone allocator.
   */
  template <typename CloneAllocator1>
  ptr_list(ptr_list<T, CloneAllocator1>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * @brief Destructor. Release all allocated elements.
   */
  ~ptr_list()
  {
    clear();
  }

  /**
   * @brief Replace the current content with the cotent of another list.
   * @param other Another @c ptr_list of the same type.
   *
   * This assignment operator replaces the content of the list by a copy
   * of the content of @p other. The list size is adjusted accordingly
   * and the newly copied elements keep their original order.
   */
  ptr_list<T, CloneAllocator>& operator=(ptr_list<T, CloneAllocator>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }

  /**
   * @brief Remove all the elements of the list.
   */
  void clear()
  {
    for(iterator first = begin(), last = end(); first != last; ++first)
      this->_delete_clone(&*first);
    eina_list_free(this->_impl._list);
    this->_impl._list = 0;
  }

  /**
   * @brief Get the current size of the list.
   * @return Number of elements in the list.
   *
   * This member function returns the current number of elements inside
   * the list.
   */
  std::size_t size() const
  {
    return _ptr_list_access_traits::size<T>(this->_impl._list);
  }

  /**
   * @brief Check if the list is empty.
   * @return @c true if the list is empty, @c false otherwise.
   *
   * This member function returns @c true if the list does not contain
   * any elements, otherwise it returns @c false.
   */
  bool empty() const
  {
    return _ptr_list_access_traits::empty<T>(this->_impl._list);
  }

  /**
   * @brief Get the clone allocator of the list.
   * @return Reference to the clone allocator.
   */
  clone_allocator_type get_clone_allocator() const
  {
    return clone_allocator_type(this->_get_clone_allocator());
  }

  /**
   * @brief Add a copy of the given element at the end of the list.
   * @param a Element to be added at the end of the list.
   *
   * This member function allocates a new element, as a copy of @p a,
   * and inserts it at the end of the list.
   */
  void push_back(const_reference a)
  {
    push_back(this->_new_clone(a));
  }

  /**
   * @brief Add the object pointed by @p p as a element at the end of the list.
   * @param p Pointer to a pre-allocated element to be inserted at the end of the list.
   *
   * This member function adds the object pointed by @p p as a element
   * at the end of the list. The list gains ownership of the pointer and
   * nothing is copied.
   *
   * @warning The list gains ownership of the given pointer and will
   * release the pointed object upon element destruction.
   */
  void push_back(pointer p)
  {
    _unique_ptr p1(p);
    push_back(p1);
  }

  /**
   * @brief Add the object pointed by @p p as a element at the end of the list.
   * @param p Reference to a @c unique_ptr pointing to a element to be inserted at the end of the list.
   *
   * This member function adds the object pointed by the given
   * @c unique_ptr as a element at the end of the list. The object
   * ownership is transfered to the list and nothing is copied.
   *
   * @warning The list gains ownership of the object managed by the
   * given @c unique_ptr and will release it upon element destruction.
   */
  template <typename Deleter>
  void push_back(std::unique_ptr<T, Deleter>& p)
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

  /**
   * @brief Add a copy of the given element at the beginning of the list.
   * @param a Element to be added at the beginning of the list.
   *
   * This member function allocates a new element, as a copy of @p a,
   * and inserts it at the beginning of the list.
   */
  void push_front(const_reference a)
  {
    push_front(this->_new_clone(a));
  }

  /**
   * @brief Add the object pointed by @p p as a element at the beginning of the list.
   * @param p Pointer to a pre-allocated element to be inserted at the beginning of the list.
   *
   * This member function adds the object pointed by @p p as a element
   * at the beginning of the list. The list gains ownership of the
   * pointer and nothing is copied.
   *
   * @warning The list gains ownership of the given pointer and will
   * release the pointed object upon element destruction.
   */
  void push_front(pointer p)
  {
    _unique_ptr p1(p);
    push_front(p1);
  }

  /**
   * @brief Add the object pointed by @p p as a element at the beginning of the list.
   * @param p Reference to a @c unique_ptr pointing to a element to be inserted at the beginning of the list.
   *
   * This member function adds the object pointed by the given
   * @c unique_ptr as a element at the beginning of the list. The object
   * ownership is transfered to the list and nothing is copied.
   *
   * @warning The list gains ownership of the object managed by the
   * given @c unique_ptr and will release it upon element destruction.
   */
  template <typename Deleter>
  void push_front(std::unique_ptr<T, Deleter>& p)
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

  /**
   * @brief Remove the last element of the list.
   */
  void pop_back()
  {
    this->_impl._list = eina_list_remove_list(this->_impl._list, eina_list_last(this->_impl._list));
  }

  /**
   * @brief Remove the first element of the list.
   */
  void pop_front()
  {
    this->_impl._list = eina_list_remove_list(this->_impl._list, this->_impl._list);
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
   * @param pv Pointer to a pre-allocated object to be inserted to the list.
   * @return Iterator pointing to the new element inserted.
   *
   * This member function adds the object pointed by @p pv as a element
   * at the given position. The new element comes right before the
   * element originally pointed by @p i. The list gains ownership of
   * the pointer and nothing is copied.
   *
   * At the end, a valid iterator pointing to the element just inserted
   * is returned.
   *
   * @warning The list gains ownership of the given pointer and will
   * release the pointed object upon element destruction.
   */
  iterator insert(iterator i, pointer pv)
  {
    _unique_ptr p(pv);
    return insert(i, p);
  }

  /**
   * @brief Insert the object pointed by @p p as a element at the given position.
   * @param i Iterator pointing to the position where the new element will be inserted.
   * @param p Reference to a @c unique_ptr pointing to the element to be inserted in the list.
   * @return Iterator pointing to the new element inserted.
   *
   * This member function adds the object pointed by @p p as a element
   * at the given position. The new element comes right before the
   * element originally pointed by @p i. The object ownership is
   * transfered to the list and nothing is copied.
   *
   * At the end, a valid iterator pointing to the element just inserted
   * is returned.
   *
   * @warning The list gains ownership of the object managed by the
   * given @c unique_ptr and will release it upon element destruction.
   */
  template <typename Deleter>
  iterator insert(iterator i, std::unique_ptr<value_type, Deleter>& p)
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

  /**
   * @brief Insert @p n copies of @p t at the given position.
   * @param i Iterator pointing to the position where the new elements will be inserted.
   * @param n Number of elements to be inserted.
   * @param t Value to be copied to each new inserted element.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts @p n new elements at position @p i
   * in the list, each one as a copy of @p t. The new elements come
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
   * @param p Pointer to a pre-allocated object to be inserted in the list.
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
   * @warning The list gains ownership of the given pointer and will
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
    iterator r = p;
    if(i != j)
      {
        r = insert(p, this->_new_clone(*i));
        ++i;
      }
    while(i != j)
      {
        insert(p, this->_new_clone(*i));
        ++i;
      }
    return r;
  }

  /**
   * @brief Remove the element at the given position.
   * @param q Iterator pointing to the element to be removed.
   * @return Iterator pointing to the element after the removed one.
   *
   * This member function removes the element pointed by the iterator
   * @p q, reducing the list size by one. At the end, a valid iterator
   * pointing to the element right after the removed one is returned.
   */
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
      i = erase(i);
    if(j.native_handle())
      return j;
    else
      return end();
  }

  /**
   * @brief Replace the content of the list by the elements in the given range.
   * @param i Iterator pointing to the beginning of the elements to be copied.
   * @param j Iterator pointing to the end of the elements to be copied.
   *          Note that the element pointed by j will NOT be copied.
   *
   * This member function replaces the current elements by copies of the
   * elements between the iterators @p i and @p j, including the element
   * pointed by @p i but not the one pointed by @p j. The size of the
   * list is adjusted accordingly and the newly copied elements remain
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
   * @brief Replace the content of the list by @p n copies @p t.
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
   * @return Reference to the last element in the list.
   */
  value_type& back()
  {
    return _ptr_list_access_traits::back<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant reference to the last element.
   * @return Constant reference to the last element in the list.
   *
   * Version of @ref back() for const-qualified objects. Returns a
   * constant reference instead.
   */
  value_type const& back() const
  {
    return _ptr_list_access_traits::back<T>(this->_impl._list);
  }

  /**
   * @brief Get a reference to the first element.
   * @return Reference to the first element of the list.
   */
  value_type& front()
  {
    return _ptr_list_access_traits::front<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant reference to the first element.
   * @return Constant reference to the first element of the list.
   *
   * Version of @ref front() for const-qualified objects. Returns a
   * constant reference instead.
   */
  value_type const& front() const
  {
    return _ptr_list_access_traits::front<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the list.
   * @return Constant iterator to the initial position of the list.
   *
   * Version of @ref begin() for const-qualified objects. Returns a
   * constant iterator instead.
   */
  const_iterator begin() const
  {
    return _ptr_list_access_traits::cbegin<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the list.
   * @return Constant iterator to the final position of the list.
   *
   * Version of @ref end() for const-qualified objects. Returns a
   * constant iterator instead.
   */
  const_iterator end() const
  {
    return _ptr_list_access_traits::cend<T>(this->_impl._list);
  }

  /**
   * @brief Get an iterator pointing to the first element of the list.
   * @return Iterator to the initial position of the list.
   *
   * This member function returns an iterator pointing to the first
   * element of the list. If the list is empty the returned iterator
   * is the same as the one returned by @ref end().
   */
  iterator begin()
  {
    return _ptr_list_access_traits::begin<T>(this->_impl._list);
  }

  /**
   * @brief Get an iterator to the position following the last element of the list.
   * @return Iterator to the final position of the list.
   *
   * This member function returns an iterator to the position following
   * the last element in the list. If the list is empty the returned
   * iterator is the same as the one returned by @ref begin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  iterator end()
  {
    return _ptr_list_access_traits::end<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the list.
   * @return Constant reverse iterator pointing to the reverse begin of the list.
   *
   * Version of @ref rbegin() for const-qualified objects. Returns a
   * constant reverse iterator instead.
   */
  const_reverse_iterator rbegin() const
  {
    return _ptr_list_access_traits::rbegin<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the list.
   * @return Constant reverse iterator pointing to the reverse end of the list.
   *
   * Version of @ref rend() for const-qualified objects. Returns a
   * constant reverse iterator instead.
   */
  const_reverse_iterator rend() const
  {
    return _ptr_list_access_traits::rend<T>(this->_impl._list);
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse begin of the list.
   * @return Reverse iterator pointing to the reverse begin of the list.
   *
   * This member function returns a reverse iterator pointing to the
   * last element of the list. If the list is empty the returned
   * reverse iterator is the same as the one returned by @ref rend().
   */
  reverse_iterator rbegin()
  {
    return _ptr_list_access_traits::rbegin<T>(this->_impl._list);
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse end of the list.
   * @return Reverse iterator pointing to the reverse end of the list.
   *
   * This member function returns a reverse iterator pointing to the
   * position before the first element of the list. If the list is
   * empty the returned iterator is the same as the one returned by
   * @ref rbegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  reverse_iterator rend()
  {
    return _ptr_list_access_traits::rend<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the list.
   * @return Constant iterator to the initial position of the list.
   *
   * This member function works like @ref begin() const but is granted
   * to return a constant iterator even for lists that are not
   * const-qualified.
   */
  const_iterator cbegin() const
  {
    return _ptr_list_access_traits::cbegin<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the list.
   * @return Constant iterator to the final position of the list.
   *
   * This member function works like @ref end() const  but is granted to
   * return a constant iterator even for lists that are not
   * const-qualified.
   */
  const_iterator cend() const
  {
    return _ptr_list_access_traits::cend<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the list.
   * @return Constant reverse iterator pointing to the reverse begin of the list.
   *
   * This member function works like @ref rbegin() const but is granted
   * to return a constant reverse iterator even for lists that are not
   * const-qualified.
   */
  const_reverse_iterator crbegin() const
  {
    return _ptr_list_access_traits::crbegin<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the list.
   * @return Constant reverse iterator pointing to the reverse end of the list.
   *
   * This member function works like @ref rend() const but is granted to
   * return a constant reverse iterator even for lists that are not
   * const-qualified.
   */
  const_reverse_iterator crend() const
  {
    return _ptr_list_access_traits::crend<T>(this->_impl._list);
  }

  /**
   * @brief Get an <tt>eina::iterator</tt> pointing to the first element of the list.
   * @return <tt>eina::iterator</tt> to the initial position of the list.
   *
   * This member function returns an <tt>eina::iterator</tt> pointing to
   * the first element of the list. If the list is empty the returned
   * iterator is the same as the one returned by @ref iend().
   */
  eina::iterator<T> ibegin()
  {
    return _ptr_list_access_traits::ibegin<T>(this->_impl._list);
  }

  /**
   * @brief Get an <tt>eina::iterator</tt> to the position following the last element of the list.
   * @return <tt>eina::iterator</tt> to the final position of the list.
   *
   * This member function returns an <tt>eina::iterator</tt> to the
   * position following the last element in the list. If the list is
   * empty the returned iterator is the same as the one returned by
   * @ref ibegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  eina::iterator<T> iend()
  {
    return _ptr_list_access_traits::iend<T>(this->_impl._list);
  }

  /**
   * @brief Get a constant <tt>eina::iterator</tt> pointing to the first element of the list.
   * @return Constant <tt>eina::iterator</tt> to the initial position of the list.
   *
   * Version of @ref ibegin() for const-qualified objects. Returns a
   * constant <tt>eina::iterator</tt> instead.
   */
  eina::iterator<T const> ibegin() const
  {
    return _ptr_list_access_traits::ibegin<T>(this->_impl._list);
  }

  /**
   * @brief Get an constant <tt>eina::iterator</tt> to the position following the last element of the list.
   * @return Constant <tt>eina::iterator</tt> to the final position of the list.
   *
   * Version of @ref iend() for const-qualified objects. Returns a
   * constant <tt>eina::iterator</tt> instead.
   */
  eina::iterator<T const> iend() const
  {
    return _ptr_list_access_traits::iend<T>(this->_impl._list);
  }

  /**
   * @brief Get an <tt>eina::iterator</tt> pointing to the first element of the list.
   * @return <tt>eina::iterator</tt> to the initial position of the list.
   *
   * This member function works like @ref ibegin() const but is granted
   * to return a constant iterator even for lists that are not
   * const-qualified.
   */
  eina::iterator<T const> cibegin() const
  {
    return _ptr_list_access_traits::cibegin<T>(this->_impl._list);
  }

  /**
   * @brief Get an constant <tt>eina::iterator</tt> to the position following the last element of the list.
   * @return Constant <tt>eina::iterator</tt> to the final position of the list.
   *
   * This member function works like @ref iend() const but is granted to
   * return a constant iterator even for lists that are not
   * const-qualified.
   */
  eina::iterator<T const> ciend() const
  {
    return _ptr_list_access_traits::ciend<T>(this->_impl._list);
  }

  /**
   * @brief Swap content between two lists.
   * @param other Other @c ptr_list of the same type.
   */
  void swap(ptr_list<T, CloneAllocator>& other)
  {
    std::swap(this->_impl._list, other._impl._list);
  }

  /**
   * @brief Get the maximum number of elements @c ptr_list can hold.
   * @return Maximum number of elements a @c ptr_list can hold.
   */
  size_type max_size() const { return -1; }

  /**
   * @brief Get a handle for the wrapped @c Eina_List.
   * @return Handle for the native Eina list.
   *
   * This member function returns the native Eina_List handle that is
   * wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically released upon object destruction.
   */
  Eina_List* native_handle()
  {
    return this->_impl._list;
  }

  /**
   * @brief Get a constant handle for the wrapped Eina_List.
   * @return Constant handle for the native Eina list.
   *
   * Version of @ref native_handle() for const-qualified objects.
   * Returns a constant handle instead.
   *
   * @see native_handle()
   */
  Eina_List const* native_handle() const
  {
    return this->_impl._list;
  }

  /**
   * @brief Get a constant @ref eina::accessor for the list.
   * @return Constant <tt>eina::accessor</tt> to the list.
   *
   * Version of @ref accessor() to const-qualified inline lists. Returns
   * a const-qualified <tt>eina::accessor</tt> instead.
   */
  eina::accessor<T const> accessor() const
  {
    return eina::accessor<T const>(eina_list_accessor_new(this->_impl._list));
  }

  /**
   * @brief Get a @ref eina::accessor for the list.
   * @return <tt>eina::accessor</tt> to the list.
   */
  eina::accessor<T> accessor()
  {
    return eina::accessor<T>(eina_list_accessor_new(this->_impl._list));
  }
};

/**
 * @brief Check if both lists are equal.
 * @param lhs @c ptr_list at the left side of the expression.
 * @param rhs @c ptr_list at the right side of the expression.
 * @return @c true if the lists are equals, @c false otherwise.
 *
 * This operator checks if the given lists are equal. To be considered
 * equal both lists need to have the same number of elements, and each
 * element in one list must be equal to the element at the same
 * position in the other list.
 */
template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator==(ptr_list<T, CloneAllocator1> const& lhs, ptr_list<T, CloneAllocator2> const& rhs)
{
  return lhs.size() == rhs.size()
    && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Check if two lists are different.
 * @param lhs @c ptr_list at the left side of the expression.
 * @param rhs @c ptr_list at the right side of the expression.
 * @return @c true if the lists are not equal , @c false otherwise.
 *
 * This operator returns the opposite of
 * @ref operator==(ptr_list<T, CloneAllocator1> const& lhs, ptr_list<T, CloneAllocator2> const& rhs).
 */
template <typename T, typename CloneAllocator1, typename CloneAllocator2>
bool operator!=(ptr_list<T, CloneAllocator1> const& lhs, ptr_list<T, CloneAllocator2> const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Swap content between two lists.
 * @param other Other @c ptr_list of the same type.
 */
template <typename T, typename CloneAllocator>
void swap(ptr_list<T, CloneAllocator>& lhs, ptr_list<T, CloneAllocator>& rhs)
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
