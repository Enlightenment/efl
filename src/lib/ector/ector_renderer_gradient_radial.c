#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Renderer_Gradient_Radial_Data Ector_Renderer_Gradient_Radial_Data;
struct _Ector_Renderer_Gradient_Radial_Data
{
   struct {
      double x, y;
   } radial, focal;
   double radius;
};

void
_ector_renderer_gradient_radial_efl_graphics_gradient_radial_center_set(Eo *obj EINA_UNUSED,
                                                                        Ector_Renderer_Gradient_Radial_Data *pd,
                                                                        double x, double y)
{
   pd->radial.x = x;
   pd->radial.y = y;
}

void
_ector_renderer_gradient_radial_efl_graphics_gradient_radial_center_get(Eo *obj EINA_UNUSED,
                                                                        Ector_Renderer_Gradient_Radial_Data *pd,
                                                                        double *x, double *y)
{
   if (x) *x = pd->radial.x;
   if (y) *y = pd->radial.y;
}

void
_ector_renderer_gradient_radial_efl_graphics_gradient_radial_radius_set(Eo *obj EINA_UNUSED,
                                                                        Ector_Renderer_Gradient_Radial_Data *pd,
                                                                        double r)
{
   pd->radius = r;
}

double
_ector_renderer_gradient_radial_efl_graphics_gradient_radial_radius_get(Eo *obj EINA_UNUSED,
                                                                        Ector_Renderer_Gradient_Radial_Data *pd)
{
   return pd->radius;
}


void
_ector_renderer_gradient_radial_efl_graphics_gradient_radial_focal_set(Eo *obj EINA_UNUSED,
                                                                       Ector_Renderer_Gradient_Radial_Data *pd,
                                                                       double x, double y)
{
   pd->focal.x = x;
   pd->focal.y = y;
}

void
_ector_renderer_gradient_radial_efl_graphics_gradient_radial_focal_get(Eo *obj EINA_UNUSED,
                                                                       Ector_Renderer_Gradient_Radial_Data *pd,
                                                                       double *x, double *y)
{
   if (x) *x = pd->focal.x;
   if (y) *y = pd->focal.y;
}

#include "ector_renderer_gradient_radial.eo.c"
