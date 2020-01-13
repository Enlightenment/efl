#ifndef EFL_GESTURE_PRIVATE_H_
#define EFL_GESTURE_PRIVATE_H_

#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_gesture_events.eo.h"

#include <Ecore.h>

/* milliseconds */
#define TAP_TOUCH_TIME_THRESHOLD (0.1 * 1000)

const Efl_Event_Description * _efl_gesture_type_get(const Eo *obj);
void efl_gesture_manager_gesture_clean_up(Eo *obj, Eo *target, const Efl_Event_Description *type);

typedef struct _Efl_Canvas_Gesture_Manager_Data                Efl_Canvas_Gesture_Manager_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Data             Efl_Canvas_Gesture_Recognizer_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Tap_Data         Efl_Canvas_Gesture_Recognizer_Tap_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Long_Tap_Data    Efl_Canvas_Gesture_Recognizer_Long_Tap_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Double_Tap_Data  Efl_Canvas_Gesture_Recognizer_Double_Tap_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data  Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Momentum_Data    Efl_Canvas_Gesture_Recognizer_Momentum_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Flick_Data       Efl_Canvas_Gesture_Recognizer_Flick_Data;
typedef struct _Efl_Canvas_Gesture_Recognizer_Zoom_Data        Efl_Canvas_Gesture_Recognizer_Zoom_Data;
typedef struct _Efl_Canvas_Gesture_Data                        Efl_Canvas_Gesture_Data;
typedef struct _Efl_Canvas_Gesture_Momentum_Data               Efl_Canvas_Gesture_Momentum_Data;
typedef struct _Efl_Canvas_Gesture_Flick_Data                  Efl_Canvas_Gesture_Flick_Data;
typedef struct _Efl_Canvas_Gesture_Zoom_Data                   Efl_Canvas_Gesture_Zoom_Data;

typedef struct _Efl_Canvas_Gesture_Touch_Data
{
   Efl_Canvas_Gesture_Touch_State state;
   Eina_Array             *touch_points;
   int                     touch_down;
   Efl_Gesture_Touch_Point_Data *cur_touch;
   Efl_Gesture_Touch_Point_Data *prev_touch;
   Eina_Bool               multi_touch;
   Eo                     *target;
} Efl_Canvas_Gesture_Touch_Data;

struct _Efl_Canvas_Gesture_Recognizer_Data
{
   Eo                            *manager; // keeps a reference of the manager
   Eo                            *gesture;
   int                            finger_size;
   Eina_Bool                      continues;
};

struct _Efl_Canvas_Gesture_Recognizer_Tap_Data
{
   Eo                             *target;
   Eo                             *gesture;
   Ecore_Timer                    *timeout;
};

struct _Efl_Canvas_Gesture_Recognizer_Long_Tap_Data
{
   Eina_List                      *target_timeout;
   Eo                             *target;
   Efl_Canvas_Gesture             *gesture;
   Ecore_Timer                    *timeout;
   double                          start_timeout;
   Eina_Bool                       is_timeout;
};

struct _Efl_Canvas_Gesture_Recognizer_Double_Tap_Data
{
   Eina_List                      *target_timeout;
   Eo                             *target;
   Eo                             *gesture;
   Ecore_Timer                    *timeout;
   double                          start_timeout;
   Eina_Bool                       is_timeout;
   int                             tap_count;
};

struct _Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data
{
   Eina_List                      *target_timeout;
   Eo                             *target;
   Eo                             *gesture;
   Ecore_Timer                    *timeout;
   double                          start_timeout;
   Eina_Bool                       is_timeout;
   int                             tap_count;
};

struct _Efl_Canvas_Gesture_Recognizer_Momentum_Data
{
   Eina_Position2D                 st_line;
   Eina_Position2D                 end_line;
   unsigned int                    t_st;
   unsigned int                    t_end;
   int                             xdir;
   int                             ydir;
   Eina_Bool                       touched;
};

struct _Efl_Canvas_Gesture_Recognizer_Flick_Data
{
   Eina_Position2D                 st_line;
   unsigned int                    t_st;
   unsigned int                    t_end;
   int                             line_length;
   double                          line_angle;
   Eina_Bool                       touched;
};

struct _Efl_Canvas_Gesture_Recognizer_Zoom_Data
{
   Efl_Gesture_Touch_Point_Data                    zoom_st;
   Efl_Gesture_Touch_Point_Data                    zoom_st1;

   Efl_Gesture_Touch_Point_Data                    zoom_mv;
   Efl_Gesture_Touch_Point_Data                    zoom_mv1;

   Evas_Coord                      zoom_base; /* Holds gap between fingers on
							                   * zoom-start  */
   double                          zoom_distance_tolerance;
   double                          zoom_finger_factor;
   double                          zoom_step;
   double                          next_step;
   Eina_Bool                       calc_temp;
};

struct _Efl_Canvas_Gesture_Data
{
   const Efl_Event_Description    *type;
   Efl_Canvas_Gesture_State        state;
   Eina_Position2D                 hotspot;
   unsigned int                    timestamp;
};

struct _Efl_Canvas_Gesture_Momentum_Data
{
   Eina_Vector2                    momentum;
};

struct _Efl_Canvas_Gesture_Flick_Data
{
   Eina_Vector2                    momentum;
   double                          angle;
};

struct _Efl_Canvas_Gesture_Zoom_Data
{
   double                          radius;
   double                          zoom;
};

#endif
