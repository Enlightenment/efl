/* Common header for maps: legacy Evas_Map API and Efl.Gfx.Mapping */

#ifndef EVAS_MAP_H
#define EVAS_MAP_H

#include "evas_common_private.h"
#include "evas_private.h"
#include <math.h>

Evas_Map *_evas_map_new(int count, Eina_Bool sync);
void _evas_map_reset(Evas_Map *m);
void _evas_map_calc_map_geometry(Evas_Object *eo_obj);
void _map_util_rotate(Evas_Map *m, double degrees, double cx, double cy);
void _map_util_zoom(Evas_Map *m, double zoomx, double zoomy, double cx, double cy);
void _map_util_translate(Evas_Map *m, double dx, double dy, double dz);
void _map_util_3d_rotate(Evas_Map *m, double dx, double dy, double dz, double cx, double cy, double cz);
void _map_util_3d_lighting(Evas_Map *m, double lx, double ly, double lz, int lr, int lg, int lb, int ar, int ag, int ab);
void _map_util_3d_perspective(Evas_Map *m, double px, double py, double z0, double foc);
void _map_util_quat_rotate(Evas_Map *m, double qx, double qy, double qz, double qw, double cx, double cy, double cz);
void _evas_object_map_enable_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool enabled);

static inline void
_evas_map_util_points_populate(Evas_Map *m, const double x, const double y,
                               const double w, const double h, const double z)
{
   Evas_Map_Point *p = m->points;
   int i;

   p[0].x = x;
   p[0].y = y;
   p[0].z = z;
   p[0].u = 0.0;
   p[0].v = 0.0;

   p[1].x = x + w;
   p[1].y = y;
   p[1].z = z;
   p[1].u = w;
   p[1].v = 0.0;

   p[2].x = x + w;
   p[2].y = y + h;
   p[2].z = z;
   p[2].u = w;
   p[2].v = h;

   p[3].x = x;
   p[3].y = y + h;
   p[3].z = z;
   p[3].u = 0.0;
   p[3].v = h;

   for (i = 0; i < 4; i++)
     {
        p[i].px = p[i].x;
        p[i].py = p[i].y;
     }
}

static inline void
_map_point_coord_set(Evas_Map *m, int idx, double x, double y, double z)
{
   Evas_Map_Point *p;

   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < m->count));

   p = m->points + idx;
   p->x = p->px = x;
   p->y = p->py = y;
   p->z = z;
}

static inline void
_map_point_coord_get(const Evas_Map *m, int idx, double *x, double *y, double *z)
{
   const Evas_Map_Point *p;

   EINA_SAFETY_ON_FALSE_GOTO(m && (idx >= 0) && (idx < m->count), error);

   p = m->points + idx;
   if (x) *x = p->x;
   if (y) *y = p->y;
   if (z) *z = p->z;
   return;

error:
   if (x) *x = 0;
   if (y) *y = 0;
   if (z) *z = 0;
}

#define MAP_OBJ_CHANGE() do { \
   _evas_map_calc_map_geometry(eo_obj); \
   evas_object_change(eo_obj, obj); \
   obj->changed_map = EINA_TRUE; \
   } while (0)

#define MAP_POPULATE_DEFAULT(m, z) \
   _evas_map_util_points_populate(m, obj->cur->geometry.x, obj->cur->geometry.y, \
                                  obj->cur->geometry.w, obj->cur->geometry.h, z)

#endif // EVAS_MAP_H

