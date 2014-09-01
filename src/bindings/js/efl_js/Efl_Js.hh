#ifndef EFL_EFL_JS_HH
#define EFL_EFL_JS_HH

#include <Eina.hh>

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

namespace efl_js {

EAPI void init(v8::Handle<v8::Object> exports);

}

#endif
