

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eo_Js.hh>
#include <Eina.hh>
#include <Eo.hh>

#include <check.h>

#include <iostream>

#include <cassert>
#include <tuple>

#include <constructor_method_class.eo.js.cc>

namespace {

static const char script[] =
  "function assert(condition, message) {\n"
  "  if (!condition) {\n"
  "      print(\"Assertion failed \", message);\n"
  "      throw message || \"Assertion failed\";\n"
  "  }\n"
  "}\n"
  "print(\"teste\");\n"
  "x = new Constructor_Method_Class(5, 10.0);"
  "x.method1(2);\n"
  "r = x.method2(3);\n"
  "x.method3(3, 11.1);\n"
  "r = x.rmethod1(3, 11.1);\n"
  "assert(r == 5);\n"
  "try\n"
  "{\n"
  "  x.method3(3);\n"
  "  assert(false, \"No exception thrown! this is wrong\");\n"
  "}\n"
  "catch (err)\n"
  "{\n"
  "  print(\"Correctly exception thrown\");\n"
  "}\n"
  "try\n"
  "{\n"
  "  y = new Constructor_Method_Class(5);\n"
  "  assert(false, \"No exception thrown! this is wrong\");\n"
  "}\n"
  "catch (err)\n"
  "{\n"
  "  print(\"Correctly exception thrown\");\n"
  "}\n"
  "try\n"
  "{\n"
  "  y = new Constructor_Method_Class(5.1, 10);\n"
  "  assert(false, \"No exception thrown! this is wrong\");\n"
  "}\n"
  "catch (err)\n"
  "{\n"
  "  print(\"Correctly exception thrown\");\n"
  "}\n"
  "z = Constructor_Method_Class(5, 10.0);\n"
  // Out parameters
  "print(\"Going to run outmethod1\");\n"
  "r = x.outmethod1();\n"
  "print(\"Ran outmethod1\");\n"
  "assert(r == 5);\n"
  "print(\"Going to run outmethod2\");\n"
  "r = x.outmethod2();\n"
  "print(\"ran outmethod2\");\n"
  "assert(Array.isArray(r));\n"
  "print(\"Return is array from outmethod2\");\n"
  "assert(r[0] == 11);\n"
  "assert(r[1] == 10);\n"
  "print(\"Going to run outmethod3\");\n"
  "r = x.outmethod3();\n"
  "print(\"ran outmethod3\");\n"
  "assert(Array.isArray(r));\n"
  "print(\"Return is array from outmethod3\");\n"
  "assert(r[0] == 11);\n"
  "assert(r[1] == 5.1);\n"
  ;

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name)
{
  v8::HandleScope handle_scope(isolate);
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

void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
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
}

START_TEST(eolian_js_test_constructor_method_test)
{
  int argc = 1;
  const char* argv[] = {"test"};
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;
  
  v8::V8::InitializeICU();
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, const_cast<char**>(argv), true);

  v8::Isolate* isolate = v8::Isolate::New();

  v8::Isolate::Scope isolate_scope(isolate);
  
  assert(isolate != 0);

  v8::Handle<v8::Context> context;
  v8::HandleScope handle_scope(isolate);

  {
    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    // Bind the global 'print' function to the C++ Print callback.
    global->Set(v8::String::NewFromUtf8(isolate, "print"),
                v8::FunctionTemplate::New(isolate, Print));

    context = v8::Context::New(isolate, NULL, global);

  }
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
  }
  context->Enter();
  {
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(v8::String::NewFromUtf8(context->GetIsolate(), "(shell)"));

    register_constructor_method_class(context->Global(), isolate);

    v8::HandleScope handle_scope(context->GetIsolate());
    ExecuteString(context->GetIsolate(),
                  v8::String::NewFromUtf8(context->GetIsolate(), script),
                  name);
  }
  context->Exit();
}
END_TEST

}

void eolian_js_test_constructor_method(TCase* tc)
{
   tcase_add_test(tc, eolian_js_test_constructor_method_test);
}
