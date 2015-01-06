#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <eina_js_log.hh>
#include <eina_js_value.hh>
#include <eina_js_compatibility.hh>

namespace efl { namespace eina { namespace js {

v8::Local<v8::String> to_v8_string(v8::Isolate *isolate, const char *fmt,
                                   va_list args)
{
#if 0
    /* TODO: unfortunately, the elegant and exception-safe version isn't
       compiling (yet!) */
    efl::eina::stringshare s(eina_stringshare_vprintf(fmt, args),
                             efl::eina::steal_stringshare_ref);
    return v8::String::NewFromUtf8(isolate, s.data(), v8::String::kNormalString,
                                   s.size());
#else
    auto s = eina_stringshare_vprintf(fmt, args);
    auto ret = compatibility_new<v8::String>(isolate, s);
    eina_stringshare_del(s);
    return ret;
#endif
}

static struct {
    void reset(v8::Isolate *isolate, v8::Handle<v8::Value> functor)
    {
        persistent.Reset(isolate, functor);
        this->isolate = isolate;
    }

    v8::Local<v8::Value> functor()
    {
        return v8::Local<v8::Value>::New(isolate, persistent);
    }

    v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>>
        persistent;
    v8::Isolate *isolate;
} js_eina_log_print_cb_data;

static void js_eina_log_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level,
                                 const char *file, const char *fnc, int line,
                                 const char *fmt, void */*data*/, va_list args)
{
    v8::Isolate *const isolate = js_eina_log_print_cb_data.isolate;
    constexpr unsigned argc = 7;

    v8::Handle<v8::Value> argv[argc] = {
        v8::String::NewFromUtf8(isolate, d->name, v8::String::kNormalString,
                                d->namelen),
        v8::String::NewFromUtf8(isolate, d->color),
        v8::Integer::New(isolate, static_cast<int>(level)),
        v8::String::NewFromUtf8(isolate, file),
        v8::String::NewFromUtf8(isolate, fnc),
        v8::Integer::New(isolate, line),
        to_v8_string(isolate, fmt, args)
    };

    auto o = js_eina_log_print_cb_data.functor();
    v8::Function::Cast(*o)->Call(v8::Undefined(isolate), argc, argv);
}

static bool valid_level_conversion(int src, Eina_Log_Level &dst)
{
    if (src != EINA_LOG_LEVEL_CRITICAL && src != EINA_LOG_LEVEL_ERR
        && src != EINA_LOG_LEVEL_WARN && src != EINA_LOG_LEVEL_INFO
        && src != EINA_LOG_LEVEL_DBG)
        return false;

    dst = static_cast<Eina_Log_Level>(src);
    return true;
}

EAPI
void register_log_level_critical(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name)
{
    int value = EINA_LOG_LEVEL_CRITICAL;
    global->Set(name, value_cast<v8::Local<v8::Value>>(value, isolate));
}

EAPI
void register_log_level_err(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    int value = EINA_LOG_LEVEL_ERR;
    global->Set(name, value_cast<v8::Local<v8::Value>>(value, isolate));
}

EAPI
void register_log_level_warn(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    int value = EINA_LOG_LEVEL_WARN;
    global->Set(name, value_cast<v8::Local<v8::Value>>(value, isolate));
}

EAPI
void register_log_level_info(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    int value = EINA_LOG_LEVEL_INFO;
    global->Set(name, value_cast<v8::Local<v8::Value>>(value, isolate));
}

EAPI
void register_log_level_dbg(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    int value = EINA_LOG_LEVEL_DBG;
    global->Set(name, value_cast<v8::Local<v8::Value>>(value, isolate));
}

EAPI
void register_log_domain_global(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name)
{
    int value = EINA_LOG_DOMAIN_GLOBAL;
    global->Set(name, value_cast<v8::Local<v8::Value>>(value, isolate));
}

EAPI
void register_log_print(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::String;
    using v8::StackTrace;
    using v8::FunctionTemplate;

    auto print = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 3 || !info[0]->IsNumber() || !info[1]->IsNumber()
            || !info[2]->IsString())
            return;

        Eina_Log_Level level;
        if (!valid_level_conversion(info[1]->NumberValue(), level))
            return;

        auto frame = StackTrace::CurrentStackTrace(info.GetIsolate(), 1,
                                                   StackTrace::kDetailed)
            ->GetFrame(0);

        eina_log_print(info[0]->NumberValue(), level,
                       *String::Utf8Value(frame->GetScriptNameOrSourceURL()),
                       *String::Utf8Value(frame->GetFunctionName()),
                       frame->GetLineNumber(), "%s",
                       *String::Utf8Value(info[2]));
    };

    global->Set(name, FunctionTemplate::New(isolate, print)->GetFunction());
}

EAPI
void register_log_domain_register(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Local;
    using v8::String;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsString())
            return;

        int d = eina_log_domain_register(*String::Utf8Value(info[0]),
                                         *String::Utf8Value(info[1]));
        info.GetReturnValue().Set(value_cast<Local<Value>>
                                  (d, info.GetIsolate()));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_domain_unregister(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsNumber())
            return;

        eina_log_domain_unregister(info[0]->NumberValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_domain_registered_level_get(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Local;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsNumber())
            return;

        int l = eina_log_domain_registered_level_get(info[0]->NumberValue());
        info.GetReturnValue().Set(value_cast<Local<Value>>
                                  (l, info.GetIsolate()));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_domain_registered_level_set(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 2 || !info[0]->IsNumber() || !info[1]->IsNumber())
            return;

        eina_log_domain_registered_level_set(info[0]->NumberValue(),
                                             info[1]->NumberValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_print_cb_set(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsFunction())
            return;

        js_eina_log_print_cb_data.reset(info.GetIsolate(), info[0]);
        eina_log_print_cb_set(js_eina_log_print_cb, NULL);
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_level_set(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsNumber())
            return;

        eina_log_level_set(info[0]->NumberValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_level_get(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Integer;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        int l = eina_log_level_get();
        info.GetReturnValue().Set(Integer::New(info.GetIsolate(), l));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_level_check(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Boolean;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsNumber())
            return;

        bool b = eina_log_level_check(info[0]->NumberValue());
        info.GetReturnValue().Set(Boolean::New(info.GetIsolate(), b));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_color_disable_set(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsBoolean())
            return;

        eina_log_color_disable_set(info[0]->BooleanValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_color_disable_get(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        bool b = eina_log_color_disable_get();
        info.GetReturnValue().Set(Boolean::New(info.GetIsolate(), b));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_file_disable_set(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsBoolean())
            return;

        eina_log_file_disable_set(info[0]->BooleanValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_file_disable_get(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        bool b = eina_log_file_disable_get();
        info.GetReturnValue().Set(Boolean::New(info.GetIsolate(), b));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_function_disable_set(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsBoolean())
            return;

        eina_log_function_disable_set(info[0]->BooleanValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_function_disable_get(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        bool b = eina_log_function_disable_get();
        info.GetReturnValue().Set(Boolean::New(info.GetIsolate(), b));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_abort_on_critical_set(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsBoolean())
            return;

        eina_log_abort_on_critical_set(info[0]->BooleanValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_abort_on_critical_get(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        bool b = eina_log_abort_on_critical_get();
        info.GetReturnValue().Set(Boolean::New(info.GetIsolate(), b));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_abort_on_critical_level_set(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsNumber())
            return;

        eina_log_abort_on_critical_level_set(info[0]->NumberValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_abort_on_critical_level_get(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Integer;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        int l = eina_log_abort_on_critical_level_get();
        info.GetReturnValue().Set(Integer::New(info.GetIsolate(), l));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_domain_level_set(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::String;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsNumber())
            return;

        eina_log_domain_level_set(*String::Utf8Value(info[0]),
                                  info[1]->NumberValue());
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_domain_level_get(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::String;
    using v8::Integer;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1 || !info[0]->IsString())
            return;

        int l = eina_log_domain_level_get(*String::Utf8Value(info[0]));
        info.GetReturnValue().Set(Integer::New(info.GetIsolate(), l));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

EAPI
void register_log_state_start(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    int value = EINA_LOG_STATE_START;
    global->Set(name, v8::Integer::New(isolate, value));
}

EAPI
void register_log_state_stop(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name)
{
    int value = EINA_LOG_STATE_STOP;
    global->Set(name, v8::Integer::New(isolate, value));
}

EAPI
void register_log_timing(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::FunctionCallbackInfo;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::String;

    auto func = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 3 || !info[0]->IsNumber() || !info[1]->IsNumber()
            || !info[2]->IsString())
            return;

        eina_log_timing(info[0]->NumberValue(),
                        static_cast<Eina_Log_State>(info[1]->NumberValue()),
                        *String::Utf8Value(info[2]));
    };

    global->Set(name, FunctionTemplate::New(isolate, func)->GetFunction());
}

} } } // namespace efl { namespace js {
