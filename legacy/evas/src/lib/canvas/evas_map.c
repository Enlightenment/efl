#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

static void
_calc_map_geometry(Evas_Object *obj)
{
   int is, was = 0, pass = 0;
   Evas_Coord x1, x2, y1, y2;
   const Evas_Map_Point *p, *p_end;

   if (!obj->cur.map) return;
   p = obj->cur.map->points;
   p_end = p + 4;
   x1 = p->x;
   x2 = p->x;
   y1 = p->y;
   y2 = p->y;
   p++;
   for (; p < p_end; p++)
     {
        if (p->x < x1) x1 = p->x;
        if (p->x > x2) x2 = p->x;
        if (p->y < y1) y1 = p->y;
        if (p->y > y2) y2 = p->y;
     }
   obj->cur.geometry.x = x1;
   obj->cur.geometry.y = y1;
   obj->cur.geometry.w = (x2 - x1) + 1;
   obj->cur.geometry.h = (y2 - y1) + 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	evas_object_recalc_clippees(obj);
	if (!pass)
	  {
	     if (!obj->smart.smart)
	       {
		  is = evas_object_is_in_output_rect(obj,
						     obj->layer->evas->pointer.x,
						     obj->layer->evas->pointer.y, 1, 1);
		  if ((is ^ was) && obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_move(obj);
   evas_object_inform_call_resize(obj);
}

EAPI void
evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->cur.usemap == !!enabled) return;
   obj->cur.usemap = enabled;
   if (obj->cur.usemap) _calc_map_geometry(obj);
}

EAPI Eina_Bool
evas_object_map_enable_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->cur.usemap;
}

static inline Evas_Map *
_evas_map_new(unsigned long count)
{
   Evas_Map *m = malloc(sizeof(Evas_Map) + count * sizeof(Evas_Map_Point));
   if (!m)
     return NULL;
   m->count = count;
   return m;
}

static inline Evas_Map *
_evas_map_dup(const Evas_Map *orig)
{
   Evas_Map *copy = _evas_map_new(orig->count);
   if (!orig)
     return NULL;
   memcpy(copy->points, orig->points, orig->count * sizeof(Evas_Map_Point));
   return copy;
}

static inline Eina_Bool
_evas_map_copy(Evas_Map *dst, const Evas_Map *src)
{
   if (dst->count != src->count)
     {
	ERR("cannot copy map of different sizes: dst=%lu, src=%lu",
	    dst->count, src->count);
	return EINA_FALSE;
     }
   memcpy(dst->points, src->points, src->count * sizeof(Evas_Map_Point));
   return EINA_TRUE;
}

static inline void
_evas_map_free(Evas_Map *m)
{
   free(m);
}

/**
 * Set current object transformation map (or unset if it's @c NULL).
 *
 * @param obj object to change transformation map.
 * @param map new map to use or @c NULL to unset map. This function
 *        will copy the given map, so it's safe to destroy it
 *        afterwards.
 *
 * @see evas_map_new()
 */
EAPI void
evas_object_map_set(Evas_Object *obj, const Evas_Map *map)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!map)
     {
        if (obj->cur.map)
          {
             if (!obj->prev.map)
               {
		  _evas_map_free(obj->cur.map);
                  obj->cur.map = NULL;
                  return;
               }
             obj->cur.map = NULL;
          }
        return;
     }
   if (!obj->cur.map)
     {
        obj->cur.map = _evas_map_dup(map);
        obj->prev.map = NULL;
     }
   else
     {
	_evas_map_copy(obj->cur.map, map);
        obj->prev.map = NULL;
     }
   if (obj->cur.usemap) _calc_map_geometry(obj);
}

/**
 * Get current object transformation map.
 *
 * @param obj object to query transformation map.
 * @return map reference to map in use. This is an internal reference,
 *         don't change it anyhow. Use evas_map_dup() if you want to
 *         change it and use somewhere else, or even take reference
 *         for long time. The returned reference may go away when
 *         another map is set or object is destroyed.
 *
 * @see evas_object_map_set()
 */
EAPI const Evas_Map *
evas_object_map_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->cur.map)
     return obj->cur.map;
   else
     return NULL;
}

/**
 * Creates map of transformation points to be later used with an evas object.
 *
 * @param count number of points in the map. So far it @b must be 4 as
 *        evas will use that exact number. The parameter is for
 *        extensibility. Giving any value other than 4 will return @c NULL.
 *
 * @return a newly allocated map or @c NULL on errors.
 *
 * @see evas_map_free()
 * @see evas_map_dup()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 *
 * @see evas_object_map_set()
 */
EAPI Evas_Map *
evas_map_new(unsigned long count)
{
   if (count != 4)
     {
	ERR("num (%lu) != 4 is unsupported!", count);
	return NULL;
     }
   return _evas_map_new(count);
}

/**
 * Copy a previously allocated map.
 *
 * @param m map to copy. Must not be @c NULL.
 *
 * @return newly allocated map with the same count and contents as @a m.
 */
EAPI Evas_Map *
evas_map_dup(const Evas_Map *m)
{
   if (!m) return NULL;
   return _evas_map_dup(m);
}

/**
 * Destroys a previously allocated map.
 *
 * @param m map to destroy. Must not be @c NULL.
 */
EAPI void
evas_map_free(Evas_Map *m)
{
   if (!m) return;
   _evas_map_free(m);
}

/**
 * Change the map point's coordinate.
 *
 * @param m map to change point. Must not be @c NULL.
 * @param idx index of point to change. Must be smaller than map size.
 * @param x TODO
 * @param y TODO
 * @param z TODO
 *
 * @see evas_map_util_rotate()
 * @see evas_map_util_zoom()
 */
EAPI void
evas_map_point_coord_set(Evas_Map *m, unsigned long idx, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->x = x;
   p->y = y;
   p->z = z;
}

/**
 * Get the map point's coordinate.
 *
 * @param m map to query point. Must not be @c NULL.
 * @param idx index of point to query. Must be smaller than map size.
 * @param x where to return TODO. If non @c NULL, it's guaranteed to
 *        be set (on error it's set to zero).
 * @param y where to return TODO. If non @c NULL, it's guaranteed to
 *        be set (on error it's set to zero).
 * @param z where to return TODO. If non @c NULL, it's guaranteed to
 *        be set (on error it's set to zero).
 */
EAPI void
evas_map_point_coord_get(const Evas_Map *m, unsigned long idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z)
{
   const Evas_Map_Point *p;
   if (!m) goto error;
   if (idx >= m->count) goto error;
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

/**
 * Change the map point's TODO.
 *
 * @param m map to change point. Must not be @c NULL.
 * @param idx index of point to change. Must be smaller than map size.
 * @param u TODO
 * @param v TODO
 */
EAPI void
evas_map_point_image_uv_set(Evas_Map *m, unsigned long idx, double u, double v)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->u = u;
   p->v = v;
}

/**
 * Get the map point's TODO.
 *
 * @param m map to query point. Must not be @c NULL.
 * @param idx index of point to query. Must be smaller than map size.
 * @param u where to return TODO. If non @c NULL, it's guaranteed to
 *        be set (on error it's set to zero).
 * @param v where to return TODO. If non @c NULL, it's guaranteed to
 *        be set (on error it's set to zero).
 */
EAPI void
evas_map_point_image_uv_get(const Evas_Map *m, unsigned long idx, double *u, double *v)
{
   const Evas_Map_Point *p;
   if (!m) goto error;
   if (idx >= m->count) goto error;
   p = m->points + idx;
   if (u) *u = p->u;
   if (v) *v = p->v;
   return;

 error:
   if (u) *u = 0.0;
   if (v) *v = 0.0;
}

/**
 * Change the map to apply the given rotation to object.
 *
 * @param m map to change. Must not be @c NULL.
 * @param degrees amount of degrees from 0.0 to 360.0 to rotate the
 *        object. This is the canonical reference, counter-clockwise. TODO
 * @param cx rotation's center horizontal positon. TODO (offset from object center? left?)
 * @param cy rotation's center vertical positon. TODO (offset from object center? top?)
 *
 * @see evas_map_point_coord_set()
 */
EAPI void
evas_map_util_rotate(Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy)
{
   double r = (degrees * M_PI) / 180.0;
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   if (m->count != 4) return;

   p = m->points;
   p_end = p + 4;

   for (; p < p_end; p++)
     {
        Evas_Coord x, y, xx, yy;

        xx = x = p->x - cx;
        yy = y = p->y - cy;

        xx = (x * cos(r));
        yy = (x * sin(r));
        x = xx + (y * cos(r + (M_PI / 2.0)));
        y = yy + (y * sin(r + (M_PI / 2.0)));

        p->x = x + cx;
        p->y = y + cy;
     }
}

/**
 * Change the map to apply the given zooming to object.
 *
 * @param m map to change. Must not be @c NULL.
 * @param zoomx horizontal zoom to use.
 * @param zoomy vertical zoom to use.
 * @param cx zooming center horizontal positon. TODO (offset from object center? left?)
 * @param cy zooming center vertical positon. TODO (offset from object center? top?)
 *
 * @see evas_map_point_coord_set()
 */
EAPI void
evas_map_util_zoom(Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy)
{
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   if (m->count != 4) return;

   p = m->points;
   p_end = p + 4;

   for (; p < p_end; p++)
     {
        Evas_Coord x, y;

        x = p->x - cx;
        y = p->y - cy;

        x = (((double)x) * zoomx);
        y = (((double)y) * zoomy);

        p->x = x + cx;
        p->y = y + cy;
     }
}
