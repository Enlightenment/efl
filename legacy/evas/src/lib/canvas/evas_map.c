#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

static void
_evas_map_calc_geom_change(Evas_Object *obj)
{
   int is, was = 0;

   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
        evas_object_recalc_clippees(obj);
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
   evas_object_inform_call_move(obj);
   evas_object_inform_call_resize(obj);
}

static void
_evas_map_calc_map_geometry(Evas_Object *obj)
{
   Evas_Coord x1, x2, yy1, yy2;
   const Evas_Map_Point *p, *p_end;
   Eina_Bool ch = EINA_FALSE;

   if (!obj->cur.map) return;
   // WARN: Do not merge below code to SLP until it is fixed.
   // It has an infinite loop bug.
   if (obj->prev.map)
     {
        if (obj->prev.map != obj->cur.map)
          {
             // FIXME: this causes an infinite loop somewhere... hard to debug
             if (obj->prev.map->count == obj->cur.map->count)
               {
                  const Evas_Map_Point *p2;
                  
                  p = obj->cur.map->points;
                  p2 = obj->prev.map->points;
                  if (memcmp(p, p2, sizeof(Evas_Map_Point) * 
                             obj->prev.map->count) != 0)
                    ch = EINA_TRUE;
                  if (!ch)
                    {
                       if (obj->cache_map) evas_map_free(obj->cache_map); 
                       obj->cache_map = obj->cur.map;
                       obj->cur.map = obj->prev.map;
                    }
               }
             else
               ch = EINA_TRUE;
          }
     }
   else
      ch = EINA_TRUE;
   
   p = obj->cur.map->points;
   p_end = p + obj->cur.map->count;
   x1 = x2 = lround(p->x);
   yy1 = yy2 = lround(p->y);
   p++;
   for (; p < p_end; p++)
     {
        Evas_Coord x, y;
        
        x = lround(p->x);
        y = lround(p->y);
        if (x < x1) x1 = x;
        if (x > x2) x2 = x;
        if (y < yy1) yy1 = y;
        if (y > yy2) yy2 = y;
     }
// this causes clip-out bugs now mapped objs canbe opaque!!!   
//   // add 1 pixel of fuzz around the map region to ensure updates are correct
//   x1 -= 1; yy1 -= 1;
//   x2 += 1; yy2 += 1;
   if (obj->cur.map->normal_geometry.x != x1) ch = 1;
   if (obj->cur.map->normal_geometry.y != yy1) ch = 1;
   if (obj->cur.map->normal_geometry.w != (x2 - x1)) ch = 1;
   if (obj->cur.map->normal_geometry.h != (yy2 - yy1)) ch = 1;
   obj->cur.map->normal_geometry.x = x1;
   obj->cur.map->normal_geometry.y = yy1;
   obj->cur.map->normal_geometry.w = (x2 - x1);
   obj->cur.map->normal_geometry.h = (yy2 - yy1);
   obj->changed_map = ch;
   // This shouldn't really be needed, but without it we do have case
   // where the clip is wrong when a map doesn't change, so always forcing
   // it, as long as someone doesn't find a better fix.
   evas_object_clip_dirty(obj);
   if (ch) _evas_map_calc_geom_change(obj);
}

static inline Evas_Map *
_evas_map_new(int count)
{
   int i;
   int alloc;
   Evas_Map *m;

   /* Adjust allocation such that: at least 4 points, and always an even
    * number: this allows the software engine to work efficiently */
   alloc = (count < 4) ? 4 : count;
   if (alloc & 0x1) alloc ++;

   m = calloc(1, sizeof(Evas_Map) + (alloc * sizeof(Evas_Map_Point)));
   if (!m) return NULL;
   m->count = count;
   m->persp.foc = 0;
   m->alpha = 1;
   m->smooth = 1;
   m->magic = MAGIC_MAP;
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
   if (dst == src) return EINA_TRUE;
   if (dst->points != src->points)
     memcpy(dst->points, src->points, src->count * sizeof(Evas_Map_Point));
   dst->smooth = src->smooth;
   dst->alpha = src->alpha;
   dst->persp = src->persp;
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
   copy->persp = orig->persp;
   return copy;
}

static inline void
_evas_map_free(Evas_Object *obj, Evas_Map *m)
{
   if (obj)
     {
        if (obj->spans)
          {
             obj->layer->evas->engine.func->image_map_clean(obj->layer->evas->engine.data.output, obj->spans);
             free(obj->spans);
             obj->spans = NULL;
          }      
     }
   m->magic = 0;
   free(m);
}

/****************************************************************************/
/* util functions for manipulating maps, so you don't need to know the math */
/****************************************************************************/
static inline void
_evas_map_util_points_populate(Evas_Map *m, const Evas_Coord x, const Evas_Coord y, const Evas_Coord w, const Evas_Coord h, const Evas_Coord z)
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

Eina_Bool
evas_map_coords_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y,
                    Evas_Coord *mx, Evas_Coord *my, int grab)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   int i, j, edges, edge[m->count][2], douv;
   Evas_Coord xe[2];
   double u[2] = { 0.0, 0.0 };
   double v[2] = { 0.0, 0.0 };

   if (m->count < 4) return 0;
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
        Evas_Coord yp, yd;

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
             return EINA_TRUE;
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
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

Eina_Bool
evas_map_inside_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y)
{
   return evas_map_coords_get(m, x, y, NULL, NULL, 0);
}

static Eina_Bool
_evas_object_map_parent_check(Evas_Object *parent)
{
   const Eina_Inlist *list;
   const Evas_Object *o;

   if (!parent) return EINA_FALSE;

   list = evas_object_smart_members_get_direct(parent->smart.parent);
   EINA_INLIST_FOREACH(list, o)
     if (o->cur.usemap) break ;
   if (o) return EINA_FALSE; /* Still some child have a map enable */
   parent->child_has_map = EINA_FALSE;
   _evas_object_map_parent_check(parent->smart.parent);
   return EINA_TRUE;
}

EAPI void
evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled)
{
   Evas_Object *parents;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Eina_Bool pchange = EINA_FALSE;

   enabled = !!enabled;
   if (obj->cur.usemap == enabled) return;
   pchange = obj->changed;
   obj->cur.usemap = enabled;
   if (enabled)
     {
        if (!obj->cur.map)
          obj->cur.map = _evas_map_new(4);
        evas_object_mapped_clip_across_mark(obj);
//        obj->cur.map->normal_geometry = obj->cur.geometry;
     }
   else
     {
        if (obj->map.surface)
          {
             obj->layer->evas->engine.func->image_map_surface_free
               (obj->layer->evas->engine.data.output,
                   obj->map.surface);
             obj->map.surface = NULL;
          }
        if (obj->cur.map)
          {
             _evas_map_calc_geom_change(obj);
             evas_object_mapped_clip_across_mark(obj);
          }
     }
   _evas_map_calc_map_geometry(obj);
   /* This is a bit heavy handed, but it fixes the case of same geometry, but
    * changed colour or UV settings. */
   evas_object_change(obj);
   if (!obj->changed_pchange) obj->changed_pchange = pchange;
   obj->changed_map = EINA_TRUE;

   if (enabled)
     {
        for (parents = obj->smart.parent; parents; parents = parents->smart.parent)
          parents->child_has_map = EINA_TRUE;
     }
   else
     {
        if (_evas_object_map_parent_check(obj->smart.parent))
          evas_object_update_bounding_box(obj);
     }
}

EAPI Eina_Bool
evas_object_map_enable_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   return obj->cur.usemap;
}

EAPI void
evas_object_map_set(Evas_Object *obj, const Evas_Map *map)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if ((!map) || (map->count < 4))
     {
        if (obj->map.surface)
          {
             obj->layer->evas->engine.func->image_map_surface_free
               (obj->layer->evas->engine.data.output,
                   obj->map.surface);
             obj->map.surface = NULL;
          }
        if (obj->cur.map)
          {
             obj->changed_map = EINA_TRUE;
             obj->prev.geometry = obj->cur.map->normal_geometry;

             if (obj->prev.map == obj->cur.map)
               obj->cur.map = NULL;
             else if (!obj->cache_map)
               {
                  obj->cache_map = obj->cur.map;
                  obj->cur.map = NULL;
               }
             else
               {
                  _evas_map_free(obj, obj->cur.map);
                  obj->cur.map = NULL;
               }

             if (!obj->prev.map)
               {
                  evas_object_mapped_clip_across_mark(obj);
                  return;
               }

             if (!obj->cur.usemap) _evas_map_calc_geom_change(obj);
             else _evas_map_calc_map_geometry(obj);
             if (obj->cur.usemap)
               evas_object_mapped_clip_across_mark(obj);
          }
        return;
     }

   if (obj->prev.map == obj->cur.map)
     obj->cur.map = NULL;

   if (!obj->cur.map)
     {
        obj->cur.map = obj->cache_map;
        obj->cache_map = NULL;
     }

   // We do have the same exact count of point in this map, so just copy it
   if ((obj->cur.map) && (obj->cur.map->count == map->count))
     _evas_map_copy(obj->cur.map, map);
   else
     {
        if (obj->cur.map) _evas_map_free(obj, obj->cur.map);
        obj->cur.map = _evas_map_dup(map);
        if (obj->cur.usemap)
           evas_object_mapped_clip_across_mark(obj);
     }

   _evas_map_calc_map_geometry(obj);
}

EAPI const Evas_Map *
evas_object_map_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   return obj->cur.map;
}

EAPI Evas_Map *
evas_map_new(int count)
{
   if (count != 4)
     {
        ERR("map point count (%i) != 4 is unsupported!", count);
        return NULL;
     }

   return _evas_map_new(count);
}

EAPI void
evas_map_smooth_set(Evas_Map *m, Eina_Bool enabled)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   m->smooth = enabled;
}

EAPI Eina_Bool
evas_map_smooth_get(const Evas_Map *m)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   return m->smooth;
}

EAPI void
evas_map_alpha_set(Evas_Map *m, Eina_Bool enabled)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   m->alpha = enabled;
}

EAPI Eina_Bool
evas_map_alpha_get(const Evas_Map *m)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   return m->alpha;
}

EAPI Evas_Map *
evas_map_dup(const Evas_Map *m)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return NULL;
   MAGIC_CHECK_END();

   return _evas_map_dup(m);
}

EAPI void
evas_map_free(Evas_Map *m)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   _evas_map_free(NULL, m);
}

EAPI int
evas_map_count_get(const Evas_Map *m)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return -1;
   MAGIC_CHECK_END();

   return m->count;
}

EAPI void
evas_map_point_coord_set(Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p;

   if (idx >= m->count) return;
   p = m->points + idx;
   p->x = p->px = x;
   p->y = p->py = y;
   p->z = z;
}

EAPI void
evas_map_point_coord_get(const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   goto error;
   MAGIC_CHECK_END();

   const Evas_Map_Point *p;

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

EAPI void
evas_map_point_image_uv_set(Evas_Map *m, int idx, double u, double v)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p;

   if (idx >= m->count) return;
   p = m->points + idx;
   p->u = u;
   p->v = v;
}

EAPI void
evas_map_point_image_uv_get(const Evas_Map *m, int idx, double *u, double *v)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   goto error;
   MAGIC_CHECK_END();

   const Evas_Map_Point *p;

   if (idx >= m->count) goto error;
   p = m->points + idx;
   if (u) *u = p->u;
   if (v) *v = p->v;
   return;

 error:
   if (u) *u = 0.0;
   if (v) *v = 0.0;
}

EAPI void
evas_map_point_color_set(Evas_Map *m, int idx, int r, int g, int b, int a)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p;

   if (idx >= m->count) return;
   p = m->points + idx;
   p->r = r;
   p->g = g;
   p->b = b;
   p->a = a;
}

EAPI void
evas_map_point_color_get(const Evas_Map *m, int idx, int *r, int *g, int *b, int *a)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   const Evas_Map_Point *p;

   if (idx >= m->count) return;
   p = m->points + idx;
   if (r) *r = p->r;
   if (g) *g = p->g;
   if (b) *b = p->b;
   if (a) *a = p->a;
}

EAPI void
evas_map_util_points_populate_from_object_full(Evas_Map *m, const Evas_Object *obj, Evas_Coord z)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (m->count != 4)
     {
        ERR("map has count=%d where 4 was expected.", m->count);
        return;
     }
   _evas_map_util_points_populate(m, obj->cur.geometry.x, obj->cur.geometry.y,
                                  obj->cur.geometry.w, obj->cur.geometry.h, z);
}

EAPI void
evas_map_util_points_populate_from_object(Evas_Map *m, const Evas_Object *obj)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (m->count != 4)
     {
        ERR("map has count=%d where 4 was expected.", m->count);
        return;
     }
   _evas_map_util_points_populate(m, obj->cur.geometry.x, obj->cur.geometry.y,
                                  obj->cur.geometry.w, obj->cur.geometry.h, 0);
}

EAPI void
evas_map_util_points_populate_from_geometry(Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord z)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   if (m->count != 4)
     {
        ERR("map has count=%d where 4 was expected.", m->count);
        return;
     }
   _evas_map_util_points_populate(m, x, y, w, h, z);
}

EAPI void
evas_map_util_points_color_set(Evas_Map *m, int r, int g, int b, int a)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p, *p_end;

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

EAPI void
evas_map_util_rotate(Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   double r = (degrees * M_PI) / 180.0;
   Evas_Map_Point *p, *p_end;

   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        double x, y, xx, yy;

        x = p->x - cx;
        y = p->y - cy;

        xx = x * cos(r);
        yy = x * sin(r);
        x = xx - (y * sin(r));
        y = yy + (y * cos(r));

        p->px = p->x = x + cx;
        p->py = p->y = y + cy;
     }
}

EAPI void
evas_map_util_zoom(Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p, *p_end;

   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        double x, y;

        x = p->x - cx;
        y = p->y - cy;

        x = (((double)x) * zoomx);
        y = (((double)y) * zoomy);

        p->px = p->x = x + cx;
        p->py = p->y = y + cy;
     }
}

EAPI void
evas_map_util_3d_rotate(Evas_Map *m, double dx, double dy, double dz,
                        Evas_Coord cx, Evas_Coord cy, Evas_Coord cz)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   double rz = (dz * M_PI) / 180.0;
   double rx = (dx * M_PI) / 180.0;
   double ry = (dy * M_PI) / 180.0;
   Evas_Map_Point *p, *p_end;

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
             x = xx - (y * sin(rz));
             y = yy + (y * cos(rz));
          }

        if (ry != 0.0)
          {
             xx = x * cos(ry);
             zz = x * sin(ry);
             x = xx - (z * sin(ry));
             z = zz + (z * cos(ry));
          }

        if (rx != 0.0)
          {
             zz = z * cos(rx);
             yy = z * sin(rx);
             z = zz - (y * sin(rx));
             y = yy + (y * cos(rx));
          }

        p->px = p->x = x + cx;
        p->py = p->y = y + cy;
        p->z = z + cz;
     }
}

EAPI void
evas_map_util_quat_rotate(Evas_Map *m, double qx, double qy, double qz,
                          double qw, double cx, double cy, double cz)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p, *p_end;

   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
       double x, y, z, uvx, uvy, uvz, uuvx, uuvy, uuvz;

       x = p->x - cx;
       y = p->y - cy;
       z = p->z - cz;

       uvx = qy * z - qz * y;
       uvy = qz * x - qx * z;
       uvz = qx * y - qy * x;

       uuvx = qy * uvz - qz * uvy;
       uuvy = qz * uvx - qx * uvz;
       uuvz = qx * uvy - qy * uvx;

       uvx *= (2.0f * qw);
       uvy *= (2.0f * qw);
       uvz *= (2.0f * qw);

       uuvx *= 2.0f;
       uuvy *= 2.0f;
       uuvz *= 2.0f;

       p->px = p->x = cx + x + uvx + uuvx;
       p->py = p->y = cy + y + uvy + uuvy;
       p->z = cz + z + uvz + uuvz;
     }
}

EAPI void
evas_map_util_3d_lighting(Evas_Map *m,
                          Evas_Coord lx, Evas_Coord ly, Evas_Coord lz,
                          int lr, int lg, int lb, int ar, int ag, int ab)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   int i;

   for (i = 0; i < m->count; i++)
     {
        double x, y, z;
        double nx, ny, nz, x1, yy1, z1, x2, yy2, z2, ln, br;
        int h, j, mr, mg, mb;

        x = m->points[i].x;
        y = m->points[i].y;
        z = m->points[i].z;
        // calc normal
        h = (i - 1 + 4) % 4 + (i & ~0x3); // prev point
        j = (i + 1)     % 4 + (i & ~0x3); // next point

        x1 = m->points[h].x - x;
        yy1 = m->points[h].y - y;
        z1 = m->points[h].z - z;

        x2 = m->points[j].x - x;
        yy2 = m->points[j].y - y;
        z2 = m->points[j].z - z;
        nx = (yy1 * z2) - (z1 * yy2);
        ny = (z1 * x2) - (x1 * z2);
        nz = (x1 * yy2) - (yy1 * x2);

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
        if (m->points[i].a != 255)
          {
             mr = (mr * m->points[i].a) / 255;
             mg = (mg * m->points[i].a) / 255;
             mb = (mb * m->points[i].a) / 255;
          }
        m->points[i].r = (m->points[i].r * mr) / 255;
        m->points[i].g = (m->points[i].g * mg) / 255;
        m->points[i].b = (m->points[i].b * mb) / 255;
     }
}

EAPI void
evas_map_util_3d_perspective(Evas_Map *m,
                             Evas_Coord px, Evas_Coord py,
                             Evas_Coord z0, Evas_Coord foc)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return;
   MAGIC_CHECK_END();

   Evas_Map_Point *p, *p_end;

   p = m->points;
   p_end = p + m->count;

   m->persp.px = px;
   m->persp.py = py;
   m->persp.z0 = z0;
   m->persp.foc = foc;

   if (foc <= 0) return;

   for (; p < p_end; p++)
     {
        double x, y, zz;

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

EAPI Eina_Bool
evas_map_util_clockwise_get(Evas_Map *m)
{
   MAGIC_CHECK(m, Evas_Map, MAGIC_MAP);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   int i, j, k, count;
   long long c;

   if (m->count < 3) return EINA_FALSE;

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
   if (count > 0) return EINA_TRUE;
   return EINA_FALSE;
}

void
evas_object_map_update(Evas_Object *obj,
                       int x, int y,
                       int imagew, int imageh,
                       int uvw, int uvh)
{
   const Evas_Map_Point *p, *p_end;
   RGBA_Map_Point *pts, *pt;

   if (obj->spans)
     {
        if (obj->spans->x != x || obj->spans->y != y ||
            obj->spans->image.w != imagew || obj->spans->image.h != imageh ||
            obj->spans->uv.w != uvw || obj->spans->uv.h != uvh)
          obj->changed_map = EINA_TRUE;
     }
   else
     {
        obj->changed_map = EINA_TRUE;
     }

   if (!obj->changed_map) return ;

   if (obj->cur.map && obj->spans && obj->cur.map->count != obj->spans->count)
     {
        if (obj->spans)
          {
             // Destroy engine side spans
             free(obj->spans);
          }
        obj->spans = NULL;
     }

   if (!obj->spans)
     obj->spans = calloc(1, sizeof (RGBA_Map) +
                         sizeof (RGBA_Map_Point) * (obj->cur.map->count - 1));

   if (!obj->spans) return ;

   obj->spans->count = obj->cur.map->count;
   obj->spans->x = x;
   obj->spans->y = y;
   obj->spans->uv.w = uvw;
   obj->spans->uv.h = uvh;
   obj->spans->image.w = imagew;
   obj->spans->image.h = imageh;

   pts = obj->spans->pts;

   p = obj->cur.map->points;
   p_end = p + obj->cur.map->count;
   pt = pts;
             
   pts[0].px = obj->cur.map->persp.px << FP;
   pts[0].py = obj->cur.map->persp.py << FP;
   pts[0].foc = obj->cur.map->persp.foc << FP;
   pts[0].z0 = obj->cur.map->persp.z0 << FP;
   // draw geom +x +y
   for (; p < p_end; p++, pt++)
     {
        pt->x = (lround(p->x) + x) * FP1;
        pt->y = (lround(p->y) + y) * FP1;
        pt->z = (lround(p->z)    ) * FP1;
        pt->fx = p->px;
        pt->fy = p->py;
        pt->fz = p->z;
        pt->u = ((lround(p->u) * imagew) / uvw) * FP1;
        pt->v = ((lround(p->v) * imageh) / uvh) * FP1;
        if      (pt->u < 0) pt->u = 0;
        else if (pt->u > (imagew * FP1)) pt->u = (imagew * FP1);
        if      (pt->v < 0) pt->v = 0;
        else if (pt->v > (imageh * FP1)) pt->v = (imageh * FP1);
        pt->col = ARGB_JOIN(p->a, p->r, p->g, p->b);
     }
   if (obj->cur.map->count & 0x1)
     {
        pts[obj->cur.map->count] = pts[obj->cur.map->count -1];
     }

   // Request engine to update it's point
}
