#include "evas_common.h"
#include "evas_private.h"

void
_evas_touch_point_append(Evas *eo_e, int id, Evas_Coord x, Evas_Coord y)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
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
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
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
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
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
evas_touch_point_list_count(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   unsigned int ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_touch_point_list_count(&ret));
   return ret;
}

void
_canvas_touch_point_list_count(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int *ret = va_arg(*list, unsigned int *);
   const Evas_Public_Data *e = _pd;
   *ret = eina_list_count(e->touch_points);
}

EAPI void
evas_touch_point_list_nth_xy_get(Evas *eo_e, unsigned int n, Evas_Coord *x, Evas_Coord *y)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_touch_point_list_nth_xy_get(n, x, y));
}

void
_canvas_touch_point_list_nth_xy_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int n = va_arg(*list, unsigned int);
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);

   Evas_Coord_Touch_Point *point = NULL;

   Evas_Public_Data *e = _pd;
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
evas_touch_point_list_nth_id_get(Evas *eo_e, unsigned int n)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return -1;
   MAGIC_CHECK_END();
   int ret = -1;
   eo_do(eo_e, evas_canvas_touch_point_list_nth_id_get(n, &ret));
   return ret;
}

void
_canvas_touch_point_list_nth_id_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int n = va_arg(*list, unsigned int);
   int *ret = va_arg(*list, int *);

   Evas_Coord_Touch_Point *point = NULL;

   Evas_Public_Data *e = _pd;
   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point) *ret = -1;
   else *ret = point->id;
}

EAPI Evas_Touch_Point_State
evas_touch_point_list_nth_state_get(Evas *eo_e, unsigned int n)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return EVAS_TOUCH_POINT_CANCEL;
   MAGIC_CHECK_END();
   Evas_Touch_Point_State ret = EVAS_TOUCH_POINT_CANCEL;
   eo_do(eo_e, evas_canvas_touch_point_list_nth_state_get(n, &ret));
   return ret;
}

void
_canvas_touch_point_list_nth_state_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int n = va_arg(*list, unsigned int);
   Evas_Touch_Point_State *ret = va_arg(*list, Evas_Touch_Point_State *);

   Evas_Coord_Touch_Point *point = NULL;

   Evas_Public_Data *e = _pd;
   point = (Evas_Coord_Touch_Point *)eina_list_nth(e->touch_points, n);
   if (!point) *ret = EVAS_TOUCH_POINT_CANCEL;
   else *ret = point->state;
}
