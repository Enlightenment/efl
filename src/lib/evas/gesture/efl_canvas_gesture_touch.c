#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_TOUCH_CLASS

//This event object accumulates all the touch points
//that are directed to a particular object from the
//first finger down to the last finger up

static void _hash_free_cb(Pointer_Data *point)
{
   free(point);
}

static inline void
_touch_points_reset(Efl_Canvas_Gesture_Touch_Data *pd)
{
   eina_hash_free(pd->touch_points);
   pd->touch_points = eina_hash_int32_new(EINA_FREE_CB(_hash_free_cb));
   pd->touch_down = 0;
   pd->state = EFL_GESTURE_TOUCH_UNKNOWN;
}

EOLIAN static Efl_Object *
_efl_canvas_gesture_touch_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Touch_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   _touch_points_reset(pd);
   return obj;
}

EOLIAN static void
_efl_canvas_gesture_touch_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Touch_Data *pd)
{
   eina_hash_free(pd->touch_points);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Gesture_Touch_State
_efl_canvas_gesture_touch_state_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   return pd->state;
}

EOLIAN static void
_efl_canvas_gesture_touch_point_record(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd,
                                int id, Eina_Vector2 pos, unsigned int timestamp, Efl_Pointer_Action action)
{
   Pointer_Data *point = eina_hash_find(pd->touch_points, &id);
   Eina_Position2D _pos = { pos.x, pos.y };

   if (action == EFL_POINTER_ACTION_DOWN)
     {
        pd->touch_down++;
        //TODO: Need to handle 2 or more case.
        if (pd->touch_down == 2)
          pd->multi_touch = EINA_TRUE;
     }
   else if ((action == EFL_POINTER_ACTION_UP) ||
            (action == EFL_POINTER_ACTION_CANCEL))
     {
        pd->touch_down--;
        if (pd->multi_touch && pd->touch_down == 1)
          pd->multi_touch = EINA_FALSE;
     }

   if (pd->touch_down < 0) goto finished_touch;

   if (point)
     {
        //The point already exists. update the cur and prev point
        point->prev = point->cur;
        point->cur.pos = _pos;
        point->cur.timestamp = timestamp;
     }
   else
     {
        //New finger
        if (!id && (action != EFL_POINTER_ACTION_DOWN))
          {
             //Discard any other event
             return;
          }
        point = calloc(1, sizeof(Pointer_Data));
        if (!point) return;
        point->start.pos = point->prev.pos = point->cur.pos = _pos;
        point->start.timestamp = point->prev.timestamp = point->cur.timestamp = timestamp;
        point->id = id;

        //Add to the hash
        eina_hash_add(pd->touch_points, &id, point);
        //FIXME: finger_list was broken
        if (id)
          pd->multi_touch = EINA_TRUE;
     }
   point->action = action;

   if (!id && (action ==  EFL_POINTER_ACTION_DOWN))
     {
        pd->state = EFL_GESTURE_TOUCH_BEGIN;
     }
   else if (action ==  EFL_POINTER_ACTION_UP)
     {
        pd->state = EFL_GESTURE_TOUCH_END;
     }
   else
     {
        pd->state = EFL_GESTURE_TOUCH_UPDATE;
     }
   return;

finished_touch:
   _touch_points_reset(pd);
}

EOLIAN static Eina_Bool
_efl_canvas_gesture_touch_multi_touch_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   return pd->multi_touch;
}

EOLIAN static Eina_Position2D
_efl_canvas_gesture_touch_start_point_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   int tool = 0;
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);
   Eina_Position2D vec = { 0, 0 };

   if (!point)
     return vec;

   return point->start.pos;
}

EOLIAN static Eina_Position2D
_efl_canvas_gesture_touch_cur_point_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   int tool = 0;
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);
   Eina_Position2D vec = { 0, 0 };

   if (!point)
     return vec;

   return point->cur.pos;
}

EOLIAN static unsigned int
_efl_canvas_gesture_touch_cur_timestamp_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   int tool = 0;
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);

   if (!point)
     return 0;

   return point->cur.timestamp;
}

EOLIAN static Eina_Vector2
_efl_canvas_gesture_touch_delta(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd, int tool)
{
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);
   Eina_Vector2 vec = { 0, 0 };
   Eina_Vector2 v1 = { point->cur.pos.x, point->cur.pos.y };
   Eina_Vector2 v2 = { point->prev.pos.x, point->prev.pos.y };

   if (!point)
     return vec;

   eina_vector2_subtract(&vec, &v1, &v2);
   return vec;
}

EOLIAN static Eina_Vector2
_efl_canvas_gesture_touch_distance(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd, int tool)
{
   Pointer_Data *point = eina_hash_find(pd->touch_points, &tool);
   Eina_Vector2 vec = { 0, 0 };
   Eina_Vector2 v1 = { point->cur.pos.x, point->cur.pos.y };
   Eina_Vector2 v2 = { point->start.pos.x, point->start.pos.y };

   if (!point)
     return vec;

   eina_vector2_subtract(&vec, &v1, &v2);
   return vec;
}

#include "efl_canvas_gesture_touch.eo.c"
