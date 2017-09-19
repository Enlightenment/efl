#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_TOUCH_CLASS

typedef struct _Pointer_Data
{
   struct
     {
        double x;
        double y;
        double timestamp;
     } start, prev, cur ;
   Efl_Pointer_Action action;
}Pointer_Data;

typedef struct _Efl_Gesture_Touch_Data
{
   Efl_Gesture_Touch_State   type;
   Eina_Hash              *touch_points;
   Eina_List              *finger_list;
   Eina_Bool               multi_touch;
   Eo                     *target;
} Efl_Gesture_Touch_Data;


// This event object accumulates all the touch points
// that are directed to a particular object from the
// first finger down to the last finger up

static void _hash_free_cb(Pointer_Data *point)
{
   free(point);
}

EOLIAN static Efl_Object *
_efl_gesture_touch_efl_object_constructor(Eo *obj, Efl_Gesture_Touch_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->touch_points = eina_hash_int32_new(EINA_FREE_CB(_hash_free_cb));

   return obj;
}

EOLIAN static void
_efl_gesture_touch_efl_object_destructor(Eo *obj, Efl_Gesture_Touch_Data *pd)
{
   eina_hash_free(pd->touch_points);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Gesture_Touch_State
_efl_gesture_touch_state_get(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd)
{
   return pd->type;
}

EOLIAN static void
_efl_gesture_touch_point_record(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd,
                              int id, double x, double y, double timestamp, Efl_Pointer_Action action)
{
   Pointer_Data *point = eina_hash_find(pd->touch_points, &id);
   if (point)
     {
       // the point already exists. update the cur and prev point
       point->prev.x = point->cur.x;
       point->prev.y = point->cur.y;
       point->prev.timestamp = point->cur.timestamp;
       point->cur.x = x;
       point->cur.y = y;
       point->cur.timestamp = timestamp;
     }
   else
     {
        // new finger
        if (!id && (action != EFL_POINTER_ACTION_DOWN))
          {
             // discard any other event
             return;
          }
        point = calloc(1, sizeof(Pointer_Data));
        point->start.x = point->prev.x = point->cur.x = x;
        point->start.y = point->prev.y = point->cur.y = y;
        point->start.timestamp = point->prev.timestamp = point->cur.timestamp = timestamp;

        // add to the hash
        eina_hash_add(pd->touch_points, &id, point);
        pd->finger_list = eina_list_append(pd->finger_list, &id);
        if (id)
          pd->multi_touch = EINA_TRUE;
     }
   point->action = action;

   if (!id && (action ==  EFL_POINTER_ACTION_DOWN))
     {
        pd->type = EFL_GESTURE_TOUCH_BEGIN;
     }
   else if ((action ==  EFL_POINTER_ACTION_UP) && (eina_list_count(pd->finger_list) == 1))
     {
        pd->type = EFL_GESTURE_TOUCH_END;
     }
   else
     {
        pd->type = EFL_GESTURE_TOUCH_UPDATE;
     }

}

EOLIAN static Eina_Bool
_efl_gesture_touch_multi_touch_get(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd)
{
   return pd->multi_touch;
}

EOLIAN static void
_efl_gesture_touch_start_point(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd, double *x, double *y)
{
   int tool = 0;
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);

   *x = 0;
   *y = 0;

   if (point)
     {
        *x = point->start.x;
        *y = point->start.y;
     }
}

EOLIAN static void
_efl_gesture_touch_delta(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd, int tool, double *x, double *y)
{
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);

   *x = 0;
   *y = 0;

   if (point)
     {
        *x = point->cur.x - point->prev.x;
        *y = point->cur.y - point->prev.y;
     }
}

EOLIAN static void
_efl_gesture_touch_distance(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd, int tool, double *x, double *y)
{
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);

   *x = 0;
   *y = 0;

   if (point)
     {
        *x = point->cur.x - point->start.x;
        *y = point->cur.y - point->start.y;
     }
}

EOLIAN static const Eina_List *
_efl_gesture_touch_finger_list_get(Eo *obj EINA_UNUSED, Efl_Gesture_Touch_Data *pd)
{
   return pd->finger_list;
}

#include "efl_gesture_touch.eo.c"