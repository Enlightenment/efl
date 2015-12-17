#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include "Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"
#include "ector_buffer.h"

#define MY_CLASS ECTOR_GL_BUFFER_CLASS

static Eo_Base *
_ector_gl_buffer_eo_base_constructor(Eo *obj, void *pd)
{
}

static void
_ector_gl_buffer_eo_base_destructor(Eo *obj, void *pd)
{
}

#include "ector_gl_buffer.eo.c"
