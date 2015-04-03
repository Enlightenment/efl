#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _Evas_VG_Gradient_Radial_Data Evas_VG_Gradient_Radial_Data;
struct _Evas_VG_Gradient_Radial_Data
{
   struct {
      double x, y;
   } center, focal;
   double radius;
};

static void
_evas_vg_gradient_radial_efl_graphics_gradient_radial_center_set(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Gradient_Radial_Data *pd,
                                                 double x, double y)
{
   pd->center.x = x;
   pd->center.y = y;
}

static void
_evas_vg_gradient_radial_efl_graphics_gradient_radial_center_get(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Gradient_Radial_Data *pd,
                                                 double *x, double *y)
{
   if (x) *x = pd->center.x;
   if (y) *y = pd->center.y;
}

static void
_evas_vg_gradient_radial_efl_graphics_gradient_radial_radius_set(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Gradient_Radial_Data *pd,
                                                 double r)
{
   pd->radius = r;
}

static double
_evas_vg_gradient_radial_efl_graphics_gradient_radial_radius_get(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Gradient_Radial_Data *pd)
{
   return pd->radius;
}

static void
_evas_vg_gradient_radial_efl_graphics_gradient_radial_focal_set(Eo *obj EINA_UNUSED,
                                                Evas_VG_Gradient_Radial_Data *pd,
                                                double x, double y)
{
   pd->focal.x = x;
   pd->focal.y = y;
}

static void
_evas_vg_gradient_radial_efl_graphics_gradient_radial_focal_get(Eo *obj EINA_UNUSED,
                                                Evas_VG_Gradient_Radial_Data *pd,
                                                double *x, double *y)
{
   if (x) *x = pd->focal.x;
   if (y) *y = pd->focal.y;
}

#include "evas_vg_gradient_radial.eo.c"
