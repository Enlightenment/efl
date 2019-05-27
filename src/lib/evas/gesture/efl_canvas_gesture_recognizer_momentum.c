#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_MOMENTUM_CLASS

#define MOMENTUM_TIMEOUT 50
#define THUMBSCROLL_FRICTION 0.95
#define THUMBSCROLL_MOMENTUM_THRESHOLD 100.0
#define EFL_GESTURE_MINIMUM_MOMENTUM 0.001

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_momentum_efl_canvas_gesture_recognizer_add(Eo *obj, Efl_Canvas_Gesture_Recognizer_Momentum_Data *pd EINA_UNUSED, Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_MOMENTUM_CLASS, obj);
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
   double thumbscroll_friction, thumbscroll_momentum_threshold;

   if (dt > 0)
     {
        velx = (dx * 1000) / dt;
        vely = (dy * 1000) / dt;
	 }

   vel = sqrt((velx * velx) + (vely * vely));

   tf = efl_gesture_recognizer_config_get(obj, "thumbscroll_friction");
   if (tf) eina_value_get(tf, &thumbscroll_friction);
   else thumbscroll_friction = THUMBSCROLL_FRICTION;

   tmt = efl_gesture_recognizer_config_get(obj, "thumbscroll_momentum_threshold");
   if (tmt) eina_value_get(tmt, &thumbscroll_momentum_threshold);
   else thumbscroll_momentum_threshold = THUMBSCROLL_MOMENTUM_THRESHOLD;

   if ((thumbscroll_friction > 0.0) &&
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

static int
_direction_get(Evas_Coord xx1,
               Evas_Coord xx2)
{
   if (xx2 < xx1) return -1;
   if (xx2 > xx1) return 1;

   return 0;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_momentum_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                                  Efl_Canvas_Gesture_Recognizer_Momentum_Data *pd,
                                                                                  Efl_Canvas_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                                                  Efl_Canvas_Gesture_Touch *event)
{
   Eina_Value *val;
   unsigned char glayer_continues_enable;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
   Efl_Canvas_Gesture_Momentum_Data *md = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_MOMENTUM_CLASS);

   val = efl_gesture_recognizer_config_get(obj, "glayer_continues_enable");
   if (val) eina_value_get(val, &glayer_continues_enable);
   else glayer_continues_enable = 1;

   //Check the touched to ignore very first event.
   //It does not have any meanging of this gesture.
   if (glayer_continues_enable && !pd->touched)
     {
       pd->touched = EINA_TRUE;

	   return EFL_GESTURE_IGNORE;
	 }

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_BEGIN:
      case EFL_GESTURE_TOUCH_UPDATE:
        {
           if (!pd->t_st)
             {
                if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_BEGIN ||
                    glayer_continues_enable)
                  {
                     pd->t_st = pd->t_end =  efl_gesture_touch_cur_timestamp_get(event);

                     pd->st_line = pd->end_line =
                        efl_gesture_touch_start_point_get(event);

                     efl_gesture_hotspot_set(gesture, pd->st_line);

                     return EFL_GESTURE_TRIGGER;
                  }
             }

           if ((efl_gesture_touch_cur_timestamp_get(event) - MOMENTUM_TIMEOUT) >
               pd->t_end)
             {
                pd->st_line = efl_gesture_touch_cur_point_get(event);
				pd->t_st = efl_gesture_touch_cur_timestamp_get(event);
				pd->xdir = pd->ydir = 0;
			 }
		   else
		     {
                int xdir, ydir;
                Eina_Position2D cur_p = efl_gesture_touch_cur_point_get(event);

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

           pd->end_line = efl_gesture_touch_cur_point_get(event);
		   pd->t_end = efl_gesture_touch_cur_timestamp_get(event);
           efl_gesture_hotspot_set(gesture, pd->end_line);

           _momentum_set(obj, md, pd->st_line, efl_gesture_touch_cur_point_get(event),
                         pd->t_st, efl_gesture_touch_cur_timestamp_get(event));

           result = EFL_GESTURE_TRIGGER;

           break;
        }

      case EFL_GESTURE_TOUCH_END:
        {
           if (!pd->t_st)
             {
                pd->touched = EINA_FALSE;

                return EFL_GESTURE_CANCEL;
             }

           if ((efl_gesture_touch_cur_timestamp_get(event) - MOMENTUM_TIMEOUT) > pd->t_end)
             {
                pd->st_line = efl_gesture_touch_cur_point_get(event);
                pd->t_st = efl_gesture_touch_cur_timestamp_get(event);
                pd->xdir = pd->ydir = 0;
             }

           pd->end_line = efl_gesture_touch_cur_point_get(event);
		   pd->t_end = efl_gesture_touch_cur_timestamp_get(event);
           efl_gesture_hotspot_set(gesture, pd->end_line);

           if ((fabs(md->momentum.x) > EFL_GESTURE_MINIMUM_MOMENTUM) ||
               (fabs(md->momentum.y) > EFL_GESTURE_MINIMUM_MOMENTUM))
                result = EFL_GESTURE_FINISH;
           else
                result = EFL_GESTURE_CANCEL;

           memset(pd, 0, sizeof(Efl_Canvas_Gesture_Recognizer_Momentum_Data));

		   break;
        }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_momentum.eo.c"
