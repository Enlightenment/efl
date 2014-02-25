#ifndef EINA_ITERATOR_HH_
#define EINA_ITERATOR_HH_

#include <Eina.h>

#include <cstdlib>
#include <iterator>

namespace efl { namespace eina {

template <typename T>
struct _common_iterator_base
{
private:
  typedef _common_iterator_base<T> self_type;
public:
  typedef T const value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef std::ptrdiff_t difference_type;
  typedef std::input_iterator_tag iterator_category;

  _common_iterator_base() {}
  explicit _common_iterator_base(Eina_Iterator* iterator)
    : _iterator(iterator) {}
  ~_common_iterator_base()
  {
    if(_iterator)
      eina_iterator_free(_iterator);
  }
  _common_iterator_base(self_type const& other)
    : _iterator(other._iterator)
  {
    other._iterator = 0;
  }
  _common_iterator_base& operator=(self_type const& other)
  {
    _iterator = other._iterator;
    other._iterator = 0;
    return *this;
  }

protected:
  mutable Eina_Iterator* _iterator;

  friend inline bool operator==(_common_iterator_base<T> const& lhs, _common_iterator_base<T> const& rhs)
  {
    return lhs._iterator == rhs._iterator;
  }
  friend inline bool operator!=(_common_iterator_base<T> const& lhs, _common_iterator_base<T> const& rhs)
  {
    return !(lhs == rhs);
  }
};

template <typename T>
struct iterator : _common_iterator_base<T const>
{
private:
  typedef _common_iterator_base<T const> base_type;
  typename base_type::pointer _value;
  typedef iterator<T> self_type;
public:
  typedef typename base_type::value_type value_type;
  typedef typename base_type::pointer pointer;
  typedef typename base_type::reference reference;
  typedef typename base_type::difference_type difference_type;
  typedef typename base_type::iterator_category iterator_category;

  explicit iterator(Eina_Iterator* iterator = 0)
    : base_type(iterator)
  {
    if(this->_iterator)
      ++*this;
  }
  self_type& operator++()
  {
    void* data;
    Eina_Bool r = ::eina_iterator_next(this->_iterator, &data);
    if(!r)
      this->_iterator = 0;
    _value = static_cast<pointer>(data);
    return *this;
  }
  self_type& operator++(int)
  {
    return ++**this;
  }
  value_type& operator*() const
  {
    return *_value;
  }
  pointer operator->() const
  {
    return _value;
  }
};

} }

#endif
