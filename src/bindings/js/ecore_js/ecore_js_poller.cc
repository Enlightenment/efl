#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

namespace efl { namespace ecore { namespace js {

namespace {
      
Ecore_Poller* extract_poller(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Poller*>(ptr);
}

v8::Local<v8::Object> wrap_poller(Ecore_Poller *poller,
                                         v8::Isolate *isolate)
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

        ecore_poller_del(extract_poller(info.This()));
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    ret->SetInternalField(0, compatibility_new<v8::External>(isolate, poller));

    return ret;
}

void register_poller_core(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name)
{
    using v8::Integer;

    global->Set(name, compatibility_new<Integer>(isolate, ECORE_POLLER_CORE));
}

void register_poller_poll_interval_set(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
            return compatibility_return();

        Ecore_Poller_Type type;

        switch ((int)(args[0]->NumberValue())) {
        case ECORE_POLLER_CORE:
            type = ECORE_POLLER_CORE;
            break;
        default:
            return compatibility_return();
        }

        ecore_poller_poll_interval_set(type, args[1]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_poller_poll_interval_get(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        Ecore_Poller_Type type;

        switch ((int)(args[0]->NumberValue())) {
        case ECORE_POLLER_CORE:
            type = ECORE_POLLER_CORE;
            break;
        default:
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();
        auto ret = ecore_poller_poll_interval_get(type);
        return compatibility_return(compatibility_new<Integer>(isolate, ret),
                                    args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_poller_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::Integer;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Isolate;
    using v8::Function;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 3 || !args[0]->IsNumber() || !args[1]->IsNumber()
            || !args[2]->IsFunction()) {
            return compatibility_return();
        }

        Ecore_Poller_Type type;

        switch ((int)(args[0]->NumberValue())) {
        case ECORE_POLLER_CORE:
            type = ECORE_POLLER_CORE;
            break;
        default:
            return compatibility_return();
        }

        auto isolate = args.GetIsolate();
        auto f = new efl::eina::js::global_ref<Value>(isolate, args[2]);

        auto cb = [](void *data) -> Eina_Bool {
            auto persistent = static_cast<efl::eina::js::global_ref<Value>*>(data);
            auto o = persistent->handle();

            auto ret = Function::Cast(*o)->Call(o->ToObject(), 0, NULL);
            auto bret = ret->IsBoolean() && ret->BooleanValue();

            if (!bret)
              {
                 persistent->dispose();
                 delete persistent;
              }

            return bret ? EINA_TRUE : EINA_FALSE;
        };

        auto ret = ecore_poller_add(type, args[1]->NumberValue(), cb, f);
        return compatibility_return(wrap_poller(ret, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

}

EAPI
void register_ecore_poller(v8::Isolate *isolate,v8::Handle<v8::Object> exports)
{
   register_poller_core(isolate, exports,
                        compatibility_new<v8::String>(isolate, "CORE"));
   register_poller_poll_interval_set(isolate, exports,
                                     compatibility_new<v8::String>(isolate,
                                                               "setPollInterval"));
   register_poller_poll_interval_get(isolate, exports,
                                     compatibility_new<v8::String>(isolate,
                                                               "getPollInterval"));
   register_poller_add(isolate, exports,
                       compatibility_new<v8::String>(isolate, "add"));
}
  
} } } // namespace efl { namespace js {
