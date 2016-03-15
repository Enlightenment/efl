#ifndef ETHUMB_JS_INIT_HH
#define ETHUMB_JS_INIT_HH

#include <Eina.hh>

#include <Eina_Js.hh>
#include <Ethumb_Client.h>
#include <Eo_Js.hh>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ETHUMB_JS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ETHUMB_JS_BUILD */
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

namespace efl { namespace ethumb { namespace js {

using ::efl::eina::js::compatibility_new;
using ::efl::eina::js::compatibility_return_type;
using ::efl::eina::js::compatibility_callback_info_type;
using ::efl::eina::js::compatibility_return;
using ::efl::eina::js::compatibility_get_pointer_internal_field;
using ::efl::eina::js::compatibility_set_pointer_internal_field;

EAPI void register_ethumb(v8::Isolate *isolate, v8::Handle<v8::Object> exports);
      
} } } // namespace efl { namespace ethumb { namespace js {

#endif /* ETHUMB_JS_INIT_HH */
