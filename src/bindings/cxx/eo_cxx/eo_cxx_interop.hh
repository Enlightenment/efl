
#ifndef EFL_EOLIAN_INTEROP_HH
#define EFL_EOLIAN_INTEROP_HH

#include <string>
#include <tuple>
#include <utility>
#include <type_traits>
#include <initializer_list>

#include <Eina.hh>
#include <Eo.hh>

#include "eo_concrete.hh"

namespace efl { namespace eolian {

template <typename T, typename Enable = void>
struct in_traits { typedef T const& type; };
template <typename T>
struct in_traits<T, typename std::enable_if<eo::is_eolian_object<T>::value>::type> { typedef T type; };
template <typename T>
struct in_traits<T, typename std::enable_if<std::is_fundamental<T>::value>::type> { typedef T type; };
template <typename R, typename...Args>
struct in_traits<R(*)(Args...)>
{
  typedef std::function<R(Args...)> type;
};
template <>
struct in_traits<eina::string_view> { typedef eina::string_view type; };
template <>
struct in_traits<eina::string_view const> { typedef eina::string_view const type; };
template <>
struct in_traits<eina::stringshare> { typedef eina::stringshare type; };
template <>
struct in_traits<eina::stringshare const> { typedef eina::stringshare const type; };
template <typename T>
struct in_traits<T&> { typedef T& type; };
template <typename T>
struct in_traits<T*> { typedef T* type; };
template <typename T, typename D>
struct in_traits<std::unique_ptr<T, D>> { typedef std::unique_ptr<T, D> type; };

template <typename T>
struct in_traits<eina::range_list<T>> { typedef eina::range_list<T> type; };
template <typename T>
struct in_traits<eina::range_array<T>> { typedef eina::range_array<T> type; };
    
template <typename T>
struct out_traits { typedef T& type; };
template <>
struct out_traits<void> { typedef void* type; };
template <typename T>
struct out_traits<T*> { typedef T* type; };
template <>
struct out_traits<Eina_Hash*> { typedef Eina_Hash*& type; };
template <typename T>
struct out_traits<eina::optional<T&>> { typedef eina::optional<T&> type; };
template <>
struct out_traits<void*> { typedef void*& type; };
template <>
struct out_traits<efl::eina::strbuf> { typedef efl::eina::strbuf_wrapper& type; };

template <typename T>
struct inout_traits { typedef T& type; };
template <>
struct inout_traits<void> { typedef void* type; };

template <typename T>
struct return_traits { typedef T type; };
template <>
struct return_traits<eina::value_view&> { typedef eina::value_view type; };
template <>
struct return_traits<eina::value_view const&> { typedef eina::value_view type; };

template <typename To, typename From, bool Own = false, typename Lhs, typename Rhs>
void assign_out(Lhs& lhs, Rhs& rhs);
  
namespace impl {

template <typename From, typename To, bool Own = false>
struct tag
{
  typedef To to;
  typedef From from;
  typedef std::integral_constant<bool, Own> own;
};
  
template <typename T>
void assign_out_impl(T*& lhs, T* rhs, tag<T*, T*>)
{
  lhs = rhs;
}
template <typename T>
void assign_out_impl(T& lhs, T*& rhs, tag<T&, T*>, typename std::enable_if<!std::is_const<T>::value>::type* = 0)
{
  lhs = *rhs;
}
template <typename T>
void assign_out_impl(T const& lhs, T const*& rhs, tag<T const&, T const*>)
{
  const_cast<T&>(lhs) = *rhs;
}
inline void assign_out_impl(void*&, void*&, tag<void*, void*>)
{
  std::abort(); // out parameter of void type?
}
inline void assign_out_impl(void*&, void*&, tag<void*, void>)
{
  // do nothing, it is an inout parameter of void
}
template <typename U, typename T, typename D>
void assign_out_impl(std::unique_ptr<T, D>& lhs, U* rhs, tag<std::unique_ptr<T, D>&, U*, true>)
{
  // with own
  lhs.reset(rhs);
}
template <typename Tag>
void assign_out_impl(bool& lhs, Eina_Bool rhs, Tag)
{
  lhs = rhs;
}
template <typename T>
void assign_out_impl(T& lhs, T& rhs, tag<T&, T>)
{
  lhs = rhs;
}
template <typename T>
void assign_out_impl(T& lhs, Eo* rhs, tag<T&, Eo*>
                     , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  lhs._reset(rhs);
}
template <typename T>
void assign_out_impl(T& lhs, Eo const* rhs, tag<T&, Eo const*>
                     , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  lhs._reset(const_cast<Eo*>(rhs));
}
template <typename Tag>
void assign_out_impl(efl::eina::string_view& view, const char* string, Tag)
{
  view = {string};
}
template <typename Tag>
void assign_out_impl(efl::eina::string_view* view, const char* string, Tag)
{
  if(view)
    *view = {string};
}
template <typename Tag>
void assign_out_impl(efl::eina::stringshare& to, const char* from, Tag)
{
  to = from;
}
template <typename T>
void assign_out_impl(T*& lhs, T& rhs, tag<T*, T>) // optional
{
  if(lhs)
    *lhs = rhs;
}
template <typename T, typename Rhs, typename U, typename O, bool B>
void assign_out_impl(efl::eina::optional<T>& lhs, Rhs& rhs, tag<efl::eina::optional<U&>, O, B>)
{
  if(lhs)
    assign_out<U&, O, true>(*lhs, rhs);
}
template <typename Tag>
void assign_out_impl(eina::value& lhs, Eina_Value& rhs, Tag)
{
  Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_CHAR);
  eina_value_flush(v);
  eina_value_copy(&rhs, v);
  lhs.reset(v);
  eina_value_flush(&rhs);
}
// This is a invalid use-case that is used in EFL. This leaks
template <typename Tag>
void assign_out_impl(eina::value_view& lhs, Eina_Value& rhs, Tag)
{
  Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_CHAR);
  eina_value_flush(v);
  eina_value_copy(&rhs, v);
  lhs.reset(v);
}
template <typename T>
void assign_out_impl(efl::eina::list<T>& lhs, Eina_List* rhs, tag<efl::eina::list<T>&, Eina_List*, true>)
{
  lhs = efl::eina::list<T>{rhs};
}
template <typename T>
void assign_out_impl(efl::eina::range_list<T>& lhs, Eina_List* rhs, tag<efl::eina::range_list<T>&, Eina_List*>)
{
  lhs = efl::eina::range_list<T>{rhs};
}
template <typename T>
void assign_out_impl(efl::eina::array<T>& lhs, Eina_Array* rhs, tag<efl::eina::array<T>&, Eina_Array*, true>)
{
  lhs = efl::eina::array<T>{rhs};
}
template <typename T>
void assign_out_impl(efl::eina::range_array<T>& lhs, Eina_Array* rhs, tag<efl::eina::range_array<T>&, Eina_Array*>)
{
  lhs = efl::eina::range_array<T>{rhs};
}
inline void assign_out_impl(Eina_Hash*& lhs, Eina_Hash*& rhs, tag<Eina_Hash*&, Eina_Hash*, true>)
{
  lhs = rhs;
}
template <typename T>
void assign_out_impl(efl::eina::iterator<T>& /*lhs*/, Eina_Iterator* /*rhs*/, tag<efl::eina::iterator<T>&, Eina_Iterator*>)
{
  // Must copy here
  std::abort();
}
template <typename T>
void assign_out_impl(efl::eina::iterator<T>& lhs, Eina_Iterator* rhs, tag<efl::eina::iterator<T>&, Eina_Iterator*, true>)
{
  lhs = efl::eina::iterator<T>{rhs};  
}
template <typename T>
void assign_out_impl(efl::eina::accessor<T>& lhs, Eina_Accessor* rhs, tag<efl::eina::accessor<T>&, Eina_Accessor*>)
{
  lhs = efl::eina::accessor<T>{ ::eina_accessor_clone(rhs) };
}
template <typename T>
void assign_out_impl(efl::eina::accessor<T>& lhs, Eina_Accessor* rhs, tag<efl::eina::accessor<T>&, Eina_Accessor*, true>)
{
  lhs = efl::eina::accessor<T>{rhs};  
}
}
    
template <typename To, typename From, bool Own, typename Lhs, typename Rhs>
void assign_out(Lhs& lhs, Rhs& rhs)
{
  return impl::assign_out_impl(lhs, rhs, impl::tag<To, From, Own>{});
}
    
namespace impl {

template <typename T>
T* convert_inout_impl(T& v, tag<T, T*>)
{
  return v;
}
inline Eina_Bool convert_inout_impl(bool v, tag<bool, Eina_Bool>)
{
  return v ? EINA_TRUE : EINA_FALSE;
}
inline void* convert_inout_impl(void* v, tag<void, void>)
{
  return v;
}
template <typename T>
T& convert_inout_impl(T& v, tag<T, T>)
{
  return v;
}
template <typename T>
Eo* convert_inout_impl(T& v, tag<T, Eo*>
                      , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return v._eo_ptr();
}
template <typename T>
Eo const* convert_inout_impl(T v, tag<T, Eo const*>
                            , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return v._eo_ptr();
}
}
    
template <typename To, typename From, typename V>
auto convert_inout(V& object) -> decltype(impl::convert_inout_impl(object, impl::tag<From, To>{}))
{
  return impl::convert_inout_impl(object, impl::tag<From, To>{});
}
    
template <typename T, typename U, bool Own = false, typename V>
T convert_to_c(V&& object);

template <typename U, typename F, typename V=void> struct function_wrapper;

namespace impl {

template <typename U, typename T, typename V>
auto convert_to_c_impl
(V&& v, tag<U, T>, typename std::enable_if<std::is_same<typename std::remove_reference<T>::type, U>::value>::type* =0) -> decltype(std::forward<V>(v))
{
  return std::forward<V>(v);
}

template <typename T>
T convert_to_c_impl(T& /*v*/, tag<T, T, true>)
{
  std::abort();
}
template <typename T>
T* convert_to_c_impl(T& v, tag<T*, T&>)
{
  return &v;
}
template <typename T>
T* convert_to_c_impl(T& /*v*/, tag<T*, T&, true>)
{
  std::abort();
}
template <typename T>
T* convert_to_c_impl(T const& v, tag<T*, T const&>) // not own
{
  return const_cast<T*>(&v);
}
template <typename T>
T* convert_to_c_impl(T const& v, tag<T*, T const&, true>) // with own
{
  T* r = static_cast<T*>(malloc(sizeof(T)));
  *r = v;
  return r;
}
template <typename T>
T const* convert_to_c_impl(T& v, tag<T const*, T&>)
{
  return &v;
}
template <typename T>
T const* convert_to_c_impl(T* v, tag<T const*, T*>)
{
  return v;
}
template <typename T>
T const& convert_to_c_impl(T const& v, tag<T, T const&>)
{
  return v;
}
template <typename T>
Eo* convert_to_c_impl(T v, tag<Eo*, T>
                      , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return v._eo_ptr();
}
template <typename T>
Eo* convert_to_c_impl(T v, tag<Eo*, T, true>
                      , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return ::efl_ref(v._eo_ptr());
}
template <typename T>
Eo const* convert_to_c_impl(T v, tag<Eo const*, T>
                            , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return const_cast<Eo const*>(v._eo_ptr());
}
template <typename T>
Eo** convert_to_c_impl(T& v, tag<Eo**, T&>
                       , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return reinterpret_cast<Eo**>(static_cast<void*>(&v));
}
inline const char* convert_to_c_impl( ::efl::eina::string_view v, tag<const char*, ::efl::eina::string_view>)
{
  return v.c_str();
}
inline char* convert_to_c_impl( ::efl::eina::string_view v, tag<char*, ::efl::eina::string_view, true>)
{
  char* string = static_cast<char*>(malloc(v.size() + 1));
  std::strcpy(string, v.c_str());
  return string;
}
inline char** convert_to_c_impl(efl::eina::string_view& /*view*/, tag<char **, efl::eina::string_view, true>)
{
  std::abort();
}
inline const char** convert_to_c_impl(efl::eina::string_view& /*view*/, tag<const char **, efl::eina::string_view, true>)
{
  std::abort();
}
inline const char* convert_to_c_impl( std::string const& v, tag<const char*, std::string const&>)
{
  std::size_t len = v.size()+1;
  char* buffer = static_cast<char*>(malloc(len));
  std::memcpy(buffer, v.data(), len);
  return buffer;
}
inline const char** convert_to_c_impl(std::string* /*view*/, tag<const char **, std::string*>)
{
  std::abort();
}
inline Eina_Value* convert_to_c_impl( ::efl::eina::value v, tag<Eina_Value*, in_traits<eina::value>::type>)
{
  Eina_Value* nv = eina_value_new(v.type_info());
  eina_value_copy(v.native_handle(), nv);
  return nv;
}
inline Eina_Value const* convert_to_c_impl( ::efl::eina::value& v, tag<Eina_Value const*, in_traits<eina::value>::type>)
{
  Eina_Value* nv = eina_value_new(v.type_info());
  eina_value_copy(v.native_handle(), nv);
  return nv;
}
inline Eina_Value const* convert_to_c_impl( ::efl::eina::value_view const& v, tag<Eina_Value const*, in_traits<eina::value_view const&>::type>)
{
  return v.native_handle();
}
inline Eina_Value* convert_to_c_impl( ::efl::eina::value_view& v, tag<Eina_Value*, in_traits<eina::value_view&>::type>)
{
  return v.native_handle();
}
inline Eina_Value* convert_to_c_impl( ::efl::eina::value_view const& v, tag<Eina_Value*, in_traits<eina::value_view const&>::type>)
{
  return const_cast<Eina_Value*>(v.native_handle());
}
inline Eina_Value const& convert_to_c_impl( ::efl::eina::value_view const& v, tag<Eina_Value, in_traits<eina::value_view const&>::type>)
{
  return *v.native_handle();
}
inline const Eina_Value convert_to_c_impl( ::efl::eina::value_view const& v, tag<const Eina_Value, in_traits<eina::value_view const&>::type>)
{
  return *v.native_handle();
}
inline Eina_Bool convert_to_c_impl( bool b, tag<Eina_Bool, bool>)
{
  return b;
}
template <typename T>
T convert_to_c_impl(efl::eina::optional<T> const& optional, tag<T, efl::eina::optional<T>const&>)
{
  return optional ? *optional : T{};
}
template <typename T>
T* convert_to_c_impl(efl::eina::optional<T const&>const& optional, tag<T*, efl::eina::optional<T const&>const&, true>)
{
  if(optional)
    {
      T* r = static_cast<T*>(malloc(sizeof(T)));
      *r = *optional;
      return r;
    }
  else
    return nullptr;
}
template <typename U, typename T>
U convert_to_c_impl(efl::eina::optional<T> const& optional, tag<U, efl::eina::optional<T>const&>)
{
  return impl::convert_to_c_impl(optional ? *optional : T{}, tag<U, typename in_traits<T>::type>{});
}
template <typename T>
Eina_List* convert_to_c_impl(efl::eina::range_list<T> range, tag<Eina_List *, efl::eina::range_list<T>>)
{
  return range.native_handle();
}
template <typename T>
Eina_List const* convert_to_c_impl(efl::eina::range_list<T> range, tag<Eina_List const *, efl::eina::range_list<T>>)
{
  return range.native_handle();
}
template <typename T>
Eina_List* convert_to_c_impl(efl::eina::list<T>const& c, tag<Eina_List *, efl::eina::list<T>const&, true>)
{
  return const_cast<Eina_List*>(c.native_handle());
}
template <typename T>
Eina_List const* convert_to_c_impl(efl::eina::list<T>const& c, tag<Eina_List const *, efl::eina::list<T>const&, true>)
{
  return c.native_handle();
}

template <typename T>
Eina_Array* convert_to_c_impl(efl::eina::range_array<T> range, tag<Eina_Array *, efl::eina::range_array<T>>)
{
  return range.native_handle();
}
template <typename T>
Eina_Array const* convert_to_c_impl(efl::eina::range_array<T> range, tag<Eina_Array const *, efl::eina::range_array<T>>)
{
  return range.native_handle();
}
template <typename T>
Eina_Array* convert_to_c_impl(efl::eina::array<T>const& c, tag<Eina_Array *, efl::eina::array<T>const&, true>)
{
  return const_cast<Eina_Array*>(c.native_handle());
}
template <typename T>
Eina_Array const* convert_to_c_impl(efl::eina::array<T>const& c, tag<Eina_Array const *, efl::eina::array<T>const&, true>)
{
  return c.native_handle();
}
template <typename T>
Eina_Iterator* convert_to_c_impl(efl::eina::iterator<T>const& i, tag<Eina_Iterator *, efl::eina::iterator<T>const&>)
{
  return const_cast<Eina_Iterator*>(i.native_handle());
}
template <typename T>
Eina_Iterator const* convert_to_c_impl(efl::eina::iterator<T>const& i, tag<Eina_Iterator const*, efl::eina::iterator<T>const&>)
{
  return i.native_handle();
}
template <typename T>
Eina_Iterator* convert_to_c_impl(efl::eina::iterator<T>const& /*i*/, tag<Eina_Iterator *, efl::eina::iterator<T>const&, true>)
{
  // Eina Iterator must be copied
  std::abort();
}
template <typename T>
Eina_Accessor* convert_to_c_impl(efl::eina::accessor<T>const& i, tag<Eina_Accessor *, efl::eina::accessor<T>const&>)
{
  return const_cast<Eina_Accessor*>(i.native_handle());
}
template <typename T>
Eina_Accessor const* convert_to_c_impl(efl::eina::accessor<T>const& i, tag<Eina_Accessor const*, efl::eina::accessor<T>const&>)
{
  return i.native_handle();
}
template <typename T>
Eina_Accessor* convert_to_c_impl(efl::eina::accessor<T>const& i, tag<Eina_Accessor *, efl::eina::accessor<T>const&, true>)
{
  return ::eina_accessor_clone(const_cast<Eina_Accessor*>(i.native_handle()));
}
inline const char** convert_to_c_impl(efl::eina::string_view /*view*/, tag<char const **, efl::eina::string_view>)
{
  std::abort();
}
inline char** convert_to_c_impl(efl::eina::string_view /*view*/, tag<char **, efl::eina::string_view, true>)
{
  std::abort();
}
inline const char* convert_to_c_impl(efl::eina::stringshare x, tag<const char*, efl::eina::stringshare>)
{
   return x.c_str();
}
inline const char* convert_to_c_impl(efl::eina::stringshare x, tag<const char*, efl::eina::stringshare, true>)
{
   return eina_stringshare_ref(x.c_str());
}
template <typename T, typename U, typename Deleter>
T* convert_to_c_impl(std::unique_ptr<U, Deleter>& v, tag<T*, std::unique_ptr<U, Deleter>>)
{
  return convert_to_c<T*, U*>(v.release());
}
template <typename T>
Eina_Array** convert_to_c_impl(efl::eina::array<T>& /*c*/, tag<Eina_Array **, efl::eina::array<T>&>)
{
  std::abort();
}
template <typename T>
Eina_Array** convert_to_c_impl(efl::eina::range_array<T>& /*c*/, tag<Eina_Array **, efl::eina::range_array<T>&>)
{
  std::abort();
}
template <typename T>
T* convert_to_c_impl(T const* p, tag<T*, T const*>) // needed for property_get
{
  return const_cast<T*>(p);
}
template <typename R, typename...Args>
typename std::add_pointer<R(Args...)>::type
convert_to_c_impl(std::function<R(Args...)> 
                  , tag
                  <
                  typename std::add_pointer<R(Args...)>::type
                  , std::function<R(Args...)>
                  >) // needed for property_get
{
  return nullptr; // not implemented naked functions
}
}

template <typename T, typename U, bool Own, typename V>
T convert_to_c(V&& object)
{
  return impl::convert_to_c_impl(std::forward<V>(object), impl::tag<T, U, Own>{});
}
namespace impl {
template <typename T>
struct is_range : std::false_type {};
template <typename T>
struct is_range<efl::eina::range_list<T>> : std::true_type {};
template <typename T>
struct is_range<efl::eina::range_array<T>> : std::true_type {};

template <typename T>
struct is_container : std::false_type {};
template <typename T>
struct is_container<efl::eina::list<T>> : std::true_type {};
template <typename T>
struct is_container<efl::eina::array<T>> : std::true_type {};
    
// event
template <typename T>
T convert_to_event(void* value, typename std::enable_if< eo::is_eolian_object<T>::value>::type* = 0)
{
  return T{::efl_ref(static_cast<Eo*>(value))};
}
template <typename T>
T convert_to_event(void* value, typename std::enable_if< is_container<T>::value
                   || is_range<T>::value>::type* = 0)
{
  return T{static_cast<typename T::native_handle_type>(value)};
}
template <typename T>
T convert_to_event(void* value, typename std::enable_if< !std::is_pointer<T>::value
                   && !is_container<T>::value && !is_range<T>::value
                   && !eo::is_eolian_object<T>::value>::type* = 0)
{
  return *static_cast<T*>(value);
}
template <typename T>
T convert_to_event(void* value, typename std::enable_if<std::is_same<T, bool>::value>::type* = 0)
{
  return *static_cast<Eina_Bool*>(value);
}
}
template <typename T>
T convert_to_event(void* value) { return impl::convert_to_event<T>(value); }
namespace impl {
    
template <typename T>
T convert_to_return(T value, tag<T, T>)
{
  return value;
}
template <typename U, typename T>
T convert_to_return(U const value, tag<U const, T>)
{
  return value;
}
template <typename T>
T& convert_to_return(T* value, tag<T*, T&>)
{
  return *value;
}
template <typename T>
T convert_to_return(Eo* value, tag<Eo*, T>, typename std::enable_if< eo::is_eolian_object<T>::value>::type* = 0)
{
  T v{eo::detail::ref(value)};
  return v;
}
template <typename T>
T convert_to_return(Eo const* value, tag<Eo const*, T>, typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return T{const_cast<Eo*>(eo::detail::ref(value))};
}
template <typename T>
eina::list<T> convert_to_return(Eina_List* value, tag<Eina_List*, eina::list<T>>)
{
  return eina::list<T>{value};
}
template <typename T>
eina::list<T> convert_to_return(Eina_List const* value, tag<Eina_List const*, eina::list<T>>)
{
  return eina::list<T>{const_cast<Eina_List*>(value)};
}
template <typename T>
eina::range_list<T> convert_to_return(Eina_List* value, tag<Eina_List*, eina::range_list<T>>)
{
  return eina::range_list<T>{value};
}
template <typename T>
eina::range_list<T> convert_to_return(Eina_List const* value, tag<Eina_List const*, eina::range_list<T>>)
{
  return eina::range_list<T>{const_cast<Eina_List*>(value)};
}
template <typename T>
eina::array<T> convert_to_return(Eina_Array* value, tag<Eina_Array*, eina::array<T>>)
{
  return eina::array<T>{value};
}
template <typename T>
eina::array<T> convert_to_return(Eina_Array const* value, tag<Eina_Array const*, eina::array<T>>)
{
  return eina::array<T>{const_cast<Eina_Array*>(value)};
}
template <typename T>
eina::range_array<T> convert_to_return(Eina_Array* value, tag<Eina_Array*, eina::range_array<T>>)
{
  return eina::range_array<T>{value};
}
template <typename T>
eina::range_array<T> convert_to_return(Eina_Array const* value, tag<Eina_Array const*, eina::range_array<T>>)
{
  return eina::range_array<T>{const_cast<Eina_Array*>(value)};
}
 
template <typename T>
eina::iterator<T> convert_to_return(Eina_Iterator* value, tag<Eina_Iterator*, eina::iterator<T>>)
{
  return eina::iterator<T>{ value };
}
template <typename T>
eina::accessor<T> convert_to_return(Eina_Accessor* value, tag<Eina_Accessor*, eina::accessor<T>>)
{
  return eina::accessor<T>{ value };
}
// Eina_Value*
inline efl::eina::value convert_to_return(Eina_Value* value, tag<Eina_Value*, efl::eina::value>)
{
  return efl::eina::value{value};
}
inline efl::eina::value_view convert_to_return(Eina_Value* value, tag<Eina_Value*, efl::eina::value_view>)
{
  return efl::eina::value_view{value};
}
template <typename T, typename U>
T convert_to_return(U* value, tag<T, U*>, typename std::enable_if<is_range<T>::value || is_container<T>::value>::type* = 0)
{
  // const should be to the type if value is const
  return T{const_cast<typename std::remove_const<U>::type*>(value)};
}
inline eina::stringshare convert_to_return(const Eina_Stringshare* value, tag<const char*, efl::eina::stringshare>)
{
  return efl::eina::stringshare(value);
}
template <typename T>
T convert_to_return(const char** /*value*/, tag<const char**, T>, typename std::enable_if<std::is_same<T, efl::eina::string_view*>::value>::type* = 0)
{
  std::abort();
}
inline eina::string_view convert_to_return(const char* value, tag<const char*, efl::eina::string_view>)
{
  return {value};
}
inline eina::string_view convert_to_return(const char** value, tag<const char**, efl::eina::string_view>)
{
  return {*value};
}
template <typename S>
inline std::string convert_to_return(const char* value, tag<const char*, S>
                                     , typename std::enable_if<std::is_same<typename std::remove_cv<S>::type, std::string>::value>::type* = 0)
{
  if(value)
    {
       std::string r{value};
       free((void*)value);
       return r;
    }
  else
    return {};
}
template <typename S>
inline std::string convert_to_return(const char** value, tag<const char**, S>
                                     , typename std::enable_if<std::is_same<typename std::remove_cv<S>::type, std::string>::value>::type* = 0)
{
  if(value)
    {
       std::string r{*value};
       free((void*)*value);
       free((void*)value);
       return r;
    }
  else
    return {};
}
template <typename S>
inline std::string convert_to_return(char* value, tag<char*, S>
                                     , typename std::enable_if<std::is_same<typename std::remove_cv<S>::type, std::string>::value>::type* = 0)
{
  if(value)
    {
       std::string r{value};
       free((void*)value);
       return r;
    }
  else
    return {};
}
template <typename S>
inline std::string convert_to_return(char** value, tag<char**, S>
                                     , typename std::enable_if<std::is_same<typename std::remove_cv<S>::type, std::string>::value>::type* = 0)
{
  if(value)
    {
       std::string r{*value};
       free((void*)*value);
       free((void*)value);
       return r;
    }
  else
    return {};
}
inline bool convert_to_return(Eina_Bool value, tag<Eina_Bool, bool>)
{
  return !!value;
}
template <typename T, typename D>
std::unique_ptr<T, D> convert_to_return(T* value, tag<T*, std::unique_ptr<T, D>>)
{
  return std::unique_ptr<T, D>{value, {}};
}
template <typename T, typename U, typename D>
std::unique_ptr<T, D> convert_to_return(U* value, tag<U*, std::unique_ptr<T, D>>)
{
  return std::unique_ptr<T, D>{convert_to_return(value, tag<U*, T*>{}), {}};
}
}

template <typename T, typename U>
T convert_to_return(U& object)
{
  return impl::convert_to_return(object, impl::tag<U, T>{});
}

/// Miscellaneous
template <typename T, typename U, typename Enable = void>
struct is_constructor_lambda : std::false_type {};
template <typename T, typename U>
struct is_constructor_lambda<T, U, decltype(std::declval<T>() ())> : std::true_type {};
template <typename T, typename U>
struct is_constructor_lambda<T, U, decltype(std::declval<T>() (std::declval<U>()))> : std::true_type {};

template <typename P>
inline void do_eo_add(Eo*& object, P const& parent
                      , Efl_Class const* klass
                      , typename std::enable_if< eo::is_eolian_object<P>::value>::type* = 0)
{
  bool const is_ref = true;
  object = ::_efl_add_internal_start(__FILE__, __LINE__, klass, parent._eo_ptr(), is_ref, EINA_FALSE);
  object = ::_efl_add_end(object, is_ref, EINA_FALSE);
}

template <typename T>
struct void_t { typedef void type; };

template <typename F, typename U>
auto call_lambda(F&& f, U&) -> typename void_t<decltype(f())>::type
{
  f();
}

template <typename F, typename U>
auto call_lambda(F&& f, U& object) -> typename void_t<decltype(f(object))>::type
{
  f(object);
}

template <typename P, typename F, typename U>
void do_eo_add(Eo*& object, P const& parent, Efl_Class const* klass
               , U& proxy
               , F&& f
               , typename std::enable_if< eo::is_eolian_object<P>::value>::type* = 0)
{
  bool is_ref = (parent._eo_ptr() != nullptr);
  object = ::_efl_add_internal_start(__FILE__, __LINE__, klass, parent._eo_ptr(), is_ref, EINA_FALSE);
  ::efl::eolian::call_lambda(std::forward<F>(f), proxy);
  object = ::_efl_add_end(object, is_ref, EINA_FALSE);
}

template <typename D, typename T>
struct light_address_of_operator
{
  operator T* () const { return static_cast<T*>(static_cast<void*>(static_cast<D const*>(this)->p)); }
};
template <typename D, typename T>
struct light_address_of_operator<D, T const>
{
  operator T const* () const { return static_cast<T const*>(static_cast<void const*>(static_cast<D const*>(this)->p)); }
};
    
template <typename T, typename...Args>
struct address_of_operator : light_address_of_operator<address_of_operator<T, Args...>, Args>...
{
  operator T* () { return p; };
  address_of_operator(T* p) : p(p) {}
  T* p;
};

template <typename T, typename...Args>
struct address_of_operator<T const, Args...> : light_address_of_operator<address_of_operator<T const, Args...>, Args>...
{
  operator T const* () { return p; };
  address_of_operator(T const* p) : p(p) {}
  T const* p;
};
    
} } // namespace efl { namespace eolian {

#endif // EFL_EOLIAN_INTEROP_HH
