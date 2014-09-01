#ifndef ELDBUS_JS_UTIL_HH
#define ELDBUS_JS_UTIL_HH

namespace efl { namespace eldbus { namespace js {

using eina::js::compatibility_get_pointer_internal_field;
using eina::js::compatibility_set_pointer_internal_field;
using eina::js::compatibility_new;
using eina::js::compatibility_callback_info_type;
using eina::js::compatibility_return_type;
using eina::js::compatibility_return;
      
v8::Local<v8::Object> wrap_eldbus_connection(Eldbus_Connection *conn,
                                             v8::Isolate *isolate);

inline
const Eldbus_Message *extract_const_eldbus_msg(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Eldbus_Message*>(o, 0);
}

inline
v8::Local<v8::Object> wrap_const_eldbus_msg(const Eldbus_Message *msg,
                                            v8::Isolate *isolate)
{
    using v8::String;
    using v8::Object;
    using v8::Boolean;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto path_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());

        auto ret = eldbus_message_path_get(msg);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto interface_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());

        auto ret = eldbus_message_interface_get(msg);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto member_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());

        auto ret = eldbus_message_member_get(msg);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto destination_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());

        auto ret = eldbus_message_destination_get(msg);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto sender_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());

        auto ret = eldbus_message_sender_get(msg);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto signature_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());

        auto ret = eldbus_message_signature_get(msg);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto error_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto msg = extract_const_eldbus_msg(info.This());
        const char *name = NULL;
        const char *text = NULL;
        auto ret = compatibility_new<Object>(isolate);

        auto bret = eldbus_message_error_get(msg, &name, &text);
        ret->Set(compatibility_new<String>(isolate, "ok"),
                 compatibility_new<Boolean>(isolate, bret));

        if (bret) {
            ret->Set(compatibility_new<String>(isolate, "name"),
                     compatibility_new<String>(isolate, name));
            ret->Set(compatibility_new<String>(isolate, "text"),
                     compatibility_new<String>(isolate, text));
        }

        return compatibility_return(ret, info);
    };

    ret->Set(compatibility_new<String>(isolate, "path_get"),
             compatibility_new<FunctionTemplate>(isolate, path_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "interface_get"),
             compatibility_new<FunctionTemplate>(isolate, interface_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "member_get"),
             compatibility_new<FunctionTemplate>(isolate, member_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "destination_get"),
             compatibility_new<FunctionTemplate>(isolate, destination_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "sender_get"),
             compatibility_new<FunctionTemplate>(isolate, sender_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "signature_get"),
             compatibility_new<FunctionTemplate>(isolate, signature_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "error_get"),
             compatibility_new<FunctionTemplate>(isolate, error_get)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0,
                                             const_cast<Eldbus_Message*>(msg));

    return ret;
}

inline Eldbus_Message *extract_eldbus_msg(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Eldbus_Message*>(o, 0);
}

inline
v8::Local<v8::Object> wrap_eldbus_msg(Eldbus_Message *msg, v8::Isolate *isolate)
{
    using v8::String;
    using v8::FunctionTemplate;

    auto ret = wrap_const_eldbus_msg(msg, isolate);

    auto ref = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto msg = extract_eldbus_msg(info.This());

        eldbus_message_ref(msg);

        return compatibility_return();
    };

    auto unref = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto msg = extract_eldbus_msg(info.This());

        eldbus_message_unref(msg);

        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "ref"),
             compatibility_new<FunctionTemplate>(isolate, ref)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "unref"),
             compatibility_new<FunctionTemplate>(isolate, unref)
             ->GetFunction());

    return ret;
}

inline
v8::Local<v8::Object> wrap_eldbus_pending(Eldbus_Pending *pending,
                                          efl::eina::js::global_ref<v8::Value>*
                                          persistent,
                                          v8::Isolate *isolate)
{
    using v8::String;
    using v8::Object;
    using v8::Boolean;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    typedef decltype(persistent) persistent_ptr_t;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(2);
    auto ret = obj_tpl->NewInstance();

    auto cancel = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto o = info.This();
        auto pending
            = compatibility_get_pointer_internal_field<Eldbus_Pending*>(o, 0);
        auto persistent
            = compatibility_get_pointer_internal_field<persistent_ptr_t>(o, 1);

        eldbus_pending_cancel(pending);
        delete persistent;
        return compatibility_return();
    };

    auto destination_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto pending
            = compatibility_get_pointer_internal_field<Eldbus_Pending*>(o, 0);

        auto ret = eldbus_pending_destination_get(pending);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto path_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto pending
            = compatibility_get_pointer_internal_field<Eldbus_Pending*>(o, 0);

        auto ret = eldbus_pending_path_get(pending);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto interface_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto pending
            = compatibility_get_pointer_internal_field<Eldbus_Pending*>(o, 0);

        auto ret = eldbus_pending_interface_get(pending);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto method_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto pending
            = compatibility_get_pointer_internal_field<Eldbus_Pending*>(o, 0);

        auto ret = eldbus_pending_method_get(pending);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    ret->Set(compatibility_new<String>(isolate, "cancel"),
             compatibility_new<FunctionTemplate>(isolate, cancel)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "destination_get"),
             compatibility_new<FunctionTemplate>(isolate, destination_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "path_get"),
             compatibility_new<FunctionTemplate>(isolate, path_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "interface_get"),
             compatibility_new<FunctionTemplate>(isolate, interface_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "method_get"),
             compatibility_new<FunctionTemplate>(isolate, method_get)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, pending);
    compatibility_set_pointer_internal_field(ret, 1, persistent);

    return ret;
}

inline
v8::Local<v8::Object>
wrap_eldbus_signal_handler(Eldbus_Signal_Handler *handler,
                           efl::eina::js::global_ref<v8::Value> *persistent,
                           v8::Isolate *isolate)
{
    using v8::String;
    using v8::Object;
    using v8::Boolean;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    typedef decltype(persistent) persistent_ptr_t;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(2);
    auto ret = obj_tpl->NewInstance();

    auto ref = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        eldbus_signal_handler_ref(handler);
        return compatibility_return();
    };

    auto unref = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        eldbus_signal_handler_unref(handler);
        return compatibility_return();
    };

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));
        auto persistent
            = compatibility_get_pointer_internal_field<persistent_ptr_t>(o, 1);

        eldbus_signal_handler_del(handler);
        delete persistent;
        return compatibility_return();
    };

    auto sender_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        auto ret = eldbus_signal_handler_sender_get(handler);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto path_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        auto ret = eldbus_signal_handler_path_get(handler);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto interface_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        auto ret = eldbus_signal_handler_interface_get(handler);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto member_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        auto ret = eldbus_signal_handler_member_get(handler);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto match_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        auto ret = eldbus_signal_handler_match_get(handler);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto connection_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto o = info.This();
        auto handler
            = (compatibility_get_pointer_internal_field<Eldbus_Signal_Handler*>
               (o, 0));

        auto ret = eldbus_signal_handler_connection_get(handler);
        return compatibility_return(wrap_eldbus_connection(ret, isolate), info);
    };

    ret->Set(compatibility_new<String>(isolate, "ref"),
             compatibility_new<FunctionTemplate>(isolate, ref)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "unref"),
             compatibility_new<FunctionTemplate>(isolate, unref)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "sender_get"),
             compatibility_new<FunctionTemplate>(isolate, sender_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "path_get"),
             compatibility_new<FunctionTemplate>(isolate, path_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "interface_get"),
             compatibility_new<FunctionTemplate>(isolate, interface_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "member_get"),
             compatibility_new<FunctionTemplate>(isolate, member_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "match_get"),
             compatibility_new<FunctionTemplate>(isolate, match_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "connection_get"),
             compatibility_new<FunctionTemplate>(isolate, connection_get)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, handler);
    compatibility_set_pointer_internal_field(ret, 1, persistent);

    return ret;
}

static
v8::Local<v8::Object> wrap_object_event_callback(Eldbus_Object *obj,
                                                 Eldbus_Object_Event_Type type,
                                                 Eldbus_Object_Event_Cb cb,
                                                 void *cb_data,
                                                 v8::Isolate *isolate)
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

    compatibility_set_pointer_internal_field(ret, 0, obj);
    compatibility_set_pointer_internal_field(ret, 1, (void*)(cb));
    compatibility_set_pointer_internal_field(ret, 2, cb_data);

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        auto o = info.This();
        auto isolate = info.GetIsolate();

        auto obj
            = (compatibility_get_pointer_internal_field<Eldbus_Object*>
               (o, 0));
        auto cb
            = (compatibility_get_pointer_internal_field
               <Eldbus_Object_Event_Cb>(o, 1));
        auto cb_data
            = (compatibility_get_pointer_internal_field
               <efl::eina::js::global_ref<Value>*>(o, 2));
        auto type
            = (static_cast<Eldbus_Object_Event_Type>
               (o->Get(compatibility_new<String>(isolate, "_type"))
                ->IntegerValue()));

        eldbus_object_event_callback_del(obj, type, cb, cb_data);

        delete cb_data;
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    return ret;
}

inline
Eldbus_Object *extract_eldbus_object(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Eldbus_Object*>(o, 0);
}

inline
v8::Local<v8::Object> wrap_eldbus_object(Eldbus_Object *object,
                                         v8::Isolate *isolate)
{
    using v8::String;
    using v8::Value;
    using v8::Handle;
    using v8::Function;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto ref = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto object = extract_eldbus_object(info.This());

        eldbus_object_ref(object);
        return compatibility_return();
    };

    auto unref = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto object = extract_eldbus_object(info.This());

        eldbus_object_unref(object);
        return compatibility_return();
    };

    auto event_callback_add = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 2 || !info[0]->IsNumber()
            || !info[1]->IsFunction()) {
            return compatibility_return();
        }

        Eldbus_Object_Event_Type type;

        switch (info[0]->IntegerValue()) {
        case ELDBUS_OBJECT_EVENT_IFACE_ADDED:
            type = ELDBUS_OBJECT_EVENT_IFACE_ADDED;
            break;
        case ELDBUS_OBJECT_EVENT_IFACE_REMOVED:
            type = ELDBUS_OBJECT_EVENT_IFACE_REMOVED;
            break;
        case ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED:
            type = ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED;
            break;
        case ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED:
            type = ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED;
            break;
        case ELDBUS_OBJECT_EVENT_DEL:
            type = ELDBUS_OBJECT_EVENT_DEL;
            break;
        case ELDBUS_OBJECT_EVENT_LAST:
            type = ELDBUS_OBJECT_EVENT_LAST;
            break;
        default:
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t{isolate, info[1]}};

        auto cb = [](void *data, Eldbus_Object *obj, void */*event_info*/) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{wrap_eldbus_object(obj, isolate)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);
        };

        auto object = extract_eldbus_object(info.This());

        eldbus_object_event_callback_add(object, type, cb, cb_data.get());
        auto ret = wrap_object_event_callback(object, type, cb,
                                              cb_data.release(), isolate);
        return compatibility_return(ret, info);
    };

    auto connection_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto object = extract_eldbus_object(info.This());

        auto conn = eldbus_object_connection_get(object);
        return compatibility_return(wrap_eldbus_connection(conn, isolate),
                                    info);
    };

    auto bus_name_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto object = extract_eldbus_object(info.This());

        auto ret = eldbus_object_bus_name_get(object);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto path_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto object = extract_eldbus_object(info.This());

        auto ret = eldbus_object_path_get(object);
        return compatibility_return(compatibility_new<String>(isolate, ret),
                                    info);
    };

    auto send = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 3 || !info[0]->IsObject() || !info[1]->IsFunction()
            || !info[2]->IsNumber()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();
        auto obj = extract_eldbus_object(info.This());
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

        auto ret = eldbus_object_send(obj, msg, cb, cb_data.get(),
                                      info[2]->NumberValue());
        return compatibility_return(wrap_eldbus_pending(ret, cb_data.release(),
                                                        isolate),
                                    info);
    };

    auto signal_handler_add = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 3 || !info[0]->IsString() || !info[1]->IsString()
            || !info[2]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();
        auto object = extract_eldbus_object(info.This());
        String::Utf8Value interface(info[0]);
        String::Utf8Value member(info[1]);

        unique_ptr<persistent_t> cb_data{new persistent_t{isolate, info[2]}};

        auto cb = [](void *data, const Eldbus_Message *msg) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{wrap_const_eldbus_msg(msg, isolate)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);
        };

        auto ret = eldbus_object_signal_handler_add(object, *interface, *member,
                                                    cb, cb_data.get());
        return
            compatibility_return(wrap_eldbus_signal_handler(ret,
                                                            cb_data.release(),
                                                            isolate),
                                 info);
    };

    auto method_call_new = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 2 || !info[0]->IsString()
            || !info[1]->IsString()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();
        auto object = extract_eldbus_object(info.This());
        auto ret = eldbus_object_method_call_new(object,
                                                 *String::Utf8Value(info[0]),
                                                 *String::Utf8Value(info[1]));
        return compatibility_return(wrap_eldbus_msg(ret, isolate), info);
    };

    ret->Set(compatibility_new<String>(isolate, "ref"),
             compatibility_new<FunctionTemplate>(isolate, ref)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "unref"),
             compatibility_new<FunctionTemplate>(isolate, unref)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "event_callback_add"),
             compatibility_new<FunctionTemplate>(isolate, event_callback_add)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "connection_get"),
             compatibility_new<FunctionTemplate>(isolate, connection_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "bus_name_get"),
             compatibility_new<FunctionTemplate>(isolate, bus_name_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "path_get"),
             compatibility_new<FunctionTemplate>(isolate, path_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "send"),
             compatibility_new<FunctionTemplate>(isolate, send)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "signal_handler_add"),
             compatibility_new<FunctionTemplate>(isolate, signal_handler_add)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "method_call_new"),
             compatibility_new<FunctionTemplate>(isolate, method_call_new)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, object);

    return ret;
}

} } } // namespace efl { namespace eldbus { namespace js {

#endif /* ELDBUS_JS_UTIL_HH */
