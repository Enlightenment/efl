#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

namespace efl { namespace ecore { namespace js {

namespace {
      
void register_callback_cancel(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::Boolean;
    global->Set(name, compatibility_new<Boolean>(isolate,
                                                 bool{ECORE_CALLBACK_CANCEL}));
}

void register_callback_renew(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Boolean;
    global->Set(name, compatibility_new<Boolean>(isolate,
                                                 bool{ECORE_CALLBACK_RENEW}));
}

void register_callback_pass_on(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Boolean;
    global->Set(name, compatibility_new<Boolean>(isolate,
                                                 bool{ECORE_CALLBACK_PASS_ON}));
}

void register_callback_done(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::Boolean;
    global->Set(name, compatibility_new<Boolean>(isolate,
                                                 bool{ECORE_CALLBACK_DONE}));
}

void register_mainloop_iterate(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        ecore_main_loop_iterate();
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_mainloop_iterate_may_block(v8::Isolate *isolate,
                                         v8::Handle<v8::Object> global,
                                         v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        auto ret = ecore_main_loop_iterate_may_block(args[0]->NumberValue());
        return compatibility_return(compatibility_new<Integer>
                                    (args.GetIsolate(), ret), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_mainloop_begin(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        ecore_main_loop_begin();
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_mainloop_quit(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        ecore_main_loop_quit();
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_mainloop_animator_ticked_get(v8::Isolate *isolate,
                                           v8::Handle<v8::Object> global,
                                           v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto ret = ecore_main_loop_animator_ticked_get();
        return compatibility_return(compatibility_new<Boolean>
                                    (args.GetIsolate(), ret), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_mainloop_nested_get(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto ret = ecore_main_loop_nested_get();
        return compatibility_return(compatibility_new<Boolean>
                                    (args.GetIsolate(), ret), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

}
      
EAPI
void register_ecore_mainloop(v8::Isolate *isolate,v8::Handle<v8::Object> exports)
{
   register_callback_cancel(isolate, exports,
                            compatibility_new<v8::String>
                            (isolate, "CALLBACK_CANCEL"));
   register_callback_renew(isolate, exports,
                           compatibility_new<v8::String>
                           (isolate, "CALLBACK_RENEW"));
   register_callback_pass_on(isolate, exports,
                             compatibility_new<v8::String>
                             (isolate, "CALLBACK_PASS_ON"));
   register_callback_done(isolate, exports,
                          compatibility_new<v8::String>
                          (isolate, "CALLBACK_DONE"));
   register_mainloop_iterate(isolate, exports,
                             compatibility_new<v8::String>
                             (isolate, "iterate"));
   register_mainloop_iterate_may_block(isolate, exports,
                                       compatibility_new<v8::String>
                                       (isolate,
                                        "iterateMayBlock"));
   register_mainloop_begin(isolate, exports,
                           compatibility_new<v8::String>
                           (isolate, "begin"));
   register_mainloop_quit(isolate, exports,
                          compatibility_new<v8::String>
                          (isolate, "quit"));
   register_mainloop_animator_ticked_get(isolate, exports,
                                         compatibility_new<v8::String>
                                         (isolate,
                                          "getAnimatorTicked"));
   register_mainloop_nested_get(isolate, exports,
                                compatibility_new<v8::String>
                                (isolate, "getNested"));
}
  
} } } // namespace efl { namespace js {
