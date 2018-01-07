
#ifdef HAVE_CONFIG_H
#include <config.h>
#include <elementary_config.h>
#endif

#include <Efl.h>
#include <Efl_Config.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Eina.hh>
#include <Eina_Js.hh>
#include <Ecore_Js.hh>
#include <Eio_Js.hh>
#include <Eldbus_Js.hh>
#include <Ethumb_Js.hh>
#include <Elementary.h>

#include <iostream>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
#  define EAPI __declspec(dllexport)
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

namespace {

void namespace_accessor_get(v8::Local<v8::Name> name, v8::PropertyCallbackInfo<v8::Value> const& info)
{
  std::cout << "was it here?" << std::endl;
  v8::Local<v8::String> name_str = name->ToString();
  assert(!!*name_str);
  v8::String::Utf8Value value(info.GetIsolate(), name_str);
  std::cout << *value << std::endl;

  if(info.Data()->IsNullOrUndefined())
    {
      std::cout << "no value already assigned" << std::endl;
    }
  else
    std::cout << "value already assigned" << std::endl;
}
  
}

#ifdef HAVE_NODEJS

#include <eina_js_node.hh>

namespace {
#else
namespace efl_js {
#endif

EAPI void init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module)
{
  static char* argv[] = {const_cast<char*>("node")};
  ::elm_init(1, argv);

  ::elm_need_ethumb();

  elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);
  
   try
     {
        // eina_container_register(exports, v8::Isolate::GetCurrent());
        // eina_log_register(exports, v8::Isolate::GetCurrent());
        // eina_value_register(exports, v8::Isolate::GetCurrent());
        // //register_ecore_mainloop(exports, v8::Isolate::GetCurrent());

        v8::Isolate* isolate = exports->GetIsolate();
       
        v8::Local<v8::ObjectTemplate> ns_obj;
          {
            auto evas = ::efl::eina::js::compatibility_new<v8::Object>(isolate);
            ns_obj = ::efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate);
            ns_obj->SetHandler({&namespace_accessor_get});
            auto obj = ns_obj->NewInstance();
            obj->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, "evas"), evas);
            module->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, "exports"), obj);
          }
     }
   catch(...)
     {
        std::cout << "Exception" << std::endl;
     }
}

#ifdef HAVE_NODEJS
}
NODE_MODULE(efl, init)
#else
} // namespace efl
#endif
