#ifndef EINA_JS_ACCESSOR_HH
#define EINA_JS_ACCESSOR_HH

#include <memory>

#include <eina_js_value.hh>
#include <eina_js_compatibility.hh>
#include <eina_js_get_value.hh>
#include <eina_js_get_value_from_c.hh>

#include <iostream>

namespace efl { namespace eina { namespace js {

template <typename T, typename A>
js::compatibility_return_type accessor_get(js::compatibility_callback_info_type info)
{
  v8::Isolate* isolate = info.GetIsolate();
  if (info.Length() != 1 || !info[0]->IsNumber())
    return js::compatibility_return();

  auto idx = js::get_value_from_javascript
    (info[0], isolate, "", js::value_tag<std::size_t>());

  std::string class_name;
  if (info.Data()->IsString())
    {
       v8::String::Utf8Value str(info.Data());
       class_name = *str;
    }

  void *ptr = compatibility_get_pointer_internal_field(info.Holder(), 0);

  return compatibility_return
    (::efl::eina::js::get_value_from_c
      (js::wrap_value<T>(container_unwrap((*static_cast<A*>(ptr))[idx]), js::value_tag<T>{})
                         , isolate, class_name.c_str()), info);
};

/* Creates a copy from \p a accessor and exports it to be manipulated by the JS
   code */
template <typename T, typename W>
inline v8::Local<v8::Object> export_accessor(::efl::eina::accessor<W> &a, v8::Isolate *isolate, const char* class_name)
{
  typedef ::efl::eina::accessor<W> accessor_type;

  static efl::eina::js::global_ref<v8::ObjectTemplate> obj_tpl
    (isolate, 
    [&]() 
    {
      auto obj_tpl = compatibility_new<v8::ObjectTemplate>(isolate);
      (*obj_tpl)->SetInternalFieldCount(1);

      // TODO: (*obj_tpl)->SetIndexedPropertyHandler(&accessor_get<accessor_type>);

      (*obj_tpl)->Set(js::compatibility_new<v8::String>(isolate, "get")
                      , js::compatibility_new<v8::FunctionTemplate>(isolate
                                                                    , &accessor_get<T, accessor_type>
                                                                    , js::compatibility_new<v8::String>(isolate, class_name)));
      return obj_tpl;
    }());

  auto instance = obj_tpl.handle()->NewInstance();
  compatibility_set_pointer_internal_field(instance, 0, &a);
  return instance;
}

/* Extracts and returns a copy from the internal accessor object from the JS
   object */
template <typename T>
::efl::eina::accessor<T>& import_accessor(v8::Handle<v8::Object> o)
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
