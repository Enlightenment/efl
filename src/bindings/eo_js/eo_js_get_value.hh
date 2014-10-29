#ifndef EFL_EO_JS_GET_VALUE_HH
#define EFL_EO_JS_GET_VALUE_HH

#include <v8.h>

#include <type_traits>

namespace efl { namespace eo { namespace js {

template <typename T>
struct value_tag
{
  typedef T type;
};

template <typename T>
inline int get_value_from_javascript
  (v8::Local<v8::Value> v, value_tag<T>
   , typename std::enable_if<std::is_integral<T>::value>::type* = 0)
{
  if(v->IsInt32())
    return v->Int32Value();
  else if(v->IsUint32())
    return v->Uint32Value();
  else
    std::abort();
  return 0;
}
template <typename T>
inline double get_value_from_javascript
  (v8::Local<v8::Value> v, value_tag<T>
   , typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
{
  if(v->IsNumber())
    return v->NumberValue();
  else
    std::abort();
}
      
} } }

#endif
