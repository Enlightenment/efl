#ifndef EVAS_ECTOR_GL_H
#define EVAS_ECTOR_GL_H

#ifdef _WIN32
# define EMODAPI __declspec(dllexport)
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EMODAPI __attribute__ ((visibility("default")))
#  else
#   define EMODAPI
#  endif
# else
#  define EMODAPI
# endif
#endif

#include "evas_ector_buffer.eo.h"
#include "evas_ector_gl_buffer.eo.h"
#include "evas_ector_gl_image_buffer.eo.h"

#endif /* ! EVAS_ECTOR_GL_H */

