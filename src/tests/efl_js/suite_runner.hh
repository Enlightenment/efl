
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>

#include <cassert>
#include <cstdlib>
#include <fstream>

#include <Eina_Js.hh>
#include <Efl_Js.hh>

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
void ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name)
{
  efl::eina::js::compatibility_handle_scope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::ScriptOrigin origin(name);
  v8::Handle<v8::Script> script = v8::Script::Compile(source, &origin);
  if (script.IsEmpty()) {
    std::cerr << "Compilation failed" << std::endl;
    std::exit(-1);
  }
  else
  {
    std::cerr << "Compilation succesful" << std::endl;
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      std::cout << "Failed with exception thrown" << std::endl;
      if(try_catch.HasCaught())
        {
          if(!try_catch.Message().IsEmpty() && !try_catch.Message()->Get().IsEmpty())
            std::cerr << "Exception " << ToCString(v8::String::Utf8Value(try_catch.Message()->Get()))
                      << std::endl;
          else
            std::cerr << "Exception without message" << std::endl;
        }
      std::exit(-1);
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

std::vector<char> read_script(const char* file)
{
  std::vector<char> script;
  std::ifstream script_file(file);
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

  return script;
}

int run_script(const char* file, void(*init)(v8::Handle<v8::Object>))
{
  efl::eina::js::compatibility_initialize();
  v8::Isolate* isolate = efl::eina::js::compatibility_isolate_new();
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
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> global = context->Global();

    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "print")
                 , efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, & ::Print)
                 ->GetFunction()); 

    std::cerr << __LINE__ << std::endl;
    v8::Handle<v8::Object> exports = efl::eina::js::compatibility_new<v8::Object>(isolate);
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "efl"), exports);

    efl_js::init(exports);

    if(init)
      {
        v8::Handle<v8::Object> exports = efl::eina::js::compatibility_new<v8::Object>(isolate);
        global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "suite"), exports);
        init(exports);
      }
    std::cerr << __LINE__ << std::endl;

    {
      efl::eina::js::compatibility_handle_scope handle_scope(v8::Isolate::GetCurrent());
      std::cerr << __LINE__ << std::endl;
      std::vector<char> script = read_script(file);
      v8::Local<v8::String> name(efl::eina::js::compatibility_new<v8::String>
                                 (nullptr, file));
      ExecuteString(v8::Isolate::GetCurrent(),
                    efl::eina::js::compatibility_new<v8::String>
                    (v8::Isolate::GetCurrent(), &script[0]), name);
      std::cerr << __LINE__ << std::endl;
    }
  }
  context->Exit();

  return 0;
}
