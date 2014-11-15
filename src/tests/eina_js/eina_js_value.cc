#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <eina_js_value.hh>
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

    "var my_value = make_value(1);"
    "var wrapped = my_value.get();"
    "assert(typeof(wrapped) === 'number', '#1');"
    "assert(wrapped === 1, '#2');"

    "my_value.set(2);"
    "assert(wrapped === 1, '#3');"
    "wrapped = my_value.get();"
    "assert(typeof(wrapped) === 'number', '#4');"
    "assert(wrapped === 2, '#5');"

    "my_value.set(true);"
    "assert(wrapped === 2, '#6');"
    "wrapped = my_value.get();"
    // boolean is represented as integer in the efl::eina::value layer
    "assert(typeof(wrapped) === 'number', '#7');"
    "assert(wrapped === 1, '#8');"

    "var captured = false;"
    "try {"
    "  my_value.set({type: 'complex object'});"
    "} catch(e) {"
    "  assert(e.code === 'std::bad_cast', '#9');"
    "  captured = true;"
    "}"
    "assert(captured === true, '#10');"

    "captured = false;"
    "try {"
    "  my_value = make_value({type: 'complex object'});"
    "} catch(e) {"
    "  assert(e.code === 'std::bad_cast', '#11');"
    "  captured = true;"
    "}"
    "assert(captured === true, '#12');"

    "destroy_value(my_value);"
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
    efl::js::register_make_value(isolate, global,
                                 v8::String::NewFromUtf8(isolate,
                                                         "make_value"));
    efl::js::register_destroy_value(isolate, global,
                                    v8::String::NewFromUtf8(isolate,
                                                            "destroy_value"));

    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(std::numeric_limits<uint64_t>::max()),
            isolate)->IsNumber());
    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(std::numeric_limits<uint64_t>::max()),
            isolate)->NumberValue()
           == double(UINT64_MAX));

    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(std::numeric_limits<unsigned char>::max()),
            isolate)->IsUint32());
    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(std::numeric_limits<unsigned char>::max()),
            isolate)->Uint32Value() == UINT8_MAX);

    if (sizeof(short) > sizeof(int32_t)) {
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned short>::max()),
                isolate)->IsNumber());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned short>::max()),
                isolate)->NumberValue()
               == double(std::numeric_limits<unsigned short>::max()));

        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<short>::max()),
                isolate)->IsNumber());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<short>::max()),
                isolate)->NumberValue()
               == double(std::numeric_limits<short>::max()));
    } else {
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned short>::max()),
                isolate)->IsUint32());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned short>::max()),
                isolate)->Uint32Value()
               == std::numeric_limits<unsigned short>::max());

        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<short>::max()),
                isolate)->IsInt32());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<short>::max()),
                isolate)->Int32Value()
               == std::numeric_limits<short>::max());
    }

    if (sizeof(int) > sizeof(int32_t)) {
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned int>::max()),
                isolate)->IsNumber());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned int>::max()),
                isolate)->NumberValue()
               == double(std::numeric_limits<unsigned int>::max()));

        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<int>::max()),
                isolate)->IsNumber());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<int>::max()),
                isolate)->NumberValue()
               == double(std::numeric_limits<int>::max()));
    } else {
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned int>::max()),
                isolate)->IsUint32());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned int>::max()),
                isolate)->Uint32Value()
               == std::numeric_limits<unsigned int>::max());

        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<int>::max()),
                isolate)->IsInt32());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<int>::max()),
                isolate)->Int32Value()
               == std::numeric_limits<int>::max());
    }

    if (sizeof(long) > sizeof(int32_t)) {
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned long>::max()),
                isolate)->IsNumber());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned long>::max()),
                isolate)->NumberValue()
               == double(std::numeric_limits<unsigned long>::max()));

        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<long>::max()),
                isolate)->IsNumber());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<long>::max()),
                isolate)->NumberValue()
               == double(std::numeric_limits<long>::max()));
    } else {
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned long>::max()),
                isolate)->IsUint32());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<unsigned long>::max()),
                isolate)->Uint32Value()
               == std::numeric_limits<unsigned long>::max());

        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<long>::max()),
                isolate)->IsInt32());
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::numeric_limits<long>::max()),
                isolate)->Int32Value()
               == std::numeric_limits<long>::max());
    }

    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(std::numeric_limits<float>::max()),
            isolate)->IsNumber());
    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(std::numeric_limits<float>::max()),
            isolate)->NumberValue()
           == double(std::numeric_limits<float>::max()));

    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(42.42), isolate)->IsNumber());
    assert(efl::js::value_cast<v8::Local<v8::Value>>
           (efl::eina::value(42.42), isolate)->NumberValue()
           == 42.42);

    assert(efl::eina::get<int>
           (efl::js::value_cast<efl::eina::value>
            (v8::Boolean::New(isolate, true))) == 1);
    assert(efl::eina::get<int32_t>
           (efl::js::value_cast<efl::eina::value>
            (v8::Integer::New(isolate, INT32_MAX))) == INT32_MAX);
    assert(efl::eina::get<uint32_t>
           (efl::js::value_cast<efl::eina::value>
            (v8::Integer::NewFromUnsigned(isolate, UINT32_MAX)))
           == UINT32_MAX);
    assert(efl::eina::get<double>
           (efl::js::value_cast<efl::eina::value>
            (v8::Number::New(isolate,
                             std::numeric_limits<double>::max())))
           == std::numeric_limits<double>::max());

    {
        const char utf8_data[] = "Matroška";

        assert(efl::js::value_cast<efl::eina::value>
               (v8::String::NewFromUtf8(isolate, utf8_data))
               == efl::eina::value(std::string(utf8_data)));
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(std::string(utf8_data)), isolate)
               ->StrictEquals(v8::String::NewFromUtf8(isolate, utf8_data)));
#ifndef EINA_JS_TEST_SKIP_STRINGSHARE
        assert(efl::js::value_cast<v8::Local<v8::Value>>
               (efl::eina::value(efl::eina::stringshare(utf8_data)), isolate)
               ->StrictEquals(v8::String::NewFromUtf8(isolate, utf8_data)));
#endif // EINA_JS_TEST_SKIP_STRINGSHARE
    }

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
