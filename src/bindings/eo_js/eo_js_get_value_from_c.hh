#ifndef EFL_EO_JS_GET_VALUE_FROM_C_HH
#define EFL_EO_JS_GET_VALUE_FROM_C_HH

#include <v8.h>

#include <type_traits>
#include <cstdlib>
#include <typeinfo>

namespace efl { namespace eo { namespace js {

template <typename T> struct print_tag {};

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate
                 , typename std::enable_if<std::is_integral<T>::value && !std::is_same<T,bool>::value>::type* = 0)
{
  return v8::Integer::New(isolate, v);
}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate
                 , typename std::enable_if<std::is_same<T,bool>::value>::type* = 0)
{
  return v8::Boolean::New(isolate, v);
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
  return v8::Number::New(isolate, v);
}
      
inline v8::Local<v8::Value>
get_value_from_c(void*, v8::Isolate*)
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
                 , typename std::enable_if<std::is_pointer<T>::value>::type* = 0)
{
  std::cout << "derefering " << typeid(print_tag<T>).name() << std::endl;
  return get_value_from_c(*object, isolate);
}

} } }

#endif
