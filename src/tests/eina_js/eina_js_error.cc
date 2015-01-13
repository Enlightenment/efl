#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <eina_js_error.hh>
#include <eina_js_compatibility.hh>
#include <Eo.hh>

efl::eina::js::compatibility_return_type print(efl::eina::js::compatibility_callback_info_type args)
{
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    efl::eina::js::compatibility_handle_scope handle_scope(args.GetIsolate());
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
  return efl::eina::js::compatibility_return();
}


static const char script[] =
    "function assert(test, message) { if (test !== true) throw message; };"

    "var captured = false;"
    "try {"
    "  clear_eina_error();"
    "} catch(e) {"
    "  captured = true;"
    "}"
    "assert(captured === false, '#1');"

    "captured = false;"
    "try {"
    "  set_eina_error();"
    "} catch(e) {"
    "  assert(e.code === 'Eina_Error', '#2');"
    "  assert(e.value === 'foobar', '#3');"
    "  captured = true;"
    "}"
    "assert(captured === true, '#4');"
    ;

int main(int argc, char *argv[])
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::js::compatibility_initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::Isolate* isolate = v8::Isolate::New();

  v8::Isolate::Scope isolate_scope(isolate);
  efl::eina::js::compatibility_handle_scope handle_scope(isolate);
  v8::Handle<v8::Context> context
    = efl::eina::js::compatibility_new<v8::Context>
    (isolate, nullptr, efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate));

  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
  }

  {
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Handle<v8::Object> global = context->Global();

    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "print"),
                efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, print)->GetFunction());
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "clear_eina_error"),
                efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, clear_eina_error)
                ->GetFunction());
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "set_eina_error"),
                efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, set_eina_error)
                ->GetFunction());

    {
        efl::eina::js::compatibility_handle_scope handle_scope(isolate);
        v8::TryCatch try_catch;
        auto source = efl::eina::js::compatibility_new<v8::String>(isolate, script);
        v8::Handle<v8::Script> script = v8::Script::Compile(std::move(source));

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
