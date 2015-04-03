#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"


static void
_update_linear_data(Ector_Renderer_Software_Gradient_Data *gdata)
{
   update_color_table(gdata);
   gdata->linear.x1 = gdata->gld->start.x;
   gdata->linear.y1 = gdata->gld->start.y;

   gdata->linear.x2 = gdata->gld->end.x;
   gdata->linear.y2 = gdata->gld->end.y;

   gdata->linear.dx = gdata->linear.x2 - gdata->linear.x1;
   gdata->linear.dy = gdata->linear.y2 - gdata->linear.y1;
   gdata->linear.l = gdata->linear.dx * gdata->linear.dx + gdata->linear.dy * gdata->linear.dy;
   gdata->linear.off = 0;

   if (gdata->linear.l != 0)
     {
        gdata->linear.dx /= gdata->linear.l;
        gdata->linear.dy /= gdata->linear.l;
        gdata->linear.off = -gdata->linear.dx * gdata->linear.x1 - gdata->linear.dy * gdata->linear.y1;
     }
}


static Eina_Bool
_ector_renderer_software_gradient_linear_ector_renderer_generic_base_prepare(Eo *obj,
                                                                             Ector_Renderer_Software_Gradient_Data *pd)
{
   if (!pd->surface)
     {
        Eo *parent;

        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->surface = eo_data_xref(parent, ECTOR_SOFTWARE_SURFACE_CLASS, obj);
     }

   _update_linear_data(pd);


   return EINA_FALSE;
}

static Eina_Bool
_ector_renderer_software_gradient_linear_ector_renderer_generic_base_draw(Eo *obj EINA_UNUSED,
                                                                          Ector_Renderer_Software_Gradient_Data *pd EINA_UNUSED,
                                                                          Ector_Rop op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                                                          unsigned int mul_col EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_software_gradient_linear_ector_renderer_software_base_fill(Eo *obj EINA_UNUSED,
                                                                           Ector_Renderer_Software_Gradient_Data *pd)
{
   ector_software_rasterizer_linear_gradient_set(pd->surface->software, pd);

   return EINA_TRUE;
}

void
_ector_renderer_software_gradient_linear_eo_base_constructor(Eo *obj,
                                                             Ector_Renderer_Software_Gradient_Data *pd)
{
   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS, eo_constructor());
   pd->gd  = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_GRADIENT_MIXIN, obj);
   pd->gld = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_MIXIN, obj);
}

void
_ector_renderer_software_gradient_linear_eo_base_destructor(Eo *obj,
                                                            Ector_Renderer_Software_Gradient_Data *pd)
{
   Eo *parent;

   destroy_color_table(pd);

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->surface, obj);

   eo_data_xunref(obj, pd->gd, obj);
   eo_data_xunref(obj, pd->gld, obj);

   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS, eo_destructor());
}

void
_ector_renderer_software_gradient_linear_efl_gfx_gradient_base_stop_set(Eo *obj, Ector_Renderer_Software_Gradient_Data *pd, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS,
               efl_gfx_gradient_stop_set(colors, length));

   destroy_color_table(pd);
}

#include "ector_renderer_software_gradient_linear.eo.c"
