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
   p_end = p + obj->cur.map->count;
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
   obj->cur.map->normal_geometry.x = x1;
   obj->cur.map->normal_geometry.y = y1;
   obj->cur.map->normal_geometry.w = (x2 - x1);
   obj->cur.map->normal_geometry.h = (y2 - y1);
   _evas_map_calc_geom_change(obj);
}

static inline Evas_Map *
_evas_map_new(int count)
{
   int i;
   Evas_Map *m = calloc(1, sizeof(Evas_Map) + count * sizeof(Evas_Map_Point));
   if (!m) return NULL;
   m->count = count;
   m->alpha = 1;
   m->smooth = 1;
   for (i = 0; i < count; i++)
     {
        m->points[i].r = 255;
        m->points[i].g = 255;
        m->points[i].b = 255;
        m->points[i].a = 255;
     }
   return m;
}

static inline Eina_Bool
_evas_map_copy(Evas_Map *dst, const Evas_Map *src)
{
   if (dst->count != src->count)
     {
	ERR("cannot copy map of different sizes: dst=%i, src=%i", dst->count, src->count);
	return EINA_FALSE;
     }
   memcpy(dst->points, src->points, src->count * sizeof(Evas_Map_Point));
   dst->smooth = src->smooth;
   dst->alpha = src->alpha;
   return EINA_TRUE;
}

static inline Evas_Map *
_evas_map_dup(const Evas_Map *orig)
{
   Evas_Map *copy = _evas_map_new(orig->count);
   if (!copy) return NULL;
   memcpy(copy->points, orig->points, orig->count * sizeof(Evas_Map_Point));
   copy->smooth = orig->smooth;
   copy->alpha = orig->alpha;
   return copy;
}

static inline void
_evas_map_free(Evas_Map *m)
{
   free(m);
}

Eina_Bool
evas_map_coords_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y,
                    Evas_Coord *mx, Evas_Coord *my, int grab)
{
   int order[4], i, j, edges, edge[4][2], douv;
   Evas_Coord xe[2];
   double u[2] = { 0.0, 0.0 };
   double v[2] = { 0.0, 0.0 };

   if (m->count != 4) return 0;
   // FIXME need to handle grab mode and extrapolte coords outside
   // map
   if (grab)
     {
        Evas_Coord ymin, ymax;
        
        ymin = m->points[0].y;
        ymax = m->points[0].y;
        for (i = 1; i < m->count; i++)
          {
             if (m->points[i].y < ymin) ymin = m->points[i].y; 
             else if (m->points[i].y > ymax) ymax = m->points[i].y; 
          }
        if (y <= ymin) y = ymin + 1;
        if (y >= ymax) y = ymax - 1;
     }
   edges = 0;
   for (i = 0; i < m->count; i++)
     {
        j = (i + 1) % m->count;
        if ((m->points[i].y <= y) && (m->points[j].y > y))
          {
             edge[edges][0] = i;
             edge[edges][1] = j;
             edges++;
          }
        else if ((m->points[j].y <= y) && (m->points[i].y > y))
          {
             edge[edges][0] = j;
             edge[edges][1] = i;
             edges++;
          }
     }
   douv = 0;
   if ((mx) || (my)) douv = 1;
   for (i = 0; i < (edges - 1); i+= 2)
     {
        Evas_Coord yp, yd, x0, x1;
        
        j = i + 1;
        yd = m->points[edge[i][1]].y - m->points[edge[i][0]].y;
        if (yd > 0)
          {
             yp = y - m->points[edge[i][0]].y;
             xe[0] = m->points[edge[i][1]].x - m->points[edge[i][0]].x;
             xe[0] = m->points[edge[i][0]].x + ((xe[0] * yp) / yd);
             if (douv)
               {
                  u[0] = m->points[edge[i][1]].u - m->points[edge[i][0]].u;
                  u[0] = m->points[edge[i][0]].u + ((u[0] * yp) / yd);
                  v[0] = m->points[edge[i][1]].v - m->points[edge[i][0]].v;
                  v[0] = m->points[edge[i][0]].v + ((v[0] * yp) / yd);
               }
          }
        else
          {
             xe[0] = m->points[edge[i][0]].x;
             if (douv)
               {
                  u[0] = m->points[edge[i][0]].u;
                  v[0] = m->points[edge[i][0]].v;
               }
          }
        yd = m->points[edge[j][1]].y - m->points[edge[j][0]].y;
        if (yd > 0)
          {
             yp = y - m->points[edge[j][0]].y;
             xe[1] = m->points[edge[j][1]].x - m->points[edge[j][0]].x;
             xe[1] = m->points[edge[j][0]].x + ((xe[1] * yp) / yd);
             if (douv)
               {
                  u[1] = m->points[edge[j][1]].u - m->points[edge[j][0]].u;
                  u[1] = m->points[edge[j][0]].u + ((u[1] * yp) / yd);
                  v[1] = m->points[edge[j][1]].v - m->points[edge[j][0]].v;
                  v[1] = m->points[edge[j][0]].v + ((v[1] * yp) / yd);
               }
          }
        else
          {
             xe[1] = m->points[edge[j][0]].x;
             if (douv)
               {
                  u[1] = m->points[edge[j][0]].u;
                  v[1] = m->points[edge[j][0]].v;
               }
          }
        if (xe[0] > xe[1])
          {
             int ti;
             
             ti = xe[0]; xe[0] = xe[1]; xe[1] = ti;
             if (douv)
               {
                  double td;
                  
                  td = u[0]; u[0] = u[1]; u[1] = td;
                  td = v[0]; v[0] = v[1]; v[1] = td;
               }
          }
        if ((x >= xe[0]) && (x < xe[1]))
          {
             if (douv)
               {
                  if (mx) 
                    *mx = u[0] + (((x - xe[0]) * (u[1] - u[0])) / 
                                  (xe[1] - xe[0]));
                 if (my)
                    *my = v[0] + (((x - xe[0]) * (v[1] - v[0])) / 
                                  (xe[1] - xe[0]));
               }
             return 1;
          }
        if (grab)
          {
             if (douv)
               {
                  if (mx) 
                    *mx = u[0] + (((x - xe[0]) * (u[1] - u[0])) / 
                                  (xe[1] - xe[0]));
                  if (my)
                    *my = v[0] + (((x - xe[0]) * (v[1] - v[0])) / 
                                  (xe[1] - xe[0]));
               }
             return 1;
          }
     }
   return 0;
}

Eina_Bool
evas_map_inside_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y)
{
   return evas_map_coords_get(m, x, y, NULL, NULL, 0);
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
   obj->cur.usemap = !!enabled;
   if (enabled)
     {
        if (!obj->cur.map)
          obj->cur.map = _evas_map_new(4);
//        obj->cur.map->normal_geometry = obj->cur.geometry;
     }
   else
     {
        if (obj->cur.map)
          {
             _evas_map_calc_geom_change(obj);
          }
     }
   _evas_map_calc_map_geometry(obj);
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
             if (obj->cur.map->surface)
               {
                  obj->layer->evas->engine.func->image_map_surface_free
                    (obj->layer->evas->engine.data.output,
                     obj->cur.map->surface);
                  obj->cur.map->surface = NULL;
               }
             obj->prev.geometry = obj->cur.map->normal_geometry;
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
   _evas_map_calc_map_geometry(obj);
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
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
 *
 * @see evas_object_map_set()
 */
EAPI Evas_Map *
evas_map_new(int count)
{
   if (count != 4)
     {
	ERR("num (%i) != 4 is unsupported!", count);
	return NULL;
     }
   return _evas_map_new(count);
}

/**
 * Set the smoothing for map rendering
 * 
 * This sets smoothing for map rendering. If the object is a type that has
 * its own smoothing settings, then both the smooth settings for this object
 * and the map must be turned off. By default smooth maps are enabled.
 * 
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable smooth map rendering
 */
EAPI void
evas_map_smooth_set(Evas_Map *m, Eina_Bool enabled)
{
   if (!m) return;
   m->smooth = enabled;
}

/**
 * get the smoothing for map rendering
 * 
 * This gets smoothing for map rendering.
 * 
 * @param m map to get the smooth from. Must not be NULL.
 */
EAPI Eina_Bool
evas_map_smooth_get(const Evas_Map *m)
{
   if (!m) return 0;
   return m->smooth;
}

/**
 * Set the alpha flag for map rendering
 * 
 * This sets alpha flag for map rendering. If the object is a type that has
 * its own alpha settings, then this will take precedence. Only image objects
 * have this currently. Fits stops alpha blending of the map area, and is
 * useful if you know the object and/or all sub-objects is 100% solid.
 * 
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable alpha map rendering
 */
EAPI void
evas_map_alpha_set(Evas_Map *m, Eina_Bool enabled)
{
   if (!m) return;
   m->alpha = enabled;
}

/**
 * get the alpha flag for map rendering
 * 
 * This gets the alph flag for map rendering.
 * 
 * @param m map to get the alpha from. Must not be NULL.
 */
EAPI Eina_Bool
evas_map_alpha_get(const Evas_Map *m)
{
   if (!m) return 0;
   return m->alpha;
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
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
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
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_object()
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

/**
 * Set the color of a vertex in the map
 *
 * This sets the color of the vertex in the map. Colors will be linearly
 * interpolated between vertex points through the map. Color will multiply
 * the "texture" pixels (like GL_MODULATE in OpenGL). The default color of
 * a vertex in a map is white solid (255, 255, 255, 255) which means it will
 * have no affect on modifying the texture pixels.
 * 
 * @param m map to change the color of.
 * @param idx index of point to change. Must be smaller than map size.
 * @param r red (0 - 255)
 * @param g green (0 - 255)
 * @param b blue (0 - 255)
 * @param a alpha (0 - 255)
 *
 * @see evas_map_util_points_color_set()
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void
evas_map_point_color_set(Evas_Map *m, int idx, int r, int g, int b, int a)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->r = r;
   p->g = g;
   p->b = b;
   p->a = a;
}

/**
 * Get the color set on a vertex in the map
 *
 * This gets the color set by evas_map_point_color_set() on the given vertex
 * of the map.
 * 
 * @param m map to get the color of the vertex from.
 * @param idx index of point get. Must be smaller than map size.
 * @param r pointer to red return
 * @param g pointer to green return
 * @param b pointer to blue return
 * @param a pointer to alpha return (0 - 255)
 * 
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void
evas_map_point_color_get(const Evas_Map *m, int idx, int *r, int *g, int *b, int *a)
{
   const Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   if (r) *r = p->r;
   if (g) *g = p->g;
   if (b) *b = p->b;
   if (a) *a = p->a;
}

/****************************************************************************/
/* util functions for manipulating maps, so you don't need to know the math */
/****************************************************************************/
static inline void
_evas_map_util_points_populate(Evas_Map *m, const Evas_Coord x, const Evas_Coord y, const Evas_Coord w, const Evas_Coord h, const Evas_Coord z)
{
   Evas_Map_Point *p = m->points;

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
}

/**
 * Populate source and destination map points to match exactly object.
 *
 * Usually one initialize map of an object to match it's original
 * position and size, then transform these with evas_map_util_*
 * functions, such as evas_map_util_rotate() or
 * evas_map_util_3d_rotate(). The original set is done by this
 * function, avoiding code duplication all around.
 *
 * @param m map to change all 4 points (must be of size 4).
 * @param obj object to use unmapped geometry to populate map coordinates.
 * @param z Point Z Coordinate hint (pre-perspective transform). This value
 *        will be used for all four points.
 *
 * @see evas_map_util_points_populate_from_object()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void
evas_map_util_points_populate_from_object_full(Evas_Map *m, const Evas_Object *obj, Evas_Coord z)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!m)
     {
	ERR("map == NULL");
	return;
     }
   if (m->count != 4)
     {
	ERR("map has count=%d where 4 was expected.", m->count);
	return;
     }
   _evas_map_util_points_populate(m, obj->cur.geometry.x, obj->cur.geometry.y,
				  obj->cur.geometry.w, obj->cur.geometry.h, z);
}

/**
 * Populate source and destination map points to match exactly object.
 *
 * Usually one initialize map of an object to match it's original
 * position and size, then transform these with evas_map_util_*
 * functions, such as evas_map_util_rotate() or
 * evas_map_util_3d_rotate(). The original set is done by this
 * function, avoiding code duplication all around.
 *
 * Z Point coordinate is assumed as 0 (zero).
 *
 * @param m map to change all 4 points (must be of size 4).
 * @param obj object to use unmapped geometry to populate map coordinates.
 *
 * @see evas_map_util_points_populate_from_object_full()
 * @see evas_map_util_points_populate_from_geometry()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void
evas_map_util_points_populate_from_object(Evas_Map *m, const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!m)
     {
	ERR("map == NULL");
	return;
     }
   if (m->count != 4)
     {
	ERR("map has count=%d where 4 was expected.", m->count);
	return;
     }
   _evas_map_util_points_populate(m, obj->cur.geometry.x, obj->cur.geometry.y,
				  obj->cur.geometry.w, obj->cur.geometry.h, 0);
}

/**
 * Populate source and destination map points to match given geometry.
 *
 * Similar to evas_map_util_points_populate_from_object_full(), this
 * call takes raw values instead of querying object's unmapped
 * geometry. The given width will be used to calculate destination
 * points (evas_map_point_coord_set()) and set the image uv
 * (evas_map_point_image_uv_set()).
 *
 * @param m map to change all 4 points (must be of size 4).
 * @param x Point X Coordinate
 * @param y Point Y Coordinate
 * @param w width to use to calculate second and third points.
 * @param h height to use to calculate third and fourth points.
 * @param z Point Z Coordinate hint (pre-perspective transform). This value
 *        will be used for all four points.
 *
 * @see evas_map_util_points_populate_from_object()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 */
EAPI void
evas_map_util_points_populate_from_geometry(Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord z)
{
   if (!m)
     {
	ERR("map == NULL");
	return;
     }
   if (m->count != 4)
     {
	ERR("map has count=%d where 4 was expected.", m->count);
	return;
     }
   _evas_map_util_points_populate(m, x, y, w, h, z);
}

/**
 * Set color of all points to given color.
 *
 * This call is useful to reuse maps after they had 3d lightning or
 * any other colorization applied before.
 *
 * @param m map to change the color of.
 * @param r red (0 - 255)
 * @param g green (0 - 255)
 * @param b blue (0 - 255)
 * @param a alpha (0 - 255)
 *
 * @see evas_map_point_color_set()
 */
EAPI void
evas_map_util_points_color_set(Evas_Map *m, int r, int g, int b, int a)
{
   Evas_Map_Point *p, *p_end;
   if (!m)
     {
	ERR("map == NULL");
	return;
     }
   p = m->points;
   p_end = p + m->count;
   for (; p < p_end; p++)
     {
	p->r = r;
	p->g = g;
	p->b = b;
	p->a = a;
     }
}

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
   p = m->points;
   p_end = p + m->count;

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
   p = m->points;
   p_end = p + m->count;

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

/**
 * Rotate the map around 3 axes in 3D
 * 
 * This will rotate not just around the "Z" axis as in evas_map_util_rotate()
 * (which is a convenience call for those only wanting 2D). This will rotate
 * around the X, Y and Z axes. The Z axis points "into" the screen with low
 * values at the screen and higher values further away. The X axis runs from
 * left to right on the screen and the Y axis from top to bottom. Like with
 * evas_map_util_rotate(0 you provide a center point to rotate around (in 3D).
 *
 * @param m map to change.
 * @param dx amount of degrees from 0.0 to 360.0 to rotate arount X axis.
 * @param dy amount of degrees from 0.0 to 360.0 to rotate arount Y axis.
 * @param dz amount of degrees from 0.0 to 360.0 to rotate arount Z axis.
 * @param cx rotation's center horizontal positon.
 * @param cy rotation's center vertical positon.
 * @param cz rotation's center vertical positon.
 */
EAPI void
evas_map_util_3d_rotate(Evas_Map *m, double dx, double dy, double dz, 
                        Evas_Coord cx, Evas_Coord cy, Evas_Coord cz)
{
   double rz = (dz * M_PI) / 180.0;
   double rx = (dx * M_PI) / 180.0;
   double ry = (dy * M_PI) / 180.0;
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        double x, y, z, xx, yy, zz;

        x = p->x - cx;
        y = p->y - cy;
        z = p->z - cz;
        
        if (rz != 0.0)
          {
             xx = x * cos(rz);
             yy = x * sin(rz);
             x = xx + (y * cos(rz + M_PI_2));
             y = yy + (y * sin(rz + M_PI_2));
          }

        if (ry != 0.0)
          {
             xx = x * cos(ry);
             zz = x * sin(ry);
             x = xx + (z * cos(ry + M_PI_2));
             z = zz + (z * sin(ry + M_PI_2));
          }
        
        if (rx != 0.0)
          {
             zz = z * cos(rx);
             yy = z * sin(rx);
             z = zz + (y * cos(rx + M_PI_2));
             y = yy + (y * sin(rx + M_PI_2));
          }
        
        p->x = x + cx;
        p->y = y + cy;
        p->z = z + cz;
     }
}

/**
 * Perform lighting calculations on the given Map
 * 
 * This is used to apply lighting calculations (from a single light source)
 * to a given map. The R, G and B values of each vertex will be modified to
 * reflect the lighting based on the lixth point coordinates, the light
 * color and the ambient color, and at what angle the map is facing the
 * light source. A surface should have its points be declared in a
 * clockwise fashion if the face is "facing" towards you (as opposed to
 * away from you) as faces have a "logical" side for lighting.
 *
 * @param m map to change.
 * @param lx X coordinate in space of light point
 * @param ly Y coordinate in space of light point
 * @param lz Z coordinate in space of light point
 * @param lr light red value (0 - 255)
 * @param lg light green value (0 - 255)
 * @param lb light blue value (0 - 255)
 * @param lr ambient color red value (0 - 255)
 * @param lg ambient color green value (0 - 255)
 * @param lb ambient color blue value (0 - 255)
 */
EAPI void
evas_map_util_3d_lighting(Evas_Map *m, 
                          Evas_Coord lx, Evas_Coord ly, Evas_Coord lz,
                          int lr, int lg, int lb, int ar, int ab, int ag)
{
   int i;
   
   if (!m) return;
   
   for (i = 0; i < m->count; i++)
     {
        double x, y, z;
        double nx, ny, nz, x1, y1, z1, x2, y2, z2, ln, br;
        int h, j, mr, mg, mb;
        
        x = m->points[i].x;
        y = m->points[i].y;
        z = m->points[i].z;
        
        // calc normal
        h = (i + m->count - 1) % m->count; // prev point
        j = (i + 1) % m->count; // next point

        x1 = m->points[h].x - x;
        y1 = m->points[h].y - y;
        z1 = m->points[h].z - z;
        
        x2 = m->points[j].x - x;
        y2 = m->points[j].y - y;
        z2 = m->points[j].z - z;
        
        nx = (y1 * z2) - (z1 * y2);
        ny = (z1 * x2) - (x1 * z2);
        nz = (x1 * y2) - (y1 * x2);
        
        ln = (nx * nx) + (ny * ny) + (nz * nz);
        ln = sqrt(ln);
        
        if (ln != 0.0)
          {
             nx /= ln;
             ny /= ln;
             nz /= ln;
          }
        
        // calc point -> light vector
        x = lx - x;
        y = ly - y;
        z = lz - z;
        
        ln = (x * x) + (y * y) + (z * z);
        ln = sqrt(ln);
        
        if (ln != 0.0)
          {
             x /= ln;
             y /= ln;
             z /= ln;
          }
        
        // brightness - tan (0.0 -> 1.0 brightness really)
        br = (nx * x) + (ny * y) + (nz * z);
        if (br < 0.0) br = 0.0;
        
        mr = ar + ((lr - ar) * br);
        mg = ag + ((lg - ag) * br);
        mb = ab + ((lb - ab) * br);
        mr = (mr * m->points[i].a) / 255;
        mg = (mg * m->points[i].a) / 255;
        mb = (mb * m->points[i].a) / 255;
        m->points[i].r = (m->points[i].r * mr) / 255;
        m->points[i].g = (m->points[i].g * mg) / 255;
        m->points[i].b = (m->points[i].b * mb) / 255;
     }
}

/**
 * Apply a perspective transform to the map
 * 
 * This applies a given perspective (3D) to the map coordinates. X, Y and Z
 * values are used. The px and py points specify the "infinite distance" point
 * in the 3D conversion (where all lines converge to like when artists draw
 * 3D by hand). The @p z0 value specifis the z value at which there is a 1:1
 * mapping between spatial coorinates and screen coordinates. Any points
 * on this z value will not have their X and Y values modified in the transform.
 * Those further away (Z value higher) will shrink into the distance, and
 * those less than this value will expand and become bigger. The @p foc value
 * determines the "focal length" of the camera. This is in reality the distance
 * between the camera lens plane itself (at or closer than this rendering
 * results are undefined) and the "z0" z value. This allows for some "depth"
 * control and @p foc must be greater than 0.
 *
 * @param m map to change.
 * @param px The pespective distance X coordinate
 * @param py The pespective distance Y coordinate
 * @param z0 The "0" z plane value
 * @param foc The focal distance
 */
EAPI void
evas_map_util_3d_perspective(Evas_Map *m,
                             Evas_Coord px, Evas_Coord py,
                             Evas_Coord z0, Evas_Coord foc)
{
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        Evas_Coord x, y, zz;

        if (foc > 0)
          {
             x = p->x - px;
             y = p->y - py;
             
             zz = ((p->z - z0) + foc);
             
             if (zz > 0)
               {
                  x = (x * foc) / zz;
                  y = (y * foc) / zz;
               }
             
             p->x = px + x;
             p->y = py + y;
          }
     }
}

/**
 * Get the clockwise state of a map
 * 
 * This determines if the output points (X and Y. Z is not used) are
 * clockwise or anti-clockwise. This can be used for "back-face culling". This
 * is where you hide objects that "face away" from you. In this case objects
 * that are not clockwise.
 *
 * @param m map to query.
 * @return 1 if clockwise, 0 otherwise
 */
EAPI Eina_Bool
evas_map_util_clockwise_get(Evas_Map *m)
{
   int i, j, k, count;
   long long c;
   
   if (!m) return 0;
   if (m->count < 3) return 0;
   
   count = 0;
   for (i = 0; i < m->count; i++)
     {
        j = (i + 1) % m->count; 
        k = (i + 2) % m->count;
        c = 
          ((m->points[j].x - m->points[i].x) *
           (m->points[k].y - m->points[j].y))
          -
          ((m->points[j].y - m->points[i].y) *
           (m->points[k].x - m->points[j].x));
        if (c < 0) count--;
        else if (c > 0) count++;
     }
   if (count > 0) return 1;
   return 0;
}
