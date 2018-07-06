#ifndef ECTOR_CAIRO_H_
# define ECTOR_CAIRO_H_

#include <Ector.h>

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

#ifdef EFL_BETA_API_SUPPORT

#ifndef _ECTOR_CAIRO_SURFACE_EO_CLASS_TYPE
#define _ECTOR_CAIRO_SURFACE_EO_CLASS_TYPE

typedef Eo Ector_Cairo_Surface;

#endif

typedef struct _cairo_t cairo_t;

#include "cairo/ector_cairo_surface.eo.h"
#include "cairo/ector_cairo_software_surface.eo.h"
#include "cairo/ector_renderer_cairo.eo.h"
#include "cairo/ector_renderer_cairo_shape.eo.h"
#include "cairo/ector_renderer_cairo_gradient_linear.eo.h"
#include "cairo/ector_renderer_cairo_gradient_radial.eo.h"

#endif

#undef EAPI
#define EAPI

#endif
