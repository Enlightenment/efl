#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

static void
_ector_renderer_gradient_linear_efl_gfx_gradient_linear_start_set(Eo *obj EINA_UNUSED,
                                                                          Ector_Renderer_Gradient_Linear_Data *pd,
                                                                          double x, double y)
{
   pd->start.x = x;
   pd->start.y = y;
}

static void
_ector_renderer_gradient_linear_efl_gfx_gradient_linear_start_get(const Eo *obj EINA_UNUSED,
                                                                          Ector_Renderer_Gradient_Linear_Data *pd,
                                                                          double *x, double *y)
{
   if (x) *x = pd->start.x;
   if (y) *y = pd->start.y;
}

static void
_ector_renderer_gradient_linear_efl_gfx_gradient_linear_end_set(Eo *obj EINA_UNUSED,
                                                                        Ector_Renderer_Gradient_Linear_Data *pd,
                                                                        double x, double y)
{
   pd->end.x = x;
   pd->end.y = y;
}

static void
_ector_renderer_gradient_linear_efl_gfx_gradient_linear_end_get(const Eo *obj EINA_UNUSED,
                                                                        Ector_Renderer_Gradient_Linear_Data *pd,
                                                                        double *x, double *y)
{
   if (x) *x = pd->end.x;
   if (y) *y = pd->end.y;
}

#include "ector_renderer_gradient_linear.eo.c"
