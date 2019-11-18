#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_CUBIC_BEZIER_INTERPOLATOR_CLASS

typedef struct _Efl_Cubic_Bezier_Interpolator_Data Efl_Cubic_Bezier_Interpolator_Data;

struct _Efl_Cubic_Bezier_Interpolator_Data
{
   double control_points[4];
};

EOLIAN static double
_efl_cubic_bezier_interpolator_efl_interpolator_interpolate(Eo *eo_obj EINA_UNUSED,
                                                            Efl_Cubic_Bezier_Interpolator_Data *pd EINA_UNUSED,
                                                            double progress)
{
   if ((progress < 0.0) || (progress > 1.0))
     return progress;

   return ecore_animator_pos_map_n(progress, ECORE_POS_MAP_CUBIC_BEZIER, 4,
                                   pd->control_points);
}

EOLIAN static void
_efl_cubic_bezier_interpolator_control_points_set(Eo *eo_obj EINA_UNUSED,
                                           Efl_Cubic_Bezier_Interpolator_Data *pd,
                                           Eina_Vector2 p1, Eina_Vector2 p2)
{
   pd->control_points[0] = p1.x;
   pd->control_points[1] = p1.y;
   pd->control_points[2] = p2.x;
   pd->control_points[3] = p2.y;
}

EOLIAN static void
_efl_cubic_bezier_interpolator_control_points_get(const Eo *eo_obj EINA_UNUSED,
                                           Efl_Cubic_Bezier_Interpolator_Data *pd,
                                           Eina_Vector2 *p1, Eina_Vector2 *p2)
{
   if (p1)
     {
        p1->x = pd->control_points[0];
        p1->y = pd->control_points[1];
     }

   if (p2)
     {
        p2->x = pd->control_points[2];
        p2->y = pd->control_points[3];
     }
}

EOLIAN static Efl_Object *
_efl_cubic_bezier_interpolator_efl_object_constructor(Eo *eo_obj,
                                                      Efl_Cubic_Bezier_Interpolator_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->control_points[0] = 1.0;
   pd->control_points[1] = 1.0;
   pd->control_points[2] = 1.0;
   pd->control_points[3] = 1.0;

   return eo_obj;
}

#include "efl_cubic_bezier_interpolator.eo.c"
