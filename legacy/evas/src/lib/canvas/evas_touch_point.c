#include "evas_common.h"
#include "evas_private.h"

void
_evas_touch_point_append(Evas *e, int id, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord_Touch_Point *point;

   /* create new Evas_Coord_Touch_Point */
   point = (Evas_Coord_Touch_Point *)calloc(1, sizeof(Evas_Coord_Touch_Point));
   point->x = x;
   point->y = y;
   point->id = id;
   point->state = EVAS_TOUCH_POINT_DOWN;
   e->touch_points = eina_list_append(e->touch_points, point);
}

void
_evas_touch_point_update(Evas *e, int id, Evas_Coord x, Evas_Coord y, Evas_Touch_Point_State state)
{
   Eina_List *l;
   Evas_Coord_Touch_Point *point = NULL;

   EINA_LIST_FOREACH(e->touch_points, l, point)
     {
        if (point->id == id)
          {
             point->x = x;
             point->y = y;
             point->state = state;
             break;
          }
     }
}

void
_evas_touch_point_remove(Evas *e, int id)
{
   Eina_List *l;
   Evas_Coord_Touch_Point *point = NULL;

   EINA_LIST_FOREACH(e->touch_points, l, point)
     {
        if (point->id == id)
          {
             e->touch_points = eina_list_remove(e->touch_points, point);
             free(point);
             break;
          }
     }
}

EAPI unsigned int
evas_touch_point_list_count(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return eina_list_count(e->touch_points);
}

EAPI void
evas_touch_point_list_nth_xy_get(Evas *e, unsigned int n, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord_Touch_Point *point = NULL;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();

   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        return;
     }
   if (x) *x = point->x;
   if (y) *y = point->y;
}

EAPI int
evas_touch_point_list_nth_id_get(Evas *e, unsigned int n)
{
   Evas_Coord_Touch_Point *point = NULL;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return -1;
   MAGIC_CHECK_END();

   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point) return -1;
   return point->id;
}

EAPI Evas_Touch_Point_State
evas_touch_point_list_nth_state_get(Evas *e, unsigned int n)
{
   Evas_Coord_Touch_Point *point = NULL;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return EVAS_TOUCH_POINT_CANCEL;
   MAGIC_CHECK_END();

   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point) return EVAS_TOUCH_POINT_CANCEL;
   return point->state;
}
