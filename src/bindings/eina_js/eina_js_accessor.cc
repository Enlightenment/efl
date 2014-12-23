#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <eina_js_accessor.hh>

namespace efl { namespace js {

EAPI void register_destroy_accessor(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Object;
    using v8::Handle;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;

    typedef void (*deleter_t)(void*);

    auto f = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsObject())
            return;

        Handle<Object> o = info[0]->ToObject();

        deleter_t deleter = reinterpret_cast<deleter_t>
            (o->GetAlignedPointerFromInternalField(1));
        deleter(o->GetAlignedPointerFromInternalField(0));
    };

    global->Set(name, FunctionTemplate::New(isolate, f)->GetFunction());
}

} } // namespace efl { namespace js {
