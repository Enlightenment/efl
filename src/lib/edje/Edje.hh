#ifndef EFL_CXX_EDJE_HH
#define EFL_CXX_EDJE_HH

#ifdef EFL_BETA_API_SUPPORT

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
# define EAPI_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define EAPI_WEAK __attribute__ ((weak))
#  else
#   define EAPI
#   define EAPI_WEAK
#  endif
# else
#  define EAPI
#  define EAPI_WEAK
# endif
#endif

#include <Edje.eo.hh>

#endif
#endif
