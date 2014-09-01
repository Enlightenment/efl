
#ifndef EFL_EINA_JS_HH
#define EFL_EINA_JS_HH

#include <Eina.hh>
#include <eo_concrete.hh>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EINA_JS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
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

#include <eina_js_container.hh>
#include <eina_js_accessor.hh>
#include <eina_js_array.hh>
#include <eina_js_compatibility.hh>
#include <eina_js_error.hh>
#include <eina_js_get_value_from_c.hh>
#include <eina_js_get_value.hh>
#include <eina_js_iterator.hh>
#include <eina_js_list.hh>
#include <eina_js_log.hh>
#include <eina_js_value.hh>
#include <eina_js_log.hh>

#undef EAPI
#define EAPI

#endif
