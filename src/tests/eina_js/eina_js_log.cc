#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <cassert>
#include <eina_js_value.hh>
#include <eina_js_log.hh>
#include <Eo.hh>

void print(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    std::cout << std::string(*str, str.length());
  }
  printf("\n");
  fflush(stdout);
}

static void eina_log_print_cb_js_test(const Eina_Log_Domain *d,
                                      Eina_Log_Level level, const char *file,
                                      const char *fnc, int line,
                                      const char */*fmt*/, void */*data*/,
                                      va_list args)
{
    static int index = 0;

    const char *msg = va_arg(args, const char*);

    std::string domains[] = {"", "", "", "mydomain", "mydomain2"};
    int levels[] = {EINA_LOG_LEVEL_DBG, EINA_LOG_LEVEL_CRITICAL,
                    EINA_LOG_LEVEL_WARN, EINA_LOG_LEVEL_INFO,
                    EINA_LOG_LEVEL_ERR};
    std::string functions[] = {"f1", "", "f2", "f3", ""};
    int lines[] = {2, 3, 4, 5, 9};
    std::string messages[] = {"I changed again", "Cool to Hate",
                              "One Fine Day", "Never Gonna Find Me",
                              "The Kids Aren't Alright"};

    assert(std::string(d->name, d->namelen) == domains[index]);
    assert(level == levels[index]);
    assert(file == std::string("offspring.js"));
    assert(fnc == functions[index]);
    assert(line == lines[index]);
    assert(msg == messages[index]);
    ++index;
}

#define X(x, y) "function " x "(){" y "};" x "();"

static const char raw_script[] =
    "function assert(test, message) { if (test !== true) throw message; };\n"

    X("f1", "log_print(LOG_DOMAIN_GLOBAL, LOG_LEVEL_DBG, 'I changed again');\n")
    "log_print(LOG_DOMAIN_GLOBAL, LOG_LEVEL_CRITICAL, 'Cool to Hate');\n"
    X("f2", "log_print(LOG_DOMAIN_GLOBAL, LOG_LEVEL_WARN, 'One Fine Day');\n")
    X("f3", "log_print(mydomain, LOG_LEVEL_INFO, 'Never Gonna Find Me');\n")

    "mydomain2 = log_domain_register('mydomain2', '');\n"
    "log_domain_registered_level_set(mydomain2, LOG_LEVEL_DBG);\n"
    "assert(log_domain_registered_level_get(mydomain2) === LOG_LEVEL_DBG,"
    "       '#1');\n"

    "log_print(mydomain2, LOG_LEVEL_ERR, \"The Kids Aren't Alright\");\n"

    "log_domain_unregister(mydomain2);\n"
    "mydomain2 = undefined;\n"

    "log_color_disable_set(true);\n"
    "assert(log_color_disable_get() === true, '#2');\n"
    "log_color_disable_set(false);\n"
    "assert(log_color_disable_get() === false, '#3');\n"

    "log_file_disable_set(true);\n"
    "assert(log_file_disable_get() === true, '#4');\n"
    "log_file_disable_set(false);\n"
    "assert(log_file_disable_get() === false, '#5');\n"

    "log_function_disable_set(true);\n"
    "assert(log_function_disable_get() === true, '#6');\n"
    "log_function_disable_set(false);\n"
    "assert(log_function_disable_get() === false, '#7');\n"

    "log_abort_on_critical_set(true);\n"
    "assert(log_abort_on_critical_get() === true, '#8');\n"
    "log_abort_on_critical_set(false);\n"
    "assert(log_abort_on_critical_get() === false, '#9');\n"

    "var entered = false;\n"
    "log_print_cb_set(function(domain, color, level, file, func, line, msg) {"
    "  assert(domain === 'mydomain', '#10');\n"
    "  assert(color === '', '#11');\n"
    "  assert(level === LOG_LEVEL_WARN, '#12');\n"
    "  assert(file === 'offspring.js', '#13');\n"
    "  assert(func === 'f4', '#14');\n"
    "  assert(line === 33, '#15');\n"
    "  assert(msg === 'What Happened To You', '#16');\n"
    "  entered = true;"
    "});\n"

    X("f4", "log_print(mydomain, LOG_LEVEL_WARN, 'What Happened To You');\n")
    "assert(entered === true, '#17');\n"

    "assert(log_level_get() === LOG_LEVEL_DBG, '#18');\n"
    "assert(log_level_check(LOG_LEVEL_INFO) === true, '#19');\n"
    "log_level_set(LOG_LEVEL_CRITICAL);\n"
    "assert(log_level_get() === LOG_LEVEL_CRITICAL, '#20');\n"
    "assert(log_level_check(LOG_LEVEL_INFO) === false, '#21');\n"

    "assert(log_abort_on_critical_get() === false, '#22');\n"
    "log_abort_on_critical_set(true);\n"
    "assert(log_abort_on_critical_get() === true, '#23');\n"

    "log_abort_on_critical_level_set(LOG_LEVEL_CRITICAL);"
    "assert(log_abort_on_critical_level_get() == LOG_LEVEL_CRITICAL, '#24');\n"
    "log_abort_on_critical_level_set(LOG_LEVEL_ERR);"
    "assert(log_abort_on_critical_level_get() == LOG_LEVEL_ERR, '#25');\n"

    "log_domain_level_set('mydomain', LOG_LEVEL_WARN);\n"
    "assert(log_domain_level_get('mydomain') === LOG_LEVEL_WARN, '#26');\n"
    "log_domain_level_set('mydomain', LOG_LEVEL_INFO);\n"
    "assert(log_domain_level_get('mydomain') === LOG_LEVEL_INFO, '#27');\n"

    "assert(typeof(LOG_STATE_START) === 'number', '#28');\n"
    "assert(typeof(LOG_STATE_STOP) === 'number', '#29');\n"
    "assert(typeof(log_timing) === 'function', '#30');\n"
    ;

#undef X

int main(int argc, char *argv[])
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  v8::V8::Initialize();
  v8::V8::InitializeICU();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::Isolate* isolate = v8::Isolate::New();

  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Context> context
      = v8::Context::New(isolate, NULL, v8::ObjectTemplate::New(isolate));

  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
  }

  {
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Handle<v8::Object> global = context->Global();

    global->Set(v8::String::NewFromUtf8(isolate, "print"),
                v8::FunctionTemplate::New(isolate, print)->GetFunction());
    {
        int d = eina_log_domain_register("mydomain", "");
        eina_log_domain_registered_level_set(d, EINA_LOG_LEVEL_DBG);
        global->Set(v8::String::NewFromUtf8(isolate, "mydomain"),
                    efl::js::value_cast<v8::Local<v8::Value>>(d, isolate));
    }
    efl::js::register_log_level_critical(isolate, global,
                                         v8::String::NewFromUtf8(isolate,
                                                                 "LOG_LEVEL"
                                                                 "_CRITICAL"));
    efl::js::register_log_level_err(isolate, global,
                                    v8::String::NewFromUtf8(isolate,
                                                            "LOG_LEVEL_ERR"));
    efl::js::register_log_level_warn(isolate, global,
                                     v8::String::NewFromUtf8(isolate,
                                                             "LOG_LEVEL_WARN"));
    efl::js::register_log_level_info(isolate, global,
                                     v8::String::NewFromUtf8(isolate,
                                                             "LOG_LEVEL_INFO"));
    efl::js::register_log_level_dbg(isolate, global,
                                    v8::String::NewFromUtf8(isolate,
                                                            "LOG_LEVEL_DBG"));
    efl::js::register_log_domain_global(isolate, global,
                                        v8::String::NewFromUtf8(isolate,
                                                                "LOG_DOMAIN"
                                                                "_GLOBAL"));
    efl::js::register_log_print(isolate, global,
                                v8::String::NewFromUtf8(isolate, "log_print"));
    efl::js::register_log_domain_register(isolate, global,
                                          v8::String::NewFromUtf8(isolate,
                                                                  "log_domain"
                                                                  "_register"));
    efl::js::register_log_domain_unregister(isolate, global,
                                            v8::String::NewFromUtf8(isolate,
                                                                    "log_domain"
                                                                    "_unregis"
                                                                    "ter"));
    efl::js
    ::register_log_domain_registered_level_get(isolate, global,
                                               v8::String
                                               ::NewFromUtf8(isolate,
                                                             "log_domain"
                                                             "_registered_level"
                                                             "_get"));
    efl::js
    ::register_log_domain_registered_level_set(isolate, global,
                                               v8::String
                                               ::NewFromUtf8(isolate,
                                                             "log_domain"
                                                             "_registered_level"
                                                             "_set"));
    efl::js::register_log_print_cb_set(isolate, global,
                                       v8::String::NewFromUtf8(isolate,
                                                               "log_print_cb"
                                                               "_set"));
    efl::js::register_log_level_set(isolate, global,
                                    v8::String::NewFromUtf8(isolate,
                                                            "log_level_set"));
    efl::js::register_log_level_get(isolate, global,
                                    v8::String::NewFromUtf8(isolate,
                                                            "log_level_get"));
    efl::js::register_log_level_check(isolate, global,
                                      v8::String::NewFromUtf8(isolate,
                                                              "log_level"
                                                              "_check"));
    efl::js::register_log_color_disable_set(isolate, global,
                                            v8::String::NewFromUtf8(isolate,
                                                                    "log_color"
                                                                    "_disable"
                                                                    "_set"));
    efl::js::register_log_color_disable_get(isolate, global,
                                            v8::String::NewFromUtf8(isolate,
                                                                    "log_color"
                                                                    "_disable"
                                                                    "_get"));
    efl::js::register_log_file_disable_set(isolate, global,
                                           v8::String::NewFromUtf8(isolate,
                                                                   "log_file"
                                                                   "_disable"
                                                                   "_set"));
    efl::js::register_log_file_disable_get(isolate, global,
                                           v8::String::NewFromUtf8(isolate,
                                                                   "log_file"
                                                                   "_disable"
                                                                   "_get"));
    efl::js
    ::register_log_abort_on_critical_set(isolate, global,
                                         v8::String::NewFromUtf8(isolate,
                                                                 "log_abort_on"
                                                                 "_critical"
                                                                 "_set"));
    efl::js
    ::register_log_abort_on_critical_get(isolate, global,
                                         v8::String::NewFromUtf8(isolate,
                                                                 "log_abort_on"
                                                                 "_critical"
                                                                 "_get"));
    efl::js
    ::register_log_function_disable_set(isolate, global,
                                        v8::String::NewFromUtf8(isolate,
                                                                "log_function"
                                                                "_disable"
                                                                "_set"));
    efl::js
    ::register_log_function_disable_get(isolate, global,
                                        v8::String::NewFromUtf8(isolate,
                                                                "log_function"
                                                                "_disable"
                                                                "_get"));
    efl::js
    ::register_log_abort_on_critical_set(isolate, global,
                                         v8::String::NewFromUtf8(isolate,
                                                                 "log_abort_on"
                                                                 "_critical"
                                                                 "_set"));
    efl::js
    ::register_log_abort_on_critical_get(isolate, global,
                                         v8::String::NewFromUtf8(isolate,
                                                                 "log_abort_on"
                                                                 "_critical"
                                                                 "_get"));
    efl::js
    ::register_log_abort_on_critical_level_set(isolate, global,
                                               v8::String
                                               ::NewFromUtf8(isolate,
                                                             "log_abort_on"
                                                             "_critical"
                                                             "_level_set"));
    efl::js
    ::register_log_abort_on_critical_level_get(isolate, global,
                                               v8::String
                                               ::NewFromUtf8(isolate,
                                                             "log_abort_on"
                                                             "_critical"
                                                             "_level_get"));
    efl::js::register_log_domain_level_set(isolate, global,
                                           v8::String::NewFromUtf8(isolate,
                                                                   "log_domain"
                                                                   "_level"
                                                                   "_set"));
    efl::js::register_log_domain_level_get(isolate, global,
                                           v8::String::NewFromUtf8(isolate,
                                                                   "log_domain"
                                                                   "_level"
                                                                   "_get"));
    efl::js::register_log_state_start(isolate, global,
                                      v8::String::NewFromUtf8(isolate,
                                                              "LOG_STATE"
                                                              "_START"));
    efl::js::register_log_state_stop(isolate, global,
                                     v8::String::NewFromUtf8(isolate,
                                                             "LOG_STATE_STOP"));
    efl::js::register_log_timing(isolate, global,
                                 v8::String::NewFromUtf8(isolate,
                                                         "log_timing"));

    eina_log_print_cb_set(eina_log_print_cb_js_test, NULL);
    eina_log_level_set(EINA_LOG_LEVEL_DBG);
    eina_log_abort_on_critical_set(EINA_FALSE);

    {
        v8::HandleScope handle_scope(isolate);
        v8::TryCatch try_catch;
        v8::Handle<v8::Script> script = [&]() {
            auto source = v8::String::NewFromUtf8(isolate, raw_script);
            auto filename = v8::String::NewFromUtf8(isolate, "offspring.js");
            return v8::Script::Compile(source, filename);
        }();

        assert(!script.IsEmpty());

        /*v8::Handle<v8::Value> result = */script->Run();

        if (try_catch.HasCaught()) {
            v8::String::Utf8Value message(try_catch.Message()->Get());
            std::cerr << std::string(*message, message.length()) << std::endl;
            std::abort();
        }
    }
  }
}
