#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "gl/Ector_Gl.h"
#include "ector_private.h"
#include "ector_gl_private.h"

typedef struct _Ector_Renderer_Gl_Data Ector_Renderer_Gl_Data;
struct _Ector_Renderer_Gl_Data
{
   Ector_Renderer_Data *base;
};

static Eina_Bool
_ector_renderer_gl_ector_renderer_prepare(Eo *obj EINA_UNUSED,
                                          Ector_Renderer_Gl_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_gl_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Gl_Data *pd EINA_UNUSED,
                                       Efl_Gfx_Render_Op op EINA_UNUSED,
                                       Eina_Array *clips EINA_UNUSED,
                                       unsigned int mul_col EINA_UNUSED)
{
   return EINA_FALSE;
}

static Efl_Object *
_ector_renderer_gl_efl_object_constructor(Eo *obj, Ector_Renderer_Gl_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_GL_CLASS));
   if (!obj) return NULL;

   pd->base = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);
   return obj;
}

static void
_ector_renderer_gl_efl_object_destructor(Eo *obj, Ector_Renderer_Gl_Data *pd)
{
   efl_data_xunref(obj, pd->base, obj);
   efl_destructor(efl_super(obj, ECTOR_RENDERER_GL_CLASS));
}

#include "ector_renderer_gl.eo.c"
