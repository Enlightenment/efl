#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

static Eina_Bool
_ector_renderer_software_gradient_radial_ector_renderer_prepare(Eo *obj, Ector_Renderer_Software_Gradient_Data *pd)
{
   if (!pd->surface)
     {
        Ector_Renderer_Data *base;

        base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
        pd->surface = efl_data_xref(base->surface, ECTOR_SOFTWARE_SURFACE_CLASS, obj);
     }

   ector_software_gradient_color_update(pd);

   pd->radial.cx = pd->grd->radial.x;
   pd->radial.cy = pd->grd->radial.y;
   pd->radial.cradius = pd->grd->radius;

   if (EINA_DBL_EQ(pd->grd->focal.x, 0.0))
     pd->radial.fx = pd->grd->radial.x;
   else
     pd->radial.fx = pd->grd->focal.x;

   if (EINA_DBL_EQ(pd->grd->focal.y, 0.0))
     pd->radial.fy = pd->grd->radial.y;
   else
     pd->radial.fy = pd->grd->focal.y;

   pd->radial.fradius = 0;

   pd->radial.dx = pd->radial.cx - pd->radial.fx;
   pd->radial.dy = pd->radial.cy - pd->radial.fy;

   pd->radial.dr = pd->radial.cradius - pd->radial.fradius;
   pd->radial.sqrfr = pd->radial.fradius * pd->radial.fradius;

   pd->radial.a = pd->radial.dr * pd->radial.dr -
     pd->radial.dx * pd->radial.dx -
     pd->radial.dy * pd->radial.dy;
   pd->radial.inv2a = 1 / (2 * pd->radial.a);

   pd->radial.extended = (pd->radial.fradius >= 0.00001f) || pd->radial.a >= 0.00001f;

   return EINA_FALSE;
}

// Clearly duplicated and should be in a common place...
static Eina_Bool
_ector_renderer_software_gradient_radial_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Software_Gradient_Data *pd EINA_UNUSED,
                                                             Efl_Gfx_Render_Op op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                                             unsigned int mul_col EINA_UNUSED)
{
   return EINA_TRUE;
}

// Clearly duplicated and should be in a common place...
static Eina_Bool
_ector_renderer_software_gradient_radial_ector_renderer_software_op_fill(Eo *obj EINA_UNUSED, Ector_Renderer_Software_Gradient_Data *pd)
{
   ector_software_rasterizer_radial_gradient_set(pd->surface->rasterizer, pd);
   ector_software_gradient_color_update(pd);
   return EINA_TRUE;
}

Eo *
_ector_renderer_software_gradient_radial_efl_object_constructor(Eo *obj,
                                                             Ector_Renderer_Software_Gradient_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS));
   pd->gd  = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_MIXIN, obj);
   pd->gld = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN, obj);
   pd->done = EINA_TRUE;

   return obj;
}

void
_ector_renderer_software_gradient_radial_efl_object_destructor(Eo *obj,
                                                            Ector_Renderer_Software_Gradient_Data *pd)
{
   Ector_Renderer_Data *base;

   destroy_color_table(pd);

   base = efl_data_scope_get(obj, ECTOR_RENDERER_CLASS);
   efl_data_xunref(base->surface, pd->surface, obj);

   efl_data_xunref(obj, pd->gd, obj);
   efl_data_xunref(obj, pd->gld, obj);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS));
}

void
_ector_renderer_software_gradient_radial_efl_gfx_gradient_stop_set(Eo *obj, Ector_Renderer_Software_Gradient_Data *pd, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   efl_gfx_gradient_stop_set(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS), colors, length);

   destroy_color_table(pd);
}

static unsigned int
_ector_renderer_software_gradient_radial_ector_renderer_crc_get(const Eo *obj, Ector_Renderer_Software_Gradient_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(efl_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS));

   crc = eina_crc((void*) pd->gd->s, sizeof (Efl_Gfx_Gradient_Spread), crc, EINA_FALSE);
   if (pd->gd->colors_count)
     crc = eina_crc((void*) pd->gd->colors, sizeof (Efl_Gfx_Gradient_Stop) * pd->gd->colors_count, crc, EINA_FALSE);
   crc = eina_crc((void*) pd->gld, sizeof (Ector_Renderer_Gradient_Radial_Data), crc, EINA_FALSE);

   return crc;
}

#include "ector_renderer_software_gradient_radial.eo.c"
