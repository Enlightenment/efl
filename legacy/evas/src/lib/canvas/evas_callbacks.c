#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info)
{
   Evas_Object_List **l_mod, *l;
   
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
	l_mod = &(obj->callbacks.in);
	break;
      case EVAS_CALLBACK_MOUSE_OUT:
	l_mod = &(obj->callbacks.out);
	break;
      case EVAS_CALLBACK_MOUSE_DOWN:
	l_mod = &(obj->callbacks.down);
	break;
      case EVAS_CALLBACK_MOUSE_UP:
	l_mod = &(obj->callbacks.up);
	break;
      case EVAS_CALLBACK_MOUSE_MOVE:
	l_mod = &(obj->callbacks.move);
	break;
      case EVAS_CALLBACK_FREE:
	l_mod = &(obj->callbacks.free);
	break;
      default:
	return;
	break;
     }
   for (l = *l_mod; l; l = l->next)
     {
	Evas_Func_Node *fn;
	
	fn = (Evas_Func_Node *)l;
	fn->func(fn->data, obj->layer->evas, obj, event_info);
     }
}

/* public functions */

void
evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), void *data)
{
   Evas_Func_Node *fn;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   
   if (!func) return;
   if (obj->smart.smart) return;
   
   fn = calloc(1, sizeof(Evas_Func_Node));
   if (!fn) return;
   fn->func = func;
   fn->data = data;
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
	obj->callbacks.in = evas_object_list_prepend(obj->callbacks.in, fn);
	break;
      case EVAS_CALLBACK_MOUSE_OUT:
	obj->callbacks.out = evas_object_list_prepend(obj->callbacks.out, fn);
	break;
      case EVAS_CALLBACK_MOUSE_DOWN:
	obj->callbacks.down = evas_object_list_prepend(obj->callbacks.down, fn);
	break;
      case EVAS_CALLBACK_MOUSE_UP:
	obj->callbacks.up = evas_object_list_prepend(obj->callbacks.up, fn);
	break;
      case EVAS_CALLBACK_MOUSE_MOVE:
	obj->callbacks.move = evas_object_list_prepend(obj->callbacks.move, fn);
	break;
      case EVAS_CALLBACK_FREE:
	obj->callbacks.free = evas_object_list_prepend(obj->callbacks.free, fn);
	break;
      default:
	return;
	break;
     }
}

void *
evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info))
{
   Evas_Object_List **l_mod, *l;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   
   if (!func) return NULL;
   if (obj->smart.smart) return NULL;
   
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
	l_mod = &(obj->callbacks.in);
	break;
      case EVAS_CALLBACK_MOUSE_OUT:
	l_mod = &(obj->callbacks.out);
	break;
      case EVAS_CALLBACK_MOUSE_DOWN:
	l_mod = &(obj->callbacks.down);
	break;
      case EVAS_CALLBACK_MOUSE_UP:
	l_mod = &(obj->callbacks.up);
	break;
      case EVAS_CALLBACK_MOUSE_MOVE:
	l_mod = &(obj->callbacks.move);
	break;
      case EVAS_CALLBACK_FREE:
	l_mod = &(obj->callbacks.free);
	break;
      default:
	return NULL;
	break;
     }
   for (l = *l_mod; l; l = l->next)
     {
	Evas_Func_Node *fn;
	
	fn = (Evas_Func_Node *)l;	
	if (fn->func == func)
	  {
	     void *data;
	     
	     data = fn->data;
	     *l_mod = evas_object_list_remove(*l_mod, fn);
	     free(fn);
	     return data;
	  }
     }
   return NULL;
}
