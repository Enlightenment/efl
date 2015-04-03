#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

static void
_update_radial_data(Ector_Renderer_Software_Gradient_Data *gdata)
{
    update_color_table(gdata);

    gdata->radial.cx = gdata->grd->radial.x;
    gdata->radial.cy = gdata->grd->radial.y;
    gdata->radial.cradius = gdata->grd->radius;

    if (!gdata->grd->focal.x)
      gdata->radial.fx = gdata->grd->radial.x;
    else
      gdata->radial.fx = gdata->grd->focal.x;

    if (!gdata->grd->focal.y)
      gdata->radial.fy = gdata->grd->radial.y;
    else
      gdata->radial.fy = gdata->grd->focal.y;

    gdata->radial.fradius = 0;

    gdata->radial.dx = gdata->radial.cx - gdata->radial.fx;
    gdata->radial.dy = gdata->radial.cy - gdata->radial.fy;

    gdata->radial.dr = gdata->radial.cradius - gdata->radial.fradius;
    gdata->radial.sqrfr = gdata->radial.fradius * gdata->radial.fradius;

    gdata->radial.a = gdata->radial.dr * gdata->radial.dr -
      gdata->radial.dx * gdata->radial.dx -
      gdata->radial.dy * gdata->radial.dy;
    gdata->radial.inv2a = 1 / (2 * gdata->radial.a);

    gdata->radial.extended = (gdata->radial.fradius >= 0.00001f) || gdata->radial.a >= 0.00001f;
}


static Eina_Bool
_ector_renderer_software_gradient_radial_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Software_Gradient_Data *pd)
{
   if (!pd->surface)
     {
        Eo *parent;

        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->surface = eo_data_xref(parent, ECTOR_SOFTWARE_SURFACE_CLASS, obj);
     }

   _update_radial_data(pd);
   return EINA_FALSE;
}

// Clearly duplicated and should be in a common place...
static Eina_Bool
_ector_renderer_software_gradient_radial_ector_renderer_generic_base_draw(Eo *obj EINA_UNUSED,
                                                                       Ector_Renderer_Software_Gradient_Data *pd EINA_UNUSED,
                                                                       Ector_Rop op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                                                       unsigned int mul_col EINA_UNUSED)
{
   return EINA_TRUE;
}

// Clearly duplicated and should be in a common place...
static Eina_Bool
_ector_renderer_software_gradient_radial_ector_renderer_software_base_fill(Eo *obj EINA_UNUSED, Ector_Renderer_Software_Gradient_Data *pd)
{
  ector_software_rasterizer_radial_gradient_set(pd->surface->software, pd);
   return EINA_TRUE;
}

void
_ector_renderer_software_gradient_radial_eo_base_constructor(Eo *obj,
                                                             Ector_Renderer_Software_Gradient_Data *pd)
{
   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS, eo_constructor());
   pd->gd  = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_GRADIENT_MIXIN, obj);
   pd->gld = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_GRADIENT_RADIAL_MIXIN, obj);
}

void
_ector_renderer_software_gradient_radial_eo_base_destructor(Eo *obj,
                                                            Ector_Renderer_Software_Gradient_Data *pd)
{
   Eo *parent;

   destroy_color_table(pd);

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->surface, obj);

   eo_data_xunref(obj, pd->gd, obj);
   eo_data_xunref(obj, pd->gld, obj);

   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS, eo_destructor());
}

void
_ector_renderer_software_gradient_radial_efl_gfx_gradient_base_stop_set(Eo *obj, Ector_Renderer_Software_Gradient_Data *pd, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS,
               efl_gfx_gradient_stop_set(colors, length));

   destroy_color_table(pd);
}

#include "ector_renderer_software_gradient_radial.eo.c"
