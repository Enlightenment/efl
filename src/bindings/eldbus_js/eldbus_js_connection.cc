#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eldbus_Js.hh>

namespace efl { namespace eldbus { namespace js {

namespace {
      
v8::Local<v8::Object> wrap_event_callback(Eldbus_Connection *conn,
                                          Eldbus_Connection_Event_Type type,
                                          Eldbus_Connection_Event_Cb cb,
                                          void *cb_data, v8::Isolate *isolate)
{
    using v8::String;
    using v8::Integer;
    using v8::Value;
    using v8::Object;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(3);
    auto ret = obj_tpl->NewInstance();

    ret->Set(compatibility_new<String>(isolate, "_type"),
             compatibility_new<Integer>(isolate, type));

    compatibility_set_pointer_internal_field(ret, 0, conn);
    compatibility_set_pointer_internal_field(ret, 1, (void*)(cb));
    compatibility_set_pointer_internal_field(ret, 2, cb_data);

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        auto o = info.This();
        auto isolate = info.GetIsolate();

        auto conn
            = (compatibility_get_pointer_internal_field<Eldbus_Connection*>
               (o, 0));
        auto cb
            = (compatibility_get_pointer_internal_field
               <Eldbus_Connection_Event_Cb>(o, 1));
        auto cb_data
            = (compatibility_get_pointer_internal_field
               <efl::eina::js::global_ref<Value>*>(o, 2));
        auto type
            = (static_cast<Eldbus_Connection_Event_Type>
               (o->Get(compatibility_new<String>(isolate, "_type"))
                ->IntegerValue()));

        eldbus_connection_event_callback_del(conn, type, cb, cb_data);

        delete cb_data;
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    return ret;
}

Eldbus_Connection* extract_eldbus_connection(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Eldbus_Connection*>(o, 0);
}

void register_timeout_infinite(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate,
                                                 ELDBUS_TIMEOUT_INFINITE));
}

void register_connection_type_unknown(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_CONNECTION_TYPE_UNKNOWN));
}

void register_connection_type_session(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_CONNECTION_TYPE_SESSION));
}

void register_connection_type_system(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_CONNECTION_TYPE_SYSTEM));
}

void register_connection_type_starter(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_CONNECTION_TYPE_STARTER));
}

void register_connection_type_address(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_CONNECTION_TYPE_ADDRESS));
}

void register_connection_type_last(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>(isolate,
                                           ELDBUS_CONNECTION_TYPE_LAST));
}

void register_connection_event_del(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate,
                                                 ELDBUS_CONNECTION_EVENT_DEL));
}

void register_connection_event_disconnected(v8::Isolate *isolate,
                                            v8::Handle<v8::Object> global,
                                            v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name,
                compatibility_new<Integer>
                (isolate, ELDBUS_CONNECTION_EVENT_DISCONNECTED));
}

void register_connection_event_last(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Integer;
    global->Set(name, compatibility_new<Integer>(isolate,
                                                 ELDBUS_CONNECTION_EVENT_LAST));
}

void register_connection_get(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        Eldbus_Connection_Type type;

        switch (args[0]->IntegerValue()) {
        case ELDBUS_CONNECTION_TYPE_UNKNOWN:
            type = ELDBUS_CONNECTION_TYPE_UNKNOWN;
            break;
        case ELDBUS_CONNECTION_TYPE_SESSION:
            type = ELDBUS_CONNECTION_TYPE_SESSION;
            break;
        case ELDBUS_CONNECTION_TYPE_SYSTEM:
            type = ELDBUS_CONNECTION_TYPE_SYSTEM;
            break;
        case ELDBUS_CONNECTION_TYPE_STARTER:
            type = ELDBUS_CONNECTION_TYPE_STARTER;
            break;
        case ELDBUS_CONNECTION_TYPE_ADDRESS:
            type = ELDBUS_CONNECTION_TYPE_ADDRESS;
            break;
        case ELDBUS_CONNECTION_TYPE_LAST:
            type = ELDBUS_CONNECTION_TYPE_LAST;
            break;
        default:
            return compatibility_return();
        }
        auto ret = eldbus_connection_get(type);
        return compatibility_return(wrap_eldbus_connection(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_private_connection_get(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        auto isolate = args.GetIsolate();
        Eldbus_Connection_Type type;

        switch (args[0]->IntegerValue()) {
        case ELDBUS_CONNECTION_TYPE_UNKNOWN:
            type = ELDBUS_CONNECTION_TYPE_UNKNOWN;
            break;
        case ELDBUS_CONNECTION_TYPE_SESSION:
            type = ELDBUS_CONNECTION_TYPE_SESSION;
            break;
        case ELDBUS_CONNECTION_TYPE_SYSTEM:
            type = ELDBUS_CONNECTION_TYPE_SYSTEM;
            break;
        case ELDBUS_CONNECTION_TYPE_STARTER:
            type = ELDBUS_CONNECTION_TYPE_STARTER;
            break;
        case ELDBUS_CONNECTION_TYPE_ADDRESS:
            type = ELDBUS_CONNECTION_TYPE_ADDRESS;
            break;
        case ELDBUS_CONNECTION_TYPE_LAST:
            type = ELDBUS_CONNECTION_TYPE_LAST;
            break;
        default:
            return compatibility_return();
        }
        auto ret = eldbus_private_connection_get(type);
        return compatibility_return(wrap_eldbus_connection(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_address_connection_get(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name)

{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();

        auto ret = eldbus_address_connection_get(*String::Utf8Value(args[0]));
        return compatibility_return(wrap_eldbus_connection(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_private_address_connection_get(v8::Isolate *isolate,
                                             v8::Handle<v8::Object> global,
                                             v8::Handle<v8::String> name)

{
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        auto isolate = args.GetIsolate();

        String::Utf8Value address(args[0]);

        auto ret = eldbus_private_address_connection_get(*address);
        return compatibility_return(wrap_eldbus_connection(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

}

v8::Local<v8::Object> wrap_eldbus_connection(Eldbus_Connection *conn,
                                             v8::Isolate *isolate)
{
    using v8::String;
    using v8::Boolean;
    using v8::Value;
    using v8::Handle;
    using v8::ObjectTemplate;
    using v8::Function;
    using v8::FunctionTemplate;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto event_callback_add = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 2 || !info[0]->IsNumber()
            || !info[1]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();
        auto conn = extract_eldbus_connection(info.This());
        Eldbus_Connection_Event_Type type;

        switch (info[0]->IntegerValue()) {
        case ELDBUS_CONNECTION_EVENT_DEL:
            type = ELDBUS_CONNECTION_EVENT_DEL;
            break;
        case ELDBUS_CONNECTION_EVENT_DISCONNECTED:
            type = ELDBUS_CONNECTION_EVENT_DISCONNECTED;
            break;
        case ELDBUS_CONNECTION_EVENT_LAST:
            type = ELDBUS_CONNECTION_EVENT_LAST;
            break;
        default:
            return compatibility_return();
        }

        unique_ptr<persistent_t> cb_data{new persistent_t{isolate, info[1]}};

        auto cb = [](void *data, Eldbus_Connection *conn,
                     void */*event_info*/) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{wrap_eldbus_connection(conn, isolate)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);
        };

        eldbus_connection_event_callback_add(conn, type, cb, cb_data.get());
        auto ret = wrap_event_callback(conn, type, cb, cb_data.release(),
                                       isolate);
        return compatibility_return(ret, info);
    };

    auto send = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 3 || !info[0]->IsObject() || !info[1]->IsFunction()
            || !info[2]->IsNumber()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();
        auto conn = extract_eldbus_connection(info.This());
        auto msg
            = (compatibility_get_pointer_internal_field<Eldbus_Message*>
               (info[0]->ToObject(), 0));

        unique_ptr<persistent_t> cb_data{new persistent_t{isolate, info[1]}};

        auto cb = [](void *data, const Eldbus_Message *msg,
                     Eldbus_Pending *pending) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_const_eldbus_msg(msg, isolate),
                wrap_eldbus_pending(pending, persistent, isolate)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);

            delete persistent;
        };

        auto ret = eldbus_connection_send(conn, msg, cb, cb_data.get(),
                                          info[2]->NumberValue());
        return compatibility_return(wrap_eldbus_pending(ret, cb_data.release(),
                                                        isolate),
                                    info);
    };

    auto unique_name_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto conn = extract_eldbus_connection(info.This());

        auto ret = eldbus_connection_unique_name_get(conn);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto object_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsString())
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto conn = extract_eldbus_connection(info.This());

        auto ret = eldbus_object_get(conn, *String::Utf8Value(info[0]),
                                     *String::Utf8Value(info[1]));
        return compatibility_return(wrap_eldbus_object(ret, isolate), info);
    };

    auto signal_handler_add = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 5 || !info[0]->IsString() || !info[1]->IsString()
            || !info[2]->IsString() || !info[3]->IsString()
            || !info[4]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();
        auto conn = extract_eldbus_connection(info.This());

        unique_ptr<persistent_t> cb_data{new persistent_t{isolate, info[4]}};

        auto cb = [](void *data, const Eldbus_Message *msg) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{wrap_const_eldbus_msg(msg, isolate)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);
        };

        auto ret = eldbus_signal_handler_add(conn, *String::Utf8Value(info[0]),
                                             *String::Utf8Value(info[1]),
                                             *String::Utf8Value(info[2]),
                                             *String::Utf8Value(info[3]),
                                             cb, cb_data.get());
        auto wrapped_ret = wrap_eldbus_signal_handler(ret, cb_data.release(),
                                                      isolate);
        return compatibility_return(wrapped_ret, info);
    };

    ret->Set(compatibility_new<String>(isolate, "event_callback_add"),
             compatibility_new<FunctionTemplate>(isolate, event_callback_add)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "send"),
             compatibility_new<FunctionTemplate>(isolate, send)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "unique_name_get"),
             compatibility_new<FunctionTemplate>(isolate, unique_name_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "object_get"),
             compatibility_new<FunctionTemplate>(isolate, object_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "signal_handler_add"),
             compatibility_new<FunctionTemplate>(isolate, signal_handler_add)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, conn);

    return ret;
}
      
EAPI
void register_eldbus_connection(v8::Isolate* isolate, v8::Handle<v8::Object> exports)
{
   using v8::String;
   register_timeout_infinite(isolate, exports,
                             compatibility_new<String>
                             (isolate, "ELDBUS_TIMEOUT_INFINITE"));
   register_connection_type_unknown(isolate, exports,
                                    compatibility_new<String>
                                    (isolate, "ELDBUS_CONNECTION_TYPE_UNKNOWN"));
   register_connection_type_session(isolate, exports,
                                    compatibility_new<String>
                                    (isolate, "ELDBUS_CONNECTION_TYPE_SESSION"));
   register_connection_type_system(isolate, exports,
                                   compatibility_new<String>
                                   (isolate, "ELDBUS_CONNECTION_TYPE_SYSTEM"));
   register_connection_type_starter(isolate, exports,
                                    compatibility_new<String>
                                    (isolate, "ELDBUS_CONNECTION_TYPE_STARTER"));
   register_connection_type_address(isolate, exports,
                                    compatibility_new<String>
                                    (isolate, "ELDBUS_CONNECTION_TYPE_ADDRESS"));
   register_connection_type_last(isolate, exports,
                                 compatibility_new<String>
                                 (isolate, "ELDBUS_CONNECTION_TYPE_LAST"));
   register_connection_event_del(isolate, exports,
                                 compatibility_new<String>
                                 (isolate, "ELDBUS_CONNECTION_EVENT_DEL"));
   register_connection_event_disconnected(isolate, exports,
                                          compatibility_new<String>
                                          (isolate,
                                           "ELDBUS_CONNECTION_EVENT"
                                           "_DISCONNECTED"));
   register_connection_event_last(isolate, exports,
                                  compatibility_new<String>
                                  (isolate, "ELDBUS_CONNECTION_EVENT_LAST"));

   register_connection_get(isolate, exports,
                           compatibility_new<String>
                           (isolate, "connection_get"));

   register_private_connection_get(isolate, exports,
                                   compatibility_new<String>
                                   (isolate, "private_connection_get"));

   register_address_connection_get(isolate, exports,
                                   compatibility_new<String>
                                   (isolate, "address_connection_get"));

   register_private_address_connection_get(isolate, exports,
                                           compatibility_new<String>
                                           (isolate, "private_address_connection_get"));
}

} } } // namespace efl { namespace eldbus { namespace js {
