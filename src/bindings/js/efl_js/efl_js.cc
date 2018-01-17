
#ifdef HAVE_CONFIG_H
#include <config.h>
#include <elementary_config.h>
#endif

#include <Efl.h>
#include <Efl_Config.h>
#include <Ecore.h>
#include <Eo.h>
#include <Ecore_Con.h>
#include <Ecore_Audio.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore_Con_Eet.h>
#include <Emotion.h>
#define ELM_INTERNAL_API_ARGESFSDFEFC
#include <Elementary.h>
extern "C" {
#include <elm_widget.h>
}

#include <Eina_Js.hh>
#include <Ecore_Js.hh>
#include <Eio_Js.hh>
#include <Eldbus_Js.hh>
#include <Ethumb_Js.hh>

#include <eo_js_list.hh>

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

// include registrations
#include <eo_js_namespace_tree.hh>

struct object_registration_win
{
  void operator()(v8::Handle<v8::Object>, v8::Isolate*) const
  {
    std::cout << "called " << __func__ << std::endl;
  }
};


struct object_registration_abc
{
  void operator()(v8::Handle<v8::Object>, v8::Isolate*) const
  {
    std::cout << "called " << __func__ << std::endl;
  }
};

#include <eolian_include_generated.js.hh>

namespace {

namespace _mpl {

#include <eolian_js_bindings.js.hh>

typedef efl::eo::js::namespace_object<efl::eo::js::name<>
                                      , _classes, _namespaces
                                      > global_namespace;

}

template <typename T>
struct is_registered
{
  static bool yes;
  
};

template <typename T> bool is_registered<T>::yes = false;

template <typename T>
void namespace_accessor_get(v8::Local<v8::Name> name, v8::PropertyCallbackInfo<v8::Value> const& info);
  
struct found_item
{
  efl::eina::string_view name;
  v8::PropertyCallbackInfo<v8::Value> const& info;
  
  template <typename Name, typename Classes, typename InnerNamespaces>
  void operator()(efl::eo::js::namespace_object<Name, Classes, InnerNamespaces> const) const
  {
    typedef efl::eo::js::namespace_object<Name, Classes, InnerNamespaces> type;
    std::cout << "returned namespace item " << typeid(typename type::name).name() << std::endl;
    if(is_registered<type>::yes)
    {
      std::cout << "Already registered" << std::endl;
    }
    else
    {
      std::cout << "Not registered yet" << std::endl;
      is_registered<type>::yes = true;

      v8::Isolate* isolate = info.GetIsolate();
      v8::Local<v8::ObjectTemplate> ns_object = ::efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate);
      ns_object->SetHandler({&namespace_accessor_get<type>});
      auto obj = ns_object->NewInstance();
      info.This()->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, name.data())
                       , obj);

      info.GetReturnValue().Set(obj);
    }
  }

  template <typename Name, typename Registration>
  void operator()(efl::eo::js::class_object<Name, Registration> const) const
  {
    typedef efl::eo::js::class_object<Name, Registration> type;
    std::cout << "returned class item " << typeid(typename type::name).name() << std::endl;
    if(is_registered<type>::yes)
    {
      std::cout << "Already registered" << std::endl;
    }
    else
    {
      std::cout << "Not registered yet" << std::endl;
      is_registered<type>::yes = true;

      auto registered = Registration{}(info.This(), info.GetIsolate());

      v8::Isolate* isolate = info.GetIsolate();
      info.This()->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, name.data())
                       , registered);
      info.GetReturnValue().Set(registered);
    }
  }

  void operator()(efl::eo::js::empty const) const
  {
    std::cout << "not found" << std::endl;
    // must register
  }
};

template <typename T>
void namespace_accessor_get(v8::Local<v8::Name> name, v8::PropertyCallbackInfo<v8::Value> const& info)
{
  std::cout << "was it here?" << std::endl;
  v8::Local<v8::String> name_str = name->ToString();
  assert(!!*name_str);
  v8::String::Utf8Value value(info.GetIsolate(), name_str);
  std::cout << "searching for " << *value << std::endl;

  // if(info.Data()->IsNullOrUndefined())
  //   {
  //     std::cout << "no value already assigned" << std::endl;
  //   }
  // else
  //   std::cout << "value already assigned" << std::endl;

  efl::eina::string_view string = *value;
  
  efl::eo::js::search_separate(string, T{}
                              , found_item{string, info});
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
            ns_obj->SetHandler({&namespace_accessor_get<_mpl::global_namespace>});
            auto obj = ns_obj->NewInstance();
            obj->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, "evas"), evas);
            module->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, "exports"), obj);
          }

          // _mpl::foo();
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
