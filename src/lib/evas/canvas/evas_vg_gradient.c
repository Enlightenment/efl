#include "evas_common_private.h"
#include "evas_private.h"

#include <strings.h>

typedef struct _Evas_VG_Gradient_Data Evas_VG_Gradient_Data;
struct _Evas_VG_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow.
   Evas_VG_Gradient_Stop *colors;
   unsigned int colors_count;

   Evas_VG_Gradient_Spread s;
};

void
_evas_vg_gradient_stop_set(Eo *obj EINA_UNUSED,
                           Evas_VG_Gradient_Data *pd,
                           const Evas_VG_Gradient_Stop *colors,
                           unsigned int length)
{
   pd->colors = realloc(pd->colors, length * sizeof(Evas_VG_Gradient_Stop));
   if (!pd->colors)
     {
        pd->colors_count = 0;
        return ;
     }

   memcpy(pd->colors, colors, length * sizeof(Evas_VG_Gradient_Stop));
   pd->colors_count = length;
}

void
_evas_vg_gradient_stop_get(Eo *obj EINA_UNUSED,
                           Evas_VG_Gradient_Data *pd,
                           const Evas_VG_Gradient_Stop **colors,
                           unsigned int *length)
{
   if (colors) *colors = pd->colors;
   if (length) *length = pd->colors_count;
}

void
_evas_vg_gradient_spread_set(Eo *obj EINA_UNUSED,
                             Evas_VG_Gradient_Data *pd,
                             Evas_VG_Gradient_Spread s)
{
   pd->s = s;
}

Evas_VG_Gradient_Spread
_evas_vg_gradient_spread_get(Eo *obj EINA_UNUSED,
                             Evas_VG_Gradient_Data *pd)
{
   return pd->s;
}

#include "evas_vg_gradient.eo.c"
