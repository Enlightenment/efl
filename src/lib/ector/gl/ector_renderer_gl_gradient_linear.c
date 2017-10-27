#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <gl/Ector_Gl.h>

#include "ector_private.h"
#include "ector_gl_private.h"

static void
_update_linear_data(Ector_Renderer_Gl_Gradient_Data *gdata)
{
   update_gradient_texture(gdata);

   gdata->linear.x1 = gdata->gld->start.x;
   gdata->linear.y1 = gdata->gld->start.y;

   gdata->linear.x2 = gdata->gld->end.x;
   gdata->linear.y2 = gdata->gld->end.y;

   gdata->linear.dx = gdata->linear.x2 - gdata->linear.x1;
   gdata->linear.dy = gdata->linear.y2 - gdata->linear.y1;
   if (gdata->linear.dx || gdata->linear.dy)
     gdata->linear.l = 1.0 /(gdata->linear.dx * gdata->linear.dx + gdata->linear.dy * gdata->linear.dy);

}

static Eina_Bool
_ector_renderer_gl_gradient_linear_ector_renderer_prepare(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Gl_Gradient_Data *pd EINA_UNUSED)
{
   _update_linear_data(pd);
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_gradient_linear_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                                       Ector_Renderer_Gl_Gradient_Data *pd EINA_UNUSED,
                                                       Efl_Gfx_Render_Op op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                                       unsigned int mul_col EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_gradient_linear_ector_renderer_gl_fill(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Gl_Gradient_Data *pd EINA_UNUSED)
{
   ector_gl_engine_param_linear_gradient(pd);
   return EINA_TRUE;
}

void
_ector_renderer_gl_gradient_linear_efl_gfx_gradient_stop_set(Eo *obj,
                                                             Ector_Renderer_Gl_Gradient_Data *pd,
                                                             const Efl_Gfx_Gradient_Stop *colors,
                                                             unsigned int length)
{
   efl_gfx_gradient_stop_set(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_LINEAR_CLASS), colors, length);

   destroy_gradient_texture(pd);
}

Eo *
_ector_renderer_gl_gradient_linear_efl_object_constructor(Eo *obj,
                                                          Ector_Renderer_Gl_Gradient_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_LINEAR_CLASS));

   pd->gd  = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_MIXIN, obj);
   pd->gld = efl_data_xref(obj, ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN, obj);
   return obj;
}

void
_ector_renderer_gl_gradient_linear_efl_object_destructor(Eo *obj,
                                                         Ector_Renderer_Gl_Gradient_Data *pd)
{
   destroy_gradient_texture(pd);

   efl_data_xunref(obj, pd->gd, obj);
   efl_data_xunref(obj, pd->gld, obj);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_GL_GRADIENT_LINEAR_CLASS));
}

#include "ector_renderer_gl_gradient_linear.eo.c"
