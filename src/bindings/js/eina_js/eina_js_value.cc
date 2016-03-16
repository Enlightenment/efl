#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina_Js.hh>

namespace efl { namespace eina { namespace js {

namespace {

compatibility_return_type eina_value_set(compatibility_callback_info_type args)
{
  if (args.Length() != 1)
    return compatibility_return();

  void *ptr = js::compatibility_get_pointer_internal_field(args.Holder(), 0);
  v8::Isolate *isolate = args.GetIsolate();
  try {
    *static_cast<value*>(ptr) = value_cast<value>(args[0]);
  } catch(const std::bad_cast &e) {
    v8::Local<v8::Object> je = compatibility_new<v8::Object>(isolate);
    je->Set(compatibility_new<v8::String>(isolate, "code"),
            compatibility_new<v8::String>(isolate, "std::bad_cast"));
    return compatibility_throw(isolate, je);
  } catch(const ::efl::eina::system_error &e) {
    v8::Local<v8::Object> je = compatibility_new<v8::Object>(isolate);
    je->Set(compatibility_new<v8::String>(isolate, "code"),
            compatibility_new<v8::String>(isolate, "std::error_code"));
    je->Set(compatibility_new<v8::String>(isolate, "category"),
            compatibility_new<v8::String>(isolate, e.code().category().name()));
    je->Set(compatibility_new<v8::String>(isolate, "value"),
            compatibility_new<v8::Integer>(isolate, e.code().value()));
    return compatibility_throw(isolate, je);
  }
  return compatibility_return();
}

compatibility_return_type eina_value_get(compatibility_callback_info_type args)
{
  void *ptr = compatibility_get_pointer_internal_field(args.Holder(), 0);
  auto &value = *static_cast<eina::value*>(ptr);
  return compatibility_return
    (value_cast<v8::Local<v8::Value>>(value, args.GetIsolate()), args);
}

compatibility_return_type eina_value_constructor(compatibility_callback_info_type args)
{
  if (args.Length() != 1)
    return compatibility_return();

  v8::Isolate* isolate = args.GetIsolate();

  try {
    std::unique_ptr<value>
      ptr(new value(value_cast<value>(args[0])));
    compatibility_set_pointer_internal_field(args.This(), 0, ptr.get());
    auto v = ptr.get();
    if (v)
      efl::eina::js::make_weak(isolate, args.This(), [v]{ delete v; });
    ptr.release();
  } catch(const std::bad_cast &e) {
    v8::Local<v8::Object> je = compatibility_new<v8::Object>(isolate);
    je->Set(compatibility_new<v8::String>(isolate, "code"),
            compatibility_new<v8::String>(isolate, "std::bad_cast"));
    return compatibility_throw(isolate,je);
  } catch(const ::efl::eina::system_error &e) {
    v8::Local<v8::Object> je = compatibility_new<v8::Object>(isolate);
    je->Set(compatibility_new<v8::String>(isolate, "code"),
            compatibility_new<v8::String>(isolate, "std::error_code"));
    je->Set(compatibility_new<v8::String>(isolate, "category"),
            compatibility_new<v8::String>(isolate, e.code().category().name()));
    je->Set(compatibility_new<v8::String>(isolate, "value"),
            compatibility_new<v8::Integer>(isolate, e.code().value()));
    return compatibility_throw(isolate, je);
  }
  // makeweak
  // {
  //     typedef global_ref<v8::Object> persistent_t;
  //     typedef v8::WeakCallbackData<v8::Object, persistent_t> cb_type;

  //     auto on_gc = [](const cb_type &data) {
  //         typedef ::efl::eina::value value_type;
  //         typedef value_type *ptr_type;

  //         auto o = data.GetValue();
  //         delete static_cast<ptr_type>
  //             (compatibility_get_pointer_internal_field(o, 0));
  //         compatibility_set_pointer_internal_field<void*>(o, 0, nullptr);

  //         delete data.GetParameter();
  //     };

  //     auto persistent = new persistent_t(isolate, args.This());
  //     persistent->SetWeak<v8::Object>(persistent, on_gc);
  // }
}

}
      
EAPI
void register_value(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
  using v8::Isolate;
  using v8::Local;
  using v8::Value;
  using v8::Integer;
  using v8::String;
  using v8::Object;
  using v8::FunctionTemplate;
  using v8::FunctionCallbackInfo;

  v8::Local<v8::FunctionTemplate> constructor = compatibility_new<v8::FunctionTemplate>(isolate, &eina_value_constructor);
  
  v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();
  instance->SetInternalFieldCount(1);

  auto prototype = constructor->PrototypeTemplate();
  
  prototype->Set(compatibility_new<v8::String>(isolate, "set")
                 , compatibility_new<FunctionTemplate>(isolate, &eina_value_set));
  prototype->Set(compatibility_new<v8::String>(isolate, "get")
                 , compatibility_new<FunctionTemplate>(isolate, &eina_value_get));
  
  global->Set(name, constructor->GetFunction());
}

} } } // namespace efl { namespace js {

EAPI
void eina_value_register(v8::Handle<v8::Object> global, v8::Isolate* isolate)
{
  efl::eina::js::register_value(isolate, global
                                , efl::eina::js::compatibility_new<v8::String>(isolate, "value"));
}
