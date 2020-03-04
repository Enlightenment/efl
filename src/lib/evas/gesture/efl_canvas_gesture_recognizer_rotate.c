#include "efl_canvas_gesture_private.h"

#define NEGATIVE_ANGLE   (-1.0) /* Magic number */

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_ROTATE_CLASS

static void
_reset_recognizer(Efl_Canvas_Gesture_Recognizer_Rotate_Data *pd)
{
   memset(&pd->rotate_st, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   memset(&pd->rotate_st1, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   memset(&pd->rotate_mv, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   memset(&pd->rotate_mv1, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   pd->prev_momentum = 0;
   pd->base_angle = 0;
   pd->next_step = pd->accum_momentum = 0;
}

#define memset do not use memset to reset rotate data, use _reset_recognizer


static void
_rotate_properties_get(Efl_Canvas_Gesture_Recognizer_Rotate_Data *pd,
                       Efl_Canvas_Gesture_Rotate_Data *gd,
                       Evas_Coord xx1,
                       Evas_Coord yy1,
                       Evas_Coord xx2,
                       Evas_Coord yy2,
                       double *angle, Eina_Bool started)
{
   /* FIXME: Fix momentum computation, it's wrong */
   double prev_angle = *angle;
   int x, y;
   gd->radius = _finger_gap_length_get(xx1, yy1, xx2, yy2, &x, &y) / 2;

   *angle = _angle_get(xx1, yy1, xx2, yy2);

   if (!started) /* Fingers are moving, compute momentum */
     {
        unsigned int tm_start =
          (pd->rotate_st.cur.timestamp > pd->rotate_st1.cur.timestamp)
          ?  pd->rotate_st.cur.timestamp : pd->rotate_st1.cur.timestamp;
        unsigned int tm_end =
          (pd->rotate_mv.cur.timestamp > pd->rotate_mv1.cur.timestamp)
          ? pd->rotate_mv.cur.timestamp : pd->rotate_mv1.cur.timestamp;

        unsigned int tm_total = tm_end - tm_start;
        if (tm_total) /* Momentum computed as:
                         accumulated rotation angle (deg) divided by time */
          {
             double m = 0;
             if (((prev_angle < 90) && ((*angle) > 270)) ||
                 /* We circle passing ZERO point */
                 ((prev_angle > 270) && ((*angle) < 90)))
               {
                  prev_angle = (*angle);
               }
             else m = prev_angle - (*angle);

             pd->accum_momentum += m;

             if ((tm_end - pd->prev_momentum_tm) < 100)
               pd->prev_momentum += m;
             else
               {
                  if (fabs(pd->prev_momentum) < 0.002)
                    pd->accum_momentum = 0.0;  /* reset momentum */

                  pd->prev_momentum = 0.0; /* Start again    */
               }

             pd->prev_momentum_tm = tm_end;
             gd->momentum = (pd->accum_momentum * 1000) / tm_total;
          }
     }
   else
     gd->momentum = 0;
}

static Eina_Bool
_on_rotation_broke_tolerance(Efl_Canvas_Gesture_Recognizer_Rotate_Data *pd, Efl_Canvas_Gesture_Rotate_Data *gd)
{
   if (pd->base_angle < 0)
     return EINA_FALSE;  /* Angle has to be computed first */

   if (pd->rotate_angular_tolerance < 0)
     return EINA_TRUE;

   double low = pd->base_angle - pd->rotate_angular_tolerance;
   double high = pd->base_angle + pd->rotate_angular_tolerance;
   double t = gd->angle;

   if (low < 0)
     {
        low += 180;
        high += 180;

        if (t < 180)
          t += 180;
        else
          t -= 180;
     }

   if (high > 360)
     {
        low -= 180;
        high -= 180;

        if (t < 180)
          t += 180;
        else
          t -= 180;
     }

   if ((t < low) || (t > high)) /* This marks that rotation action has
                                 * started */
     {
        pd->rotate_angular_tolerance = NEGATIVE_ANGLE;
        pd->base_angle = gd->angle; /* Avoid jump in angle value */
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_rotate_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Rotate_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_ROTATE_CLASS;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_rotate_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                            Efl_Canvas_Gesture_Recognizer_Rotate_Data *pd,
                                                                            Efl_Canvas_Gesture *gesture,
                                                                            Efl_Object *watched,
                                                                            Efl_Canvas_Gesture_Touch *event)
{
   Eina_Value *val;
   unsigned char glayer_continues_enable;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
   Efl_Canvas_Gesture_Rotate_Data *gd = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_ROTATE_CLASS);
   Efl_Canvas_Gesture_Touch_Data *td = efl_data_scope_get(event, EFL_CANVAS_GESTURE_TOUCH_CLASS);
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   const Efl_Gesture_Touch_Point_Data *p1 = efl_gesture_touch_data_get(event, 0);
   const Efl_Gesture_Touch_Point_Data *p2 = efl_gesture_touch_data_get(event, 1);

   //FIXME: Wheel rotate test first here.

   val = _recognizer_config_get(obj, "glayer_continues_enable");
   if (val) eina_value_get(val, &glayer_continues_enable);
   else glayer_continues_enable = 1;

   rd->continues = EINA_TRUE;

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
      {
         if ((!glayer_continues_enable) && (!pd->rotate_st.cur.timestamp))
           {
              return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
           }
         EINA_FALLTHROUGH;
      }

      case EFL_GESTURE_TOUCH_STATE_BEGIN:
      {
         if (td->touch_down > 2)
           {
              _reset_recognizer(pd);
              if (efl_gesture_state_get(gesture) == EFL_GESTURE_STATE_CANCELED)
                return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
              return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }
         if (td->touch_down == 1)
           {
              return EFL_GESTURE_RECOGNIZER_RESULT_MAYBE;
           }

         if (!pd->rotate_st.cur.timestamp)   /* Now scan touched-devices list
                                            * and find other finger */
           {
              if (!_event_multi_touch_get(event))
                return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
              pd->base_angle = NEGATIVE_ANGLE;
              val = _recognizer_config_get(obj, "glayer_rotate_angular_tolerance");
              if (val) eina_value_get(val, &pd->rotate_angular_tolerance);
              else pd->rotate_angular_tolerance = 2.0;

              memcpy(&pd->rotate_st, p2, sizeof(Efl_Gesture_Touch_Point_Data));
              memcpy(&pd->rotate_st1, p1, sizeof(Efl_Gesture_Touch_Point_Data));

              memcpy(&pd->rotate_mv, p2, sizeof(Efl_Gesture_Touch_Point_Data));
              memcpy(&pd->rotate_mv1, p1, sizeof(Efl_Gesture_Touch_Point_Data));

              _rotate_properties_get(pd, gd,
                                     pd->rotate_st.cur.pos.x, pd->rotate_st.cur.pos.y,
                                     pd->rotate_st1.cur.pos.x, pd->rotate_st1.cur.pos.y,
                                     &pd->base_angle, EINA_TRUE);
              if ((efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_STARTED) &&
                  (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_UPDATED))
                return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

              return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }

         if (p2->id == pd->rotate_mv.id)
           memcpy(&pd->rotate_mv, p2, sizeof(Efl_Gesture_Touch_Point_Data));
         else if (p2->id == pd->rotate_mv1.id)
           memcpy(&pd->rotate_mv1, p2, sizeof(Efl_Gesture_Touch_Point_Data));

         _rotate_properties_get(pd, gd,
                                pd->rotate_mv.cur.pos.x, pd->rotate_mv.cur.pos.y,
                                pd->rotate_mv1.cur.pos.x, pd->rotate_mv1.cur.pos.y,
                                &gd->angle, EINA_FALSE);

        if (_on_rotation_broke_tolerance(pd, gd)) /* Rotation broke
                                               * tolerance, report
                                               * move */
           {
              double d = gd->angle - pd->next_step;

              if (d < 0.0) d = (-d);

              if (d >= pd->rotate_step)
                {
                   pd->next_step = gd->angle;

                   return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
                }
           }

         return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
      }

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         /* no gesture was started, so no gesture should be detected */
         if ((td->touch_down == 0) || (!pd->rotate_st.cur.timestamp))
           {
              rd->continues = EINA_FALSE;

              _reset_recognizer(pd);
              efl_gesture_manager_recognizer_cleanup(efl_provider_find(obj, EFL_CANVAS_GESTURE_MANAGER_CLASS), obj, watched);

              return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
           }
         _reset_recognizer(pd);
         if (pd->rotate_angular_tolerance < 0)
           {
              return EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
           }

         if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE)
           {
              return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }
      }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_rotate.eo.c"
