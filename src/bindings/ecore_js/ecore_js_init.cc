#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ecore_js_init.hh>
#include <Ecore.h>

namespace efl { namespace ecore { namespace js {

EAPI
void register_init(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto init = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_init();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, init)
                ->GetFunction());
}

EAPI
void register_shutdown(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                       v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto shutdown = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ecore_shutdown();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, shutdown)
                ->GetFunction());
}

} } } // namespace efl { namespace js {
