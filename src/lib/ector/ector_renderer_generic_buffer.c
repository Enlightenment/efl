#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "ector_private.h"
#include "ector_renderer_generic_buffer.eo.h"

#define MY_CLASS ECTOR_RENDERER_GENERIC_BUFFER_MIXIN

EOLIAN static Efl_Gfx_Fill_Spread
_ector_renderer_generic_buffer_efl_gfx_fill_fill_spread_get(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Buffer_Data *pd)
{
   return pd->fill.spread;
}

EOLIAN static void
_ector_renderer_generic_buffer_efl_gfx_fill_fill_spread_set(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Buffer_Data *pd, Efl_Gfx_Fill_Spread spread)
{
   pd->fill.spread = spread;
}

EOLIAN static void
_ector_renderer_generic_buffer_efl_gfx_fill_fill_get(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Buffer_Data *pd, int *x, int *y, int *w, int *h)
{
   if (x) *x = pd->fill.x;
   if (y) *y = pd->fill.y;
   if (w) *w = pd->fill.w;
   if (h) *h = pd->fill.h;
}

EOLIAN static void
_ector_renderer_generic_buffer_efl_gfx_fill_fill_set(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Buffer_Data *pd, int x, int y, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   pd->fill.x = x;
   pd->fill.y = y;
   pd->fill.w = w;
   pd->fill.h = h;
}

EOLIAN static void
_ector_renderer_generic_buffer_buffer_set(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Buffer_Data *pd, Ector_Generic_Buffer *buf)
{
   _eo_xrefplace(&pd->eo_buffer, buf, obj);
}

EOLIAN static Ector_Generic_Buffer *
_ector_renderer_generic_buffer_buffer_get(Eo *obj EINA_UNUSED, Ector_Renderer_Generic_Buffer_Data *pd)
{
   return pd->eo_buffer;
}

EOLIAN static Eo_Base *
_ector_renderer_generic_buffer_eo_base_constructor(Eo *obj, Ector_Renderer_Generic_Buffer_Data *pd)
{
   Eo_Base *ret;

   eo_do_super(obj, MY_CLASS, ret = eo_constructor());
   pd->fill.spread = EFL_GFX_FILL_REPEAT;

   return ret;
}

#include "ector_renderer_generic_buffer.eo.c"
