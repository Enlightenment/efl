#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_VG_SHAPE_CLASS

typedef struct _Evas_VG_Shape_Data Evas_VG_Shape_Data;
struct _Evas_VG_Shape_Data
{
   Evas_VG_Path_Command *op;
   double *points;
   unsigned int op_count;
   unsigned int pts_counts;
};

Eina_Bool
_evas_vg_shape_path_set(Eo *obj, Evas_VG_Shape_Data *pd,
                        Evas_VG_Path_Command *op, double *points)
{
   free(pd->points);
   pd->points = NULL;
   free(pd->op);
   pd->op = NULL;

   return evas_vg_path_dup(&pd->op, &pd->points, op, points);
}

Eina_Bool
_evas_vg_shape_bounds_get(Eo *obj, Evas_VG_Shape_Data *pd, Eina_Rectangle *r)
{
}

void
_evas_vg_shape_eo_base_constructor(Eo *obj, Evas_VG_Shape_Data *pd)
{
   Eo *parent;

   eo_super_do(obj, MY_CLASS, eo_constructor());

   eo_do(obj, parent = eo_parent_get());
   if (!eo_isa(obj, EVAS_VG_CONTAINER_CLASS) &&
       !eo_isa(obj, EVAS_VG_CLASS))
     {
        ERR("Parent must be either an Evas_Object_VG or an Evas_VG_Container.");
        eo_error_set(obj);
     }
}
