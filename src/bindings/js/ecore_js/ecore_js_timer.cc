#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

#include <memory>
#include <cstdlib>

namespace efl { namespace ecore { namespace js {

namespace {

Ecore_Timer* extract_timer(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Timer*>(ptr);
}

v8::Local<v8::Object> wrap_timer(Ecore_Timer *timer,
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

        ecore_timer_del(extract_timer(info.This()));
        return compatibility_return();
    };

    auto freeze = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_timer_freeze(extract_timer(info.This()));
        return compatibility_return();
    };

    auto freeze_get = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        auto ret = ecore_timer_freeze_get(extract_timer(info.This()));
        return compatibility_return(compatibility_new<Boolean>
                                    (info.GetIsolate(), bool(ret)),
                                    info);
    };

    auto thaw = [](compatibility_callback_info_type info)
        -> compatibility_return_type {
        if (info.Length() != 0)
            return compatibility_return();

        ecore_timer_thaw(extract_timer(info.This()));
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "freeze"),
             compatibility_new<FunctionTemplate>(isolate, freeze)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "freeze_get"),
             compatibility_new<FunctionTemplate>(isolate, freeze_get)
             ->GetFunction());
    ret->Set(compatibility_new<String>(isolate, "thaw"),
             compatibility_new<FunctionTemplate>(isolate, thaw)->GetFunction());

    ret->SetInternalField(0, compatibility_new<v8::External>(isolate, timer));

    return ret;
}

void register_timer_precision_get(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::Number;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        auto ret = ecore_timer_precision_get();
        return compatibility_return(compatibility_new<Number>
                                    (args.GetIsolate(), ret), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_timer_precision_set(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        ecore_timer_precision_set(args[0]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_timer_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::Local;
    using v8::Value;
    using v8::Undefined;
    using v8::Function;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsNumber()
            || !args[1]->IsFunction()) {
            return compatibility_return();
        }

        auto f = new efl::eina::js::global_ref<Value>(args.GetIsolate(), args[1]);

        auto cb = [](void *data) -> Eina_Bool {
            auto persistent = reinterpret_cast<efl::eina::js::global_ref<Value>*>(data);
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

        auto ret = ecore_timer_add(args[0]->NumberValue(), cb, f);
        return compatibility_return(wrap_timer(ret, args.GetIsolate()), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

void register_timer_loop_add(v8::Isolate *isolate,
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
        if (args.Length() != 2 || !args[0]->IsNumber()
            || !args[1]->IsFunction()) {
            return compatibility_return();
        }

        auto f = new efl::eina::js::global_ref<Value>(args.GetIsolate(), args[1]);

        auto cb = [](void *d) -> Eina_Bool {
            auto persistent = static_cast<efl::eina::js::global_ref<Value>*>(d);
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

        auto ret = ecore_timer_loop_add(args[0]->NumberValue(), cb, f);

        return compatibility_return(wrap_timer(ret, args.GetIsolate()), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

}

EAPI
void register_ecore_timer(v8::Isolate *isolate,v8::Handle<v8::Object> exports)
{
   register_timer_precision_get(isolate, exports,
                                compatibility_new<v8::String>(isolate,
                                                          "getPrecision"));
   register_timer_precision_set(isolate, exports,
                                compatibility_new<v8::String>(isolate,
                                                          "setPrecision"));
   register_timer_add(isolate, exports,
                      compatibility_new<v8::String>(isolate, "add"));
   register_timer_loop_add(isolate, exports,
                           compatibility_new<v8::String>(isolate,
                                                     "addLoop"));
}

} } } // namespace efl { namespace ecore { namespace js {
