#include "evas_common.h"
#include "evas_private.h"

static void evas_object_event_callback_list_post_free(Eina_Inlist **list);
static void evas_object_event_callback_clear(Evas_Object *obj);

static void
evas_object_event_callback_list_post_free(Eina_Inlist **list)
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
   evas_object_event_callback_list_post_free(&obj->callbacks->callbacks);
   if (!obj->callbacks->callbacks)
     {
        free(obj->callbacks);
	obj->callbacks = NULL;
     }
}

void
evas_object_event_callback_all_del(Evas_Object *obj)
{
   Eina_Inlist *l;

   if (!obj->callbacks) return;
   for (l = obj->callbacks->callbacks; l; l = l->next)
     {
	Evas_Func_Node *fn;

	fn = (Evas_Func_Node *)l;
	fn->delete_me = 1;
     }
}

void
evas_object_event_callback_cleanup(Evas_Object *obj)
{
   /* MEM OK */
   if (!obj->callbacks) return;
   evas_object_event_callback_list_post_free(&obj->callbacks->callbacks);
   free(obj->callbacks);
   obj->callbacks = NULL;
}

void
evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info)
{
   /* MEM OK */
   Eina_Inlist **l_mod = NULL, *l;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;
   Evas *e;

   if (obj->delete_me) return;
   e = evas_object_evas_get(obj);
     
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
	          if (fn->func)
	            fn->func(fn->data, obj->layer->evas, obj, event_info);
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

/**
 * @defgroup Evas_Object_Callback_Group Object Callback Functions
 *
 * Functions that add and remove callbacks to evas objects.
 */

/**
 * @todo Move this next code example and most of the documentation for
 * this next function into the group documentation.
 */

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
 * object. If pointermode is EVAS_OBJECT_POINTER_MODE_AUTOGRAB (default)
 * this causes this object to passively grab the mouse until all mouse
 * buttons have been released.
 * That means if this mouse button is the first to be pressed, all future
 * mouse events will be reported to only this object until no buttons are
 * down. That includes mouse move events, in and out events, and further
 * button presses. When all buttons are released, event propagation occurs
 * as normal.
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
 * @ingroup Evas_Object_Callback_Group
 */
EAPI void
evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), const void *data)
{
   /* MEM OK */
   Evas_Func_Node *fn;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (!func) return;

   fn = evas_mem_calloc(sizeof(Evas_Func_Node));
   if (!fn) return;
   fn->func = func;
   fn->data = (void *)data;
   fn->type = type;

   if (!obj->callbacks)
     obj->callbacks = evas_mem_calloc(sizeof(Evas_Callbacks));
   if (!obj->callbacks)
     {
	free(fn);
	return;
     }
   obj->callbacks->callbacks = 
     eina_inlist_append(obj->callbacks->callbacks, EINA_INLIST_GET(fn));
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
 * @ingroup Evas_Object_Callback_Group
 */
EAPI void *
evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info))
{
   /* MEM OK */
   Eina_Inlist *l;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   if (!func) return NULL;

   if (!obj->callbacks) return NULL;

   for (l = obj->callbacks->callbacks; l; l = l->next)
     {
	Evas_Func_Node *fn;

	fn = (Evas_Func_Node *)l;
	if ((fn->func == func) && (fn->type == type) && (!fn->delete_me))
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

/**
 * Delete a callback function from an object
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @param data The data pointer that was to be passed to the callback
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the object
 * @p obj which was triggered by the event type @p type and was calling the
 * function @p func with data @p data when triggered. If the removal is
 * successful it will also return the data pointer that was passed to
 * evas_object_event_callback_add() (that will be the same as the parameter)
 * when the callback was added to the object. If not successful NULL will be
 * returned.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del_full(object, EVAS_CALLBACK_MOUSE_UP, up_callback, data);
 * @endcode
 * @ingroup Evas_Object_Callback_Group
 */
EAPI void *
evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), const void *data)
{
   /* MEM OK */
   Eina_Inlist *l;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();

   if (!func) return NULL;

   if (!obj->callbacks) return NULL;

   for (l = obj->callbacks->callbacks; l; l = l->next)
     {
	Evas_Func_Node *fn;

	fn = (Evas_Func_Node *)l;
	if ((fn->func == func) && (fn->type == type) && (fn->data == data) && (!fn->delete_me))
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
