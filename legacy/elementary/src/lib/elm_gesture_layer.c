#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS ELM_GESTURE_LAYER_CLASS

#define MY_CLASS_NAME "Elm_Gesture_Layer"
#define MY_CLASS_NAME_LEGACY "elm_gesture_layer"

/* Some defaults */
#define ELM_MOUSE_DEVICE             0
/* ELM_GESTURE_NEGATIVE_ANGLE - magic number says we didn't compute this yet */
#define ELM_GESTURE_NEGATIVE_ANGLE   (-1.0) /* Magic number */
#define ELM_GESTURE_MOMENTUM_DELAY   25
#define ELM_GESTURE_MOMENTUM_TIMEOUT 50
#define ELM_GESTURE_MULTI_TIMEOUT    50
#define ELM_GESTURE_MINIMUM_MOMENTUM 0.001

/* Some Trigo values */
#define RAD_90DEG                    M_PI_2
#define RAD_180DEG                   M_PI
#define RAD_270DEG                   (M_PI_2 * 3)
#define RAD_360DEG                   (M_PI * 2)

#define RAD2DEG(x) ((x) * 57.295779513)
#define DEG2RAD(x) ((x) / 57.295779513)

static void *
_glayer_buf_dup(void *buf, size_t size)
{
   void *p;

   p = malloc(size);
   memcpy(p, buf, size);

   return p;
}

#define COPY_EVENT_INFO(EV) _glayer_buf_dup(EV, sizeof(*EV))

#define SET_TEST_BIT(P)                               \
  do {                                                \
       P->test = P->cbs[ELM_GESTURE_STATE_START] || \
         P->cbs[ELM_GESTURE_STATE_MOVE] ||          \
         P->cbs[ELM_GESTURE_STATE_END] ||           \
         P->cbs[ELM_GESTURE_STATE_ABORT];           \
    } while (0)

#define IS_TESTED_GESTURE(gesture) \
  ((gesture) ? (gesture)->test : EINA_FALSE)

#define IS_TESTED(T) \
  ((sd->gesture[T]) ? sd->gesture[T]->test : EINA_FALSE)

#define ELM_GESTURE_LAYER_DATA_GET(o, sd) \
  Elm_Gesture_Layer_Data * sd = eo_data_scope_get(o, MY_CLASS)

#define ELM_GESTURE_LAYER_DATA_GET_OR_RETURN(o, ptr) \
  ELM_GESTURE_LAYER_DATA_GET(o, ptr);                \
  if (!ptr)                                          \
    {                                                \
       CRI("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_GESTURE_LAYER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_GESTURE_LAYER_DATA_GET(o, ptr);                         \
  if (!ptr)                                                   \
    {                                                         \
       CRI("No widget data for object %p (%s)",          \
                o, evas_object_type_get(o));                  \
       return val;                                            \
    }

#define ELM_GESTURE_LAYER_CHECK(obj)                                      \
  if (!obj || !eo_isa(obj, MY_CLASS)) \
    return

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
   Evas_Coord         x, y;
   unsigned int       timestamp;
   int                device;
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
   EINA_INLIST;
   void                *user_data; /**< Holds user data to CB (like sd) */
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
   Evas_Object      *obj;
   void             *data; /**< Holds gesture intemidiate processing data */
   Eina_Inlist      *cbs[ELM_GESTURE_STATE_ABORT + 1]; /**< Callback info (Func_Data) for states */
   Elm_Gesture_Type  g_type; /**< gesture type */
   Elm_Gesture_State state; /**< gesture state */
   void             *info; /**< Data for the state callback */
   Eina_Bool         test; /**< if true this gesture should be tested on input */
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

typedef struct
{
   void (*test)(Evas_Object *obj, Pointer_Event *pe,
                void *event_info, Evas_Callback_Type event_type,
                Elm_Gesture_Type g_type);
   void (*reset)(Gesture_Info *gesture);
   void (*cont_reset)(Gesture_Info *gesture); /* Can be NULL. */
} Tests_Array_Funcs;

/* functions referred by _glayer_tests_array */
static void _tap_gesture_test(Evas_Object *obj,
                              Pointer_Event *pe,
                              void *event_info,
                              Evas_Callback_Type event_type,
                              Elm_Gesture_Type g_type);
static void _tap_gestures_test_reset(Gesture_Info *gesture);
static void _n_long_tap_test(Evas_Object *obj,
                             Pointer_Event *pe,
                             void *event_info,
                             Evas_Callback_Type event_type,
                             Elm_Gesture_Type g_type);
static void _n_long_tap_test_reset(Gesture_Info *gesture);
static void _momentum_test(Evas_Object *obj,
                           Pointer_Event *pe,
                           void *event_info,
                           Evas_Callback_Type event_type,
                           Elm_Gesture_Type g_type);
static void _momentum_test_reset(Gesture_Info *gesture);
static void _n_line_test(Evas_Object *obj,
                         Pointer_Event *pe,
                         void *event_info,
                         Evas_Callback_Type event_type,
                         Elm_Gesture_Type g_type);
static void _line_test_reset(Gesture_Info *gesture);
static void _zoom_test(Evas_Object *obj,
                       Pointer_Event *pe,
                       void *event_info,
                       Evas_Callback_Type event_type,
                       Elm_Gesture_Type g_type);
static void _zoom_test_reset(Gesture_Info *gesture);
static void _rotate_test(Evas_Object *obj,
                         Pointer_Event *pe,
                         void *event_info,
                         Evas_Callback_Type event_type,
                         Elm_Gesture_Type g_type);
static void _rotate_test_reset(Gesture_Info *gesture);

static void _event_process(void *data,
                           Evas_Object *obj,
                           void *event_info,
                           Evas_Callback_Type event_type);

static void _callbacks_unregister(Evas_Object *obj);

/* Should be the same order as _Elm_Gesture_Type */
static Tests_Array_Funcs _glayer_tests_array[] = {
   { NULL, NULL, NULL },     /** Because someone made an awful mistake. */
   { _tap_gesture_test, _tap_gestures_test_reset, NULL },
   /* ELM_GESTURE_N_TAPS */
   { _n_long_tap_test, _n_long_tap_test_reset, NULL },
   /* ELM_GESTURE_N_LONG_TAPS */
   { _tap_gesture_test, _tap_gestures_test_reset, NULL },
   /* ELM_GESTURE_N_DOUBLE_TAPS */
   { _tap_gesture_test, _tap_gestures_test_reset, NULL },
   /* ELM_GESTURE_N_TRIPLE_TAPS */
   { _momentum_test, _momentum_test_reset, _momentum_test_reset },
   /* ELM_GESTURE_MOMENTUM */
   { _n_line_test, _line_test_reset, _line_test_reset },
   /* ELM_GESTURE_N_LINES */
   { _n_line_test, _line_test_reset, _line_test_reset },
   /* ELM_GESTURE_N_FLICKS */
   { _zoom_test, _zoom_test_reset, _zoom_test_reset },
   /* ELM_GESTURE_ZOOM */
   { _rotate_test, _rotate_test_reset, _rotate_test_reset },
   /* ELM_GESTURE_ROTATE */
   { NULL, NULL, NULL }
};

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
   void              *event;
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

/* All *Type structs hold result for the user in 'info' field
 * The rest is gesture processing intermediate data.
 * NOTE: info field must be FIRST in the struct.
 * This is used when reporting ABORT in _event_history_clear() */
struct _Taps_Type
{
   Elm_Gesture_Taps_Info info;
   unsigned int          sum_x;
   unsigned int          sum_y;
   unsigned int          n_taps_needed;
   unsigned int          n_taps;
   Eina_List            *l;
};
typedef struct _Taps_Type Taps_Type;

struct _Long_Tap_Type
{
   Elm_Gesture_Taps_Info info;
   Evas_Coord            center_x;
   Evas_Coord            center_y;
   Ecore_Timer          *timeout; /* When this expires, long tap STARTed */
   Eina_List            *touched;
};
typedef struct _Long_Tap_Type Long_Tap_Type;

struct _Momentum_Type /* Fields used by _line_test() */
{
   Elm_Gesture_Momentum_Info info;
   Evas_Coord_Point          line_st;
   Evas_Coord_Point          line_end;
   unsigned int              t_st_x; /* Time start on X */
   unsigned int              t_st_y; /* Time start on Y */
   unsigned int              t_end; /* Time end        */
   unsigned int              t_up; /* Recent up event time */
   int                       xdir, ydir;
};
typedef struct _Momentum_Type Momentum_Type;

struct _Line_Data
{
   Evas_Coord_Point line_st;
   Evas_Coord_Point line_end;
   Evas_Coord       line_length;
   unsigned int     t_st; /* Time start */
   unsigned int     t_end; /* Time end   */
   int              device;
   double           line_angle; /* Current angle of line */
};
typedef struct _Line_Data Line_Data;

struct _Line_Type /* Fields used by _line_test() */
{
   Elm_Gesture_Line_Info info;
   Eina_List            *list; /* List of Line_Data */
};
typedef struct _Line_Type Line_Type;

struct _Zoom_Type /* Fields used by _zoom_test() */
{
   Elm_Gesture_Zoom_Info   info;
   Pointer_Event           zoom_st;
   Pointer_Event           zoom_mv;
   Pointer_Event           zoom_st1;
   Pointer_Event           zoom_mv1;
   Evas_Event_Mouse_Wheel *zoom_wheel;
   Evas_Coord              zoom_base; /* Holds gap between fingers on
                                       * zoom-start  */
   Evas_Coord              zoom_distance_tolerance;
   unsigned int            m_st_tm; /* momentum start time */
   unsigned int            m_prev_tm; /* momentum prev time  */
   int                     dir; /* Direction: 1=zoom-in, (-1)=zoom-out */
   double                  m_base; /* zoom value when momentum starts */
   double                  next_step;
};
typedef struct _Zoom_Type Zoom_Type;

struct _Rotate_Type /* Fields used by _rotation_test() */
{
   Elm_Gesture_Rotate_Info info;
   Pointer_Event           rotate_st;
   Pointer_Event           rotate_mv;
   Pointer_Event           rotate_st1;
   Pointer_Event           rotate_mv1;
   unsigned int            prev_momentum_tm; /* timestamp of prev_momentum */
   double                  prev_momentum; /* Snapshot of momentum 0.01
                                           * sec ago */
   double                  accum_momentum;
   double                  rotate_angular_tolerance;
   double                  next_step;
};
typedef struct _Rotate_Type                  Rotate_Type;

typedef struct _Elm_Gesture_Layer_Data Elm_Gesture_Layer_Data;
struct _Elm_Gesture_Layer_Data
{
   Evas_Object          *target; /* Target Widget */
   Event_History        *event_history_list;

   int                   line_min_length;
   Evas_Coord            zoom_distance_tolerance;
   Evas_Coord            line_distance_tolerance;
   double                line_angular_tolerance;
   double                zoom_wheel_factor; /* mouse wheel zoom steps */
   double                zoom_finger_factor; /* used for zoom factor */
   double                rotate_angular_tolerance;
   unsigned int          flick_time_limit_ms;
   double                long_tap_start_timeout;
   Eina_Bool             glayer_continues_enable;
   double                double_tap_timeout;

   double                zoom_step;
   double                rotate_step;

   Gesture_Info         *gesture[ELM_GESTURE_LAST];
   Eina_List            *pending; /* List of devices need to refeed
                                   * *UP event */
   Eina_List            *touched; /* Information  of touched devices */

   /* Taps Gestures */
   Evas_Coord           tap_finger_size;    /* Default from Config */
   Ecore_Timer          *gest_taps_timeout; /* When this expires, dbl
                                             * click/taps ABORTed  */

   Eina_Bool             repeat_events : 1;
};

/* START - Functions to manage touched-device list */
/**
 * @internal
 * This function is used to find if device is touched
 *
 * @ingroup Elm_Gesture_Layer
 */
static int
_device_compare(const void *data1,
                const void *data2)
{
   /* Compare the two device numbers */
   return ((Pointer_Event *)data1)->device - ((Pointer_Event *)data2)->device;
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
_touched_device_remove(Eina_List *list,
                       Pointer_Event *pe)
{
   Eina_List *lst = NULL;
   Pointer_Event *p = eina_list_search_unsorted(list, _device_compare, pe);
   if (p)
     {
        lst = eina_list_remove(list, p);
        free(p);
        return lst;
     }

   return list;
}

/**
 * @internal
 *
 * Recoed Pointer Event in touched device list
 * Note: This fuction allocates memory for PE event
 * This memory is released in _touched_device_remove()
 * @param list Pointer to touched device list.
 * @param Pointer_Event Pointer to PE.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_touched_device_add(Eina_List *list,
                    Pointer_Event *pe)
{
   Pointer_Event *p = eina_list_search_unsorted(list, _device_compare, pe);

   if (p) /* We like to track device touch-position, overwrite info */
     {
        memcpy(p, pe, sizeof(Pointer_Event));
        return list;
     }

   if ((pe->event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
       (pe->event_type == EVAS_CALLBACK_MULTI_DOWN)) /* Add touched
                                                      * device on DOWN
                                                      * event only */
     {
        p = malloc(sizeof(Pointer_Event));
        /* Freed in _touched_device_remove()    */
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
_event_flag_get(void *event_info,
                Evas_Callback_Type event_type)
{
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
        return ((Evas_Event_Mouse_In *)event_info)->event_flags;

      case EVAS_CALLBACK_MOUSE_OUT:
        return ((Evas_Event_Mouse_Out *)event_info)->event_flags;

      case EVAS_CALLBACK_MOUSE_DOWN:
        return ((Evas_Event_Mouse_Down *)event_info)->event_flags;

      case EVAS_CALLBACK_MOUSE_MOVE:
        return ((Evas_Event_Mouse_Move *)event_info)->event_flags;

      case EVAS_CALLBACK_MOUSE_UP:
        return ((Evas_Event_Mouse_Up *)event_info)->event_flags;

      case EVAS_CALLBACK_MOUSE_WHEEL:
        return ((Evas_Event_Mouse_Wheel *)event_info)->event_flags;

      case EVAS_CALLBACK_MULTI_DOWN:
        return ((Evas_Event_Multi_Down *)event_info)->event_flags;

      case EVAS_CALLBACK_MULTI_MOVE:
        return ((Evas_Event_Multi_Move *)event_info)->event_flags;

      case EVAS_CALLBACK_MULTI_UP:
        return ((Evas_Event_Multi_Up *)event_info)->event_flags;

      case EVAS_CALLBACK_KEY_DOWN:
        return ((Evas_Event_Key_Down *)event_info)->event_flags;

      case EVAS_CALLBACK_KEY_UP:
        return ((Evas_Event_Key_Up *)event_info)->event_flags;

      default:
        return EVAS_EVENT_FLAG_NONE;
     }
}

/**
 * @internal
 *
 * Sets event flag to value returned from user callback
 * @param sd Widget Data
 * @param event_info pointer to event.
 * @param event_type what type was ev (mouse down, etc...)
 * @param ev_flags event flags
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_event_consume(Elm_Gesture_Layer_Data *sd,
               void *event_info,
               Evas_Callback_Type event_type,
               Evas_Event_Flags ev_flags)
{
   /* Mark EVAS_EVENT_FLAG_ON_HOLD on events that are used by gesture layer */
   /* ev_flags != EVAS_EVENT_FLAG_NONE means target used event and g-layer  */
   /* should not refeed this event.  */
   if (!event_info)
     return;  /* This happens when restarting gestures  */

   if (!sd->repeat_events) ev_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   if (ev_flags)
     {
        switch (event_type)
          {
           case EVAS_CALLBACK_MOUSE_DOWN:
             ((Evas_Event_Mouse_Down *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_MOUSE_MOVE:
             ((Evas_Event_Mouse_Move *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_MOUSE_UP:
             ((Evas_Event_Mouse_Up *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_MOUSE_WHEEL:
             ((Evas_Event_Mouse_Wheel *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_MULTI_DOWN:
             ((Evas_Event_Multi_Down *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_MULTI_MOVE:
             ((Evas_Event_Multi_Move *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_MULTI_UP:
             ((Evas_Event_Multi_Up *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_KEY_DOWN:
             ((Evas_Event_Key_Down *)event_info)->event_flags |= ev_flags;
             break;

           case EVAS_CALLBACK_KEY_UP:
             ((Evas_Event_Key_Up *)event_info)->event_flags |= ev_flags;
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
_state_report(Gesture_Info *gesture,
              void *info)
{
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   /* We report current state (START, MOVE, END, ABORT), once */
   if ((gesture->state != ELM_GESTURE_STATE_UNDEFINED) &&
       (gesture->cbs[gesture->state])) /* Fill state-info struct and
                                        * send ptr to user
                                        * callback */
     {
        Func_Data *cb_info;
        EINA_INLIST_FOREACH(gesture->cbs[gesture->state], cb_info)
           flags |= cb_info->cb(cb_info->user_data, info);
     }

   return EVAS_EVENT_FLAG_NONE;
}

/**
 * @internal
 *
 * Update state for a given gesture.
 * We may update gesture state to:
 * - @c UNDEFINED - current input did not start gesure yet.
 * - @c START - gesture started according to input.
 * - @c MOVE - gusture in progress.
 * - @c END - gesture completed according to input.
 * - @c ABORT - input does not matches gesure.
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
_state_set(Gesture_Info *g,
           Elm_Gesture_State s,
           void *info,
           Eina_Bool force)
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
     return _state_report(g, g->info);

   return EVAS_EVENT_FLAG_NONE;
}

/**
 * @internal
 *
 * This resets all gesture states and sets test-bit.
 * this is used for restarting gestures to listen to input.
 * happens after we complete a gesture or no gesture was detected.
 * @param sd Widget data of the gesture-layer object.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_states_reset(Elm_Gesture_Layer_Data *sd)
{
   int i;
   Gesture_Info *p;

   for (i = ELM_GESTURE_FIRST; i < ELM_GESTURE_LAST; i++)
     {
        p = sd->gesture[i];
        if (p)
          {
             _state_set(p, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
             SET_TEST_BIT(p);
          }
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
_pointer_event_make(void *data EINA_UNUSED,
                    void *event_info,
                    Evas_Callback_Type event_type,
                    Pointer_Event *pe)
{
   memset(pe, '\0', sizeof(*pe));
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
        pe->x = ((Evas_Event_Mouse_Down *)event_info)->canvas.x;
        pe->y = ((Evas_Event_Mouse_Down *)event_info)->canvas.y;
        pe->timestamp = ((Evas_Event_Mouse_Down *)event_info)->timestamp;
        pe->device = ELM_MOUSE_DEVICE;
        break;

      case EVAS_CALLBACK_MOUSE_UP:
        pe->x = ((Evas_Event_Mouse_Up *)event_info)->canvas.x;
        pe->y = ((Evas_Event_Mouse_Up *)event_info)->canvas.y;
        pe->timestamp = ((Evas_Event_Mouse_Up *)event_info)->timestamp;
        pe->device = ELM_MOUSE_DEVICE;
        break;

      case EVAS_CALLBACK_MOUSE_MOVE:
        pe->x = ((Evas_Event_Mouse_Move *)event_info)->cur.canvas.x;
        pe->y = ((Evas_Event_Mouse_Move *)event_info)->cur.canvas.y;
        pe->timestamp = ((Evas_Event_Mouse_Move *)event_info)->timestamp;
        pe->device = ELM_MOUSE_DEVICE;
        break;

      case EVAS_CALLBACK_MULTI_DOWN:
        pe->x = ((Evas_Event_Multi_Down *)event_info)->canvas.x;
        pe->y = ((Evas_Event_Multi_Down *)event_info)->canvas.y;
        pe->timestamp = ((Evas_Event_Multi_Down *)event_info)->timestamp;
        pe->device = ((Evas_Event_Multi_Down *)event_info)->device;
        break;

      case EVAS_CALLBACK_MULTI_UP:
        pe->x = ((Evas_Event_Multi_Up *)event_info)->canvas.x;
        pe->y = ((Evas_Event_Multi_Up *)event_info)->canvas.y;
        pe->timestamp = ((Evas_Event_Multi_Up *)event_info)->timestamp;
        pe->device = ((Evas_Event_Multi_Up *)event_info)->device;
        break;

      case EVAS_CALLBACK_MULTI_MOVE:
        pe->x = ((Evas_Event_Multi_Move *)event_info)->cur.canvas.x;
        pe->y = ((Evas_Event_Multi_Move *)event_info)->cur.canvas.y;
        pe->timestamp = ((Evas_Event_Multi_Move *)event_info)->timestamp;
        pe->device = ((Evas_Event_Multi_Move *)event_info)->device;
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
_event_info_copy(void *event,
                 Evas_Callback_Type event_type)
{
   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
        return COPY_EVENT_INFO((Evas_Event_Mouse_Down *)event);
        break;

      case EVAS_CALLBACK_MOUSE_MOVE:
        return COPY_EVENT_INFO((Evas_Event_Mouse_Move *)event);
        break;

      case EVAS_CALLBACK_MOUSE_UP:
        return COPY_EVENT_INFO((Evas_Event_Mouse_Up *)event);
        break;

      case EVAS_CALLBACK_MOUSE_WHEEL:
        return COPY_EVENT_INFO((Evas_Event_Mouse_Wheel *)event);
        break;

      case EVAS_CALLBACK_MULTI_DOWN:
        return COPY_EVENT_INFO((Evas_Event_Multi_Down *)event);
        break;

      case EVAS_CALLBACK_MULTI_MOVE:
        return COPY_EVENT_INFO((Evas_Event_Multi_Move *)event);
        break;

      case EVAS_CALLBACK_MULTI_UP:
        return COPY_EVENT_INFO((Evas_Event_Multi_Up *)event);
        break;

      case EVAS_CALLBACK_KEY_DOWN:
        return COPY_EVENT_INFO((Evas_Event_Key_Down *)event);
        break;

      case EVAS_CALLBACK_KEY_UP:
        return COPY_EVENT_INFO((Evas_Event_Key_Up *)event);
        break;

      default:
        return NULL;
     }
}

static Eina_Bool
_event_history_add(Evas_Object *obj,
                   void *event,
                   Evas_Callback_Type event_type)
{
   Event_History *ev;

   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   ev = malloc(sizeof(Event_History));
   ev->event = _event_info_copy(event, event_type);  /* Freed on
                                                      * _event_history_clear */
   ev->event_type = event_type;
   sd->event_history_list = (Event_History *)eina_inlist_append(
       EINA_INLIST_GET(sd->event_history_list), EINA_INLIST_GET(ev));

   return EINA_TRUE;
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
_mouse_down_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   if (((Evas_Event_Mouse_Down *)event_info)->button != 1)
     return;  /* We only process left-click at the moment */

   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_DOWN);
}

static void
_mouse_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_MOVE);
}

static void
_key_down_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   _event_process(data, obj, event_info, EVAS_CALLBACK_KEY_DOWN);
}

static void
_key_up_cb(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info)
{
   _event_process(data, obj, event_info, EVAS_CALLBACK_KEY_UP);
}

static void
_mouse_up_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   if (((Evas_Event_Mouse_Up *)event_info)->button != 1)
     return;  /* We only process left-click at the moment */

   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_UP);
}

static void
_mouse_wheel_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   _event_process(data, obj, event_info, EVAS_CALLBACK_MOUSE_WHEEL);
}

static void
_multi_down_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   /* Skip the mouse duplicates. */
   if (((Evas_Event_Multi_Down *) event_info)->device == 0) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MULTI_DOWN);
}

static void
_multi_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   /* Skip the mouse duplicates. */
   if (((Evas_Event_Multi_Move *) event_info)->device == 0) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MULTI_MOVE);
}

static void
_multi_up_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   /* Skip the mouse duplicates. */
   if (((Evas_Event_Multi_Up *) event_info)->device == 0) return;

   _event_process(data, obj, event_info, EVAS_CALLBACK_MULTI_UP);
}

static void
_target_del_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   _callbacks_unregister(data);
   ELM_GESTURE_LAYER_DATA_GET(data, sd);
   sd->target = NULL;
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
_callbacks_register(Evas_Object *obj)
{
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!sd->target) return;

   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, obj);
   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);

   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, obj);

   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MULTI_DOWN, _multi_down_cb, obj);
   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MULTI_MOVE, _multi_move_cb, obj);
   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_MULTI_UP, _multi_up_cb, obj);

   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, obj);
   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_KEY_UP, _key_up_cb, obj);

   evas_object_event_callback_add
     (sd->target, EVAS_CALLBACK_DEL, _target_del_cb, obj);
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
_callbacks_unregister(Evas_Object *obj)
{
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!sd->target) return;

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, obj);
   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, obj);

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MULTI_DOWN, _multi_down_cb, obj);

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MULTI_MOVE, _multi_move_cb, obj);

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_MULTI_UP, _multi_up_cb, obj);

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, obj);
   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_KEY_UP, _key_up_cb, obj);

   evas_object_event_callback_del_full
     (sd->target, EVAS_CALLBACK_DEL, _target_del_cb, obj);
}

/**
 * @internal
 * This function is used to find if device number
 * is found in a list of devices.
 * The list contains devices for refeeding *UP event
 *
 * @ingroup Elm_Gesture_Layer
 */
static int
_device_in_pending_cmp(const void *data1,
                       const void *data2)
{
   /* Compare the two device numbers */
   return ((intptr_t)data1) - ((intptr_t)data2);
}

/**
 * @internal
 *
 * This functions returns pending-device node
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_device_is_pending(Eina_List *list,
                   void *event,
                   Evas_Callback_Type event_type)
{
   int device = ELM_MOUSE_DEVICE;

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_UP:
        break;

      case EVAS_CALLBACK_MULTI_UP:
        device = ((Evas_Event_Multi_Up *)event)->device;
        break;

      default:
        return NULL;
     }

   return eina_list_search_unsorted_list
            (list, _device_in_pending_cmp, (void *)(intptr_t)device);
}

/**
 * @internal
 *
 * This functions adds device to refeed-pending device list
 * @ingroup Elm_Gesture_Layer
 */
static Eina_List *
_pending_device_add(Eina_List *list,
                    void *event,
                    Evas_Callback_Type event_type)
{
   int device = ELM_MOUSE_DEVICE;

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
        break;

      case EVAS_CALLBACK_MULTI_DOWN:
        device = ((Evas_Event_Multi_Down *)event)->device;
        break;

      default:
        return list;
     }

   if (!eina_list_search_unsorted_list
         (list, _device_in_pending_cmp, (void *)(intptr_t)device))
     {
        return eina_list_append(list, (void *)(intptr_t)device);
     }

   return list;
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
   int i;
   Gesture_Info *p;
   Evas *e = evas_object_evas_get(obj);
   Eina_Bool gesture_found = EINA_FALSE;

   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   for (i = ELM_GESTURE_FIRST; i < ELM_GESTURE_LAST; i++)
     {
        p = sd->gesture[i];
        if (p)
          {
             if (p->state == ELM_GESTURE_STATE_END)
               {
                  gesture_found = EINA_TRUE;
               }
             else
               {  /* Report ABORT to all gestures that still not finished */
                  if (sd->target)
                    _state_set(p, ELM_GESTURE_STATE_ABORT,
                          sd->gesture[i]->info, EINA_FALSE);
               }
          }
     }

   _states_reset(sd); /* we are ready to start testing for gestures again */

   /* Clear all gestures intermediate data */
   {
      /* FIXME: +1 because of the mistake in the enum. */
      Gesture_Info **gitr = sd->gesture + 1;
      Tests_Array_Funcs *fitr = _glayer_tests_array + 1;
      for (; fitr->reset; fitr++, gitr++)
        {
           if (IS_TESTED_GESTURE(*gitr))
             fitr->reset(*gitr);
        }
   }

   /* Disable gesture layer so refeeded events won't be consumed by it */
   _callbacks_unregister(obj);
   while (sd->event_history_list)
     {
        Event_History *t;
        t = sd->event_history_list;
        Eina_List *pending = _device_is_pending
            (sd->pending, sd->event_history_list->event,
            sd->event_history_list->event_type);

        /* Refeed events if no gesture matched input */
        if (pending || ((!gesture_found) && (!sd->repeat_events)))
          {
             evas_event_refeed_event(e, sd->event_history_list->event,
                                     sd->event_history_list->event_type);

             if (pending)
               {
                  sd->pending = eina_list_remove_list(sd->pending, pending);
               }
             else
               {
                  sd->pending = _pending_device_add
                      (sd->pending, sd->event_history_list->event,
                      sd->event_history_list->event_type);
               }
          }

        free(sd->event_history_list->event);
        sd->event_history_list = (Event_History *)eina_inlist_remove(
            EINA_INLIST_GET(sd->event_history_list),
            EINA_INLIST_GET(sd->event_history_list));
        free(t);
     }
   _callbacks_register(obj);
   return EINA_TRUE;
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
   int i;
   Eina_Bool reset_s = EINA_TRUE, all_undefined = EINA_TRUE;

   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   /* Clear history if all we have aborted gestures */
   for (i = ELM_GESTURE_FIRST; i < ELM_GESTURE_LAST; i++)
     {  /* If no gesture started and all we have aborted gestures, reset all */
       Gesture_Info *p = sd->gesture[i];

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
 * @param sd gesture layer widget data.
 * @param states_reset flag that marks gestures were reset in history clear.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_continues_gestures_restart(void *data,
                            Eina_Bool states_reset)
{
   ELM_GESTURE_LAYER_DATA_GET(data, sd);

   /* Test all the gestures */
   {
      /* FIXME: +1 because of the mistake in the enum. */
      Gesture_Info **gitr = sd->gesture + 1;
      Tests_Array_Funcs *fitr = _glayer_tests_array + 1;
      for (; fitr->test; fitr++, gitr++)
        {
           Gesture_Info *g = *gitr;
           Eina_Bool tmp = (g) ?
             ((states_reset) || ((g->state != ELM_GESTURE_STATE_START)
                                 && (g->state != ELM_GESTURE_STATE_MOVE)))
             : EINA_FALSE;
           if (tmp && fitr->cont_reset)
             {
                fitr->cont_reset(g);
                _state_set(g, ELM_GESTURE_STATE_UNDEFINED, NULL, EINA_FALSE);
                SET_TEST_BIT(g);
             }
        }
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
_event_process(void *data,
               Evas_Object *obj EINA_UNUSED,
               void *event_info,
               Evas_Callback_Type event_type)
{
   Pointer_Event _pe;
   Pointer_Event *pe = NULL;

   ELM_GESTURE_LAYER_DATA_GET(data, sd);

   /* Start testing candidate gesture from here */
   if (_pointer_event_make(data, event_info, event_type, &_pe))
     pe = &_pe;

   /* Test all the gestures */
   {
      /* FIXME: +1 because of the mistake in the enum. */
      Gesture_Info **gitr = sd->gesture + 1;
      Tests_Array_Funcs *fitr = _glayer_tests_array + 1;
      for (; fitr->test; fitr++, gitr++)
        {
           if (IS_TESTED_GESTURE(*gitr))
             fitr->test(data, pe, event_info, event_type, (*gitr)->g_type);
        }
   }

   if (_event_flag_get(event_info, event_type) & EVAS_EVENT_FLAG_ON_HOLD)
     _event_history_add(data, event_info, event_type);

   /* we maintain list of touched devices              */
   /* We also use move to track current device x.y pos */
   if ((event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
       (event_type == EVAS_CALLBACK_MULTI_DOWN) ||
       (event_type == EVAS_CALLBACK_MOUSE_MOVE) ||
       (event_type == EVAS_CALLBACK_MULTI_MOVE))
     {
        sd->touched = _touched_device_add(sd->touched, pe);
     }
   else if ((event_type == EVAS_CALLBACK_MOUSE_UP) ||
            (event_type == EVAS_CALLBACK_MULTI_UP))
     {
        sd->touched = _touched_device_remove(sd->touched, pe);
     }

   /* Report current states and clear history if needed */
   Eina_Bool states_reset = _clear_if_finished(data);
   if (sd->glayer_continues_enable)
     _continues_gestures_restart(data, states_reset);
}

static Eina_Bool
_inside(Evas_Coord xx1,
        Evas_Coord yy1,
        Evas_Coord xx2,
        Evas_Coord yy2,
        Evas_Coord w)
{
   w >>= 1; /* Use half the distance, from center to all directions */
   if (!w)  /* use system default instead */
     w = elm_config_finger_size_get() >> 1; /* Finger size devided by 2 */

   if (xx1 < (xx2 - w))
     return EINA_FALSE;

   if (xx1 > (xx2 + w))
     return EINA_FALSE;

   if (yy1 < (yy2 - w))
     return EINA_FALSE;

   if (yy1 > (yy2 + w))
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
   Eina_List *data;
   Pointer_Event *pe;

   EINA_SAFETY_ON_NULL_RETURN(gesture);
   ELM_GESTURE_LAYER_DATA_GET(gesture->obj, sd);

   ELM_SAFE_FREE(sd->gest_taps_timeout, ecore_timer_del);

   if (!gesture->data)
     return;

   EINA_LIST_FREE(((Taps_Type *)gesture->data)->l, data)
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
   Pointer_Event *p;
   Long_Tap_Type *st;

   EINA_SAFETY_ON_NULL_RETURN(gesture);
   if (!gesture->data) return;

   st = gesture->data;

   EINA_LIST_FREE(st->touched, p)
     free(p);
   st->touched = NULL;

   ELM_SAFE_FREE(st->timeout, ecore_timer_del);
   memset(gesture->data, 0, sizeof(Long_Tap_Type));
}

static void
_momentum_test_reset(Gesture_Info *gesture)
{
   EINA_SAFETY_ON_NULL_RETURN(gesture);
   if (!gesture->data) return;

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
   Line_Type *st;
   Line_Data *t_line;

   EINA_SAFETY_ON_NULL_RETURN(gesture);
   if (!gesture->data) return;

   st = gesture->data;

   EINA_LIST_FREE(st->list, t_line)
     free(t_line);
   st->list = NULL;
}

static void
_zoom_test_reset(Gesture_Info *gesture)
{
   Zoom_Type *st;
   Evas_Modifier_Mask mask;

   EINA_SAFETY_ON_NULL_RETURN(gesture);
   if (!gesture->data) return;
   ELM_GESTURE_LAYER_DATA_GET(gesture->obj, sd);

   st = gesture->data;
   mask = evas_key_modifier_mask_get(
       evas_object_evas_get(sd->target), "Control");
   evas_object_key_ungrab(sd->target, "Control_L", mask, 0);
   evas_object_key_ungrab(sd->target, "Control_R", mask, 0);

   memset(st, 0, sizeof(Zoom_Type));
   st->zoom_distance_tolerance = sd->zoom_distance_tolerance;
   st->info.zoom = 1.0;
}

static void
_rotate_test_reset(Gesture_Info *gesture)
{
   Rotate_Type *st;

   EINA_SAFETY_ON_NULL_RETURN(gesture);
   if (!gesture->data) return;

   ELM_GESTURE_LAYER_DATA_GET(gesture->obj, sd);
   st = gesture->data;

   memset(st, 0, sizeof(Rotate_Type));
   st->info.base_angle = ELM_GESTURE_NEGATIVE_ANGLE;
   st->rotate_angular_tolerance = sd->rotate_angular_tolerance;
}

static Eina_List *
_match_fingers_compare(Eina_List *list,
                       Pointer_Event *pe1,
                       Evas_Coord w)
{
    /* Compare coords of first item in list to cur coords */
   Eina_List *pe_list;
   Eina_List *l;

   EINA_LIST_FOREACH(list, l, pe_list)
     {
        Pointer_Event *pe2 = eina_list_data_get(pe_list);

        if (_inside(pe1->x, pe1->y, pe2->x, pe2->y, w))
          return pe_list;
     }

  return NULL;
}

static int
_pe_device_compare(const void *data1,
                   const void *data2)
{
   /* Compare device of first item in list to our pe device */
   const Pointer_Event *pe1 = eina_list_data_get(data1);
   const Pointer_Event *pe2 = data2;

   if (pe1->device == pe2->device)
     return 0;
   else if (pe1->device < pe2->device)
     return -1;
   else
     return 1;
}

static Eina_List *
_pointer_event_record(Taps_Type *st,
                      Eina_List *pe_list,
                      Pointer_Event *pe,
                      Elm_Gesture_Layer_Data *sd,
                      void *event_info,
                      Evas_Callback_Type event_type)
{
   /* Keep copy of pe and record it in list */
   Pointer_Event *p = malloc(sizeof(Pointer_Event));

   memcpy(p, pe, sizeof(Pointer_Event));
   _event_consume(sd, event_info, event_type, EVAS_EVENT_FLAG_NONE);

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
 * This function computes minimum rect to bound taps at idx index
 *
 * @param taps [in] List of lists containing taps info.
 * @param idx [in] index of events taken from lists.
 * @param r [out] rect object to save info
 * @return EINA_TRUE if managed to compute rect.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_taps_rect_get(Eina_List *taps, int idx, Evas_Coord_Rectangle *r)
{  /* Build a rect bounding all taps at index idx */
   Eina_List *l;
   Evas_Coord bx = 0, by = 0;
   Eina_List *pe_list;
   Eina_Bool was_init = EINA_FALSE;

   EINA_LIST_FOREACH(taps, l, pe_list)
     {
        Pointer_Event *pe = eina_list_nth(pe_list, idx);
        if (!pe) continue;  /* Not suppose to happen */

        if (was_init)
          {
             if (pe->x < r->x) r->x = pe->x;
             if (pe->y < r->y) r->y = pe->y;
             if (pe->x > bx) bx = pe->x;
             if (pe->y > by) by = pe->y;
          }
        else
          {
             r->x = bx = pe->x;
             r->y = by = pe->y;
             was_init = EINA_TRUE;
          }
     }

   r->w = bx - r->x;
   r->h = by - r->y;
   return was_init;
}

/**
 * @internal
 *
 * This function checks if the tap gesture is done.
 *
 * @param data gesture info pointer
 * @return EINA_TRUE if it is done.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Eina_Bool
_tap_gesture_check_finish(Gesture_Info *gesture, Evas_Coord tap_finger_size)
{
   /* Here we check if taps-gesture was completed successfuly */
   /* Count how many taps were recieved on each device then   */
   /* determine if it matches n_taps_needed defined on START  */
   unsigned int i;
   Taps_Type *st = gesture->data;
   Eina_List *l;
   Eina_List *pe_list;
   Evas_Coord_Rectangle base;
   Evas_Coord_Rectangle tmp;
   if (!tap_finger_size)  /* Use system default if not set by user */
     tap_finger_size = elm_config_finger_size_get();

   if (!st->l) return EINA_FALSE;
   EINA_LIST_FOREACH(st->l, l, pe_list)
     {
        /* No match taps number on device, ABORT */
        if (eina_list_count(pe_list) != st->n_taps_needed)
          {
             return EINA_FALSE;
          }
     }

   /* Now bound each tap touches in a rect, compare diff within tolerance */
   /* Get rect based on first DOWN events for all devices */
   if (!_taps_rect_get(st->l, 0, &base))
     return EINA_FALSE;  /* Should not happen */

   for (i = 1; i < st->n_taps_needed; i++)
     {  /* Compare all other rects to base, tolerance is finger size */
        if (_taps_rect_get(st->l, i, &tmp))
          {
             if (abs(tmp.x - base.x) > tap_finger_size)
               return EINA_FALSE;

             if (abs(tmp.y - base.y) > tap_finger_size)
               return EINA_FALSE;

             if (abs((tmp.x + tmp.w) - (base.x + base.w)) > tap_finger_size)
               return EINA_FALSE;

             if (abs((tmp.y + tmp.h) - (base.y + base.h)) > tap_finger_size)
               return EINA_FALSE;
          }
     }

   return EINA_TRUE;
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
_tap_gesture_finish(void *data, Evas_Coord tap_finger_size)
{
   /* This function will test each tap gesture when timer expires */
   Elm_Gesture_State s = ELM_GESTURE_STATE_ABORT;
   Gesture_Info *gesture = data;
   Taps_Type *st = gesture->data;

   if (_tap_gesture_check_finish(gesture, tap_finger_size))
     {
        s = ELM_GESTURE_STATE_END;
     }

   st->info.n = eina_list_count(st->l);
   _state_set(gesture, s, gesture->info, EINA_FALSE);
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
   ELM_GESTURE_LAYER_DATA_GET(data, sd);

   if (IS_TESTED(ELM_GESTURE_N_TAPS))
     _tap_gesture_finish(sd->gesture[ELM_GESTURE_N_TAPS],
           sd->tap_finger_size);

   if (IS_TESTED(ELM_GESTURE_N_DOUBLE_TAPS))
     _tap_gesture_finish(sd->gesture[ELM_GESTURE_N_DOUBLE_TAPS],
           sd->tap_finger_size);

   if (IS_TESTED(ELM_GESTURE_N_TRIPLE_TAPS))
     _tap_gesture_finish(sd->gesture[ELM_GESTURE_N_TRIPLE_TAPS],
           sd->tap_finger_size);

   _clear_if_finished(data);
   sd->gest_taps_timeout = NULL;

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

   _state_set(gesture, ELM_GESTURE_STATE_MOVE,
              gesture->data, EINA_TRUE);

   return ECORE_CALLBACK_RENEW;
}

/**
 * @internal
 *
 * This function checks the state of a tap gesture.
 *
 * @param sd Gesture Layer Widget Data.
 * @param pe The recent input event as stored in pe struct.
 * @param event_info Original input event pointer.
 * @param event_type Type of original input event.
 * @param gesture what gesture is tested
 * @param how many taps for this gesture (1, 2 or 3)
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_tap_gesture_test(Evas_Object *obj,
                  Pointer_Event *pe,
                  void *event_info,
                  Evas_Callback_Type event_type,
                  Elm_Gesture_Type g_type)
{
   int taps = 0;
   Taps_Type *st;
   Gesture_Info *gesture;
   Eina_List *pe_list = NULL;
   Pointer_Event *pe_last = NULL;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;

   /* Here we fill Tap struct */
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!pe)
     return;

   gesture = sd->gesture[g_type];
   if (!gesture) return;

   switch (g_type)
     {
      case ELM_GESTURE_N_TAPS:
         taps = 1;
         break;

      case ELM_GESTURE_N_DOUBLE_TAPS:
         taps = 2;
         break;

      case ELM_GESTURE_N_TRIPLE_TAPS:
         taps = 3;
         break;

      default:
         taps = 0;
         break;
     }

   st = gesture->data;
   if (!st) /* Allocated once on first time */
     {
        st = calloc(1, sizeof(Taps_Type));
        gesture->data = st;
        _tap_gestures_test_reset(gesture);
     }

   switch (pe->event_type)
     {
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_DOWN:
         /* Each device taps (DOWN, UP event) registered in same list    */
         /* Find list for this device or start a new list if not found   */
         pe_list = eina_list_search_unsorted(st->l, _pe_device_compare, pe);
         if (pe_list)
           {  /* This device touched before, verify that this tap is on  */
              /* top of a previous tap (including a tap of other device) */
              if (!_match_fingers_compare(st->l, pe, sd->tap_finger_size))
                {  /* New DOWN event is not on top of any prev touch     */
                   ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                         &st->info, EINA_FALSE);
                   _event_consume(sd, event_info, event_type, ev_flag);

                   return;
                }
           }

         /* All tests are good, register this tap in device list */
         pe_list = _pointer_event_record
            (st, pe_list, pe, sd, event_info, event_type);

         if (!sd->gest_taps_timeout)
           {
              if (sd->double_tap_timeout > 0.0)
                {
                   sd->gest_taps_timeout =
                      ecore_timer_add(sd->double_tap_timeout,
                            _multi_tap_timeout, gesture->obj);
                }
           }
         else  /* We re-allocate gest_taps_timeout between taps */
           ecore_timer_reset(sd->gest_taps_timeout);

         if ((pe->device == 0) && (eina_list_count(pe_list) == 1))
           {  /* This is the first mouse down we got */
              ev_flag = _state_set(gesture, ELM_GESTURE_STATE_START,
                    &st->info, EINA_FALSE);
              _event_consume(sd, event_info, event_type, ev_flag);

              st->n_taps_needed = taps * 2;  /* count DOWN and UP */

              return;
           }
         else if (eina_list_count(pe_list) > st->n_taps_needed)
           {  /* If we arleady got too many touches for this gesture. */
              ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                    &st->info, EINA_FALSE);
           }

         if (gesture->state == ELM_GESTURE_STATE_MOVE)
           {  /* Report MOVE if all devices have same DOWN/UP count */
              /* Should be in MOVE state from last UP event */
              Eina_List *l;
              Eina_Bool move = EINA_TRUE;
              unsigned int n = 0;

              EINA_LIST_FOREACH(st->l, l, pe_list)
                {
                   if (n == 0)
                     {
                        n = eina_list_count(pe_list);
                     }
                   else if (n != eina_list_count(pe_list))
                     {
                        move = EINA_FALSE;
                     }
                }

              if (move && (n > 0))
                {
                   ev_flag = _state_set(gesture, ELM_GESTURE_STATE_MOVE,
                         &st->info, EINA_TRUE);
                }
           }

         break;

      case EVAS_CALLBACK_MULTI_UP:
      case EVAS_CALLBACK_MOUSE_UP:
         pe_list = eina_list_search_unsorted(st->l, _pe_device_compare, pe);
         if (!pe_list) return;

         _pointer_event_record(st, pe_list, pe, sd, event_info, event_type);

         if (((gesture->g_type == ELM_GESTURE_N_TAPS) &&
                  !IS_TESTED(ELM_GESTURE_N_DOUBLE_TAPS) &&
                  !IS_TESTED(ELM_GESTURE_N_TRIPLE_TAPS)) ||
               ((gesture->g_type == ELM_GESTURE_N_DOUBLE_TAPS) &&
                !IS_TESTED(ELM_GESTURE_N_TRIPLE_TAPS)))
           {  /* Test for finish immidiatly, not waiting for timeout */
              if (_tap_gesture_check_finish(gesture, sd->tap_finger_size))
                {
                   _tap_gesture_finish(gesture, sd->tap_finger_size);
                   return;
                }
           }

         if ((gesture->state == ELM_GESTURE_STATE_START) ||
               (gesture->state == ELM_GESTURE_STATE_MOVE))
           {  /* Tap gesture started, no finger on surface. Report MOVE */
              Eina_List *l;
              Eina_Bool move = EINA_TRUE;
              unsigned int n = 0;

              /* Report move only if all devices have same DOWN/UP count */
              EINA_LIST_FOREACH(st->l, l, pe_list)
                {
                   if (n == 0)
                     {
                        n = eina_list_count(pe_list);
                     }
                   else if (n != eina_list_count(pe_list))
                     {
                        move = EINA_FALSE;
                     }
                }

              if ((move && (n > 0)) && (n < st->n_taps_needed))
                {  /* Set number of fingers and report MOVE */
                   /* We don't report MOVE when (n >= st->n_taps_needed)
                      because will be END or ABORT at this stage */
                   st->info.n = eina_list_count(st->l);
                   ev_flag = _state_set(gesture, ELM_GESTURE_STATE_MOVE,
                         &st->info, EINA_TRUE);
                }
           }

         break;

      case EVAS_CALLBACK_MULTI_MOVE:
      case EVAS_CALLBACK_MOUSE_MOVE:
         /* Verify that user didn't move out of tap area before next tap */
         /* BUT: we need to skip some MOVE events coming before DOWN     */
         /* when user taps next tap. So fetch LAST recorded event for    */
         /* device (DOWN or UP event), ignore all MOVEs if last was UP   */
         pe_last = eina_list_data_get(eina_list_last(
                  eina_list_search_unsorted(st->l, _pe_device_compare, pe)));

         if (pe_last)
           {  /* pe_last is the last event recorded for this device */
              if ((pe_last->event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
                    (pe_last->event_type == EVAS_CALLBACK_MULTI_DOWN))
                {  /* Test only MOVE events that come after DOWN event */
                   if (!_inside(pe_last->x, pe_last->y, pe->x, pe->y,
                            sd->tap_finger_size))
                     {
                        ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                              &st->info, EINA_FALSE);
                        _event_consume(sd, event_info, event_type, ev_flag);
                     }
                }
           }
         break;

      default:
         return;
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
                     Evas_Coord *x_out,
                     Evas_Coord *y_out,
                     Pointer_Event *pe)
{
   Eina_List *l;
   Pointer_Event *p;
   Evas_Coord x = 0, y = 0;

   if (!eina_list_count(st->touched))
     return;

   EINA_LIST_FOREACH(st->touched, l, p)
     {  /* Accumulate all then take avarage */
       if (p->device == pe->device) /* This will take care of values
                                     * coming from MOVE event */
         {
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
_n_long_tap_test(Evas_Object *obj,
                 Pointer_Event *pe,
                 void *event_info,
                 Evas_Callback_Type event_type,
                 Elm_Gesture_Type g_type)
{
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   Gesture_Info *gesture;
   Long_Tap_Type *st;

   /* Here we fill Recent_Taps struct and fire-up click/tap timers */
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!pe) /* this happens when unhandled event arrived */
     return;  /* see _make_pointer_event function */

   gesture = sd->gesture[g_type];
   if (!gesture) return;

   st = gesture->data;
   if (!st) /* Allocated once on first time */
     {
        st = calloc(1, sizeof(Long_Tap_Type));
        gesture->data = st;
        _n_long_tap_test_reset(gesture);
     }

   switch (pe->event_type)
     {
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_DOWN:
        st->touched = _touched_device_add(st->touched, pe);
        st->info.n = eina_list_count(st->touched);

        _event_consume(sd, event_info, event_type, ev_flag);
        _compute_taps_center(st, &st->info.x, &st->info.y, pe);
        st->center_x = st->info.x;  /* Update coords for */
        st->center_y = st->info.y;  /* reporting START  */

        /* This is the first mouse down we got */
        if (eina_list_count(st->touched) == 1)
          {
             _state_set(gesture, ELM_GESTURE_STATE_START,
                   gesture->data, EINA_FALSE);
             st->info.timestamp = pe->timestamp;

             /* To test long tap */
             /* When this timer expires, gesture STARTED */
             if ((!st->timeout) && (sd->long_tap_start_timeout > 0.0))
               st->timeout = ecore_timer_add(sd->long_tap_start_timeout,
                                             _long_tap_timeout, gesture);
          }
        else
          {
             if (st->timeout)
                ecore_timer_reset(st->timeout);
          }

        break;

      case EVAS_CALLBACK_MULTI_UP:
      case EVAS_CALLBACK_MOUSE_UP:
        st->touched = _touched_device_remove(st->touched, pe);
        _compute_taps_center(st, &st->center_x, &st->center_y, pe);
        if (st->info.n)
          {
             if (gesture->state == ELM_GESTURE_STATE_MOVE)
               ev_flag = _state_set(gesture, ELM_GESTURE_STATE_END,
                                    &st->info, EINA_FALSE);
             else
               ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                                    &st->info, EINA_FALSE);

             ELM_SAFE_FREE(st->timeout, ecore_timer_del);
             _event_consume(sd, event_info, event_type, ev_flag);
          }

        break;

      case EVAS_CALLBACK_MULTI_MOVE:
      case EVAS_CALLBACK_MOUSE_MOVE:
        if (st->info.n &&
            ((gesture->state == ELM_GESTURE_STATE_START) ||
             /* Report MOVE only if STARTED */
             (gesture->state == ELM_GESTURE_STATE_MOVE)))
          {
             Evas_Coord x = 0;
             Evas_Coord y = 0;

             _compute_taps_center(st, &x, &y, pe);
             /* ABORT if user moved fingers out of tap area */
             if (!_inside(x, y, st->center_x, st->center_y,
                      sd->tap_finger_size))
               {
                  ELM_SAFE_FREE(st->timeout, ecore_timer_del);

                  /* Report MOVE if gesture started */
                  ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                        &st->info, EINA_FALSE);
               }

             _event_consume(sd, event_info, event_type, ev_flag);
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
 * @param xx1 x coord where user started gesture.
 * @param yy1 y coord where user started gesture.
 * @param xx2 x coord where user completed gesture.
 * @param yy2 y coord where user completed gesture.
 * @param t1x timestamp for X, when user started gesture.
 * @param t1y timestamp for Y, when user started gesture.
 * @param t2  timestamp when user completed gesture.
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_momentum_set(Elm_Gesture_Momentum_Info *momentum,
              Evas_Coord xx1,
              Evas_Coord yy1,
              Evas_Coord xx2,
              Evas_Coord yy2,
              unsigned int t1x,
              unsigned int t1y,
              unsigned int t2)
{
   Evas_Coord velx = 0, vely = 0, vel;
   Evas_Coord dx = xx2 - xx1;
   Evas_Coord dy = yy2 - yy1;
   int dtx = t2 - t1x;
   int dty = t2 - t1y;

   if (dtx > 0)
     velx = (dx * 1000) / dtx;

   if (dty > 0)
     vely = (dy * 1000) / dty;

   vel = sqrt((velx * velx) + (vely * vely));

   if ((_elm_config->thumbscroll_friction > 0.0) &&
       (vel > _elm_config->thumbscroll_momentum_threshold)) /* report
                                                             * momentum */
     {
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
 * @param xx1 first finger x location.
 * @param yy1 first finger y location.
 * @param xx2 second finger x location.
 * @param yy2 second finger y location.
 *
 * @return angle of the line between (xx1,yy1), (xx2,yy2) in Deg.
 * Angles now are given in DEG, not RAD.
 * ZERO angle at 12-oclock, growing clockwise.
 *
 * @ingroup Elm_Gesture_Layer
 */
static double
_angle_get(Evas_Coord xx1,
           Evas_Coord yy1,
           Evas_Coord xx2,
           Evas_Coord yy2)
{
   double a, xx, yy, rt = (-1);

   xx = fabs(xx2 - xx1);
   yy = fabs(yy2 - yy1);

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
        else
          {  /* Vertical line */
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

/**
 * @internal
 *
 * This function is used for computing the magnitude and direction
 * of vector between two points.
 *
 * @param xx1 first finger x location.
 * @param yy1 first finger y location.
 * @param xx2 second finger x location.
 * @param yy2 second finger y location.
 * @param l length computed (output)
 * @param a angle computed (output)
 *
 * @ingroup Elm_Gesture_Layer
 */
static void
_vector_get(Evas_Coord xx1,
            Evas_Coord yy1,
            Evas_Coord xx2,
            Evas_Coord yy2,
            Evas_Coord *l,
            double *a)
{
   Evas_Coord xx, yy;

   xx = xx2 - xx1;
   yy = yy2 - yy1;
   *l = (Evas_Coord)sqrt((xx * xx) + (yy * yy));
   *a = _angle_get(xx1, yy1, xx2, yy2);
}

static int
_direction_get(Evas_Coord xx1,
               Evas_Coord xx2)
{
   if (xx2 < xx1) return -1;
   if (xx2 > xx1) return 1;

   return 0;
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
_momentum_test(Evas_Object *obj,
               Pointer_Event *pe,
               void *event_info,
               Evas_Callback_Type event_type,
               Elm_Gesture_Type g_type)
{
   Eina_List *l;
   Pointer_Event *p;
   Momentum_Type *st;
   Gesture_Info *gesture;
   Pointer_Event pe_local;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   Elm_Gesture_State state_to_report = ELM_GESTURE_STATE_MOVE;
   unsigned int cnt = 1; /* We start counter counting current pe event */

   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   gesture = sd->gesture[g_type];
   if (!gesture) return;

   /* When continues enable = TRUE a gesture may START on MOVE event */
   /* We don't allow this to happen with the if-statement below.     */
   /* When continues enable = FALSE a gesture may START on DOWN only */
   /* Therefor it would NOT start on MOVE event.                     */
   /* NOTE that touched list is updated AFTER this function returns  */
   /* so (count == 0) when we get here on first touch on surface.    */
   if ((sd->glayer_continues_enable) && (!eina_list_count(sd->touched)))
     return;  /* Got move on mouse-over move */

   st = gesture->data;
   if (!st) /* Allocated once on first time */
     {
        st = calloc(1, sizeof(Momentum_Type));
        gesture->data = st;
        _momentum_test_reset(gesture);
     }

   if (!pe)
     return;

   /* First make avarage of all touched devices to determine center point */
   pe_local = *pe; /* Copy pe event info to local */
   EINA_LIST_FOREACH(sd->touched, l, p)
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
                 (sd->glayer_continues_enable))    /* start also on MOVE */
               {   /* We start on MOVE when cont-enabled only */
                 st->line_st.x = st->line_end.x = pe_local.x;
                 st->line_st.y = st->line_end.y = pe_local.y;
                 st->t_st_x = st->t_st_y = st->t_end = pe_local.timestamp;
                 st->xdir = st->ydir = 0;
                 st->info.x2 = st->info.x1 = pe_local.x;
                 st->info.y2 = st->info.y1 = pe_local.y;
                 st->info.tx = st->info.ty = pe_local.timestamp;
                 ev_flag = _state_set(gesture, ELM_GESTURE_STATE_START,
                                      &st->info, EINA_FALSE);
                 _event_consume(sd, event_info, event_type, ev_flag);
               }

             return;
          }

        if (st->t_up)
          {
             Eina_Bool force = EINA_TRUE;   /* for move state */

             /* ABORT if got DOWN or MOVE event after UP+timeout */
             if ((st->t_up + ELM_GESTURE_MULTI_TIMEOUT) < pe_local.timestamp)
               {
                  state_to_report = ELM_GESTURE_STATE_ABORT;
                  force = EINA_FALSE;
               }

             /* We report state but don't compute momentum now */
             ev_flag = _state_set(gesture, state_to_report, &st->info,
                                  force);
             _event_consume(sd, event_info, event_type, ev_flag);
             return;  /* Stop computing when user remove finger */
          }

        /*  Too long of a wait, reset all values */
        if ((pe_local.timestamp - ELM_GESTURE_MOMENTUM_TIMEOUT) > st->t_end)
          {
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

             xdir = _direction_get(st->line_end.x, pe_local.x);
             ydir = _direction_get(st->line_end.y, pe_local.y);
             if (xdir && (xdir != st->xdir))
               {
                  st->line_st.x = st->line_end.x;
                  st->info.tx = st->t_st_x = st->t_end;
                  st->xdir = xdir;
               }

             if (ydir && (ydir != st->ydir))
               {
                  st->line_st.y = st->line_end.y;
                  st->info.ty = st->t_st_y = st->t_end;
                  st->ydir = ydir;
               }
          }

        st->info.x2 = st->line_end.x = pe_local.x;
        st->info.y2 = st->line_end.y = pe_local.y;
        st->t_end = pe_local.timestamp;
        _momentum_set(&st->info, st->line_st.x, st->line_st.y,
                      pe_local.x, pe_local.y, st->t_st_x, st->t_st_y,
                      pe_local.timestamp);

        ev_flag = _state_set(gesture, state_to_report, &st->info,
                             EINA_TRUE);
        _event_consume(sd, event_info, event_type, ev_flag);
        break;

      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
        st->t_up = pe_local.timestamp; /* Record recent up event time */
        if ((cnt > 1) || /* Ignore if more fingers touch surface        */
            (!st->t_st_x)) /* IGNORE if info was cleared, long press,move */
          return;

        /* Too long of a wait, reset all values */
        if ((pe_local.timestamp - ELM_GESTURE_MOMENTUM_TIMEOUT) > st->t_end)
          {
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

        if ((fabs(st->info.mx) > ELM_GESTURE_MINIMUM_MOMENTUM) ||
            (fabs(st->info.my) > ELM_GESTURE_MINIMUM_MOMENTUM))
          state_to_report = ELM_GESTURE_STATE_END;
        else
          state_to_report = ELM_GESTURE_STATE_ABORT;

        ev_flag = _state_set(gesture, state_to_report, &st->info,
                             EINA_FALSE);
        _event_consume(sd, event_info, event_type, ev_flag);
        return;

      default:
        return;
     }
}

static int
_line_device_compare(const void *data1,
                     const void *data2)
{
   /* Compare device component of line struct */
   const Line_Data *ln1 = data1;
   const int *device = data2;

   if (ln1->t_st) /* Compare only with lines that started */
     return ln1->device - (*device);

   return -1;
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
_single_line_process(Elm_Gesture_Line_Info *info,
                     Line_Data *st,
                     Pointer_Event *pe,
                     Evas_Callback_Type event_type)
{
   /* Record events and set momentum for line pointed by st */
   if (!pe)
     return EINA_FALSE;

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MULTI_MOVE:
        if (!st->t_st) /* This happens only when line starts */
          {
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
   _momentum_set(&info->momentum, st->line_st.x, st->line_st.y, pe->x, pe->y,
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
_n_line_test(Evas_Object *obj,
             Pointer_Event *pe,
             void *event_info,
             Evas_Callback_Type event_type,
             Elm_Gesture_Type g_type)
{
   unsigned cnt;
   Line_Type *st;
   Eina_List *list;
   Gesture_Info *gesture;
   Line_Data *line = NULL;

   if (!pe)
     return;

   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   gesture = sd->gesture[g_type];
   if (!gesture ) return;

   /* When continues enable = TRUE a gesture may START on MOVE event */
   /* We don't allow this to happen with the if-statement below.     */
   /* When continues enable = FALSE a gesture may START on DOWN only */
   /* Therefor it would NOT start on MOVE event.                     */
   /* NOTE that touched list is updated AFTER this function returns  */
   /* so (count == 0) when we get here on first touch on surface.    */
   if ((sd->glayer_continues_enable) && (!eina_list_count(sd->touched)))
     return;  /* Got move on mouse-over move */

   st = gesture->data;
   if (!st)
     {
        st = calloc(1, sizeof(Line_Type));
        gesture->data = st;
     }

   list = st->list;
   cnt = eina_list_count(list);

   if (cnt) /* list is not empty, locate this device on list */
     {
        line = (Line_Data *)eina_list_search_unsorted
            (st->list, _line_device_compare, &pe->device);
     }

   if (!line) /* List is empty or device not found, new line-struct on
               * START only */
     {
        if ((event_type == EVAS_CALLBACK_MOUSE_DOWN) ||
            (event_type == EVAS_CALLBACK_MULTI_DOWN) ||
            ((sd->glayer_continues_enable) &&   /* START on MOVE also */
             ((event_type == EVAS_CALLBACK_MOUSE_MOVE) ||
              /* Allocate new item on START only */
              (event_type == EVAS_CALLBACK_MULTI_MOVE))))
          {
             line = calloc(1, sizeof(Line_Data));
             _line_data_reset(line);
             list = eina_list_append(list, line);
             st->list = list;
          }
     }

   if (!line) /* This may happen on MOVE that comes before DOWN      */
     return;  /* No line-struct to work with, can't continue testing */

   /* update st with input */
   if (_single_line_process(&st->info, line, pe, event_type))
     _event_consume(sd, event_info, event_type, EVAS_EVENT_FLAG_NONE);

   /* Get direction and magnitude of the line */
   double angle;
   _vector_get(line->line_st.x, line->line_st.y, pe->x, pe->y,
               &line->line_length, &angle);

   /* These are used later to compare lines length */
   Evas_Coord shortest_line_len = line->line_length;
   Evas_Coord longest_line_len = line->line_length;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;

   /* Now update line-state */
   if (line->t_st) /* Analyze line only if line started */
     {
        if (line->line_angle >= 0.0) /* if line direction was set, we
                                      * test if broke tolerance */
          {
             double a = fabs(angle - line->line_angle);
             /* Distance from line */
             double d = (tan(DEG2RAD(a))) * line->line_length;
             /* Broke tolerance: abort line and start a new one */
             if ((d > sd->line_distance_tolerance) ||
                 (a > sd->line_angular_tolerance))
               {
                  ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                                       &st->info, EINA_FALSE);
                  _event_consume(sd, event_info, event_type, ev_flag);
                  return;
               }

             /* We may finish line if momentum is zero */
             if (sd->glayer_continues_enable)
               {
                  /* This is for continues-gesture */
                  /* Finish line on zero momentum for continues gesture */
                  if ((!st->info.momentum.mx) && (!st->info.momentum.my))
                    {
                       line->line_end.x = pe->x;
                       line->line_end.y = pe->y;
                       line->t_end = pe->timestamp;
                    }
               }
          }
        else
          {  /* Record the line angle as it broke minimum length for line */
            if (line->line_length >= sd->line_min_length)
              st->info.angle = line->line_angle = angle;
          }

        if (line->t_end)
          {
             if (line->line_angle < 0.0) /* it's not a line, too short
                                          * more close to a tap */
               {
                  ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                                       &st->info, EINA_FALSE);
                  _event_consume(sd, event_info, event_type, ev_flag);
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
             if (t_line->line_angle >= 0) /* Compare angle only with
                                           * lines with direction
                                           * defined */
               {
                  if (fabs(base_angle - t_line->line_angle) >
                      sd->line_angular_tolerance)
                    lines_parallel = EINA_FALSE;
               }
          }

        if (t_line->line_length) /* update only if this line is used */
          {
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
        /* user lift one finger then starts again without line-end - ABORT */
        (event_type == EVAS_CALLBACK_MULTI_DOWN)))
     {
        ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
                             EINA_FALSE);
        _event_consume(sd, event_info, event_type, ev_flag);
        return;
     }

   if (!lines_parallel) /* Lines are NOT at same direction, abort this
                         * gesture */
     {
        ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
                             EINA_FALSE);
        _event_consume(sd, event_info, event_type, ev_flag);
        return;
     }

   /* We report ABORT if lines length are NOT matching when fingers are up */
   if ((longest_line_len - shortest_line_len) >
       (elm_config_finger_size_get() * 2))
     {
        ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
                             EINA_FALSE);
        _event_consume(sd, event_info, event_type, ev_flag);
        return;
     }

   /* We consider FLICK as a fast line.ABORT if take too long to finish */
   if ((g_type == ELM_GESTURE_N_FLICKS) && ((tm_end - tm_start) >
                                            sd->flick_time_limit_ms))
     {
        ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT, &st->info,
                             EINA_FALSE);
        _event_consume(sd, event_info, event_type, ev_flag);
        return;
     }

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
        if ((started) && (started == ended))
          {
             ev_flag = _state_set(gesture, ELM_GESTURE_STATE_END,
                                  &st->info, EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);
          }

        return;

      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
        if (started)
          {
             /* For continues gesture */
             if (sd->glayer_continues_enable && (started == ended))
               {
                  ev_flag = _state_set(gesture, ELM_GESTURE_STATE_END,
                                       &st->info, EINA_FALSE);
                  _event_consume(sd, event_info, event_type, ev_flag);
               }
             else
               {   /* When continues, may START on MOVE event too */
                 Elm_Gesture_State s = ELM_GESTURE_STATE_MOVE;

                 /* This happens when: on n > 1 lines then one finger up */
                 /* caused abort, then put finger down.                  */
                 /* This will stop line from starting again.             */
                 /* Number of lines, MUST match touched-device in list   */
                 if ((!sd->glayer_continues_enable) &&
                     (eina_list_count(st->list) <
                      eina_list_count(sd->touched)))
                   s = ELM_GESTURE_STATE_ABORT;

                 if (gesture->state == ELM_GESTURE_STATE_UNDEFINED)
                   s = ELM_GESTURE_STATE_START;

                 ev_flag = _state_set(gesture, s, &st->info, EINA_TRUE);
                 _event_consume(sd, event_info, event_type, ev_flag);
               }
          }
        break;

      default:
        return;   /* Unhandeld event type */
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
_on_rotation_broke_tolerance(Rotate_Type *st)
{
   if (st->info.base_angle < 0)
     return EINA_FALSE;  /* Angle has to be computed first */

   if (st->rotate_angular_tolerance < 0)
     return EINA_TRUE;

   double low = st->info.base_angle - st->rotate_angular_tolerance;
   double high = st->info.base_angle + st->rotate_angular_tolerance;
   double t = st->info.angle;

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

   if ((t < low) || (t > high)) /* This marks that roation action has
                                 * started */
     {
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
 * @param xx1 first finger x location.
 * @param yy1 first finger y location.
 * @param xx2 second finger x location.
 * @param yy2 second finger y location.
 * @param x  Get center point x cord (output)
 * @param y  Get center point y cord (output)
 *
 * @return length of the line between (xx1,yy1), (xx2,yy2) in pixels.
 *
 * @ingroup Elm_Gesture_Layer
 */
static Evas_Coord
_finger_gap_length_get(Evas_Coord xx1,
                       Evas_Coord yy1,
                       Evas_Coord xx2,
                       Evas_Coord yy2,
                       Evas_Coord *x,
                       Evas_Coord *y)
{
   double a, b, xx, yy, gap;
   xx = fabs(xx2 - xx1);
   yy = fabs(yy2 - yy1);
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

/**
 * @internal
 *
 * This function is used for computing zoom value.
 *
 * @param st Pointer to zoom data based on user input.
 * @param tm_end Recent input event timestamp.
 * @param zoom_val Current computed zoom value.
 *
 * @return zoom momentum
 *
 * @ingroup Elm_Gesture_Layer
 */
static double
_zoom_momentum_get(Zoom_Type *st,
                   unsigned int tm_end,
                   double zoom_val)
{
   unsigned int tm_total;
   if (!st->m_st_tm) /* Init, and we don't start computing momentum yet */
     {
        st->m_st_tm = st->m_prev_tm = tm_end;
        st->m_base = zoom_val;
        return 0.0;
     }

   if ((tm_end - ELM_GESTURE_MOMENTUM_DELAY) < st->m_st_tm)
     return 0.0;  /* we don't start to compute momentum yet */

   if (st->dir) /* if direction was already defined, check if changed */
     {
        if (((st->dir < 0) && (zoom_val > st->info.zoom)) ||
            /* Direction changed, reset momentum */
            ((st->dir > 0) && (zoom_val < st->info.zoom)))
          {
             st->m_st_tm = 0;
             st->dir = (-st->dir);
             return 0.0;
          }
     }
   else
     st->dir = (zoom_val > st->info.zoom) ? 1 : -1;  /* init */

   if ((tm_end - ELM_GESTURE_MOMENTUM_TIMEOUT) > st->m_prev_tm)
     {
        st->m_st_tm = 0; /* Rest momentum when waiting too long */
        return 0.0;
     }

   st->m_prev_tm = tm_end;
   tm_total = tm_end - st->m_st_tm;

   if (tm_total)
     return ((zoom_val - st->m_base) * 1000) / tm_total;
   else
     return 0.0;
}

/**
 * @internal
 *
 * This function is used for computing zoom value.
 *
 * @param st Pointer to zoom data based on user input.
 * @param xx1 first finger x location.
 * @param yy1 first finger y location.
 * @param xx2 second finger x location.
 * @param yy2 second finger y location.
 * @param factor zoom-factor, used to determine how fast zoom works.
 *
 * @return zoom value, when 1.0 means no zoom, 0.5 half size...
 *
 * @ingroup Elm_Gesture_Layer
 */
static double
_zoom_compute(Zoom_Type *st,
              Evas_Coord xx1,
              Evas_Coord yy1,
              Evas_Coord xx2,
              Evas_Coord yy2,
              double zoom_finger_factor)
{
   double rt = 1.0;
   unsigned int tm_end = (st->zoom_mv.timestamp > st->zoom_mv1.timestamp) ?
     st->zoom_mv.timestamp : st->zoom_mv1.timestamp;

   Evas_Coord diam = _finger_gap_length_get(xx1, yy1, xx2, yy2,
                                            &st->info.x, &st->info.y);

   st->info.radius = diam / 2;

   if (!st->zoom_base)
     {
        st->zoom_base = diam;
        return st->info.zoom;
     }

   if (st->zoom_distance_tolerance) /* zoom tolerance <> ZERO, means
                                    * zoom action NOT started yet */
     {
        /* avoid jump with zoom value when break tolerance */
        if (diam < (st->zoom_base - st->zoom_distance_tolerance))
          {
             st->zoom_base -= st->zoom_distance_tolerance;
             st->zoom_distance_tolerance = 0;
          }

        /* avoid jump with zoom value when break tolerance */
        if (diam > (st->zoom_base + st->zoom_distance_tolerance))
          {
             st->zoom_base += st->zoom_distance_tolerance;
             st->zoom_distance_tolerance = 0;
          }

        return rt;
     }

   /* We use factor only on the difference between gap-base   */
   /* if gap=120, base=100, we get ((120-100)/100)=0.2*factor */
   rt = ((1.0) + ((((float)diam - (float)st->zoom_base) /
                   (float)st->zoom_base) * zoom_finger_factor));

   /* Momentum: zoom per second: */
   st->info.momentum = _zoom_momentum_get(st, tm_end, rt);

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
_zoom_with_wheel_test(Evas_Object *obj,
                      void *event_info,
                      Evas_Callback_Type event_type,
                      Elm_Gesture_Type g_type)
{
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!sd->gesture[g_type]) return;

   Gesture_Info *gesture_zoom = sd->gesture[g_type];
   Zoom_Type *st = gesture_zoom->data;
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   if (!st) /* Allocated once on first time, used for zoom intermediate data */
     {
        st = calloc(1, sizeof(Zoom_Type));
        gesture_zoom->data = st;
        _zoom_test_reset(gesture_zoom);
     }

   switch (event_type)
     {
      case EVAS_CALLBACK_KEY_UP:
      {
         Evas_Event_Key_Up *p = event_info;
         if ((!strcmp(p->key, "Control_L")) ||
             /* Test if we ended a zoom gesture when releasing CTRL */
             (!strcmp(p->key, "Control_R")))
           {
              if ((st->zoom_wheel) &&
                  ((gesture_zoom->state == ELM_GESTURE_STATE_START) ||
                   /* User released CTRL after zooming */
                   (gesture_zoom->state == ELM_GESTURE_STATE_MOVE)))
                {
                   st->info.momentum = _zoom_momentum_get
                       (st, p->timestamp, st->info.zoom);

                   ev_flag = _state_set
                       (gesture_zoom, ELM_GESTURE_STATE_END, &st->info,
                       EINA_FALSE);
                   _event_consume(sd, event_info, event_type, ev_flag);

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
               ((Evas_Event_Mouse_Wheel *)event_info)->modifiers,
               "Control")) /* if using wheel witout CTRL after starting zoom */
           {
              if ((st->zoom_wheel) &&
                  ((gesture_zoom->state == ELM_GESTURE_STATE_START) ||
                   (gesture_zoom->state == ELM_GESTURE_STATE_MOVE)))
                {
                   ev_flag = _state_set
                       (gesture_zoom, ELM_GESTURE_STATE_END, &st->info,
                       EINA_FALSE);
                   _event_consume(sd, event_info, event_type, ev_flag);

                   return;
                }
              else
                return;  /* Ignore mouse-wheel without control */
           }

         /* Using mouse wheel with CTRL for zoom */
         /* (zoom_wheel == NULL) and (zoom_distance_tolerance == 0) we
          * continue a zoom gesture */
         if (st->zoom_wheel || (st->zoom_distance_tolerance == 0))
           {
              force = EINA_TRUE;
              s = ELM_GESTURE_STATE_MOVE;
           }
         else
           { /* On first wheel event, report START */
             Evas_Modifier_Mask mask = evas_key_modifier_mask_get(
                 evas_object_evas_get(sd->target), "Control");
             force = EINA_FALSE;
             s = ELM_GESTURE_STATE_START;
             if (!evas_object_key_grab
                   (sd->target, "Control_L", mask, 0, EINA_FALSE))
               ERR("Failed to Grabbed CTRL_L");
             if (!evas_object_key_grab
                   (sd->target, "Control_R", mask, 0, EINA_FALSE))
               ERR("Failed to Grabbed CTRL_R");
           }

         st->zoom_distance_tolerance = 0; /* Cancel tolerance */
         st->zoom_wheel = (Evas_Event_Mouse_Wheel *)event_info;
         st->info.x = st->zoom_wheel->canvas.x;
         st->info.y = st->zoom_wheel->canvas.y;

         if (st->zoom_wheel->z < 0) /* zoom in */
           st->info.zoom += (sd->zoom_finger_factor * sd->zoom_wheel_factor);

         if (st->zoom_wheel->z > 0) /* zoom out */
           st->info.zoom -= (sd->zoom_finger_factor * sd->zoom_wheel_factor);

         if (st->info.zoom < 0.0)
           st->info.zoom = 0.0;

         st->info.momentum = _zoom_momentum_get
             (st, st->zoom_wheel->timestamp, st->info.zoom);

         ev_flag = _state_set(gesture_zoom, s, &st->info, force);
         _event_consume(sd, event_info, event_type, ev_flag);
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
_zoom_test(Evas_Object *obj,
           Pointer_Event *pe,
           void *event_info,
           Evas_Callback_Type event_type,
           Elm_Gesture_Type g_type)
{
   /* Test for wheel zoom. */
   _zoom_with_wheel_test(obj, event_info, event_type, ELM_GESTURE_ZOOM);

   if (!_elm_config->glayer_zoom_finger_enable)
     return;

   if (!pe)
     return;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!sd->gesture[g_type]) return;

   Gesture_Info *gesture_zoom = sd->gesture[g_type];
   Zoom_Type *st = gesture_zoom->data;

   if (!st) /* Allocated once on first time, used for zoom data */
     {
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
        if ((!sd->glayer_continues_enable) &&
            (!st->zoom_st.timestamp))
          return;
        // fallthrough is intentional
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
      { /* Here we take care of zoom-start and zoom move */
        Eina_List *l;
        Pointer_Event *p;

        if (eina_list_count(sd->touched) > 2) /* Process zoom only
                                               * when 2 fingers on
                                               * surface */
          {
             ev_flag = _state_set
                 (gesture_zoom, ELM_GESTURE_STATE_ABORT, &st->info,
                 EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);

             return;
          }

        if (!st->zoom_st.timestamp) /* Now scan touched-devices list
                                     * and find other finger */
          {
             EINA_LIST_FOREACH(sd->touched, l, p)
               { /* Device of other finger <> pe device */
                 if (p->device != pe->device)
                   break;
               }

             if (!p) /* Single finger on touch */
               return;

             /* Record down fingers */
             _event_consume(sd, event_info, event_type, ev_flag);
             memcpy(&st->zoom_st, pe, sizeof(Pointer_Event));
             memcpy(&st->zoom_st1, p, sizeof(Pointer_Event));

             /* Set mv field as well to be ready for MOVE events  */
             memcpy(&st->zoom_mv, pe, sizeof(Pointer_Event));
             memcpy(&st->zoom_mv1, p, sizeof(Pointer_Event));

             /* Here we have zoom_st, zoom_st1 set, report START  */
             /* Set zoom-base after BOTH down events  recorded    */
             /* Compute length of line between fingers zoom start */
             st->info.zoom = 1.0;
             st->zoom_base = _finger_gap_length_get
                 (st->zoom_st1.x, st->zoom_st1.y, st->zoom_st.x, st->zoom_st.y,
                 &st->info.x, &st->info.y);

             st->info.radius = st->zoom_base / 2;

             if ((gesture_zoom->state != ELM_GESTURE_STATE_START) &&
                 /* zoom started with mouse-wheel, don't report twice */
                 (gesture_zoom->state != ELM_GESTURE_STATE_MOVE))
               {
                  ev_flag = _state_set
                      (gesture_zoom, ELM_GESTURE_STATE_START, &st->info,
                      EINA_FALSE);
                  _event_consume(sd, event_info, event_type, ev_flag);
               }

             return; /* Zoom started */
          } /* End of ZOOM_START handling */

        /* if we got here, we have (exacally) two fingers on surfce */
        /* we also after START, report MOVE */
        /* First detect which finger moved  */
        if (pe->device == st->zoom_mv.device)
          memcpy(&st->zoom_mv, pe, sizeof(Pointer_Event));
        else if (pe->device == st->zoom_mv1.device)
          memcpy(&st->zoom_mv1, pe, sizeof(Pointer_Event));

        /* Compute change in zoom as fingers move */
        st->info.zoom = _zoom_compute(st,
                                      st->zoom_mv.x, st->zoom_mv.y,
                                      st->zoom_mv1.x, st->zoom_mv1.y,
                                      sd->zoom_finger_factor);

        if (!st->zoom_distance_tolerance) /* Zoom broke tolerance,
                                           * report move */
          {
             double d = st->info.zoom - st->next_step;
             if (d < 0.0)
               d = (-d);

             if (d >= sd->zoom_step) /* Report move in steps */
               {
                  st->next_step = st->info.zoom;

                  ev_flag = _state_set(gesture_zoom,
                                       ELM_GESTURE_STATE_MOVE,
                                       &st->info, EINA_TRUE);
                  _event_consume(sd, event_info, event_type, ev_flag);
               }
          } /* End of ZOOM_MOVE handling */

        return;
      }

      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
        /* Reset timestamp of finger-up.This is used later
           by _zoom_test_reset() to retain finger-down data */
        _event_consume(sd, event_info, event_type, ev_flag);
        if (((st->zoom_wheel) || (st->zoom_base)) &&
            (st->zoom_distance_tolerance == 0))
          {
             ev_flag = _state_set(gesture_zoom, ELM_GESTURE_STATE_END,
                                  &st->info, EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);

             return;
          }

        /* if we got here not a ZOOM */
        /* Must be != undefined, if gesture started */
        if (gesture_zoom->state != ELM_GESTURE_STATE_UNDEFINED)
          {
             ev_flag = _state_set
                 (gesture_zoom, ELM_GESTURE_STATE_ABORT, &st->info,
                 EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);
          }

        _zoom_test_reset(gesture_zoom);

        return;

      default:
        return;
     }
}

static void
_rotate_properties_get(Rotate_Type *st,
                       Evas_Coord xx1,
                       Evas_Coord yy1,
                       Evas_Coord xx2,
                       Evas_Coord yy2,
                       double *angle)
{
   /* FIXME: Fix momentum computation, it's wrong */
   double prev_angle = *angle;

   st->info.radius = _finger_gap_length_get(xx1, yy1, xx2, yy2,
                                            &st->info.x, &st->info.y) / 2;

   *angle = _angle_get(xx1, yy1, xx2, yy2);

   if (angle == &st->info.angle) /* Fingers are moving, compute momentum */
     {
        unsigned int tm_start =
          (st->rotate_st.timestamp > st->rotate_st1.timestamp)
          ?  st->rotate_st.timestamp : st->rotate_st1.timestamp;
        unsigned int tm_end =
          (st->rotate_mv.timestamp > st->rotate_mv1.timestamp)
          ? st->rotate_mv.timestamp : st->rotate_mv1.timestamp;

        unsigned int tm_total = tm_end - tm_start;
        if (tm_total) /* Momentum computed as:
                         accumulated roation angle (deg) divided by time */
          {
             double m = 0;
             if (((prev_angle < 90) && ((*angle) > 270)) ||
                 /* We circle passing ZERO point */
                 ((prev_angle > 270) && ((*angle) < 90)))
               {
                  prev_angle = (*angle);
               }
             else m = prev_angle - (*angle);

             st->accum_momentum += m;

             if ((tm_end - st->prev_momentum_tm) < 100)
               st->prev_momentum += m;
             else
               {
                  if (fabs(st->prev_momentum) < 0.002)
                    st->accum_momentum = 0.0;  /* reset momentum */

                  st->prev_momentum = 0.0; /* Start again    */
               }

             st->prev_momentum_tm = tm_end;
             st->info.momentum = (st->accum_momentum * 1000) / tm_total;
          }
     }
   else
     st->info.momentum = 0;
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
_rotate_test(Evas_Object *obj,
             Pointer_Event *pe,
             void *event_info,
             Evas_Callback_Type event_type,
             Elm_Gesture_Type g_type)
{
   Evas_Event_Flags ev_flag = EVAS_EVENT_FLAG_NONE;
   Gesture_Info *gesture;
   Rotate_Type *st = NULL;

   if (!_elm_config->glayer_rotate_finger_enable)
     return;

   if (!pe)
     return;

   ELM_GESTURE_LAYER_DATA_GET(obj, sd);

   if (!sd->gesture[g_type]) return;

   gesture = sd->gesture[g_type];
   if (!gesture) return ;

   st = gesture->data;
   if (!st) /* Allocated once on first time */
     {
       st = calloc(1, sizeof(Rotate_Type));
       gesture->data = st;
       _rotate_test_reset(gesture);
     }

   switch (event_type)
     {
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MULTI_MOVE:
        /* if non-continues mode and gesture NOT started, ignore MOVE */
        if ((!sd->glayer_continues_enable) &&
            (!st->rotate_st.timestamp))
          return;
        // fallthrough is intentional
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MULTI_DOWN:
      { /* Here we take care of rotate-start and rotate move */
        Eina_List *l;
        Pointer_Event *p;

        if (eina_list_count(sd->touched) > 2) /* Process rotate only
                                               * when 2 fingers on
                                               * surface */
          {
             ev_flag = _state_set
                 (gesture, ELM_GESTURE_STATE_ABORT, &st->info, EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);

             return;
          }

        if (!st->rotate_st.timestamp) /* Now scan touched-devices list
                                       * and find other finger */
          {
             EINA_LIST_FOREACH(sd->touched, l, p)
               { /* Device of other finger <> pe device */
                 if (p->device != pe->device)
                   break;
               }

             if (!p)
               return;  /* Single finger on touch */

             /* Record down fingers */
             _event_consume(sd, event_info, event_type, ev_flag);
             memcpy(&st->rotate_st, pe, sizeof(Pointer_Event));
             memcpy(&st->rotate_st1, p, sizeof(Pointer_Event));

             /* Set mv field as well to be ready for MOVE events  */
             memcpy(&st->rotate_mv, pe, sizeof(Pointer_Event));
             memcpy(&st->rotate_mv1, p, sizeof(Pointer_Event));

             /* Here we have rotate_st, rotate_st1 set, report START  */
             /* Set rotate-base after BOTH down events  recorded    */
             /* Compute length of line between fingers rotate start */
             _rotate_properties_get(st,
                                    st->rotate_st.x, st->rotate_st.y,
                                    st->rotate_st1.x, st->rotate_st1.y,
                                    &st->info.base_angle);

             ev_flag = _state_set(gesture, ELM_GESTURE_STATE_START,
                                  &st->info, EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);

             return; /* Rotate started */
          } /* End of ROTATE_START handling */

        /* if we got here, we have (exacally) two fingers on surfce */
        /* we also after START, report MOVE */
        /* First detect which finger moved  */
        if (pe->device == st->rotate_mv.device)
          memcpy(&st->rotate_mv, pe, sizeof(Pointer_Event));
        else if (pe->device == st->rotate_mv1.device)
          memcpy(&st->rotate_mv1, pe, sizeof(Pointer_Event));

        /* Compute change in rotate as fingers move */
        _rotate_properties_get(st,
                               st->rotate_mv.x, st->rotate_mv.y,
                               st->rotate_mv1.x, st->rotate_mv1.y,
                               &st->info.angle);

        if (_on_rotation_broke_tolerance(st)) /* Rotation broke
                                               * tolerance, report
                                               * move */
          {
             double d = st->info.angle - st->next_step;
             if (d < 0)
               d = (-d);

             if (d >= sd->rotate_step) /* Report move in steps */
               {
                  st->next_step = st->info.angle;

                  ev_flag = _state_set
                      (gesture, ELM_GESTURE_STATE_MOVE, &st->info, EINA_TRUE);
                  _event_consume(sd, event_info, event_type, ev_flag);
               }
          } /* End of ROTATE_MOVE handling */

        return;
      }

      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MULTI_UP:
        _event_consume(sd, event_info, event_type, ev_flag);
        /* Reset timestamp of finger-up.This is used later
           by rotate_test_reset() to retain finger-down data */
        if (st->rotate_angular_tolerance < 0)
          {
             ev_flag = _state_set(gesture, ELM_GESTURE_STATE_END,
                                  &st->info, EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);

             return;
          }

        /* Must be != undefined, if gesture started */
        if (gesture->state != ELM_GESTURE_STATE_UNDEFINED)
          {
             ev_flag = _state_set(gesture, ELM_GESTURE_STATE_ABORT,
                                  &st->info, EINA_FALSE);
             _event_consume(sd, event_info, event_type, ev_flag);
          }

        _rotate_test_reset(gesture);
        return;

      default:
        return;
     }
}

EOLIAN static Eina_Bool
_elm_gesture_layer_elm_widget_disable(Eo *obj, Elm_Gesture_Layer_Data *_pd EINA_UNUSED)
{
   if (elm_widget_disabled_get(obj))
     _callbacks_unregister(obj);
   else
     _callbacks_register(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_gesture_layer_evas_object_smart_add(Eo *obj, Elm_Gesture_Layer_Data *priv)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->line_min_length =
     _elm_config->glayer_line_min_length * elm_config_finger_size_get();
   priv->zoom_distance_tolerance = _elm_config->glayer_zoom_distance_tolerance
     * elm_config_finger_size_get();
   priv->line_distance_tolerance = _elm_config->glayer_line_distance_tolerance
     * elm_config_finger_size_get();
   priv->zoom_finger_factor = _elm_config->glayer_zoom_finger_factor;
   /* mouse wheel zoom steps */
   priv->zoom_wheel_factor = _elm_config->glayer_zoom_wheel_factor;
   priv->rotate_angular_tolerance =
     _elm_config->glayer_rotate_angular_tolerance;
   priv->line_angular_tolerance = _elm_config->glayer_line_angular_tolerance;
   priv->flick_time_limit_ms = _elm_config->glayer_flick_time_limit_ms;
   priv->long_tap_start_timeout = _elm_config->glayer_long_tap_start_timeout;
   priv->repeat_events = EINA_TRUE;
   priv->glayer_continues_enable = _elm_config->glayer_continues_enable;

   /* FIXME: Hack to get around old configs - if too small, enlarge. */
   if (_elm_config->glayer_double_tap_timeout < 0.00001)
     _elm_config->glayer_double_tap_timeout = 0.25;
   priv->double_tap_timeout = _elm_config->glayer_double_tap_timeout;

   memset(priv->gesture, 0, sizeof(priv->gesture));
}

static void _cbs_clean(Elm_Gesture_Layer_Data *sd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type);

EOLIAN static void
_elm_gesture_layer_evas_object_smart_del(Eo *obj, Elm_Gesture_Layer_Data *sd)
{
   Pointer_Event *data;
   int i;

   /* Clear all gestures intermediate data, stop any timers */
   {
      /* FIXME: +1 because of the mistake in the enum. */
      Gesture_Info **gitr = sd->gesture + 1;
      Tests_Array_Funcs *fitr = _glayer_tests_array + 1;
      for (; fitr->reset; fitr++, gitr++)
        {
           if (IS_TESTED_GESTURE(*gitr))
             fitr->reset(*gitr);
        }
   }

   /* First Free all gestures internal data structures */
   for (i = 0; i < ELM_GESTURE_LAST; i++)
     if (sd->gesture[i])
       {
          if (sd->gesture[i]->data)
            free(sd->gesture[i]->data);

          _cbs_clean(sd, i, ELM_GESTURE_STATE_START);
          _cbs_clean(sd, i, ELM_GESTURE_STATE_MOVE);
          _cbs_clean(sd, i, ELM_GESTURE_STATE_END);
          _cbs_clean(sd, i, ELM_GESTURE_STATE_ABORT);
          free(sd->gesture[i]);
          sd->gesture[i] = NULL; /* Referenced by _event_history_clear */
       }
   ecore_timer_del(sd->gest_taps_timeout);

   /* Then take care of clearing events */
   _event_history_clear(obj);
   sd->pending = eina_list_free(sd->pending);

   EINA_LIST_FREE(sd->touched, data)
     free(data);

   if (!elm_widget_disabled_get(obj))
     _callbacks_unregister(obj);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_gesture_layer_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static void
_elm_gesture_layer_eo_base_constructor(Eo *obj, Elm_Gesture_Layer_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EOLIAN static Eina_Bool
_elm_gesture_layer_hold_events_get(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd)
{
   return !sd->repeat_events;
}

EOLIAN static void
_elm_gesture_layer_hold_events_set(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd, Eina_Bool hold_events)
{
   sd->repeat_events = !(!!hold_events);
}

EOLIAN static double
_elm_gesture_layer_zoom_step_get(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd)
{
   return sd->zoom_step;
}

EOLIAN static void
_elm_gesture_layer_zoom_step_set(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd, double step)
{
   if (step < 0) return;

   sd->zoom_step = step;
}

EOLIAN static double
_elm_gesture_layer_rotate_step_get(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd)
{
   return sd->rotate_step;
}

EOLIAN static void
_elm_gesture_layer_rotate_step_set(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd, double step)
{
   if (step < 0) return;

   sd->rotate_step = step;
}

EOLIAN static Eina_Bool
_elm_gesture_layer_attach(Eo *obj, Elm_Gesture_Layer_Data *sd, Evas_Object *target)
{
   if (!target) return EINA_FALSE;

   /* if was attached before, unregister callbacks first */
   if (sd->target)
     _callbacks_unregister(obj);

   sd->target = target;

   _callbacks_register(obj);
   return EINA_TRUE;
}

static void
_cbs_clean(Elm_Gesture_Layer_Data *sd,
          Elm_Gesture_Type idx,
          Elm_Gesture_State cb_type)
{
   if (!sd->gesture[idx]) return;

   Func_Data *cb_info;
   EINA_INLIST_FREE(sd->gesture[idx]->cbs[cb_type], cb_info)
     {
        sd->gesture[idx]->cbs[cb_type] = eina_inlist_remove(
              sd->gesture[idx]->cbs[cb_type], EINA_INLIST_GET(cb_info));
        free(cb_info);
     }
   SET_TEST_BIT(sd->gesture[idx]);
}

EOLIAN static void
_elm_gesture_layer_cb_set(Eo *obj, Elm_Gesture_Layer_Data *sd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   /* Clear gesture intermediate data, stop any timers */
   if (IS_TESTED_GESTURE(sd->gesture[idx]))
     _glayer_tests_array[idx].reset(sd->gesture[idx]);

   _cbs_clean(sd, idx, cb_type); // for ABI compat.
   eo_do(obj, elm_obj_gesture_layer_cb_add(idx, cb_type, cb, data));
}

EOLIAN static void
_elm_gesture_layer_cb_add(Eo *obj, Elm_Gesture_Layer_Data *sd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   if (!cb) return;

   Gesture_Info *p;

   if (!sd->gesture[idx])
     sd->gesture[idx] = calloc(1, sizeof(Gesture_Info));
   if (!sd->gesture[idx]) return;

   Func_Data *cb_info = calloc(1, sizeof(*cb_info));
   if (!cb_info) return;
   cb_info->cb = cb;
   cb_info->user_data = data;

   p = sd->gesture[idx];
   p->obj = obj;
   p->g_type = idx;
   p->cbs[cb_type] = eina_inlist_append(p->cbs[cb_type],
         EINA_INLIST_GET(cb_info));
   p->state = ELM_GESTURE_STATE_UNDEFINED;
   SET_TEST_BIT(p);
}

EOLIAN static void
_elm_gesture_layer_cb_del(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   if (!sd->gesture[idx]) return;

   Eina_Inlist *itr;
   Func_Data *cb_info;
   EINA_INLIST_FOREACH_SAFE(sd->gesture[idx]->cbs[cb_type], itr, cb_info)
     {
        if (cb_info->cb == cb && cb_info->user_data == data)
          {
             /* Clear gesture intermediate data, stop any timers */
             if (IS_TESTED_GESTURE(sd->gesture[idx]))
                _glayer_tests_array[idx].reset(sd->gesture[idx]);

             sd->gesture[idx]->cbs[cb_type] = eina_inlist_remove(
                   sd->gesture[idx]->cbs[cb_type], EINA_INLIST_GET(cb_info));
             free(cb_info);
             SET_TEST_BIT(sd->gesture[idx]);
             return;
          }
     }
}

EAPI void
elm_gesture_layer_line_min_length_set(Evas_Object *obj, int line_min_length)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->line_min_length = line_min_length;
}

EAPI int
elm_gesture_layer_line_min_length_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->line_min_length;
}

EAPI void
elm_gesture_layer_zoom_distance_tolerance_set(Evas_Object *obj, Evas_Coord zoom_distance_tolerance)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->zoom_distance_tolerance = zoom_distance_tolerance;
}

EAPI Evas_Coord
elm_gesture_layer_zoom_distance_tolerance_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->zoom_distance_tolerance;
}

EAPI void
elm_gesture_layer_line_distance_tolerance_set(Evas_Object *obj, Evas_Coord line_distance_tolerance)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->line_distance_tolerance = line_distance_tolerance;
}

EAPI Evas_Coord
elm_gesture_layer_line_distance_tolerance_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->line_distance_tolerance;
}

EAPI void
elm_gesture_layer_line_angular_tolerance_set(Evas_Object *obj, double line_angular_tolerance)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->line_angular_tolerance = line_angular_tolerance;
}

EAPI double
elm_gesture_layer_line_angular_tolerance_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->line_angular_tolerance;
}

EAPI void
elm_gesture_layer_zoom_wheel_factor_set(Evas_Object *obj, double zoom_wheel_factor)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->zoom_wheel_factor = zoom_wheel_factor;
}

EAPI double
elm_gesture_layer_zoom_wheel_factor_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->zoom_wheel_factor;
}

EAPI void
elm_gesture_layer_zoom_finger_factor_set(Evas_Object *obj, double zoom_finger_factor)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->zoom_finger_factor = zoom_finger_factor;
}

EAPI double
elm_gesture_layer_zoom_finger_factor_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->zoom_finger_factor;
}

EAPI void
elm_gesture_layer_rotate_angular_tolerance_set(Evas_Object *obj, double rotate_angular_tolerance)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->rotate_angular_tolerance = rotate_angular_tolerance;
}

EAPI double
elm_gesture_layer_rotate_angular_tolerance_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->rotate_angular_tolerance;
}

EAPI void
elm_gesture_layer_flick_time_limit_ms_set(Evas_Object *obj, unsigned int flick_time_limit_ms)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->flick_time_limit_ms = flick_time_limit_ms;
}

EAPI unsigned int
elm_gesture_layer_flick_time_limit_ms_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->flick_time_limit_ms;
}

EAPI void
elm_gesture_layer_long_tap_start_timeout_set(Evas_Object *obj, double long_tap_start_timeout)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->long_tap_start_timeout = long_tap_start_timeout;
}

EAPI double
elm_gesture_layer_long_tap_start_timeout_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->long_tap_start_timeout;
}

EAPI void
elm_gesture_layer_continues_enable_set(Evas_Object *obj, Eina_Bool continues_enable)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->glayer_continues_enable = continues_enable;
}

EAPI Eina_Bool
elm_gesture_layer_continues_enable_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->glayer_continues_enable;
}

EAPI void
elm_gesture_layer_double_tap_timeout_set(Evas_Object *obj, double double_tap_timeout)
{
   ELM_GESTURE_LAYER_CHECK(obj);
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   sd->double_tap_timeout = double_tap_timeout;
}

EAPI double
elm_gesture_layer_double_tap_timeout_get(const Evas_Object *obj)
{
   ELM_GESTURE_LAYER_CHECK(obj) 0.0;
   ELM_GESTURE_LAYER_DATA_GET(obj, sd);
   return sd->double_tap_timeout;
}

EOLIAN static void
_elm_gesture_layer_tap_finger_size_set(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd, Evas_Coord sz)
{
   if (sz < 0)
      sz = 0;  /* Should not be negative, will reset to system value */

   sd->tap_finger_size = sz;
}

EOLIAN static Evas_Coord
_elm_gesture_layer_tap_finger_size_get(Eo *obj EINA_UNUSED, Elm_Gesture_Layer_Data *sd)
{
   return sd->tap_finger_size;
}

static void
_elm_gesture_layer_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_gesture_layer.eo.c"
