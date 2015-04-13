
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>
#include EINA_STRINGIZE(V8_INCLUDE_HEADER)

#include <cassert>
#include <cstdlib>
#include <fstream>

#include <Eina.h>
#include <Eo.hh>

#include <eina_js_accessor.hh>
#include <eina_js_list.hh>
#include <eina_js_iterator.hh>
#include <eina_js_error.hh>

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

efl::eina::js::compatibility_return_type clear_eina_error(efl::eina::js::compatibility_callback_info_type args)
{
    eina_error_set(0);
    return efl::eina::js::convert_error_to_javascript_exception(args.GetIsolate());
}

efl::eina::js::compatibility_return_type set_eina_error(efl::eina::js::compatibility_callback_info_type args)
{
    eina_error_set(eina_error_msg_static_register("foobar"));
    return efl::eina::js::convert_error_to_javascript_exception(args.GetIsolate());
}

EAPI void eina_container_register(v8::Handle<v8::Object> global, v8::Isolate* isolate);
EAPI v8::Local<v8::Function> get_list_instance_template();

efl::eina::ptr_list<int> list;
efl::eina::js::range_eina_list<int> raw_list;

efl::eina::array<int> array;

void test_setup(v8::Handle<v8::Object> exports)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();

  // container globals
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
               , get_list_instance_template()->NewInstance(1, a));
  std::cerr << __LINE__ << std::endl;

  // error globals
  exports->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "clear_eina_error"),
              efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, clear_eina_error)
              ->GetFunction());
  exports->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "set_eina_error"),
              efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, set_eina_error)
              ->GetFunction());
  std::cerr << __LINE__ << std::endl;

  // accessor
  Eina_Array* array = eina_array_new(3);
  eina_array_push(array, new int(42));
  eina_array_push(array, new int(24));
  eina_array_push(array, new int(0));

  static efl::eina::accessor<int> acc(eina_array_accessor_new(array));
  std::cerr << __LINE__ << std::endl;
  
  v8::Local<v8::Object> wrapped_acc = efl::eina::js::export_accessor( acc, isolate);
  exports->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "acc"), wrapped_acc);

  static efl::eina::iterator<int> it(eina_array_iterator_new(array));

  v8::Local<v8::Object> wrapped_it = efl::eina::js::export_iterator(&it, isolate);

  exports->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "it"), wrapped_it);
  
  std::cerr << __LINE__ << std::endl;

  efl::eina::js::register_value
    (isolate, exports
     , efl::eina::js::compatibility_new<v8::String>(isolate, "value"));
  
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
  v8::Handle<v8::Context> context
    = efl::eina::js::compatibility_new<v8::Context>
    (isolate, nullptr
     , efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate));
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
    v8::Local<v8::Object> global = context->Global();
    v8::Handle<v8::Object> console = efl::eina::js::compatibility_new<v8::Object>(isolate);
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "console"), console);
    console->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "log")
                 , efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, & ::Print)
                 ->GetFunction()); 

    std::cerr << __LINE__ << std::endl;
    v8::Handle<v8::Object> exports = efl::eina::js::compatibility_new<v8::Object>(isolate);
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "suite"), exports);

    test_setup(exports);
    std::cerr << __LINE__ << std::endl;

    
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
#include EINA_STRINGIZE(NODE_INCLUDE_HEADER)

namespace {

void eina_js_module_init(v8::Handle<v8::Object> exports)
{
  fprintf(stderr, "teste\n"); fflush(stderr);
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

NODE_MODULE(eina_js_suite_mod, ::eina_js_module_init)

#endif
