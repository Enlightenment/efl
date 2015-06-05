#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ecore_js_throttle.hh>
#include <Ecore.h>

namespace efl { namespace ecore { namespace js {

EAPI
void register_throttle_adjust(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        ecore_throttle_adjust(args[0]->NumberValue());
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_throttle_get(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                           v8::Handle<v8::String> name)
{
    using v8::Number;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_throttle_get();
        return compatibility_return(compatibility_new<Number>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

} } } // namespace efl { namespace js {
