#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "ector_private.h"
#include "ector_buffer.eo.h"

EOLIAN static Efl_Gfx_Colorspace
_ector_buffer_cspace_get(const Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd)
{
   return pd->cspace;
}

EOLIAN static void
_ector_buffer_size_get(const Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd, int *w, int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

EOLIAN static Ector_Buffer_Flag
_ector_buffer_flags_get(const Eo *obj EINA_UNUSED, Ector_Buffer_Data *pd EINA_UNUSED)
{
   return ECTOR_BUFFER_FLAG_NONE;
}

#include "ector_buffer.eo.c"
#include "ector_surface.eo.c"
