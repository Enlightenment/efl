#include <Elementary.h>
#include "elm_priv.h"
/** @defgroup Elm_Gesture_Layer Gesture Layer */

/* Some defaults */
#define ELM_MOUSE_DEVICE 0
/* ELM_GESTURE_NEGATIVE_ANGLE - magic number says we didn't compute this yet */
#define ELM_GESTURE_NEGATIVE_ANGLE (-1.0) /* Magic number */
#define ELM_GESTURE_MOMENTUM_TIMEOUT 50
#define ELM_GESTURE_MULTI_TIMEOUT 50

/* Some Trigo values */
#define RAD_90DEG  M_PI_2
#define RAD_180DEG M_PI
#define RAD_270DEG (M_PI_2 * 3)
#define RAD_360DEG (M_PI * 2)
/* #define DEBUG_GESTURE_LAYER 1 */

static void *
_glayer_bufdup(void *buf, size_t size)
{
   void *p;
   p = malloc(size);
   memcpy(p, buf, size);
   return p;
}
#define COPY_EVENT_INFO(EV) _glayer_bufdup(EV, sizeof(*EV))


#define SET_TEST_BIT(P) do { \
   P->test = P->fn[ELM_GESTURE_STATE_START].cb || P->fn[ELM_GESTURE_STATE_MOVE].cb || P->fn[ELM_GESTURE_STATE_END].cb || P->fn[ELM_GESTURE_STATE_ABORT].cb; \
} while (0)

#define IS_TESTED(T) ((wd->gesture[T]) ? wd->gesture[T]->test : EINA_FALSE)

/**
 * @internal
 *
 * @struct _Func_Data
 * Struct holds callback information.
 *
 * @ingroup Elm_Gesture_Layer
 */
struct _Func_Data
{
   void *user_data; /**< Holds user data to CB (like sd) */
   Elm_Gesture_Event_Cb cb;
};

/**
 * @internal
 *
 * @typedef Func_Data
 * type for callback information
 *
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Func_Data Func_Data;

/**
 * @internal
 *
 * @struct _Gesture_Info
 * Struct holds gesture info
 *
 * @ingroup Elm_Gesture_Layer
 */
struct _Gesture_Info
{
  Evas_Object *obj;
  void *data; /**< Holds gesture intemidiate processing data */
  Func_Data fn[ELM_GESTURE_STATE_ABORT + 1]; /**< Callback info for states */
  Elm_Gesture_Types g_type;  /**< gesture type */
  Elm_Gesture_State state;  /**< gesture state */
  void *info;                        /**< Data for the state callback */
  Eina_Bool test; /**< if true this gesture should be tested on input */
};

/**
 * @internal
 *
 * @typedef Gesture_Info
 * Type for _Gesture_Info
 *
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Gesture_Info Gesture_Info;

/**
 * @internal
 *
 * @struct _Event_History
 * Struct holds event history.
 * These events are repeated if no gesture found.
 *
 * @ingroup Elm_Gesture_Layer
 */
struct _Event_History
{
   EINA_INLIST;
   void *event;
   Evas_Callback_Type event_type;
};

/**
 * @internal
 *
 * @typedef Event_History
 * Type for _Event_History
 *
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Event_History Event_History;

/**
 * @internal
 *
 * @struct _Pointer_Event
 * Struct holds pointer-event info
 * This is a generic pointer event structure
 *
 * @ingroup Elm_Gesture_Layer
 */
struct _Pointer_Event
{
   Evas_Coord x, y;
   unsigned int timestamp;
   int device;
   Evas_Callback_Type event_type;
};

/**
 * @internal
 *
 * @typedef Pointer_Event
 * Type for generic pointer event structure
 *
 * @ingroup Elm_Gesture_Layer
 */
typedef struct _Pointer_Event Pointer_Event;

/* All *Type structs hold result for the user in 'info' field
 * The rest is gesture processing intermediate data.
 * NOTE: info field must be FIRST in the struct.
 * This is used when reporting ABORT in event_history_clear() */
struct _Taps_Type
{
   Elm_Gesture_Taps_Info info;
   unsigned int sum_x;
   unsigned int sum_y;
   unsigned int n_taps_needed;
   unsigned int n_taps;
   Eina_List *l;
};
typedef struct _Taps_Type Taps_Type;

struct _Long_Tap_Type
{
   Elm_Gesture_Taps_Info info;
   Evas_Coord center_x;
   Evas_Coord center_y;
   unsigned int max_touched;
   Ecore_Timer *timeout; /* When this expires, long tap STARTed */
   Eina_List *touched;
};
typedef struct _Long_Tap_Type Long_Tap_Type;

struct _Momentum_Type
{  /* Fields used by _line_test() */
   Elm_Gesture_Momentum_Info info;
   Evas_Coord_Point line_st;
   Evas_Coord_Point line_end;
   unsigned int t_st_x;  /* Time start on X */
   unsigned int t_st_y;  /* Time start on Y */
   unsigned int t_end;   /* Time end        */
   unsigned int t_up; /* Recent up event time */
   int xdir, ydir;
};
typedef struct _Momentum_Type Momentum_Type;

struct _Line_Data
{
   Evas_Coord_Point line_st;
   Evas_Coord_Point line_end;
   Evas_Coord line_length;
   unsigned int t_st;  /* Time start */
   unsigned int t_end; /* Time end   */
   int device;
   double line_angle;  /* Current angle of line */
};
typedef struct _Line_Data Line_Data;

struct _Line_Type
{  /* Fields used by _line_test() */
   Elm_Gesture_Line_Info info;
   Eina_List *list; /* List of Line_Data */
};
typedef struct _Line_Type Line_Type;

struct _Zoom_Type
{  /* Fields used by _zoom_test() */
   Elm_Gesture_Zoom_Info info;
   Pointer_Event zoom_st;
   Pointer_Event zoom_mv;
   Pointer_Event zoom_st1;
   Pointer_Event zoom_mv1;
   Evas_Event_Mouse_Wheel *zoom_wheel;
   Evas_Coord zoom_base;  /* Holds gap between fingers on zoom-start  */
   Evas_Coord zoom_distance_tolerance;
   Elm_Gesture_Momentum_Info momentum1;      /* For continues gesture */
   Elm_Gesture_Momentum_Info momentum2;      /* For continues gesture */
   double next_step;
};
typedef struct _Zoom_Type Zoom_Type;

struct _Rotate_Type
{  /* Fields used by _rotation_test() */
   Elm_Gesture_Rotate_Info info;
   Pointer_Event rotate_st;
   Pointer_Event rotate_mv;
   Pointer_Event rotate_st1;
   Pointer_Event rotate_mv1;
   double rotate_angular_tolerance;
   double next_step;
   Elm_Gesture_Momentum_Info momentum1;      /* For continues gesture */
   Elm_Gesture_Momentum_Info momentum2;      /* For continues gesture */
};
typedef struct _Rotate_Type Rotate_Type;

struct _Widget_Data
{
   Evas_Object *target;  /* Target Widget */
   Event_History *event_history_list;

   int line_min_length;
   Evas_Coord zoom_distance_tolerance;
   Evas_Coord line_distance_tolerance;
   double line_angular_tolerance;
   double zoom_wheel_factor; /* mouse wheel zoom steps */
   double zoom_finger_factor; /* used for zoom factor */
   double rotate_angular_tolerance;
   unsigned int flick_time_limit_ms;
   double long_tap_start_timeout;
   Eina_Bool glayer_continues_enable;

   double zoom_step;
   double rotate_step;

   Gesture_Info *gesture[ELM_GESTURE_LAST];
   Ecore_Timer *dbl_timeout; /* When this expires, dbl click/taps ABORTed  */
   Eina_List *pending; /* List of devices need to refeed *UP event */
   Eina_List *touched;  /* Information  of touched devices   */

   Eina_Bool repeat_events : 1;
};
typedef struct _Widget_Data Widget_Data;

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);

static Eina_Bool _event_history_clear(Evas_Object *obj);
static void _reset_states(Widget_Data *wd);
static void _key_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static void _key_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static void _zoom_with_wheel_test(Evas_Object *obj, void *event_info, Evas_Callback_Type event_type, Elm_Gesture_Types g_type);
static void _mouse_wheel(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static void _mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _multi_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static void _multi_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static void _multi_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);

/* START - Functions to manage touched-device list */
/**
 * @internal
 * This function is used to find if device is touched
 *
 * @ingroup Elm_Gesture_Layer
 */
static int
compare_device(const void *data1, const void *data2)
{  /* Compare the two device numbers */
   return (((Pointer_Event *) data1)->device -((Pointer_Event *) data2)->device);
}

/**
 * @internal
 *
 * Remove Pointer Event from touched device list
 * @param list Pointer to touched device list.
 * @param Pointer_Event Pointer to PE.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_remove_touched_device(Eina_List *list, Pointer_Event *pe)
{
   Pointer_Event *p = eina_list_search_unsorted(list, compare_device, pe);
   if (p)
     {
        free(p);
        return eina_list_remove(list, p);
     }

   return list;
}

/**
 * @internal
 *
 * Recoed Pointer Event in touched device list
 * Note: This fuction allocates memory for PE event
 * This memory is released in _remove_touched_device()
 * @param list Pointer to touched device list.
 * @param Pointer_Event Pointer to PE.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_add_touched_device(Eina_List *list, Pointer_Event *pe)
{
   Pointer_Event *p = eina_list_search_unsorted(list, compare_device, pe);
   if (p)
     {  /* We like to track device touch-position, overwrite info */
        memcpy(p, pe, sizeof(Pointer_Event));
        return list;
     }

   if ((pe->event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
         (pe->event_type == EVAS_CALLBACK_MULTI_DOWN))
     {  /* Add touched device on DOWN event only */
        p = malloc(sizeof(Pointer_Event));
        /* Freed in _remove_touched_device()    */
        memcpy(p, pe, sizeof(Pointer_Event));
        return eina_list_append(list, p);
     }

   return list;
}
/* END   - Functions to manage touched-device list */

/**
 * @internal
 *
 * Get event flag
 * @param event_info pointer to event.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Evas_Event_Flags
_get_event_flag(void *event_info, Evas_Callback_Type event_type)
{
   switch(event_type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
         return ((Evas_Event_Mouse_In *) event_info)->event_flags;
      case EVAS_CALLBACK_MOUSE_OUT:
         return ((Evas_Event_Mouse_Out *) event_info)->event_flags;
      case EVAS_CALLBACK_MOUSE_DOWN:
         return ((Evas_Event_Mouse_Down *) event_info)->event_flags;
      case EVAS_CALLBACK_MOUSE_MOVE:
         return ((Evas_Event_Mouse_Move *) event_info)->event_flags;
      case EVAS_CALLBACK_MOUSE_UP:
         return ((Evas_Event_Mouse_Up *) event_info)->event_flags;
      case EVAS_CALLBACK_MOUSE_WHEEL:
         return ((Evas_Event_Mouse_Wheel *) event_info)->event_flags;
      case EVAS_CALLBACK_MULTI_DOWN:
         return ((Evas_Event_Multi_Down *) event_info)->event_flags;
      case EVAS_CALLBACK_MULTI_MOVE:
         return ((Evas_Event_Multi_Move *) event_info)->event_flags;
      case EVAS_CALLBACK_MULTI_UP:
         return ((Evas_Event_Multi_Up *) event_info)->event_flags;
      case EVAS_CALLBACK_KEY_DOWN:
         return ((Evas_Event_Key_Down *) event_info)->event_flags;
      case EVAS_CALLBACK_KEY_UP:
         return ((Evas_Event_Key_Up *) event_info)->event_flags;
      default:
         return EVAS_EVENT_FLAG_NONE;
     }
}

/**
 * @internal
 *
 * Sets event flag to value returned from user callback
 * @param wd Widget Data
 * @param event_info pointer to event.
 * @param event_type what type was ev (mouse down, etc...)
 * @param ev_flags event flags
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
consume_event(Widget_Data *wd, void *event_info,
      Evas_Callback_Type event_type, Evas_Event_Flags ev_flags)
{  /* Mark EVAS_EVENT_FLAG_ON_HOLD on events that are used by gesture layer */
   /* ev_flags != EVAS_EVENT_FLAG_NONE means target used event and g-layer  */
   /* should not refeed this event.                                         */
   if(!event_info)
     return;  /* This happens when restarting gestures  */

   if ((ev_flags) || (!wd->repeat_events))
     {
        switch(event_type)
          {
           case EVAS_CALLBACK_MOUSE_DOWN:
              ((Evas_Event_Mouse_Down *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_MOUSE_MOVE:
              ((Evas_Event_Mouse_Move *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_MOUSE_UP:
              ((Evas_Event_Mouse_Up *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_MOUSE_WHEEL:
              ((Evas_Event_Mouse_Wheel *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_MULTI_DOWN:
              ((Evas_Event_Multi_Down *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_MULTI_MOVE:
              ((Evas_Event_Multi_Move *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_MULTI_UP:
              ((Evas_Event_Multi_Up *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_KEY_DOWN:
              ((Evas_Event_Key_Down *) event_info)->event_flags |= ev_flags;
              break;
           case EVAS_CALLBACK_KEY_UP:
              ((Evas_Event_Key_Up *) event_info)->event_flags |= ev_flags;
              break;
           default:
              return;
          }
     }
}

/**
 * @internal
 *
 * Report current state of a gesture by calling user callback.
 * @param gesture what gesture state we report.
 * @param info inforamtion for user callback
 *
 * @ingroup Elm_Gesture_Layer
 */
static Evas_Event_Flags
_report_state(Gesture_Info *gesture, void *info)
{  /* We report current state (START, MOVE, END, ABORT), once */
#if defined(DEBUG_GESTURE_LAYER)
   printf("%s reporting gesture=<%d> state=<%d>\n" , __func__, gesture->g_type,
         gesture->state);
#endif
   if ((gesture->state != ELM_GESTURE_STATE_UNDEFINED) &&
         (gesture->fn[gesture->state].cb))
     {  /* Fill state-info struct and send ptr to user callback */
        return gesture->fn[gesture->state].cb(
              gesture->fn[gesture->state].user_data, info);
     }

   return EVAS_EVENT_FLAG_NONE;
}

/**
 * @internal
 *
 * Update state for a given gesture.
 * We may update gesture state to:
 * UNDEFINED - current input did not start gesure yet.
 * START - gesture started according to input.
 * MOVE - gusture in progress.
 * END - gesture completed according to input.
 * ABORT - input does not matches gesure.
 * note that we may move from UNDEFINED to ABORT
 * because we may detect that gesture will not START
 * with a given input.
 *
 * @param g given gesture to change state.
 * @param s gesure new state.
 * @param info buffer to be sent to user callback on report_state.
 * @param force makes report_state to report the new-state even
 * if its same as current state. Works for MOVE - gesture in progress.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Evas_Event_Flags
_set_state(Gesture_Info *g, Elm_Gesture_State s,
      void *info, Eina_Bool force)
{
   Elm_Gesture_State old_state;
   if ((g->state == s) && (!force))
     return EVAS_EVENT_FLAG_NONE;

   old_state = g->state;

   g->state = s;
   g->info = info;  /* Information for user callback */
   if ((g->state == ELM_GESTURE_STATE_ABORT) ||
         (g->state == ELM_GESTURE_STATE_END))
     g->test = EINA_FALSE;

   if ((g->state != ELM_GESTURE_STATE_UNDEFINED) &&
         (!((old_state == ELM_GESTURE_STATE_UNDEFINED) &&
            (s == ELM_GESTURE_STATE_ABORT))))
     return _report_state(g, g->info);

   return EVAS_EVENT_FLAG_NONE;
}

/**
 * @internal
 *
 * This resets all gesture states and sets test-bit.
 * this is used for restarting gestures to listen to input.
 * happens after we complete a gesture or no gesture was detected.
 * @param wd Widget data of the gesture-layer object.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_reset_states(Widget_Data *wd)
{
   int i;
   Gesture_Info *p;
   for (i = ELM_GESTURE_FIRST; i < ELM_GESTURE_LAST; i++)
     {
        p = wd->gesture[i];
        if (p)
          {
             _set_state(p, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
             SET_TEST_BIT(p);
          }
     }
}

/**
 * @internal
 *
 * if gesture was NOT detected AND we only have gestures in ABORT state
 * we clear history immediately to be ready for input.
 *
 * @param obj The gesture-layer object.
 * @return TRUE on event history_clear
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_clear_if_finished(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   int i;

   /* Clear history if all we have aborted gestures */
   Eina_Bool reset_s = EINA_TRUE, all_undefined = EINA_TRUE;
   for (i = ELM_GESTURE_FIRST; i < ELM_GESTURE_LAST; i++)
     {  /* If no gesture started and all we have aborted gestures, reset all */
        Gesture_Info *p = wd->gesture[i];
        if ((p) && (p->state != ELM_GESTURE_STATE_UNDEFINED))
          {
             if ((p->state == ELM_GESTURE_STATE_START) ||
                   (p->state == ELM_GESTURE_STATE_MOVE))
               reset_s = EINA_FALSE;

             all_undefined = EINA_FALSE;
          }
     }

   if (reset_s && (!all_undefined))
     return _event_history_clear(obj);

   return EINA_FALSE;
}

static Eina_Bool
_inside(Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2)
{
   int w = elm_finger_size_get() >> 1; /* Finger size devided by 2 */
   if (x1 < (x2 - w))
     return EINA_FALSE;

   if (x1 > (x2 + w))
     return EINA_FALSE;

   if (y1 < (y2 - w))
     return EINA_FALSE;

   if (y1 > (y2 + w))
     return EINA_FALSE;

   return EINA_TRUE;
}

/* All *test_reset() funcs are called to clear
 * gesture intermediate data.
 * This happens when we need to reset our tests.
 * for example when gesture is detected or all ABORTed. */
static void
_tap_gestures_test_reset(Gesture_Info *gesture)
{
   if (!gesture)
     return;

   Widget_Data *wd = elm_widget_data_get(gesture->obj);
   wd->dbl_timeout = NULL;
   Eina_List *data;
   Pointer_Event *pe;

   if (!gesture->data)
     return;

   EINA_LIST_FREE(((Taps_Type *) gesture->data)->l, data)
      EINA_LIST_FREE(data, pe)
         free(pe);

  memset(gesture->data, 0, sizeof(Taps_Type));
}

/* All *test_reset() funcs are called to clear
 * gesture intermediate data.
 * This happens when we need to reset our tests.
 * for example when gesture is detected or all ABORTed. */
static void
_n_long_tap_test_reset(Gesture_Info *gesture)
{
   if (!gesture)
     return;

   if (!gesture->data)
     return;

   Long_Tap_Type *st = gesture->data;
   Eina_List *l;
   Pointer_Event *p;
   EINA_LIST_FOREACH(st->touched, l, p)
      free(p);

   eina_list_free(st->touched);
   if (st->timeout) ecore_timer_del(st->timeout);
   memset(gesture->data, 0, sizeof(Long_Tap_Type));
}

static void
_momentum_test_reset(Gesture_Info *gesture)
{
   if (!gesture)
     return;

   if (!gesture->data)
     return;

   memset(gesture->data, 0, sizeof(Momentum_Type));
}

static void
_line_data_reset(Line_Data *st)
{
   if (!st)
     return;

   memset(st, 0, sizeof(Line_Data));
   st->line_angle = ELM_GESTURE_NEGATIVE_ANGLE;
}

static void
_line_test_reset(Gesture_Info *gesture)
{
   if (!gesture)
     return;

   if (!gesture->data)
     return;

   Line_Type *st = gesture->data;
   Eina_List *list = st->list;
   Eina_List *l;
   Line_Data *t_line;
   EINA_LIST_FOREACH(list, l, t_line)
      free(t_line);

   eina_list_free(list);
   st->list = NULL;
}

static void
_zoom_test_reset(Gesture_Info *gesture)
{
   if (!gesture)
     return;

   if (!gesture->data)
     return;

   Widget_Data *wd = elm_widget_data_get(gesture->obj);
   Zoom_Type *st = gesture->data;
   Evas_Modifier_Mask mask = evas_key_modifier_mask_get(
         evas_object_evas_get(wd->target), "Control");
   evas_object_key_ungrab(wd->target, "Control_L", mask, 0);
   evas_object_key_ungrab(wd->target, "Control_R", mask, 0);

   memset(st, 0, sizeof(Zoom_Type));
   st->zoom_distance_tolerance = wd->zoom_distance_tolerance;
   st->info.zoom = 1.0;
}

static void
_rotate_test_reset(Gesture_Info *gesture)
{
   if (!gesture)
     return;

   if (!gesture->data)
     return;

   Widget_Data *wd = elm_widget_data_get(gesture->obj);
   Rotate_Type *st = gesture->data;

   memset(st, 0, sizeof(Rotate_Type));
   st->info.base_angle = ELM_GESTURE_NEGATIVE_ANGLE;
   st->rotate_angular_tolerance = wd->rotate_angular_tolerance;
}


/**
 * @internal
 *
 * We register callbacks when gesture layer is attached to an object
 * or when its enabled after disable.
 *
 * @param obj The gesture-layer object.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_register_callbacks(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->target)
     {
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MOUSE_DOWN,
              _mouse_down, obj);
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MOUSE_MOVE,
              _mouse_move, obj);
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MOUSE_UP,
              _mouse_up, obj);

        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MOUSE_WHEEL,
              _mouse_wheel, obj);

        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MULTI_DOWN,
              _multi_down, obj);
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MULTI_MOVE,
              _multi_move, obj);
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_MULTI_UP,
              _multi_up, obj);

        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_KEY_DOWN,
              _key_down_cb, obj);
        evas_object_event_callback_add(wd->target, EVAS_CALLBACK_KEY_UP,
              _key_up_cb, obj);
     }
}

/**
 * @internal
 *
 * We unregister callbacks when gesture layer is disabled.
 *
 * @param obj The gesture-layer object.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_unregister_callbacks(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->target)
     {
        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MOUSE_DOWN,
              _mouse_down);
        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MOUSE_MOVE,
              _mouse_move);
        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MOUSE_UP,
              _mouse_up);

        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MOUSE_WHEEL,
              _mouse_wheel);

        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MULTI_DOWN,
              _multi_down);

        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MULTI_MOVE,
              _multi_move);

        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_MULTI_UP,
              _multi_up);

        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_KEY_DOWN,
              _key_down_cb);
        evas_object_event_callback_del(wd->target, EVAS_CALLBACK_KEY_UP,
              _key_up_cb);
     }
}

/* START - Event history list handling functions */
/**
 * @internal
 * This function is used to find if device number
 * is found in a list of devices.
 * The list contains devices for refeeding *UP event
 *
 * @ingroup Elm_Gesture_Layer
 */
static int
device_in_pending_list(const void *data1, const void *data2)
{  /* Compare the two device numbers */
   return (((intptr_t) data1) - ((intptr_t) data2));
}

/**
 * @internal
 *
 * This functions adds device to refeed-pending device list
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_add_device_pending(Eina_List *list, void *event, Evas_Callback_Type event_type)
{
   int device = ELM_MOUSE_DEVICE;
   switch(event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
         break;
      case EVAS_CALLBACK_MULTI_DOWN:
         device = ((Evas_Event_Multi_Down *) event)->device;
         break;
      default:
         return list;
     }

   if (!eina_list_search_unsorted_list(list, device_in_pending_list,
            (intptr_t*) device))
     {
        return eina_list_append(list, (intptr_t*) device);
     }

   return list;
}

/**
 * @internal
 *
 * This functions returns pending-device node
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_device_is_pending(Eina_List *list, void *event, Evas_Callback_Type event_type)
{
   int device = ELM_MOUSE_DEVICE;
   switch(event_type)
     {
      case EVAS_CALLBACK_MOUSE_UP:
         break;
      case EVAS_CALLBACK_MULTI_UP:
         device = ((Evas_Event_Multi_Up *) event)->device;
         break;
      default:
        return NULL;
     }

   return eina_list_search_unsorted_list(list, device_in_pending_list,
         (intptr_t *) device);
}

/**
 * @internal
 *
 * This function reports ABORT to all none-detected gestures
 * Then resets test bits for all desired gesures
 * and clears input-events history.
 * note: if no gesture was detected, events from history list
 * are streamed to the widget because it's unused by layer.
 * user may cancel refeed of events by setting repeat events.
 *
 * @param obj The gesture-layer object.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_event_history_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   int i;
   Gesture_Info *p;
   Evas *e = evas_object_evas_get(obj);
   Eina_Bool gesture_found = EINA_FALSE;
   for (i = ELM_GESTURE_FIRST ; i < ELM_GESTURE_LAST; i++)
     {
        p = wd->gesture[i];
        if (p)
          {
             if (p->state == ELM_GESTURE_STATE_END)
               gesture_found = EINA_TRUE;
             else
               {  /* Report ABORT to all gestures that still not finished */
                  _set_state(p, ELM_GESTURE_STATE_ABORT, wd->gesture[i]->info,
                        EINA_FALSE);
               }
          }
     }

   _reset_states(wd); /* we are ready to start testing for gestures again */

   /* Clear all gestures intermediate data */
   if (IS_TESTED(ELM_GESTURE_N_LONG_TAPS))
     {  /* We do not clear a long-tap gesture if fingers still on surface */
        /* and gesture timer still pending to test gesture state          */
        Long_Tap_Type *st = wd->gesture[ELM_GESTURE_N_LONG_TAPS]->data;
        if ((st) &&  /* st not allocated if clear occurs before 1st input */
              ((!eina_list_count(st->touched)) || (!st->timeout)))
          _n_long_tap_test_reset(wd->gesture[ELM_GESTURE_N_LONG_TAPS]);
     }

   if (wd->dbl_timeout)
     {
        ecore_timer_del(wd->dbl_timeout);
        wd->dbl_timeout = NULL;
     }

   _tap_gestures_test_reset(wd->gesture[ELM_GESTURE_N_TAPS]);
   _tap_gestures_test_reset(wd->gesture[ELM_GESTURE_N_DOUBLE_TAPS]);
   _tap_gestures_test_reset(wd->gesture[ELM_GESTURE_N_TRIPLE_TAPS]);
   _momentum_test_reset(wd->gesture[ELM_GESTURE_MOMENTUM]);
   _line_test_reset(wd->gesture[ELM_GESTURE_N_LINES]);
   _line_test_reset(wd->gesture[ELM_GESTURE_N_FLICKS]);
   _zoom_test_reset(wd->gesture[ELM_GESTURE_ZOOM]);
   _rotate_test_reset(wd->gesture[ELM_GESTURE_ROTATE]);

   /* Disable gesture layer so refeeded events won't be consumed by it */
   _unregister_callbacks(obj);
   while (wd->event_history_list)
     {
        Event_History *t;
        t = wd->event_history_list;
        Eina_List *pending = _device_is_pending(wd->pending,
              wd->event_history_list->event,
              wd->event_history_list->event_type);

        /* Refeed events if no gesture matched input */
        if (pending || ((!gesture_found) && (!wd->repeat_events)))
          {
             evas_event_refeed_event(e, wd->event_history_list->event,
                   wd->event_history_list->event_type);

             if (pending)
               {
                  wd->pending = eina_list_remove_list(wd->pending, pending);
               }
             else
               {
                  wd->pending = _add_device_pending(wd->pending,
                        wd->event_history_list->event,
                        wd->event_history_list->event_type);
               }
          }

        free(wd->event_history_list->event);
        wd->event_history_list = (Event_History *) eina_inlist_remove(
              EINA_INLIST_GET(wd->event_history_list),
              EINA_INLIST_GET(wd->event_history_list));
        free(t);
     }
   _register_callbacks(obj);
   return EINA_TRUE;
}

/**
 * @internal
 *
 * This function copies input events.
 * We copy event info before adding it to history.
 * The memory is freed when we clear history.
 *
 * @param event the event to copy
 * @param event_type event type to copy
 *
 * @ingroup Elm_Gesture_Layer
 */
static void *
_copy_event_info(void *event, Evas_Callback_Type event_type)
{
   switch(event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
         return COPY_EVENT_INFO((Evas_Event_Mouse_Down *) event);
         break;
      case EVAS_CALLBACK_MOUSE_MOVE:
         return COPY_EVENT_INFO((Evas_Event_Mouse_Move *) event);
         break;
      case EVAS_CALLBACK_MOUSE_UP:
         return COPY_EVENT_INFO((Evas_Event_Mouse_Up *) event);
         break;
      case EVAS_CALLBACK_MOUSE_WHEEL:
         return COPY_EVENT_INFO((Evas_Event_Mouse_Wheel *) event);
         break;
      case EVAS_CALLBACK_MULTI_DOWN:
         return COPY_EVENT_INFO((Evas_Event_Multi_Down *) event);
         break;
      case EVAS_CALLBACK_MULTI_MOVE:
         return COPY_EVENT_INFO((Evas_Event_Multi_Move *) event);
         break;
      case EVAS_CALLBACK_MULTI_UP:
         return COPY_EVENT_INFO((Evas_Event_Multi_Up *) event);
         break;
      case EVAS_CALLBACK_KEY_DOWN:
         return COPY_EVENT_INFO((Evas_Event_Key_Down *) event);
         break;
      case EVAS_CALLBACK_KEY_UP:
         return COPY_EVENT_INFO((Evas_Event_Key_Up *) event);
         break;
      default:
         return NULL;
     }
}

static Eina_Bool
_event_history_add(Evas_Object *obj, void *event, Evas_Callback_Type event_type)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Event_History *ev;
   if (!wd) return EINA_FALSE;

   ev = malloc(sizeof(Event_History));
   ev->event = _copy_event_info(event, event_type);  /* Freed on event_history_clear */
   ev->event_type = event_type;
   wd->event_history_list = (Event_History *) eina_inlist_append(
         EINA_INLIST_GET(wd->event_history_list), EINA_INLIST_GET(ev));

   return EINA_TRUE;
}
/* END - Event history list handling functions */

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   _event_history_clear(obj);
   eina_list_free(wd->pending);

   Pointer_Event *data;
   EINA_LIST_FREE(wd->touched, data)
      free(data);

   if (!elm_widget_disabled_get(obj))
     _unregister_callbacks(obj);

   /* Free all gestures internal data structures */
   int i;
   for (i = 0; i < ELM_GESTURE_LAST; i++)
     if (wd->gesture[i])
       {
          if (wd->gesture[i]->data)
            free(wd->gesture[i]->data);

          free(wd->gesture[i]);
       }

   free(wd);
}

static int
compare_match_fingers(const void *data1, const void *data2)
{  /* Compare coords of first item in list to cur coords */
   const Pointer_Event *pe1 = eina_list_data_get(data1);
   const Pointer_Event *pe2 = data2;

   if (_inside(pe1->x, pe1->y, pe2->x, pe2->y))
     return 0;
   else if (pe1->x < pe2->x)
     return -1;
   else
     {
        if (pe1->x == pe2->x)
          return pe1->y - pe2->y;
        else
          return 1;
     }
}

static int
compare_pe_device(const void *data1, const void *data2)
{  /* Compare device of first item in list to our pe device */
   const Pointer_Event *pe1 = eina_list_data_get(data1);
   const Pointer_Event *pe2 = data2;

   /* Only match if last was a down event */
   if ((pe1->event_type != EVAS_CALLBACK_MULTI_DOWN) &&
         (pe1->event_type != EVAS_CALLBACK_MOUSE_DOWN))
     return 1;


   if (pe1->device == pe2->device)
     return 0;
   else if (pe1->device < pe2->device)
     return -1;
   else
     return 1;
}

static Eina_List*
_record_pointer_event(Taps_Type *st, Eina_List *pe_list, Pointer_Event *pe,
      Widget_Data *wd, void *event_info, Evas_Callback_Type event_type)
{  /* Keep copy of pe and record it in list */
   Pointer_Event *p = malloc(sizeof(Pointer_Event));
   memcpy(p, pe, sizeof(Pointer_Event));
   consume_event(wd, event_info, event_type, EVAS_EVENT_FLAG_NONE);

   st->sum_x += pe->x;
   st->sum_y += pe->y;
   st->n_taps++;

   /* This will also update middle-point to report to user later */
   st->info.x = st->sum_x / st->n_taps;
   st->info.y = st->sum_y / st->n_taps;
   st->info.timestamp = pe->timestamp;

   if (!pe_list)
     {
        pe_list = eina_list_append(pe_list, p);
        st->l = eina_list_append(st->l, pe_list);
     }
   else
     pe_list = eina_list_append(pe_list, p);

   return pe_list;
}

/**
 * @internal
 *
 * This function sets state a tap-gesture to END or ABORT
 *
 * @param data gesture info pointer
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_tap_gesture_finish(void *data)
{  /* This function will test each tap gesture when timer expires */
   Gesture_Info *gesture = data;
   Elm_Gesture_State s = ELM_GESTURE_STATE_END;
   /* Here we check if taps-gesture was completed successfuly */
   /* Count how many taps were recieved on each device then   */
   /* determine if it matches n_taps_needed defined on START  */
   Taps_Type *st = gesture->data;
   Eina_List *l;
   Eina_List *pe_list;
   EINA_LIST_FOREACH(st->l, l, pe_list)
     {
        if (eina_list_count(pe_list) != st->n_taps_needed)
          {  /* No match taps number on device, ABORT */
             s = ELM_GESTURE_STATE_ABORT;
             break;
          }
     }

   st->info.n = eina_list_count(st->l);
   _set_state(gesture, s, gesture->info, EINA_FALSE);
   _tap_gestures_test_reset(gesture);
}

/**
 * @internal
 *
 * when this timer expires we finish tap gestures.
 *
 * @param data The gesture-layer object.
 * @return cancles callback for this timer.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_multi_tap_timeout(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return EINA_FALSE;

   if (IS_TESTED(ELM_GESTURE_N_TAPS))
     _tap_gesture_finish(wd->gesture[ELM_GESTURE_N_TAPS]);

   if (IS_TESTED(ELM_GESTURE_N_DOUBLE_TAPS))
   _tap_gesture_finish(wd->gesture[ELM_GESTURE_N_DOUBLE_TAPS]);

   if (IS_TESTED(ELM_GESTURE_N_TRIPLE_TAPS))
   _tap_gesture_finish(wd->gesture[ELM_GESTURE_N_TRIPLE_TAPS]);

   _clear_if_finished(data);
   wd->dbl_timeout = NULL;
   return ECORE_CALLBACK_CANCEL;
}

/**
 * @internal
 *
 * when this timer expires we START long tap gesture
 *
 * @param data The gesture-layer object.
 * @return cancles callback for this timer.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_long_tap_timeout(void *data)
{
   Gesture_Info *gesture = data;
   Long_Tap_Type *st = gesture->data;
   st->timeout = NULL;

   _set_state(gesture, ELM_GESTURE_STATE_START,
         gesture->data, EINA_FALSE);

   return ECORE_CALLBACK_CANCEL;
}


/**
 * @internal
 *
 * This function checks if a tap gesture should start
 *
 * @param wd Gesture Layer Widget Data.
 * @param pe The recent input event as stored in pe struct.
 * @param event_info Original input event pointer.
 * @param event_type Type of original input event.
 * @param gesture what gesture is tested
 * @param how many taps for this gesture (1, 2 or 3)
 *
 * @return Flag to determine if we need to set a timer for finish
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_tap_gesture_start(Widget_Data *wd, Pointer_Event *pe,
      void *event_info, Evas_Callback_Type event_type,
      Gesture_Info *gesture, int taps)
{  /* Here we fill Tap struct */
   Taps_Type *st = gesture->data;
   if (!st)
     {  /* Allocated once on first time */
        st = calloc(1, sizeof(Taps_Type));
        gesture->data = st;
        _tap_gestures_test_reset(gesture);
     }

   Eina_List *pe_list = NULL;
   Pointer_Event *pe_down = NULL;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   switch (pe->event_type)
     {
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_DOWN:
         /* Check if got tap on same cord was tapped before */
         pe_list = eina_list_search_unsorted(st->l, compare_match_fingers, pe);

         if ((!pe_list) &&
               eina_list_search_unsorted(st->l, compare_pe_device, pe))
           {  /* This device was touched in other cord before completion */
              ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT,
                    &st->info, EINA_FALSE);
              consume_event(wd, event_info, event_type, ev_flag);

              return EINA_FALSE;
           }

         pe_list = _record_pointer_event(st, pe_list, pe, wd, event_info, event_type);
         if ((pe->device == 0) && (eina_list_count(pe_list) == 1))
           {  /* This is the first mouse down we got */
              ev_flag = _set_state(gesture, ELM_GESTURE_STATE_START,
                    &st->info, EINA_FALSE);
              consume_event(wd, event_info, event_type, ev_flag);

              st->n_taps_needed = taps * 2; /* count DOWN and UP */

              return EINA_TRUE;
           }

         break;

      case EVAS_CALLBACK_MULTI_UP:
      case EVAS_CALLBACK_MOUSE_UP:
         pe_list = eina_list_search_unsorted(st->l, compare_pe_device, pe);
         if (!pe_list)
           return EINA_FALSE;

         pe_list = _record_pointer_event(st, pe_list, pe, wd, event_info, event_type);
         break;

      case EVAS_CALLBACK_MULTI_MOVE:
      case EVAS_CALLBACK_MOUSE_MOVE:
         /* Get first event in first list, this has to be a Mouse Down event  */
         /* and verify that user didn't move out of this area before next tap */
         pe_list = eina_list_search_unsorted(st->l, compare_pe_device, pe);
         if (pe_list)
           {
              pe_down = eina_list_data_get(pe_list);
              if (!_inside(pe_down->x, pe_down->y, pe->x, pe->y))
                {
                   ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT,
                         &st->info, EINA_FALSE);
                   consume_event(wd, event_info, event_type, ev_flag);
                }
           }
         break;

      default:
         return EINA_FALSE;
     }

   return EINA_FALSE;
}


/**
 * @internal
 *
 * This function checks all click/tap and double/triple taps
 *
 * @param obj The gesture-layer object.
 * @param pe The recent input event as stored in pe struct.
 * @param event_info Original input event pointer.
 * @param event_type Type of original input event.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_tap_gestures_test(Evas_Object *obj, Pointer_Event *pe,
      void *event_info, Evas_Callback_Type event_type)
{  /* Here we fill Recent_Taps struct and fire-up click/tap timers */
   Eina_Bool need_timer = EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (!pe)   /* this happens when unhandled event arrived */
     return;  /* see _make_pointer_event function */

   if (IS_TESTED(ELM_GESTURE_N_TAPS))
     need_timer |= _tap_gesture_start(wd, pe, event_info, event_type,
           wd->gesture[ELM_GESTURE_N_TAPS], 1);

   if (IS_TESTED(ELM_GESTURE_N_DOUBLE_TAPS))
     need_timer |= _tap_gesture_start(wd, pe, event_info, event_type,
           wd->gesture[ELM_GESTURE_N_DOUBLE_TAPS], 2);

   if (IS_TESTED(ELM_GESTURE_N_TRIPLE_TAPS))
     need_timer |= _tap_gesture_start(wd, pe, event_info, event_type,
           wd->gesture[ELM_GESTURE_N_TRIPLE_TAPS], 3);

   if ((need_timer) && (!wd->dbl_timeout))
     {  /* Set a timer to finish these gestures */
        wd->dbl_timeout = ecore_timer_add(0.4, _multi_tap_timeout,
              obj);
     }
}

/**
 * @internal
 *
 * This function computes center-point for  long-tap gesture
 *
 * @param st Long Tap gesture info pointer
 * @param pe The recent input event as stored in pe struct.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_compute_taps_center(Long_Tap_Type *st,
      Evas_Coord *x_out, Evas_Coord *y_out, Pointer_Event *pe)
{
   if(!eina_list_count(st->touched))
     return;

   Eina_List *l;
   Pointer_Event *p;
   Evas_Coord x = 0, y = 0;
   EINA_LIST_FOREACH(st->touched, l, p)
     {  /* Accumulate all then take avarage */
        if (p->device == pe->device)
          {  /* This will take care of values coming from MOVE event */
             x += pe->x;
             y += pe->y;
          }
        else
          {
             x += p->x;
             y += p->y;
          }
     }

   *x_out = x / eina_list_count(st->touched);
   *y_out = y / eina_list_count(st->touched);
}

/**
 * @internal
 *
 * This function checks N long-tap gesture.
 *
 * @param obj The gesture-layer object.
 * @param pe The recent input event as stored in pe struct.
 * @param event_info Original input event pointer.
 * @param event_type Type of original input event.
 * @param g_type what Gesture we are testing.
 * @param taps How many click/taps we test for.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_n_long_tap_test(Evas_Object *obj, Pointer_Event *pe,
                 void *event_info, Evas_Callback_Type event_type,
                 Elm_Gesture_Types g_type)
{  /* Here we fill Recent_Taps struct and fire-up click/tap timers */
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (!pe)   /* this happens when unhandled event arrived */
     return;  /* see _make_pointer_event function */
   Gesture_Info *gesture = wd->gesture[g_type];
   if (!gesture) return;

   Long_Tap_Type *st = gesture->data;
   if (!st)
     {  /* Allocated once on first time */
        st = calloc(1, sizeof(Long_Tap_Type));
        gesture->data = st;
        _n_long_tap_test_reset(gesture);
     }

   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   switch (pe->event_type)
     {
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_DOWN:
        st->touched = _add_touched_device(st->touched, pe);
        st->info.n = eina_list_count(st->touched);
        if (st->info.n > st->max_touched)
          st->max_touched = st->info.n;
        else
          {  /* User removed finger from touch, then put back - ABORT */
             if ((gesture->state == ELM_GESTURE_STATE_START) ||
                 (gesture->state == ELM_GESTURE_STATE_MOVE))
               {
                  ev_flag =_set_state(gesture, ELM_GESTURE_STATE_ABORT,
                                      &st->info, EINA_FALSE);
                  consume_event(wd, event_info, event_type, ev_flag);
               }
          }

        if ((pe->device == 0) && (eina_list_count(st->touched) == 1))
          {  /* This is the first mouse down we got */
             st->info.timestamp = pe->timestamp;

             /* To test long tap */
             /* When this timer expires, gesture STARTED */
             if (!st->timeout)
               st->timeout = ecore_timer_add(wd->long_tap_start_timeout,
                                             _long_tap_timeout, gesture);
          }

        consume_event(wd, event_info, event_type, ev_flag);
        _compute_taps_center(st, &st->info.x, &st->info.y, pe);
        st->center_x = st->info.x;
        st->center_y = st->info.y;
        break;

      case EVAS_CALLBACK_MULTI_UP:
      case EVAS_CALLBACK_MOUSE_UP:
        st->touched = _remove_touched_device(st->touched, pe);
        _compute_taps_center(st, &st->center_x, &st->center_y, pe);
        if (st->info.n &&
            ((gesture->state == ELM_GESTURE_STATE_START) ||
                (gesture->state == ELM_GESTURE_STATE_MOVE)))
          {  /* Report END only for gesture that STARTed */
             if (eina_list_count(st->touched) == 0)
               {  /* Report END only at last release event */
                  ev_flag =_set_state(gesture, ELM_GESTURE_STATE_END,
                                      &st->info, EINA_FALSE);
                  consume_event(wd, event_info, event_type, ev_flag);
               }
          }
        else
          {  /* Stop test, user lifts finger before long-start */
             if (st->timeout) ecore_timer_del(st->timeout);
             st->timeout = NULL;
             ev_flag =_set_state(gesture, ELM_GESTURE_STATE_ABORT,
                                 &st->info, EINA_FALSE);
             consume_event(wd, event_info, event_type, ev_flag);
          }

        break;

      case EVAS_CALLBACK_MULTI_MOVE:
      case EVAS_CALLBACK_MOUSE_MOVE:
        if(st->info.n &&
           ((gesture->state == ELM_GESTURE_STATE_START) ||
               (gesture->state == ELM_GESTURE_STATE_MOVE)))
          {  /* Report MOVE only if STARTED */
             Evas_Coord x = 0;
             Evas_Coord y = 0;
             Elm_Gesture_State state_to_report = ELM_GESTURE_STATE_MOVE;

             _compute_taps_center(st, &x, &y, pe);
             /* ABORT if user moved fingers out of tap area */
#if defined(DEBUG_GESTURE_LAYER)
             printf("%s x,y=(%d,%d) st->info.x,st->info.y=(%d,%d)\n",__func__,x,y,st->info.x,st->info.y);
#endif
             if (!_inside(x, y, st->center_x, st->center_y))
               state_to_report = ELM_GESTURE_STATE_ABORT;

             /* Report MOVE if gesture started */
             ev_flag = _set_state(gesture, state_to_report,
                                  &st->info, EINA_TRUE);
             consume_event(wd, event_info, event_type, ev_flag);
          }
        break;

      default:
        return;
     }
}

/**
 * @internal
 *
 * This function computes momentum for MOMENTUM, LINE and FLICK gestures
 * This momentum value will be sent to widget when gesture is completed.
 *
 * @param momentum pointer to buffer where we record momentum value.
 * @param x1 x coord where user started gesture.
 * @param y1 y coord where user started gesture.
 * @param x2 x coord where user completed gesture.
 * @param y2 y coord where user completed gesture.
 * @param t1x timestamp for X, when user started gesture.
 * @param t1y timestamp for Y, when user started gesture.
 * @param t2  timestamp when user completed gesture.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_set_momentum(Elm_Gesture_Momentum_Info *momentum, Evas_Coord x1, Evas_Coord y1,
      Evas_Coord x2, Evas_Coord y2, unsigned int t1x, unsigned int t1y,
      unsigned int t2)
{
   Evas_Coord velx = 0, vely = 0, vel;
   Evas_Coord dx = x2 - x1;
   Evas_Coord dy = y2 - y1;
   int dtx = t2 - t1x;
   int dty = t2 - t1y;
   if (dtx > 0)
     velx = (dx * 1000) / dtx;

   if (dty > 0)
     vely = (dy * 1000) / dty;

   vel = sqrt((velx * velx) + (vely * vely));

   if ((_elm_config->thumbscroll_friction > 0.0) &&
         (vel > _elm_config->thumbscroll_momentum_threshold))
     {  /* report momentum */
        momentum->mx = velx;
        momentum->my = vely;
     }
   else
     {
        momentum->mx = 0;
        momentum->my = 0;
     }
}

/**
 * @internal
 *
 * This function is used for computing rotation angle (DEG).
 *
 * @param x1 first finger x location.
 * @param y1 first finger y location.
 * @param x2 second finger x location.
 * @param y2 second finger y location.
 *
 * @return angle of the line between (x1,y1), (x2,y2) in Radians.
 *
 * @ingroup Elm_Gesture_Layer
 */
static double
get_angle(Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2)
{
   double a, xx, yy;
   xx = fabs(x2 - x1);
   yy = fabs(y2 - y1);

   if (((int) xx) && ((int) yy))
     {
        a = atan(yy / xx);
        if (x1 < x2)
          {
             if (y1 < y2)
               {
                  return RAD_360DEG - a;
               }
             else
               {
                  return (a);
               }
          }
        else
          {
             if (y1 < y2)
               {
                  return RAD_180DEG + a;
               }
             else
               {
                  return RAD_180DEG - a;
               }
          }
     }

   if (((int) xx))
     {  /* Horizontal line */
        if (x2 < x1)
          {
             return RAD_180DEG;
          }
        else
          {
             return 0.0;
          }
     }

   /* Vertical line */
   if (y2 < y1)
     {
        return RAD_90DEG;
     }
   else
     {
        return RAD_270DEG;
     }
}

/**
 * @internal
 *
 * This function is used for computing the magnitude and direction
 * of vector between two points.
 *
 * @param x1 first finger x location.
 * @param y1 first finger y location.
 * @param x2 second finger x location.
 * @param y2 second finger y location.
 * @param l length computed (output)
 * @param a angle computed (output)
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
get_vector(Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2,
      Evas_Coord *l, double *a)
{
   Evas_Coord xx, yy;
   xx = x2 - x1;
   yy = y2 - y1;
   *l = (Evas_Coord) sqrt(xx*xx + yy*yy);
   *a = get_angle(x1, y1, x2, y2);
}

static int
_get_direction(Evas_Coord x1, Evas_Coord x2)
{
   if (x1 == x2)
     return 0;
   else if (x2 < x1)
     return -1;
   else
     return 1;
}
/**
 * @internal
 *
 * This function tests momentum gesture.
 * @param obj The gesture-layer object.
 * @param pe The recent input event as stored in pe struct.
 * @param event_info recent input event.
 * @param event_type recent event type.
 * @param g_type what Gesture we are testing.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_momentum_test(Evas_Object *obj, Pointer_Event *pe,
      void *event_info, Evas_Callback_Type event_type,
      Elm_Gesture_Types g_type)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Gesture_Info *gesture = wd->gesture[g_type];
   if (!gesture ) return;

   if (!eina_list_count(wd->touched))
     return; /* Got move on mouse-over move */

   Momentum_Type *st = gesture->data;
   Elm_Gesture_State state_to_report = ELM_GESTURE_STATE_MOVE;
   if (!st)
     {  /* Allocated once on first time */
        st = calloc(1, sizeof(Momentum_Type));
        gesture->data = st;
        _momentum_test_reset(gesture);
     }

   if (!pe)
     return;

   /* First make avarage of all touched devices to determine center point */
   Eina_List *l;
   Pointer_Event *p;
   Pointer_Event pe_local = *pe;           /* Copy pe event info to local */
   unsigned int cnt = 1;    /* We start counter counting current pe event */
   EINA_LIST_FOREACH(wd->touched, l, p)
      if (p->device != pe_local.device)
        {
           pe_local.x += p->x;
           pe_local.y += p->y;
           cnt++;
        }


   /* Compute avarage to get center point */
   pe_local.x /= cnt;
   pe_local.y /= cnt;

   /* If user added finger - reset gesture */
   if ((st->info.n) && (st->info.n < cnt))
     state_to_report = ELM_GESTURE_STATE_ABORT;

   if (st->info.n < cnt)
     st->info.n = cnt;

   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
         if (!st->t_st_x)
           {
              if ((event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
                    (event_type == EVAS_CALLBACK_MULTI_DOWN) ||
                    (wd->glayer_continues_enable)) /* start also on MOVE */
                {  /* We start on MOVE when cont-enabled only */
                   st->line_st.x = st->line_end.x = pe_local.x;
                   st->line_st.y = st->line_end.y = pe_local.y;
                   st->t_st_x = st->t_st_y = st->t_end = pe_local.timestamp;
                   st->xdir = st->ydir = 0;
                   st->info.x2 = st->info.x1 = pe_local.x;
                   st->info.y2 = st->info.y1 = pe_local.y;
                   st->info.tx = st->info.ty = pe_local.timestamp;
                   ev_flag = _set_state(gesture, ELM_GESTURE_STATE_START,
                         &st->info, EINA_FALSE);
                   consume_event(wd, event_info, event_type, ev_flag);
                }

              return;
           }


         /* ABORT gesture if got DOWN or MOVE event after UP+timeout */
         if ((st->t_up) &&
         ((st->t_up + ELM_GESTURE_MULTI_TIMEOUT) < pe_local.timestamp))
           state_to_report = ELM_GESTURE_STATE_ABORT;

         if ((pe_local.timestamp - ELM_GESTURE_MOMENTUM_TIMEOUT) > st->t_end)
           {  /*  Too long of a wait, reset all values */
              st->line_st.x = pe_local.x;
              st->line_st.y = pe_local.y;
              st->t_st_y = st->t_st_x = pe_local.timestamp;
              st->info.tx = st->t_st_x;
              st->info.ty = st->t_st_y;
              st->xdir = st->ydir = 0;
           }
         else
           {
              int xdir, ydir;
              xdir = _get_direction(st->line_st.x, pe_local.x);
              ydir = _get_direction(st->line_st.y, pe_local.y);
              if (!xdir || (xdir == (-st->xdir)))
                {
                   st->line_st.x = st->line_end.x;
                   st->info.tx = st->t_st_x = st->t_end;
                   st->xdir = xdir;
                }

              if (!ydir || (ydir == (-st->ydir)))
                {
                   st->line_st.y = st->line_end.y;
                   st->info.ty = st->t_st_y = st->t_end;
                   st->ydir = ydir;
                }
           }

         st->info.x2 = st->line_end.x = pe_local.x;
         st->info.y2 = st->line_end.y = pe_local.y;
         st->t_end = pe_local.timestamp;
         _set_momentum(&st->info, st->line_st.x, st->line_st.y, pe_local.x, pe_local.y,
               st->t_st_x, st->t_st_y, pe_local.timestamp);
         ev_flag = _set_state(gesture, state_to_report, &st->info,
               EINA_TRUE);
         consume_event(wd, event_info, event_type, ev_flag);
         break;


      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
         st->t_up = pe_local.timestamp;       /* Record recent up event time */
         if ((cnt > 1) ||     /* Ignore if more fingers touch surface        */
               (!st->t_st_x)) /* IGNORE if info was cleared, long press,move */
           return;

         if ((pe_local.timestamp - ELM_GESTURE_MOMENTUM_TIMEOUT) > st->t_end)
           {  /* Too long of a wait, reset all values */
              st->line_st.x = pe_local.x;
              st->line_st.y = pe_local.y;
              st->t_st_y = st->t_st_x = pe_local.timestamp;
              st->xdir = st->ydir = 0;
           }

         st->info.x2 = pe_local.x;
         st->info.y2 = pe_local.y;
         st->line_end.x = pe_local.x;
         st->line_end.y = pe_local.y;
         st->t_end = pe_local.timestamp;

         _set_momentum(&st->info, st->line_st.x, st->line_st.y, pe_local.x, pe_local.y,
               st->t_st_x, st->t_st_y, pe_local.timestamp);

         ev_flag = _set_state(gesture, ELM_GESTURE_STATE_END, &st->info,
               EINA_FALSE);
         consume_event(wd, event_info, event_type, ev_flag);
         return;

      default:
         return;
     }
}

static int
compare_line_device(const void *data1, const void *data2)
{  /* Compare device component of line struct */
   const Line_Data *ln1 = data1;
   const int *device = data2;

   if (ln1->t_st) /* Compare only with lines that started */
     return (ln1->device - (*device));

   return (-1);
}

/**
 * @internal
 *
 * This function construct line struct from input.
 * @param info pointer to store line momentum.
 * @param st line info to store input data.
 * @param pe The recent input event as stored in pe struct.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_single_line_process(Elm_Gesture_Line_Info *info, Line_Data *st,
      Pointer_Event *pe, Evas_Callback_Type event_type)
{  /* Record events and set momentum for line pointed by st */
   if (!pe)
     return EINA_FALSE;

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MULTI_MOVE:
         if (!st->t_st)
           {  /* This happens only when line starts */
              st->line_st.x = pe->x;
              st->line_st.y = pe->y;
              st->t_st = pe->timestamp;
              st->device = pe->device;
              info->momentum.x1 = pe->x;
              info->momentum.y1 = pe->y;
              info->momentum.tx = pe->timestamp;
              info->momentum.ty = pe->timestamp;

              return EINA_TRUE;
           }

         break;

      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
         /* IGNORE if line info was cleared, like long press, move */
         if (!st->t_st)
           return EINA_FALSE;

         st->line_end.x = pe->x;
         st->line_end.y = pe->y;
         st->t_end = pe->timestamp;
         break;

      default:
         return EINA_FALSE;
     }

   if (!st->t_st)
     {
        _line_data_reset(st);
        return EINA_FALSE;
     }

   info->momentum.x2 = pe->x;
   info->momentum.y2 = pe->y;
   _set_momentum(&info->momentum, st->line_st.x, st->line_st.y, pe->x, pe->y,
         st->t_st, st->t_st, pe->timestamp);

   return EINA_TRUE;
}

/**
 * @internal
 *
 * This function test for (n) line gesture.
 * @param obj The gesture-layer object.
 * @param pe The recent input event as stored in pe struct.
 * @param event_info Original input event pointer.
 * @param event_type Type of original input event.
 * @param g_type what Gesture we are testing.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_n_line_test(Evas_Object *obj, Pointer_Event *pe, void *event_info,
      Evas_Callback_Type event_type, Elm_Gesture_Types g_type)
{
   if (!pe)
     return;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Gesture_Info *gesture = wd->gesture[g_type];
   if (!gesture ) return;

   if (!eina_list_count(wd->touched))
     return; /* Got move on mouse-over move */

   Line_Type *st = gesture->data;
   if (!st)
     {
        st = calloc(1, sizeof(Line_Type));
        gesture->data = st;
     }

   Line_Data *line = NULL;
   Eina_List *list = st->list;
   unsigned cnt = eina_list_count(list);

   if (cnt)
     {  /* list is not empty, locate this device on list */
        line = (Line_Data *) eina_list_search_unsorted(st->list,
              compare_line_device, &pe->device);
     }

   if (!line)
     {  /* List is empty or device not found, new line-struct on START only */
        if ((event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
              (event_type == EVAS_CALLBACK_MULTI_DOWN) ||
              ((wd->glayer_continues_enable) && /* START on MOVE also */
               ((event_type == EVAS_CALLBACK_MOUSE_MOVE) ||
                (event_type == EVAS_CALLBACK_MULTI_MOVE))))
          {  /* Allocate new item on START only */
             line = calloc(1, sizeof(Line_Data));
             _line_data_reset(line);
             list = eina_list_append(list, line);
             st->list = list;
          }
     }

   if (!line)  /* This may happen on MOVE that comes before DOWN      */
     return;   /* No line-struct to work with, can't continue testing */

   if (_single_line_process(&st->info, line, pe, event_type)) /* update st with input */
     consume_event(wd, event_info, event_type, EVAS_EVENT_FLAG_NONE);

   /* Get direction and magnitude of the line */
   double angle;
   get_vector(line->line_st.x, line->line_st.y, pe->x, pe->y,
         &line->line_length, &angle);

   /* These are used later to compare lines length */
   Evas_Coord shortest_line_len = line->line_length;
   Evas_Coord longest_line_len = line->line_length;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;

   /* Now update line-state */
   if (line->t_st)
     {  /* Analyze line only if line started */
        if (line->line_angle >= 0.0)
          {  /* if line direction was set, we test if broke tolerance */
             double a = fabs(angle - line->line_angle);

             double d = (tan(a)) * line->line_length; /* Distance from line */
#if defined(DEBUG_GESTURE_LAYER)
             printf("%s a=<%f> d=<%f>\n", __func__, (a * 57.295779513), d);
#endif
             if ((d > wd->line_distance_tolerance) || (a > wd->line_angular_tolerance))
               {  /* Broke tolerance: abort line and start a new one */
                  ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT,
                        &st->info, EINA_FALSE);
                  consume_event(wd, event_info, event_type, ev_flag);
                  return;
               }

             if (wd->glayer_continues_enable)
               {  /* We may finish line if momentum is zero */
                  /* This is for continues-gesture */
                  if ((!st->info.momentum.mx) && (!st->info.momentum.my))
                    {  /* Finish line on zero momentum for continues gesture */
                       line->line_end.x = pe->x;
                       line->line_end.y = pe->y;
                       line->t_end = pe->timestamp;
                    }
               }
          }
        else
          {  /* Record the line angle as it broke minimum length for line */
             if (line->line_length >= wd->line_min_length)
               st->info.angle = line->line_angle = angle;
          }


        if (line->t_end)
          {
             if (line->line_angle < 0.0)
               { /* it's not a line, too short more close to a tap */
                  ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT,
                        &st->info, EINA_FALSE);
                  consume_event(wd, event_info, event_type, ev_flag);
                  return;
               }
          }
     }

   /* Count how many lines already started / ended */
   int started = 0;
   int ended = 0;
   unsigned int tm_start = pe->timestamp;
   unsigned int tm_end = pe->timestamp;
   Eina_List *l;
   Line_Data *t_line;
   double base_angle = ELM_GESTURE_NEGATIVE_ANGLE;
   Eina_Bool lines_parallel = EINA_TRUE;
   EINA_LIST_FOREACH(list, l, t_line)
     {
        if (base_angle < 0)
          base_angle = t_line->line_angle;
        else
          {
             if (t_line->line_angle >= 0)
               {  /* Compare angle only with lines with direction defined */
                  if (fabs(base_angle - t_line->line_angle) >
                        wd->line_angular_tolerance)
                    lines_parallel = EINA_FALSE;
               }
          }

        if (t_line->line_length)
          {  /* update only if this line is used */
             if (shortest_line_len > t_line->line_length)
               shortest_line_len = t_line->line_length;

             if (longest_line_len < t_line->line_length)
               longest_line_len = t_line->line_length;
          }

        if (t_line->t_st)
          {
             started++;
             if (t_line->t_st < tm_start)
               tm_start = t_line->t_st;
          }

        if (t_line->t_end)
          {
             ended++;
             if (t_line->t_end < tm_end)
               tm_end = t_line->t_end;
          }
     }

   st->info.momentum.n = started;


   if (ended &&
         ((event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
          (event_type == EVAS_CALLBACK_MULTI_DOWN)))
     {  /* user lift one finger then starts again without line-end - ABORT */
        ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
              EINA_FALSE);
        consume_event(wd, event_info, event_type, ev_flag);
        return;
     }

   if (!lines_parallel)
     { /* Lines are NOT at same direction, abort this gesture */
        ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
              EINA_FALSE);
        consume_event(wd, event_info, event_type, ev_flag);
        return;
     }


   /* We report ABORT if lines length are NOT matching when fingers are up */
   if ((longest_line_len - shortest_line_len) > (elm_finger_size_get()*2))
     {
        ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
              EINA_FALSE);
        consume_event(wd, event_info, event_type, ev_flag);
        return;
     }

   if ((g_type == ELM_GESTURE_N_FLICKS) && ((tm_end - tm_start) > wd->flick_time_limit_ms))
     {  /* We consider FLICK as a fast line.ABORT if take too long to finish */
        ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
              EINA_FALSE);
        consume_event(wd, event_info, event_type, ev_flag);
        return;
     }

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
         if ((started) && (started == ended))
           {
              ev_flag = _set_state(gesture, ELM_GESTURE_STATE_END,
                    &st->info, EINA_FALSE);
              consume_event(wd, event_info, event_type, ev_flag);
           }

         return;

      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
         if (started)
           {
              if (wd->glayer_continues_enable && (started == ended))
                {  /* For continues gesture */
                   ev_flag = _set_state(gesture, ELM_GESTURE_STATE_END,
                         &st->info, EINA_FALSE);
                   consume_event(wd, event_info, event_type, ev_flag);
                }
              else
                {  /* When continues, may START on MOVE event too */
                   Elm_Gesture_State s = ELM_GESTURE_STATE_MOVE;

                   /* This happens when: on n > 1 lines then one finger up */
                   /* caused abort, then put finger down.                  */
                   /* This will stop line from starting again.             */
                   /* Number of lines, MUST match touched-device in list   */
                   if ((!wd->glayer_continues_enable) &&
                         (eina_list_count(st->list) < eina_list_count(wd->touched)))
                     s = ELM_GESTURE_STATE_ABORT;

                   if (gesture->state == ELM_GESTURE_STATE_UNDEFINED)
                     s = ELM_GESTURE_STATE_START;

                   ev_flag = _set_state(gesture, s, &st->info, EINA_TRUE);
                   consume_event(wd, event_info, event_type, ev_flag);
                }
           }
         break;

      default:
         return;  /* Unhandeld event type */
     }
}

/**
 * @internal
 *
 * This function is used to check if rotation gesture started.
 * @param st Contains current rotation values from user input.
 * @return TRUE/FALSE if we need to set rotation START.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
rotation_broke_tolerance(Rotate_Type *st)
{
   if (st->info.base_angle < 0)
     return EINA_FALSE; /* Angle has to be computed first */

   if (st->rotate_angular_tolerance < 0)
     return EINA_TRUE;

   double low  = st->info.base_angle - st->rotate_angular_tolerance;
   double high = st->info.base_angle + st->rotate_angular_tolerance;
   double t = st->info.angle;

   if (low < 0)
     {
        low += RAD_180DEG;
        high += RAD_180DEG;

        if (t < RAD_180DEG)
          t += RAD_180DEG;
        else
          t -= RAD_180DEG;
     }

   if (high > RAD_360DEG)
     {
        low -= RAD_180DEG;
        high -= RAD_180DEG;

        if (t < RAD_180DEG)
          t += RAD_180DEG;
        else
          t -= RAD_180DEG;
     }

#if defined(DEBUG_GESTURE_LAYER)
   printf("%s angle=<%f> low=<%f> high=<%f>\n", __func__, t, low, high);
#endif
   if ((t < low) || (t > high))
     {  /* This marks that roation action has started */
        st->rotate_angular_tolerance = ELM_GESTURE_NEGATIVE_ANGLE;
        st->info.base_angle = st->info.angle; /* Avoid jump in angle value */
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

/**
 * @internal
 *
 * This function is used for computing the gap between fingers.
 * It returns the length and center point between fingers.
 *
 * @param x1 first finger x location.
 * @param y1 first finger y location.
 * @param x2 second finger x location.
 * @param y2 second finger y location.
 * @param x  Gets center point x cord (output)
 * @param y  Gets center point y cord (output)
 *
 * @return length of the line between (x1,y1), (x2,y2) in pixels.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Evas_Coord
get_finger_gap_length(Evas_Coord x1, Evas_Coord y1, Evas_Coord x2,
      Evas_Coord y2, Evas_Coord *x, Evas_Coord *y)
{
   double a, b, xx, yy, gap;
   xx = fabs(x2 - x1);
   yy = fabs(y2 - y1);
   gap = sqrt(xx*xx + yy*yy);

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
   if (((int) xx) && ((int) yy))
     {
        double A = atan((yy / xx));
#if defined(DEBUG_GESTURE_LAYER)
        printf("xx=<%f> yy=<%f> A=<%f>\n", xx, yy, A);
#endif
        a = (Evas_Coord) ((gap / 2) * sin(A));
        b = (Evas_Coord) ((gap / 2) * cos(A));
        *x = (Evas_Coord) ((x2 > x1) ? (x1 + b) : (x2 + b));
        *y = (Evas_Coord) ((y2 > y1) ? (y1 + a) : (y2 + a));
     }
   else
     {
        if ((int) xx)
          {  /* horiz line, take half width */
#if defined(DEBUG_GESTURE_LAYER)
             printf("==== HORIZ ====\n");
#endif
             *x = (Evas_Coord) (xx / 2);
             *y = (Evas_Coord) (y1);
          }

        if ((int) yy)
          {  /* vert line, take half width */
#if defined(DEBUG_GESTURE_LAYER)
             printf("==== VERT ====\n");
#endif
             *x = (Evas_Coord) (x1);
             *y = (Evas_Coord) (yy / 2);
          }
     }
   /* END   - Compute zoom center point */

   return (Evas_Coord) gap;
}

/**
 * @internal
 *
 * This function is used for computing zoom value.
 *
 * @param st Pointer to zoom data based on user input.
 * @param x1 first finger x location.
 * @param y1 first finger y location.
 * @param x2 second finger x location.
 * @param y2 second finger y location.
 * @param factor zoom-factor, used to determine how fast zoom works.
 *
 * @return zoom value, when 1.0 means no zoom, 0.5 half size...
 *
 * @ingroup Elm_Gesture_Layer
 */
static double
compute_zoom(Zoom_Type *st, Evas_Coord x1, Evas_Coord y1, unsigned int tm1,
      Evas_Coord x2, Evas_Coord y2, unsigned int tm2, double zoom_finger_factor)
{
   double rt = 1.0;
   Evas_Coord diam = get_finger_gap_length(x1, y1, x2, y2,
         &st->info.x, &st->info.y);

   st->info.radius = diam / 2;

   if (!st->zoom_base)
     {
        st->zoom_base = diam;
        return st->info.zoom;
     }

   if (st->zoom_distance_tolerance)
     {  /* zoom tolerance <> ZERO, means zoom action NOT started yet */
        if (diam < (st->zoom_base - st->zoom_distance_tolerance))
          {  /* avoid jump with zoom value when break tolerance */
             st->zoom_base -= st->zoom_distance_tolerance;
             st->zoom_distance_tolerance = 0;
          }

        if (diam > (st->zoom_base + st->zoom_distance_tolerance))
          {  /* avoid jump with zoom value when break tolerance */
             st->zoom_base += st->zoom_distance_tolerance;
             st->zoom_distance_tolerance = 0;
          }

        return rt;
     }

   /* We use factor only on the difference between gap-base   */
   /* if gap=120, base=100, we get ((120-100)/100)=0.2*factor */
   rt = ((1.0) + ((((float) diam - (float) st->zoom_base) /
               (float) st->zoom_base) * zoom_finger_factor));

#if 0
   /* Momentum: zoom per second: (NOT YET SUPPORTED) */
   st->info.momentum = (((rt - 1.0) * 1000) / (tm2 - tm1));
#else
   (void) tm1;
   (void) tm2;
#endif
   return rt;
}

/**
 * @internal
 *
 * This function handles zoom with mouse wheel.
 * thats a combination of wheel + CTRL key.
 * @param obj The gesture-layer object.
 * @param event_info Original input event pointer.
 * @param event_type Type of original input event.
 * @param g_type what Gesture we are testing.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_zoom_with_wheel_test(Evas_Object *obj, void *event_info,
      Evas_Callback_Type event_type, Elm_Gesture_Types g_type)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->gesture[g_type]) return;

   Gesture_Info *gesture_zoom = wd->gesture[g_type];
   Zoom_Type *st = gesture_zoom->data;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   if (!st)
     {  /* Allocated once on first time, used for zoom intermediate data */
        st = calloc(1, sizeof(Zoom_Type));
        gesture_zoom->data = st;
        _zoom_test_reset(gesture_zoom);
     }

   switch (event_type)
     {
      case EVAS_CALLBACK_KEY_UP:
           {
              Evas_Event_Key_Up *p = event_info;
              if ((!strcmp(p->keyname, "Control_L")) ||
                    (!strcmp(p->keyname, "Control_R")))
                {  /* Test if we ended a zoom gesture when releasing CTRL */
                   if ((st->zoom_wheel) &&
                         ((gesture_zoom->state == ELM_GESTURE_STATE_START) ||
                          (gesture_zoom->state == ELM_GESTURE_STATE_MOVE)))
                     {  /* User released CTRL after zooming */
                        ev_flag = _set_state(gesture_zoom,
                              ELM_GESTURE_STATE_END, &st->info, EINA_FALSE);
                        consume_event(wd, event_info, event_type, ev_flag);

                        return;
                     }
                }
              break;
           }

      case EVAS_CALLBACK_MOUSE_WHEEL:
           {
              Eina_Bool force;
              Elm_Gesture_State s;
              if (!evas_key_modifier_is_set(
                       ((Evas_Event_Mouse_Wheel *) event_info)->modifiers,
                       "Control"))
                {  /* if using wheel witout CTRL after starting zoom */
                   if ((st->zoom_wheel) &&
                         ((gesture_zoom->state == ELM_GESTURE_STATE_START) ||
                          (gesture_zoom->state == ELM_GESTURE_STATE_MOVE)))
                     {
                        ev_flag = _set_state(gesture_zoom,
                              ELM_GESTURE_STATE_END, &st->info, EINA_FALSE);
                        consume_event(wd, event_info, event_type, ev_flag);

                        return;
                     }
                   else
                     return; /* Ignore mouse-wheel without control */
                }

              /* Using mouse wheel with CTRL for zoom */
              if (st->zoom_wheel || (st->zoom_distance_tolerance == 0))
                {  /* when (zoom_wheel == NULL) and (zoom_distance_tolerance == 0)
                      we continue a zoom gesture */
                   force = EINA_TRUE;
                   s = ELM_GESTURE_STATE_MOVE;
                }
              else
                {  /* On first wheel event, report START */
                   Evas_Modifier_Mask mask = evas_key_modifier_mask_get(
                            evas_object_evas_get(wd->target), "Control");
                   force = EINA_FALSE;
                   s = ELM_GESTURE_STATE_START;
                   if (!evas_object_key_grab(wd->target, "Control_L", mask, 0, EINA_FALSE))
                     ERR("Failed to Grabbed CTRL_L");
                   if (!evas_object_key_grab(wd->target, "Control_R", mask, 0, EINA_FALSE))
                     ERR("Failed to Grabbed CTRL_R");
                }

              st->zoom_distance_tolerance = 0; /* Cancel tolerance */
              st->zoom_wheel = (Evas_Event_Mouse_Wheel *) event_info;
              st->info.x  = st->zoom_wheel->canvas.x;
              st->info.y  = st->zoom_wheel->canvas.y;

              if (st->zoom_wheel->z < 0) /* zoom in */
                st->info.zoom += (wd->zoom_finger_factor * wd->zoom_wheel_factor);

              if (st->zoom_wheel->z > 0) /* zoom out */
                st->info.zoom -= (wd->zoom_finger_factor * wd->zoom_wheel_factor);

              if (st->info.zoom < 0.0)
                st->info.zoom = 0.0;

              ev_flag = _set_state(gesture_zoom, s, &st->info, force);
              consume_event(wd, event_info, event_type, ev_flag);
              break;
           }

      default:
           return;
     }
}

/**
 * @internal
 *
 * This function is used to test zoom gesture.
 * user may combine zoom, rotation together.
 * so its possible that both will be detected from input.
 * (both are two-finger movement-oriented gestures)
 *
 * @param obj The gesture-layer object.
 * @param event_info Pointer to recent input event.
 * @param event_type Recent input event type.
 * @param g_type what Gesture we are testing.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_zoom_test(Evas_Object *obj, Pointer_Event *pe, void *event_info,
      Evas_Callback_Type event_type, Elm_Gesture_Types g_type)
{
   if (!pe)
     return;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->gesture[g_type]) return;

   Gesture_Info *gesture_zoom = wd->gesture[g_type];
   Zoom_Type *st = gesture_zoom->data;

   if (!st)
     {  /* Allocated once on first time, used for zoom data */
        st = calloc(1, sizeof(Zoom_Type));
        gesture_zoom->data = st;
        _zoom_test_reset(gesture_zoom);
     }


   /* Start - new zoom testing, letting all fingers start */
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
         /* if non-continues mode and gesture NOT started, ignore MOVE */
         if ((!wd->glayer_continues_enable) &&
               (!st->zoom_st.timestamp))
           return;

      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
           {  /* Here we take care of zoom-start and zoom move */
              Eina_List *l;
              Pointer_Event *p;

              if(eina_list_count(wd->touched) > 2)
                {  /* Process zoom only when 2 fingers on surface */
                   ev_flag = _set_state(gesture_zoom,
                         ELM_GESTURE_STATE_ABORT, &st->info, EINA_FALSE);
                   consume_event(wd, event_info, event_type, ev_flag);

                   return;
                }

              if (!st->zoom_st.timestamp)
                {  /* Now scan touched-devices list and find other finger */
                   EINA_LIST_FOREACH(wd->touched, l, p)
                     {  /* Device of other finger <> pe device */
                        if (p->device != pe->device)
                          break;
                     }

                   if (!p)  /* Single finger on touch */
                        return;

                   /* Record down fingers */
                   consume_event(wd, event_info, event_type, ev_flag);
                   memcpy(&st->zoom_st, pe, sizeof(Pointer_Event));
                   memcpy(&st->zoom_st1, p, sizeof(Pointer_Event));

                   /* Set mv field as well to be ready for MOVE events  */
                   memcpy(&st->zoom_mv, pe, sizeof(Pointer_Event));
                   memcpy(&st->zoom_mv1, p, sizeof(Pointer_Event));

                   /* Here we have zoom_st, zoom_st1 set, report START  */
                   /* Set zoom-base after BOTH down events  recorded    */
                   /* Compute length of line between fingers zoom start */
                   st->info.zoom = 1.0;
                   st->zoom_base = get_finger_gap_length(st->zoom_st1.x,
                         st->zoom_st1.y, st->zoom_st.x,  st->zoom_st.y,
                         &st->info.x, &st->info.y);

                   st->info.radius = st->zoom_base / 2;

                   if ((gesture_zoom->state != ELM_GESTURE_STATE_START) &&
                         (gesture_zoom->state != ELM_GESTURE_STATE_MOVE))
                     {  /* zoom started with mouse-wheel, don't report twice */
                        ev_flag = _set_state(gesture_zoom,
                              ELM_GESTURE_STATE_START, &st->info, EINA_FALSE);
                        consume_event(wd, event_info, event_type, ev_flag);
                     }

                   return;  /* Zoom started */
                }  /* End of ZOOM_START handling */


              /* if we got here, we have (exacally) two fingers on surfce */
              /* we also after START, report MOVE */
              /* First detect which finger moved  */
              if (pe->device == st->zoom_mv.device)
                memcpy(&st->zoom_mv, pe, sizeof(Pointer_Event));
              else if (pe->device == st->zoom_mv1.device)
                memcpy(&st->zoom_mv1, pe, sizeof(Pointer_Event));

              /* Compute change in zoom as fingers move */
                st->info.zoom = compute_zoom(st,
                      st->zoom_mv.x, st->zoom_mv.y, st->zoom_mv.timestamp,
                      st->zoom_mv1.x, st->zoom_mv1.y, st->zoom_mv1.timestamp,
                      wd->zoom_finger_factor);

              if (!st->zoom_distance_tolerance)
                {  /* Zoom broke tolerance, report move */
                   double d = st->info.zoom - st->next_step;
                   if (d < 0.0)
                     d = (-d);

                   if (d >= wd->zoom_step)
                     {  /* Report move in steps */
                        st->next_step = st->info.zoom;

                        ev_flag = _set_state(gesture_zoom,
                              ELM_GESTURE_STATE_MOVE,
                              &st->info, EINA_TRUE);
                        consume_event(wd, event_info, event_type, ev_flag);
                     }
                }  /* End of ZOOM_MOVE handling */

              return;
           }

      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
         /* Reset timestamp of finger-up.This is used later
            by _zoom_test_reset() to retain finger-down data */
         consume_event(wd, event_info, event_type, ev_flag);
         if (((st->zoom_wheel) || (st->zoom_base)) &&
               (st->zoom_distance_tolerance == 0))
           {
              ev_flag = _set_state(gesture_zoom, ELM_GESTURE_STATE_END,
                    &st->info, EINA_FALSE);
              consume_event(wd, event_info, event_type, ev_flag);

              return;
           }

         /* if we got here not a ZOOM */
         if (gesture_zoom->state != ELM_GESTURE_STATE_UNDEFINED)
           {  /* Must be != undefined, if gesture started */
              ev_flag = _set_state(gesture_zoom,
                    ELM_GESTURE_STATE_ABORT, &st->info, EINA_FALSE);
              consume_event(wd, event_info, event_type, ev_flag);
           }

         _zoom_test_reset(gesture_zoom);

         return;

      default:
         return;
       }
}

static void
_get_rotate_properties(Rotate_Type *st,
      Evas_Coord x1, Evas_Coord y1, unsigned int tm1,
      Evas_Coord x2, Evas_Coord y2, unsigned int tm2,
      double *angle)
{
   st->info.radius = get_finger_gap_length(x1, y1, x2, y2,
         &st->info.x, &st->info.y) / 2;

   *angle = get_angle(x1, y1, x2, y2);
#if 0 /* (NOT YET SUPPORTED) */
   if (angle == &st->info.angle)
     {  /* Compute momentum: TODO: bug when breaking 0, 360 values */
        st->info.momentum = (((*angle) - st->info.base_angle) /
           (fabs(tm2 - tm1))) * 1000;
     }
   else
     st->info.momentum = 0;
#else
   (void) tm1;
   (void) tm2;
#endif
}

/**
 * @internal
 *
 * This function is used to test rotation gesture.
 * user may combine zoom, rotation together.
 * so its possible that both will be detected from input.
 * (both are two-finger movement-oriented gestures)
 *
 * @param obj The gesture-layer object.
 * @param event_info Pointer to recent input event.
 * @param event_type Recent input event type.
 * @param g_type what Gesture we are testing.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_rotate_test(Evas_Object *obj, Pointer_Event *pe, void *event_info,
      Evas_Callback_Type event_type, Elm_Gesture_Types g_type)
{
   if (!pe)
     return;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->gesture[g_type]) return;

   Gesture_Info *gesture = wd->gesture[g_type];
   Rotate_Type *st = gesture->data;
   if (gesture)
   {
      st = gesture->data;
      if (!st)
        {  /* Allocated once on first time */
           st = calloc(1, sizeof(Rotate_Type));
           gesture->data = st;
           _rotate_test_reset(gesture);
        }
   }

   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
         /* if non-continues mode and gesture NOT started, ignore MOVE */
         if ((!wd->glayer_continues_enable) &&
               (!st->rotate_st.timestamp))
           return;

      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
           {  /* Here we take care of rotate-start and rotate move */
              Eina_List *l;
              Pointer_Event *p;

              if(eina_list_count(wd->touched) > 2)
                {  /* Process rotate only when 2 fingers on surface */
                   ev_flag = _set_state(gesture,
                         ELM_GESTURE_STATE_ABORT, &st->info, EINA_FALSE);
                   consume_event(wd, event_info, event_type, ev_flag);

                   return;
                }

              if (!st->rotate_st.timestamp)
                {  /* Now scan touched-devices list and find other finger */
                   EINA_LIST_FOREACH(wd->touched, l, p)
                     {  /* Device of other finger <> pe device */
                        if (p->device != pe->device)
                          break;
                     }

                   if (!p)
                        return;  /* Single finger on touch */

                   /* Record down fingers */
                   consume_event(wd, event_info, event_type, ev_flag);
                   memcpy(&st->rotate_st, pe, sizeof(Pointer_Event));
                   memcpy(&st->rotate_st1, p, sizeof(Pointer_Event));

                   /* Set mv field as well to be ready for MOVE events  */
                   memcpy(&st->rotate_mv, pe, sizeof(Pointer_Event));
                   memcpy(&st->rotate_mv1, p, sizeof(Pointer_Event));

                   /* Here we have rotate_st, rotate_st1 set, report START  */
                   /* Set rotate-base after BOTH down events  recorded    */
                   /* Compute length of line between fingers rotate start */
                   _get_rotate_properties(st,
                         st->rotate_st.x, st->rotate_st.y,
                         st->rotate_st.timestamp,
                         st->rotate_st1.x, st->rotate_st1.y,
                         st->rotate_st1.timestamp, &st->info.base_angle);

                   ev_flag = _set_state(gesture, ELM_GESTURE_STATE_START,
                         &st->info, EINA_FALSE);
                   consume_event(wd, event_info, event_type, ev_flag);

                   return;  /* Rotate started */
                }  /* End of ROTATE_START handling */


              /* if we got here, we have (exacally) two fingers on surfce */
              /* we also after START, report MOVE */
              /* First detect which finger moved  */
              if (pe->device == st->rotate_mv.device)
                memcpy(&st->rotate_mv, pe, sizeof(Pointer_Event));
              else if (pe->device == st->rotate_mv1.device)
                memcpy(&st->rotate_mv1, pe, sizeof(Pointer_Event));

              /* Compute change in rotate as fingers move */
              _get_rotate_properties(st,
                    st->rotate_mv.x, st->rotate_mv.y,
                    st->rotate_mv.timestamp,
                    st->rotate_mv1.x, st->rotate_mv1.y,
                    st->rotate_mv1.timestamp, &st->info.angle);

              if (rotation_broke_tolerance(st))
                {  /* Rotation broke tolerance, report move */
                   double d = st->info.angle - st->next_step;
                   if (d < 0.0)
                     d = (-d);

                   if (d >= wd->rotate_step)
                     {  /* Report move in steps */
                        st->next_step = st->info.angle;

                        ev_flag = _set_state(gesture,
                              ELM_GESTURE_STATE_MOVE, &st->info, EINA_TRUE);
                        consume_event(wd, event_info, event_type, ev_flag);
                     }
                }  /* End of ROTATE_MOVE handling */

              return;
           }

      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
           consume_event(wd, event_info, event_type, ev_flag);
           /* Reset timestamp of finger-up.This is used later
              by rotate_test_reset() to retain finger-down data */
           if (st->rotate_angular_tolerance < 0)
             {
                ev_flag = _set_state(gesture, ELM_GESTURE_STATE_END,
                      &st->info, EINA_FALSE);
                consume_event(wd, event_info, event_type, ev_flag);

                return;
             }

           if (gesture->state != ELM_GESTURE_STATE_UNDEFINED)
             {  /* Must be != undefined, if gesture started */
                ev_flag = _set_state(gesture, ELM_GESTURE_STATE_ABORT,
                      &st->info, EINA_FALSE);
                consume_event(wd, event_info, event_type, ev_flag);
             }

           _rotate_test_reset(gesture);
           return;

      default:
         return;
       }
}

/**
 * @internal
 *
 * This function is used to save input events in an abstract struct
 * to be used later by getsure-testing functions.
 *
 * @param data The gesture-layer object.
 * @param event_info Pointer to recent input event.
 * @param event_type Recent input event type.
 * @param pe The abstract data-struct (output).
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_make_pointer_event(void *data, void *event_info,
      Evas_Callback_Type event_type, Pointer_Event *pe)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return EINA_FALSE;

   memset(pe, '\0', sizeof(*pe));
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
           pe->x = ((Evas_Event_Mouse_Down *) event_info)->canvas.x;
           pe->y = ((Evas_Event_Mouse_Down *) event_info)->canvas.y;
           pe->timestamp = ((Evas_Event_Mouse_Down *) event_info)->timestamp;
           pe->device = ELM_MOUSE_DEVICE;
           break;

      case EVAS_CALLBACK_MOUSE_UP:
           pe->x = ((Evas_Event_Mouse_Up *) event_info)->canvas.x;
           pe->y = ((Evas_Event_Mouse_Up *) event_info)->canvas.y;
           pe->timestamp = ((Evas_Event_Mouse_Up *) event_info)->timestamp;
           pe->device = ELM_MOUSE_DEVICE;
           break;

      case EVAS_CALLBACK_MOUSE_MOVE:
           pe->x = ((Evas_Event_Mouse_Move *) event_info)->cur.canvas.x;
           pe->y = ((Evas_Event_Mouse_Move *) event_info)->cur.canvas.y;
           pe->timestamp = ((Evas_Event_Mouse_Move *) event_info)->timestamp;
           pe->device = ELM_MOUSE_DEVICE;
           break;

      case EVAS_CALLBACK_MULTI_DOWN:
           pe->x = ((Evas_Event_Multi_Down *) event_info)->canvas.x;
           pe->y = ((Evas_Event_Multi_Down *) event_info)->canvas.y;
           pe->timestamp = ((Evas_Event_Multi_Down *) event_info)->timestamp;
           pe->device = ((Evas_Event_Multi_Down *) event_info)->device;
           break;

      case EVAS_CALLBACK_MULTI_UP:
           pe->x = ((Evas_Event_Multi_Up *) event_info)->canvas.x;
           pe->y = ((Evas_Event_Multi_Up *) event_info)->canvas.y;
           pe->timestamp = ((Evas_Event_Multi_Up *) event_info)->timestamp;
           pe->device = ((Evas_Event_Multi_Up *) event_info)->device;
           break;

      case EVAS_CALLBACK_MULTI_MOVE:
           pe->x = ((Evas_Event_Multi_Move *) event_info)->cur.canvas.x;
           pe->y = ((Evas_Event_Multi_Move *) event_info)->cur.canvas.y;
           pe->timestamp = ((Evas_Event_Multi_Move *) event_info)->timestamp;
           pe->device = ((Evas_Event_Multi_Move *) event_info)->device;
           break;

      default:
           return EINA_FALSE;
     }

   pe->event_type = event_type;
   return EINA_TRUE;
}

/**
 * @internal
 *
 * This function restartes line, flick, zoom and rotate gestures
 * when gesture-layer continues-gestures enabled.
 * Example of continues-gesture:
 * When doing a line, user stops moving finger but keeps fingers on touch.
 * This will cause line-end, then as user continues moving his finger
 * it re-starts line gesture.
 * When continue mode is disabled, user has to lift finger from touch
 * to end a gesture. Them touch-again to start a new one.
 *
 * @param data The gesture-layer object.
 * @param wd gesture layer widget data.
 * @param states_reset flag that marks gestures were reset in history clear.
 *
 * @ingroup Elm_Gesture_Layer
 */
void continues_gestures_restart(void *data, Eina_Bool states_reset)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   /* Run through events to restart gestures */
   Gesture_Info *g;
   Eina_Bool n_momentum, n_lines, n_flicks, zoom, rotate;
   /* We turn-on flag for finished, aborted, not-started gestures */
   g = wd->gesture[ELM_GESTURE_MOMENTUM];
   n_momentum = (g) ? ((states_reset) | ((g->state != ELM_GESTURE_STATE_START)
         && (g->state != ELM_GESTURE_STATE_MOVE))) : EINA_FALSE;
   if (n_momentum)
     {
        _momentum_test_reset(wd->gesture[ELM_GESTURE_MOMENTUM]);
        _set_state(g, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
        SET_TEST_BIT(g);
     }

   g = wd->gesture[ELM_GESTURE_N_LINES];
   n_lines = (g) ? ((states_reset) | ((g->state != ELM_GESTURE_STATE_START)
         && (g->state != ELM_GESTURE_STATE_MOVE))) : EINA_FALSE;
   if (n_lines)
     {
        _line_test_reset(wd->gesture[ELM_GESTURE_N_LINES]);
        _set_state(g, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
        SET_TEST_BIT(g);
     }

   g = wd->gesture[ELM_GESTURE_N_FLICKS];
   n_flicks = (g) ? ((states_reset) | ((g->state != ELM_GESTURE_STATE_START)
         && (g->state != ELM_GESTURE_STATE_MOVE))) : EINA_FALSE;
   if (n_flicks)
     {
        _line_test_reset(wd->gesture[ELM_GESTURE_N_FLICKS]);
        _set_state(g, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
        SET_TEST_BIT(g);
     }

   g = wd->gesture[ELM_GESTURE_ZOOM];
   zoom = (g) ? ((states_reset) | ((g->state != ELM_GESTURE_STATE_START)
         && (g->state != ELM_GESTURE_STATE_MOVE))) : EINA_FALSE;
   if (zoom)
     {
        _zoom_test_reset(wd->gesture[ELM_GESTURE_ZOOM]);
        _set_state(g, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
        SET_TEST_BIT(g);
     }

   g = wd->gesture[ELM_GESTURE_ROTATE];
   rotate = (g) ? ((states_reset) | ((g->state != ELM_GESTURE_STATE_START)
         && (g->state != ELM_GESTURE_STATE_MOVE))) : EINA_FALSE;
   if (rotate)
     {
        _rotate_test_reset(wd->gesture[ELM_GESTURE_ROTATE]);
        _set_state(g, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
        SET_TEST_BIT(g);
     }
}

/**
 * @internal
 *
 * This function the core-function where input handling is done.
 * Here we get user input and stream it to gesture testing.
 * We notify user about any gestures with new state:
 * Valid states are:
 * START - gesture started.
 * MOVE - gesture is ongoing.
 * END - gesture was completed.
 * ABORT - gesture was aborted after START, MOVE (will NOT be completed)
 *
 * We also check if a gesture was detected, then reset event history
 * If no gestures were found we reset gesture test flag
 * after streaming event-history to widget.
 * (stream to the widget all events not consumed as a gesture)
 *
 * @param data The gesture-layer object.
 * @param event_info Pointer to recent input event.
 * @param event_type Recent input event type.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_event_process(void *data, Evas_Object *obj __UNUSED__,
      void *event_info, Evas_Callback_Type event_type)
{
   Pointer_Event _pe;
   Pointer_Event *pe = NULL;
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

#if defined(DEBUG_GESTURE_LAYER)
   int i;
   Gesture_Info *g;
   printf("Gesture | State | is tested\n");
   for(i = ELM_GESTURE_N_TAPS; i < ELM_GESTURE_LAST; i++)
     {
        g = wd->gesture[i];
        if(g)
          printf("   %d       %d       %d\n", i, g->state, g->test);
     }
#endif

   /* Start testing candidate gesture from here */
   if (_make_pointer_event(data, event_info, event_type, &_pe))
     pe = &_pe;

   if (IS_TESTED(ELM_GESTURE_N_LONG_TAPS))
     _n_long_tap_test(data, pe, event_info, event_type,
           ELM_GESTURE_N_LONG_TAPS);

   /* This takes care of single, double and tripple tap */
   _tap_gestures_test(data, pe, event_info, event_type);

   if (IS_TESTED(ELM_GESTURE_MOMENTUM))
     _momentum_test(data, pe, event_info, event_type,
           ELM_GESTURE_MOMENTUM);

   if (IS_TESTED(ELM_GESTURE_N_LINES))
     _n_line_test(data, pe, event_info, event_type, ELM_GESTURE_N_LINES);

   if (IS_TESTED(ELM_GESTURE_N_FLICKS))
     _n_line_test(data, pe, event_info, event_type, ELM_GESTURE_N_FLICKS);

   if (IS_TESTED(ELM_GESTURE_ZOOM))
     _zoom_test(data, pe, event_info, event_type, ELM_GESTURE_ZOOM);

   if (IS_TESTED(ELM_GESTURE_ZOOM))
     _zoom_with_wheel_test(data, event_info, event_type, ELM_GESTURE_ZOOM);

   if (IS_TESTED(ELM_GESTURE_ROTATE))
     _rotate_test(data, pe, event_info, event_type, ELM_GESTURE_ROTATE);

   if (_get_event_flag(event_info, event_type) & EVAS_EVENT_FLAG_ON_HOLD)
     _event_history_add(data, event_info, event_type);
   else if ((event_type == EVAS_CALLBACK_MOUSE_UP) ||
         (event_type == EVAS_CALLBACK_MULTI_UP))
     {
        Eina_List *pending = _device_is_pending(wd->pending, event_info, event_type);
        if (pending)
          {
             consume_event(wd, event_info, event_type, EVAS_EVENT_FLAG_ON_HOLD);
             _event_history_add(data, event_info, event_type);
          }
     }

   /* we maintain list of touched devices              */
   /* We also use move to track current device x.y pos */
   if ((event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
         (event_type == EVAS_CALLBACK_MULTI_DOWN) ||
         (event_type == EVAS_CALLBACK_MOUSE_MOVE) ||
         (event_type == EVAS_CALLBACK_MULTI_MOVE))
     {
        wd->touched = _add_touched_device(wd->touched, pe);
     }
   else if ((event_type == EVAS_CALLBACK_MOUSE_UP) ||
         (event_type == EVAS_CALLBACK_MULTI_UP))
     {
        wd->touched = _remove_touched_device(wd->touched, pe);
     }

   /* Report current states and clear history if needed */
   Eina_Bool states_reset = _clear_if_finished(data);
   if (wd->glayer_continues_enable)
     continues_gestures_restart(data, states_reset);
}


/**
 * For all _mouse_* / multi_* functions wethen send this event to
 * _event_process function.
 *
 * @param data The gesture-layer object.
 * @param event_info Pointer to recent input event.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (((Evas_Event_Mouse_Down *) event_info)->button != 1)
     return; /* We only process left-click at the moment */

   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_DOWN);
}

static void
_mouse_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_MOVE);
}

static void
_key_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_KEY_DOWN);
}

static void
_key_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_KEY_UP);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   if (((Evas_Event_Mouse_Up *) event_info)->button != 1)
     return; /* We only process left-click at the moment */

   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_UP);
}

static void
_mouse_wheel(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_WHEEL);
}

static void
_multi_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MULTI_DOWN);
}

static void
_multi_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MULTI_MOVE);
}

static void
_multi_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MULTI_UP);
}

EAPI Eina_Bool
elm_gesture_layer_hold_events_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return !wd->repeat_events;
}

EAPI void
elm_gesture_layer_hold_events_set(Evas_Object *obj, Eina_Bool r)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->repeat_events = !r;
}

EAPI void
elm_gesture_layer_zoom_step_set(Evas_Object *obj, double s)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (s < 0.0)
     return;

   wd->zoom_step = s;
}

EAPI void
elm_gesture_layer_rotate_step_set(Evas_Object *obj, double s)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (s < 0.0)
     return;

   wd->rotate_step = s;
}

EAPI Eina_Bool
elm_gesture_layer_attach(Evas_Object *obj, Evas_Object *t)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   if (!t)
     return EINA_FALSE;

   /* if was attached before, unregister callbacks first */
   if (wd->target)
     _unregister_callbacks(obj);

   wd->target = t;

   _register_callbacks(obj);
   return EINA_TRUE;
}

EAPI void
elm_gesture_layer_cb_set(Evas_Object *obj, Elm_Gesture_Types idx,
      Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Gesture_Info *p;
   if (!wd) return;

   if (!wd->gesture[idx])
     wd->gesture[idx] = calloc(1, sizeof(Gesture_Info));
   if (!wd->gesture[idx]) return;

   p = wd->gesture[idx];
   p->obj = obj;
   p->g_type = idx;
   p->fn[cb_type].cb = cb;
   p->fn[cb_type].user_data = data;
   p->state = ELM_GESTURE_STATE_UNDEFINED;
   SET_TEST_BIT(p);
}

static void
_disable_hook(Evas_Object *obj)
{
   if (elm_widget_disabled_get(obj))
     _unregister_callbacks(obj);
   else
     _register_callbacks(obj);
}

EAPI Evas_Object *
elm_gesture_layer_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   if (!e) return NULL;
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "gesture_layer");
   elm_widget_type_set(obj, "gesture_layer");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);

   wd->target = NULL;
   wd->line_min_length =_elm_config->glayer_line_min_length * elm_finger_size_get();
   wd->zoom_distance_tolerance = _elm_config->glayer_zoom_distance_tolerance * elm_finger_size_get();
   wd->line_distance_tolerance = _elm_config->glayer_line_distance_tolerance * elm_finger_size_get();
   wd->zoom_finger_factor = _elm_config->glayer_zoom_finger_factor;
   wd->zoom_wheel_factor = _elm_config->glayer_zoom_wheel_factor; /* mouse wheel zoom steps */
   wd->rotate_angular_tolerance = _elm_config->glayer_rotate_angular_tolerance;
   wd->line_angular_tolerance = _elm_config->glayer_line_angular_tolerance;
   wd->flick_time_limit_ms = _elm_config->glayer_flick_time_limit_ms;
   wd->long_tap_start_timeout = _elm_config->glayer_long_tap_start_timeout;
   wd->repeat_events = EINA_TRUE;
   wd->glayer_continues_enable = _elm_config->glayer_continues_enable;

#if defined(DEBUG_GESTURE_LAYER)
   printf("size of Gestures = <%d>\n", sizeof(wd->gesture));
   printf("initial values:\n\tzoom_finger_factor=<%f>\n\tzoom_distance_tolerance=<%d>\n\tline_min_length=<%d>\n\tline_distance_tolerance=<%d>\n\tzoom_wheel_factor=<%f>\n\trotate_angular_tolerance=<%f>\n\twd->line_angular_tolerance=<%f>\n\twd->flick_time_limit_ms=<%d>\n\twd->long_tap_start_timeout=<%f>\n\twd->zoom_step=<%f>\n\twd->rotate_step=<%f>\n\twd->glayer_continues_enable=<%d>\n ", wd->zoom_finger_factor, wd->zoom_distance_tolerance, wd->line_min_length, wd->line_distance_tolerance, wd->zoom_wheel_factor, wd->rotate_angular_tolerance, wd->line_angular_tolerance, wd->flick_time_limit_ms, wd->long_tap_start_timeout, wd->zoom_step, wd->rotate_step, wd->glayer_continues_enable);
#endif
   memset(wd->gesture, 0, sizeof(wd->gesture));

   return obj;
}
