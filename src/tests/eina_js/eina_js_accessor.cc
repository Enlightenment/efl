#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <eina_js_accessor.hh>
#include <eina_js_compatibility.hh>
#include <Eo.hh>
#include <Eina.hh>

#include <iostream>

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

    "assert(acc.get(0) === 42, '#1');"
    "assert(acc.get(1) === 24, '#2');"
    "destroy_accessor(acc);"
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
    efl::eina::js::register_destroy_accessor
      (isolate, global
       , efl::eina::js::compatibility_new<v8::String>(isolate, "destroy_accessor"));

    Eina_Array *array = [](){
        static int impl[2] = {42, 24};
        Eina_Array *a = eina_array_new(2);
        eina_array_push(a, impl);
        eina_array_push(a, impl+1);
        return a;
    }();
    efl::eina::accessor<int> acc(eina_array_accessor_new(array));

    v8::Local<v8::Object> wrapped_acc = efl::eina::js::export_accessor(acc, isolate);

    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "acc"), wrapped_acc);

    assert(efl::eina::js::import_accessor<int>(wrapped_acc)[0] == 42);

    {
        efl::eina::js::compatibility_handle_scope h(isolate);
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

    eina_array_free(array);
  }
}
