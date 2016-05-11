#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "gl/Ector_GL.h"
#include "ector_private.h"
#include "ector_gl_private.h"

typedef struct _Ector_Renderer_GL_Data Ector_Renderer_GL_Data;
struct _Ector_Renderer_GL_Data
{
   Ector_Renderer_Generic_Base_Data *base;
};

static Eina_Bool
_ector_renderer_gl_ector_renderer_generic_base_prepare(Eo *obj EINA_UNUSED,
                                                            Ector_Renderer_GL_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_ector_renderer_generic_base_draw(Eo *obj EINA_UNUSED,
                                                         Ector_Renderer_GL_Data *pd,
                                                         Efl_Gfx_Render_Op op,
                                                         Eina_Array *clips,
                                                         unsigned int mul_col EINA_UNUSED)
{
   Eina_Bool r;

   r = ector_gl_surface_state_define(pd->base->surface, op, clips);

   return r;
}

static Eo_Base *
_ector_renderer_gl_eo_base_constructor(Eo *obj, Ector_Renderer_GL_Data *pd)
{
   obj = eo_constructor(eo_super(obj, ECTOR_RENDERER_GL_CLASS));
   if (!obj) return NULL;

   pd->base = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, obj);
   return obj;
}

static void
_ector_renderer_gl_eo_base_destructor(Eo *obj, Ector_Renderer_GL_Data *pd)
{
   eo_data_xunref(obj, pd->base, obj);
}

#include "ector_renderer_gl.eo.c"
