#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "gl/Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"

typedef struct _Ector_Renderer_GL_Gradient_Radial_Data Ector_Renderer_GL_Gradient_Radial_Data;
struct _Ector_Renderer_GL_Gradient_Radial_Data
{
   Ector_Renderer_Gradient_Radial_Data *radial;
   Ector_Renderer_Gradient_Data *gradient;
   Ector_Renderer_Data *base;
};

static Eina_Bool
_ector_renderer_gl_gradient_radial_ector_renderer_prepare(Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd)
{
   // FIXME: prepare something
   (void) obj;
   (void) pd;

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_gradient_radial_ector_renderer_draw(Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd, Efl_Gfx_Render_Op op, Eina_Array *clips, unsigned int mul_col)
{
   ector_renderer_draw(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS), op, clips, mul_col);

   // FIXME: draw something !
   (void) pd;

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_gradient_radial_ector_renderer_gl_op_fill(Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd, uint64_t flags, GLshort *vertex, unsigned int vertex_count, unsigned int mul_col)
{
   // FIXME: The idea here is to select the right shader and push the needed parameter for it
   // along with the other value
   (void) obj;
   (void) pd;
   (void) flags;
   (void) vertex;
   (void) vertex_count;
   (void) mul_col;

   return EINA_TRUE;
}

static void
_ector_renderer_gl_gradient_radial_efl_gfx_path_bounds_get(const Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_GL_Gradient_Radial_Data *pd,
                                                             Eina_Rect *r)
{
   EINA_RECTANGLE_SET(r,
                      pd->base->origin.x + pd->radial->radial.x - pd->radial->radius,
                      pd->base->origin.y + pd->radial->radial.y - pd->radial->radius,
                      pd->radial->radius * 2, pd->radial->radius * 2 );
}

static unsigned int
_ector_renderer_gl_gradient_radial_ector_renderer_crc_get(const Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS));

   crc = eina_crc((void*) pd->gradient->s, sizeof (Efl_Gfx_Gradient_Spread), crc, EINA_FALSE);
   if (pd->gradient->colors_count)
     crc = eina_crc((void*) pd->gradient->colors, sizeof (Efl_Gfx_Gradient_Stop) * pd->gradient->colors_count, crc, EINA_FALSE);
   crc = eina_crc((void*) pd->radial, sizeof (Ector_Renderer_Gradient_Radial_Data), crc, EINA_FALSE);

   return crc;
}

static Efl_Object *
_ector_renderer_gl_gradient_radial_efl_object_constructor(Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS));

   if (!obj) return NULL;

   pd->base = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);
   pd->radial = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN, obj);
   pd->gradient = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_MIXIN, obj);

   return obj;
}

static void
_ector_renderer_gl_gradient_radial_efl_object_destructor(Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd)
{
   efl_data_xunref(obj, pd->base, obj);
   efl_data_xunref(obj, pd->radial, obj);
   efl_data_xunref(obj, pd->gradient, obj);
}

static void
_ector_renderer_gl_gradient_radial_efl_gfx_gradient_stop_set(Eo *obj, Ector_Renderer_GL_Gradient_Radial_Data *pd EINA_UNUSED, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   efl_gfx_gradient_stop_set(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS), colors, length);
}

#include "ector_renderer_gl_gradient_radial.eo.c"
