#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Renderer_Gradient_Data Ector_Renderer_Gradient_Data;
struct _Ector_Renderer_Gradient_Data
{
};

void
_ector_renderer_gradient_efl_graphics_gradient_stop_set(Eo *obj,
                                                        Ector_Renderer_Gradient_Data *pd,
                                                        const Efl_Graphics_Gradient_Stop *colors,
                                                        unsigned int length)
{
}

void
_ector_renderer_gradient_efl_graphics_gradient_stop_get(Eo *obj,
                                                        Ector_Renderer_Gradient_Data *pd,
                                                        const Efl_Graphics_Gradient_Stop **colors,
                                                        unsigned int *length)
{
}

void
_ector_renderer_gradient_efl_graphics_gradient_spread_set(Eo *obj,
                                                          Ector_Renderer_Gradient_Data *pd,
                                                          Efl_Graphics_Gradient_Spread s)
{
}

Efl_Graphics_Gradient_Spread
_ector_renderer_gradient_efl_graphics_gradient_spread_get(Eo *obj,
                                                          Ector_Renderer_Gradient_Data *pd)
{
}


#include "ector_renderer_gradient.eo.c"
