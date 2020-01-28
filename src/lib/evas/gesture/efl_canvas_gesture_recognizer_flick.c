#include "efl_canvas_gesture_private.h"

#define MY_CLASS                       EFL_CANVAS_GESTURE_RECOGNIZER_FLICK_CLASS

#define MOMENTUM_TIMEOUT               50
#define THUMBSCROLL_FRICTION           0.95
#define THUMBSCROLL_MOMENTUM_THRESHOLD 100.0
#define EFL_GESTURE_MINIMUM_MOMENTUM   0.001

#define RAD2DEG(x) ((x) * 57.295779513)
#define DEG2RAD(x) ((x) / 57.295779513)

#define memset do not use memset to reset flick data, use _reset_recognizer

static void
_reset_recognizer(Efl_Canvas_Gesture_Recognizer_Flick_Data *pd)
{
   pd->st_line = EINA_POSITION2D(0, 0);
   pd->t_st = pd->t_end = 0;
   pd->line_length = 0;
   pd->line_angle = -1;
   pd->touched = EINA_FALSE;
}

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_flick_efl_canvas_gesture_recognizer_add(Eo *obj, Efl_Canvas_Gesture_Recognizer_Flick_Data *pd EINA_UNUSED, Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_FLICK_CLASS, obj);
}

static void
_momentum_set(Eo *obj,
              Efl_Canvas_Gesture_Flick_Data *fd,
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
   double thumbscroll_friction, thumbscroll_momentum_threshold;

   if (dt > 0)
     {
        velx = (dx * 1000) / dt;
        vely = (dy * 1000) / dt;
     }

   vel = sqrt((velx * velx) + (vely * vely));

   tf = _recognizer_config_get(obj, "thumbscroll_friction");
   if (tf) eina_value_get(tf, &thumbscroll_friction);
   else thumbscroll_friction = THUMBSCROLL_FRICTION;

   tmt = _recognizer_config_get(obj, "thumbscroll_momentum_threshold");
   if (tmt) eina_value_get(tmt, &thumbscroll_momentum_threshold);
   else thumbscroll_momentum_threshold = THUMBSCROLL_MOMENTUM_THRESHOLD;

   if ((thumbscroll_friction > 0.0) &&
       (vel > thumbscroll_momentum_threshold)) /* report
                                                * momentum */
     {
        fd->momentum.x = velx;
        fd->momentum.y = vely;
     }
   else
     {
        fd->momentum.x = 0;
        fd->momentum.y = 0;
     }
}

static void
_single_line_process(Eo *obj,
                     Efl_Canvas_Gesture_Recognizer_Flick_Data *pd,
                     Efl_Canvas_Gesture *gesture,
                     Efl_Canvas_Gesture_Flick_Data *fd,
                     Efl_Canvas_Gesture_Touch *event)
{
   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
        if (!pd->t_st)
          {
             pd->st_line = efl_gesture_touch_cur_point_get(event);
             pd->t_st = efl_gesture_touch_cur_timestamp_get(event);

             efl_gesture_hotspot_set(gesture, pd->st_line);

             return;
          }

        break;

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         if (!pd->t_st) return;

         pd->t_end = efl_gesture_touch_cur_timestamp_get(event);

         break;
      }

      default:

        return;
     }

   _momentum_set(obj, fd, pd->st_line, efl_gesture_touch_cur_point_get(event),
                 pd->t_st, efl_gesture_touch_cur_timestamp_get(event));
}

static double
_angle_get(Evas_Coord xx1,
           Evas_Coord yy1,
           Evas_Coord xx2,
           Evas_Coord yy2)
{
   double a, xx, yy, rt = (-1);

   xx = abs(xx2 - xx1);
   yy = abs(yy2 - yy1);

   if (((int)xx) && ((int)yy))
     {
        rt = a = RAD2DEG(atan(yy / xx));
        if (xx1 < xx2)
          {
             if (yy1 < yy2) rt = 360 - a;
             else rt = a;
          }
        else
          {
             if (yy1 < yy2) rt = 180 + a;
             else rt = 180 - a;
          }
     }

   if (rt < 0) /* Do this only if rt is not set */
     {
        if (((int)xx)) /* Horizontal line */
          {
             if (xx2 < xx1) rt = 180;
             else rt = 0.0;
          }
        else /* Vertical line */
          {
             if (yy2 < yy1) rt = 90;
             else rt = 270;
          }
     }

   /* Now we want to change from:
    *                      90                   0
    * original circle   180   0   We want:  270   90
    *                     270                 180
    */
   rt = 450 - rt;
   if (rt >= 360) rt -= 360;

   return rt;
}

static void
_vector_get(Eina_Position2D v1,
            Eina_Position2D v2,
            int *l,
            double *a)
{
   int xx, yy;

   xx = (int)(v2.x - v1.x);
   yy = (int)(v2.y - v1.y);
   *l = (int)sqrt((xx * xx) + (yy * yy));
   *a = _angle_get((int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y);
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_flick_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                             Efl_Canvas_Gesture_Recognizer_Flick_Data *pd,
                                                                             Efl_Canvas_Gesture *gesture, Efl_Object *watched,
                                                                             Efl_Canvas_Gesture_Touch *event)
{
   double angle, line_angle;
   Eina_Value *val;
   unsigned char glayer_continues_enable;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
   Eina_Bool touch_up = EINA_FALSE;
   int points = efl_gesture_touch_points_count_get(event);
   Efl_Canvas_Gesture_Flick_Data *fd = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_FLICK_CLASS);
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   Eina_Bool dead = EINA_FALSE;

   val = _recognizer_config_get(obj, "glayer_continues_enable");
   if (val) eina_value_get(val, &glayer_continues_enable);
   else glayer_continues_enable = 1;

   //We need to cover events that occur continuously in the mouse down state
   //without mouse up.
   //Recognizing the gesture again, even though it was canceled during gesture
   //recognition.
   if (efl_gesture_state_get(gesture) == EFL_GESTURE_STATE_CANCELED)
     efl_gesture_state_set(gesture, EFL_GESTURE_STATE_NONE);

   if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_STATE_END)
     touch_up = EINA_TRUE;

   //This is to handle a case with a mouse click on the target object.
   if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_STATE_END && !pd->touched)
     {
        efl_gesture_manager_gesture_clean_up(efl_provider_find(obj, EFL_CANVAS_GESTURE_MANAGER_CLASS), watched, EFL_EVENT_GESTURE_FLICK, obj);
        dead = EINA_TRUE;
     }

   if (glayer_continues_enable && !pd->touched)
     {
        pd->touched = EINA_TRUE;
        pd->line_angle = -1.0;
        rd->continues = EINA_TRUE;
        /* this has been deleted */
        if (!dead)
          fd->id = -1;

        return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
     }
   if (pd->touched && (efl_gesture_touch_cur_data_get(event)->action == EFL_POINTER_ACTION_DOWN))
     {
        /* a second finger was pressed at the same time-ish as the first: combine into same event */
        if (efl_gesture_touch_cur_timestamp_get(event) - efl_gesture_timestamp_get(gesture) < TAP_TOUCH_TIME_THRESHOLD)
          return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
     }
   if (pd->t_st && (points > 1) && (fd->id != efl_gesture_touch_cur_data_get(event)->id))
     {
        int xdir[2], ydir[2];
        const Efl_Gesture_Touch_Point_Data *data = efl_gesture_touch_cur_data_get(event);
        const Efl_Gesture_Touch_Point_Data *data2;

        if (fd->id == -1) return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
        data2 = efl_gesture_touch_data_get(event, fd->id);
        xdir[0] = _direction_get(data->prev.pos.x, data->cur.pos.x);
        ydir[0] = _direction_get(data->prev.pos.y, data->cur.pos.y);
        xdir[1] = _direction_get(data2->prev.pos.x, data2->cur.pos.x);
        ydir[1] = _direction_get(data2->prev.pos.y, data2->cur.pos.y);
        if ((xdir[0] != xdir[1]) || (ydir[0] != ydir[1]))
          {
             rd->continues = EINA_FALSE;
             _reset_recognizer(pd);
             return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
          }
        return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
     }

   _single_line_process(obj, pd, gesture, fd, event);
   _vector_get(pd->st_line, efl_gesture_touch_cur_point_get(event),
               &pd->line_length, &angle);

   line_angle = pd->line_angle;
   if (pd->t_st)
     {
        if ((line_angle > 0.0) || EINA_DBL_EQ(line_angle, 0.0))
          {
             double line_distance_tolerance, line_angular_tolerance;
             double a = fabs(angle - line_angle);
             double d = (tan(DEG2RAD(a))) * pd->line_length;

             val = _recognizer_config_get(obj, "glayer_line_distance_tolerance");
             if (val) eina_value_get(val, &line_distance_tolerance);
             else line_distance_tolerance = 3.0;

             line_distance_tolerance *= pd->finger_size;

             val = _recognizer_config_get(obj, "glayer_line_angular_tolerance");
             if (val) eina_value_get(val, &line_angular_tolerance);
             else line_angular_tolerance = 20.0;

             if ((d > line_distance_tolerance) ||
                 (a > line_angular_tolerance))
               {
                  _reset_recognizer(pd);

                  if (touch_up) rd->continues = EINA_FALSE;

                  return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
               }

             /* We may finish line if momentum is zero */
             if (glayer_continues_enable)
               {
                  /* This is for continues-gesture */
                  /* Finish line on zero momentum for continues gesture */
                  if ((!fd->momentum.x) && (!fd->momentum.y))
                    pd->t_end = efl_gesture_touch_cur_timestamp_get(event);
               }
          }
        else
          {
             double line_min_length;

             val = _recognizer_config_get(obj, "glayer_line_min_length");
             if (val) eina_value_get(val, &line_min_length);
             else line_min_length = 1.0;

             line_min_length *= pd->finger_size;

             if (pd->line_length >= line_min_length)
               line_angle = fd->angle = pd->line_angle = angle;
          }

        if (pd->t_end)
          {
             if (line_angle < 0.0)
               {
                  _reset_recognizer(pd);

                  if (touch_up) rd->continues = EINA_FALSE;

                  return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
               }
          }
     }

   unsigned int tm_end = efl_gesture_touch_cur_timestamp_get(event);
   if (pd->t_end)
     {
        if (pd->t_end < tm_end)
          tm_end = pd->t_end;
     }

   unsigned int time_limit_ms;
   val = _recognizer_config_get(obj, "glayer_flick_time_limit_ms");
   if (val) eina_value_get(val, &time_limit_ms);
   else time_limit_ms = 120;

   if ((tm_end - pd->t_st) > time_limit_ms)
     {
        _reset_recognizer(pd);

        if (touch_up) rd->continues = EINA_FALSE;

        return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
     }

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
        if (!glayer_continues_enable)
          fd->id = efl_gesture_touch_cur_data_get(event)->id;
        EINA_FALLTHROUGH;
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
      {
         if (pd->t_st)
           {
              if (glayer_continues_enable && pd->t_end)
                {
                   result = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
                }
              else
                {
                   result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
                }
           }
         break;
      }

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         if (!pd->t_st)
           {
              pd->touched = EINA_FALSE;
              rd->continues = EINA_FALSE;

              return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }
         if (pd->t_st && pd->t_end)
           {
              rd->continues = EINA_FALSE;

              result = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
           }

         efl_gesture_hotspot_set(gesture, efl_gesture_touch_cur_point_get(event));

         _reset_recognizer(pd);

         rd->continues = EINA_FALSE;

         break;
      }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_flick.eo.c"
