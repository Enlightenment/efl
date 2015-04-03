#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Renderer_Generic_Gradient_Linear_Data Ector_Renderer_Generic_Gradient_Linear_Data;
struct _Ector_Renderer_Generic_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

void
_ector_renderer_gradient_linear_efl_graphics_gradient_linear_start_set(Eo *obj EINA_UNUSED,
                                                                       Ector_Renderer_Generic_Gradient_Linear_Data *pd,
                                                                       double x, double y)
{
   pd->start.x = x;
   pd->start.y = y;
}

void
_ector_renderer_gradient_linear_efl_graphics_gradient_linear_start_get(Eo *obj EINA_UNUSED,
                                                                       Ector_Renderer_Generic_Gradient_Linear_Data *pd,
                                                                       double *x, double *y)
{
   if (x) *x = pd->start.x;
   if (y) *y = pd->start.y;
}

void
_ector_renderer_gradient_linear_efl_graphics_gradient_linear_end_set(Eo *obj EINA_UNUSED,
                                                                     Ector_Renderer_Generic_Gradient_Linear_Data *pd,
                                                                     double x, double y)
{
   pd->end.x = x;
   pd->end.y = y;
}

void
_ector_renderer_gradient_linear_efl_graphics_gradient_linear_end_get(Eo *obj EINA_UNUSED,
                                                                     Ector_Renderer_Generic_Gradient_Linear_Data *pd,
                                                                     double *x, double *y)
{
   if (x) *x = pd->end.x;
   if (y) *y = pd->end.y;
}

#include "ector_renderer_generic_gradient_linear.eo.c"
