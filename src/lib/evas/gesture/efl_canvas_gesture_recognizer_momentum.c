#include "efl_canvas_gesture_private.h"

#define MY_CLASS                       EFL_CANVAS_GESTURE_RECOGNIZER_MOMENTUM_CLASS

#define MOMENTUM_TIMEOUT               50
#define THUMBSCROLL_FRICTION           0.95
#define THUMBSCROLL_MOMENTUM_THRESHOLD 100.0
#define EFL_GESTURE_MINIMUM_MOMENTUM   0.001

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_momentum_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Momentum_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_MOMENTUM_CLASS;
}

static void
_momentum_set(Eo *obj,
              Efl_Canvas_Gesture_Momentum_Data *md,
              Eina_Position2D v1,
              Eina_Position2D v2,
              unsigned int t1,
              unsigned int t2)
{
   Evas_Coord velx = 0, vely = 0, vel;
   Evas_Coord dx = v2.x - v1.x;
   Evas_Coord dy = v2.y - v1.y;
   int dt = t2 - t1;
   Eina_Value *tf, *tmt;
   double thumbscroll_momentum_friction, thumbscroll_momentum_threshold;

   if (dt > 0)
     {
        velx = (dx * 1000) / dt;
        vely = (dy * 1000) / dt;
     }

   vel = sqrt((velx * velx) + (vely * vely));

   tf = _recognizer_config_get(obj, "thumbscroll_momentum_friction");
   if (tf) eina_value_get(tf, &thumbscroll_momentum_friction);
   else thumbscroll_momentum_friction = THUMBSCROLL_FRICTION;

   tmt = _recognizer_config_get(obj, "thumbscroll_momentum_threshold");
   if (tmt) eina_value_get(tmt, &thumbscroll_momentum_threshold);
   else thumbscroll_momentum_threshold = THUMBSCROLL_MOMENTUM_THRESHOLD;

   if ((thumbscroll_momentum_friction > 0.0) &&
       (vel > thumbscroll_momentum_threshold)) /* report
                                                * momentum */
     {
        md->momentum.x = velx;
        md->momentum.y = vely;
     }
   else
     {
        md->momentum.x = 0;
        md->momentum.y = 0;
     }
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_momentum_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                                Efl_Canvas_Gesture_Recognizer_Momentum_Data *pd,
                                                                                Efl_Canvas_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                                                Efl_Canvas_Gesture_Touch *event)
{
   Eina_Value *val;
   unsigned char glayer_continues_enable;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   Efl_Canvas_Gesture_Momentum_Data *md = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_MOMENTUM_CLASS);

   val = _recognizer_config_get(obj, "glayer_continues_enable");
   if (val) eina_value_get(val, &glayer_continues_enable);
   else glayer_continues_enable = 1;

   //Check the touched to ignore very first event.
   //It does not have any meanging of this gesture.
   if (glayer_continues_enable && !pd->touched)
     {
        if (efl_gesture_touch_state_get(event) != EFL_GESTURE_TOUCH_STATE_END)
          {
             /* guard against successive multi-touch cancels */
             if (efl_gesture_touch_points_count_get(event) == 1)
               {
                  pd->touched = EINA_TRUE;
                  rd->continues = EINA_TRUE;
                  md->id = -1;
               }
          }

        return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
     }
   if (pd->touched && (efl_gesture_touch_current_data_get(event)->action == EFL_POINTER_ACTION_DOWN))
     {
        /* a second finger was pressed at the same time-ish as the first: combine into same event */
        if (efl_gesture_touch_points_count_get(event) > 1)
          {
             if (efl_gesture_touch_current_timestamp_get(event) - efl_gesture_touch_previous_data_get(event)->cur.timestamp < TAP_TOUCH_TIME_THRESHOLD)
               return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
          }
        else if (efl_gesture_touch_current_timestamp_get(event) - efl_gesture_timestamp_get(gesture) < TAP_TOUCH_TIME_THRESHOLD)
          return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
     }
   if (pd->t_st && (md->id != -1) && (md->id != efl_gesture_touch_current_data_get(event)->id))
     {
        int xdir, ydir;
        const Efl_Gesture_Touch_Point_Data *data = efl_gesture_touch_current_data_get(event);
        xdir = _direction_get(data->prev.pos.x, data->cur.pos.x);
        ydir = _direction_get(data->prev.pos.y, data->cur.pos.y);
        if ((xdir != pd->xdir) || (ydir != pd->ydir))
          {
             memset(pd, 0, sizeof(Efl_Canvas_Gesture_Recognizer_Momentum_Data));
             rd->continues = EINA_FALSE;
             return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
          }
        return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
     }

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
      {
         if (!pd->t_st)
           {
              if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_STATE_BEGIN ||
                  glayer_continues_enable)
                {

                   pd->t_st = pd->t_end = efl_gesture_touch_current_timestamp_get(event);

                   pd->st_line = pd->end_line =
                       efl_gesture_touch_start_point_get(event);

                   efl_gesture_hotspot_set(gesture, pd->st_line);
                   md->id = efl_gesture_touch_current_data_get(event)->id;
                   if (efl_gesture_touch_previous_data_get(event))
                     {
                        /* if multiple fingers are pressed simultaneously, start tracking the latest finger for gesture */
                        if (efl_gesture_touch_previous_data_get(event)->action == efl_gesture_touch_current_data_get(event)->action)
                          return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
                     }
                   return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
                }
           }

         if ((efl_gesture_touch_current_timestamp_get(event) - MOMENTUM_TIMEOUT) >
             pd->t_end)
           {
              pd->st_line = efl_gesture_touch_current_point_get(event);
              pd->t_st = efl_gesture_touch_current_timestamp_get(event);
              pd->xdir = pd->ydir = 0;
           }
         else
           {
              int xdir, ydir;
              Eina_Position2D cur_p = efl_gesture_touch_current_point_get(event);

              xdir = _direction_get(pd->end_line.x, cur_p.x);
              ydir = _direction_get(pd->end_line.y, cur_p.y);

              if (xdir && (xdir != pd->xdir))
                {
                   pd->st_line.x = pd->end_line.x;
                   pd->t_st = pd->t_end;
                   pd->xdir = xdir;
                }

              if (ydir && (ydir != pd->ydir))
                {
                   pd->st_line.y = pd->end_line.y;
                   pd->t_st = pd->t_end;
                   pd->ydir = ydir;
                }
           }

         pd->end_line = efl_gesture_touch_current_point_get(event);
         pd->t_end = efl_gesture_touch_current_timestamp_get(event);
         efl_gesture_hotspot_set(gesture, pd->end_line);

         _momentum_set(obj, md, pd->st_line, efl_gesture_touch_current_point_get(event),
                       pd->t_st, efl_gesture_touch_current_timestamp_get(event));

         result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

         break;
      }

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         Eina_Bool touched = !!efl_gesture_touch_points_count_get(event);
         if (!pd->t_st)
           {
              Eina_Bool prev_touched = pd->touched;

              rd->continues = pd->touched = touched;

              if (prev_touched)
                return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
              return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
           }

         if ((efl_gesture_touch_current_timestamp_get(event) - MOMENTUM_TIMEOUT) > pd->t_end)
           {
              pd->st_line = efl_gesture_touch_current_point_get(event);
              pd->t_st = efl_gesture_touch_current_timestamp_get(event);
              pd->xdir = pd->ydir = 0;
           }

         pd->end_line = efl_gesture_touch_current_point_get(event);
         pd->t_end = efl_gesture_touch_current_timestamp_get(event);
         rd->continues = touched;
         efl_gesture_hotspot_set(gesture, pd->end_line);

         if ((fabs(md->momentum.x) > EFL_GESTURE_MINIMUM_MOMENTUM) ||
             (fabs(md->momentum.y) > EFL_GESTURE_MINIMUM_MOMENTUM))
           result = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
         else
           result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;

         memset(pd, 0, sizeof(Efl_Canvas_Gesture_Recognizer_Momentum_Data));
         pd->touched = touched;

         break;
      }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_momentum.eo.c"
