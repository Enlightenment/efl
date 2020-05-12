#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS

static void
_reset_recognizer(Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd)
{
   memset(&pd->zoom_st, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   memset(&pd->zoom_st1, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   memset(&pd->zoom_mv, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   memset(&pd->zoom_mv1, 0, sizeof(Efl_Gesture_Touch_Point_Data));
   pd->zoom_base = 0;
   pd->zoom_step = pd->next_step = pd->zoom_finger_factor = pd->zoom_distance_tolerance = 0;
   pd->calc_temp = EINA_FALSE;
}

#define memset do not use memset to reset zoom data, use _reset_recognizer

static double
_zoom_compute(Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd,
              Efl_Canvas_Gesture_Zoom_Data *zd,
              Evas_Coord xx1,
              Evas_Coord yy1,
              Evas_Coord xx2,
              Evas_Coord yy2,
              double zoom_finger_factor)
{
   double rt = 1.0, zoom_distance_tolerance = pd->zoom_distance_tolerance;
   //TODO: Enable below code if the zoom momentum is need
   //unsigned int tm_end = (pd->zoom_mv.cur.timestamp > pd->zoom_mv1.cur.timestamp) ?
   //   pd->zoom_mv.cur.timestamp : pd->zoom_mv1.cur.timestamp;

   int x, y; //Hot spot
   Evas_Coord diam = _finger_gap_length_get(xx1, yy1, xx2, yy2,
                                            &x, &y);

   zd->radius = diam / 2.0;

   if (!pd->zoom_base)
     {
        pd->zoom_base = diam;
        return zd->zoom;
     }

   if (EINA_DBL_NONZERO(zoom_distance_tolerance)) /* zoom tolerance <> ZERO, means
                                    * zoom action NOT started yet */
     {
        /* avoid jump with zoom value when break tolerance */
        if (diam < (pd->zoom_base - pd->zoom_distance_tolerance))
          {
             pd->zoom_base -= pd->zoom_distance_tolerance;
             pd->zoom_distance_tolerance = 0;
          }

        /* avoid jump with zoom value when break tolerance */
        if (diam > (pd->zoom_base + pd->zoom_distance_tolerance))
          {
             pd->zoom_base += pd->zoom_distance_tolerance;
             pd->zoom_distance_tolerance = 0;
          }

        return rt;
     }

   /* We use factor only on the difference between gap-base   */
   /* if gap=120, base=100, we get ((120-100)/100)=0.2*factor */
   rt = ((1.0) + ((((float)diam - (float)pd->zoom_base) /
                   (float)pd->zoom_base) * zoom_finger_factor));

   //TODO: Enable below code if the zoom momentum is need
   /* Momentum: zoom per second: */
   //zd->momentum = _zoom_momentum_get(st, tm_end, rt);

   return rt;
}

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_zoom_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_ZOOM_CLASS;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_zoom_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                            Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd,
                                                                            Efl_Canvas_Gesture *gesture,
                                                                            Efl_Object *watched,
                                                                            Efl_Canvas_Gesture_Touch *event)
{
   Eina_Value *val;
   unsigned char zoom_finger_enable;
   unsigned char glayer_continues_enable;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
   Efl_Canvas_Gesture_Zoom_Data *zd = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_ZOOM_CLASS);
   Efl_Canvas_Gesture_Touch_Data *td = efl_data_scope_get(event, EFL_CANVAS_GESTURE_TOUCH_CLASS);
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   double zoom_distance_tolerance = pd->zoom_distance_tolerance;

   //FIXME: Wheel zoom test first here.

   val = _recognizer_config_get(obj, "glayer_continues_enable");
   if (val) eina_value_get(val, &glayer_continues_enable);
   else glayer_continues_enable = 1;

   val = _recognizer_config_get(obj, "glayer_zoom_finger_enable");
   if (val) eina_value_get(val, &zoom_finger_enable);
   else zoom_finger_enable = 1;

   val = _recognizer_config_get(obj, "glayer_zoom_finger_factor");
   if (val) eina_value_get(val, &pd->zoom_finger_factor);
   else pd->zoom_finger_factor = 1.0;

   rd->continues = EINA_TRUE;

   if (!EINA_DBL_NONZERO(zoom_distance_tolerance) && !pd->calc_temp)
     {
        pd->calc_temp = EINA_TRUE;
        val = _recognizer_config_get(obj, "glayer_zoom_distance_tolerance");
        if (val) eina_value_get(val, &pd->zoom_distance_tolerance);
        else pd->zoom_distance_tolerance = 1.0;

        pd->zoom_distance_tolerance *= pd->finger_size;
     }

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
      {
         if ((!glayer_continues_enable) && (!pd->zoom_st.cur.timestamp))
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

         if (!pd->zoom_st.cur.timestamp)   /* Now scan touched-devices list
                                            * and find other finger */
           {
              if (!_event_multi_touch_get(event))
                return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;

              const Efl_Gesture_Touch_Point_Data *p1 = efl_gesture_touch_data_get(event, 0);
              const Efl_Gesture_Touch_Point_Data *p2 = efl_gesture_touch_data_get(event, 1);

              memcpy(&pd->zoom_st, p2, sizeof(Efl_Gesture_Touch_Point_Data));
              memcpy(&pd->zoom_st1, p1, sizeof(Efl_Gesture_Touch_Point_Data));

              memcpy(&pd->zoom_mv, p2, sizeof(Efl_Gesture_Touch_Point_Data));
              memcpy(&pd->zoom_mv1, p1, sizeof(Efl_Gesture_Touch_Point_Data));

              int x, y;  //Hot spot
              zd->zoom = 1.0;
              pd->zoom_base = _finger_gap_length_get(pd->zoom_st1.cur.pos.x,
                                                     pd->zoom_st1.cur.pos.y,
                                                     pd->zoom_st.cur.pos.x,
                                                     pd->zoom_st.cur.pos.y,
                                                     &x, &y);

              zd->radius = pd->zoom_base / 2.0;

              if ((efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_STARTED) &&
                  (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_UPDATED))
                return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

              return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }

         const Efl_Gesture_Touch_Point_Data *p2 = efl_gesture_touch_data_get(event, 1);
         if (p2->id == pd->zoom_mv.id)
           memcpy(&pd->zoom_mv, p2, sizeof(Efl_Gesture_Touch_Point_Data));
         else if (p2->id == pd->zoom_mv1.id)
           memcpy(&pd->zoom_mv1, p2, sizeof(Efl_Gesture_Touch_Point_Data));

         zd->zoom = _zoom_compute(pd, zd, pd->zoom_mv.cur.pos.x,
                                  pd->zoom_mv.cur.pos.y, pd->zoom_mv1.cur.pos.x,
                                  pd->zoom_mv1.cur.pos.y, pd->zoom_finger_factor);

         if (!EINA_DBL_NONZERO(zoom_distance_tolerance))
           {
              double d = zd->zoom - pd->next_step;

              if (d < 0.0) d = (-d);

              if (d >= pd->zoom_step)
                {
                   pd->next_step = zd->zoom;

                   return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
                }
           }

         return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
      }

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         /* no gesture was started, so no gesture should be detected */
         if ((td->touch_down == 0) || (!pd->zoom_st.cur.timestamp))
           {
              rd->continues = EINA_FALSE;

              _reset_recognizer(pd);
              efl_gesture_manager_recognizer_cleanup(efl_provider_find(obj, EFL_CANVAS_GESTURE_MANAGER_CLASS), obj, watched);

              return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
           }
         if ((pd->zoom_base) && EINA_DBL_EQ(pd->zoom_distance_tolerance, 0))
           {
              _reset_recognizer(pd);

              return EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
           }

         if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE)
           {
              _reset_recognizer(pd);

              return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }
      }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_zoom.eo.c"
