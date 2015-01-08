
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include V8_INCLUDE_HEADER

#include <cassert>
#include <cstdlib>

#include <Eina.h>
#include <Eina.hh>
#include <Eo.hh>

#include <eina_js_list.hh>

static const char script[] =
  "function assert(condition, message) {\n"
  "  if (!condition) {\n"
  "      print(\"Assertion failed \", message);\n"
  "      throw message || \"Assertion failed\";\n"
  "  }\n"
  "}\n"
  "print(\"teste\");\n"
  "var l1 = raw_list;\n"
  "print (\"l1 \", l1.toString());\n"
  "assert (l1.length == 3)\n;\n"
  "var l2 = raw_list;\n"
  "print (\"l2 \", l2.toString());\n"
  "assert (l2.length == 3)\n;\n"
  "var c = l1.concat(l2);\n"
  "print (\"c \", c.toString());\n"
  "assert (c.length == (l1.length + l2.length));\n"
  "assert (c[0] == l1[0]);\n"
  "assert (c[1] == l1[1]);\n"
  "assert (c[2] == l1[2]);\n"
  "assert (c[3] == l2[0]);\n"
  "assert (c[4] == l2[1]);\n"
  "assert (c[5] == l2[2]);\n"
  "assert (c.indexOf(c[0]) == 0);\n"
  "assert (c.indexOf(c[1]) == 1);\n"
  "assert (c.indexOf(c[2]) == 2);\n"
  "assert (c.indexOf(c[3]) == 0);\n"
  "assert (c.indexOf(c[4]) == 1);\n"
  "assert (c.indexOf(c[5]) == 2);\n"
  "assert (c.lastIndexOf(c[0]) == 3);\n"
  "assert (c.lastIndexOf(c[1]) == 4);\n"
  "assert (c.lastIndexOf(c[2]) == 5);\n"
  "assert (c.lastIndexOf(c[3]) == 3);\n"
  "assert (c.lastIndexOf(c[4]) == 4);\n"
  "assert (c.lastIndexOf(c[5]) == 5);\n"
  "var s1 = l1.slice(1, 3);\n"
  "print (\"s1 \", s1.toString());\n"
  "assert (s1.length == 2);\n"
  "assert (s1[0] == l1[1]);\n"
  "assert (s1[1] == l1[2]);\n"
  "var s2 = c.slice(1, 3);\n"
  "print (\"s2 \", s2.toString());\n"
  "assert (s2.length == 2);\n"
  "assert (s2[0] == l1[1]);\n"
  "assert (s2[1] == l1[2]);\n"
  ;

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name)
{
  efl::eina::js::compatibility_handle_scope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::ScriptOrigin origin(name);
  v8::Handle<v8::Script> script = v8::Script::Compile(source, &origin);
  if (script.IsEmpty()) {
    std::abort();
    // Print errors that happened during compilation.
    // if (report_exceptions)
    //   ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      std::cout << "Failed with exception thrown" << std::endl;
      assert(try_catch.HasCaught());
      std::abort();
      // Print errors that happened during execution.
      // if (report_exceptions)
      //   ReportException(isolate, &try_catch);
      return false;
    } else {
      assert(!try_catch.HasCaught());
      // if (print_result && !result->IsUndefined()) {
      //   // If all went well and the result wasn't undefined then print
      //   // the returned value.
      //   v8::String::Utf8Value str(result);
      //   const char* cstr = ToCString(str);
      //   printf("%s\n", cstr);
      // }
      return true;
    }
  }
}

efl::eina::js::compatibility_return_type Print(efl::eina::js::compatibility_callback_info_type args)
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
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
  return efl::eina::js::compatibility_return();
}

EAPI void eina_container_register(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI v8::Handle<v8::FunctionTemplate> get_list_instance_template();

int main(int, char*[])
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::js::compatibility_initialize();
  v8::Isolate* isolate = v8::Isolate::New();
  assert(isolate != 0);

  v8::Isolate::Scope isolate_scope(isolate);
  
  efl::eina::js::compatibility_handle_scope handle_scope(isolate);
  v8::Handle<v8::Context> context;

  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));

  efl::eina::js::range_eina_list<int> raw_list(list.native_handle());
  {
    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate);
    // Bind the global 'print' function to the C++ Print callback.
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "print"),
                efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, Print));
    // // Bind the global 'read' function to the C++ Read callback.
    // global->Set(v8::String::NewFromUtf8(isolate, "read"),
    //             v8::FunctionTemplate::New(isolate, Read));
    // // Bind the global 'load' function to the C++ Load callback.
    // global->Set(v8::String::NewFromUtf8(isolate, "load"),
    //             v8::FunctionTemplate::New(isolate, Load));
    // // Bind the 'quit' function
    // global->Set(v8::String::NewFromUtf8(isolate, "quit"),
    //             v8::FunctionTemplate::New(isolate, Quit));
    // // Bind the 'version' function
    // global->Set(v8::String::NewFromUtf8(isolate, "version"),
    //             v8::FunctionTemplate::New(isolate, Version));

    context = efl::eina::js::compatibility_new<v8::Context>(isolate, nullptr, global);
    eina_container_register(context->Global(), isolate);
  }
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
  }
  context->Enter();
  {
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(efl::eina::js::compatibility_new<v8::String>
                               (nullptr, "(shell)"));

    v8::Handle<v8::Value> a[] = {efl::eina::js::compatibility_new<v8::External>(isolate, &raw_list)};
    context->Global()->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "raw_list")
                , get_list_instance_template()->GetFunction()->NewInstance(1, a));     
    
    efl::eina::js::compatibility_handle_scope handle_scope(v8::Isolate::GetCurrent());
    ExecuteString(v8::Isolate::GetCurrent(),
                  efl::eina::js::compatibility_new<v8::String>(v8::Isolate::GetCurrent(), script),
                  name);
  }
  context->Exit();
}

//#ifdef USE_NODEJS
#include <node/node.h>

namespace {

void init(v8::Handle<v8::Object> exports)
{
  try
    {
      eina_container_register(exports, v8::Isolate::GetCurrent());
    }
  catch(...)
    {
      std::cout << "Error" << std::endl;
    }
}
  
}

NODE_MODULE(eina_js_suite, init)

//#endif
