#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ecore_js_idle.hh>
#include <Ecore.h>

namespace efl { namespace ecore { namespace js {

struct persistent_with_isolate_t
{
    template<class S>
    persistent_with_isolate_t(v8::Isolate *isolate, v8::Handle<S> that)
        : isolate(isolate)
        , persistent(isolate, that)
    {}

    v8::Isolate *isolate;
    v8::Persistent<v8::Value> persistent;
};

static Ecore_Idler* extract_idler(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Idler*>(ptr);
}

static
v8::Local<v8::Object> wrap_idler(Ecore_Idler *idler, v8::Isolate *isolate)
{
    using v8::Boolean;
    using v8::String;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_idler_del(extract_idler(info.This()));
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    ret->SetInternalField(0, compatibility_new<v8::External>(isolate, idler));

    return ret;
}

static Ecore_Idle_Enterer* extract_idle_enterer(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Idle_Enterer*>(ptr);
}

static v8::Local<v8::Object> wrap_idle_enterer(Ecore_Idle_Enterer *idle_enterer,
                                               v8::Isolate *isolate)
{
    using v8::Boolean;
    using v8::String;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using v8::External;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_idle_enterer_del(extract_idle_enterer(info.This()));
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    ret->SetInternalField(0,
                          compatibility_new<External>(isolate, idle_enterer));

    return ret;
}

static Ecore_Idle_Exiter* extract_idle_exiter(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Idle_Exiter*>(ptr);
}

static v8::Local<v8::Object> wrap_idle_exiter(Ecore_Idle_Exiter *idle_exiter,
                                              v8::Isolate *isolate)
{
    using v8::Boolean;
    using v8::String;
    using v8::ObjectTemplate;
    using v8::FunctionTemplate;
    using v8::External;

    auto obj_tpl = compatibility_new<ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(1);
    auto ret = obj_tpl->NewInstance();

    auto del = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_idle_exiter_del(extract_idle_exiter(info.This()));
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    ret->SetInternalField(0, compatibility_new<External>(isolate, idle_exiter));

    return ret;
}

EAPI
void register_idler_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        persistent_with_isolate_t *f
            = new persistent_with_isolate_t(args.GetIsolate(), args[0]);
        auto ret = ecore_idler_add([](void *data) -> Eina_Bool {
            persistent_with_isolate_t *persistent
                = reinterpret_cast<persistent_with_isolate_t *>(data);
            auto value = Local<Value>::New(persistent->isolate,
                                           persistent->persistent);
            auto closure = Function::Cast(*value);

            auto ret = closure->Call(Undefined(persistent->isolate), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
                delete persistent;

            return bret ? EINA_TRUE : EINA_FALSE;
        }, f);

        return compatibility_return(wrap_idler(ret, args.GetIsolate()), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_idle_enterer_add(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        persistent_with_isolate_t *f
            = new persistent_with_isolate_t(args.GetIsolate(), args[0]);
    auto ret = ecore_idle_enterer_add([](void *data) -> Eina_Bool {
            persistent_with_isolate_t *persistent
                = reinterpret_cast<persistent_with_isolate_t *>(data);
            auto value = Local<Value>::New(persistent->isolate,
                                           persistent->persistent);
            auto closure = Function::Cast(*value);

            auto ret = closure->Call(Undefined(persistent->isolate), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
                delete persistent;

            return bret ? EINA_TRUE : EINA_FALSE;
        }, f);

        return compatibility_return(wrap_idle_enterer(ret, args.GetIsolate()),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_idle_enterer_before_add(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        persistent_with_isolate_t *f
            = new persistent_with_isolate_t(args.GetIsolate(), args[0]);
        auto ret = ecore_idle_enterer_before_add([](void *data) -> Eina_Bool {
            persistent_with_isolate_t *persistent
                = reinterpret_cast<persistent_with_isolate_t *>(data);
            auto value = Local<Value>::New(persistent->isolate,
                                           persistent->persistent);
            auto closure = Function::Cast(*value);

            auto ret = closure->Call(Undefined(persistent->isolate), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
                delete persistent;

            return bret ? EINA_TRUE : EINA_FALSE;
        }, f);

        return compatibility_return(wrap_idle_enterer(ret, args.GetIsolate()),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_idle_exiter_add(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        persistent_with_isolate_t *f
            = new persistent_with_isolate_t(args.GetIsolate(), args[0]);
        auto ret = ecore_idle_exiter_add([](void *data) -> Eina_Bool {
            persistent_with_isolate_t *persistent
                = reinterpret_cast<persistent_with_isolate_t *>(data);
            auto value = Local<Value>::New(persistent->isolate,
                                           persistent->persistent);
            auto closure = Function::Cast(*value);

            auto ret = closure->Call(Undefined(persistent->isolate), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
                delete persistent;

            return bret ? EINA_TRUE : EINA_FALSE;
        }, f);

        return compatibility_return(wrap_idle_exiter(ret, args.GetIsolate()),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

} } } // namespace efl { namespace js {
