#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_TOUCH_CLASS

//This event object accumulates all the touch points
//that are directed to a particular object from the
//first finger down to the last finger up

static inline void
_touch_points_reset(Efl_Canvas_Gesture_Touch_Data *pd)
{
   while (eina_array_count(pd->touch_points))
     free(eina_array_pop(pd->touch_points));
   pd->touch_down = 0;
   pd->prev_touch = pd->cur_touch = NULL;
   pd->state = EFL_GESTURE_TOUCH_STATE_UNKNOWN;
}

EOLIAN static Efl_Object *
_efl_canvas_gesture_touch_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Touch_Data *pd)
{
   pd->touch_points = eina_array_new(2);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->touch_points, NULL);
   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_gesture_touch_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Touch_Data *pd)
{
   while (eina_array_count(pd->touch_points))
     free(eina_array_pop(pd->touch_points));
   eina_array_free(pd->touch_points);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Gesture_Touch_State
_efl_canvas_gesture_touch_state_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   return pd->state;
}

EOLIAN static void
_efl_canvas_gesture_touch_point_record(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd, Efl_Input_Pointer *event)
{
   Efl_Input_Pointer_Data *pointer_data = efl_data_scope_get(event, EFL_INPUT_POINTER_CLASS);
   int id = pointer_data->touch_id;
   int timestamp = pointer_data->timestamp;
   Efl_Pointer_Action action = pointer_data->action;
   Eina_Vector2 pos = pointer_data->cur;

   Eina_Position2D _pos = { pos.x, pos.y };
   Efl_Gesture_Touch_Point_Data *point = NULL;

   if (eina_array_count(pd->touch_points) >= (unsigned int)id + 1)
     point = eina_array_data_get(pd->touch_points, id);

   if (action == EFL_POINTER_ACTION_DOWN)
     {
        if ((!point) || (!point->cur.pressed))
          pd->touch_down++;
     }
   else if ((action == EFL_POINTER_ACTION_UP) ||
            (action == EFL_POINTER_ACTION_CANCEL))
     {
        if (point && point->cur.pressed)
          pd->touch_down--;
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
        if (action != EFL_POINTER_ACTION_DOWN)
          {
             //Discard any other event
             return;
          }

        point = calloc(1, sizeof(Efl_Gesture_Touch_Point_Data));
        if (!point) return;
        point->start.pos = point->prev.pos = point->cur.pos = _pos;
        point->start.timestamp = point->prev.timestamp = point->cur.timestamp = timestamp;
        point->id = id;

        eina_array_push(pd->touch_points, point);
     }
   if (pd->cur_touch != point)
     pd->prev_touch = pd->cur_touch;
   pd->cur_touch = point;
   point->action = action;

   if (action == EFL_POINTER_ACTION_DOWN)
     {
        point->cur.pressed = EINA_TRUE;
        if (!id)
          pd->state = EFL_GESTURE_TOUCH_STATE_BEGIN;
        else
          pd->state = EFL_GESTURE_TOUCH_STATE_UPDATE;
     }
   else if (action == EFL_POINTER_ACTION_UP)
     {
        point->cur.pressed = EINA_FALSE;
        pd->state = EFL_GESTURE_TOUCH_STATE_END;
     }
   else
     {
        pd->state = EFL_GESTURE_TOUCH_STATE_UPDATE;
     }
   return;

finished_touch:
   _touch_points_reset(pd);
}

EOLIAN static unsigned int
_efl_canvas_gesture_touch_touch_points_count_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   return pd->touch_down;
}

EOLIAN static const Efl_Gesture_Touch_Point_Data *
_efl_canvas_gesture_touch_current_data_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
    return pd->cur_touch;
}

EOLIAN static const Efl_Gesture_Touch_Point_Data *
_efl_canvas_gesture_touch_previous_data_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
    return pd->prev_touch;
}

EOLIAN static const Efl_Gesture_Touch_Point_Data *
_efl_canvas_gesture_touch_data_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd, unsigned int id)
{
    return eina_array_count(pd->touch_points) >= id + 1 ? eina_array_data_get(pd->touch_points, id) : NULL;
}

EOLIAN static Eina_Position2D
_efl_canvas_gesture_touch_start_point_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd EINA_UNUSED)
{
   const Efl_Gesture_Touch_Point_Data *point = efl_gesture_touch_data_get(obj, 0);
   Eina_Position2D vec = { 0, 0 };

   if (!point)
     return vec;

   return point->start.pos;
}

EOLIAN static Eina_Position2D
_efl_canvas_gesture_touch_current_point_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   const Efl_Gesture_Touch_Point_Data *point = pd->cur_touch;
   Eina_Position2D vec = { 0, 0 };

   if (!point)
     return vec;

   return point->cur.pos;
}

EOLIAN static unsigned int
_efl_canvas_gesture_touch_current_timestamp_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd)
{
   const Efl_Gesture_Touch_Point_Data *point = pd->cur_touch;

   if (!point)
     return 0;

   return point->cur.timestamp;
}

EOLIAN static Eina_Vector2
_efl_canvas_gesture_touch_delta(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd, int tool)
{
   Efl_Gesture_Touch_Point_Data *point = eina_array_count(pd->touch_points) >= (unsigned)tool + 1 ? eina_array_data_get(pd->touch_points, tool) : NULL;
   Eina_Vector2 vec = { 0, 0 };

   EINA_SAFETY_ON_NULL_RETURN_VAL(point, vec);

   Eina_Vector2 v1 = { point->cur.pos.x, point->cur.pos.y };
   Eina_Vector2 v2 = { point->prev.pos.x, point->prev.pos.y };

   eina_vector2_subtract(&vec, &v1, &v2);
   return vec;
}

EOLIAN static Eina_Vector2
_efl_canvas_gesture_touch_distance(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Touch_Data *pd, int tool)
{
   Efl_Gesture_Touch_Point_Data *point = eina_array_count(pd->touch_points) >= (unsigned)tool + 1 ? eina_array_data_get(pd->touch_points, tool) : NULL;
   Eina_Vector2 vec = { 0, 0 };

   EINA_SAFETY_ON_NULL_RETURN_VAL(point, vec);

   Eina_Vector2 v1 = { point->cur.pos.x, point->cur.pos.y };
   Eina_Vector2 v2 = { point->start.pos.x, point->start.pos.y };

   eina_vector2_subtract(&vec, &v1, &v2);
   return vec;
}

#include "efl_canvas_gesture_touch.eo.c"
