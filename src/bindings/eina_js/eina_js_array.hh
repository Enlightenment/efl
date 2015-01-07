#ifndef EINA_JS_ARRAY_HH
#define EINA_JS_ARRAY_HH

#include <iostream>

#include <eina_js_container.hh>
#include <Eina.hh>
#include <eina_array.hh>

#include <iterator>

namespace efl { namespace eina { namespace js {
    
template <typename T>
struct eina_array : eina_container_common<efl::eina::array
                                    <T
                                    , typename std::conditional
                                      <std::is_base_of<efl::eo::base, T>::value
                                       , efl::eina::eo_clone_allocator
                                       , efl::eina::malloc_clone_allocator
                                       >::type> >
{
  typedef eina_container_common<efl::eina::array<T
                           , typename std::conditional
                           <std::is_base_of<efl::eo::base, T>::value
                            , efl::eina::eo_clone_allocator
                            , efl::eina::malloc_clone_allocator
                            >::type> > base_type;
  using base_type::base_type;
  typedef typename base_type::container_type container_type;

  eina_container_base* concat(eina_container_base const& other) const
  {
    return detail::concat(*this, other);
  }
  eina_container_base* slice(std::int64_t i, std::int64_t j) const
  {
    return detail::slice(*this, i, j);
  }
  js::container_type get_container_type() const { return array_container_type; }
};

template <typename T>
struct range_eina_array : eina_container_common<efl::eina::range_array<T> >
{
  typedef eina_container_common<efl::eina::range_array<T> > base_type;
  using base_type::base_type;
  typedef typename base_type::container_type container_type;
  typedef typename std::conditional
    <std::is_base_of<efl::eo::base, T>::value
     , efl::eina::eo_clone_allocator
     , efl::eina::malloc_clone_allocator
     >::type clone_allocator_type;
  
  eina_container_base* concat(eina_container_base const& other) const
  {
    std::cout << __func__ << std::endl;
    range_eina_array<T>const& rhs = static_cast<range_eina_array<T>const&>(other);
    efl::eina::array<T, clone_allocator_type>
      array(this->_container.begin(), this->_container.end());
    array.insert(array.end(), rhs._container.begin(), rhs._container.end());
    return new eina_array<T>(array.release_native_handle());
  }
  eina_container_base* slice(std::int64_t i, std::int64_t j) const
  {
    efl::eina::array<T, clone_allocator_type>
      array(std::next(this->_container.begin(), i), std::next(this->_container.begin(), j));
    return new eina_array<T>(array.release_native_handle());
  }
  js::container_type get_container_type() const { return array_container_type; }
};

} } }

#endif
