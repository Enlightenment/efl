#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include <gl/Ector_Gl.h>
#include "ector_private.h"
#include "ector_gl_private.h"
#include "draw.h"

static Ector_Renderer *
_ector_gl_surface_ector_surface_renderer_factory_new(Eo *obj,
                                                     Ector_Gl_Surface_Data *pd EINA_UNUSED,
                                                     const Efl_Class *type)
{
   if (type == ECTOR_RENDERER_SHAPE_MIXIN)
     return efl_add(ECTOR_RENDERER_GL_SHAPE_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN)
     return efl_add(ECTOR_RENDERER_GL_GRADIENT_LINEAR_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN)
     return efl_add(ECTOR_RENDERER_GL_GRADIENT_RADIAL_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));

   ERR("Couldn't find class for type: %s\n", efl_class_name_get(type));
   return NULL;
}

static void
_ector_gl_surface_context_info_set(Eo *obj EINA_UNUSED, Ector_Gl_Surface_Data *pd, int width, int height, int rotation, Eina_Rect *clip, Eina_Bool clear)
{
   GLint current_fbo;

   GL.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

   if (current_fbo) // render to texture
     {
        pd->context.flip = EINA_TRUE;
        pd->context.rotation = 0;
     }
   else // default frame buffer
     {
        pd->context.flip = EINA_FALSE;
        pd->context.rotation = rotation;
     }

   //@TODO update the stencil info
   pd->context.stencil = EINA_FALSE;

   pd->context.fbo = current_fbo;
   pd->context.w = width;
   pd->context.h = height;
   pd->context.clear = clear;

   pd->context.clip.x = clip->x;
   pd->context.clip.y = clip->y;
   pd->context.clip.w = clip->w;
   pd->context.clip.h = clip->h;

}

void
_ector_gl_surface_vg_info_set(Eo *obj EINA_UNUSED, Ector_Gl_Surface_Data *pd, Eina_Rect  *geom, int color)
{
   pd->vg.geom.x = geom->x;
   pd->vg.geom.y = geom->y;
   pd->vg.geom.w = geom->w;
   pd->vg.geom.h = geom->h;
   pd->vg.color = color;
}

static void
_ector_gl_surface_draw_begin(Eo *obj EINA_UNUSED, Ector_Gl_Surface_Data *pd EINA_UNUSED)
{
   if (pd->direct_draw)
     {
        ector_gl_engine_surface_size_set(pd->context.w, pd->context.h);
        ector_gl_engine_master_clip_set(&(pd->context.clip));
        ector_gl_engine_master_offset_set(pd->vg.geom.x, pd->vg.geom.y);
        ector_gl_engine_info_set(pd->direct_draw, pd->context.stencil,
                                 pd->context.clear, pd->context.flip, pd->context.rotation);
     }
   else
     {
        Eina_Rectangle vg_clip = {0, 0, pd->vg.geom.w, pd->vg.geom.h};
        Eina_Rectangle ctx_clip;
        // translate clip to vg co-ordinate
        ctx_clip.x = pd->context.clip.x - pd->vg.geom.x;
        ctx_clip.y = pd->context.clip.y - pd->vg.geom.y;
        ctx_clip.w = pd->context.clip.w;
        ctx_clip.h = pd->context.clip.h;

        if (!eina_rectangle_intersection(&vg_clip, &ctx_clip))
          return;
        ector_gl_engine_surface_size_set(pd->vg.geom.w, pd->vg.geom.h);
        ector_gl_engine_master_clip_set(&vg_clip);
        ector_gl_engine_master_offset_set(0, 0);
        ector_gl_engine_info_set(pd->direct_draw, EINA_FALSE, EINA_TRUE, EINA_TRUE, 0);
     }

   ector_gl_engine_mul_color_set(pd->vg.color);

   ector_gl_engine_begin();
}

static void
_ector_gl_surface_draw_end(Eo *obj EINA_UNUSED, Ector_Gl_Surface_Data *pd)
{
   if (!pd->direct_draw)
     {
        if (!eina_rectangle_intersection(&(pd->vg.geom), &(pd->context.clip)))
          goto end;
        ector_gl_engine_info_set(pd->direct_draw, pd->context.stencil,
                                 pd->context.clear, pd->context.flip, pd->context.rotation);
        ector_gl_engine_surface_copy(pd->context.fbo, pd->context.w, pd->context.h, &(pd->vg.geom));
     }
end:
   ector_gl_engine_end();
}

static Eo *
_ector_gl_surface_efl_object_constructor(Eo *obj,
                                         Ector_Gl_Surface_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, ECTOR_GL_SURFACE_CLASS));

   ector_gl_engine_init();

   pd->direct_draw = EINA_FALSE;

   return obj;
}

static void
_ector_gl_surface_efl_object_destructor(Eo *obj EINA_UNUSED,
                                        Ector_Gl_Surface_Data *pd EINA_UNUSED)
{
   ector_gl_engine_shutdown();
   efl_destructor(efl_super(obj, ECTOR_GL_SURFACE_CLASS));
}

#include "ector_gl_surface.eo.c"
