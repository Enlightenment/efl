#include "evas_common_private.h"
#include "evas_private.h"

int _evas_event_counter = 0;

EVAS_MEMPOOL(_mp_pc);

extern Eina_Hash* signals_hash_table;

EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_IN =
   EO_EVENT_DESCRIPTION("Mouse In", "Mouse In Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_OUT =
   EO_EVENT_DESCRIPTION("Mouse Out", "Mouse Out Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_DOWN =
   EO_EVENT_DESCRIPTION("Mouse Down", "Mouse Button Down Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_UP =
   EO_EVENT_DESCRIPTION("Mouse Up", "Mouse Button Up Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_MOVE =
   EO_EVENT_DESCRIPTION("Mouse Move", "Mouse Move Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOUSE_WHEEL =
   EO_EVENT_DESCRIPTION("Mouse Wheel", "Mouse Wheel Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MULTI_DOWN =
   EO_EVENT_DESCRIPTION("Multi Down", "Mouse-touch Down Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MULTI_UP =
   EO_EVENT_DESCRIPTION("Multi Up", "Mouse-touch Up Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MULTI_MOVE =
   EO_EVENT_DESCRIPTION("Multi Move", "Multi-touch Move Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_FREE =
   EO_EVENT_DESCRIPTION("Free", "Object Being Freed (Called after Del)");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_KEY_DOWN =
   EO_EVENT_DESCRIPTION("Key Down", "Key Press Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_KEY_UP =
   EO_EVENT_DESCRIPTION("Key Up", "Key Release Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_FOCUS_IN =
   EO_EVENT_DESCRIPTION("Focus In", "Focus In Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_FOCUS_OUT =
   EO_EVENT_DESCRIPTION("Focus Out", "Focus Out Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_SHOW =
   EO_EVENT_DESCRIPTION("Show", "Show Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_HIDE =
   EO_EVENT_DESCRIPTION("Hide", "Hide Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_MOVE =
   EO_EVENT_DESCRIPTION("Move", "Move Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_RESIZE =
   EO_EVENT_DESCRIPTION("Resize", "Resize Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_RESTACK =
   EO_EVENT_DESCRIPTION("Restack", "Restack Event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_DEL =
   EO_EVENT_DESCRIPTION("Del", "Object Being Deleted (called before Free)");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_HOLD =
   EO_EVENT_DESCRIPTION("Hold", "Events go on/off hold");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_CHANGED_SIZE_HINTS =
   EO_EVENT_DESCRIPTION("Changed Size Hints", "Size hints changed event");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_IMAGE_PRELOADED =
   EO_EVENT_DESCRIPTION("Image Preloaded", "Image has been preloaded");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_IMAGE_RESIZE=
   EO_EVENT_DESCRIPTION("Image Resize", "Image resize");
EAPI const Eo_Event_Description _EVAS_OBJECT_EVENT_IMAGE_UNLOADED =
   EO_EVENT_DESCRIPTION("Image Unloaded", "Image data has been unloaded (by some mechanism in Evas that throw out original image data)");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_PRE =
   EO_EVENT_DESCRIPTION("Render Pre", "Called just before rendering starts on the canvas target @since 1.2");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_POST =
   EO_EVENT_DESCRIPTION("Render Post", "Called just after rendering stops on the canvas target @since 1.2");

EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_FOCUS_IN =
   EO_HOT_EVENT_DESCRIPTION("Canvas Focus In", "Canvas got focus as a whole");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_FOCUS_OUT =
   EO_HOT_EVENT_DESCRIPTION("Canvas Focus Out", "Canvas lost focus as a whole");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE =
   EO_HOT_EVENT_DESCRIPTION("Render Flush Pre", "Called just before rendering is updated on the canvas target");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_POST =
   EO_HOT_EVENT_DESCRIPTION("Render Flush Post", "Called just after rendering is updated on the canvas target");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN =
   EO_HOT_EVENT_DESCRIPTION("Canvas Object Focus In", "Canvas object got focus");
EAPI const Eo_Event_Description _EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT =
   EO_HOT_EVENT_DESCRIPTION("Canvas Object Focus Out", "Canvas object lost focus");

/**
 * Evas events descriptions for Eo.
 */
static const Eo_Event_Description *_legacy_evas_callback_table[EVAS_CALLBACK_LAST] =
{
   EVAS_OBJECT_EVENT_MOUSE_IN,
   EVAS_OBJECT_EVENT_MOUSE_OUT,
   EVAS_OBJECT_EVENT_MOUSE_DOWN,
   EVAS_OBJECT_EVENT_MOUSE_UP,
   EVAS_OBJECT_EVENT_MOUSE_MOVE,
   EVAS_OBJECT_EVENT_MOUSE_WHEEL,
   EVAS_OBJECT_EVENT_MULTI_DOWN,
   EVAS_OBJECT_EVENT_MULTI_UP,
   EVAS_OBJECT_EVENT_MULTI_MOVE,
   EVAS_OBJECT_EVENT_FREE,
   EVAS_OBJECT_EVENT_KEY_DOWN,
   EVAS_OBJECT_EVENT_KEY_UP,
   EVAS_OBJECT_EVENT_FOCUS_IN,
   EVAS_OBJECT_EVENT_FOCUS_OUT,
   EVAS_OBJECT_EVENT_SHOW,
   EVAS_OBJECT_EVENT_HIDE,
   EVAS_OBJECT_EVENT_MOVE,
   EVAS_OBJECT_EVENT_RESIZE,
   EVAS_OBJECT_EVENT_RESTACK,
   EVAS_OBJECT_EVENT_DEL,
   EVAS_OBJECT_EVENT_HOLD,
   EVAS_OBJECT_EVENT_CHANGED_SIZE_HINTS,
   EVAS_OBJECT_EVENT_IMAGE_PRELOADED,
   EVAS_CANVAS_EVENT_FOCUS_IN,
   EVAS_CANVAS_EVENT_FOCUS_OUT,
   EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE,
   EVAS_CANVAS_EVENT_RENDER_FLUSH_POST,
   EVAS_CANVAS_EVENT_OBJECT_FOCUS_IN,
   EVAS_CANVAS_EVENT_OBJECT_FOCUS_OUT,
   EVAS_OBJECT_EVENT_IMAGE_UNLOADED,
   EVAS_CANVAS_EVENT_RENDER_PRE,
   EVAS_CANVAS_EVENT_RENDER_POST,
   EVAS_OBJECT_EVENT_IMAGE_RESIZE,
   NULL
};

typedef struct
{
   EINA_INLIST;
   Evas_Object_Event_Cb func;
   void *data;
   Evas_Callback_Type type;
} _eo_evas_object_cb_info;

static Eina_Bool
_eo_evas_object_cb(void *data, Eo *eo_obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   _eo_evas_object_cb_info *info = data;
   if (info->func) info->func(info->data, evas_object_evas_get(eo_obj), eo_obj, event_info);
   return EINA_TRUE;
}

typedef struct
{
   EINA_INLIST;
   Evas_Event_Cb func;
   void *data;
   Evas_Callback_Type type;
} _eo_evas_cb_info;

static Eina_Bool
_eo_evas_cb(void *data, Eo *eo_obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   _eo_evas_cb_info *info = data;
   if (info->func) info->func(info->data, eo_obj, event_info);
   return EINA_TRUE;
}

void
_evas_post_event_callback_call(Evas *eo_e, Evas_Public_Data *e)
{
   Evas_Post_Callback *pc;
   int skip = 0;
   static int first_run = 1; // FIXME: This is a workaround to prevent this
                             // function from being called recursively.

   if (e->delete_me || (!first_run)) return;
   _evas_walk(e);
   first_run = 0;
   EINA_LIST_FREE(e->post_events, pc)
     {
        if ((!skip) && (!e->delete_me) && (!pc->delete_me))
          {
             if (!pc->func((void*)pc->data, eo_e)) skip = 1;
          }
        EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
   first_run = 1;
   _evas_unwalk(e);
}

void
_evas_post_event_callback_free(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Post_Callback *pc;

   EINA_LIST_FREE(e->post_events, pc)
     {
        EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
}

void
evas_object_event_callback_all_del(Evas_Object *eo_obj)
{
   _eo_evas_object_cb_info *info;
   Eina_Inlist *itr;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj) return;
   if (!obj->callbacks) return;
   EINA_INLIST_FOREACH_SAFE(obj->callbacks, itr, info)
     {
        eo_do(eo_obj, eo_event_callback_del(
                 _legacy_evas_callback_table[info->type], _eo_evas_object_cb, info));

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
   _eo_evas_object_cb_info *info;
   Eina_Inlist *itr;
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);

   if (!e) return;
   if (!e->callbacks) return;

   EINA_INLIST_FOREACH_SAFE(e->callbacks, itr, info)
     {
        eo_do(eo_e, eo_event_callback_del(
                 _legacy_evas_callback_table[info->type], _eo_evas_cb, info));

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
   eo_do(eo_e, eo_event_callback_call(_legacy_evas_callback_table[type], event_info));
}

void
evas_object_event_callback_call(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Callback_Type type, void *event_info, int event_id)
{
   /* MEM OK */
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;
   Evas_Public_Data *e;

   if (!obj) return;
   if ((obj->delete_me) || (!obj->layer)) return;
   if ((obj->last_event == event_id) &&
       (obj->last_event_type == type)) return;
   if (obj->last_event > event_id)
     {
        if ((obj->last_event_type == EVAS_CALLBACK_MOUSE_OUT) &&
            ((type >= EVAS_CALLBACK_MOUSE_DOWN) &&
             (type <= EVAS_CALLBACK_MULTI_MOVE)))
          {
             return;
          }
     }
   obj->last_event = event_id;
   obj->last_event_type = type;
   if (!(e = obj->layer->evas)) return;

   _evas_walk(e);
     {
        switch (type)
          {
           case EVAS_CALLBACK_MOUSE_DOWN:
                {
                   Evas_Event_Mouse_Down *ev = event_info;

                   flags = ev->flags;
                   if (ev->flags & (EVAS_BUTTON_DOUBLE_CLICK | EVAS_BUTTON_TRIPLE_CLICK))
                     {
                        if (obj->last_mouse_down_counter < (e->last_mouse_down_counter - 1))
                          ev->flags &= ~(EVAS_BUTTON_DOUBLE_CLICK | EVAS_BUTTON_TRIPLE_CLICK);
                     }
                   obj->last_mouse_down_counter = e->last_mouse_down_counter;
                   break;
                }
           case EVAS_CALLBACK_MOUSE_UP:
                {
                   Evas_Event_Mouse_Up *ev = event_info;

                   flags = ev->flags;
                   if (ev->flags & (EVAS_BUTTON_DOUBLE_CLICK | EVAS_BUTTON_TRIPLE_CLICK))
                     {
                        if (obj->last_mouse_up_counter < (e->last_mouse_up_counter - 1))
                          ev->flags &= ~(EVAS_BUTTON_DOUBLE_CLICK | EVAS_BUTTON_TRIPLE_CLICK);
                     }
                   obj->last_mouse_up_counter = e->last_mouse_up_counter;
                   break;
                }
           default:
              break;
          }

        eo_do(eo_obj, eo_event_callback_call(_legacy_evas_callback_table[type], event_info));

        if (type == EVAS_CALLBACK_MOUSE_DOWN)
          {
             Evas_Event_Mouse_Down *ev = event_info;
             ev->flags = flags;
          }
        else if (type == EVAS_CALLBACK_MOUSE_UP)
          {
             Evas_Event_Mouse_Up *ev = event_info;
             ev->flags = flags;
          }
     }

   if (!obj->no_propagate)
     {
        if ((obj->smart.parent) && (type != EVAS_CALLBACK_FREE) &&
              (type <= EVAS_CALLBACK_KEY_UP))
          {
             Evas_Object_Protected_Data *smart_parent = eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
             evas_object_event_callback_call(obj->smart.parent, smart_parent, type, event_info, event_id);
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!func) return;

   _eo_evas_object_cb_info *cb_info = calloc(1, sizeof(*cb_info));
   cb_info->func = func;
   cb_info->data = (void *)data;
   cb_info->type = type;

   const Eo_Event_Description *desc = _legacy_evas_callback_table[type];
   eo_do(eo_obj, eo_event_callback_priority_add(desc, priority, _eo_evas_object_cb, cb_info));

   obj->callbacks =
      eina_inlist_append(obj->callbacks, EINA_INLIST_GET(cb_info));
}

EAPI void *
evas_object_event_callback_del(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Object_Event_Cb func)
{
   _eo_evas_object_cb_info *info;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj) return NULL;
   if (!func) return NULL;

   if (!obj->callbacks) return NULL;

   EINA_INLIST_FOREACH(obj->callbacks, info)
     {
        if ((info->func == func) && (info->type == type))
          {
             void *tmp = info->data;
             eo_do(eo_obj, eo_event_callback_del(
                      _legacy_evas_callback_table[type], _eo_evas_object_cb, info));

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
   _eo_evas_object_cb_info *info;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj) return NULL;
   if (!func) return NULL;

   if (!obj->callbacks) return NULL;

   EINA_INLIST_FOREACH(obj->callbacks, info)
     {
        if ((info->func == func) && (info->type == type) && info->data == data)
          {
             void *tmp = info->data;
             eo_do(eo_obj, eo_event_callback_del(
                      _legacy_evas_callback_table[type], _eo_evas_object_cb, info));

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
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!func) return;

   _eo_evas_cb_info *cb_info = calloc(1, sizeof(*cb_info));
   cb_info->func = func;
   cb_info->data = (void *)data;
   cb_info->type = type;

   const Eo_Event_Description *desc = _legacy_evas_callback_table[type];
   eo_do(eo_e, eo_event_callback_priority_add(desc, priority, _eo_evas_cb, cb_info));

   e->callbacks = eina_inlist_append(e->callbacks, EINA_INLIST_GET(cb_info));
}

EAPI void *
evas_event_callback_del(Evas *eo_e, Evas_Callback_Type type, Evas_Event_Cb func)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   _eo_evas_cb_info *info;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e) return NULL;
   if (!func) return NULL;

   if (!e->callbacks) return NULL;

   EINA_INLIST_FOREACH(e->callbacks, info)
     {
        if ((info->func == func) && (info->type == type))
          {
             void *tmp = info->data;
             eo_do(eo_e, eo_event_callback_del(
                      _legacy_evas_callback_table[type], _eo_evas_cb, info));

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
   _eo_evas_cb_info *info;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);

   if (!e) return NULL;
   if (!func) return NULL;

   if (!e->callbacks) return NULL;

   EINA_INLIST_FOREACH(e->callbacks, info)
     {
        if ((info->func == func) && (info->type == type) && (info->data == data))
          {
             void *tmp = info->data;
             eo_do(eo_e, eo_event_callback_del(
                      _legacy_evas_callback_table[type], _eo_evas_cb, info));

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
   Evas_Post_Callback *pc;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   if (!e) return;
   EVAS_MEMPOOL_INIT(_mp_pc, "evas_post_callback", Evas_Post_Callback, 64, );
   pc = EVAS_MEMPOOL_ALLOC(_mp_pc, Evas_Post_Callback);
   if (!pc) return;
   EVAS_MEMPOOL_PREP(_mp_pc, pc, Evas_Post_Callback);
   if (e->delete_me) return;

   pc->func = func;
   pc->data = data;
   e->post_events = eina_list_prepend(e->post_events, pc);
}

EAPI void
evas_post_event_callback_remove(Evas *eo_e, Evas_Object_Event_Post_Cb func)
{
   Evas_Post_Callback *pc;
   Eina_List *l;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   if (!e) return;
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
   Evas_Post_Callback *pc;
   Eina_List *l;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   if (!e) return;
   EINA_LIST_FOREACH(e->post_events, l, pc)
     {
        if ((pc->func == func) && (pc->data == data))
          {
             pc->delete_me = 1;
             return;
          }
     }
}
