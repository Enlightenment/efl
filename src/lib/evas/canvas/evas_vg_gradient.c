#include "evas_common_private.h"
#include "evas_private.h"

#include <strings.h>

typedef struct _Evas_VG_Gradient_Data Evas_VG_Gradient_Data;
struct _Evas_VG_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow (Using Ector ?).
   Efl_Graphics_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Graphics_Gradient_Spread s;
};

static void
_evas_vg_gradient_efl_graphics_gradient_stop_set(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Gradient_Data *pd,
                                                 const Efl_Graphics_Gradient_Stop *colors,
                                                 unsigned int length)
{
   pd->colors = realloc(pd->colors, length * sizeof(Efl_Graphics_Gradient_Stop));
   if (!pd->colors)
     {
        pd->colors_count = 0;
        return ;
     }

   memcpy(pd->colors, colors, length * sizeof(Efl_Graphics_Gradient_Stop));
   pd->colors_count = length;
}

static void
_evas_vg_gradient_efl_graphics_gradient_stop_get(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Gradient_Data *pd,
                                                 const Efl_Graphics_Gradient_Stop **colors,
                                                 unsigned int *length)
{
   if (colors) *colors = pd->colors;
   if (length) *length = pd->colors_count;
}

static void
_evas_vg_gradient_efl_graphics_gradient_spread_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Gradient_Data *pd,
                                                   Efl_Graphics_Gradient_Spread s)
{
   pd->s = s;
}

static Efl_Graphics_Gradient_Spread
_evas_vg_gradient_efl_graphics_gradient_spread_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Gradient_Data *pd)
{
   return pd->s;
}

#include "evas_vg_gradient.eo.c"
