
#ifndef EFL_ELDBUS_JS_HH
#define EFL_ELDBUS_JS_HH

#include <Eina.hh>
#include <Eina_Js.hh>

#include <Eldbus.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

namespace efl { namespace eldbus { namespace js {

EAPI void register_eldbus_connection(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
EAPI void register_eldbus_core(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
EAPI void register_eldbus_message(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
EAPI void register_eldbus_object_mapper(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
EAPI void register_eldbus(v8::Isolate* isolate, v8::Handle<v8::Object> exports);

} } }

#undef EAPI
#define EAPI

#include <eldbus_js_util.hh>

#endif
