#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

static void
_calc_map_geometry(Evas_Object *obj)
{
   int i;
   int is, was = 0, pass = 0;
   Evas_Coord x1, x2, y1, y2;
   
   if (!obj->cur.mappoints) return;
   x1 = obj->cur.mappoints[0].x;
   x2 = obj->cur.mappoints[0].x;
   y1 = obj->cur.mappoints[0].y;
   y2 = obj->cur.mappoints[0].y;
   for (i = 1; i < 4;  i++)
     {
        if (obj->cur.mappoints[i].x < x1) x1 = obj->cur.mappoints[i].x;
        if (obj->cur.mappoints[i].x > x2) x2 = obj->cur.mappoints[i].x;
        if (obj->cur.mappoints[i].y < y1) y1 = obj->cur.mappoints[i].y;
        if (obj->cur.mappoints[i].y > y2) y2 = obj->cur.mappoints[i].y;
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

EAPI void
evas_object_map_set(Evas_Object *obj, const Evas_Map_Point *points)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!points)
     {
        if (obj->cur.mappoints)
          {
             if (!obj->prev.mappoints)
               {
                  free(obj->cur.mappoints);
                  obj->cur.mappoints = NULL;
                  return;
               }
             obj->cur.mappoints = NULL;
          }
        return;
     }
   if (!obj->cur.mappoints)
     {
        obj->cur.mappoints = malloc(4 * sizeof(Evas_Map_Point));
        if (obj->cur.mappoints)
          {
             memcpy(obj->cur.mappoints, points, 4 * sizeof(Evas_Map_Point));
          }
        obj->prev.mappoints = NULL;
     }
   else
     {
        memcpy(obj->cur.mappoints, points, 4 * sizeof(Evas_Map_Point));
        obj->prev.mappoints = NULL;
     }
   if (obj->cur.usemap) _calc_map_geometry(obj);
}

EAPI void
evas_object_map_get(const Evas_Object *obj, Evas_Map_Point *points)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->cur.mappoints)
     {
        memcpy(points, obj->cur.mappoints, 4 * sizeof(Evas_Map_Point));
     }
}


EAPI Evas_Map_Point *
evas_map_new(int num)
{
   return calloc(num, sizeof(Evas_Map_Point));
}

EAPI void
evas_map_free(Evas_Map_Point *mp)
{
   if (!mp) return;
   free(mp);
}

EAPI void
evas_map_point_coord_set(Evas_Map_Point *mp, int n, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   if (!mp) return;
   mp[n].x = x;
   mp[n].y = y;
   mp[n].z = z;
}

EAPI void
evas_map_point_coord_get(const Evas_Map_Point *mp, int n, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z)
{
   if (!mp) return;
   if (x) *x = mp[n].x;
   if (y) *y = mp[n].y;
   if (z) *z = mp[n].z;
}

EAPI void
evas_map_point_image_uv_set(Evas_Map_Point *mp, int n, double u, double v)
{
   if (!mp) return;
   mp[n].u = u;
   mp[n].v = v;
}

EAPI void
evas_map_point_image_uv_get(const Evas_Map_Point *mp, int n, double *u, double *v)
{
   if (!mp) return;
   if (u) *u = mp[n].u;
   if (v) *v = mp[n].v;
}

EAPI void
evas_map_util_rotate(Evas_Map_Point *mp, double degrees, Evas_Coord cx, Evas_Coord cy)
{
   double r = (degrees * M_PI) / 180.0;
   int i;

   for (i = 0; i < 4;  i++)
     {
        Evas_Coord x, y, xx, yy;
        
        xx = x = mp[i].x - cx;
        yy = y = mp[i].y - cy;
        
        xx = (x * cos(r));
        yy = (x * sin(r));
        x = xx + (y * cos(r + (M_PI / 2.0)));
        y = yy + (y * sin(r + (M_PI / 2.0)));
        
        mp[i].x = x + cx;
        mp[i].y = y + cy;
     }
}

EAPI void
evas_map_util_zoom(Evas_Map_Point *mp, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy)
{
   int i;

   for (i = 0; i < 4;  i++)
     {
        Evas_Coord x, y;
        
        x = mp[i].x - cx;
        y = mp[i].y - cy;
        
        x = (((double)x) * zoomx);
        y = (((double)y) * zoomy);
        
        mp[i].x = x + cx;
        mp[i].y = y + cy;
     }
}
