#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <strings.h>

static void
_efl_vg_gradient_efl_gfx_gradient_base_stop_set(Eo *obj EINA_UNUSED,
                                                Efl_VG_Gradient_Data *pd,
                                                const Efl_Gfx_Gradient_Stop *colors,
                                                unsigned int length)
{
   pd->colors = realloc(pd->colors, length * sizeof(Efl_Gfx_Gradient_Stop));
   if (!pd->colors)
     {
        pd->colors_count = 0;
        return ;
     }

   memcpy(pd->colors, colors, length * sizeof(Efl_Gfx_Gradient_Stop));
   pd->colors_count = length;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_gradient_efl_gfx_gradient_base_stop_get(Eo *obj EINA_UNUSED,
                                                Efl_VG_Gradient_Data *pd,
                                                const Efl_Gfx_Gradient_Stop **colors,
                                                unsigned int *length)
{
   if (colors) *colors = pd->colors;
   if (length) *length = pd->colors_count;
}

static void
_efl_vg_gradient_efl_gfx_gradient_base_spread_set(Eo *obj EINA_UNUSED,
                                                  Efl_VG_Gradient_Data *pd,
                                                  Efl_Gfx_Gradient_Spread s)
{
   pd->s = s;

   _efl_vg_base_changed(obj);
}

static Efl_Gfx_Gradient_Spread
_efl_vg_gradient_efl_gfx_gradient_base_spread_get(Eo *obj EINA_UNUSED,
                                                  Efl_VG_Gradient_Data *pd)
{
   return pd->s;
}

EAPI void
evas_vg_gradient_stop_set(Eo *obj, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   eo_do(obj, efl_gfx_gradient_stop_set(colors, length));
}

EAPI void
evas_vg_gradient_stop_get(Eo *obj, const Efl_Gfx_Gradient_Stop **colors, unsigned int *length)
{
   eo_do(obj, efl_gfx_gradient_stop_get(colors, length));
}

EAPI void
evas_vg_gradient_spread_set(Eo *obj, Efl_Gfx_Gradient_Spread s)
{
   eo_do(obj, efl_gfx_gradient_spread_set(s));
}

EAPI Efl_Gfx_Gradient_Spread
evas_vg_gradient_spread_get(Eo *obj)
{
   Efl_Gfx_Gradient_Spread ret;

   return eo_do_ret(obj, ret, efl_gfx_gradient_spread_get());
}

#include "efl_vg_gradient.eo.c"
