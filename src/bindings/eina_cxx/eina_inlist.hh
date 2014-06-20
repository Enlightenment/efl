#ifndef EINA_INLIST_HH_
#define EINA_INLIST_HH_

#include <Eina.h>
#include <eina_lists_auxiliary.hh>
#include <eina_type_traits.hh>
#include <eina_accessor.hh>
#include <eina_range_types.hh>

#include <iterator>
#include <algorithm>

/**
 * @addtogroup Eina_Cxx_Containers_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Inline_List_Group Inline List
 * @ingroup Eina_Cxx_Containers_Group
 *
 * @{
 */

/**
 * @internal
 */
template <typename T>
struct _inlist_node
{
  EINA_INLIST;
  T object;
};

/**
 * @internal
 */
template <typename T>
_inlist_node<T>* _get_node(Eina_Inlist* l)
{
  return static_cast<_inlist_node<T>*>(static_cast<void*>(l));
}

/**
 * @internal
 */
template <typename T>
_inlist_node<T> const* _get_node(Eina_Inlist const* l)
{
  return const_cast<Eina_Inlist*>(l);
}

/**
 * @internal
 */
template <typename T>
Eina_Inlist* _get_list(_inlist_node<T>* n)
{
  if(n)
    return EINA_INLIST_GET(n);
  else
    return 0;
}

/**
 * @internal
 */
template <typename T>
Eina_Inlist const* _get_list(_inlist_node<T> const* n)
{
  return _get_list(const_cast<_inlist_node<T>*>(n));
}

/**
 * @internal
 * Iterator for Inline List
 */
template <typename T>
struct _inlist_iterator
{
  typedef typename std::remove_const<T>::type value_type; /**< Type for the list elements. */
  typedef value_type* pointer; /**< Type for a pointer to an element. */
  typedef value_type& reference; /**< Type for a reference to an element. */
  typedef std::ptrdiff_t difference_type;  /**< Type to represent the distance between two iterators. */
  typedef std::bidirectional_iterator_tag iterator_category; /**< Defines the iterator as being a bidirectional iterator. */

  /**
   * @brief Default constructor. Creates an uninitialized iterator.
   */
  _inlist_iterator() {}

  /**
   * @brief Creates an iterator from a inline list and a node.
   * @param list Pointer to the inline list.
   * @param node Pointer to the node.
   */
  explicit _inlist_iterator(_inlist_node<value_type>* list, _inlist_node<value_type>* node)
    : _list(list), _node(node) {}

  /**
   * @brief Copy constructor. Creates a copy of the given iterator.
   * @param other Other iterator.
   */
  _inlist_iterator(_inlist_iterator<typename std::remove_const<T>::type> const& other)
    : _list(other._list), _node(other._node) {}

  /**
   * @brief Move the iterator to the next position in the list.
   * @return The iterator itself.
   *
   * This operator increments the iterator, making it point to the
   * position right after the current one.
   * At the end, it returns a reference to itself.
   */
  _inlist_iterator<T>& operator++()
  {
    _node = _get_node<value_type>(_node->__in_list.next);
    return *this;
  }

  /**
   * @brief Move the iterator to the next position in the list.
   * @return Copy of the iterator before the increment.
   *
   * This operator increments the iterator, making it point to the next
   * position right after the current one.
   * At the end, it returns a copy of the iterator before the increment.
   */
  _inlist_iterator<T> operator++(int)
  {
    _inlist_iterator<T> tmp(*this);
    ++*this;
    return tmp;
  }

  /**
   * @brief Move the iterator to the previous position in the list.
   * @return The iterator itself.
   *
   * This operator decrements the iterator, making it point to the
   * position right before the current one.
   * At the end, it returns a reference to itself.
   */
  _inlist_iterator<T>& operator--()
  {
    if(_node)
      _node = _get_node<value_type>(_node->__in_list.prev);
    else
      _node = _get_node<value_type>(_list->__in_list.last);
    return *this;
  }

  /**
   * @brief Move the iterator to the previous position in the list.
   * @return Copy of the iterator before the decrement.
   *
   * This operator decrements the iterator, making it point to the
   * position right before the current one.
   * At the end, it returns a copy of the iterator before the decrement.
   */
  _inlist_iterator<T> operator--(int)
  {
    _inlist_iterator<T> tmp(*this);
    --*this;
    return tmp;
  }

  /**
   * @brief Get a reference to the element currently pointed by the iterator.
   * @return Reference to the current element.
   */
  T& operator*() const
  {
    return _node->object;
  }

  /**
   * @brief Return a pointer to the current element, which member will be accessed.
   * @return Pointer to the element currently pointed by the iterator.
   */
  T* operator->() const
  {
    return &_node->object;
  }

  /**
   * @internal
   */
  _inlist_node<value_type>* native_handle()
  {
    return _node;
  }

  /**
   * @internal
   */
  _inlist_node<value_type> const* native_handle() const
  {
    return _node;
  }
private:
  _inlist_node<value_type>* _list; /**< Handle to the original list.  */
  _inlist_node<value_type>* _node; /**< Handle to the current node.  */

  /**
   * @brief Check if both iterators are pointing to the same node.
   * @param lhs First iterator to be compared.
   * @param rhs Second iterator to be compared.
   * @return @c true if both iterators are pointing to the same node, @c false otherwise.
   */
  template <typename U>
  friend struct _inlist_iterator;
  friend bool operator==(_inlist_iterator<T> lhs, _inlist_iterator<T> rhs)
  {
    return lhs._node == rhs._node;
  }
};

/**
 * @brief Check if iterators are not pointing to the same node.
 * @param lhs First iterator to be compared.
 * @param rhs Second iterator to be compared.
 * @return @c true if iterators are not pointing to the same node, @c false otherwise.
 */
template <typename T>
bool operator!=(_inlist_iterator<T> lhs, _inlist_iterator<T> rhs)
{
  return !(lhs == rhs);
}

/**
 * @internal
 */
struct _inlist_access_traits {
  template <typename T>
  struct const_iterator
  {
    typedef _inlist_iterator<T const> type;
  };
  template <typename T>
  struct iterator
  {
    typedef _inlist_iterator<T> type;
  };
  template <typename T>
  struct const_native_handle
  {
    typedef Eina_Inlist const* type;
  };
  template <typename T>
  struct native_handle
  {
    typedef Eina_Inlist* type;
  };

  template <typename T>
  static Eina_Inlist* native_handle_from_const(Eina_Inlist const* list)
  {
    return const_cast<Eina_Inlist*>(list);
  }
  template <typename T>
  static std::size_t size(Eina_Inlist const* list)
  {
    return ::eina_inlist_count(list);
  }
  template <typename T>
  static bool empty(Eina_Inlist const* list)
  {
    return list == 0;
  }
  template <typename T>
  static T& back(Eina_Inlist* list)
  {
    return _get_node<T>(list->last)->object;
  }
  template <typename T>
  static T const& back(Eina_Inlist const* list)
  {
    return _inlist_access_traits::back<T>(const_cast<Eina_Inlist*>(list));
  }
  template <typename T>
  static T& front(Eina_Inlist* list)
  {
    return _get_node<T>(list)->object;
  }
  template <typename T>
  static T const& front(Eina_Inlist const* list)
  {
    return _inlist_access_traits::front<T>(const_cast<Eina_Inlist*>(list));
  }
  template <typename T>
  static _inlist_iterator<T const> begin(Eina_Inlist const* list)
  {
    return _inlist_access_traits::begin<T>(const_cast<Eina_Inlist*>(list));
  }
  template <typename T>
  static _inlist_iterator<T const> end(Eina_Inlist const* list)
  {
    return _inlist_access_traits::end<T>(const_cast<Eina_Inlist*>(list));
  }
  template <typename T>
  static _inlist_iterator<T> begin(Eina_Inlist* list)
  {
    return _inlist_iterator<T>(_get_node<T>(list), _get_node<T>(list));
  }
  template <typename T>
  static _inlist_iterator<T> end(Eina_Inlist* list)
  {
    return _inlist_iterator<T>(_get_node<T>(list), 0);
  }
  template <typename T>
  static std::reverse_iterator<_inlist_iterator<T> > rbegin(Eina_Inlist* list)
  {
    return std::reverse_iterator<_inlist_iterator<T> >(_inlist_access_traits::end<T>(list));
  }
  template <typename T>
  static std::reverse_iterator<_inlist_iterator<T> > rend(Eina_Inlist* list)
  {
    return std::reverse_iterator<_inlist_iterator<T> >(_inlist_access_traits::begin<T>(list));
  }
  template <typename T>
  static std::reverse_iterator<_inlist_iterator<T const> > rbegin(Eina_Inlist const* list)
  {
    return _inlist_access_traits::rbegin<T>(const_cast<Eina_Inlist*>(list));
  }
  template <typename T>
  static std::reverse_iterator<_inlist_iterator<T const> > rend(Eina_Inlist const* list)
  {
    return _inlist_access_traits::rend<T>(const_cast<Eina_Inlist*>(list));
  }
  template <typename T>
  static _inlist_iterator<T const> cbegin(Eina_Inlist const* list)
  {
    return _inlist_access_traits::begin<T>(list);
  }
  template <typename T>
  static _inlist_iterator<T const> cend(Eina_Inlist const* list)
  {
    return _inlist_access_traits::end<T>(list);
  }
  template <typename T>
  static std::reverse_iterator<_inlist_iterator<T const> > crbegin(Eina_Inlist const* list)
  {
    return _inlist_access_traits::rbegin<T>(list);
  }
  template <typename T>
  static std::reverse_iterator<_inlist_iterator<T const> > crend(Eina_Inlist const* list)
  {
    return _inlist_access_traits::rend<T>(list);
  }
};

template <typename T, typename Allocator>
class inlist;

/**
 * @ingroup Eina_Cxx_Range_Group
 *
 * Range for inline list elements.
 */
template <typename T>
struct range_inlist : _range_template<T, _inlist_access_traits>
{
  typedef _range_template<T, _inlist_access_traits> _base_type; /**< Type for the base class. */
  typedef typename _base_type::value_type value_type; /**< The type of each element. */
  typedef typename _base_type::native_handle_type native_handle_type; /** Type for the native Eina inline list handle. */

  /**
   * @brief Creates a range from a native Eina inline list handle.
   */
  range_inlist(native_handle_type list)
    : _base_type(list) {}

  /**
   * @brief Creates a range from a inline list object.
   */
  template <typename Allocator>
  range_inlist(inlist<value_type, Allocator>& list)
    : _base_type(list.native_handle())
  {}
};

/**
 * @brief Check the given ranges are equal to each other.
 * @param lhs Range object at the left side of the expression.
 * @param rhs Range object at the right side of the expression.
 * @return @c true if the ranges are equal, @c false otherwise.
 *
 * This operator checks if the given ranges are equal to each other. To
 * be considered equal both ranges need to have the same size, and each
 * element in one range must be equal to the element at the same
 * position in the other.
 */
template <typename T>
bool operator==(range_inlist<T>const& lhs, range_inlist<T>const& rhs)
{
  return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Returns the opposite of @ref operator==(range_inlist<T>const& lhs, range_inlist<T>const& rhs).
 */
template <typename U>
bool operator!=(range_inlist<U> const& lhs, range_inlist<U>const& rhs)
{
  return !(lhs == rhs);
}

/**
 * Common implementations for inline list.
 */
template <typename T, typename Allocator>
struct _inlist_common_base
{
  typedef typename Allocator::template rebind<_inlist_node<T> >::other node_allocator_type; /**< Type for the allocator of the node. */
  typedef Allocator allocator_type; /**< Type for the allocator. */
  typedef _inlist_node<T> node_type; /**< Type for the list node.  */
  typedef Eina_Inlist* native_handle_type; /**< Native type. */

  /**
   * @brief Creates a list with the given allocator.
   * @param allocator Allocator object.
   */
  _inlist_common_base(Allocator allocator)
    : _impl(allocator) {}

  /**
   * @brief Wraps the native object.
   * @param inlist The native inlist object (Eina_Inlist*).
   */
  _inlist_common_base(native_handle_type inlist)
    : _impl(inlist) {}

  /**
   * @brief Creates an empty inline list.
   */
  _inlist_common_base()
  {}

  /**
   * @brief Destructor. Deallocate all nodes of the list.
   */
  ~_inlist_common_base()
  {
    clear();
  }

  /**
   * @brief Deallocate all nodes of the list.
   */
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

  /**
   * @brief Get the allocator used by the list.
   */
  node_allocator_type& get_node_allocator()
  {
    return _impl;
  }

  /**
   * @internal
   */
  // For EBO
  struct _inlist_impl : node_allocator_type
  {
    _inlist_impl(Allocator allocator)
      : node_allocator_type(allocator), _list(0)
    {}
     explicit _inlist_impl(native_handle_type list)
       : _list(list)
     {}
    _inlist_impl() : _list(0) {}
    native_handle_type _list;
  };

  _inlist_impl _impl; /**< @internal */
private:

  /** Disabled copy constructor. */
  _inlist_common_base(_inlist_common_base const& other);

  /** Disabled assignment operator. */
  _inlist_common_base& operator=(_inlist_common_base const& other);
};

/**
 * C++ wrapper for the native Eina inline list.
 *
 * It provides an OOP interface to the @c Eina_Inlist functions, and
 * automatically take care of allocating and deallocating resources using
 * the RAII programming idiom.
 *
 * It also provides additional member functions to facilitate the access
 * to the list content, much like a STL list.
 */
template <typename T, typename Allocator = std::allocator<T> >
class inlist : protected _inlist_common_base<T, Allocator>
{
  typedef _inlist_common_base<T, Allocator> _base_type; /**< Type for the base class. */
  typedef typename _base_type::node_type _node_type; /**< Type for each node */
public:
  typedef typename _base_type::allocator_type allocator_type; /**< Type for the allocator. */
  typedef typename allocator_type::value_type value_type; /**< The type of each element. */
  typedef typename allocator_type::reference reference; /**< Type for a reference to an element. */
  typedef typename allocator_type::const_reference const_reference; /**< Type for a constant reference to an element. */
  typedef _inlist_iterator<T const> const_iterator; /**< Type for constant iterator for this kind of container. */
  typedef _inlist_iterator<T> iterator; /**< Type for iterator for this kind of container. */
  typedef typename allocator_type::pointer pointer; /**< Type for a pointer to an element. */
  typedef typename allocator_type::const_pointer const_pointer; /**< Type for a constant pointer for an element. */
  typedef std::size_t size_type; /**< Type for size information. */
  typedef std::ptrdiff_t difference_type; /**< Type to represent the distance between two iterators. */
  typedef typename _base_type::native_handle_type native_handle_type; /**< The native handle type. */

  typedef std::reverse_iterator<iterator> reverse_iterator; /**< Type for reverse iterator for this kind of container. */
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator; /**< Type for constant reverse iterator for this kind of container. */

  using _base_type::clear;

  /**
   * @brief Default constructor. Creates an empty inline list.
   */
  inlist() {}

  /**
   * @brief Construct an inlist from a native object.
   * @param list The native object.
   */
  inlist(native_handle_type list)
    : _inlist_common_base<T, Allocator>(list)
  {}

  /**
   * @brief Construct an inline list object with @p n copies of @p t.
   * @param n Number of elements.
   * @param t Value to be copied to each element.
   *
   * This constructor creates an inline list with @p n elements, each
   * one as a copy of @p t.
   */
  inlist(size_type n, value_type const& t)
  {
    while(n--)
      push_back(t);
  }

  /**
   * @brief Create a inline list coping the elements from the given range.
   * @param i Iterator to the initial position. The element pointed by this iterator will be copied.
   * @param j Iterator to the final position. The element pointed by this iterator will NOT be copied.
   *
   * This constructor creates a inline list with copies of the elements
   * between @p i and @p j in the same order.
   *
   * @note The ending element (pointed by @p j) is not copied.
   */
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

  /**
   * @brief Copy constructor. Creates a copy of the given inline list.
   * @param other Another inline list of the same type.
   *
   * This constructor creates an inline list containing a copy of each
   * element inside @p other in the same order.
   */
  inlist(inlist<T, Allocator>const& other)
    : _base_type()
  {
    insert(end(), other.begin(), other.end());
  }

  /**
   * @brief Replace current content with the content of another inline list.
   * @param other Another inline list of the same type.
   *
   * This assignment operator replaces the content of the list by a copy
   * of the content of @p other. The list size is adjusted accordingly
   * and the newly copied elements keep their original order.
   */
  inlist<T, Allocator>& operator=(inlist<T, Allocator>const& other)
  {
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }

  /**
   * @brief Get the current size of the inline list.
   * @return Number of elements in the inline list.
   *
   * This member function returns the current number of elements inside
   * the inline list.
   */
  size_type size() const
  {
    return _inlist_access_traits::size<T>(native_handle());
  }

  /**
   * @brief Check if the inline list is empty.
   * @return @c true if the inline list is empty, @c false otherwise.
   *
   * This member function returns @c true if the inline list does not
   * contain any elements, otherwise it returns @c false.
   */
  bool empty() const
  {
    return _inlist_access_traits::empty<T>(native_handle());
  }

  /**
   * @brief Get the allocator used in this inline list.
   */
  allocator_type get_allocator() const
  {
    return allocator_type(this->get_node_allocator());
  }

  /**
   * @brief Add a copy of the given element at the end of the inline list.
   * @param value Element to be added at the end of the inline list.
   *
   * This member function allocates a new element at the end of the
   * inline list, the content of @p value is copied to the new element.
   */
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

  /**
   * @brief Add a copy of the given element at the beginning of the inline list.
   * @param value Element to be added at the beginning of the inline list.
   *
   * This member function allocates a new element at the beginning of
   * the inline list, the content of @p value is copied to the new
   * element.
   */
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

  /**
   * @brief Remove the last element of the inline list.
   */
  void pop_back()
  {
    this->_impl._list = eina_inlist_remove(this->_impl._list, this->_impl._list->last);
  }

  /**
   * @brief Remove the first element of the inline list.
   */
  void pop_front()
  {
    this->_impl._list = eina_inlist_remove(this->_impl._list, this->_impl._list);
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

  /**
   * @brief Insert @p n copies of @p t at the given position.
   * @param i Iterator pointing to the position where the new elements will be inserted.
   * @param n Number of elements to be inserted.
   * @param t Value to be copied to each new inserted element.
   * @return Iterator pointing to the first inserted element.
   *
   * This member function inserts @p n new elements at position @p i
   * in the inline list, each one as a copy of @p t. The new elements
   * come right before the element originally pointed by @p i.
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
        iterator r(_get_node<T>(this->_impl._list), _get_node<T>(_get_list(q.native_handle())->next));
        this->_impl._list = eina_inlist_remove(this->_impl._list, _get_list(q.native_handle()));
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
   * @brief Replace the content of the inline list by the elements in the given range.
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
   * @brief Replace the content of the inline list by @p n copies @p t.
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
   * @return Reference to the last element in the inline list.
   */
  value_type& back()
  {
    return _inlist_access_traits::back<T>(native_handle());
  }

  /**
   * @brief Get a constant reference to the last element.
   * @return Constant reference to the last element in the inline list.
   *
   * Version of @ref back() for const-qualified inline list objects.
   * Returns a constant reference instead.
   */
  value_type const& back() const
  {
    return _inlist_access_traits::back<T>(native_handle());
  }

  /**
   * @brief Get a reference to the first element.
   * @return Reference to the first element of the inline list.
   */
  value_type& front()
  {
    return _inlist_access_traits::front<T>(native_handle());
  }

  /**
   * @brief Get a constant reference to the first element.
   * @return Constant reference to the first element of the inline list.
   *
   * Version of @ref front() for const-qualified inline list objects.
   * Returns a constant reference instead.
   */
  value_type const& front() const
  {
    return _inlist_access_traits::front<T>(native_handle());
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the inline list.
   * @return Constant iterator to the initial position of the inline list.
   *
   * Version of @ref begin() for const-qualified inline list objects.
   * Returns a constant iterator instead.
   */
  const_iterator begin() const
  {
    return _inlist_access_traits::begin<T>(native_handle());
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the inline list.
   * @return Constant iterator to the final position of the inline list.
   *
   * Version of @ref end() for const-qualified inline list objects.
   * Returns a constant iterator instead.
   */
  const_iterator end() const
  {
    return _inlist_access_traits::end<T>(native_handle());
  }

  /**
   * @brief Get an iterator pointing to the first element of the inline list.
   * @return Iterator to the initial position of the inline list.
   *
   * This member function returns an iterator pointing to the first
   * element of the inline list. If the list is empty the returned
   * iterator is the same as the one returned by @ref end().
   */
  iterator begin()
  {
    return _inlist_access_traits::begin<T>(native_handle());
  }

  /**
   * @brief Get an iterator to the position following the last element of the inline list.
   * @return Iterator to the final position of the inline list.
   *
   * This member function returns an iterator to the position following
   * the last element in the inline list. If the list is empty the
   * returned iterator is the same as the one returned by @ref begin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  iterator end()
  {
    return _inlist_access_traits::end<T>(native_handle());
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the inline list.
   * @return Constant reverse iterator pointing to the reverse begin of the inline list.
   *
   * Version of @ref rbegin() for const-qualified inline list objects.
   * Returns a constant reverse iterator instead.
   */
  const_reverse_iterator rbegin() const
  {
    return reverse_iterator(end());
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the inline list.
   * @return Constant reverse iterator pointing to the reverse end of the inline list.
   *
   * Version of @ref rend() for const-qualified inline list objects.
   * Returns a constant reverse iterator instead.
   */
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse begin of the inline list.
   * @return Reverse iterator pointing to the reverse begin of the inline list.
   *
   * This member function returns a reverse iterator pointing to the
   * last element of the inline list. If the list is empty the returned
   * reverse iterator is the same as the one returned by @ref rend().
   */
  reverse_iterator rbegin()
  {
    return reverse_iterator(end());
  }

  /**
   * @brief Get a reverse iterator pointing to the reverse end of the inline list.
   * @return Reverse iterator pointing to the reverse end of the inline list.
   *
   * This member function returns a reverse iterator pointing to the
   * position before the first element of the inline list. If the list
   * is empty the returned iterator is the same as the one returned by
   * @ref rbegin().
   *
   * @note Note that attempting to access this position causes undefined
   * behavior.
   */
  reverse_iterator rend()
  {
    return reverse_iterator(begin());
  }

  /**
   * @brief Get a constant iterator pointing to the first element of the inline list.
   * @return Constant iterator to the initial position of the inline list.
   *
   * This member function works like @ref begin() const but is granted
   * to return a constant iterator even for lists that are not
   * const-qualified.
   */
  const_iterator cbegin() const
  {
    return begin();
  }

  /**
   * @brief Get a constant iterator to the position following the last element of the inline list.
   * @return Constant iterator to the final position of the inline list.
   *
   * This member function works like @ref end() const but is granted to
   * return a constant iterator even for lists that are not
   * const-qualified.
   */
  const_iterator cend() const
  {
    return end();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse begin of the inline list.
   * @return Constant reverse iterator pointing to the reverse begin of the inline list.
   *
   * This member function works like @ref rbegin() const but is granted
   * to return a constant reverse iterator even for lists that are not
   * const-qualified.
   */
  const_reverse_iterator crbegin() const
  {
    return rbegin();
  }

  /**
   * @brief Get a constant reverse iterator pointing to the reverse end of the inline list.
   * @return Constant reverse iterator pointing to the reverse end of the inline list.
   *
   * This member function works like @ref rend() const but is granted to
   * return a constant reverse iterator even for lists that are not
   * const-qualified.
   */
  const_reverse_iterator crend() const
  {
    return rend();
  }

  /**
   * @brief Swap content between two inline lists.
   * @param other Other inline list of the same type.
   */
  void swap(inlist<T, Allocator>& other)
  {
    std::swap(this->_impl._list, other._impl._list);
  }

  /**
   * @brief Get the maximum number of elements a inline list can hold.
   * @return Maximum number of elements a inline list can hold.
   */
  size_type max_size() const { return -1; }

  /**
   * @brief Get the handle for the wrapped Eina_Inlist.
   * @return Internal handle for the native Eina inline list.
   *
   * This member function returns the native Eina_Inlist handle that is
   * wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  Eina_Inlist* native_handle()
  {
    return this->_impl._list;
  }

  /**
   * @brief Get the handle for the wrapped Eina_Inlist.
   * @return Internal handle for the native Eina inline list.
   *
   * This member function returns the native Eina_Inlist handle that is
   * wrapped inside this object.
   *
   * @warning It is important to take care when using it, since the
   * handle will be automatically release upon object destruction.
   */
  Eina_Inlist const* native_handle() const
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
    return eina::accessor<T const>(eina_inlist_accessor_new(this->_impl._list));
  }

  /**
   * @brief Get a @ref eina::accessor for the list.
   * @return <tt>eina::accessor</tt> to the list.
   */
  eina::accessor<T> accessor()
  {
    return eina::accessor<T>(eina_inlist_accessor_new(this->_impl._list));
  }
};

/**
 * @brief Check if two inline lists are equal.
 * @param lhs Inline list at the left side of the expression.
 * @param rhs Inline list at the right side of the expression.
 * @return @c true if the lists are equals, @c false otherwise.
 *
 * This operator checks if the given inline lists are equal. To be
 * considered equal both lists need to have the same number of elements,
 * and each element in one list must be equal to the element at the same
 * position in the other list.
 */
template <typename T, typename Allocator1, typename Allocator2>
bool operator==(inlist<T, Allocator1> const& lhs, inlist<T, Allocator2> const& rhs)
{
  return lhs.size() == rhs.size() &&
    std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Return the opposite of @ref operator==(inlist<T, Allocator1> const& lhs, inlist<T, Allocator2> const& rhs).
 */
template <typename T, typename Allocator1, typename Allocator2>
bool operator!=(inlist<T, Allocator1> const& lhs, inlist<T, Allocator2> const& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Swap content between two inline lists.
 * @param other Other inline list of the same type.
 */
template <typename T, typename Allocator>
void swap(inlist<T, Allocator>& lhs, inlist<T, Allocator>& rhs)
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
