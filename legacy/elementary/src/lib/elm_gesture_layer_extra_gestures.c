#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Eina.h>
#include <Elementary.h>

//#define DEBUGON
#ifdef DEBUGON
# define gl_debug(x...) fprintf(stderr, __FILE__": " x)
#else
# define gl_debug(x...) do { } while (0)
#endif

struct _Func_Data
{
   EINA_INLIST;
   void                 *user_data; /**< Holds user data to CB (like sd) */
   Elm_Gesture_Event_Cb cb;
};
typedef struct _Func_Data Func_Data;

struct _Tap_Longpress_Info
{
   Evas_Object          *obj;
   Eina_Inlist          *cbs[ELM_GESTURE_STATE_ABORT + 1]; /**< Callback info (Func_Data) for states */
   void                 *data;

   Ecore_Timer          *timer_between_taps;
   unsigned int         nb_taps_on_single : 7;
   Eina_Bool            long_tap_started  : 1;
};

typedef struct _Tap_Longpress_Info Tap_Longpress_Info;

static Evas_Event_Flags
_cb_call(Tap_Longpress_Info *info, Elm_Gesture_State state, void *event_info)
{
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   Func_Data *cb_info;
   EINA_INLIST_FOREACH(info->cbs[state], cb_info)
      flags |= cb_info->cb(cb_info->user_data, event_info);
   return flags;
}

static Evas_Event_Flags
_tap_long_single_tap_start_cb(void *data, void *event_info)
{
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   if (!info->nb_taps_on_single)
     {
        gl_debug("\n%s\n", __FUNCTION__);
        _cb_call(info, ELM_GESTURE_STATE_START, event_info);
     }
   return flags;
}

static Evas_Event_Flags
_tap_long_single_tap_abort_cb(void *data, void *event_info EINA_UNUSED)
{
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   if (!info->long_tap_started)
     {
        gl_debug("%s\n", __FUNCTION__);
        _cb_call(info, ELM_GESTURE_STATE_ABORT, NULL);
        info->nb_taps_on_single = 0;
     }
   return flags;
}

static Eina_Bool
_tap_long_timeout(void *data)
{
   gl_debug("%s\n", __FUNCTION__);
   Tap_Longpress_Info *info = data;
   _tap_long_single_tap_abort_cb(info, NULL);
   info->timer_between_taps = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Evas_Event_Flags
_tap_long_single_tap_end_cb(void *data, void *event_info)
{
   gl_debug("%s\n", __FUNCTION__);
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   double timeout_between_taps = elm_gesture_layer_double_tap_timeout_get(info->obj);
   info->timer_between_taps = ecore_timer_add(timeout_between_taps,
                            _tap_long_timeout, info);
   info->nb_taps_on_single = ((Elm_Gesture_Taps_Info *)event_info)->n;
   return flags;
}

static Evas_Event_Flags
_tap_long_long_tap_start_cb(void *data, void *event_info EINA_UNUSED)
{
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   if (info->nb_taps_on_single && info->timer_between_taps)
     {
        gl_debug("%s\n", __FUNCTION__);
        info->long_tap_started = EINA_TRUE;
        ecore_timer_del(info->timer_between_taps);
        info->timer_between_taps = NULL;
     }
   return flags;
}

static Evas_Event_Flags
_tap_long_long_tap_abort_cb(void *data, void *event_info EINA_UNUSED)
{
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   if (info->long_tap_started)
     {
        gl_debug("%s\n", __FUNCTION__);
        _cb_call(info, ELM_GESTURE_STATE_ABORT, NULL);
        info->nb_taps_on_single = 0;
        info->long_tap_started = EINA_FALSE;
     }
   return flags;
}

static Evas_Event_Flags
_tap_long_long_tap_move_cb(void *data, void *event_info)
{
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   if (info->long_tap_started)
     {
        if (((Elm_Gesture_Taps_Info *)event_info)->n != info->nb_taps_on_single)
          {
             _tap_long_long_tap_abort_cb(info, NULL);
          }
        else
          {
             gl_debug("%s\n", __FUNCTION__);
             _cb_call(info, ELM_GESTURE_STATE_MOVE, event_info);
          }
     }
   return flags;
}

static Evas_Event_Flags
_tap_long_long_tap_end_cb(void *data, void *event_info)
{
   Tap_Longpress_Info *info = data;
   Evas_Event_Flags flags = EVAS_EVENT_FLAG_NONE;
   if (info->long_tap_started)
     {
        gl_debug("%s\n", __FUNCTION__);
        _cb_call(info, ELM_GESTURE_STATE_END, event_info);
        info->long_tap_started = EINA_FALSE;
        info->nb_taps_on_single = 0;
     }
   return flags;
}

static void
_object_delete(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Tap_Longpress_Info *info = evas_object_data_get(obj, "Tap-Longpress");
   if (info)
     {
        Eina_Inlist *itr;
        Func_Data *cb_info;
        int state;
        for (state = ELM_GESTURE_STATE_START; state <= ELM_GESTURE_STATE_ABORT; state++)
          {
             EINA_INLIST_FOREACH_SAFE(info->cbs[state], itr, cb_info)
               {
                  info->cbs[state] = eina_inlist_remove(
                        info->cbs[state], EINA_INLIST_GET(cb_info));
                  free(cb_info);
               }
          }
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_START, _tap_long_single_tap_start_cb, info);
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_ABORT, _tap_long_single_tap_abort_cb, info);
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END, _tap_long_single_tap_end_cb, info);
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_START, _tap_long_long_tap_start_cb, info);
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_MOVE, _tap_long_long_tap_move_cb, info);
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_ABORT, _tap_long_long_tap_abort_cb, info);
        elm_gesture_layer_cb_del(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_END, _tap_long_long_tap_end_cb, info);
        evas_object_data_del(obj, "Tap-Longpress");
        free(info);
     }
}

EAPI void elm_gesture_layer_tap_longpress_cb_add(Evas_Object *obj, Elm_Gesture_State state, Elm_Gesture_Event_Cb cb, void *data)
{
   Tap_Longpress_Info *info = evas_object_data_get(obj, "Tap-Longpress");
   if (!info)
     {
        info = calloc(1, sizeof(*info));
        info->obj = obj;
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_START, _tap_long_single_tap_start_cb, info);
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_ABORT, _tap_long_single_tap_abort_cb, info);
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END, _tap_long_single_tap_end_cb, info);
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_START, _tap_long_long_tap_start_cb, info);
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_MOVE, _tap_long_long_tap_move_cb, info);
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_ABORT, _tap_long_long_tap_abort_cb, info);
        elm_gesture_layer_cb_add(obj, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_END, _tap_long_long_tap_end_cb, info);
        evas_object_data_set(obj, "Tap-Longpress", info);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _object_delete, NULL);
     }

   Func_Data *cb_info = calloc(1, sizeof(*cb_info));
   if (!cb_info) return;
   cb_info->cb = cb;
   cb_info->user_data = data;
   info->cbs[state] = eina_inlist_append(info->cbs[state],
         EINA_INLIST_GET(cb_info));
}

EAPI void elm_gesture_layer_tap_longpress_cb_del(Evas_Object *obj, Elm_Gesture_State state, Elm_Gesture_Event_Cb cb, void *data)
{
   Tap_Longpress_Info *info = evas_object_data_get(obj, "Tap-Longpress");
   if (!info) return;

   Eina_Inlist *itr;
   Func_Data *cb_info;
   EINA_INLIST_FOREACH_SAFE(info->cbs[state], itr, cb_info)
     {
        if (cb_info->cb == cb && cb_info->user_data == data)
          {
             info->cbs[state] = eina_inlist_remove(
                   info->cbs[state], EINA_INLIST_GET(cb_info));
             free(cb_info);
             break;
          }
     }
   if (!info->cbs[ELM_GESTURE_STATE_START] &&
         !info->cbs[ELM_GESTURE_STATE_MOVE] &&
         !info->cbs[ELM_GESTURE_STATE_END] &&
         !info->cbs[ELM_GESTURE_STATE_ABORT])
     {
        _object_delete(NULL, NULL, obj, NULL);
     }
}
