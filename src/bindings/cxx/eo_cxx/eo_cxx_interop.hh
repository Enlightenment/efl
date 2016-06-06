
#ifndef EFL_EOLIAN_INTEROP_HH
#define EFL_EOLIAN_INTEROP_HH

#include <string>
#include <tuple>
#include <utility>
#include <type_traits>
#include <initializer_list>
#include <future>

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
template <>
struct in_traits<eina::string_view> { typedef eina::string_view type; };
template <>
struct in_traits<eina::string_view const> { typedef eina::string_view const type; };
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
struct out_traits { typedef T type; };
template <typename T>
struct out_traits<efl::eina::future<T>> { typedef efl::eina::future<T>& type; };

template <typename T>
struct inout_traits { typedef T type; };
template <typename T>
struct inout_traits<efl::eina::future<T>> { typedef efl::eina::future<T>& type; };

namespace impl {

template <typename From, typename To>
struct tag
{
  typedef To to;
  typedef From from;
};
  
template <typename To, typename From, typename Lhs, typename Rhs>
void assign_out(Lhs& lhs, Rhs& rhs);
  
template <typename T>
void assign_out_impl(T*& lhs, T& rhs, tag<T*, T>)
{
  *lhs = rhs;
}
template <typename U, typename T>
void assign_out_impl(std::unique_ptr<T, void(*)(const void*)>& lhs, U* rhs, tag<std::unique_ptr<T, void(*)(const void*)>&, U*>)
{
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
template <typename T>
void assign_out_impl(efl::eina::future<T>& /*v*/, Eina_Promise*, tag<efl::eina::future<T>&, Eina_Promise*>)
{
}
template <typename Tag>
void assign_out_impl(efl::eina::string_view& view, const char* string, Tag)
{
  view = {string};
}
template <typename T, typename Rhs, typename U, typename O>
void assign_out_impl(efl::eina::optional<T>& lhs, Rhs*& rhs, tag<efl::eina::optional<U>&, O>)
{
  if(rhs)
    assign_out<U, O>(*lhs, rhs);
  else
    lhs.disengage();
}
template <typename T, typename Rhs, typename U, typename O>
void assign_out_impl(efl::eina::optional<T>& lhs, Rhs& rhs, tag<efl::eina::optional<U>&, O>)
{
  assign_out<U, O>(*lhs, rhs);
}
template <typename T, typename Rhs, typename U, typename O>
void assign_out_impl(efl::eina::optional<T>& lhs, Rhs*& rhs, tag<efl::eina::optional<U>, O>)
{
  if(rhs)
    assign_out<U, O>(*lhs, rhs);
  else
    lhs.disengage();
}
template <typename T, typename Rhs, typename U, typename O>
void assign_out_impl(efl::eina::optional<T>& lhs, Rhs& rhs, tag<efl::eina::optional<U>, O>)
{
  assign_out<U, O>(*lhs, rhs);
}
template <typename Tag>
void assign_out_impl(eina::value& lhs, Eina_Value const& rhs, Tag)
{
  Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_CHAR);
  eina_value_flush(v);
  eina_value_copy(&rhs, v);
  lhs = {v};
}
template <typename T, typename Tag>
void assign_out_impl(efl::eina::list<T>& lhs, Eina_List* rhs, Tag)
{
  lhs = efl::eina::list<T>{rhs};
}
}
    
template <typename To, typename From, typename Lhs, typename Rhs>
void assign_out(Lhs& lhs, Rhs& rhs)
{
  return impl::assign_out_impl(lhs, rhs, impl::tag<To, From>{});
}
    
namespace impl {

template <typename T>
T* convert_inout_impl(T& v, tag<T, T*>)
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
template <typename T>
Eina_Promise* convert_inout_impl(efl::eina::future<T>& /*v*/, tag<efl::eina::future<T>, Eina_Promise*>)
{
  return nullptr;
}
}
    
template <typename To, typename From, typename V>
auto convert_inout(V& object) -> decltype(impl::convert_inout_impl(object, impl::tag<From, To>{}))
{
  return impl::convert_inout_impl(object, impl::tag<From, To>{});
}
    
template <typename T, typename U, typename V>
T convert_to_c(V&& object);
    
namespace impl {

template <typename U, typename T>
auto convert_to_c_impl
(T&& v, tag<U, U>, typename std::enable_if<std::is_same<typename std::remove_reference<T>::type, U>::value>::type* =0) -> decltype(std::forward<T>(v))
{
  return std::forward<T>(v);
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
Eo const* convert_to_c_impl(T v, tag<Eo const*, T>
                            , typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return v._eo_ptr();
}
inline const char* convert_to_c_impl( ::efl::eina::string_view v, tag<const char*, ::efl::eina::string_view>)
{
  return v.c_str();
}
inline const char** convert_to_c_impl(efl::eina::string_view* /*view*/, tag<const char **, efl::eina::string_view*>)
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
inline Eina_Value const* convert_to_c_impl( ::efl::eina::value v, tag<Eina_Value const*, in_traits<eina::value>::type>)
{
  Eina_Value* nv = eina_value_new(v.type_info());
  eina_value_copy(v.native_handle(), nv);
  return nv;
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
Eina_List* convert_to_c_impl(efl::eina::list<T>const& c, tag<Eina_List *, efl::eina::list<T>const&>)
{
  return const_cast<Eina_List*>(c.native_handle());
}
template <typename T>
Eina_List const* convert_to_c_impl(efl::eina::list<T>const& c, tag<Eina_List const *, efl::eina::list<T>const&>)
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
Eina_Array* convert_to_c_impl(efl::eina::array<T>const& c, tag<Eina_Array *, efl::eina::array<T>const&>)
{
  return c.native_handle();
}
template <typename T>
Eina_Array const* convert_to_c_impl(efl::eina::array<T>const& c, tag<Eina_Array const *, efl::eina::array<T>const&>)
{
  return c.native_handle();
}
inline const char** convert_to_c_impl(efl::eina::string_view /*view*/, tag<char const **, efl::eina::string_view>)
{
  std::abort();
}
// inline const char* convert_to_c_impl(std::string const& x, tag<const char*, std::string>)
// {
//    return x.c_str();
// }
inline const char* convert_to_c_impl(efl::eina::stringshare x, tag<const char*, efl::eina::stringshare>)
{
   return x.c_str();
}
template <typename T>
Eina_Promise* convert_to_c_impl(efl::eina::future<T> const&, tag<Eina_Promise*, efl::eina::future<T>const&>)
{
  std::abort();
}
template <typename T, typename U, typename Deleter>
T* convert_to_c_impl(std::unique_ptr<U, Deleter>& v, tag<T*, std::unique_ptr<U, Deleter>>)
{
  return convert_to_c<T*, U*>(v.release());
}

template <typename T>
Eina_Array** convert_to_c_impl(efl::eina::array<T>& c, tag<Eina_Array **, efl::eina::array<T>&>)
{
  std::abort();
}
template <typename T>
Eina_Array** convert_to_c_impl(efl::eina::range_array<T>& c, tag<Eina_Array **, efl::eina::range_array<T>&>)
{
  std::abort();
}
}
    
template <typename T, typename U, typename V>
T convert_to_c(V&& object)
{
  return impl::convert_to_c_impl(std::forward<V>(object), impl::tag<T, U>{});
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
  return T{::eo_ref(static_cast<Eo*>(value))};
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
template <typename T>
T convert_to_return(Eo* value, tag<Eo*, T>, typename std::enable_if< eo::is_eolian_object<T>::value>::type* = 0)
{
  return T{value};
}
template <typename T>
T convert_to_return(Eo const* value, tag<Eo const*, T>, typename std::enable_if<eo::is_eolian_object<T>::value>::type* = 0)
{
  return T{const_cast<Eo*>(value)};
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
  return eina::iterator<T>{value};
}
template <typename T>
struct is_future : std::false_type {};
template <typename T>
struct is_future<efl::eina::future<T>> : std::true_type {};
template <typename T>
T convert_to_return(Eina_Promise* /*value*/, tag<Eina_Promise*, T>, typename std::enable_if<is_future<T>::value>::type* = 0)
{
  std::abort();
  return {};
}
// Eina_Value*
inline efl::eina::value convert_to_return(Eina_Value* value, tag<Eina_Value*, efl::eina::value>)
{
  return efl::eina::value{value};
}
template <typename T, typename U>
T convert_to_return(U* value, tag<T, U*>, typename std::enable_if<is_range<T>::value || is_container<T>::value>::type* = 0)
{
  // const should be to the type if value is const
  return T{const_cast<typename std::remove_const<U>::type*>(value)};
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
inline std::string convert_to_return(const char* value, tag<const char*, std::string>)
{
  return {value};
}
inline bool convert_to_return(Eina_Bool value, tag<Eina_Bool, bool>)
{
  return !!value;
}
template <typename T>
std::unique_ptr<T, void(*)(const void*)> convert_to_return(T* value, tag<T*, std::unique_ptr<T, void(*)(const void*)>>)
{
  return std::unique_ptr<T, void(*)(const void*)>{value, (void(*)(const void*))&free};
}
template <typename T, typename U>
std::unique_ptr<T, void(*)(const void*)> convert_to_return(U* value, tag<U*, std::unique_ptr<T, void(*)(const void*)>>)
{
  return std::unique_ptr<T, void(*)(const void*)>{convert_to_return(value, tag<U*, T*>{}), (void(*)(const void*))&free};
}
}

template <typename T, typename U>
T convert_to_return(U& object)
{
  return impl::convert_to_return(object, impl::tag<U, T>{});
}

/// Miscellaneous
template <typename T, typename Enable = void>
struct is_callable : std::false_type {};
template <typename T>
struct is_callable<T, decltype(std::declval<T>() ())> : std::true_type {};

inline void do_eo_add(Eo*& object, efl::eo::concrete const& parent
                      , Eo_Class const* klass)
{
  object = ::_eo_add_internal_start(__FILE__, __LINE__, klass, parent._eo_ptr(), EINA_TRUE, EINA_FALSE);
  object = ::_eo_add_end(object, EINA_FALSE);
}
template <typename F>
void do_eo_add(Eo*& object, efl::eo::concrete const& parent, Eo_Class const* klass, F f)
{
  object = ::_eo_add_internal_start(__FILE__, __LINE__, klass, parent._eo_ptr(), EINA_TRUE, EINA_FALSE);
  f();
  object = ::_eo_add_end(object, EINA_FALSE);
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
