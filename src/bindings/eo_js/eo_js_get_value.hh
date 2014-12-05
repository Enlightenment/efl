#ifndef EFL_EO_JS_GET_VALUE_HH
#define EFL_EO_JS_GET_VALUE_HH

#if 0
#include <v8.h>
#else
#include <node/v8.h>
#endif

#include <type_traits>
#include <cstdlib>
#include <iostream>
#include <typeinfo>

namespace efl { namespace eo { namespace js {

template <typename T>
struct value_tag
{
  typedef T type;
};

template <typename T>
inline int get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , value_tag<T>
   , typename std::enable_if<(std::is_integral<T>::value && !std::is_same<T, Eina_Bool>::value)>::type* = 0)
{
  if(v->IsInt32())
    return v->Int32Value();
  else if(v->IsUint32())
    return v->Uint32Value();
  else
    {
#if 0
      isolate->
#else
        v8::
#endif
        ThrowException
        (v8::Exception::TypeError(v8::String::New/*FromUtf8*/(/*isolate,*/ "Type expected is different. Expected Integral type")));

      throw std::logic_error("");
    }
  return 0;
}

inline char* get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* /*isolate*/
   , value_tag<char*>)
{
  if(v->IsString())
    {
      v8::String::Utf8Value str(v->ToString());
      std::cerr << "String " << *str << std::endl;
      return const_cast<char*>(*str);
    }
  else
    {
#if 0
      isolate->
#else
        v8::
#endif
        ThrowException
        (v8::Exception::TypeError(v8::String::New/*FromUtf8*/(/*isolate,*/ "Type expected is different. Expected Integral type")));

      throw std::logic_error("");
    }
  return 0;
}
   
inline const char* get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , value_tag<const char*>)
{
  return get_value_from_javascript(v, isolate, value_tag<char*>());
}

template <typename T>
inline T get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* /*isolate*/
   , value_tag<T>
   , typename std::enable_if<std::is_enum<T>::value>::type* = 0)
{
  if(v->IsInt32())
    return static_cast<T>(v->Int32Value());
  else if(v->IsUint32())
    return static_cast<T>(v->Uint32Value());
  else
    {
#if 0
      isolate->
#else
        v8::
#endif
        ThrowException
        (v8::Exception::TypeError(v8::String::New/*FromUtf8*/(/*isolate,*/ "Type expected is different. Expected Integral type")));

      throw std::logic_error("");
    }
  return T();
}
      
inline int get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* /*isolate*/
   , value_tag<Eina_Bool>)
{
  if(v->IsBoolean() || v->IsBooleanObject())
    return v->BooleanValue();
  else
    {
#if 0
      isolate->
#else
        v8::
#endif
        ThrowException
        (v8::Exception::TypeError(v8::String::New/*FromUtf8*/(/*isolate,*/ "Type expected is different. Expected Boolean type")));

      throw std::logic_error("");
    }
  return 0;
}

template <typename T>
inline double get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , value_tag<T>
   , typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
{
  if(v->IsNumber())
    return v->NumberValue();
  else
    {
#if 0
      isolate->
#else
        v8::
#endif
        ThrowException
        (v8::Exception::TypeError(v8::String::New/*FromUtf8*/(/*isolate,*/ "Type expected is different. Expected floating point type")));
      throw std::logic_error("");
    }
}

template <typename T>
inline T get_value_from_javascript
  (v8::Local<v8::Value>, v8::Isolate*, value_tag<T>
   , typename std::enable_if<!std::is_floating_point<T>::value && !std::is_integral<T>::value
   && !std::is_enum<T>::value>::type* = 0)
{
  std::cerr << "Trying to convert to " << typeid(T).name() << " to call a C function" << std::endl;
  std::abort();
}
      
} } }

#endif
