#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

static void
_ector_renderer_gradient_radial_efl_gfx_gradient_radial_center_set(Eo *obj EINA_UNUSED,
                                                                           Ector_Renderer_Gradient_Radial_Data *pd,
                                                                           double x, double y)
{
   pd->radial.x = x;
   pd->radial.y = y;
}

static void
_ector_renderer_gradient_radial_efl_gfx_gradient_radial_center_get(const Eo *obj EINA_UNUSED,
                                                                           Ector_Renderer_Gradient_Radial_Data *pd,
                                                                           double *x, double *y)
{
   if (x) *x = pd->radial.x;
   if (y) *y = pd->radial.y;
}

static void
_ector_renderer_gradient_radial_efl_gfx_gradient_radial_radius_set(Eo *obj EINA_UNUSED,
                                                                           Ector_Renderer_Gradient_Radial_Data *pd,
                                                                           double r)
{
   pd->radius = r;
}

static double
_ector_renderer_gradient_radial_efl_gfx_gradient_radial_radius_get(const Eo *obj EINA_UNUSED,
                                                                           Ector_Renderer_Gradient_Radial_Data *pd)
{
   return pd->radius;
}


static void
_ector_renderer_gradient_radial_efl_gfx_gradient_radial_focal_set(Eo *obj EINA_UNUSED,
                                                                          Ector_Renderer_Gradient_Radial_Data *pd,
                                                                          double x, double y)
{
   pd->focal.x = x;
   pd->focal.y = y;
}

static void
_ector_renderer_gradient_radial_efl_gfx_gradient_radial_focal_get(const Eo *obj EINA_UNUSED,
                                                                          Ector_Renderer_Gradient_Radial_Data *pd,
                                                                          double *x, double *y)
{
   if (x) *x = pd->focal.x;
   if (y) *y = pd->focal.y;
}

#include "ector_renderer_gradient_radial.eo.c"
