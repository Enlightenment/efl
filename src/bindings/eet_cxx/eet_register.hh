#ifndef EFL_EET_REGISTER_HH_
#define EFL_EET_REGISTER_HH_

#include <eet_type.hh>
#include <eet_composite.hh>

namespace efl { namespace eet {

template <typename, typename...> struct descriptor;

namespace _detail {

template <typename T>
struct member_type;

template <typename T, typename U>
struct member_type<T(U::*)>
{
  typedef T type;
};

template <typename T>
struct object_type;

template <typename T, typename U>
struct object_type<T(U::*)>
{
  typedef U type;
};

template <typename F, typename T, typename... Args>
struct member_info
{
  typedef F member_type;

  const char* name;
  F member;
  eet::descriptor<T, Args...> const* descriptor;
};

template <typename F>
struct member_info<F, void>
{
  typedef F member_type;

  const char* name;
  F member;
};

template <typename F, typename U, typename... Args>
void descriptor_type_register_impl
 (std::false_type
  , ::Eet_Data_Descriptor* cls
  , member_desc_info i
  , member_info<F, U, Args...> arg0
  , typename std::enable_if
  <
    !std::is_pointer<typename _detail::member_type<F>::type>::value
  >::type* = 0)
{
  // composition by value
  static_assert(std::is_member_object_pointer<F>::value, "");
  typedef typename _detail::member_type<F>::type member_type;
  static_assert(!std::is_pointer<member_type>::value, "");
  static_assert(std::is_same<member_type, U>::value, "");
  static_assert(std::is_pod<member_type>::value, "");

  _detail::descriptor_type_register_composite(cls, i, arg0.descriptor);
}

template <typename F, typename U, typename... Args>
void descriptor_type_register_impl
 (std::false_type
  , ::Eet_Data_Descriptor* cls
  , member_desc_info i
  , member_info<F, U, Args...> arg0
  , typename std::enable_if
  <
    std::is_pointer<typename _detail::member_type<F>::type>::value
  >::type* = 0)
{
  // composition by pointer
  static_assert(std::is_member_object_pointer<F>::value, "");
  typedef typename _detail::member_type<F>::type pointer_member_type;
  static_assert(std::is_pointer<pointer_member_type>::value, "");
  typedef typename std::remove_pointer<pointer_member_type>::type member_type;
  static_assert(std::is_same<member_type, U>::value, "");

  eet_data_descriptor_element_add
    (cls, i.name
     , EET_T_UNKNOW
     , EET_G_UNKNOWN
     , i.offset
     , 0
     , nullptr
     , const_cast<descriptor<U, Args...>*>(arg0.descriptor)->native_handle());
}

template <typename F>
void descriptor_type_register_impl
 (std::true_type, ::Eet_Data_Descriptor* cls
  , member_desc_info i
  , member_info<F, void>)
{
  static_assert(std::is_member_object_pointer<F>::value, "");
  typedef typename _detail::member_type<F>::type member_type;

  eet_data_descriptor_element_add(cls, i.name, _eet_type<member_type>::value, EET_G_UNKNOWN
                                  , i.offset, 0, nullptr, nullptr);
}

inline void descriptor_type_register( ::Eet_Data_Descriptor*, member_desc_info*)
{
}

template <typename F, typename D, typename... Args, typename... FArgs>
void descriptor_type_register( ::Eet_Data_Descriptor* cls, member_desc_info* i
                               , member_info<F, D, Args...> a0, FArgs... args)
{
  static_assert(std::is_member_object_pointer<F>::value, "");
  typedef typename _detail::member_type<F>::type member_type;

  _detail::descriptor_type_register_impl(is_eet_primitive<member_type>(), cls, *i, a0);
  _detail::descriptor_type_register(cls, ++i, args...);
}

} } }

#endif
