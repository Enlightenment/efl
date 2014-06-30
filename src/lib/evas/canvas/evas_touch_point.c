#include "evas_common_private.h"
#include "evas_private.h"

void
_evas_touch_point_append(Evas *eo_e, int id, Evas_Coord x, Evas_Coord y)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
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
_evas_touch_point_update(Evas *eo_e, int id, Evas_Coord x, Evas_Coord y, Evas_Touch_Point_State state)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
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
_evas_touch_point_remove(Evas *eo_e, int id)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
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

EOLIAN unsigned int
_evas_canvas_touch_point_list_count(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return eina_list_count(e->touch_points);
}

EOLIAN void
_evas_canvas_touch_point_list_nth_xy_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, unsigned int n, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord_Touch_Point *point = NULL;

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

EOLIAN int
_evas_canvas_touch_point_list_nth_id_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, unsigned int n)
{
   Evas_Coord_Touch_Point *point = NULL;

   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point) return -1;
   else return point->id;
}

EOLIAN Evas_Touch_Point_State
_evas_canvas_touch_point_list_nth_state_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, unsigned int n)
{
   Evas_Coord_Touch_Point *point = NULL;

   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point) return EVAS_TOUCH_POINT_CANCEL;
   else return point->state;
}

