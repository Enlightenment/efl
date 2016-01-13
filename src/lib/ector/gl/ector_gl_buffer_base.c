#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ector_GL.h"
#include "ector_gl_private.h"
#include "ector_buffer.h"
#include "ector_gl_buffer_base.eo.h"

#define MY_CLASS ECTOR_GL_BUFFER_BASE_MIXIN

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
_ector_gl_buffer_base_vertices_get(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd, double *x, double *y, double *w, double *h)
{
   if (x) *x = (double) pd->atlas.x / pd->atlas.w;
   if (y) *y = (double) pd->atlas.y / pd->atlas.h;
   if (w) *w = (double) pd->generic->w / pd->atlas.w;
   if (h) *h = (double) pd->generic->h / pd->atlas.h;
}

EOLIAN static void
_ector_gl_buffer_base_attach(Eo *obj EINA_UNUSED, Ector_GL_Buffer_Base_Data *pd,
                             int texid, int fboid, Efl_Gfx_Colorspace cspace,
                             int imw, int imh, int tx, int ty, int tw, int th,
                             int l, int r, int t, int b)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->generic);
   EINA_SAFETY_ON_FALSE_RETURN(!pd->generic->immutable);

   pd->generic->cspace = cspace;
   pd->generic->w = imw;
   pd->generic->h = imh;
   pd->atlas.x = tx;
   pd->atlas.y = ty;
   pd->atlas.w = tw;
   pd->atlas.h = th;
   pd->generic->l = l;
   pd->generic->r = r;
   pd->generic->t = t;
   pd->generic->b = b;
   if (!(tx - l) && !(ty - t) && ((tw + l + r) == imw) && ((th + t + b) == imh))
     pd->whole = EINA_TRUE;
   pd->fboid = fboid;
   pd->texid = texid;
}

#include "ector_gl_buffer_base.eo.c"
