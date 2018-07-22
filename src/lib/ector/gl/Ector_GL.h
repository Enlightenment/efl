#ifndef ECTOR_GL_H_
# define ECTOR_GL_H_

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

#ifndef _ECTOR_GL_SURFACE_EO_CLASS_TYPE
#define _ECTOR_GL_SURFACE_EO_CLASS_TYPE

typedef Eo Ector_Cairo_Surface;

#endif

typedef unsigned int GLuint;
typedef short        GLshort;

#include "gl/ector_gl_buffer.eo.h"
#include "gl/ector_gl_surface.eo.h"
#include "gl/ector_renderer_gl.eo.h"
#include "gl/ector_renderer_gl_shape.eo.h"
#include "gl/ector_renderer_gl_gradient_linear.eo.h"
#include "gl/ector_renderer_gl_gradient_radial.eo.h"

#endif

#undef EAPI
#define EAPI

#endif
