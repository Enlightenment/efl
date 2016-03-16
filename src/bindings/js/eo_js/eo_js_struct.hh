#ifndef EFL_EO_JS_STRUCT_HH
#define EFL_EO_JS_STRUCT_HH

#include <eina_tuple.hh>
#include <eina_tuple_c.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <vector>

namespace efl { namespace eo { namespace js {

template <typename S, typename M, M S::*MPtr, typename K>
eina::js::compatibility_accessor_getter_return_type
get_struct_member(v8::Local<v8::String>, eina::js::compatibility_accessor_getter_callback_info_type info)
{
  v8::Local<v8::Object> self_obj = eina::js::compatibility_cast<v8::Object>(info.This());
  auto sct = static_cast<S*>(eina::js::compatibility_get_pointer_internal_field(self_obj, 0));
  return eina::js::compatibility_return(eina::js::get_value_from_c(sct->*MPtr, info.GetIsolate(), K::class_name()), info);
}

template <typename S, typename GenTag, typename M, M S::*MPtr, typename K>
eina::js::compatibility_accessor_setter_return_type
set_struct_member(v8::Local<v8::String>, v8::Local<v8::Value> value, eina::js::compatibility_accessor_setter_callback_info_type info)
{
  v8::Local<v8::Object> self_obj = eina::js::compatibility_cast<v8::Object>(info.This());
  auto sct = static_cast<S*>(eina::js::compatibility_get_pointer_internal_field(self_obj, 0));
  sct->*MPtr = eina::js::get_value_from_javascript(value, info.GetIsolate(), K::class_name(), eina::js::value_tag<GenTag>());
}

template <typename S>
eina::js::compatibility_return_type new_struct(eina::js::compatibility_callback_info_type args)
{
  if (!args.IsConstructCall())
    return eina::js::compatibility_throw
       (nullptr, v8::Exception::TypeError
         (eina::js::compatibility_new<v8::String>(nullptr, "Not constructor call")));

  if(args.Length() == 0)
    {
      void* p = std::malloc(sizeof(S));
      std::memset(p, 0, sizeof(S));
      eina::js::compatibility_set_pointer_internal_field(args.This(), 0, p);
      efl::eina::js::make_weak(args.GetIsolate(), args.This(), [p]{ free(p); });
    }
  else
    {
      if (!args[0]->IsExternal())
        return eina::js::compatibility_throw
           (nullptr, v8::Exception::TypeError
             (eina::js::compatibility_new<v8::String>(nullptr, "Invalid argument type for constructor call")));

      S* p = reinterpret_cast<S*>(v8::External::Cast(*args[0])->Value());
      eina::js::compatibility_set_pointer_internal_field(args.This(), 0, static_cast<void*>(p));
    }

  return eina::js::compatibility_return();
}

template <typename S, typename F>
inline void register_struct_persistent(v8::Isolate* isolate, const char* name, const char* full_name, F&& fields_func)
{
  v8::Handle<v8::FunctionTemplate> constructor = eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eo::js::new_struct<S>);
  constructor->SetClassName(eina::js::compatibility_new<v8::String>(isolate, name));

  v8::Local<v8::ObjectTemplate> instance_t = constructor->InstanceTemplate();
  instance_t->SetInternalFieldCount(1);
  v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();

  fields_func(isolate, prototype);

  static efl::eina::js::global_ref<v8::ObjectTemplate> instance_persistent{isolate, instance_t};
  static efl::eina::js::global_ref<v8::Function> instance_template{isolate, constructor->GetFunction()};
  (void)instance_persistent;

  eina::js::register_class_constructor(full_name, instance_template.handle());
}

template <typename S, typename F>
void register_struct_exports(v8::Isolate* isolate, const char* name, v8::Handle<v8::Object> exports, F&& fields_func)
{
  v8::Handle<v8::FunctionTemplate> constructor = eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eo::js::new_struct<S>);
  constructor->SetClassName(eina::js::compatibility_new<v8::String>(isolate, name));

  v8::Local<v8::ObjectTemplate> instance_t = constructor->InstanceTemplate();
  instance_t->SetInternalFieldCount(1);
  v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();

  fields_func(isolate, prototype);

  exports->Set(eina::js::compatibility_new<v8::String>(isolate, name), constructor->GetFunction());
}


template <typename S, typename F>
void register_struct(v8::Isolate* isolate, const char* name, const char* full_name, v8::Handle<v8::Object> exports, F&& fields_func)
{
  register_struct_persistent<S>(isolate, name, full_name, std::forward<F>(fields_func));
  register_struct_exports<S>(isolate, name, exports, std::forward<F>(fields_func));
}

} } }

#endif
