#ifndef EINA_JS_ACCESSOR_HH
#define EINA_JS_ACCESSOR_HH

#include <Eina.hh>
#include EINA_STRINGIZE(V8_INCLUDE_HEADER)
#include <memory>

#include <eina_js_value.hh>
#include <eina_js_compatibility.hh>

namespace efl { namespace eina { namespace js {

/* Creates a copy from \p a accessor and exports it to be manipulated by the JS
   code */
template<class T>
v8::Local<v8::Object> export_accessor(::efl::eina::accessor<T> &a,
                                      v8::Isolate *isolate)
  ;
// {
//     using v8::Local;
//     using v8::Value;
//     using v8::String;
//     using v8::FunctionCallbackInfo;
//     using v8::FunctionTemplate;
//     using v8::ObjectTemplate;

//     typedef ::efl::eina::accessor<T> value_type;
//     typedef value_type *ptr_type;
//     typedef void (*deleter_t)(void*);

//     auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
//     obj_tpl->SetInternalFieldCount(2);

//     auto ret = obj_tpl->NewInstance();

//     auto get = [](const FunctionCallbackInfo<Value> &info) {
//         if (info.Length() != 1 || !info[0]->IsNumber())
//             return;

//         auto idx = [&info]() -> std::size_t {
//             auto idx = info[0];

//             if (idx->IsInt32())
//                 return idx->ToInt32()->Value();
//             else if (idx->IsUint32())
//                 return idx->ToUint32()->Value();
//             else
//                 return idx->ToNumber()->Value();
//         }();

//         void *ptr = info.Holder()->GetAlignedPointerFromInternalField(0);
//         auto &value = *static_cast<ptr_type>(ptr);
//         info.GetReturnValue().Set(value_cast<Local<Value>>(value[idx],
//                                                            info.GetIsolate()));
//     };

//     ret->Set(String::NewFromUtf8(isolate, "get"),
//              FunctionTemplate::New(isolate, get)->GetFunction());

//     {
//         deleter_t deleter = [](void *a) {
//             delete static_cast<ptr_type>(a);
//         };
//         std::unique_ptr<value_type> ptr(new value_type(a));
//         ret->SetAlignedPointerInInternalField(0, ptr.get());
//         ret->SetAlignedPointerInInternalField(1,
//                                               reinterpret_cast<void*>(deleter));
//         ptr.release();
//     }

//     return ret;
// }

/* Extracts and returns a copy from the internal accessor object from the JS
   object */
template<class T>
::efl::eina::accessor<T> import_accessor(v8::Handle<v8::Object> o)
  ;
// {
//     typedef ::efl::eina::accessor<T> value_type;
//     typedef value_type *ptr_type;

//     ptr_type acc = reinterpret_cast<ptr_type>
//         (o->GetAlignedPointerFromInternalField(0));

//     return value_type(*acc);
// }

/* Registers the function to destroy the accessor objects to the JS code */
void register_destroy_accessor(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

} } } // namespace efl::js

#endif /* EINA_JS_ACCESSOR_HH */
