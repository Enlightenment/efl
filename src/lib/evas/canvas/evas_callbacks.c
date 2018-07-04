#define EFL_CANVAS_OBJECT_BETA
#define EVAS_CANVAS_BETA

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

EVAS_MEMPOOL(_mp_pc);

extern Eina_Hash* signals_hash_table;

/* Legacy events, do not use anywhere */
static const Efl_Event_Description _EVAS_OBJECT_EVENT_FREE = EFL_EVENT_DESCRIPTION("free");
static const Efl_Event_Description _EVAS_OBJECT_EVENT_DEL = EFL_EVENT_DESCRIPTION("del");
#define EVAS_OBJECT_EVENT_FREE (&(_EVAS_OBJECT_EVENT_FREE))
#define EVAS_OBJECT_EVENT_DEL (&(_EVAS_OBJECT_EVENT_DEL))

/**
 * Evas events descriptions for Eo.
 */
#define DEFINE_EVAS_CALLBACKS(FUNC, LAST, ...)                          \
  static const Efl_Event_Description *FUNC(unsigned int index)          \
  {                                                                     \
     static const Efl_Event_Description *internals[LAST] = { NULL };    \
                                                                        \
     if (index >= LAST) return NULL;                                    \
     if (internals[0] == NULL)                                          \
       {                                                                \
          memcpy(internals,                                             \
                 ((const Efl_Event_Description*[]) { __VA_ARGS__ }),    \
                 sizeof ((const Efl_Event_Description *[]) { __VA_ARGS__ })); \
       }                                                                \
     return internals[index];                                           \
  }

DEFINE_EVAS_CALLBACKS(_legacy_evas_callback_table, EVAS_CALLBACK_LAST,
                      EFL_EVENT_POINTER_IN,
                      EFL_EVENT_POINTER_OUT,
                      EFL_EVENT_POINTER_DOWN,
                      EFL_EVENT_POINTER_UP,
                      EFL_EVENT_POINTER_MOVE,
                      EFL_EVENT_POINTER_WHEEL,
                      EFL_EVENT_FINGER_DOWN,
                      EFL_EVENT_FINGER_UP,
                      EFL_EVENT_FINGER_MOVE,
                      EVAS_OBJECT_EVENT_FREE,
                      EFL_EVENT_KEY_DOWN,
                      EFL_EVENT_KEY_UP,
                      EFL_EVENT_FOCUS_IN,
                      EFL_EVENT_FOCUS_OUT,
                      EFL_GFX_ENTITY_EVENT_SHOW,
                      EFL_GFX_ENTITY_EVENT_HIDE,
                      EFL_GFX_ENTITY_EVENT_MOVE,
                      EFL_GFX_ENTITY_EVENT_RESIZE,
                      EFL_GFX_ENTITY_EVENT_RESTACK,
                      EVAS_OBJECT_EVENT_DEL,
                      EFL_EVENT_HOLD,
                      EFL_GFX_ENTITY_EVENT_CHANGE_SIZE_HINTS,
                      EFL_GFX_IMAGE_EVENT_PRELOAD,
                      EFL_CANVAS_SCENE_EVENT_FOCUS_IN,
                      EFL_CANVAS_SCENE_EVENT_FOCUS_OUT,
                      EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE,
                      EVAS_CANVAS_EVENT_RENDER_FLUSH_POST,
                      EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_IN,
                      EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_OUT,
                      EFL_GFX_IMAGE_EVENT_UNLOAD,
                      EFL_CANVAS_SCENE_EVENT_RENDER_PRE,
                      EFL_CANVAS_SCENE_EVENT_RENDER_POST,
                      EFL_GFX_IMAGE_EVENT_RESIZE,
                      EFL_CANVAS_SCENE_EVENT_DEVICE_CHANGED,
                      EFL_EVENT_POINTER_AXIS,
                      EVAS_CANVAS_EVENT_VIEWPORT_RESIZE );

static inline Evas_Callback_Type
_legacy_evas_callback_type(const Efl_Event_Description *desc)
{
   Evas_Callback_Type type;

   for (type = 0; type < EVAS_CALLBACK_LAST; type++)
     {
        if (_legacy_evas_callback_table(type) == desc)
          return type;
     }

   return EVAS_CALLBACK_LAST;
}

typedef enum {
   EFL_EVENT_TYPE_NULL,
   EFL_EVENT_TYPE_OBJECT,
   EFL_EVENT_TYPE_STRUCT,
   EFL_EVENT_TYPE_POINTER,
   EFL_EVENT_TYPE_KEY,
   EFL_EVENT_TYPE_HOLD,
   EFL_EVENT_TYPE_FOCUS
} Efl_Event_Info_Type;

typedef struct
{
   EINA_INLIST;
   union {
      Evas_Event_Cb         evas_cb;
      Evas_Object_Event_Cb  object_cb;
   } func;
   void                    *data;
   Evas_Callback_Type       type;
   Efl_Event_Info_Type      efl_event_type;
} Evas_Event_Cb_Wrapper_Info;

static int
_evas_event_efl_event_info_type(Evas_Callback_Type type)
{
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
      case EVAS_CALLBACK_MOUSE_OUT:
      case EVAS_CALLBACK_MOUSE_DOWN:
      case EVAS_CALLBACK_MOUSE_UP:
      case EVAS_CALLBACK_MOUSE_MOVE:
      case EVAS_CALLBACK_MOUSE_WHEEL:
      case EVAS_CALLBACK_MULTI_DOWN:
      case EVAS_CALLBACK_MULTI_UP:
      case EVAS_CALLBACK_MULTI_MOVE:
      case EVAS_CALLBACK_AXIS_UPDATE:
        return EFL_EVENT_TYPE_POINTER;

      case EVAS_CALLBACK_KEY_DOWN:
      case EVAS_CALLBACK_KEY_UP:
        return EFL_EVENT_TYPE_KEY;

      case EVAS_CALLBACK_HOLD:
        return EFL_EVENT_TYPE_HOLD;

      case EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN:
      case EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT: /* Efl.Canvas.Object */
        return EFL_EVENT_TYPE_OBJECT;

      case EVAS_CALLBACK_RENDER_POST: /* Efl_Gfx_Event_Render_Post */
        return EFL_EVENT_TYPE_STRUCT;

      case EVAS_CALLBACK_DEVICE_CHANGED: /* Efl.Input.Device */
        return EFL_EVENT_TYPE_OBJECT;

      case EVAS_CALLBACK_FOCUS_IN:
      case EVAS_CALLBACK_FOCUS_OUT:
          return EFL_EVENT_TYPE_FOCUS;

      default:
        return EFL_EVENT_TYPE_NULL;
     }
}

static void
_eo_evas_object_cb(void *data, const Efl_Event *event)
{
   Evas_Event_Flags *event_flags = NULL, evflags = EVAS_EVENT_FLAG_NONE;
   Efl_Input_Event *efl_event_info = event->info;
   Evas_Event_Cb_Wrapper_Info *info = data;
   void *event_info;
   Evas *evas;

   if (!info->func.object_cb) return;
   evas = evas_object_evas_get(event->object);

   event_info = event->info;
   switch (info->efl_event_type)
     {
      case EFL_EVENT_TYPE_POINTER:
        event_info = efl_input_pointer_legacy_info_fill(evas, efl_event_info, info->type, &event_flags);
        break;

      case EFL_EVENT_TYPE_KEY:
        event_info = efl_input_key_legacy_info_fill(efl_event_info, &event_flags);
        break;

      case EFL_EVENT_TYPE_HOLD:
        event_info = efl_input_hold_legacy_info_fill(efl_event_info, &event_flags);
        break;

      case EFL_EVENT_TYPE_FOCUS:
      case EFL_EVENT_TYPE_NULL:
         info->func.object_cb(info->data, evas, event->object, NULL);
         return;

      case EFL_EVENT_TYPE_STRUCT:
      case EFL_EVENT_TYPE_OBJECT:
        info->func.object_cb(info->data, evas, event->object, event_info);
        return;

      default: return;
     }

   if (!event_info) return;
   if (event_flags) evflags = *event_flags;
   info->func.object_cb(info->data, evas, event->object, event_info);
   if (event_flags && (evflags != *event_flags))
     efl_input_event_flags_set(efl_event_info, *event_flags);
}

static void
_eo_evas_cb(void *data, const Efl_Event *event)
{
   Evas_Event_Cb_Wrapper_Info *info = data;
   Efl_Input_Event *efl_event_info = event->info;
   Evas *evas = event->object;
   void *event_info;

   if (!info->func.evas_cb) return;

   if (event->desc == EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_IN ||
       event->desc == EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_OUT)
     {
        event_info = efl_input_focus_object_get(efl_event_info);
        goto emit;
     }

   event_info = event->info;
   switch (info->efl_event_type)
     {
      case EFL_EVENT_TYPE_POINTER:
        event_info = efl_input_pointer_legacy_info_fill(evas, efl_event_info, info->type, NULL);
        break;

      case EFL_EVENT_TYPE_KEY:
        event_info = efl_input_key_legacy_info_fill(efl_event_info, NULL);
        break;

      case EFL_EVENT_TYPE_HOLD:
        event_info = efl_input_hold_legacy_info_fill(efl_event_info, NULL);
        break;

      case EFL_EVENT_TYPE_FOCUS:
      case EFL_EVENT_TYPE_NULL:
        event_info = NULL;
        break;

      case EFL_EVENT_TYPE_STRUCT:
      case EFL_EVENT_TYPE_OBJECT:
        break;
     }

emit:
   info->func.evas_cb(info->data, event->object, event_info);
}

void
_evas_post_event_callback_call_real(Evas *eo_e, Evas_Public_Data *e, int min_event_id)
{
   Evas_Post_Callback *pc;
   Eina_List *l, *l_next;
   int skip = 0;

   if (e->delete_me) return;

   _evas_walk(e);
   e->running_post_events++;
   EINA_LIST_FOREACH_SAFE(e->post_events, l, l_next, pc)
     {
        if ((unsigned int) pc->event_id < (unsigned int) min_event_id) break;
        e->post_events = eina_list_remove_list(e->post_events, l);
        if ((!skip) && (!e->delete_me) && (!pc->delete_me))
          {
             if (!pc->func((void*)pc->data, eo_e)) skip = 1;
          }
        EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
   e->running_post_events--;
   _evas_unwalk(e);

   if (!e->running_post_events && e->post_events
       && (e->current_event == EVAS_CALLBACK_LAST))
     {
        WRN("Not all post-event callbacks have been processed!");
        _evas_post_event_callback_call_real(eo_e, e, 0);
     }
}

void
_evas_post_event_callback_free(Evas *eo_e)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Post_Callback *pc;

   if (EINA_LIKELY(!e->post_events)) return;

   EINA_LIST_FREE(e->post_events, pc)
     {
        EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
}

void
evas_object_event_callback_all_del(Evas_Object *eo_obj)
{
   Evas_Event_Cb_Wrapper_Info *info;
   Eina_Inlist *itr;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (!obj) return;
   if (!obj->callbacks) return;
   EINA_INLIST_FOREACH_SAFE(obj->callbacks, itr, info)
     {
        efl_event_callback_del(eo_obj, _legacy_evas_callback_table(info->type), _eo_evas_object_cb, info);

        obj->callbacks =
           eina_inlist_remove(obj->callbacks, EINA_INLIST_GET(info));
        free(info);
     }
}

void
evas_object_event_callback_cleanup(Evas_Object *eo_obj)
{
   evas_object_event_callback_all_del(eo_obj);
}

void
evas_event_callback_all_del(Evas *eo_e)
{
   Evas_Event_Cb_Wrapper_Info *info;
   Eina_Inlist *itr;
   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (!e) return;
   if (!e->callbacks) return;

   EINA_INLIST_FOREACH_SAFE(e->callbacks, itr, info)
     {
        efl_event_callback_del(eo_e, _legacy_evas_callback_table(info->type), _eo_evas_cb, info);

        e->callbacks =
           eina_inlist_remove(e->callbacks, EINA_INLIST_GET(info));
        free(info);
     }
}

void
evas_event_callback_cleanup(Evas *eo_e)
{
   evas_event_callback_all_del(eo_e);
}

void
evas_event_callback_call(Evas *eo_e, Evas_Callback_Type type, void *event_info)
{
   efl_event_callback_legacy_call(eo_e, _legacy_evas_callback_table(type), event_info);
}

void
evas_object_event_callback_call(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj,
                                Evas_Callback_Type type, void *event_info, int event_id,
                                const Efl_Event_Description *efl_event_desc)
{
   /* MEM OK */
   const Evas_Button_Flags CLICK_MASK = EVAS_BUTTON_DOUBLE_CLICK | EVAS_BUTTON_TRIPLE_CLICK;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;
   Evas_Callback_Type prev_type;
   Evas_Public_Data *e;

   if (!obj) return;
   if ((obj->delete_me) || (!obj->layer)) return;
   if ((obj->last_event_id == event_id) &&
       (obj->last_event_type == type)) return;
   if (obj->last_event_id > event_id)
     {
        if ((obj->last_event_type == EVAS_CALLBACK_MOUSE_OUT) &&
            ((type >= EVAS_CALLBACK_MOUSE_DOWN) &&
             (type <= EVAS_CALLBACK_MULTI_MOVE)))
          {
             return;
          }
     }
   obj->last_event_id = event_id;
   obj->last_event_type = type;
   if (!(e = obj->layer->evas)) return;

   _evas_walk(e);

   // gesture hook
   if ( type == EVAS_CALLBACK_MOUSE_MOVE ||
        type == EVAS_CALLBACK_MULTI_MOVE ||
        type == EVAS_CALLBACK_MOUSE_DOWN ||
        type == EVAS_CALLBACK_MULTI_DOWN ||
        type == EVAS_CALLBACK_MOUSE_UP ||
        type == EVAS_CALLBACK_MULTI_UP)
     _efl_canvas_gesture_manager_filter_event(e->gesture_manager, eo_obj, event_info);

   if (!_evas_object_callback_has_by_type(obj, type))
     goto nothing_here;

   if ((type == EVAS_CALLBACK_MOUSE_DOWN) || (type == EVAS_CALLBACK_MOUSE_UP))
     {
        flags = efl_input_pointer_button_flags_get(event_info);
        if (flags & CLICK_MASK)
          {
             if (obj->last_mouse_down_counter < (e->last_mouse_down_counter - 1))
               efl_input_pointer_button_flags_set(event_info, flags & ~CLICK_MASK);
          }
        obj->last_mouse_down_counter = e->last_mouse_down_counter;
     }

   if (!efl_event_desc)
     {
        /* This can happen for DEL and FREE which are defined only in here */
        efl_event_desc = _legacy_evas_callback_table(type);
     }

   prev_type = e->current_event;
   e->current_event = type;

   efl_event_callback_legacy_call(eo_obj, efl_event_desc, event_info);

   /* multi events with finger 0 - only for eo callbacks */
   if (type == EVAS_CALLBACK_MOUSE_DOWN)
     {
        if (_evas_object_callback_has_by_type(obj, EVAS_CALLBACK_MULTI_DOWN))
          {
             e->current_event = EVAS_CALLBACK_MULTI_DOWN;
             efl_event_callback_call(eo_obj, EFL_EVENT_FINGER_DOWN, event_info);
          }
        efl_input_pointer_button_flags_set(event_info, flags);
     }
   else if (type == EVAS_CALLBACK_MOUSE_UP)
     {
        if (_evas_object_callback_has_by_type(obj, EVAS_CALLBACK_MULTI_UP))
          {
             e->current_event = EVAS_CALLBACK_MULTI_UP;
             efl_event_callback_call(eo_obj, EFL_EVENT_FINGER_UP, event_info);
          }
        efl_input_pointer_button_flags_set(event_info, flags);
     }
   else if (type == EVAS_CALLBACK_MOUSE_MOVE)
     {
        if (_evas_object_callback_has_by_type(obj, EVAS_CALLBACK_MULTI_MOVE))
          {
             e->current_event = EVAS_CALLBACK_MULTI_MOVE;
             efl_event_callback_call(eo_obj, EFL_EVENT_FINGER_MOVE, event_info);
          }
     }

   e->current_event = prev_type;

nothing_here:
   if (!obj->no_propagate)
     {
        if ((obj->smart.parent || ((obj->events) && obj->events->parent)) &&
            (type != EVAS_CALLBACK_FREE) &&
            (type <= EVAS_CALLBACK_KEY_UP))
          {
             Evas_Object_Protected_Data *parent_obj;
             Eo *parent;

             parent = ((obj->events) && obj->events->parent) ?
               obj->events->parent: obj->smart.parent;
             parent_obj = efl_data_scope_get(parent, EFL_CANVAS_OBJECT_CLASS);
             evas_object_event_callback_call(parent, parent_obj, type, event_info, event_id, efl_event_desc);
          }
     }
   _evas_unwalk(e);
}

EAPI void
evas_object_event_callback_add(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data)
{
   evas_object_event_callback_priority_add(eo_obj, type,
                                           EVAS_CALLBACK_PRIORITY_DEFAULT, func, data);
}

EAPI void
evas_object_event_callback_priority_add(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Object_Event_Cb func, const void *data)
{
   Evas_Object_Protected_Data *obj;
   Evas_Event_Cb_Wrapper_Info *cb_info;
   const Efl_Event_Description *desc;

   EINA_SAFETY_ON_NULL_RETURN(eo_obj);
   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_SAFETY_ON_TRUE_RETURN(efl_invalidated_get(eo_obj));

   obj = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   cb_info = calloc(1, sizeof(*cb_info));
   cb_info->func.object_cb = func;
   cb_info->data = (void *)data;
   cb_info->type = type;
   cb_info->efl_event_type = _evas_event_efl_event_info_type(type);

   desc = _legacy_evas_callback_table(type);
   efl_event_callback_priority_add(eo_obj, desc, priority, _eo_evas_object_cb, cb_info);

   obj->callbacks =
      eina_inlist_append(obj->callbacks, EINA_INLIST_GET(cb_info));
}

EAPI void *
evas_object_event_callback_del(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Object_Event_Cb func)
{
   Evas_Object_Protected_Data *obj;
   Evas_Event_Cb_Wrapper_Info *info;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   obj = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   if (!obj->callbacks) return NULL;

   EINA_INLIST_REVERSE_FOREACH(obj->callbacks, info)
     {
        if ((info->func.object_cb == func) && (info->type == type))
          {
             void *tmp = info->data;
             efl_event_callback_del(eo_obj, _legacy_evas_callback_table(type), _eo_evas_object_cb, info);

             obj->callbacks =
                eina_inlist_remove(obj->callbacks, EINA_INLIST_GET(info));
             free(info);
             return tmp;
          }
     }
   return NULL;
}

EAPI void *
evas_object_event_callback_del_full(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data)
{
   Evas_Object_Protected_Data *obj;
   Evas_Event_Cb_Wrapper_Info *info;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   obj = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   if (!obj->callbacks) return NULL;

   EINA_INLIST_FOREACH(obj->callbacks, info)
     {
        if ((info->func.object_cb == func) && (info->type == type) && info->data == data)
          {
             void *tmp = info->data;
             efl_event_callback_del(eo_obj, _legacy_evas_callback_table(type), _eo_evas_object_cb, info);

             obj->callbacks =
                eina_inlist_remove(obj->callbacks, EINA_INLIST_GET(info));
             free(info);
             return tmp;
          }
     }
   return NULL;
}

EAPI void
evas_event_callback_add(Evas *eo_e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data)
{
   evas_event_callback_priority_add(eo_e, type, EVAS_CALLBACK_PRIORITY_DEFAULT,
                                    func, data);
}

EAPI void
evas_event_callback_priority_add(Evas *eo_e, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Event_Cb func, const void *data)
{
   Evas_Public_Data *e;
   Evas_Event_Cb_Wrapper_Info *cb_info;
   const Efl_Event_Description *desc;

   EINA_SAFETY_ON_NULL_RETURN(eo_e);
   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_SAFETY_ON_TRUE_RETURN(efl_invalidated_get(eo_e));

   e = efl_data_scope_safe_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(e);

   cb_info = calloc(1, sizeof(*cb_info));
   cb_info->func.evas_cb = func;
   cb_info->data = (void *)data;
   cb_info->type = type;
   cb_info->efl_event_type = _evas_event_efl_event_info_type(type);

   desc = _legacy_evas_callback_table(type);
   efl_event_callback_priority_add(eo_e, desc, priority, _eo_evas_cb, cb_info);

   e->callbacks = eina_inlist_append(e->callbacks, EINA_INLIST_GET(cb_info));
}

EAPI void *
evas_event_callback_del(Evas *eo_e, Evas_Callback_Type type, Evas_Event_Cb func)
{
   Evas_Public_Data *e;
   Evas_Event_Cb_Wrapper_Info *info;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_e, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   e = efl_data_scope_safe_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   if (!e->callbacks) return NULL;

   EINA_INLIST_REVERSE_FOREACH(e->callbacks, info)
     {
        if ((info->func.evas_cb == func) && (info->type == type))
          {
             void *tmp = info->data;
             efl_event_callback_del(eo_e, _legacy_evas_callback_table(type), _eo_evas_cb, info);

             e->callbacks =
                eina_inlist_remove(e->callbacks, EINA_INLIST_GET(info));
             free(info);
             return tmp;
          }
     }
   return NULL;
}

EAPI void *
evas_event_callback_del_full(Evas *eo_e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data)
{
   Evas_Public_Data *e;
   Evas_Event_Cb_Wrapper_Info *info;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_e, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   e = efl_data_scope_safe_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   if (!e->callbacks) return NULL;

   EINA_INLIST_FOREACH(e->callbacks, info)
     {
        if ((info->func.evas_cb == func) && (info->type == type) && (info->data == data))
          {
             void *tmp = info->data;
             efl_event_callback_del(eo_e, _legacy_evas_callback_table(type), _eo_evas_cb, info);

             e->callbacks =
                eina_inlist_remove(e->callbacks, EINA_INLIST_GET(info));
             free(info);
             return tmp;
          }
     }
   return NULL;
}

EAPI void
evas_post_event_callback_push(Evas *eo_e, Evas_Object_Event_Post_Cb func, const void *data)
{
   Evas_Public_Data *e;
   Evas_Post_Callback *pc;

   EINA_SAFETY_ON_NULL_RETURN(eo_e);
   EINA_SAFETY_ON_TRUE_RETURN(efl_invalidated_get(eo_e));

   e = efl_data_scope_safe_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(e);
   if (e->delete_me) return;
   if (e->current_event == EVAS_CALLBACK_LAST)
     {
        ERR("%s() can only be called from an input event callback!", __FUNCTION__);
        return;
     }
   EVAS_MEMPOOL_INIT(_mp_pc, "evas_post_callback", Evas_Post_Callback, 64, );
   pc = EVAS_MEMPOOL_ALLOC(_mp_pc, Evas_Post_Callback);
   if (!pc) return;
   EVAS_MEMPOOL_PREP(_mp_pc, pc, Evas_Post_Callback);

   pc->func = func;
   pc->data = data;
   pc->type = e->current_event;
   pc->event_id = _evas_event_counter;
   e->post_events = eina_list_prepend(e->post_events, pc);
}

EAPI void
evas_post_event_callback_remove(Evas *eo_e, Evas_Object_Event_Post_Cb func)
{
   Evas_Public_Data *e;
   Evas_Post_Callback *pc;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(eo_e);

   e = efl_data_scope_safe_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_LIST_FOREACH(e->post_events, l, pc)
     {
        if (pc->func == func)
          {
             pc->delete_me = 1;
             return;
          }
     }
}

EAPI void
evas_post_event_callback_remove_full(Evas *eo_e, Evas_Object_Event_Post_Cb func, const void *data)
{
   Evas_Public_Data *e;
   Evas_Post_Callback *pc;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(eo_e);

   e = efl_data_scope_safe_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_LIST_FOREACH(e->post_events, l, pc)
     {
        if ((pc->func == func) && (pc->data == data))
          {
             pc->delete_me = 1;
             return;
          }
     }
}

static void
_animator_repeater(void *data, const Efl_Event *event)
{
   Evas_Object_Protected_Data *obj = data;

   efl_event_callback_legacy_call(obj->object, EFL_EVENT_ANIMATOR_TICK, event->info);
   DBG("Emitting animator tick on %p.", obj->object);
}

static void
_check_event_catcher_add(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   Evas_Object_Protected_Data *obj = data;
   Evas_Callback_Type type = EVAS_CALLBACK_LAST;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (obj->layer->evas->gesture_manager)
          _efl_canvas_gesture_manager_callback_add_hook(obj->layer->evas->gesture_manager, obj->object, array[i].desc);

        if (array[i].desc == EFL_EVENT_ANIMATOR_TICK)
          {
             if (obj->animator_ref++ > 0) break;

             efl_event_callback_add(obj->layer->evas->evas, EFL_EVENT_ANIMATOR_TICK, _animator_repeater, obj);
             DBG("Registering an animator tick on canvas %p for object %p.",
                 obj->layer->evas->evas, obj->object);
          }
        else if ((type = _legacy_evas_callback_type(array[i].desc)) != EVAS_CALLBACK_LAST)
          {
             obj->callback_mask |= (((uint64_t)1) << type);
          }
     }
}

static void
_check_event_catcher_del(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   Evas_Object_Protected_Data *obj = data;
   int i;

   if (!obj->layer ||
       !obj->layer->evas)
     return ;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (obj->layer->evas->gesture_manager)
          _efl_canvas_gesture_manager_callback_del_hook(obj->layer->evas->gesture_manager, obj->object, array[i].desc);

        if (array[i].desc == EFL_EVENT_ANIMATOR_TICK)
          {
             if ((--obj->animator_ref) > 0) break;

             efl_event_callback_del(obj->layer->evas->evas, EFL_EVENT_ANIMATOR_TICK, _animator_repeater, obj);
             DBG("Unregistering an animator tick on canvas %p for object %p.",
                 obj->layer->evas->evas, obj->object);
          }
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(event_catcher_watch,
                          { EFL_EVENT_CALLBACK_ADD, _check_event_catcher_add },
                          { EFL_EVENT_CALLBACK_DEL, _check_event_catcher_del });

void
evas_object_callback_init(Efl_Canvas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   efl_event_callback_array_add(eo_obj, event_catcher_watch(), obj);
}

void
evas_object_callback_shutdown(Efl_Canvas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   efl_event_callback_array_del(eo_obj, event_catcher_watch(), obj);
}
