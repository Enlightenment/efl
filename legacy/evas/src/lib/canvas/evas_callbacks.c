#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

static void evas_object_event_callback_list_free(Evas_Object_List **list);
static void evas_object_event_callback_list_post_free(Evas_Object_List **list);
static void evas_object_event_callback_clear(Evas_Object *obj);

static void
evas_object_event_callback_list_free(Evas_Object_List **list)
{
   /* MEM OK */
   while (*list)
     {
	Evas_Func_Node *fn;
	
	fn = (Evas_Func_Node *)(*list);
	*list = evas_object_list_remove(*list, fn);
	free(fn);
     }
}

static void
evas_object_event_callback_list_post_free(Evas_Object_List **list)
{
   Evas_Object_List *l;
   
   /* MEM OK */
   for (l = *list; l;)
     {
	Evas_Func_Node *fn;
	
	fn = (Evas_Func_Node *)l;
	l = l->next;
	if (fn->delete_me)
	  {
	     *list = evas_object_list_remove(*list, fn);
	     free(fn);
	  }
     }
}

static void
evas_object_event_callback_clear(Evas_Object *obj)
{
   if (!obj->callbacks) return;
   if (!obj->callbacks->deletions_waiting) return;
   obj->callbacks->deletions_waiting = 0;
   evas_object_event_callback_list_post_free(&(obj->callbacks->in));
   evas_object_event_callback_list_post_free(&(obj->callbacks->out));
   evas_object_event_callback_list_post_free(&(obj->callbacks->down));
   evas_object_event_callback_list_post_free(&(obj->callbacks->up));
   evas_object_event_callback_list_post_free(&(obj->callbacks->move));
   evas_object_event_callback_list_post_free(&(obj->callbacks->wheel));
   evas_object_event_callback_list_post_free(&(obj->callbacks->free));
   evas_object_event_callback_list_post_free(&(obj->callbacks->key_down));
   evas_object_event_callback_list_post_free(&(obj->callbacks->key_up));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_focus_in));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_focus_out));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_show));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_hide));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_move));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_resize));
   evas_object_event_callback_list_post_free(&(obj->callbacks->obj_restack));
   if ((!obj->callbacks->in) &&
       (!obj->callbacks->out) &&
       (!obj->callbacks->down) &&
       (!obj->callbacks->up) &&
       (!obj->callbacks->move) &&
       (!obj->callbacks->wheel) &&
       (!obj->callbacks->free) &&
       (!obj->callbacks->key_down) &&
       (!obj->callbacks->key_up) &&
       (!obj->callbacks->obj_focus_in) &&
       (!obj->callbacks->obj_focus_out) &&
       (!obj->callbacks->obj_show) &&
       (!obj->callbacks->obj_hide) &&
       (!obj->callbacks->obj_move) &&
       (!obj->callbacks->obj_resize) &&
       (!obj->callbacks->obj_restack))
     {
	free(obj->callbacks);
	obj->callbacks = NULL;	
     }
}

void
evas_object_event_callback_cleanup(Evas_Object *obj)
{
   /* MEM OK */
   if (!obj->callbacks) return;
   evas_object_event_callback_list_free(&(obj->callbacks->in));
   evas_object_event_callback_list_free(&(obj->callbacks->out));
   evas_object_event_callback_list_free(&(obj->callbacks->down));
   evas_object_event_callback_list_free(&(obj->callbacks->up));
   evas_object_event_callback_list_free(&(obj->callbacks->move));
   evas_object_event_callback_list_free(&(obj->callbacks->wheel));
   evas_object_event_callback_list_free(&(obj->callbacks->free));
   evas_object_event_callback_list_free(&(obj->callbacks->key_down));
   evas_object_event_callback_list_free(&(obj->callbacks->key_up));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_focus_in));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_focus_out));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_show));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_hide));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_move));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_resize));
   evas_object_event_callback_list_free(&(obj->callbacks->obj_restack));
   free(obj->callbacks);
   obj->callbacks = NULL;
}

void
evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info)
{
   /* MEM OK */
   Evas_Object_List **l_mod, *l;
   
   if (!obj->callbacks)
     {
	if (obj->smart.parent)
	  evas_object_event_callback_call(obj->smart.parent, type, event_info);
	return;
     }
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
	l_mod = &(obj->callbacks->in);
	break;
      case EVAS_CALLBACK_MOUSE_OUT:
	l_mod = &(obj->callbacks->out);
	break;
      case EVAS_CALLBACK_MOUSE_DOWN:
	l_mod = &(obj->callbacks->down);
	break;
      case EVAS_CALLBACK_MOUSE_UP:
	l_mod = &(obj->callbacks->up);
	break;
      case EVAS_CALLBACK_MOUSE_MOVE:
	l_mod = &(obj->callbacks->move);
	break;
	  case EVAS_CALLBACK_MOUSE_WHEEL:
	l_mod = &(obj->callbacks->wheel);
	break;
      case EVAS_CALLBACK_FREE:
	l_mod = &(obj->callbacks->free);
	break;
      case EVAS_CALLBACK_KEY_DOWN:
	l_mod = &(obj->callbacks->key_down);
	break;
      case EVAS_CALLBACK_KEY_UP:
	l_mod = &(obj->callbacks->key_up);
	break;
      case EVAS_CALLBACK_FOCUS_IN:
	l_mod = &(obj->callbacks->obj_focus_in);
	break;
      case EVAS_CALLBACK_FOCUS_OUT:
	l_mod = &(obj->callbacks->obj_focus_out);
	break;
      case EVAS_CALLBACK_SHOW:
	l_mod = &(obj->callbacks->obj_show);
	break;
      case EVAS_CALLBACK_HIDE:
	l_mod = &(obj->callbacks->obj_hide);
	break;
      case EVAS_CALLBACK_MOVE:
	l_mod = &(obj->callbacks->obj_move);
	break;
      case EVAS_CALLBACK_RESIZE:
	l_mod = &(obj->callbacks->obj_resize);
	break;
      case EVAS_CALLBACK_RESTACK:
	l_mod = &(obj->callbacks->obj_restack);
	break;
      default:
	return;
	break;
     }
   obj->callbacks->walking_list++;
   for (l = *l_mod; l; l = l->next)
     {
	Evas_Func_Node *fn;
	
	fn = (Evas_Func_Node *)l;
	if (!fn->delete_me)
	  fn->func(fn->data, obj->layer->evas, obj, event_info);
	if (obj->delete_me) break;
     }
   obj->callbacks->walking_list--;
   if (!obj->callbacks->walking_list)
     evas_object_event_callback_clear(obj);
   if (obj->smart.parent)
     evas_object_event_callback_call(obj->smart.parent, type, event_info);
}

/**
 * Add a callback function to an object
 * @param obj Object to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param func The function to be called when the event is triggered
 * @param data The data pointer to be passed to @p func
 * 
 * This function adds a function callback to an object when the event of type
 * @p type occurs on object @p obj. The function is @p func.
 * 
 * In the event of a memory allocation error during addition of the callback to
 * the object, evas_alloc_error() should be used to determine the nature of
 * the error, if any, and the program should sensibly try and recover.
 * 
 * The function will be passed the pointer @p data when it is called. A 
 * callback function must look like this:
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
 * The event type @p type to trigger the function may be one of 
 * EVAS_CALLBACK_MOUSE_IN, EVAS_CALLBACK_MOUSE_OUT, EVAS_CALLBACK_MOUSE_DOWN,
 * EVAS_CALLBACK_MOUSE_UP, EVAS_CALLBACK_MOUSE_MOVE, EVAS_CALLBACK_MOUSE_WHEEL,
 * EVAS_CALLBACK_FREE, EVAS_CALLBACK_KEY_DOWN, EVAS_CALLBACK_KEY_UP, 
 * EVAS_CALLBACK_FOCUS_IN, EVAS_CALLBACK_FOCUS_OUT, EVAS_CALLBACK_SHOW, 
 * EVAS_CALLBACK_HIDE, EVAS_CALLBACK_MOVE, EVAS_CALLBACK_RESIZE or EVAS_CALLBACK_RESTACK.
 * This determines the kind of event that will trigger the callback to be called.
 * The @p event_info pointer passed to the callback will be one of the 
 * following, depending on the event triggering it:
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
 * out event will be reported as soon as the mouse is no longer grabbed (no
 * mouse buttons are depressed). Out events will be reported once all buttons
 * are released, if the mouse has left the object.
 * 
 * EVAS_CALLBACK_MOUSE_DOWN: event_info = pointer to Evas_Event_Mouse_Down
 * 
 * This event is triggered by a mouse button being depressed while over an
 * object. This causes this object to passively grab the mouse until all mouse
 * buttons have been released. That means if this mouse button is the first to
 * be pressed, all future mouse events will be reported to only this object
 * until no buttons are down. That includes mouse move events, in and out
 * events, and further button presses. When all buttons are released, event
 * propagation occurs as normal.
 * 
 * EVAS_CALLBACK_MOUSE_UP: event_info = pointer to Evas_Event_Mouse_Up
 * 
 * This event is triggered by a mouse button being released while over an
 * object or when passively grabbed to an object. If this is the last mouse
 * button to be raised on an object then the passive grab is released and
 * event processing will continue as normal.
 * 
 * EVAS_CALLBACK_MOUSE_MOVE: event_info = pointer to Evas_Event_Mouse_Move
 * 
 * This event is triggered by the mouse pointer moving while over an object or
 * passively grabbed to an object.
 *
 * EVAS_CALLBACK_MOUSE_WHEEL: event_info = pointer to Evas_Event_Mouse_Wheel
 *
 * This event is triggered by the mouse wheel being rolled while over an object
 * or passively grabbed to an object.
 *
 * EVAS_CALLBACK_FREE: event_info = NULL
 * 
 * This event is triggered just before Evas is about to free all memory used
 * by an object and remove all references to it. This is useful for programs
 * to use if they attached data to an object and want to free it when the
 * object is deleted. The object is still valid when this callback is called,
 * but after this callback returns, there is no guarantee on the object's
 * validity.
 * 
 * EVAS_CALLBACK_KEY_DOWN: event_info = pointer to Evas_Event_Key_Down
 * 
 * This callback is called when a key is pressed and the focus is on the
 * object, or a key has been grabbed to a particular object which wants to
 * intercept the key press regardless of what object has the focus.
 * 
 * EVAS_CALLBACK_KEY_UP: event_info = pointer to Evas_Event_Key_Up
 * 
 * This callback is called when a key is released and the focus is on the
 * object, or a key has been grabbed to a particular object which wants to
 * intercept the key release regardless of what object has the focus.
 * 
 * EVAS_CALLBACK_FOCUS_IN: event_info = NULL
 * 
 * This event is called when an object gains the focus. When the callback is
 * called the object has already gained the focus.
 * 
 * EVAS_CALLBACK_FOCUS_OUT: event_info = NULL
 * 
 * This event is triggered by an object losing the focus. When the callback is
 * called the object has already lost the focus.
 * 
 * EVAS_CALLBACK_SHOW: event_info = NULL
 * 
 * This event is triggered by the object being shown by evas_object_show().
 * 
 * EVAS_CALLBACK_HIDE: event_info = NULL
 * 
 * This event is triggered by an object being hidden by evas_object_hide().
 * 
 * EVAS_CALLBACK_MOVE: event_info = NULL
 * 
 * This event is triggered by an object being moved. evas_object_move() can
 * trigger this, as can any object-specific manipulations that would mean the
 * object's origin could move.
 * 
 * EVAS_CALLBACK_RESIZE: event_info = NULL
 * 
 * This event is triggered by an object being resized. Resizes can be
 * triggered by evas_object_resize() or by any object-specific calls that may
 * cause the object to resize.
 * 
 * Example:
 * @code
 * extern Evas_Object *object;
 * extern void *my_data;
 * void down_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 * 
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_UP, up_callback, my_data);
 * if (evas_alloc_error() != EVAS_ALLOC_ERROR_NONE)
 *   {
 *     fprintf(stderr, "ERROR: Callback registering failed! Abort!\n");
 *     exit(-1);
 *   }
 * evas_object_event_callback_add(object, EVAS_CALLBACK_MOUSE_DOWN, down_callback, my_data);
 * if (evas_alloc_error() != EVAS_ALLOC_ERROR_NONE)
 *   {
 *     fprintf(stderr, "ERROR: Callback registering failed! Abort!\n");
 *     exit(-1);
 *   }
 * @endcode
 */
void
evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;
   Evas_Object_List **l_mod;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   
   if (!func) return;
   
   fn = evas_mem_calloc(sizeof(Evas_Func_Node));
   if (!fn) return;
   fn->func = func;
   fn->data = (void *)data;

   if (!obj->callbacks)
     obj->callbacks = evas_mem_calloc(sizeof(Evas_Callbacks));
   if (!obj->callbacks)
     {
	free(fn);
	return;
     }
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
	l_mod = &(obj->callbacks->in);
	break;
      case EVAS_CALLBACK_MOUSE_OUT:
	l_mod = &(obj->callbacks->out);
	break;
      case EVAS_CALLBACK_MOUSE_DOWN:
	l_mod = &(obj->callbacks->down);
	break;
      case EVAS_CALLBACK_MOUSE_UP:
	l_mod = &(obj->callbacks->up);
	break;
      case EVAS_CALLBACK_MOUSE_MOVE:
	l_mod = &(obj->callbacks->move);
	break;
	  case EVAS_CALLBACK_MOUSE_WHEEL:
	l_mod = &(obj->callbacks->wheel);
	break;
      case EVAS_CALLBACK_FREE:
	l_mod = &(obj->callbacks->free);
	break;
      case EVAS_CALLBACK_KEY_DOWN:
	l_mod = &(obj->callbacks->key_down);
	break;
      case EVAS_CALLBACK_KEY_UP:
	l_mod = &(obj->callbacks->key_up);
	break;
      case EVAS_CALLBACK_FOCUS_IN:
	l_mod = &(obj->callbacks->obj_focus_in);
	break;
      case EVAS_CALLBACK_FOCUS_OUT:
	l_mod = &(obj->callbacks->obj_focus_out);
	break;
      case EVAS_CALLBACK_SHOW:
	l_mod = &(obj->callbacks->obj_show);
	break;
      case EVAS_CALLBACK_HIDE:
	l_mod = &(obj->callbacks->obj_hide);
	break;
      case EVAS_CALLBACK_MOVE:
	l_mod = &(obj->callbacks->obj_move);
	break;
      case EVAS_CALLBACK_RESIZE:
	l_mod = &(obj->callbacks->obj_resize);
	break;
      case EVAS_CALLBACK_RESTACK:
	l_mod = &(obj->callbacks->obj_restack);
	break;
      default:
	free(fn);
	return;
	break;
     }
   *l_mod = evas_object_list_append(*l_mod, fn);
}

/**
 * Delete a callback function from an object
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @return The data pointer that was to be passed to the callback
 * 
 * This function removes the most recently added callback from the object
 * @p obj which was triggered by the event type @p type and was calling the
 * function @p func when triggered. If the removal is successful it will also
 * return the data pointer that was passed to evas_object_event_callback_add()
 * when the callback was added to the object. If not successful NULL will be
 * returned.
 * 
 * Example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 * 
 * my_data = evas_object_event_callback_del(object, EVAS_CALLBACK_MOUSE_UP, up_callback);
 * @endcode
 */
void *
evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info))
{
   /* MEM OK */
   Evas_Object_List **l_mod, *l;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   
   if (!func) return NULL;
   
   if (!obj->callbacks) return NULL;
   
   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
	l_mod = &(obj->callbacks->in);
	break;
      case EVAS_CALLBACK_MOUSE_OUT:
	l_mod = &(obj->callbacks->out);
	break;
      case EVAS_CALLBACK_MOUSE_DOWN:
	l_mod = &(obj->callbacks->down);
	break;
      case EVAS_CALLBACK_MOUSE_UP:
	l_mod = &(obj->callbacks->up);
	break;
      case EVAS_CALLBACK_MOUSE_MOVE:
	l_mod = &(obj->callbacks->move);
	break;
	  case EVAS_CALLBACK_MOUSE_WHEEL:
	l_mod = &(obj->callbacks->wheel);
	break;
      case EVAS_CALLBACK_FREE:
	l_mod = &(obj->callbacks->free);
	break;
      case EVAS_CALLBACK_KEY_DOWN:
	l_mod = &(obj->callbacks->key_down);
	break;
      case EVAS_CALLBACK_KEY_UP:
	l_mod = &(obj->callbacks->key_up);
	break;
      case EVAS_CALLBACK_FOCUS_IN:
	l_mod = &(obj->callbacks->obj_focus_in);
	break;
      case EVAS_CALLBACK_FOCUS_OUT:
	l_mod = &(obj->callbacks->obj_focus_out);
	break;
      case EVAS_CALLBACK_SHOW:
	l_mod = &(obj->callbacks->obj_show);
	break;
      case EVAS_CALLBACK_HIDE:
	l_mod = &(obj->callbacks->obj_hide);
	break;
      case EVAS_CALLBACK_MOVE:
	l_mod = &(obj->callbacks->obj_move);
	break;
      case EVAS_CALLBACK_RESIZE:
	l_mod = &(obj->callbacks->obj_resize);
	break;
      case EVAS_CALLBACK_RESTACK:
	l_mod = &(obj->callbacks->obj_restack);
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
	     fn->delete_me = 1;
	     obj->callbacks->deletions_waiting = 1;
	     if (!obj->callbacks->walking_list)
	       evas_object_event_callback_clear(obj);
	     return data;
	  }
     }
   return NULL;
}
