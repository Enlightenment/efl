#include "evas_common_private.h"
#include "evas_private.h"

#include <strings.h>

typedef struct _Evas_VG_Gradient_Linear_Data Evas_VG_Gradient_Linear_Data;
struct _Evas_VG_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_start_set(Eo *obj EINA_UNUSED,
                                   Evas_VG_Gradient_Linear_Data *pd,
                                   double x, double y)
{
   pd->start.x = x;
   pd->start.y = y;
}

void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_start_get(Eo *obj EINA_UNUSED,
                                   Evas_VG_Gradient_Linear_Data *pd,
                                   double *x, double *y)
{
   if (x) *x = pd->start.x;
   if (y) *y = pd->start.y;
}

void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_end_set(Eo *obj EINA_UNUSED,
                                 Evas_VG_Gradient_Linear_Data *pd,
                                 double x, double y)
{
   pd->end.x = x;
   pd->end.y = y;
}

void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_end_get(Eo *obj EINA_UNUSED,
                                 Evas_VG_Gradient_Linear_Data *pd,
                                 double *x, double *y)
{
   if (x) *x = pd->end.x;
   if (y) *y = pd->end.y;
}

#include "evas_vg_gradient_linear.eo.c"
