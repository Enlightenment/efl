#ifndef EVAS_ECTOR_GL_H
#define EVAS_ECTOR_GL_H

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

#include "evas_ector_buffer.eo.h"
#include "evas_ector_software_buffer.eo.h"

#undef EAPI
#define EAPI

#endif /* ! EVAS_ECTOR_GL_H */

