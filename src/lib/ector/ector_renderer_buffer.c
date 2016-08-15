#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "ector_private.h"
#include "ector_renderer_buffer.eo.h"

#define MY_CLASS ECTOR_RENDERER_BUFFER_MIXIN

EOLIAN static void
_ector_renderer_buffer_efl_gfx_fill_fill_get(Eo *obj EINA_UNUSED, Ector_Renderer_Buffer_Data *pd, int *x, int *y, int *w, int *h)
{
   if (x) *x = pd->fill.x;
   if (y) *y = pd->fill.y;
   if (w) *w = pd->fill.w;
   if (h) *h = pd->fill.h;
}

EOLIAN static void
_ector_renderer_buffer_efl_gfx_fill_fill_set(Eo *obj EINA_UNUSED, Ector_Renderer_Buffer_Data *pd, int x, int y, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   pd->fill.x = x;
   pd->fill.y = y;
   pd->fill.w = w;
   pd->fill.h = h;
}

EOLIAN static void
_ector_renderer_buffer_buffer_set(Eo *obj EINA_UNUSED, Ector_Renderer_Buffer_Data *pd, Ector_Buffer *buf)
{
   _efl_xrefplace(&pd->eo_buffer, buf, obj);
}

EOLIAN static Ector_Buffer *
_ector_renderer_buffer_buffer_get(Eo *obj EINA_UNUSED, Ector_Renderer_Buffer_Data *pd)
{
   return pd->eo_buffer;
}

#include "ector_renderer_buffer.eo.c"
