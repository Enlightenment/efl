#ifndef ECTOR_SOFTWARE_H_
#define ECTOR_SOFTWARE_H_

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

#include "software/ector_software_surface.eo.h"
#include "software/ector_software_buffer.eo.h"
#include "software/ector_software_buffer_base.eo.h"
#include "software/ector_renderer_software.eo.h"
#include "software/ector_renderer_software_shape.eo.h"
#include "software/ector_renderer_software_gradient_linear.eo.h"
#include "software/ector_renderer_software_gradient_radial.eo.h"

#endif

#undef EAPI
#define EAPI

#endif
