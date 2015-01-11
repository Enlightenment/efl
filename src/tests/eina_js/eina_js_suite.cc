
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

#ifndef HAVE_NODEJS

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

#else
#include <node/node.h>

namespace {

efl::eina::ptr_list<int> list;
efl::eina::js::range_eina_list<int> raw_list;

void init(v8::Handle<v8::Object> exports)
{
  try
    {
      eina_init();
      eo_init();
      
      std::cerr << __LINE__ << std::endl;
      list.push_back(new int(5));
      list.push_back(new int(10));
      list.push_back(new int(15));
      std::cerr << __LINE__ << std::endl;

      raw_list = efl::eina::js::range_eina_list<int>(list.native_handle());

      std::cerr << __LINE__ << std::endl;
      eina_container_register(exports, v8::Isolate::GetCurrent());
      std::cerr << __LINE__ << std::endl;

      v8::Handle<v8::Value> a[] = {efl::eina::js::compatibility_new<v8::External>(nullptr, &raw_list)};
      std::cerr << __LINE__ << std::endl;
      exports->Set(efl::eina::js::compatibility_new<v8::String>(nullptr, "raw_list")
                   , get_list_instance_template()->GetFunction()->NewInstance(1, a));

      std::cerr << "registered" << std::endl;
    }
  catch(...)
    {
      std::cerr << "Error" << std::endl;
    }
}
  
}

NODE_MODULE(eina_js_suite, init)

#endif
