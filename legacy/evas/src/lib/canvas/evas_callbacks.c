#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info)
{
   /* MEM OK */
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
/**
 * Add a callback function to an object
 * @param obj Object to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param func The function to be called when the event is triggered
 * @param data The data pointer to be passed to @p func
 * 
 * This function adds a function callback to an object when the event of type
 * @p type occurs on object @p obj. The function will be passed the pointer
 * @p data when it is called. A callback function must look like this:
 * 
 * @code
 * void callback (void *data, Evas *e, Evas_Object *obj, void *event_info);
 * @endcode
 * 
 * The first parameter @p data in this function will be the same value passed
 * to evas_object_event_callback_add() as the @p data parameter. The second
 * parameter is a convenience for the programmer to know what evas canvas the
 * event occured on. The third parameter @p obj is the Object handle on which
 * the event occured. The foruth parameter @p event_info is a pointer to a
 * data structure that may or may not be passed to the callback, depending on
 * the event type that triggered the callback.
 * 
 * The event type @p type to trigger the function mys be one of 
 * EVAS_CALLBACK_MOUSE_IN, EVAS_CALLBACK_MOUSE_OUT, EVAS_CALLBACK_MOUSE_DOWN,
 * EVAS_CALLBACK_MOUSE_UP, EVAS_CALLBACK_MOUSE_MOVE, EVAS_CALLBACK_FREE, 
 * EVAS_CALLBACK_KEY_DOWN, EVAS_CALLBACK_KEY_UP, EVAS_CALLBACK_FOCUS_IN
 * or EVAS_CALLBACK_FOCUS_OUT. This determines the kind of event that will
 * trigger the callback to be called. The @p event_info pointer passed to the
 * callback will be one of the following, depending on the event tiggering it:
 * 
 * EVAS_CALLBACK_MOUSE_IN: event_info = pointer to Evas_Event_Mouse_In
 * 
 * This event is triggered when the mouse pointer enters the region of 
 * the object @p obj. This may occur by the mouse pointer being moved by
 * evas_event_feed_mouse_move() or evas_event_feed_mouse_move_data() calls,
 * or by the object being shown, raised, moved, resized, or other objects
 * being moved out of the way, hidden, lowered or moved out of the way.
 * 
 * EVAS_CALLBACK_MOUSE_OUT: event_info = pointer to Evas_Event_Mouse_Out
 * 
 * This event is triggered exactly like EVAS_CALLBACK_MOUSE_IN is, but occurs
 * when the mouse pointer exits an object. Note that no out events will be
 * reported if the mouse pointer is implicitly grabbed to an object (the
 * mouse buttons are down at all and any were pressed on that object). An
 * out event will be reported as soon as the mouse is no longer grabbed
 * (no mouse buttons are depressed).
 * 
 * EVAS_CALLBACK_MOUSE_DOWN: event_info = pointer to Evas_Event_Mouse_Down
 * 
 * EVAS_CALLBACK_MOUSE_UP: event_info = pointer to Evas_Event_Mouse_Up
 * 
 * EVAS_CALLBACK_FREE: event_info = NULL
 * 
 * EVAS_CALLBACK_KEY_DOWN: event_info = pointer to Evas_Event_Key_Down
 * 
 * EVAS_CALLBACK_KEY_UP: event_info = pointer to Evas_Event_Key_Up
 * 
 * EVAS_CALLBACK_FOCUS_IN: event_info = NULL
 * 
 * EVAS_CALLBACK_FOCUS_OUT: event_info = NULL
 * 
 * Example:
 * @code
 * extern Evas_Object *object;
 * extern void *my_data;
 * void down_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 * 
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_UP, up_callback, my_data);
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_DOWN, down_callback, my_data);
 * @endcode
 */
void
evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   
   if (!func) return;
   if (obj->smart.smart) return;
   
   fn = evas_mem_calloc(sizeof(Evas_Func_Node));
   if (!fn) return;
   fn->func = func;
   fn->data = data;
   do
     {
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
	     free(fn);
	     return;
	     break;
	  }
	if (!evas_list_alloc_error()) return;
	MERR_BAD();	
	if (!evas_mem_free(sizeof(Evas_List)))
	  {
	     if (!evas_mem_degrade(sizeof(Evas_List)))
	       {
		  MERR_FATAL();
		  return;
	       }
	  }
     }
   while (evas_list_alloc_error());
}

void *
evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info))
{
   /* MEM OK */
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
