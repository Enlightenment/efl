#ifndef EINA_JS_LIST_HH
#define EINA_JS_LIST_HH

#include <iostream>

#include <eina_js_container.hh>
#include <Eina.hh>

#include <iterator>

namespace efl { namespace js {
    
template <typename C>
struct eina_container_common : eina_container_type_specific<C, typename C::value_type>
{
  eina_container_common() : _container(0) {}
  eina_container_common(typename C::native_handle_type raw) : _container(raw) {}

  std::size_t size() const { return _container.size(); }
  v8::Local<v8::String> to_string(v8::Isolate* isolate) const
  {
    std::cout << "to_string" << std::endl;
    typedef typename container_type::const_iterator iterator;
    std::stringstream s;
    for(iterator first = _container.begin()
          , last = _container.end()
          , last_elem = std::next(last, -1); first != last; ++first)
      {
        s << *first;
        if(first !=  last_elem)
          s << ",";
      }
    std::cout << "string " << s.str() << std::endl;
    return v8::String::NewFromUtf8(isolate, s.str().c_str());
  }

  C _container;
  typedef C container_type;
};

namespace detail {
    
template <typename T>
eina_container_base* concat(T const& self, eina_container_base const& other)
{
  std::cout << __func__ << std::endl;
  T const& rhs = static_cast<T const&>(other);
  typedef typename T::container_type container_type;
  container_type container(self._container.begin(), self._container.end());
  container.insert(container.end(), rhs._container.begin(), rhs._container.end());
  return new T(container.release_native_handle());
}

template <typename T>
eina_container_base* slice(T const& self, std::int64_t i, std::int64_t j)
{
  std::cout << __func__ << std::endl;
  typedef typename T::container_type container_type;
  container_type container(std::next(self._container.begin(), i), std::next(self._container.begin(), j));
  return new T(container.release_native_handle());
}

}
    
template <typename T>
struct eina_list : eina_container_common<efl::eina::list
                                    <T
                                    , typename std::conditional
                                      <std::is_base_of<efl::eo::base, T>::value
                                       , efl::eina::eo_clone_allocator
                                       , efl::eina::malloc_clone_allocator
                                       >::type> >
{
  typedef eina_container_common<efl::eina::list<T
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
  js::container_type get_container_type() const { return list_container_type; }
};

template <typename T>
struct range_eina_list : eina_container_common<efl::eina::range_list<T> >
{
  typedef eina_container_common<efl::eina::range_list<T> > base_type;
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
    range_eina_list<T>const& rhs = static_cast<range_eina_list<T>const&>(other);
    efl::eina::list<T, clone_allocator_type>
      list(this->_container.begin(), this->_container.end());
    list.insert(list.end(), rhs._container.begin(), rhs._container.end());
    return new eina_list<T>(list.release_native_handle());
  }
  eina_container_base* slice(std::int64_t i, std::int64_t j) const
  {
    efl::eina::list<T, clone_allocator_type>
      list(std::next(this->_container.begin(), i), std::next(this->_container.begin(), j));
    return new eina_list<T>(list.release_native_handle());
  }
  js::container_type get_container_type() const { return list_container_type; }
};

} }

#endif
