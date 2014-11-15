#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <eina_js_value.hh>

namespace efl { namespace js {

EAPI
void register_make_value(v8::Isolate *isolate, v8::Handle<v8::Object> global,
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

    auto ctor = [](const FunctionCallbackInfo<Value> &info) {
        auto set = [](const FunctionCallbackInfo<Value> &info) {
            if (info.Length() != 1)
                return;

            void *ptr = info.Holder()->GetAlignedPointerFromInternalField(0);
            Isolate *isolate = info.GetIsolate();
            try {
                *static_cast<ptr_type>(ptr) = value_cast<value_type>(info[0]);
            } catch(const std::bad_cast &e) {
                Local<Object> je = Object::New(isolate);
                je->Set(String::NewFromUtf8(isolate, "code"),
                        String::NewFromUtf8(isolate, "std::bad_cast"));
                isolate->ThrowException(je);
            } catch(const ::efl::eina::system_error &e) {
                Local<Object> je = Object::New(isolate);
                je->Set(String::NewFromUtf8(isolate, "code"),
                        String::NewFromUtf8(isolate, "std::error_code"));
                je->Set(String::NewFromUtf8(isolate, "category"),
                        String::NewFromUtf8(isolate, e.code().category().name()));
                je->Set(String::NewFromUtf8(isolate, "value"),
                        Integer::New(isolate, e.code().value()));
                isolate->ThrowException(je);
            }
        };
        auto get = [](const FunctionCallbackInfo<Value> &info) {
            void *ptr = info.Holder()->GetAlignedPointerFromInternalField(0);
            auto &value = *static_cast<ptr_type>(ptr);
            info.GetReturnValue().Set(value_cast<Local<Value>>
                                      (value, info.GetIsolate()));
        };

        if (info.Length() != 1)
            return;

        Isolate *isolate = info.GetIsolate();
        auto obj_tpl = v8::ObjectTemplate::New(isolate);
        obj_tpl->SetInternalFieldCount(1);

        auto ret = obj_tpl->NewInstance();
        info.GetReturnValue().Set(ret);

        ret->Set(String::NewFromUtf8(isolate, "set"),
                 FunctionTemplate::New(isolate, set)->GetFunction());
        ret->Set(String::NewFromUtf8(isolate, "get"),
                 FunctionTemplate::New(isolate, get)->GetFunction());

        try {
            std::unique_ptr<value_type>
                ptr(new value_type(value_cast<value_type>(info[0])));
            ret->SetAlignedPointerInInternalField(0, ptr.get());
            ptr.release();
        } catch(const std::bad_cast &e) {
                Local<Object> je = Object::New(isolate);
                je->Set(String::NewFromUtf8(isolate, "code"),
                        String::NewFromUtf8(isolate, "std::bad_cast"));
                isolate->ThrowException(je);
        } catch(const ::efl::eina::system_error &e) {
                Local<Object> je = Object::New(isolate);
                je->Set(String::NewFromUtf8(isolate, "code"),
                        String::NewFromUtf8(isolate, "std::error_code"));
                je->Set(String::NewFromUtf8(isolate, "category"),
                        String::NewFromUtf8(isolate, e.code().category().name()));
                je->Set(String::NewFromUtf8(isolate, "value"),
                        Integer::New(isolate, e.code().value()));
                isolate->ThrowException(je);
        }
    };

    global->Set(name, FunctionTemplate::New(isolate, ctor)->GetFunction());
}

EAPI
void register_destroy_value(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;

    typedef ::efl::eina::value value_type;
    typedef value_type *ptr_type;

    auto dtor = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1)
            return;

        auto o = info[0]->ToObject();
        delete static_cast<ptr_type>(o->GetAlignedPointerFromInternalField(0));
        o->SetAlignedPointerInInternalField(0, nullptr);
        assert(o->GetAlignedPointerFromInternalField(0) == nullptr);
    };

    global->Set(name, FunctionTemplate::New(isolate, dtor)->GetFunction());
}

} } // namespace efl { namespace js {
