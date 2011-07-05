#include "evas_common.h"
#include "evas_private.h"

static void evas_object_event_callback_clear(Evas_Object *obj);
static void evas_event_callback_clear(Evas *e);
int _evas_event_counter = 0;

EVAS_MEMPOOL(_mp_fn);
EVAS_MEMPOOL(_mp_cb);
EVAS_MEMPOOL(_mp_pc);

void
_evas_post_event_callback_call(Evas *e)
{
   Evas_Post_Callback *pc;
   int skip = 0;

   if (e->delete_me) return;
   _evas_walk(e);
   EINA_LIST_FREE(e->post_events, pc)
     {
        if ((!skip) && (!e->delete_me) && (!pc->delete_me))
          {
             if (!pc->func((void*)pc->data, e)) skip = 1;
          }
       EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
   _evas_unwalk(e);
}

void
_evas_post_event_callback_free(Evas *e)
{
   Evas_Post_Callback *pc;
   
   EINA_LIST_FREE(e->post_events, pc)
     {
       EVAS_MEMPOOL_FREE(_mp_pc, pc);
     }
   _evas_unwalk(e);
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
evas_object_event_callback_clear(Evas_Object *obj)
{
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
evas_event_callback_clear(Evas *e)
{
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
evas_object_event_callback_all_del(Evas_Object *obj)
{
   Evas_Func_Node *fn;

   if (!obj->callbacks) return;
   EINA_INLIST_FOREACH(obj->callbacks->callbacks, fn)
     fn->delete_me = 1;
}

void
evas_object_event_callback_cleanup(Evas_Object *obj)
{
   /* MEM OK */
   if (!obj->callbacks) return;
   evas_event_callback_list_post_free(&obj->callbacks->callbacks);
   EVAS_MEMPOOL_FREE(_mp_cb, obj->callbacks);
   obj->callbacks = NULL;
}

void
evas_event_callback_all_del(Evas *e)
{
   Evas_Func_Node *fn;

   if (!e->callbacks) return;
   EINA_INLIST_FOREACH(e->callbacks->callbacks, fn)
     fn->delete_me = 1;
}

void
evas_event_callback_cleanup(Evas *e)
{
   /* MEM OK */
   if (!e->callbacks) return;
   evas_event_callback_list_post_free(&e->callbacks->callbacks);
   EVAS_MEMPOOL_FREE(_mp_cb, e->callbacks);
   e->callbacks = NULL;
}

void
evas_event_callback_call(Evas *e, Evas_Callback_Type type, void *event_info)
{
   Eina_Inlist **l_mod = NULL, *l;

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
	            func(fn->data, e, event_info);
	       }
	     if (e->delete_me) break;
          }
        e->callbacks->walking_list--;
        if (!e->callbacks->walking_list)
          {
	     evas_event_callback_clear(e);
             l_mod = NULL;
          }
     }
   _evas_unwalk(e);
}

void
evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info)
{
   /* MEM OK */
   Eina_Inlist **l_mod = NULL, *l;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;
   Evas *e;

   if ((obj->delete_me) || (!obj->layer)) return;
   if ((obj->last_event == _evas_event_counter) &&
       (obj->last_event_type == type)) return;
   obj->last_event = _evas_event_counter;
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
	            func(fn->data, obj->layer->evas, obj, event_info);
	       }
	     if (obj->delete_me) break;
          }
        obj->callbacks->walking_list--;
        if (!obj->callbacks->walking_list)
	  {
	     evas_object_event_callback_clear(obj);
	     l_mod = NULL;
	  }

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
	       evas_object_event_callback_call(obj->smart.parent, type, event_info);
	  }
     }
   _evas_unwalk(e);
}



EAPI void
evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!func) return;

   if (!obj->callbacks)
     {
        EVAS_MEMPOOL_INIT(_mp_cb, "evas_callbacks", Evas_Callbacks, 512, );
        obj->callbacks = EVAS_MEMPOOL_ALLOC(_mp_cb, Evas_Callbacks);
        if (!obj->callbacks) return;
        EVAS_MEMPOOL_PREP(_mp_cb, obj->callbacks, Evas_Callbacks);
     }
  
   EVAS_MEMPOOL_INIT(_mp_fn, "evas_func_node", Evas_Func_Node, 2048, );
   fn = EVAS_MEMPOOL_ALLOC(_mp_fn, Evas_Func_Node);
   if (!fn) return;
   EVAS_MEMPOOL_PREP(_mp_fn, fn, Evas_Func_Node);
   fn->func = func;
   fn->data = (void *)data;
   fn->type = type;

   obj->callbacks->callbacks =
     eina_inlist_prepend(obj->callbacks->callbacks, EINA_INLIST_GET(fn));
}

EAPI void *
evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

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
	       evas_object_event_callback_clear(obj);
	     return tmp;
	  }
     }
   return NULL;
}

EAPI void *
evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

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
	       evas_object_event_callback_clear(obj);
	     return tmp;
	  }
     }
   return NULL;
}



EAPI void
evas_event_callback_add(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!func) return;

   if (!e->callbacks)
     {
        EVAS_MEMPOOL_INIT(_mp_cb, "evas_callbacks", Evas_Callbacks, 512, );
        e->callbacks = EVAS_MEMPOOL_ALLOC(_mp_cb, Evas_Callbacks);
        if (!e->callbacks) return;
        EVAS_MEMPOOL_PREP(_mp_cb, e->callbacks, Evas_Callbacks);
     }
  
   EVAS_MEMPOOL_INIT(_mp_fn, "evas_func_node", Evas_Func_Node, 2048, );
   fn = EVAS_MEMPOOL_ALLOC(_mp_fn, Evas_Func_Node);
   if (!fn) return;
   EVAS_MEMPOOL_PREP(_mp_fn, fn, Evas_Func_Node);
   fn->func = func;
   fn->data = (void *)data;
   fn->type = type;

   e->callbacks->callbacks =
     eina_inlist_prepend(e->callbacks->callbacks, EINA_INLIST_GET(fn));
}

EAPI void *
evas_event_callback_del(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

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
	       evas_event_callback_clear(e);
	     return data;
	  }
     }
   return NULL;
}

EAPI void *
evas_event_callback_del_full(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

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
	       evas_event_callback_clear(e);
	     return tmp;
	  }
     }
   return NULL;
}

EAPI void
evas_post_event_callback_push(Evas *e, Evas_Object_Event_Post_Cb func, const void *data)
{
   Evas_Post_Callback *pc;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
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
evas_post_event_callback_remove(Evas *e, Evas_Object_Event_Post_Cb func)
{
   Evas_Post_Callback *pc;
   Eina_List *l;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
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
evas_post_event_callback_remove_full(Evas *e, Evas_Object_Event_Post_Cb func, const void *data)
{
   Evas_Post_Callback *pc;
   Eina_List *l;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   EINA_LIST_FOREACH(e->post_events, l, pc)
     {
        if ((pc->func == func) && (pc->data == data))
          {
             pc->delete_me = 1;
             return;
          }
     }
}
