#define EFL_CANVAS_GESTURE_RECOGNIZER_PROTECTED
#include "efl_canvas_gesture_private.h"
#define RAD2DEG(x) ((x) * 57.295779513)

#define MY_CLASS                                    EFL_CANVAS_GESTURE_RECOGNIZER_CLASS
#include "efl_canvas_gesture_recognizer.eo.h"

Eina_Value *
_recognizer_config_get(const Eo *obj, const char *name)
{
   Eo *config = efl_provider_find(obj, EFL_CONFIG_INTERFACE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(config, NULL);
   return efl_config_get(config, name);
}

EOLIAN static Eina_Bool
_efl_canvas_gesture_recognizer_continues_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd)
{
   return pd->continues;
}

EOLIAN static void
_efl_canvas_gesture_recognizer_continues_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd, Eina_Bool value)
{
   pd->continues = !!value;
}

int
_direction_get(Evas_Coord xx1, Evas_Coord xx2)
{
   if (xx2 < xx1) return -1;
   if (xx2 > xx1) return 1;

   return 0;
}

Eina_Bool
_event_multi_touch_get(const Efl_Canvas_Gesture_Touch *event)
{
   return efl_gesture_touch_points_count_get(event) > 1;
}


double
_angle_get(Evas_Coord xx1, Evas_Coord yy1, Evas_Coord xx2, Evas_Coord yy2)
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

Evas_Coord
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

#include "efl_canvas_gesture_recognizer.eo.c"
