#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <eina_js_value.hh>
#include <eina_js_compatibility.hh>

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
  std::cout << __func__ << ':' << __LINE__ << std::endl;
  if (args.Length() != 1)
    return compatibility_return();

  std::cout << __func__ << ':' << __LINE__ << std::endl;
  v8::Isolate* isolate = args.GetIsolate();

  try {
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    std::unique_ptr<value>
      ptr(new value(value_cast<value>(args[0])));
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    compatibility_set_pointer_internal_field(args.This(), 0, ptr.get());
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    ptr.release();
  } catch(const std::bad_cast &e) {
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    v8::Local<v8::Object> je = compatibility_new<v8::Object>(isolate);
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    je->Set(compatibility_new<v8::String>(isolate, "code"),
            compatibility_new<v8::String>(isolate, "std::bad_cast"));
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    return compatibility_throw(isolate,je);
  } catch(const ::efl::eina::system_error &e) {
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    v8::Local<v8::Object> je = compatibility_new<v8::Object>(isolate);
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    je->Set(compatibility_new<v8::String>(isolate, "code"),
            compatibility_new<v8::String>(isolate, "std::error_code"));
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    je->Set(compatibility_new<v8::String>(isolate, "category"),
            compatibility_new<v8::String>(isolate, e.code().category().name()));
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    je->Set(compatibility_new<v8::String>(isolate, "value"),
            compatibility_new<v8::Integer>(isolate, e.code().value()));
  std::cout << __func__ << ':' << __LINE__ << std::endl;
    return compatibility_throw(isolate, je);
  }
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

  typedef ::efl::eina::value value_type;
  typedef value_type *ptr_type;

  static compatibility_persistent<v8::FunctionTemplate> constructor
     {isolate, compatibility_new<v8::FunctionTemplate>(isolate, &eina_value_constructor)};
  
  static compatibility_persistent<v8::ObjectTemplate>
    instance = {isolate, constructor.handle()->InstanceTemplate()};
  instance.handle()->SetInternalFieldCount(1);

  auto prototype = constructor.handle()->PrototypeTemplate();
  
  prototype->Set(compatibility_new<v8::String>(isolate, "set")
                 , compatibility_new<FunctionTemplate>(isolate, &eina_value_set));
  prototype->Set(compatibility_new<v8::String>(isolate, "get")
                 , compatibility_new<FunctionTemplate>(isolate, &eina_value_get));
  
  global->Set(name, constructor.handle()->GetFunction());
}

EAPI
void register_destroy_value(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    typedef ::efl::eina::value value_type;
    typedef value_type *ptr_type;

    auto dtor = [](compatibility_callback_info_type info) -> js::compatibility_return_type
      {
        if (info.Length() != 1)
          return compatibility_return();

        auto o = info[0]->ToObject();
        delete static_cast<ptr_type>(compatibility_get_pointer_internal_field(o, 0));
        compatibility_set_pointer_internal_field<void*>(o, 0, nullptr);
        assert(compatibility_get_pointer_internal_field(o, 0) == nullptr);
        return compatibility_return();
      };

    global->Set(name, compatibility_new<v8::FunctionTemplate>(isolate, dtor)->GetFunction());
}

} } } // namespace efl { namespace js {
