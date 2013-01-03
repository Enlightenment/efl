#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct _EPhysics_Shape {
     Eina_Inlist *points;
};

static EPhysics_Point *
_ephysics_shape_point_new(void)
{
   EPhysics_Point *point;

   point = (EPhysics_Point *)calloc(1, sizeof(EPhysics_Point));
   if (!point)
     {
        ERR("Failed to allocate point.");
        return NULL;
     }

   return point;
}

const Eina_Inlist *
ephysics_shape_points_get(const EPhysics_Shape *shape)
{
   return shape->points;
}

EAPI EPhysics_Shape *
ephysics_shape_new(void)
{
   EPhysics_Shape *shape;

   shape = (EPhysics_Shape *)calloc(1, sizeof(EPhysics_Shape));
   if (!shape)
     {
        ERR("Failed to allocate shape.");
        return NULL;
     }

   return shape;
}

EAPI void
ephysics_shape_del(EPhysics_Shape *shape)
{
   EPhysics_Point *point;

   if (!shape)
     {
        ERR("Can't delete shape, it's null.");
        return;
     }

   while (shape->points)
     {
        point = EINA_INLIST_CONTAINER_GET(shape->points, EPhysics_Point);
        shape->points = eina_inlist_remove(shape->points, shape->points);
        free(point);
     }

   free(shape);
}

EAPI Eina_Bool
ephysics_shape_point_add(EPhysics_Shape *shape, double x, double y, double z)
{
   EPhysics_Point *point;

   if (!shape)
     {
        ERR("Can't add point to shape, it's null.");
        return EINA_FALSE;;
     }

   point = _ephysics_shape_point_new();
   if (!point)
     return EINA_FALSE;;

   point->x = x;
   point->y = y;
   point->z = z;

   shape->points = eina_inlist_append(shape->points, EINA_INLIST_GET(point));

   return EINA_TRUE;
}

/* TODO: load points from file */
EAPI EPhysics_Shape *
ephysics_shape_load(const char *filename EINA_UNUSED)
{
   EPhysics_Shape *shape;

   shape = ephysics_shape_new();
   if (!shape)
     return NULL;

   return shape;
}

/* TODO: save points to file */
EAPI Eina_Bool
ephysics_shape_save(const EPhysics_Shape *shape EINA_UNUSED, const char *filename EINA_UNUSED)
{
   return EINA_TRUE;
}

#ifdef  __cplusplus
}
#endif
