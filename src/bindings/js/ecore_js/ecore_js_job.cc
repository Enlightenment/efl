#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore_Js.hh>

namespace efl { namespace ecore { namespace js {

static Ecore_Job* extract_job(v8::Local<v8::Object> object)
{
    auto ptr = v8::External::Cast(*object->GetInternalField(0))->Value();
    return reinterpret_cast<Ecore_Job*>(ptr);
}

static v8::Local<v8::Object> wrap_job(Ecore_Job *job, v8::Isolate *isolate)
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

        ecore_job_del(extract_job(info.This()));
        return compatibility_return();
    };

    ret->Set(compatibility_new<String>(isolate, "del"),
             compatibility_new<FunctionTemplate>(isolate, del)->GetFunction());

    ret->SetInternalField(0, compatibility_new<v8::External>(isolate, job));

    return ret;
}

EAPI
void register_job_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
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

        auto f = new efl::eina::js::global_ref<Value>(args.GetIsolate(), args[0]);
        auto ret = ecore_job_add([](void *data) {
                auto persistent = static_cast<efl::eina::js::global_ref<Value>*>(data);
        	auto o = persistent->handle();

                Function::Cast(*o)->Call(o->ToObject(), 0, NULL);

                persistent->dispose();
                delete persistent;
            }, f);

        return compatibility_return(wrap_job(ret, args.GetIsolate()), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_ecore_job(v8::Isolate *isolate,v8::Handle<v8::Object> exports)
{
   register_job_add(isolate, exports,
                    compatibility_new<v8::String>(isolate, "add"));
}
      
} } } // namespace efl { namespace js {
