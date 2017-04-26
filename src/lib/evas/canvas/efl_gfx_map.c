#include "evas_map.h"

#define MY_CLASS EFL_GFX_MAP_MIXIN

/* Efl.Gfx.Map implementation - relies on legacy for now */
// note: cur vs. prev is not handled

#define MAP_OBJ_CHANGE() do { \
   _evas_map_calc_map_geometry(eo_obj); \
   evas_object_change(eo_obj, obj); \
   obj->changed_map = EINA_TRUE; \
   } while (0)

#define MAP_POPULATE_DEFAULT(m, z) \
   _evas_map_util_points_populate(m, obj->cur->geometry.x, obj->cur->geometry.y, \
                                  obj->cur->geometry.w, obj->cur->geometry.h, z)

EOLIAN static Eina_Bool
_efl_gfx_map_map_clockwise_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return evas_map_util_clockwise_get((Evas_Map *) evas_object_map_get(eo_obj));
}

EOLIAN static void
_efl_gfx_map_map_smooth_set(Eo *eo_obj, void *_pd EINA_UNUSED, Eina_Bool smooth)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (smooth) return;
        if (!efl_gfx_map_populate(eo_obj, 0))
          return;
     }

   m = (Evas_Map *) obj->map->cur.map;
   m->smooth = smooth;
   MAP_OBJ_CHANGE();
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_smooth_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   const Evas_Map *om = evas_object_map_get(eo_obj);
   return om ? om->smooth : EINA_TRUE;
}

EOLIAN static void
_efl_gfx_map_map_alpha_set(Eo *eo_obj, void *_pd EINA_UNUSED, Eina_Bool alpha)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (alpha) return;
        if (!efl_gfx_map_populate(eo_obj, 0))
          return;
     }

   m = (Evas_Map *) obj->map->cur.map;
   m->alpha = alpha;
   MAP_OBJ_CHANGE();
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_alpha_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   const Evas_Map *om = evas_object_map_get(eo_obj);
   return om ? om->alpha : EINA_TRUE;
}

EOLIAN static void
_efl_gfx_map_map_point_coord_set(Eo *eo_obj, void *_pd EINA_UNUSED, int idx, double x, double y, double z)
{
   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m;

   if (!obj->map->cur.map && !efl_gfx_map_populate(eo_obj, 0))
     return;

   m = (Evas_Map *) obj->map->cur.map;
   _map_point_coord_set(m, idx, x, y, z);
   MAP_OBJ_CHANGE();
}

EOLIAN static void
_efl_gfx_map_map_point_coord_get(Eo *eo_obj, void *_pd EINA_UNUSED, int idx, double *x, double *y, double *z)
{
   const Evas_Map *om = evas_object_map_get(eo_obj);
   _map_point_coord_get(om, idx, x, y, z);
}

EOLIAN static void
_efl_gfx_map_map_point_image_uv_set(Eo *eo_obj, void *_pd EINA_UNUSED, int idx, double u, double v)
{
   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m;

   if (!obj->map->cur.map && !efl_gfx_map_populate(eo_obj, 0))
     return;

   m = (Evas_Map *) obj->map->cur.map;
   evas_map_point_image_uv_set(m, idx, u, v);
   MAP_OBJ_CHANGE();
}

EOLIAN static void
_efl_gfx_map_map_point_image_uv_get(Eo *eo_obj, void *_pd EINA_UNUSED, int idx, double *u, double *v)
{
   const Evas_Map *om = evas_object_map_get(eo_obj);

   evas_map_point_image_uv_get(om, idx, u, v);
}

EOLIAN static void
_efl_gfx_map_map_color_set(Eo *eo_obj, void *_pd EINA_UNUSED, int idx, int r, int g, int b, int a)
{
   EINA_SAFETY_ON_FALSE_RETURN((idx >= -1) && (idx < 4));

   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m;

   if (!obj->map->cur.map && !efl_gfx_map_populate(eo_obj, 0))
     return;

   m = (Evas_Map *) obj->map->cur.map;
   if (idx == -1)
     evas_map_util_points_color_set(m, r, g, b, a);
   else
     evas_map_point_color_set(m, idx, r, g, b, a);
   MAP_OBJ_CHANGE();
}

EOLIAN static void
_efl_gfx_map_map_color_get(Eo *eo_obj, void *_pd EINA_UNUSED, int idx, int *r, int *g, int *b, int *a)
{
   const Evas_Map *om = evas_object_map_get(eo_obj);

   evas_map_point_color_get(om, idx, r, g, b, a);
}

EOLIAN static void
_efl_gfx_map_map_move_sync_set(Eo *eo_obj, void *_pd EINA_UNUSED, Eina_Bool enable)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (enable) return;
        if (!efl_gfx_map_populate(eo_obj, 0))
          return;
     }

   m = (Evas_Map *) obj->map->cur.map;
   m->move_sync.enabled = enable;
   MAP_OBJ_CHANGE();
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_move_sync_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   const Evas_Map *om = evas_object_map_get(eo_obj);

   return om ? om->move_sync.enabled : EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_populate(Eo *eo_obj, void *_pd EINA_UNUSED, double z)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        m = _evas_map_new(4, EINA_TRUE);
        if (!m) return EINA_FALSE;
        MAP_POPULATE_DEFAULT(m, z);
        evas_object_map_set(eo_obj, m);
        evas_map_free(m);
     }
   else
     {
        m = (Evas_Map *) obj->map->cur.map;
        MAP_POPULATE_DEFAULT(m, z);
        MAP_OBJ_CHANGE();
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_populate_manual(Eo *eo_obj, void *_pd EINA_UNUSED, double x, double y, double w, double h, double z)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        m = _evas_map_new(4, EINA_TRUE);
        if (!m) return EINA_FALSE;
        _evas_map_util_points_populate(m, x, y, w, h, z);
        evas_object_map_set(eo_obj, m);
        evas_map_free(m);
     }
   else
     {
        m = (Evas_Map *) obj->map->cur.map;
        _evas_map_util_points_populate(m, x, y, w, h, z);
        MAP_OBJ_CHANGE();
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_rotate(Eo *eo_obj, void *_pd EINA_UNUSED, double degrees, double cx, double cy)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (!efl_gfx_map_populate(eo_obj, 0))
          return EINA_FALSE;
     }

   m = (Evas_Map *) obj->map->cur.map;
   _map_util_rotate(m, degrees, cx, cy);
   MAP_OBJ_CHANGE();

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_zoom(Eo *eo_obj, void *_pd EINA_UNUSED, double zoomx, double zoomy, double cx, double cy)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (!efl_gfx_map_populate(eo_obj, 0))
          return EINA_FALSE;
     }

   m = (Evas_Map *) obj->map->cur.map;
   _map_util_zoom(m, zoomx, zoomy, cx, cy);
   MAP_OBJ_CHANGE();

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_rotate_3d(Eo *eo_obj, void *_pd EINA_UNUSED, double dx, double dy, double dz, double cx, double cy, double cz)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (!efl_gfx_map_populate(eo_obj, 0))
          return EINA_FALSE;
     }

   m = (Evas_Map *) obj->map->cur.map;
   _map_util_3d_rotate(m, dx, dy, dz, cx, cy, cz);
   MAP_OBJ_CHANGE();

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_quat_rotate(Eo *eo_obj, void *_pd EINA_UNUSED, double qx, double qy, double qz, double qw, double cx, double cy, double cz)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (!efl_gfx_map_populate(eo_obj, 0))
          return EINA_FALSE;
     }

   m = (Evas_Map *) obj->map->cur.map;
   evas_map_util_quat_rotate(m, qx, qy, qz, qw, cx, cy, cz);
   MAP_OBJ_CHANGE();

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_lightning_3d(Eo *eo_obj, void *_pd EINA_UNUSED, double lx, double ly, double lz, int lr, int lg, int lb, int ar, int ag, int ab)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (!efl_gfx_map_populate(eo_obj, 0))
          return EINA_FALSE;
     }

   m = (Evas_Map *) obj->map->cur.map;
   _map_util_3d_lighting(m, lx, ly, lz, lr, lg, lb, ar, ag, ab);
   MAP_OBJ_CHANGE();

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_perspective_3d(Eo *eo_obj, void *_pd EINA_UNUSED, double px, double py, double z0, double foc)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   Evas_Map *m;

   if (!obj->map->cur.map)
     {
        if (!efl_gfx_map_populate(eo_obj, 0))
          return EINA_FALSE;
     }

   m = (Evas_Map *) obj->map->cur.map;
   _map_util_3d_perspective(m, px, py, z0, foc);
   MAP_OBJ_CHANGE();

   return EINA_TRUE;
}

EOLIAN static double
_efl_gfx_map_map_point_z_get(Eo *eo_obj, void *_pd EINA_UNUSED, int idx)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, 0.0);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((idx >= 0) && (idx < 4), 0.0);

   const Evas_Map *m = obj->map->cur.map;
   const Evas_Map_Point *p = m ? (m->points + idx) : NULL;

   return p ? p->z : 0.0;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_dup(Eo *eo_obj, void *_pd EINA_UNUSED, const Efl_Gfx_Map *other)
{
   evas_object_map_set(eo_obj, evas_object_map_get(other));
   return EINA_TRUE;
}

#include "canvas/efl_gfx_map.eo.c"
