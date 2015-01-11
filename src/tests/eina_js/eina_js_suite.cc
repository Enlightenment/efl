
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include V8_INCLUDE_HEADER

#include <cassert>
#include <cstdlib>
#include <fstream>

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
    std::cerr << "Compilation failed" << std::endl;
    std::abort();
    // Print errors that happened during compilation.
    // if (report_exceptions)
    //   ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    std::cerr << "Compilation succesful" << std::endl;
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      std::cout << "Failed with exception thrown" << std::endl;
      //assert(try_catch.HasCaught());
      //std::abort();
      // Print errors that happened during execution.
      // if (report_exceptions)
      //   ReportException(isolate, &try_catch);
      if(try_catch.HasCaught())
        std::cerr << "Exception " << ToCString(v8::String::Utf8Value(try_catch.Message()->Get()))
                  << std::endl;
      std::abort();
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

efl::eina::ptr_list<int> list;
efl::eina::js::range_eina_list<int> raw_list;

void test_setup(v8::Handle<v8::Object> exports)
{
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
  std::cerr << __LINE__ << std::endl;

}

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

  {
    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate);
    context = efl::eina::js::compatibility_new<v8::Context>(isolate, nullptr, global);
  }
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
  }
  context->Enter();
  {
    std::vector<char> script;
    {
      std::ifstream script_file(TESTS_SRC_DIR "/eina_js_suite.js");
      script_file.seekg(0, std::ios::end);
      std::size_t script_size = script_file.tellg();
      script_file.seekg(0, std::ios::beg);
      script.resize(script_size+1);
      script_file.rdbuf()->sgetn(&script[0], script_size);
      auto line_break = std::find(script.begin(), script.end(), '\n');
      assert(line_break != script.end());
      ++line_break;
      std::fill(script.begin(), line_break, ' ');

      std::cerr << "program:" << std::endl;
      std::copy(script.begin(), script.end(), std::ostream_iterator<char>(std::cerr));
      std::cerr << "end of program" << std::endl;
    }

    
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(efl::eina::js::compatibility_new<v8::String>
                               (nullptr, "(shell)"));

    std::cerr << __LINE__ << std::endl;
    v8::Handle<v8::Object> exports = efl::eina::js::compatibility_new<v8::Object>(isolate);
    context->Global()->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "suite"), exports);
    test_setup(exports);
    std::cerr << __LINE__ << std::endl;

    v8::Handle<v8::Object> console = efl::eina::js::compatibility_new<v8::Object>(isolate);
    context->Global()->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "console"), console);
    console->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "log")
                 , efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, & ::Print)
                 ->GetFunction());

    
    efl::eina::js::compatibility_handle_scope handle_scope(v8::Isolate::GetCurrent());
    std::cerr << __LINE__ << std::endl;
    ExecuteString(v8::Isolate::GetCurrent(),
                  efl::eina::js::compatibility_new<v8::String>(v8::Isolate::GetCurrent(), &script[0]),
                  name);
  std::cerr << __LINE__ << std::endl;
  }
  context->Exit();
}

#else
#include <node/node.h>

namespace {

void init(v8::Handle<v8::Object> exports)
{
  try
    {
      eina_init();
      eo_init();

      test_setup(exports);
      
      std::cerr << "registered" << std::endl;
    }
  catch(...)
    {
      std::cerr << "Error" << std::endl;
      std::abort();
    }
}
  
}

NODE_MODULE(eina_js_suite, init)

#endif
