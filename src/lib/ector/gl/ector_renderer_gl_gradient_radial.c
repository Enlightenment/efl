#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <gl/Ector_Gl.h>

#include "ector_private.h"
#include "ector_gl_private.h"


static void
_update_radial_data(Ector_Renderer_Gl_Gradient_Data *gdata)
{
   update_gradient_texture(gdata);

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

   gdata->radial.dx = gdata->radial.cx - gdata->radial.fx; // fmp.x
   gdata->radial.dy = gdata->radial.cy - gdata->radial.fy; // fmp.y

   gdata->radial.dr = gdata->radial.cradius - gdata->radial.fradius;

   gdata->radial.sqrfr = gdata->radial.fradius * gdata->radial.fradius;

   gdata->radial.a = gdata->radial.dr * gdata->radial.dr -
                     gdata->radial.dx * gdata->radial.dx -
                     gdata->radial.dy * gdata->radial.dy;
   gdata->radial.inv2a = 1.0 / (2.0 * gdata->radial.a);

   gdata->radial.extended = (gdata->radial.fradius >= 0.00001f) || gdata->radial.a >= 0.00001f;
}

static Eina_Bool
_ector_renderer_gl_gradient_radial_ector_renderer_prepare(Eo *obj EINA_UNUSED,
                                                                       Ector_Renderer_Gl_Gradient_Data *pd EINA_UNUSED)
{
   _update_radial_data(pd);

   return EINA_FALSE;
}

// Clearly duplicated and should be in a common place...
static Eina_Bool
_ector_renderer_gl_gradient_radial_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                                       Ector_Renderer_Gl_Gradient_Data *pd EINA_UNUSED,
                                                       Efl_Gfx_Render_Op op EINA_UNUSED,
                                                       Eina_Array *clips EINA_UNUSED,
                                                       unsigned int mul_col EINA_UNUSED)
{
   return EINA_TRUE;
}

// Clearly duplicated and should be in a common place...
static Eina_Bool
_ector_renderer_gl_gradient_radial_ector_renderer_gl_fill(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Gl_Gradient_Data *pd)
{
   ector_gl_engine_param_radial_gradient(pd);
   return EINA_FALSE;
}

void
_ector_renderer_gl_gradient_radial_efl_gfx_gradient_stop_set(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Gl_Gradient_Data *pd,
                                                             const Efl_Gfx_Gradient_Stop *colors,
                                                             unsigned int length)
{
   efl_gfx_gradient_stop_set(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS), colors, length);

   destroy_gradient_texture(pd);
}

static Efl_Object *
_ector_renderer_gl_gradient_radial_efl_object_constructor(Eo *obj, Ector_Renderer_Gl_Gradient_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS));

   if (!obj) return NULL;

   pd->gd = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_MIXIN, obj);
   pd->grd = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN, obj);

   return obj;
}

static void
_ector_renderer_gl_gradient_radial_efl_object_destructor(Eo *obj, Ector_Renderer_Gl_Gradient_Data *pd)
{
   destroy_gradient_texture(pd);
   efl_data_xunref(obj, pd->gd, obj);
   efl_data_xunref(obj, pd->grd, obj);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS));
}


#include "ector_renderer_gl_gradient_radial.eo.c"
