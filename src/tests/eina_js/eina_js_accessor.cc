#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <eina_js_accessor.hh>
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

    "assert(acc.get(0) === 42, '#1');"
    "assert(acc.get(1) === 24, '#2');"
    "destroy_accessor(acc);"
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
    efl::js::register_destroy_accessor(isolate, global,
                                       v8::String::NewFromUtf8(isolate,
                                                               "destroy_accessor"));

    Eina_Array *array = [](){
        static int impl[2] = {42, 24};
        Eina_Array *a = eina_array_new(2);
        eina_array_push(a, impl);
        eina_array_push(a, impl+1);
        return a;
    }();
    efl::eina::accessor<int> acc(eina_array_accessor_new(array));

    v8::Local<v8::Object> wrapped_acc = efl::js::export_accessor(acc, isolate);

    global->Set(v8::String::NewFromUtf8(isolate, "acc"), wrapped_acc);

    assert(efl::js::import_accessor<int>(wrapped_acc)[0] == 42);

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

    eina_array_free(array);
  }
}
