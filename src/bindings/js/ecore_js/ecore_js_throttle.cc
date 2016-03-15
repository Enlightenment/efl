#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

namespace efl { namespace ecore { namespace js {

namespace {

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

}
      
EAPI
void register_ecore_throttle(v8::Isolate *isolate,v8::Handle<v8::Object> exports)
{
   register_throttle_adjust(isolate, exports,
                            compatibility_new<v8::String>(isolate,
                                                      "adjust"));
   register_throttle_get(isolate, exports,
                         compatibility_new<v8::String>(isolate,
                                                   "get"));
}
      
} } } // namespace efl { namespace js {
