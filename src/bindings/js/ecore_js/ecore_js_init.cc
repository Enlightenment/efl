#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

namespace efl { namespace ecore { namespace js {

namespace {

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
        auto ret = ::ecore_init();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, init)
                ->GetFunction());
}

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

}

EAPI
void register_ecore(v8::Isolate *isolate,v8::Handle<v8::Object> exports)
{
   efl::ecore::js::register_init(isolate, exports,
                                 efl::eina::js::compatibility_new<v8::String>
                                 (isolate, "ecore_init"));
   efl::ecore::js::register_shutdown(isolate, exports,
                                     efl::eina::js::compatibility_new<v8::String>
                                     (isolate, "ecore_shutdown"));
   register_ecore_animator(isolate, exports);
   register_ecore_event(isolate, exports);
   register_ecore_idle(isolate, exports);
   register_ecore_job(isolate, exports);
   register_ecore_mainloop(isolate, exports);
   register_ecore_poller(isolate, exports);
   register_ecore_throttle(isolate, exports);
   register_ecore_timer(isolate, exports);
}
      
} } } // namespace efl { namespace js {
