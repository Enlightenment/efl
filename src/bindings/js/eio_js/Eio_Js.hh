#ifndef EIO_JS_INIT_HH
#define EIO_JS_INIT_HH

#include <Eio.h>
#include <Ecore.h>

#include <Eina.hh>
#include <Eo_Js.hh>
#include <Eina_Js.hh>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EIO_JS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EIO_JS_BUILD */
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

namespace efl { namespace eio { namespace js {

EAPI
void register_eio(v8::Isolate* isolate, v8::Handle<v8::Object> exports);      

} } } // namespace efl { namespace eio { namespace js {

#endif /* EIO_JS_INIT_HH */
