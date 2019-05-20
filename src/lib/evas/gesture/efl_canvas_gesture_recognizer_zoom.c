#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS

static Evas_Coord
_finger_gap_length_get(Evas_Coord xx1,
		Evas_Coord yy1,
		Evas_Coord xx2,
		Evas_Coord yy2,
		Evas_Coord *x,
		Evas_Coord *y)
{
   double a, b, xx, yy, gap;
   xx = abs(xx2 - xx1);
   yy = abs(yy2 - yy1);
   gap = sqrt((xx * xx) + (yy * yy));

   /* START - Compute zoom center point */
   /* The triangle defined as follows:
    *             B
    *           / |
    *          /  |
    *     gap /   | a
    *        /    |
    *       A-----C
    *          b
    * http://en.wikipedia.org/wiki/Trigonometric_functions
    *************************************/
   if (((int)xx) && ((int)yy))
     {
        double A = atan((yy / xx));
        a = (Evas_Coord)((gap / 2) * sin(A));
        b = (Evas_Coord)((gap / 2) * cos(A));
        *x = (Evas_Coord)((xx2 > xx1) ? (xx1 + b) : (xx2 + b));
        *y = (Evas_Coord)((yy2 > yy1) ? (yy1 + a) : (yy2 + a));
     }
   else
     {
        if ((int)xx) /* horiz line, take half width */
          {
             *x = (Evas_Coord)((xx1 + xx2) / 2);
             *y = (Evas_Coord)(yy1);
          }

        if ((int)yy) /* vert line, take half width */
          {
             *x = (Evas_Coord)(xx1);
             *y = (Evas_Coord)((yy1 + yy2) / 2);
          }
     }
   /* END   - Compute zoom center point */

   return (Evas_Coord)gap;
}

static double
_zoom_compute(Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd,
              Efl_Canvas_Gesture_Zoom_Data *zd,
              Evas_Coord xx1,
              Evas_Coord yy1,
              Evas_Coord xx2,
              Evas_Coord yy2,
              double zoom_finger_factor)
{
   double rt = 1.0;
   //TODO: Enable below code if the zoom momentum is need
   //unsigned int tm_end = (pd->zoom_mv.cur.timestamp > pd->zoom_mv1.cur.timestamp) ?
   //   pd->zoom_mv.cur.timestamp : pd->zoom_mv1.cur.timestamp;

   int x,y; //Hot spot
   Evas_Coord diam = _finger_gap_length_get(xx1, yy1, xx2, yy2,
                                            &x, &y);

   zd->radius = diam / 2;

   if (!pd->zoom_base)
     {
        pd->zoom_base = diam;
        return zd->zoom;
     }

   if (pd->zoom_distance_tolerance) /* zoom tolerance <> ZERO, means
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

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_zoom_efl_canvas_gesture_recognizer_add(Eo *obj,
                                                                      Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd EINA_UNUSED,
                                                                      Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_ZOOM_CLASS, obj);
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_zoom_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                            Efl_Canvas_Gesture_Recognizer_Zoom_Data *pd,
                                                                            Efl_Canvas_Gesture *gesture,
                                                                            Efl_Object *watched,
                                                                            Efl_Canvas_Gesture_Touch *event)
{
   int id1 = 0;
   int id2 = 1;
   Eina_Value *val;
   unsigned char zoom_finger_enable;
   unsigned char glayer_continues_enable;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
   Efl_Canvas_Gesture_Zoom_Data *zd = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_ZOOM_CLASS);
   Efl_Canvas_Gesture_Touch_Data *td = efl_data_scope_get(event, EFL_CANVAS_GESTURE_TOUCH_CLASS);
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

   //FIXME: Wheel zoom test first here.

   val = efl_gesture_recognizer_config_get(obj, "glayer_continues_enable");
   if (val) eina_value_get(val, &glayer_continues_enable);
   else glayer_continues_enable = 1;

   val = efl_gesture_recognizer_config_get(obj, "glayer_zoom_finger_enable");
   if (val) eina_value_get(val, &zoom_finger_enable);
   else zoom_finger_enable = 1;

   val = efl_gesture_recognizer_config_get(obj, "glayer_zoom_finger_factor");
   if (val)  eina_value_get(val, &pd->zoom_finger_factor);
   else pd->zoom_finger_factor = 1.0;

   rd->continues = EINA_TRUE;

   if (!pd->zoom_distance_tolerance && !pd->calc_temp)
     {
        pd->calc_temp = EINA_TRUE;
        val = efl_gesture_recognizer_config_get(obj, "glayer_zoom_distance_tolerance");
        if (val)  eina_value_get(val, &pd->zoom_distance_tolerance);
        else pd->zoom_distance_tolerance = 1.0;

        pd->zoom_distance_tolerance *= rd->finger_size;
     }

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_UPDATE:
        {
           if ((!glayer_continues_enable) && (!pd->zoom_st.cur.timestamp))
             {
                return EFL_GESTURE_IGNORE;
             }
           EINA_FALLTHROUGH;
        }
      case EFL_GESTURE_TOUCH_BEGIN:
        {
           if (td->touch_down > 2)
             {
                return EFL_GESTURE_CANCEL;
             }

           if (!pd->zoom_st.cur.timestamp) /* Now scan touched-devices list
                                            * and find other finger */
             {
                if (!efl_gesture_touch_multi_touch_get(event))
                  return EFL_GESTURE_IGNORE;

                Pointer_Data *p1 = eina_hash_find(td->touch_points, &id1);
                Pointer_Data *p2 = eina_hash_find(td->touch_points, &id2);

                memcpy(&pd->zoom_st, p2, sizeof(Pointer_Data));
                memcpy(&pd->zoom_st1, p1, sizeof(Pointer_Data));

                memcpy(&pd->zoom_mv, p2, sizeof(Pointer_Data));
                memcpy(&pd->zoom_mv1, p1, sizeof(Pointer_Data));

                int x,y; //Hot spot
                zd->zoom = 1.0;
                pd->zoom_base = _finger_gap_length_get(pd->zoom_st1.cur.pos.x,
                                                       pd->zoom_st1.cur.pos.y,
                                                       pd->zoom_st.cur.pos.x,
                                                       pd->zoom_st.cur.pos.y,
                                                       &x, &y);

                zd->radius = pd->zoom_base / 2;

                if ((efl_gesture_state_get(gesture) != EFL_GESTURE_STARTED) &&
                    (efl_gesture_state_get(gesture) != EFL_GESTURE_UPDATED))
                  return EFL_GESTURE_TRIGGER;

                return EFL_GESTURE_CANCEL;
             }

           Pointer_Data *p2 = eina_hash_find(td->touch_points, &id2);
           if (p2->id == pd->zoom_mv.id)
             memcpy(&pd->zoom_mv, p2, sizeof(Pointer_Data));
           else if (p2->id == pd->zoom_mv1.id)
             memcpy(&pd->zoom_mv1, p2, sizeof(Pointer_Data));

           zd->zoom = _zoom_compute(pd, zd, pd->zoom_mv.cur.pos.x,
                                    pd->zoom_mv.cur.pos.y, pd->zoom_mv1.cur.pos.x,
                                    pd->zoom_mv1.cur.pos.y, pd->zoom_finger_factor);


           if (!pd->zoom_distance_tolerance)
             {
                double d = zd->zoom - pd->next_step;

                if (d < 0.0) d = (-d);

                if (d >= pd->zoom_step)
                  {
                     pd->next_step = zd->zoom;

                     return EFL_GESTURE_TRIGGER;
                  }
             }

           return EFL_GESTURE_IGNORE;
        }
      case EFL_GESTURE_TOUCH_END:
        {
           if (td->touch_down == 0)
             {
                rd->continues = EINA_FALSE;

                memset(pd, 0, sizeof(Efl_Canvas_Gesture_Recognizer_Zoom_Data));
                efl_gesture_manager_gesture_clean_up(rd->manager, watched, EFL_EVENT_GESTURE_ZOOM);

                return EFL_GESTURE_IGNORE;
             }
           if ((pd->zoom_base) && (pd->zoom_distance_tolerance == 0))
             {
                memset(pd, 0, sizeof(Efl_Canvas_Gesture_Recognizer_Zoom_Data));

                return EFL_GESTURE_FINISH;
             }

           if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE)
             {
                memset(pd, 0, sizeof(Efl_Canvas_Gesture_Recognizer_Zoom_Data));

                return EFL_GESTURE_CANCEL;
             }
        }

      default:

         break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_zoom.eo.c"
