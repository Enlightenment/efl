#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>

#include <Eo_Js.hh>
#include <Eina.hh>
#include <Eo.hh>
// #include <efl_js.hh>

using namespace std;
using namespace v8;

const char PATH_SEPARATOR =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

static std::string get_file_contents(const char *filename) {
    std::ifstream in(filename, std::ios::in);
    if (in) {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return contents.str();
    } else {
        throw(errno);
    }
}

static std::string get_filename(std::string path)
{
    int beginIdx = path.rfind(PATH_SEPARATOR);
    return path.substr(beginIdx + 1);
}

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << get_filename(name) << " <option(s)> [SOURCE]\n" << std::endl
              << "Options:" << std::endl
              << "\t-h, --help\t\t Show this help message" << std::endl;
}

/*
 * Basic console.log implementation with space-separated values,
 * no substitution
 */
void Log(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    for (int i=0; i < args.Length(); i++)
    {
        if (i != 0)
            std::cout << " ";
        String::Utf8Value string(args[i]);
        std::cout << *string;
    }

    std::cout << std::endl;

    args.GetReturnValue().Set(v8::Null(isolate));
}


int main(int argc, char* argv[])
{

    std::string script_source;
    char *filename = 0;

    for (int i=1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            show_usage(argv[0]);
            return 0;
        }
        else
        {
            filename = argv[i];
        }
    }

    if (!filename)
    {
        std::cerr << "Error: No source provided." << std::endl;
        show_usage(argv[0]);
        return 1;
    }

    try
    {
        script_source = get_file_contents(filename);
    } catch (int errno)
    {
        perror("Error: ");
        return 1;
    }


    efl::eina::js::compatibility_initialize();
    v8::V8::SetFlagsFromCommandLine(&argc, const_cast<char**>(argv), true);

    v8::Isolate* isolate = efl::eina::js::compatibility_isolate_new();
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handleScope(isolate);

        Local<Context> context = Context::New(isolate, NULL);
        Context::Scope context_scope(context);
        context->Enter();

        // Setup the console and log
        Local<Object> console = Object::New(isolate);
        Local<FunctionTemplate> log = FunctionTemplate::New(isolate, Log);
        console->Set(String::NewFromUtf8(isolate, "log"), log->GetFunction());

        Local<Object> global = context->Global();
        global->Set(String::NewFromUtf8(isolate, "console"), console);

        // Set up the efl exports; Needed to enter the context before this
        // due to creating Objects instead of Objects Templates
        // WIP: Commented out due to potential missing v8 platform implementation issues
        // Local<Object> efl_exports = Object::New(isolate);
        // global->Set(String::NewFromUtf8(isolate, "efl"), efl_exports);
        // efl_js::init(efl_exports);

        // And now the user's script
        Local<String> source = String::NewFromUtf8(isolate, script_source.c_str());

        Local<Script> script = Script::Compile(source);

        TryCatch tryCatch(isolate);
        Local<Value> result = script->Run();

        if (result.IsEmpty())
        {
            Local<Value> exception = tryCatch.Exception();
            String::Utf8Value exception_str(exception);
            printf("Exception: %s\n", *exception_str);
        }

    }

    V8::Dispose();
    return 0;
}
