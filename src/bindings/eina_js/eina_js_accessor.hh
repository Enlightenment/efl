#ifndef EINA_JS_ACCESSOR_HH
#define EINA_JS_ACCESSOR_HH

#include <Eina.hh>
#include EINA_STRINGIZE(V8_INCLUDE_HEADER)
#include <memory>

#include <eina_js_value.hh>
#include <eina_js_compatibility.hh>
#include <eina_js_get_value.hh>
#include <eina_js_get_value_from_c.hh>

#include <iostream>

namespace efl { namespace eina { namespace js {

template <typename T>
js::compatibility_return_type accessor_get(js::compatibility_callback_info_type info)
{
  std::cout << __func__ << ":" << __LINE__ << std::endl;
  v8::Isolate* isolate = info.GetIsolate();
  if (info.Length() != 1 || !info[0]->IsNumber())
    return js::compatibility_return();

  auto idx = js::get_value_from_javascript
    (info[0], isolate, js::value_tag<std::size_t>());

  std::cout << __func__ << ":" << __LINE__ << std::endl;
  void *ptr = compatibility_get_pointer_internal_field(info.Holder(), 0);
  std::cout << __func__ << ":" << __LINE__ << std::endl;
  return compatibility_return
    (js::get_value_from_c((*static_cast<T*>(ptr))[idx], isolate), info);
};
      
/* Creates a copy from \p a accessor and exports it to be manipulated by the JS
   code */
template<class T>
v8::Local<v8::Object> export_accessor( ::efl::eina::accessor<T> &a, v8::Isolate *isolate)
{
  typedef ::efl::eina::accessor<T> accessor_type;
    
  static auto obj_tpl = [&]() -> compatibility_persistent<v8::ObjectTemplate>
    {
      auto obj_tpl = compatibility_new<v8::ObjectTemplate>(isolate);
      obj_tpl->SetInternalFieldCount(1);

      obj_tpl->Set(js::compatibility_new<v8::String>(isolate, "get")
                   , js::compatibility_new<v8::FunctionTemplate>(isolate, &accessor_get<accessor_type>));
  
      return {isolate, obj_tpl};
    }();

  auto instance = obj_tpl.handle()->NewInstance();
  compatibility_set_pointer_internal_field(instance, 0, &a);
  return instance;
}

/* Extracts and returns a copy from the internal accessor object from the JS
   object */
template<class T>
::efl::eina::accessor<T> import_accessor(v8::Handle<v8::Object> o)
{
  typedef ::efl::eina::accessor<T> accessor_type;

  void* ptr = compatibility_get_pointer_internal_field(o, 0);
  
  return *static_cast<accessor_type*>(ptr);
}

/* Registers the function to destroy the accessor objects to the JS code */
void register_destroy_accessor(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

} } } // namespace efl::js

#endif /* EINA_JS_ACCESSOR_HH */
