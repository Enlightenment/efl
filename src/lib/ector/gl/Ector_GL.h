#ifndef ECTOR_GL_H_
# define ECTOR_GL_H_

#include <Ector.h>

#include "ector_api.h"

#ifdef EFL_BETA_API_SUPPORT

#ifndef _ECTOR_GL_SURFACE_EO_CLASS_TYPE
#define _ECTOR_GL_SURFACE_EO_CLASS_TYPE

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

#endif
