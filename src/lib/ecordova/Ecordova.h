#ifndef _ECORDOVA_H
#define _ECORDOVA_H

#include <Ecore.h>
#include <Efl.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORDOVA_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORDOVA_BUILD */
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

#ifdef __cplusplus
extern "C" {
#endif

#include "Ecordova_Common.h"
#ifdef EFL_EO_API_SUPPORT
#include "Ecordova_Eo.h"
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
