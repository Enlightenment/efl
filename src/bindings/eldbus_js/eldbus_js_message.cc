#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eldbus_Js.hh>

namespace efl { namespace eldbus { namespace js {

namespace {

void register_message_method_call_new(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 4 || !args[0]->IsString() || !args[1]->IsString()
            || !args[2]->IsString() || !args[3]->IsString()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();
        auto ret = eldbus_message_method_call_new(*String::Utf8Value(args[0]),
                                                  *String::Utf8Value(args[1]),
                                                  *String::Utf8Value(args[2]),
                                                  *String::Utf8Value(args[3]));
        return compatibility_return(wrap_eldbus_msg(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_message_error_new(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 3 || !args[0]->IsObject() || !args[1]->IsString()
            || !args[2]->IsString()) {
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();
        auto ret = eldbus_message_error_new(extract_eldbus_msg(args[0]
                                                               ->ToObject()),
                                            *String::Utf8Value(args[1]),
                                            *String::Utf8Value(args[2]));
        return compatibility_return(wrap_eldbus_msg(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_message_method_return_new(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsObject())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto eldbus_msg = extract_eldbus_msg(args[0]->ToObject());
        auto ret = eldbus_message_method_return_new(eldbus_msg);
        return compatibility_return(wrap_eldbus_msg(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

}

EAPI
void register_eldbus_message(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
  register_message_method_call_new
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "message_method_call_new"));

  register_message_error_new
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "message_error_new"));

  register_message_method_return_new
    (isolate, exports, compatibility_new<v8::String>
     (isolate, "message_method_return_new"));
}

} } } // namespace efl { namespace eldbus { namespace js {
