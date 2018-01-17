#ifndef EFL_EO_JS_NAMESPACE_ACCESSOR_HH
#define EFL_EO_JS_NAMESPACE_ACCESSOR_HH

namespace efl { namespace eo { namespace js {

inline void accessor_get(v8::Local<v8::Name> name, v8::PropertyCallbackInfo<v8::Value> const& info)
{
  v8::Local<v8::String> name_str = name->ToString();
  
}

// inline v8::Local<v8::Object> create_namespace_object()
// {
  
// }

      
} } }

#endif
