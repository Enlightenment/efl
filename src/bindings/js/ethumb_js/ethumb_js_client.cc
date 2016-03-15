#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ethumb_Js.hh>

namespace efl { namespace ethumb { namespace js {

namespace {

Ethumb_Exists* extract_ethumb_exists(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Ethumb_Exists*>(object, 0);
}

efl::eina::js::global_ref<v8::Value>*
extract_ethumb_exists_cb(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field
        <efl::eina::js::global_ref<v8::Value>*>(object, 1);
}

v8::Local<v8::Object> wrap_ethumb_exists(Ethumb_Exists *exists,
                                         efl::eina::js::global_ref<v8::Value>*
                                         cb,
                                         v8::Isolate *isolate)
{
    using v8::String;
    using v8::Boolean;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(2);
    auto ret = obj_tpl->NewInstance();

    auto cancel = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto exists = extract_ethumb_exists(info.This());
        auto cb = extract_ethumb_exists_cb(info.This());

        if (ethumb_client_thumb_exists_check(exists))
            return compatibility_return();

        delete cb;

        ethumb_client_thumb_exists_cancel(exists);
        return compatibility_return();
    };

    auto check = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();
        auto exists = extract_ethumb_exists(info.This());

        auto ret = ethumb_client_thumb_exists_check(exists);

        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    info);
    };

    ret->Set(compatibility_new<String>(isolate, "cancel"),
             compatibility_new<FunctionTemplate>(isolate, cancel)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "check"),
             compatibility_new<FunctionTemplate>(isolate, check)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, exists);
    compatibility_set_pointer_internal_field(ret, 1, cb);

    return ret;
}

// ====================================================

Ethumb_Client_Async* extract_ethumb_client_async(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Ethumb_Client_Async*>(o, 0);
}

efl::eina::js::global_ref<v8::Value>*
extract_ethumb_client_async_cb(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field
        <efl::eina::js::global_ref<v8::Value>*>(object, 1);
}

Ethumb_Client* extract_ethumb_client_async_client(v8::Local<v8::Object> o)
{
    return compatibility_get_pointer_internal_field<Ethumb_Client*>(o, 0);
}

v8::Local<v8::Object>
wrap_ethumb_client_async(Ethumb_Client_Async *request,
                         efl::eina::js::global_ref<v8::Value>* cb,
                         Ethumb_Client *client,
                         v8::Isolate *isolate)
{
    using v8::String;
    using v8::Boolean;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(3);
    auto ret = obj_tpl->NewInstance();

    auto cancel = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto request = extract_ethumb_client_async(info.This());
        auto cb = extract_ethumb_client_async_cb(info.This());
        auto client = extract_ethumb_client_async_client(info.This());

        delete cb;

        ethumb_client_thumb_async_cancel(client, request);
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "cancel"),
             compatibility_new<FunctionTemplate>(isolate, cancel)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, request);
    compatibility_set_pointer_internal_field(ret, 1, cb);
    compatibility_set_pointer_internal_field(ret, 2, client);

    return ret;
}

Ethumb_Client* extract_ethumb_client(v8::Local<v8::Object> object)
{
    return compatibility_get_pointer_internal_field<Ethumb_Client*>(object, 0);
}

v8::Local<v8::Object> wrap_ethumb_client(Ethumb_Client *client,
                                                v8::Isolate *isolate)
{
    using v8::Integer;
    using v8::String;
    using v8::Boolean;
    using v8::Value;
    using v8::Object;
    using v8::ObjectTemplate;
    using v8::Handle;
    using v8::Function;
    using v8::FunctionTemplate;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto disconnect = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ethumb_client_disconnect(extract_ethumb_client(info.This()));
        return compatibility_return();
    };

    auto on_server_die_callback_set = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 1 || !info[0]->IsFunction())
            return compatibility_return();

        auto isolate = info.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t(isolate, info[0])};

        auto client = extract_ethumb_client(info.This());
        auto server_die_cb = [](void *data, Ethumb_Client *client) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{wrap_ethumb_client(client, isolate)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);
        };
        auto free_data = [](void *data) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            delete persistent;
        };

        ethumb_client_on_server_die_callback_set(client, server_die_cb,
                                                 cb_data.release(), free_data);
        return compatibility_return();
    };

    auto file_set = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsString())
            return compatibility_return();

        auto isolate = info.GetIsolate();

        auto ret = ethumb_client_file_set(extract_ethumb_client(info.This()),
                                          *String::Utf8Value(info[0]),
                                          *String::Utf8Value(info[1]));
        return compatibility_return(compatibility_new<Boolean>(isolate, ret),
                                    info);
    };

    auto file_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto isolate = info.GetIsolate();

        auto ret = compatibility_new<Object>(isolate);
        const char *path;
        const char *key;

        ethumb_client_file_get(extract_ethumb_client(info.This()), &path, &key);

        ret->Set(compatibility_new<String>(isolate, "path"),
                 compatibility_new<String>(isolate, path));
        ret->Set(compatibility_new<String>(isolate, "key"),
                 compatibility_new<String>(isolate, key));

        return compatibility_return(ret, info);
    };

    auto file_free = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ethumb_client_file_free(extract_ethumb_client(info.This()));

        return compatibility_return();
    };

    auto thumb_exists = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 1 || !info[0]->IsFunction())
            return compatibility_return();

        auto isolate = info.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t(isolate, info[0])};

        auto client = extract_ethumb_client(info.This());
        auto exists_cb = [](void *data, Ethumb_Client *client,
                            Ethumb_Exists *thread, Eina_Bool exists) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[3] = {
                wrap_ethumb_client(client, isolate),
                wrap_ethumb_exists(thread, NULL, isolate),
                compatibility_new<Boolean>(isolate, exists)
            };

            Function::Cast(*o)->Call(o->ToObject(), 3, args);

            delete persistent;
        };

        auto ret = ethumb_client_thumb_exists(client, exists_cb,
                                              cb_data.get());
        return compatibility_return(wrap_ethumb_exists(ret, cb_data.release(),
                                                       isolate),
                                    info);
    };

    auto generate = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 1 || !info[0]->IsFunction())
            return compatibility_return();

        auto isolate = info.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t(isolate, info[0])};

        auto client = extract_ethumb_client(info.This());
        auto generated_cb = [](void *data, Ethumb_Client *client, int id,
                               const char *file, const char *key,
                               const char *thumb_path, const char *thumb_key,
                               Eina_Bool success) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[7] = {
                wrap_ethumb_client(client, isolate),
                compatibility_new<Integer>(isolate, id),
                compatibility_new<String>(isolate, file),
                compatibility_new<String>(isolate, key),
                compatibility_new<String>(isolate, thumb_path),
                compatibility_new<String>(isolate, thumb_key),
                compatibility_new<Boolean>(isolate, success)
            };

            Function::Cast(*o)->Call(o->ToObject(), 7, args);
        };
        auto free_data = [](void *data) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            delete persistent;
        };

        auto ret = ethumb_client_generate(client, generated_cb,
                                          cb_data.release(), free_data);
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    info);
    };

    auto generate_cancel = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 2 || !info[0]->IsNumber()
            || !info[1]->IsFunction()) {
            return compatibility_return();
        }

        auto isolate = info.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t(isolate, info[1])};

        auto client = extract_ethumb_client(info.This());
        auto cancel_cb = [](void *data, Eina_Bool success) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{compatibility_new<Boolean>(isolate, success)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);
        };
        auto free_data = [](void *data) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            delete persistent;
        };

        ethumb_client_generate_cancel(client, info[0]->IntegerValue(),
                                      cancel_cb, cb_data.release(), free_data);
        return compatibility_return();
    };

    auto generate_cancel_all = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 1 || !info[0]->IsFunction())
            return compatibility_return();

        auto client = extract_ethumb_client(info.This());

        ethumb_client_generate_cancel_all(client);
        return compatibility_return();
    };

    auto thumb_async_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 1 || !info[0]->IsFunction())
            return compatibility_return();

        auto isolate = info.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t(isolate, info[0])};

        auto client = extract_ethumb_client(info.This());
        auto done_cb = [](Ethumb_Client *client, const char *thumb_path,
                          const char *thumb_key, void *data) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[3] = {
                wrap_ethumb_client(client, isolate),
                compatibility_new<String>(isolate, thumb_path),
                compatibility_new<String>(isolate, thumb_key)
            };

            Function::Cast(*o)->Call(o->ToObject(), 3, args);

            delete persistent;
        };
        auto error_cb = [](Ethumb_Client *client, void *data) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args{wrap_ethumb_client(client, isolate)};

            Function::Cast(*o)->Call(o->ToObject(), 1, &args);

            delete persistent;
        };

        auto ret = ethumb_client_thumb_async_get(client, done_cb, error_cb,
                                                 cb_data.get());
        auto wrapped_ret = wrap_ethumb_client_async(ret, cb_data.release(),
                                                    client, isolate);
        return compatibility_return(wrapped_ret, info);
    };

    ret->Set(compatibility_new<String>(isolate, "disconnect"),
             compatibility_new<FunctionTemplate>(isolate, disconnect)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "setOnServerDieCallback"),
             compatibility_new<FunctionTemplate>(isolate,
                                                 on_server_die_callback_set)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "setFile"),
             compatibility_new<FunctionTemplate>(isolate, file_set)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "getFile"),
             compatibility_new<FunctionTemplate>(isolate, file_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "freeFile"),
             compatibility_new<FunctionTemplate>(isolate, file_free)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "existsThumb"),
             compatibility_new<FunctionTemplate>(isolate, thumb_exists)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "generate"),
             compatibility_new<FunctionTemplate>(isolate, generate)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "cancelGenerate"),
             compatibility_new<FunctionTemplate>(isolate, generate_cancel)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "cancelAllGenerate"),
             compatibility_new<FunctionTemplate>(isolate, generate_cancel_all)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "getThumbAsync"),
             compatibility_new<FunctionTemplate>(isolate, thumb_async_get)
             ->GetFunction());

    compatibility_set_pointer_internal_field(ret, 0, client);

    return ret;
}

EAPI
void register_client_init(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto init = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ethumb_client_init();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, init)
                ->GetFunction());
}

EAPI
void register_client_shutdown(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto shutdown = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto isolate = args.GetIsolate();
        auto ret = ethumb_client_shutdown();
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, shutdown)
                ->GetFunction());
}

EAPI
void register_client_connect(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::Boolean;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Function;
    using v8::Handle;
    using std::unique_ptr;

    typedef efl::eina::js::global_ref<Value> persistent_t;

    auto shutdown = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        auto isolate = args.GetIsolate();

        unique_ptr<persistent_t> cb_data{new persistent_t(isolate, args[0])};

        auto connect_cb = [](void *data, Ethumb_Client *client,
                             Eina_Bool success) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            auto o = persistent->handle();

            auto isolate = v8::Isolate::GetCurrent();

            Handle<Value> args[2] = {
                wrap_ethumb_client(client, isolate),
                compatibility_new<Boolean>(isolate, success)
            };

            Function::Cast(*o)->Call(o->ToObject(), 2, args);
        };
        auto free_data = [](void *data) {
            auto persistent = reinterpret_cast<persistent_t*>(data);
            delete persistent;
        };

        auto ret = ethumb_client_connect(connect_cb, cb_data.release(),
                                         free_data);
        return compatibility_return(wrap_ethumb_client(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, shutdown)
                ->GetFunction());
}

}

EAPI
void register_ethumb(v8::Isolate *isolate, v8::Handle<v8::Object> exports)
{
   using v8::String;
   auto ethumbNamespace = efl::eo::js::get_namespace({"Ethumb"}, isolate, exports);
   register_client_init(isolate, ethumbNamespace,
                        compatibility_new<String>(isolate,
                                                  "initClient"));
   register_client_shutdown(isolate, ethumbNamespace,
                            compatibility_new<String>(isolate,
                                                      "shutdownClient"));
   register_client_connect(isolate, ethumbNamespace,
                           compatibility_new<String>(isolate,
                                                     "connectClient"));
}
      
} } } // namespace efl { namespace ethumb { namespace js {
