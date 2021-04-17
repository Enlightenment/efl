#ifndef EVAS_ECTOR_SOFTWARE_H
#define EVAS_ECTOR_SOFTWARE_H

#ifdef _WIN32
# ifndef EFL_MODULE_STATIC
#  define EMODAPI __declspec(dllexport)
# else
#  define EMODAPI
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EMODAPI __attribute__ ((visibility("default")))
#   define EMODAPI_WEAK __attribute__ ((weak))
#  else
#   define EMODAPI
#   define EMODAPI_WEAK
#  endif
# else
#  define EMODAPI
#  define EMODAPI_WEAK
# endif
#endif

#include "evas_ector_buffer.eo.h"
#include "evas_ector_software_buffer.eo.h"

#endif /* ! EVAS_ECTOR_SOFTWARE_H */

