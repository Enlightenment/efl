#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <eina_js_iterator.hh>
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

static const char script[] =
    "function assert(test, message) { if (test !== true) throw message; };"

    "assert(it.next().value === 42, '#1');"
    "assert(it.next().value === 24, '#2');"
    "destroy_iterator(it);"
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
    efl::js
    ::register_destroy_iterator(isolate, global,
                                v8::String::NewFromUtf8(isolate,
                                                        "destroy_iterator"));

    Eina_Array *array = [](){
        static int impl[3] = {42, 24, 0};
        Eina_Array *a = eina_array_new(3);
        eina_array_push(a, impl);
        eina_array_push(a, impl+1);
        eina_array_push(a, impl+2);
        return a;
    }();
    efl::eina::iterator<int> it(eina_array_iterator_new(array));

    v8::Local<v8::Object> wrapped_it = efl::js::export_iterator(&it, isolate);

    global->Set(v8::String::NewFromUtf8(isolate, "it"), wrapped_it);

    assert(**efl::js::import_iterator<int>(wrapped_it) == 42);

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
