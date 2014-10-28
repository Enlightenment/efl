#ifndef EFL_EO_JS_GET_VALUE_HH
#define EFL_EO_JS_GET_VALUE_HH

namespace efl { namespace eo { namespace js {

template <typename T>
struct value_tag
{
  typedef T type;
};

inline int get_value_from_javascript(v8::Local<v8::Value> v, value_tag<int>)
{
  return 0;
}
inline double get_value_from_javascript(v8::Local<v8::Value> v, value_tag<double>)
{
  return 0.0;
}
      
} } }

#endif
