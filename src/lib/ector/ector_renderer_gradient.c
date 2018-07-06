#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

static void
_ector_renderer_gradient_efl_gfx_gradient_stop_set(Eo *obj EINA_UNUSED,
                                                                Ector_Renderer_Gradient_Data *pd,
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
}

static void
_ector_renderer_gradient_efl_gfx_gradient_stop_get(const Eo *obj EINA_UNUSED,
                                                                Ector_Renderer_Gradient_Data *pd,
                                                                const Efl_Gfx_Gradient_Stop **colors,
                                                                unsigned int *length)
{
   if (colors) *colors = pd->colors;
   if (length) *length = pd->colors_count;
}

static void
_ector_renderer_gradient_efl_gfx_gradient_spread_set(Eo *obj EINA_UNUSED,
                                                                  Ector_Renderer_Gradient_Data *pd,
                                                                  Efl_Gfx_Gradient_Spread s)
{
   pd->s = s;
}

static Efl_Gfx_Gradient_Spread
_ector_renderer_gradient_efl_gfx_gradient_spread_get(const Eo *obj EINA_UNUSED,
                                                                  Ector_Renderer_Gradient_Data *pd)
{
   return pd->s;
}


#include "ector_renderer_gradient.eo.c"
