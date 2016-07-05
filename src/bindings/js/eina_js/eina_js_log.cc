#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <map>
#include <Eina_Js.hh>

namespace efl { namespace eina { namespace js {

v8::Local<v8::String> to_v8_string(v8::Isolate *isolate, const char *fmt,
                                   va_list args)
{
    using v8::String;
#if 0
    /* TODO: unfortunately, the elegant and exception-safe version isn't
       compiling (yet!) */
    efl::eina::stringshare s(eina_stringshare_vprintf(fmt, args),
                             efl::eina::steal_stringshare_ref);
    return compatibility_new<String>(isolate, s.data(), String::kNormalString,
                                     s.size());
#else
    auto s = eina_stringshare_vprintf(fmt, args);
    auto ret = compatibility_new<String>(isolate, s);
    eina_stringshare_del(s);
    return ret;
#endif
}

static global_ref<v8::Value> js_eina_log_print_cb_data;
static std::map<int, const char *> js_eina_log_color_map;

static void js_eina_log_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level,
                                 const char *file, const char *fnc, int line,
                                 const char *fmt, void */*data*/, va_list args)
{
    using v8::String;
    using v8::Integer;
    using v8::Isolate;

    Isolate *const isolate = Isolate::GetCurrent();
    constexpr unsigned argc = 7;

    v8::Handle<v8::Value> argv[argc] = {
        compatibility_new<String>(isolate, d->name ? d->name : ""),
        compatibility_new<String>(isolate, d->color ? d->color : ""),
        compatibility_new<Integer>(isolate, static_cast<int>(level)),
        compatibility_new<String>(isolate, file),
        compatibility_new<String>(isolate, fnc),
        compatibility_new<Integer>(isolate, line),
        to_v8_string(isolate, fmt, args)
    };

    auto o = js_eina_log_print_cb_data.handle();
    v8::Function::Cast(*o)->Call(o->ToObject(), argc, argv);
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
void register_log_print(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::String;
    using v8::StackTrace;
    using v8::FunctionTemplate;

    auto print = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 3 || !args[0]->IsNumber() || !args[1]->IsNumber()
            || !args[2]->IsString())
            return compatibility_return();

        Eina_Log_Level level;
        if (!valid_level_conversion(args[1]->NumberValue(), level))
            return compatibility_return();

        auto frame = compatibility_current_stack_trace<>(args.GetIsolate(), 1,
                             StackTrace::kDetailed)->GetFrame(0);

        eina_log_print(args[0]->NumberValue(), level,
                       *String::Utf8Value(frame->GetScriptNameOrSourceURL()),
                       *String::Utf8Value(frame->GetFunctionName()),
                       frame->GetLineNumber(), "%s",
                       *String::Utf8Value(args[2]));
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, print)
                ->GetFunction());
}

EAPI
void register_log_domain_register(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Local;
    using v8::String;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString())
            return compatibility_return();

        // We duplicate the color string as eina takes a const char* but does take care of 
        // its lifetime, assuming a ever lasting string.
        const char *color = strdup(*String::Utf8Value(args[1]));
        int d = eina_log_domain_register(*String::Utf8Value(args[0]),
                                         color);
        js_eina_log_color_map[d] = color;

        auto isolate = args.GetIsolate();
        return compatibility_return(value_cast<Local<Value>>(d, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_domain_unregister(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        int domain = args[0]->NumberValue();
        eina_log_domain_unregister(domain);
        free((void*)js_eina_log_color_map[domain]);
        js_eina_log_color_map.erase(domain);
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_domain_registered_level_get(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Local;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        int l = eina_log_domain_registered_level_get(args[0]->NumberValue());
        auto isolate = args.GetIsolate();
        return compatibility_return(value_cast<Local<Value>>(l, isolate), args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_domain_registered_level_set(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber())
            return compatibility_return();

        eina_log_domain_registered_level_set(args[0]->NumberValue(),
                                             args[1]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_print_cb_set(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsFunction())
            return compatibility_return();

        js_eina_log_print_cb_data
          = global_ref<v8::Value>(args.GetIsolate(), args[0]);
        eina_log_print_cb_set(js_eina_log_print_cb, NULL);
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_level_set(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        eina_log_level_set(args[0]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_level_get(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Integer;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        int l = eina_log_level_get();
        auto ret = compatibility_new<Integer>(args.GetIsolate(), l);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_level_check(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Boolean;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        bool b = eina_log_level_check(args[0]->NumberValue());
        auto ret = compatibility_new<Boolean>(args.GetIsolate(), b);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_color_disable_set(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsBoolean())
            return compatibility_return();

        eina_log_color_disable_set(args[0]->BooleanValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_color_disable_get(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        bool b = eina_log_color_disable_get();
        auto ret = compatibility_new<Boolean>(args.GetIsolate(), b);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_file_disable_set(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsBoolean())
            return compatibility_return();

        eina_log_file_disable_set(args[0]->BooleanValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_file_disable_get(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        bool b = eina_log_file_disable_get();
        auto ret = compatibility_new<Boolean>(args.GetIsolate(), b);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_function_disable_set(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsBoolean())
            return compatibility_return();

        eina_log_function_disable_set(args[0]->BooleanValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_function_disable_get(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        bool b = eina_log_function_disable_get();
        auto ret = compatibility_new<Boolean>(args.GetIsolate(), b);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_abort_on_critical_set(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsBoolean())
            return compatibility_return();

        eina_log_abort_on_critical_set(args[0]->BooleanValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_abort_on_critical_get(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::Boolean;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        bool b = eina_log_abort_on_critical_get();
        auto ret = compatibility_new<Boolean>(args.GetIsolate(), b);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_abort_on_critical_level_set(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsNumber())
            return compatibility_return();

        eina_log_abort_on_critical_level_set(args[0]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_abort_on_critical_level_get(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::Integer;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 0)
            return compatibility_return();

        int l = eina_log_abort_on_critical_level_get();
        auto ret = compatibility_new<Integer>(args.GetIsolate(), l);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_domain_level_set(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::String;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsNumber())
            return compatibility_return();

        eina_log_domain_level_set(*String::Utf8Value(args[0]),
                                  args[1]->NumberValue());
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_domain_level_get(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::String;
    using v8::Integer;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 1 || !args[0]->IsString())
            return compatibility_return();

        int l = eina_log_domain_level_get(*String::Utf8Value(args[0]));
        auto ret = compatibility_new<Integer>(args.GetIsolate(), l);
        return compatibility_return(ret, args);
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

EAPI
void register_log_timing(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name)
{
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::String;

    auto f = [](compatibility_callback_info_type args)
        -> compatibility_return_type {
        if (args.Length() != 3 || !args[0]->IsNumber() || !args[1]->IsNumber()
            || !args[2]->IsString()) {
            return compatibility_return();
        }

        eina_log_timing(args[0]->NumberValue(),
                        static_cast<Eina_Log_State>(args[1]->NumberValue()),
                        *String::Utf8Value(args[2]));
        return compatibility_return();
    };

    global->Set(name,
                compatibility_new<FunctionTemplate>(isolate, f)->GetFunction());
}

static void register_constant(v8::Isolate *isolate, v8::Handle<v8::Object> global, int value, const char* name)
{
    global->Set(compatibility_new<v8::String>(isolate, name), compatibility_new<v8::Integer>(isolate, value));
}

static void register_constant(v8::Isolate *isolate, v8::Handle<v8::Object> global, const char* value, const char* name)
{
    global->Set(compatibility_new<v8::String>(isolate, name), compatibility_new<v8::String>(isolate, value));
}

EAPI void register_log_constants(v8::Isolate *isolate, v8::Handle<v8::Object> global)
{
    register_constant(isolate, global, EINA_LOG_STATE_START, "LOG_STATE_START");
    register_constant(isolate, global, EINA_LOG_STATE_START, "LOG_STATE_STOP");

    register_constant(isolate, global, EINA_LOG_LEVEL_CRITICAL, "LOG_LEVEL_CRITICAL");
    register_constant(isolate, global, EINA_LOG_LEVEL_ERR, "LOG_LEVEL_ERR");
    register_constant(isolate, global, EINA_LOG_LEVEL_WARN, "LOG_LEVEL_WARN");
    register_constant(isolate, global, EINA_LOG_LEVEL_INFO, "LOG_LEVEL_INFO");
    register_constant(isolate, global, EINA_LOG_LEVEL_DBG, "LOG_LEVEL_DBG");
    register_constant(isolate, global, EINA_LOG_LEVELS, "LOG_LEVELS");
    register_constant(isolate, global, EINA_LOG_LEVEL_UNKNOWN, "LOG_LEVEL_UNKNOWN");

    register_constant(isolate, global, EINA_LOG_DOMAIN_GLOBAL, "LOG_DOMAIN_GLOBAL");

    register_constant(isolate, global, EINA_COLOR_LIGHTRED, "COLOR_LIGHTRED");
    register_constant(isolate, global, EINA_COLOR_RED, "COLOR_RED");
    register_constant(isolate, global, EINA_COLOR_LIGHTBLUE, "COLOR_LIGHTBLUE");
    register_constant(isolate, global, EINA_COLOR_BLUE, "COLOR_BLUE");
    register_constant(isolate, global, EINA_COLOR_GREEN, "COLOR_GREEN");
    register_constant(isolate, global, EINA_COLOR_YELLOW, "COLOR_YELLOW");
    register_constant(isolate, global, EINA_COLOR_ORANGE, "COLOR_ORANGE");
    register_constant(isolate, global, EINA_COLOR_WHITE, "COLOR_WHITE");
    register_constant(isolate, global, EINA_COLOR_LIGHTCYAN, "COLOR_LIGHTCYAN");
    register_constant(isolate, global, EINA_COLOR_CYAN, "COLOR_CYAN");
    register_constant(isolate, global, EINA_COLOR_RESET, "COLOR_RESET");
    register_constant(isolate, global, EINA_COLOR_HIGH, "COLOR_HIGH");
}


#define REGISTER_LOG_HELPER(isolate, global, level, name) \
{ \
    using v8::String; \
    using v8::FunctionTemplate;\
    using v8::StackTrace;\
\
    auto wrapper = [](compatibility_callback_info_type args)\
        -> compatibility_return_type {\
        if (args.Length() != 1 || !args[0]->IsString()) {\
            eina::js::compatibility_throw \
                (args.GetIsolate(), v8::Exception::TypeError(\
                                        eina::js::compatibility_new<v8::String>(args.GetIsolate(), "Message must be a string."))); \
            return compatibility_return();\
        } \
\
        auto frame = compatibility_current_stack_trace<>(args.GetIsolate(), 1,\
                             StackTrace::kDetailed)->GetFrame(0);\
\
        eina_log_print(EINA_LOG_DOMAIN_GLOBAL, level,\
                       *String::Utf8Value(frame->GetScriptNameOrSourceURL()),\
                       *String::Utf8Value(frame->GetFunctionName()),\
                       frame->GetLineNumber(), "%s",\
                       *String::Utf8Value(args[0]));\
\
        return compatibility_return();\
    };\
\
    global->Set(compatibility_new<String>(isolate, name),\
                compatibility_new<FunctionTemplate>(isolate, wrapper)->GetFunction());\
}

EAPI void register_log_helpers(v8::Isolate *isolate, v8::Handle<v8::Object> global)
{
    REGISTER_LOG_HELPER(isolate, global, EINA_LOG_LEVEL_CRITICAL, "logCritical");
    REGISTER_LOG_HELPER(isolate, global, EINA_LOG_LEVEL_ERR, "logError");
    REGISTER_LOG_HELPER(isolate, global, EINA_LOG_LEVEL_WARN, "logWarning");
    REGISTER_LOG_HELPER(isolate, global, EINA_LOG_LEVEL_INFO, "logInfo");
    REGISTER_LOG_HELPER(isolate, global, EINA_LOG_LEVEL_DBG, "logDebug");
}

} } } // namespace efl { namespace js {

EAPI
void eina_log_register(v8::Handle<v8::Object> exports, v8::Isolate* isolate)
{
    using efl::eina::js::compatibility_new;

    efl::eina::js::register_log_domain_register(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "registerLogDomain"));
    efl::eina::js::register_log_domain_unregister(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "unregisterLogDomain"));
    efl::eina::js::register_log_domain_registered_level_set(isolate, exports
                                             , compatibility_new<v8::String>(isolate, "setLogDomainRegisteredLevel"));
    efl::eina::js::register_log_domain_registered_level_get(isolate, exports
                                             , compatibility_new<v8::String>(isolate, "getLogDomainRegisteredLevel"));

    efl::eina::js::register_log_print(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "logPrint"
));
    efl::eina::js::register_log_print_cb_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogPrintCb"
));

    efl::eina::js::register_log_level_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogLevel"
));
    efl::eina::js::register_log_level_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogLevel"
));
    efl::eina::js::register_log_level_check(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "checkLogLevel"
));

    efl::eina::js::register_log_color_disable_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogColorDisable"
));
    efl::eina::js::register_log_color_disable_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogColorDisable"
));

    efl::eina::js::register_log_file_disable_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogFileDisable"
));
    efl::eina::js::register_log_file_disable_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogFileDisable"
));

    efl::eina::js::register_log_function_disable_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogFunctionDisable"
));
    efl::eina::js::register_log_function_disable_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogFunctionDisable"
));

    efl::eina::js::register_log_abort_on_critical_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogAbortOnCritical"
));
    efl::eina::js::register_log_abort_on_critical_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogAbortOnCritical"
));
    efl::eina::js::register_log_abort_on_critical_level_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogAbortOnCriticalLevel"
));
    efl::eina::js::register_log_abort_on_critical_level_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogAbortOnCriticalLevel"
));

    efl::eina::js::register_log_domain_level_set(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "setLogDomainLevel"
));
    efl::eina::js::register_log_domain_level_get(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "getLogDomainLevel"
));

    efl::eina::js::register_log_timing(isolate, exports
                                            , compatibility_new<v8::String>(isolate, "logTiming"));

    efl::eina::js::register_log_constants(isolate, exports);

    efl::eina::js::register_log_helpers(isolate, exports);
}
