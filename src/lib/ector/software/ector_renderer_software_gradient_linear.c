#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

static Eina_Bool
_ector_renderer_software_gradient_linear_ector_renderer_prepare(Eo *obj,
                                                                Ector_Renderer_Software_Gradient_Data *pd)
{
   if (!pd->surface)
     {
        Ector_Renderer_Data *base;

        base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
        pd->surface = efl_data_xref(base->surface, ECTOR_SOFTWARE_SURFACE_CLASS, obj);
     }

   ector_software_gradient_color_update(pd);

   pd->linear.x1 = pd->gld->start.x;
   pd->linear.y1 = pd->gld->start.y;

   pd->linear.x2 = pd->gld->end.x;
   pd->linear.y2 = pd->gld->end.y;

   pd->linear.dx = pd->linear.x2 - pd->linear.x1;
   pd->linear.dy = pd->linear.y2 - pd->linear.y1;
   pd->linear.l = pd->linear.dx * pd->linear.dx + pd->linear.dy * pd->linear.dy;
   pd->linear.off = 0;

   if (!EINA_DBL_EQ(pd->linear.l, 0.0))
     {
        pd->linear.dx /= pd->linear.l;
        pd->linear.dy /= pd->linear.l;
        pd->linear.off = -pd->linear.dx * pd->linear.x1 - pd->linear.dy * pd->linear.y1;
     }

   return EINA_FALSE;
}

static Eina_Bool
_ector_renderer_software_gradient_linear_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Software_Gradient_Data *pd EINA_UNUSED,
                                                             Efl_Gfx_Render_Op op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                                             unsigned int mul_col EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_software_gradient_linear_ector_renderer_software_op_fill(Eo *obj EINA_UNUSED,
                                                                      Ector_Renderer_Software_Gradient_Data *pd)
{
   ector_software_rasterizer_linear_gradient_set(pd->surface->rasterizer, pd);
   ector_software_gradient_color_update(pd);
   return EINA_TRUE;
}

static Eo *
_ector_renderer_software_gradient_linear_efl_object_constructor(Eo *obj,
                                                             Ector_Renderer_Software_Gradient_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS));
   if (!obj) return NULL;

   pd->gd  = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_MIXIN, obj);
   pd->gld = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN, obj);
   pd->done = EINA_TRUE;

   return obj;
}

static void
_ector_renderer_software_gradient_linear_efl_object_destructor(Eo *obj,
                                                            Ector_Renderer_Software_Gradient_Data *pd)
{
   Ector_Renderer_Data *base;

   destroy_color_table(pd);

   base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
   efl_data_xunref(base->surface, pd->surface, obj);

   efl_data_xunref(obj, pd->gd, obj);
   efl_data_xunref(obj, pd->gld, obj);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS));
}

void
_ector_renderer_software_gradient_linear_efl_gfx_gradient_stop_set(Eo *obj, Ector_Renderer_Software_Gradient_Data *pd, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   efl_gfx_gradient_stop_set(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS), colors, length);

   destroy_color_table(pd);
}

static unsigned int
_ector_renderer_software_gradient_linear_ector_renderer_crc_get(const Eo *obj, Ector_Renderer_Software_Gradient_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS));

   crc = eina_crc((void*) pd->gd->s, sizeof (Efl_Gfx_Gradient_Spread), crc, EINA_FALSE);
   if (pd->gd->colors_count)
     crc = eina_crc((void*) pd->gd->colors, sizeof (Efl_Gfx_Gradient_Stop) * pd->gd->colors_count, crc, EINA_FALSE);
   crc = eina_crc((void*) pd->gld, sizeof (Ector_Renderer_Gradient_Linear_Data), crc, EINA_FALSE);

   return crc;
}

#include "ector_renderer_software_gradient_linear.eo.c"
