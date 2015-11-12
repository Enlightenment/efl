#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"
#include "ector_generic_buffer.eo.h"
#include "ector_gl_buffer_base.eo.h"

typedef struct _Ector_GL_Buffer_Base_Data Ector_GL_Buffer_Base_Data;
struct _Ector_GL_Buffer_Base_Data
{
};

static Ector_Buffer_Flag
_ector_gl_buffer_base_ector_generic_buffer_flags_get(Eo *obj, Ector_GL_Buffer_Base_Data *pd)
{
}

static Eina_Bool
_ector_gl_buffer_base_ector_generic_buffer_pixels_set(Eo *obj, Ector_GL_Buffer_Base_Data *pd, void *pixels, int width, int height, int stride, Efl_Gfx_Colorspace cspace, Eina_Bool writable, unsigned char l, unsigned char r, unsigned char t, unsigned char b)
{
}

static uint8_t *
_ector_gl_buffer_base_ector_generic_buffer_span_get(Eo *obj, Ector_GL_Buffer_Base_Data *pd, int x, int y, unsigned int w, Efl_Gfx_Colorspace cspace, unsigned int *length)
{
}

static void
_ector_gl_buffer_base_ector_generic_buffer_span_free(Eo *obj, Ector_GL_Buffer_Base_Data *pd, uint8_t *data)
{
}

static uint8_t *
_ector_gl_buffer_base_ector_generic_buffer_map(Eo *obj, Ector_GL_Buffer_Base_Data *pd, int *offset, unsigned int *length, Ector_Buffer_Access_Flag mode, unsigned int x, unsigned int y, unsigned int w, unsigned int h, Efl_Gfx_Colorspace cspace, unsigned int *stride)
{
}

static void
_ector_gl_buffer_base_ector_generic_buffer_unmap(Eo *obj, Ector_GL_Buffer_Base_Data *pd, void *data, int offset, unsigned int length)
{
}

static Eo_Base *
_ector_gl_buffer_eo_base_constructor(Eo *obj, void *pd)
{
}

static void
_ector_gl_buffer_eo_base_destructor(Eo *obj, void *pd)
{
}

#include "ector_gl_buffer.eo.c"
#include "ector_gl_buffer_base.eo.c"
