
#ifndef EFL_ECORE_JS_HH
#define EFL_ECORE_JS_HH

#include <Ecore.hh>
#include <Ecore_File.h>
#include <Eina_Js.hh>
#include <Eo_Js.hh>

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

namespace efl { namespace ecore { namespace js {

using ::efl::eina::js::compatibility_new;
using ::efl::eina::js::compatibility_return_type;
using ::efl::eina::js::compatibility_callback_info_type;
using ::efl::eina::js::compatibility_return;
using ::efl::eina::js::compatibility_get_pointer_internal_field;
using ::efl::eina::js::compatibility_set_pointer_internal_field;

EAPI void register_ecore_animator(v8::Isolate *isolate,v8::Handle<v8::Object> exports);
EAPI void register_ecore_event(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
EAPI void register_ecore_idle(v8::Isolate *isolate,v8::Handle<v8::Object> exports);
EAPI void register_ecore_job(v8::Isolate *isolate,v8::Handle<v8::Object> exports);
EAPI void register_ecore_mainloop(v8::Isolate *isolate,v8::Handle<v8::Object> exports);
EAPI void register_ecore_poller(v8::Isolate *isolate,v8::Handle<v8::Object> exports);
EAPI void register_ecore_throttle(v8::Isolate *isolate,v8::Handle<v8::Object> exports);
EAPI void register_ecore_timer(v8::Isolate *isolate,v8::Handle<v8::Object> exports);

EAPI void register_ecore(v8::Isolate *isolate,v8::Handle<v8::Object> exports);

} } }

#undef EAPI
#define EAPI

#endif
