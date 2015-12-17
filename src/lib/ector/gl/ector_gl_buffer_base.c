#define EFL_BETA_API_SUPPORT
#include <Eo.h>

#include "Ector_GL.h"
#include "ector_gl_private.h"
#include "ector_buffer.h"
#include "ector_gl_buffer_base.eo.h"

#define MY_CLASS ECTOR_GL_BUFFER_BASE_MIXIN

typedef struct
{
   Ector_Generic_Buffer_Data *generic;
   int texid;
   int fboid;
   int w, h; // Texture size, not the atlas
   struct {
      // x,y offset within the atlas
      // w,h size of the atlas itself
      int x, y, w, h;
   } atlas;
   Eina_Bool whole : 1;
} Ector_GL_Buffer_Base_Data;

EOLIAN static int
_ector_gl_buffer_base_texture_get(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd)
{
   return pd->texid;
}

EOLIAN static int
_ector_gl_buffer_base_fbo_get(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd)
{
   return pd->fboid;
}

EOLIAN static Eina_Bool
_ector_gl_buffer_base_whole_get(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd)
{
   return pd->whole;
}

EOLIAN static void
_ector_gl_buffer_base_size_get(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd, int *w, int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

EOLIAN static void
_ector_gl_buffer_base_vertices_get(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd, double *x, double *y, double *w, double *h)
{
   if (x) *x = (double) pd->atlas.x / pd->atlas.w;
   if (y) *y = (double) pd->atlas.y / pd->atlas.h;
   if (w) *w = (double) pd->w / pd->atlas.w;
   if (h) *h = (double) pd->h / pd->atlas.h;
}

#include "ector_gl_buffer_base.eo.c"
