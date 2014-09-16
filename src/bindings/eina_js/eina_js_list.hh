#ifndef EINA_JS_LIST_HH
#define EINA_JS_LIST_HH

#include <iostream>

#include <Eina.hh>

namespace efl { namespace js {

struct eina_list_base
{
  virtual ~eina_list_base() {}

  virtual std::size_t size() const = 0;
  virtual eina_list_base* concat(eina_list_base const& rhs) const = 0;
  //virtual v8::Local<v8::Value> operator[](std::size_t) const = 0;
  virtual v8::Local<v8::String> to_string(v8::Isolate*) const = 0;
};

template <typename C>
struct eina_list_common : eina_list_base
{
  eina_list_common() : _container(0) {}
  eina_list_common(Eina_List* raw) : _container(raw) {}

  std::size_t size() const { return _container.size(); }
  v8::Local<v8::String> to_string(v8::Isolate* isolate) const
  {
    std::cout << "to_string" << std::endl;
    typedef typename container_type::const_iterator iterator;
    std::stringstream s;
    s << '[';
    for(iterator first = _container.begin()
          , last = _container.end()
          , last_elem = std::next(last, -1); first != last; ++first)
      {
        s << *first;
        if(first !=  last)
          s << ", ";
      }
    s << ']';
    std::cout << "string " << s.str() << std::endl;
    return v8::String::NewFromUtf8(isolate, s.str().c_str());
  }

  C _container;
  typedef C container_type;
};

template <typename T>
struct eina_list : eina_list_common<efl::eina::list
                                    <T
                                    , typename std::conditional
                                      <std::is_base_of<efl::eo::base, T>::value
                                       , efl::eina::eo_clone_allocator
                                       , efl::eina::malloc_clone_allocator
                                       >::type> >
{
  typedef eina_list_common<efl::eina::list<T
                           , typename std::conditional
                           <std::is_base_of<efl::eo::base, T>::value
                            , efl::eina::eo_clone_allocator
                            , efl::eina::malloc_clone_allocator
                            >::type> > base_type;
  using base_type::base_type;
  typedef typename base_type::container_type container_type;

  eina_list_base* concat(eina_list_base const& other) const
  {
    std::cout << __func__ << std::endl;
    eina_list<T>const& rhs = static_cast<eina_list<T>const&>(other);
    container_type list(this->_container.begin(), this->_container.end());
    list.insert(list.end(), rhs._container.begin(), rhs._container.end());
    return new eina_list<T>(list.release_native_handle());
  }
};

template <typename T>
struct range_eina_list : eina_list_common<efl::eina::range_list<T> >
{
  typedef eina_list_common<efl::eina::range_list<T> > base_type;
  using base_type::base_type;
  typedef typename base_type::container_type container_type;
  typedef typename std::conditional
    <std::is_base_of<efl::eo::base, T>::value
     , efl::eina::eo_clone_allocator
     , efl::eina::malloc_clone_allocator
     >::type clone_allocator_type;
  
  eina_list_base* concat(eina_list_base const& other) const
  {
    std::cout << __func__ << std::endl;
    range_eina_list<T>const& rhs = static_cast<range_eina_list<T>const&>(other);
    efl::eina::list<T, clone_allocator_type>
      list(this->_container.begin(), this->_container.end());
    list.insert(list.end(), rhs._container.begin(), rhs._container.end());
    return new eina_list<T>(list.release_native_handle());
  }
};

} }

#endif
