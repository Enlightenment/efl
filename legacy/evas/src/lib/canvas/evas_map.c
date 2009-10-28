#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

static void
_evas_map_calc_geom_change(Evas_Object *obj)
{
   int is, was = 0, pass = 0;

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

static void
_evas_map_calc_map_geometry(Evas_Object *obj)
{
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
   _evas_map_calc_geom_change(obj);
}

static inline Evas_Map *
_evas_map_new(int count)
{
   Evas_Map *m = calloc(1, sizeof(Evas_Map) + count * sizeof(Evas_Map_Point));
   if (!m) return NULL;
   m->count = count;
   return m;
}

static inline Evas_Map *
_evas_map_dup(const Evas_Map *orig)
{
   Evas_Map *copy = _evas_map_new(orig->count);
   if (!copy) return NULL;
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
 * Enable or disable the map that is set
 * 
 * This enables the map that is set or disables it. On enable, the object
 * geometry will be saved, and the new geometry will change (position and
 * size) to reflect the map geometry set. If none is set yet, this may be
 * an undefined geometry, unless you have already set the map with
 * evas_object_map_set(). It is suggested you first set a map with
 * evas_object_map_set() with valid useful coordinatesm then enable and
 * disable the map with evas_object_map_enable_set() as needed.
 * 
 * @param obj object to enable the map on
 * @param enbled enabled state
 */
EAPI void
evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->cur.usemap == !!enabled) return;
   obj->cur.usemap = enabled;
   if (enabled)
     {
        if (!obj->cur.map)
          obj->cur.map = _evas_map_new(4);
        obj->cur.map->normal_geometry = obj->cur.geometry;
     }
   else
     {
        if (obj->cur.map)
          {
             obj->cur.geometry = obj->cur.map->normal_geometry;
             _evas_map_calc_geom_change(obj);
          }
     }
   if (obj->cur.usemap) _evas_map_calc_map_geometry(obj);
}

/**
 * Get the map enabled state
 * 
 * This returns the currently enabled state of the map on the object indicated.
 * The default map enable state is off. You can enable and disable it with
 * evas_object_map_enable_set().
 * 
 * @param obj object to get the map enabled state from
 * @return the map enabled state
 */
EAPI Eina_Bool
evas_object_map_enable_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->cur.usemap;
}

/**
 * Set current object transformation map.
 * 
 * This sets the map on a given object. It is copied from the @p map pointer,
 * so there is no need to keep the @p map object if you don't need it anymore.
 * 
 * A map is a set of 4 points which have canvas x, y coordinates per point,
 * with an optional z point value as a hint for perspective correction, if it
 * is available. As well each point has u and v coordinates. These are like
 * "texture coordinates" in OpenGL in that they define a point in the source
 * image that is mapped to that map vertex/point. The u corresponds to the x
 * coordinate of this mapped point and v, the y coordinate. Note that these
 * coordinates describe a bounding region to sample. If you have a 200x100
 * source image and wannt to display it at 200x100 with proper pixel
 * precision, then do:
 * 
 * @code
 * Evas_Map *m = evas_map_new(4);
 * evas_map_point_coord_set(m, 0,   0,   0, 0);
 * evas_map_point_coord_set(m, 1, 200,   0, 0);
 * evas_map_point_coord_set(m, 2, 200, 100, 0);
 * evas_map_point_coord_set(m, 3,   0, 100, 0);
 * evas_map_point_image_uv_set(m, 0,   0,   0);
 * evas_map_point_image_uv_set(m, 1, 200,   0);
 * evas_map_point_image_uv_set(m, 2, 200, 100);
 * evas_map_point_image_uv_set(m, 3,   0, 100);
 * evas_object_map_set(obj, m);
 * evas_map_free(m);
 * @endcode
 * 
 * Note that the map points a uv coordinates match the image geometry. If
 * the @p map parameter is NULL, the sotred map will be freed and geometry
 * prior to enabling/setting a map will be restored.
 *
 * @param obj object to change transformation map
 * @param map new map to use
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
             obj->cur.geometry = obj->cur.map->normal_geometry;
             if (!obj->prev.map)
               {
		  _evas_map_free(obj->cur.map);
                  obj->cur.map = NULL;
                  return;
               }
             obj->cur.map = NULL;
             if (!obj->cur.usemap) _evas_map_calc_geom_change(obj);
             else _evas_map_calc_map_geometry(obj);
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
   if (obj->cur.usemap) _evas_map_calc_map_geometry(obj);
}

/**
 * Get current object transformation map.
 * 
 * This returns the current internal map set on the indicated object. It is
 * intended for read-only acces and is only valid as long as the object is
 * not deleted or the map on the object is not changed. If you wish to modify
 * the map and set it back do the following:
 * 
 * @code
 * const Evas_Map *m = evas_object_map_get(obj);
 * Evas_Map *m2 = evas_map_dup(m);
 * evas_map_util_rotate(m2, 30.0, 0, 0);
 * evas_object_map_set(obj);
 * evas_map_free(m2);
 * @endcode
 *
 * @param obj object to query transformation map.
 * @return map reference to map in use. This is an internal data structure, so
 * do not modify it.
 *
 * @see evas_object_map_set()
 */
EAPI const Evas_Map *
evas_object_map_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->cur.map) return obj->cur.map;
   return NULL;
}

/**
 * Create map of transformation points to be later used with an evas object.
 *
 * This creates a set of points (currently only 4 is supported. no other
 * number for @p count will work). That is empty and ready to be modified
 * with evas_map calls.
 * 
 * @param count number of points in the map. *
 * @return a newly allocated map or NULL on errors.
 *
 * @see evas_map_free()
 * @see evas_map_dup()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 *
 * @see evas_object_map_set()
 */
EAPI Evas_Map *
evas_map_new(int count)
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
 * This makes a duplicate of the @p m object and returns it.
 *
 * @param m map to copy. Must not be NULL.
 * @return newly allocated map with the same count and contents as @p m.
 */
EAPI Evas_Map *
evas_map_dup(const Evas_Map *m)
{
   if (!m) return NULL;
   return _evas_map_dup(m);
}

/**
 * Free a previously allocated map.
 *
 * This frees a givem map @p m and all memory associated with it. You must NOT
 * free a map returned by evas_object_map_get() as this is internal.
 * 
 * @param m map to free.
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
 * This sets the fixen point's coordinate in the map. Note that points
 * describe the outline of a quadrangle and are ordered either clockwise
 * or anit-clock-wise. It is suggested to keep your quadrangles concave and
 * non-complex, though these polygon modes may work, they may not render
 * a desired set of output. The quadrangle will use points 0 and 1 , 1 and 2,
 * 2 and 3, and 3 and 0 to describe the edges of the quandrangle.
 * 
 * The X and Y and Z coordinates are in canvas units. Z is optional and may
 * or may not be honored in drawing. Z is a hint and does not affect the
 * X and Y rendered coordinates. It may be used for calculating fills with
 * perspective correct rendering.
 * 
 * Remember all coordinates are canvas global ones like with move and reize
 * in evas.
 *
 * @param m map to change point. Must not be @c NULL.
 * @param idx index of point to change. Must be smaller than map size.
 * @param x Point X Coordinate
 * @param y Point Y Coordinate
 * @param z Point Z Coordinate hint (pre-perspective transform)
 *
 * @see evas_map_util_rotate()
 * @see evas_map_util_zoom()
 */
EAPI void
evas_map_point_coord_set(Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z)
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
 * This returns the coordinates of the given point in the map.
 *
 * @param m map to query point.
 * @param idx index of point to query. Must be smaller than map size.
 * @param x where to return the X coordinate.
 * @param y where to return the Y coordinate.
 * @param z where to return the Z coordinate.
 */
EAPI void
evas_map_point_coord_get(const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z)
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
 * Change the map point's U and V texture source point
 *
 * This sets the U and V coordinates for the point. This determines which
 * coordinate in the source image is mapped to the given point, much like
 * OpenGL and textures. Notes that these points do select the pixel, but
 * are double floating point values to allow for accuracy and sub-pixel
 * selection.
 * 
 * @param m map to change the point of.
 * @param idx index of point to change. Must be smaller than map size.
 * @param u the X coordinate within the image/texture source
 * @param v the Y coordinate within the image/texture source
 * 
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void
evas_map_point_image_uv_set(Evas_Map *m, int idx, double u, double v)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->u = u;
   p->v = v;
}

/**
 * Get the map point's U and V texture source points
 *
 * This returns the texture points set by evas_map_point_image_uv_set().
 * 
 * @param m map to query point.
 * @param idx index of point to query. Must be smaller than map size.
 * @param u where to write the X coordinate within the image/texture source
 * @param v where to write the Y coordinate within the image/texture source
 */
EAPI void
evas_map_point_image_uv_get(const Evas_Map *m, int idx, double *u, double *v)
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

/****************************************************************************/
/* util functions for manipulating maps, so you don't need to know the math */
/****************************************************************************/

/**
 * Change the map to apply the given rotation.
 * 
 * This rotates the indicated map's coordinates around the center coordinate
 * given by @p cx and @p cy as the rotation center. The points will have their
 * X and Y coordinates rotated clockwise by @p degrees degress (360.0 is a
 * full rotation). Negative values for degrees will rotate counter-clockwise
 * by that amount. All coordinates are canvas global coordinates.
 *
 * @param m map to change.
 * @param degrees amount of degrees from 0.0 to 360.0 to rotate.
 * @param cx rotation's center horizontal positon.
 * @param cy rotation's center vertical positon.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_zoom()
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
 * Change the map to apply the given zooming.
 *
 * Like evas_map_util_rotate(), this zooms the points of the map from a center
 * point. That center is defined by @p cx and @p cy. The @p zoomx and @p zoomy
 * parameters specific how much to zoom in the X and Y direction respectively.
 * A value of 1.0 means "don't zoom". 2.0 means "dobule the size". 0.5 is
 * "half the size" etc. All coordinates are canvas global coordinates.
 * 
 * @param m map to change.
 * @param zoomx horizontal zoom to use.
 * @param zoomy vertical zoom to use.
 * @param cx zooming center horizontal positon.
 * @param cy zooming center vertical positon.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_rotate()
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
