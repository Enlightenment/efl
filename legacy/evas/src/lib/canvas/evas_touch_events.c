#include "evas_common.h"
#include "evas_private.h"

static void
_evas_touch_point_append(Evas *e, int id, int x, int y)
{
   Evas_Coord_Touch_Point *point;

   /* create new Evas_Coord_Touch_Point */
   point = (Evas_Coord_Touch_Point *)calloc(1, sizeof(Evas_Coord_Touch_Point));
   point->x = x;
   point->y = y;
   point->id = id;
   point->state = EVAS_TOUCH_POINT_DOWN;

   _evas_walk(e);
   e->touch_points = eina_list_append(e->touch_points, point);
   _evas_unwalk(e);
}

static Evas_Coord_Touch_Point *
_evas_touch_point_update(Evas *e, int id, int x, int y, Evas_Touch_Point_State state)
{
   Eina_List *l;
   Evas_Coord_Touch_Point *point = NULL;

   _evas_walk(e);
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
   _evas_unwalk(e);

   return point;
}

static void
_evas_event_feed_touch(Evas *e, unsigned int timestamp, Evas_Event_Touch_Type type)
{
   Evas_Event_Touch ev;
   Eina_List *l, *copy;
   Evas_Coord_Touch_Point *point;
   Evas_Object *obj;
   const void *data;

   _evas_walk(e);
   /* set Evas_Event_Touch's members */
   ev.points = NULL;
   EINA_LIST_FOREACH(e->touch_points, l, point)
     ev.points = eina_list_append(ev.points, point);
   ev.modifiers = &(e->modifiers);
   ev.timestamp = timestamp;
   ev.type = type;

   /* make copy of object list */
   copy = NULL;
   EINA_LIST_FOREACH(e->pointer.object.in, l, data)
     copy = eina_list_append(copy, data);

   /* call all EVAS_CALLBACK_TOUCH's callbacks */
   EINA_LIST_FOREACH(copy, l, obj)
     {
        if (!obj->delete_me && (e->events_frozen <= 0))
           evas_object_event_callback_call(obj, EVAS_CALLBACK_TOUCH, &ev);
        if (e->delete_me) break;
     }
   if (copy) eina_list_free(copy);
   if (ev.points) eina_list_free(ev.points);

   /* if finger is released or pressed, reset all touch point's state */
   if (type != EVAS_EVENT_TOUCH_MOVE)
     {
        EINA_LIST_FOREACH(e->touch_points, l, point)
          point->state = EVAS_TOUCH_POINT_STILL;
     }
   _evas_unwalk(e);
}

void
_evas_event_touch_down(Evas *e, Evas_Coord x, Evas_Coord y, int id, unsigned int timestamp)
{
   _evas_touch_point_append(e, id, x, y);
   _evas_event_feed_touch(e, timestamp, EVAS_EVENT_TOUCH_BEGIN);
}

void
_evas_event_touch_up(Evas *e, Evas_Coord x, Evas_Coord y, int id, unsigned int timestamp)
{
   Evas_Coord_Touch_Point *point;

   /* update touch point in the touch_points list */
   point = _evas_touch_point_update(e, id, x, y, EVAS_TOUCH_POINT_UP);
   if (!point) return;

   _evas_walk(e);
   _evas_event_feed_touch(e, timestamp, EVAS_EVENT_TOUCH_END);
   e->touch_points = eina_list_remove(e->touch_points, point);
   _evas_unwalk(e);
}

void
_evas_event_touch_move(Evas *e, Evas_Coord x, Evas_Coord y, int id, unsigned int timestamp)
{
   Eina_List *l;
   Evas_Coord_Touch_Point *point;

   _evas_walk(e);
   EINA_LIST_FOREACH(e->touch_points, l, point)
     {
        if (point->id == id)
          {
             /* update touch point */
             point->x = x;
             point->y = y;
             point->state = EVAS_TOUCH_POINT_MOVE;

             _evas_event_feed_touch(e, timestamp, EVAS_EVENT_TOUCH_MOVE);
          }
     }
   _evas_unwalk(e);
}
