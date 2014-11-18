#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <eina_js_error.hh>
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

void clear_eina_error(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    eina_error_set(0);
    efl::js::convert_error_to_javascript_exception(args.GetIsolate());
}

void set_eina_error(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    eina_error_set(eina_error_msg_static_register("foobar"));
    efl::js::convert_error_to_javascript_exception(args.GetIsolate());
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
    global->Set(v8::String::NewFromUtf8(isolate, "clear_eina_error"),
                v8::FunctionTemplate::New(isolate, clear_eina_error)
                ->GetFunction());
    global->Set(v8::String::NewFromUtf8(isolate, "set_eina_error"),
                v8::FunctionTemplate::New(isolate, set_eina_error)
                ->GetFunction());

    {
        v8::HandleScope handle_scope(isolate);
        v8::TryCatch try_catch;
        auto source = v8::String::NewFromUtf8(isolate, script);
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
