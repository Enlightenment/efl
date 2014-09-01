#ifndef EINA_JS_ARRAY_HH
#define EINA_JS_ARRAY_HH

#include <iostream>

#include <iterator>

#include <eina_js_container.hh>

namespace efl { namespace eina { namespace js {
    
template <typename T, typename K, typename W>
struct eina_array : eina_container_common<efl::eina::array
                                    <W
                                    , typename std::conditional
                                      <std::is_base_of<efl::eo::concrete, W>::value
                                       , efl::eina::eo_clone_allocator
                                       , efl::eina::malloc_clone_allocator
                                       >::type>, T, K>
{
  typedef eina_container_common<efl::eina::array<W
                           , typename std::conditional
                           <std::is_base_of<efl::eo::concrete, W>::value
                            , efl::eina::eo_clone_allocator
                            , efl::eina::malloc_clone_allocator
                            >::type>, T, K> base_type;
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
  v8::Local<v8::Value> set(v8::Isolate* isolate, std::size_t index, v8::Local<v8::Value> v)
  {
    return detail::set<T,W>(isolate, *this, index, v);
  }
  int push(v8::Isolate* isolate, v8::Local<v8::Value> v)
  {
    return detail::push<T,W>(isolate, *this, v);
  }
  v8::Local<v8::Value> pop(v8::Isolate* isolate)
  {
    return detail::pop<T,W,K>(isolate, *this);
  }
  js::container_type get_container_type() const { return array_container_type; }
};

template <typename T, typename K, typename W>
struct range_eina_array : eina_container_common<efl::eina::range_array<W>, T, K>
{
  typedef eina_container_common<efl::eina::range_array<W>, T, K> base_type;
  using base_type::base_type;
  typedef typename base_type::container_type container_type;
  typedef typename std::conditional
    <std::is_base_of<efl::eo::concrete, W>::value
     , efl::eina::eo_clone_allocator
     , efl::eina::malloc_clone_allocator
     >::type clone_allocator_type;
  
  eina_container_base* concat(eina_container_base const& other) const
  {
    range_eina_array<T, K, W> const& rhs = static_cast<range_eina_array<T, K, W>const&>(other);
    efl::eina::array<W, clone_allocator_type>
      array(this->_container.begin(), this->_container.end());
    array.insert(array.end(), rhs._container.begin(), rhs._container.end());
    return new eina_array<T, K, W>(array.release_native_handle());
  }
  eina_container_base* slice(std::int64_t i, std::int64_t j) const
  {
    efl::eina::array<W, clone_allocator_type>
      array(std::next(this->_container.begin(), i), std::next(this->_container.begin(), j));
    return new eina_array<T, K, W>(array.release_native_handle());
  }
  js::container_type get_container_type() const { return array_container_type; }
};

// Problematic types.
template <>
struct eina_array<_Elm_Calendar_Mark*, js::nonclass_cls_name_getter, _Elm_Calendar_Mark>
  : empty_container_base
{ eina_array(Eina_Array const*){} };
template <>
struct eina_array<Elm_Gen_Item*, js::nonclass_cls_name_getter, Elm_Gen_Item>
  : empty_container_base
{ eina_array(Eina_Array const*){} };
template <>
struct eina_array<_Evas_Textblock_Rectangle*, js::nonclass_cls_name_getter, _Evas_Textblock_Rectangle>
  : empty_container_base
{ eina_array(Eina_Array const*){} };
template <>
struct eina_array<_Elm_Map_Overlay*, js::nonclass_cls_name_getter, _Elm_Map_Overlay>
  : empty_container_base
{ eina_array(Eina_Array const*){} };

template <>
struct range_eina_array<_Elm_Calendar_Mark*, js::nonclass_cls_name_getter, _Elm_Calendar_Mark>
  : empty_container_base
{ range_eina_array(Eina_Array const*){} };
template <>
struct range_eina_array<Elm_Gen_Item*, js::nonclass_cls_name_getter, Elm_Gen_Item>
  : empty_container_base
{ range_eina_array(Eina_Array const*){} };
template <>
struct range_eina_array<_Evas_Textblock_Rectangle*, js::nonclass_cls_name_getter, _Evas_Textblock_Rectangle>
  : empty_container_base
{ range_eina_array(Eina_Array const*){} };
template <>
struct range_eina_array<_Elm_Map_Overlay*, js::nonclass_cls_name_getter, _Elm_Map_Overlay>
  : empty_container_base
{ range_eina_array(Eina_Array const*){} };

} } }

#endif
