#include "evas_common.h"
#include "evas_private.h"

static void evas_object_event_callback_clear(Evas_Object *eo_obj);
static void evas_event_callback_clear(Evas *eo_e);
int _evas_event_counter = 0;

EVAS_MEMPOOL(_mp_fn);
EVAS_MEMPOOL(_mp_cb);
EVAS_MEMPOOL(_mp_pc);

extern Eina_Hash* signals_hash_table;

EAPI const Eo_Event_Description _EVAS_EVENT_MOUSE_IN =
   EO_EVENT_DESCRIPTION("Mouse In", "Mouse In Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MOUSE_OUT =
   EO_EVENT_DESCRIPTION("Mouse Out", "Mouse Out Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MOUSE_DOWN =
   EO_EVENT_DESCRIPTION("Mouse Down", "Mouse Button Down Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MOUSE_UP =
   EO_EVENT_DESCRIPTION("Mouse Up", "Mouse Button Up Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MOUSE_MOVE =
   EO_EVENT_DESCRIPTION("Mouse Move", "Mouse Move Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MOUSE_WHEEL =
   EO_EVENT_DESCRIPTION("Mouse Wheel", "Mouse Wheel Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MULTI_DOWN =
   EO_EVENT_DESCRIPTION("Multi Down", "Mouse-touch Down Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MULTI_UP =
   EO_EVENT_DESCRIPTION("Multi Up", "Mouse-touch Up Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MULTI_MOVE =
   EO_EVENT_DESCRIPTION("Multi Move", "Multi-touch Move Event");
EAPI const Eo_Event_Description _EVAS_EVENT_FREE =
   EO_EVENT_DESCRIPTION("Free", "Object Being Freed (Called after Del)");
EAPI const Eo_Event_Description _EVAS_EVENT_KEY_DOWN =
   EO_EVENT_DESCRIPTION("Key Down", "Key Press Event");
EAPI const Eo_Event_Description _EVAS_EVENT_KEY_UP =
   EO_EVENT_DESCRIPTION("Key Up", "Key Release Event");
EAPI const Eo_Event_Description _EVAS_EVENT_FOCUS_IN =
   EO_EVENT_DESCRIPTION("Focus In", "Focus In Event");
EAPI const Eo_Event_Description _EVAS_EVENT_FOCUS_OUT =
   EO_EVENT_DESCRIPTION("Focus Out", "Focus Out Event");
EAPI const Eo_Event_Description _EVAS_EVENT_SHOW =
   EO_EVENT_DESCRIPTION("Show", "Show Event");
EAPI const Eo_Event_Description _EVAS_EVENT_HIDE =
   EO_EVENT_DESCRIPTION("Hide", "Hide Event");
EAPI const Eo_Event_Description _EVAS_EVENT_MOVE =
   EO_EVENT_DESCRIPTION("Move", "Move Event");
EAPI const Eo_Event_Description _EVAS_EVENT_RESIZE =
   EO_EVENT_DESCRIPTION("Resize", "Resize Event");
EAPI const Eo_Event_Description _EVAS_EVENT_RESTACK =
   EO_EVENT_DESCRIPTION("Restack", "Restack Event");
EAPI const Eo_Event_Description _EVAS_EVENT_DEL =
   EO_EVENT_DESCRIPTION("Del", "Object Being Deleted (called before Free)");
EAPI const Eo_Event_Description _EVAS_EVENT_HOLD =
   EO_EVENT_DESCRIPTION("Hold", "Events go on/off hold");
EAPI const Eo_Event_Description _EVAS_EVENT_CHANGED_SIZE_HINTS =
   EO_EVENT_DESCRIPTION("Changed Size Hints", "Size hints changed event");
EAPI const Eo_Event_Description _EVAS_EVENT_IMAGE_PRELOADED =
   EO_EVENT_DESCRIPTION("Image Preloaded", "Image has been preloaded");
EAPI const Eo_Event_Description _EVAS_EVENT_IMAGE_RESIZE=
   EO_EVENT_DESCRIPTION("Image Resize", "Image resize");
EAPI const Eo_Event_Description _EVAS_EVENT_CANVAS_FOCUS_IN =
   EO_EVENT_DESCRIPTION("Canvas Focus In", "Canvas got focus as a whole");
EAPI const Eo_Event_Description _EVAS_EVENT_CANVAS_FOCUS_OUT =
   EO_EVENT_DESCRIPTION("Canvas Focus Out", "Canvas lost focus as a whole");
EAPI const Eo_Event_Description _EVAS_EVENT_RENDER_FLUSH_PRE =
   EO_EVENT_DESCRIPTION("Render Flush Pre", "Called just before rendering is updated on the canvas target");
EAPI const Eo_Event_Description _EVAS_EVENT_RENDER_FLUSH_POST =
   EO_EVENT_DESCRIPTION("Render Flush Post", "Called just after rendering is updated on the canvas target");
EAPI const Eo_Event_Description _EVAS_EVENT_CANVAS_OBJECT_FOCUS_IN =
   EO_EVENT_DESCRIPTION("Canvas Object Focus In", "Canvas object got focus");
EAPI const Eo_Event_Description _EVAS_EVENT_CANVAS_OBJECT_FOCUS_OUT =
   EO_EVENT_DESCRIPTION("Canvas Object Focus Out", "Canvas object lost focus");
EAPI const Eo_Event_Description _EVAS_EVENT_IMAGE_UNLOADED =
   EO_EVENT_DESCRIPTION("Image Unloaded", "Image data has been unloaded (by some mechanism in Evas that throw out original image data)");
EAPI const Eo_Event_Description _EVAS_EVENT_RENDER_PRE =
   EO_EVENT_DESCRIPTION("Render Pre", "Called just before rendering starts on the canvas target @since 1.2");
EAPI const Eo_Event_Description _EVAS_EVENT_RENDER_POST =
   EO_EVENT_DESCRIPTION("Render Post", "Called just after rendering stops on the canvas target @since 1.2");

/**
 * Evas events descriptions for Eo.
 */
static const Eo_Event_Description *_legacy_evas_callback_table[EVAS_CALLBACK_LAST] =
{
   EVAS_EVENT_MOUSE_IN,
   EVAS_EVENT_MOUSE_OUT,
   EVAS_EVENT_MOUSE_DOWN,
   EVAS_EVENT_MOUSE_UP,
   EVAS_EVENT_MOUSE_MOVE,
   EVAS_EVENT_MOUSE_WHEEL,
   EVAS_EVENT_MULTI_DOWN,
   EVAS_EVENT_MULTI_UP,
   EVAS_EVENT_MULTI_MOVE,
   EVAS_EVENT_FREE,
   EVAS_EVENT_KEY_DOWN,
   EVAS_EVENT_KEY_UP,
   EVAS_EVENT_FOCUS_IN,
   EVAS_EVENT_FOCUS_OUT,
   EVAS_EVENT_SHOW,
   EVAS_EVENT_HIDE,
   EVAS_EVENT_MOVE,
   EVAS_EVENT_RESIZE,
   EVAS_EVENT_RESTACK,
   EVAS_EVENT_DEL,
   EVAS_EVENT_HOLD,
   EVAS_EVENT_CHANGED_SIZE_HINTS,
   EVAS_EVENT_IMAGE_PRELOADED,
   EVAS_EVENT_IMAGE_RESIZE,
   EVAS_EVENT_CANVAS_FOCUS_IN,
   EVAS_EVENT_CANVAS_FOCUS_OUT,
   EVAS_EVENT_RENDER_FLUSH_PRE,
   EVAS_EVENT_RENDER_FLUSH_POST,
   EVAS_EVENT_CANVAS_OBJECT_FOCUS_IN,
   EVAS_EVENT_CANVAS_OBJECT_FOCUS_OUT,
   EVAS_EVENT_IMAGE_UNLOADED,
   EVAS_EVENT_RENDER_PRE,
   EVAS_EVENT_RENDER_POST
};
 
void
_evas_post_event_callback_call(Evas *eo_e, Evas_Public_Data *e)
{
   Evas_Post_Callback *pc;
   int skip = 0;

   if (e->delete_me) return;
   _evas_walk(e);
   EINA_LIST_FREE(e->post_events, pc)
     {
        if ((!skip) && (!e->delete_me) && (!pc->delete_me))
          {
             if (!pc->func((void*)pc->data, eo_e)) skip = 1;
          }
        EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
   _evas_unwalk(e);
}

void
_evas_post_event_callback_free(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   Evas_Post_Callback *pc;

   EINA_LIST_FREE(e->post_events, pc)
     {
        EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
}

void
evas_event_callback_list_post_free(Eina_Inlist **list)
{
   Eina_Inlist *l;

   /* MEM OK */
   for (l = *list; l;)
     {
        Evas_Func_Node *fn;

        fn = (Evas_Func_Node *)l;
        l = l->next;
        if (fn->delete_me)
          {
             *list = eina_inlist_remove(*list, EINA_INLIST_GET(fn));
             EVAS_MEMPOOL_FREE(_mp_fn, fn);
          }
     }
}

static void
evas_object_event_callback_clear(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj) return;
   if (!obj->callbacks) return;
   if (!obj->callbacks->deletions_waiting) return;
   obj->callbacks->deletions_waiting = 0;
   evas_event_callback_list_post_free(&obj->callbacks->callbacks);
   if (!obj->callbacks->callbacks)
     {
        EVAS_MEMPOOL_FREE(_mp_cb, obj->callbacks);
        obj->callbacks = NULL;
     }
}

static void
evas_event_callback_clear(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   if (!e) return;
   if (!e->callbacks) return;
   if (!e->callbacks->deletions_waiting) return;
   e->callbacks->deletions_waiting = 0;
   evas_event_callback_list_post_free(&e->callbacks->callbacks);
   if (!e->callbacks->callbacks)
     {
        EVAS_MEMPOOL_FREE(_mp_cb, e->callbacks);
        e->callbacks = NULL;
     }
}

void
evas_object_event_callback_all_del(Evas_Object *eo_obj)
{
   Evas_Func_Node *fn;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj) return;
   if (!obj->callbacks) return;
   EINA_INLIST_FOREACH(obj->callbacks->callbacks, fn)
      fn->delete_me = 1;
}

void
evas_object_event_callback_cleanup(Evas_Object *eo_obj)
{
   /* MEM OK */
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   if (!obj) return;
   if (!obj->callbacks) return;
   evas_event_callback_list_post_free(&obj->callbacks->callbacks);
   EVAS_MEMPOOL_FREE(_mp_cb, obj->callbacks);
   obj->callbacks = NULL;
}

void
evas_event_callback_all_del(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   Evas_Func_Node *fn;

   if (!e) return;
   if (!e->callbacks) return;
   EINA_INLIST_FOREACH(e->callbacks->callbacks, fn)
      fn->delete_me = 1;
}

void
evas_event_callback_cleanup(Evas *eo_e)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   /* MEM OK */
   if (!e) return;
   if (!e->callbacks) return;
   evas_event_callback_list_post_free(&e->callbacks->callbacks);
   EVAS_MEMPOOL_FREE(_mp_cb, e->callbacks);
   e->callbacks = NULL;
}

void
evas_event_callback_call(Evas *eo_e, Evas_Callback_Type type, void *event_info)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   Eina_Inlist **l_mod = NULL, *l;

   if (!e) return;
   _evas_walk(e);
   if (e->callbacks)
     {
        l_mod = &e->callbacks->callbacks;
        e->callbacks->walking_list++;
        for (l = *l_mod; l; l = l->next)
          {
             Evas_Func_Node *fn;

             fn = (Evas_Func_Node *)l;
             if ((fn->type == type) && (!fn->delete_me))
               {
                  Evas_Event_Cb func = fn->func;
                  if (func)
                    func(fn->data, eo_e, event_info);
               }
             if (e->delete_me) break;
          }
        e->callbacks->walking_list--;
        if (!e->callbacks->walking_list)
          {
             evas_event_callback_clear(eo_e);
             l_mod = NULL;
          }
     }
   _evas_unwalk(e);
}

void
evas_object_event_callback_call(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Callback_Type type, void *event_info, int event_id)
{
   /* MEM OK */
   Eina_Inlist **l_mod = NULL, *l;
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
   if (obj->callbacks)
     {
        l_mod = &obj->callbacks->callbacks;
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
        obj->callbacks->walking_list++;
        for (l = *l_mod; l; l = l->next)
          {
             Evas_Func_Node *fn;

             fn = (Evas_Func_Node *)l;
             if ((fn->type == type) && (!fn->delete_me))
               {
                  Evas_Object_Event_Cb func = fn->func;
                  if (func)
                    func(fn->data, obj->layer->evas->evas, eo_obj, event_info);
               }
             if (obj->delete_me) break;
          }
        obj->callbacks->walking_list--;
        if (!obj->callbacks->walking_list)
          {
             evas_object_event_callback_clear(eo_obj);
             l_mod = NULL;
          }

        const Eo_Event_Description *event_desc = eina_hash_find(signals_hash_table, _legacy_evas_callback_table[type]->name);
        if (event_desc)
           eo_do(eo_obj, eo_event_callback_call(_legacy_evas_callback_table[type], event_info, NULL));

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

   if (!((obj->no_propagate) && (l_mod) && (*l_mod)))
     {
        if (!obj->no_propagate)
          {
             if ((obj->smart.parent) && (type != EVAS_CALLBACK_FREE) &&
                 (type <= EVAS_CALLBACK_KEY_UP))
               {
                  Evas_Object_Protected_Data *smart_parent = eo_data_get(obj->smart.parent, EVAS_OBJ_CLASS);
                  evas_object_event_callback_call(obj->smart.parent, smart_parent, type, event_info, event_id);
               }
          }
     }
   _evas_unwalk(e);
}

static int
_callback_priority_cmp(const void *_a, const void *_b)
{
   const Evas_Func_Node *a, *b;
   a = EINA_INLIST_CONTAINER_GET(_a, Evas_Func_Node);
   b = EINA_INLIST_CONTAINER_GET(_b, Evas_Func_Node);
   if (a->priority < b->priority)
     return -1;
   else
     return 1;
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
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!obj) return;
   if (!func) return;

   if (!obj->callbacks)
     {
        EVAS_MEMPOOL_INIT(_mp_cb, "evas_callbacks", Evas_Callbacks, 64, );
        obj->callbacks = EVAS_MEMPOOL_ALLOC(_mp_cb, Evas_Callbacks);
        if (!obj->callbacks) return;
        EVAS_MEMPOOL_PREP(_mp_cb, obj->callbacks, Evas_Callbacks);
     }

   EVAS_MEMPOOL_INIT(_mp_fn, "evas_func_node", Evas_Func_Node, 128, );
   fn = EVAS_MEMPOOL_ALLOC(_mp_fn, Evas_Func_Node);
   if (!fn) return;
   EVAS_MEMPOOL_PREP(_mp_fn, fn, Evas_Func_Node);
   fn->func = func;
   fn->data = (void *)data;
   fn->type = type;
   fn->priority = priority;

   obj->callbacks->callbacks =
      eina_inlist_sorted_insert(obj->callbacks->callbacks, EINA_INLIST_GET(fn),
                                _callback_priority_cmp);
}

EAPI void *
evas_object_event_callback_del(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Object_Event_Cb func)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj) return NULL;
   if (!func) return NULL;

   if (!obj->callbacks) return NULL;

   EINA_INLIST_FOREACH(obj->callbacks->callbacks, fn)
     {
        if ((fn->func == func) && (fn->type == type) && (!fn->delete_me))
          {
             void *tmp;

             tmp = fn->data;
             fn->delete_me = 1;
             obj->callbacks->deletions_waiting = 1;
             if (!obj->callbacks->walking_list)
               evas_object_event_callback_clear(eo_obj);
             return tmp;
          }
     }
   return NULL;
}

EAPI void *
evas_object_event_callback_del_full(Evas_Object *eo_obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (!obj) return NULL;
   if (!func) return NULL;

   if (!obj->callbacks) return NULL;

   EINA_INLIST_FOREACH(obj->callbacks->callbacks, fn)
     {
        if ((fn->func == func) && (fn->type == type) && (fn->data == data) && (!fn->delete_me))
          {
             void *tmp;

             tmp = fn->data;
             fn->delete_me = 1;
             obj->callbacks->deletions_waiting = 1;
             if (!obj->callbacks->walking_list)
               evas_object_event_callback_clear(eo_obj);
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
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!e) return;
   if (!func) return;

   if (!e->callbacks)
     {
        EVAS_MEMPOOL_INIT(_mp_cb, "evas_callbacks", Evas_Callbacks, 64, );
        e->callbacks = EVAS_MEMPOOL_ALLOC(_mp_cb, Evas_Callbacks);
        if (!e->callbacks) return;
        EVAS_MEMPOOL_PREP(_mp_cb, e->callbacks, Evas_Callbacks);
     }

   EVAS_MEMPOOL_INIT(_mp_fn, "evas_func_node", Evas_Func_Node, 128, );
   fn = EVAS_MEMPOOL_ALLOC(_mp_fn, Evas_Func_Node);
   if (!fn) return;
   EVAS_MEMPOOL_PREP(_mp_fn, fn, Evas_Func_Node);
   fn->func = func;
   fn->data = (void *)data;
   fn->type = type;
   fn->priority = priority;

   e->callbacks->callbacks = eina_inlist_sorted_insert(e->callbacks->callbacks,
                                                       EINA_INLIST_GET(fn), _callback_priority_cmp);
}

EAPI void *
evas_event_callback_del(Evas *eo_e, Evas_Callback_Type type, Evas_Event_Cb func)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e) return NULL;
   if (!func) return NULL;

   if (!e->callbacks) return NULL;

   EINA_INLIST_FOREACH(e->callbacks->callbacks, fn)
     {
        if ((fn->func == func) && (fn->type == type) && (!fn->delete_me))
          {
             void *data;

             data = fn->data;
             fn->delete_me = 1;
             e->callbacks->deletions_waiting = 1;
             if (!e->callbacks->walking_list)
               evas_event_callback_clear(eo_e);
             return data;
          }
     }
   return NULL;
}

EAPI void *
evas_event_callback_del_full(Evas *eo_e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);

   if (!e) return NULL;
   if (!func) return NULL;

   if (!e->callbacks) return NULL;

   EINA_INLIST_FOREACH(e->callbacks->callbacks, fn)
     {
        if ((fn->func == func) && (fn->type == type) && (fn->data == data) && (!fn->delete_me))
          {
             void *tmp;

             tmp = fn->data;
             fn->delete_me = 1;
             e->callbacks->deletions_waiting = 1;
             if (!e->callbacks->walking_list)
               evas_event_callback_clear(eo_e);
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

   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
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

   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
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

   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
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
