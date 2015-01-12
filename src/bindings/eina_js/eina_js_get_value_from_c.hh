#ifndef EFL_EINA_JS_GET_VALUE_FROM_C_HH
#define EFL_EINA_JS_GET_VALUE_FROM_C_HH

#include <eina_js_compatibility.hh>

#include EINA_STRINGIZE(V8_INCLUDE_HEADER)

#include <type_traits>
#include <cstdlib>
#include <typeinfo>

namespace efl { namespace eina { namespace js {

template <typename T> struct print_tag {};

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate
                 , typename std::enable_if<std::is_integral<T>::value && !std::is_same<T,bool>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Integer>(isolate, v);
}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate
                 , typename std::enable_if<std::is_same<T,bool>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Boolean>(isolate, v);
}

template <typename T>
inline v8::Local<T>
get_value_from_c(v8::Local<T> v, v8::Isolate*)
{
  return v;
}
      
// template <typename T>
// inline v8::Local<v8::Value>
// get_value_from_c(T v, v8::Isolate* isolate
//                  , typename std::enable_if<std::is_integral<T>::value>::type* = 0)
// {
//   return v8::Integer::New(isolate, v);
// }
      
template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate
                 , typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Number>(isolate, v);
}

inline v8::Local<v8::Value>
get_value_from_c(const char* v, v8::Isolate* isolate)
{
  return eina::js::compatibility_new<v8::String>(isolate, v);
}

inline v8::Local<v8::Value>
get_value_from_c(char* v, v8::Isolate* isolate)
{
  return js::get_value_from_c(const_cast<const char*>(v), isolate);
}
      
inline v8::Local<v8::Value>
get_value_from_c(void*, v8::Isolate*)
{
  std::cout << "aborting because we don't know type void*" << std::endl;
  std::abort();
}

inline v8::Local<v8::Value>
get_value_from_c(const void*, v8::Isolate*)
{
  std::cout << "aborting because we don't know type void*" << std::endl;
  std::abort();
}
      
template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T&, v8::Isolate*
                 , typename std::enable_if
                 <!std::is_pointer<T>::value
                 && !std::is_integral<T>::value
                 && !std::is_floating_point<T>::value
                 >::type* = 0)
{
  std::cout << "aborting because we don't know type " << typeid(print_tag<T>).name() << std::endl;
  std::abort();
}
      
template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T& object, v8::Isolate* isolate
                 , typename std::enable_if<std::is_pointer<T>::value
                 && !(std::is_same<T, char*>::value || std::is_same<T, const char*>::value)>::type* = 0)
{
  std::cout << "derefering " << typeid(print_tag<T>).name() << std::endl;
  return get_value_from_c(*object, isolate);
}

} } }

#endif
