#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "ector_private.h"
#include "ector_buffer.eo.h"

EOLIAN static Efl_Gfx_Colorspace
_ector_buffer_cspace_get(Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd)
{
   return pd->cspace;
}

EOLIAN static void
_ector_buffer_border_get(Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd EINA_UNUSED, int *l, int *r, int *t, int *b)
{
   if (l) *l = pd->l;
   if (r) *r = pd->r;
   if (t) *t = pd->t;
   if (b) *b = pd->b;
}

EOLIAN static void
_ector_buffer_size_get(Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd, int *w, int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

EOLIAN static Ector_Buffer_Flag
_ector_buffer_flags_get(Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd EINA_UNUSED)
{
   return ECTOR_BUFFER_FLAG_NONE;
}

#include "ector_buffer.eo.c"
#include "ector_surface.eo.c"
