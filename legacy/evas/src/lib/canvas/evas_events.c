#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

int
evas_event_passes_through(Evas_Object *obj)
{
   if (obj->pass_events) return 1;
   if (obj->smart.parent) return evas_event_passes_through(obj->smart.parent);
   return 0;
}

Evas_List *
evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y)
{
   Evas_Object_List *l;
   Evas_List *in = NULL;

   if (!e->layers) return NULL;
   for (l = ((Evas_Object_List *)(e->layers))->last; l; l = l->prev)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;
	
	lay = (Evas_Layer *)l;	     
	for (l2 = ((Evas_Object_List *)(lay->objects))->last; l2; l2 = l2->prev)
	  {
	     Evas_Object *obj;
	     
	     obj = (Evas_Object *)l2;
	     if (obj == stop) goto done;
	     if ((!evas_event_passes_through(obj)) && (!obj->smart.smart))
	       {
		  evas_object_clip_recalc(obj);
		  if ((evas_object_is_in_output_rect(obj, x, y, 1, 1)) &&
		      (obj->cur.visible) &&
		      (obj->delete_me == 0) &&
		      (evas_object_clippers_is_visible(obj)) &&
		      (!obj->clip.clipees))
		    {
		       in = evas_list_append(in, obj);
		       if (!obj->repeat_events) goto done;
		    }
	       }
	  }
     }
   done:
   return in;
}

static Evas_List *evas_event_list_copy(Evas_List *list);
static Evas_List *
evas_event_list_copy(Evas_List *list)
{
   Evas_List *l, *new_l = NULL;
   
   for (l = list; l; l = l->next)
     new_l = evas_list_append(new_l, l->data);
   return new_l;
}
/* public functions */

/**
 * @defgroup Evas_Event_Freezing_Group Evas Event Freezing Functions
 *
 * Functions that deal with the freezing of event processing of an evas.
 */

/**
 * Freeze alll event processing
 * @param e The canvas to freeze event processing on
 * 
 * This function will indicate to evas that the canvas @p e is to have all
 * event processing frozen until a matching evas_event_thaw() function is
 * called on the same canvas. Every freeze call must be matched by a thaw call
 * in order to completely thaw out a canvas.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * extern Evas_Object *object;
 * 
 * evas_event_freeze(evas);
 * evas_object_move(object, 50, 100);
 * evas_object_resize(object, 200, 200);
 * evas_event_thaw(evas);
 * @endcode
 * @ingroup Evas_Event_Freezing_Group
 */
void
evas_event_freeze(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen++;
}

/**
 * Thaw a canvas out after freezing
 * @param e The canvas to thaw out
 * 
 * This will thaw out a canvas after a matching evas_event_freeze() call. If
 * this call completely thaws out a canvas, events will start being processed
 * again after this call, but this call will not invole any "missed" events
 * to be evaluated.
 * 
 * See evas_event_freeze() for an example.
 * @ingroup Evas_Event_Freezing_Group
 */
void
evas_event_thaw(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen--;
   if (e->events_frozen < 0)
     evas_debug_generic("  Thaw of events when already thawed!!!\n");
}

/**
 * Return the freeze count of a given canvas
 * @param e The canvas to fetch the freeze count from
 * 
 * This returns the number of times the canvas has been told to freeze events.
 * It is possible to call evas_event_freeze() multiple times, and these must
 * be matched by evas_event_thaw() calls. This call allows the program to
 * discover just how many times things have been frozen in case it may want
 * to break out of a deep freeze state where the count is high.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * 
 * while (evas_event_freeze_get(evas) > 0) evas_event_thaw(evas);
 * @endcode
 * @ingroup Evas_Event_Freezing_Group
 */
int
evas_event_freeze_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return e->events_frozen;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_mouse_down(Evas *e, int b, Evas_Button_Flags flags, const void *data)
{
   Evas_List *l, *copy;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((b < 1) || (b > 32)) return;
   
   e->pointer.button |= (1 << (b - 1));

   if (e->events_frozen > 0) return;
   
   copy = evas_event_list_copy(e->pointer.object.in);
   for (l = copy; l; l = l->next)
     {
	Evas_Object *obj;
	Evas_Event_Mouse_Down ev;
	
	obj = l->data;
	obj->mouse_grabbed++;
	e->pointer.mouse_grabbed++;
		       
	ev.button = b;
	ev.output.x = e->pointer.x;
	ev.output.y = e->pointer.y;
	ev.canvas.x = e->pointer.canvas_x;
	ev.canvas.y = e->pointer.canvas_y;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.flags = flags;
	if (!e->events_frozen)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_DOWN, &ev);
     }
   if (copy) copy = evas_list_free(copy);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_mouse_up(Evas *e, int b, Evas_Button_Flags flags, const void *data)
{
   Evas_List *l, *copy;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   if ((b < 1) || (b > 32)) return;
   
   e->pointer.button &= ~(1 << (b - 1));

   if (e->events_frozen > 0) return;
   
   copy = evas_event_list_copy(e->pointer.object.in);
   for (l = copy; l; l = l->next)
     {
	Evas_Object *obj;
	Evas_Event_Mouse_Up ev;
	
	obj = l->data;
	if (obj->mouse_grabbed > 0) obj->mouse_grabbed--;
	if (e->pointer.mouse_grabbed > 0) e->pointer.mouse_grabbed--;
	ev.button = b;
	ev.output.x = e->pointer.x;
	ev.output.y = e->pointer.y;
	ev.canvas.x = e->pointer.canvas_x;
	ev.canvas.y = e->pointer.canvas_y;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.flags = flags;
	if (!e->events_frozen)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_UP, &ev);
     }
   if (copy) copy = evas_list_free(copy);
   if (!e->pointer.button) 
     {
	Evas_List *ins;	
	Evas_List *l;
   
	/* get new list of ins */
	ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
	/* go thru old list of in objects */
	copy = evas_event_list_copy(e->pointer.object.in);
	for (l = copy; l; l = l->next)
	  {
	     Evas_Object *obj;
	    
	     obj = l->data;
	     if ((!evas_list_find(ins, obj)) ||
		 (!e->pointer.inside))
	       {
		  Evas_Event_Mouse_Out ev;
		  
		  obj->mouse_in = 0;
		  ev.buttons = e->pointer.button;
		  ev.output.x = e->pointer.x;
		  ev.output.y = e->pointer.y;
		  ev.canvas.x = e->pointer.canvas_x;
		  ev.canvas.y = e->pointer.canvas_y;
		  ev.data = (void *)data;
		  ev.modifiers = &(e->modifiers);
		  ev.locks = &(e->locks);		  
		  if (!e->events_frozen)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
	       }
	  }
	if (copy) copy = evas_list_free(copy);
	if (e->pointer.inside)
	  {
	     for (l = ins; l; l = l->next)
	       {
		  Evas_Object *obj;
		  
		  obj = l->data;
		  
		  if (!evas_list_find(e->pointer.object.in, obj))
		    {
		       Evas_Event_Mouse_In ev;
		       
		       obj->mouse_in = 1;
		       ev.buttons = e->pointer.button;
		       ev.output.x = e->pointer.x;
		       ev.output.y = e->pointer.y;
		       ev.canvas.x = e->pointer.canvas_x;
		       ev.canvas.y = e->pointer.canvas_y;
		       ev.data = (void *)data;
		       ev.modifiers = &(e->modifiers);
		       ev.locks = &(e->locks);
		       if (!e->events_frozen)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
		    }
	       }
	  }
	else
	  {
	     ins = evas_list_free(ins);
	  }
	/* free our old list of ins */
	e->pointer.object.in = evas_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
	if (e->pointer.inside)
	  evas_event_feed_mouse_move(e, e->pointer.x, e->pointer.y, data);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_mouse_wheel(Evas *e, int direction, int z, const void *data)
{
   Evas_List *l, *copy;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   
   copy = evas_event_list_copy(e->pointer.object.in);
   
   for (l = copy; l; l = l->next)
   {
      Evas_Event_Mouse_Wheel ev;
      Evas_Object *obj = l->data;
	  
      ev.direction = direction;
      ev.z = z;
      ev.output.x = e->pointer.x;
      ev.output.y = e->pointer.y;
      ev.canvas.x = e->pointer.canvas_x;
      ev.canvas.y = e->pointer.canvas_y;
      ev.data = (void *) data;
      ev.modifiers = &(e->modifiers);
      ev.locks = &(e->locks);	
      if (!e->events_frozen)   
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_WHEEL, &ev);
   }

   return;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_mouse_move(Evas *e, int x, int y, const void *data)
{
   int px, py;
   Evas_Coord pcx, pcy;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   px = e->pointer.x;
   py = e->pointer.y;
   pcx = e->pointer.canvas_x;
   pcy = e->pointer.canvas_y;
   
   if (e->events_frozen > 0) return;
   
   e->pointer.x = x;
   e->pointer.y = y;
   e->pointer.canvas_x = evas_coord_screen_x_to_world(e, x);
   e->pointer.canvas_y = evas_coord_screen_y_to_world(e, y);
   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;
   /* if our mouse button is grabbed to any objects */   
   if (e->pointer.mouse_grabbed != 0)
     {
	/* go thru old list of in objects */
	Evas_List *outs = NULL;
	Evas_List *l, *copy;

	copy = evas_event_list_copy(e->pointer.object.in);
	for (l = copy; l; l = l->next)
	  {
	     Evas_Object *obj;
	     
	     obj = l->data;
	     if ((obj->cur.visible) &&
		 (evas_object_clippers_is_visible(obj)) &&
		 (!evas_event_passes_through(obj)) &&
		 (!obj->smart.smart) &&
		 (!obj->clip.clipees))
	       {
		  if ((px != x) || (py != y))
		    {
		       Evas_Event_Mouse_Move ev;
		       
		       ev.buttons = e->pointer.button;
		       ev.cur.output.x = e->pointer.x;
		       ev.cur.output.y = e->pointer.y;
		       ev.cur.canvas.x = e->pointer.canvas_x;
		       ev.cur.canvas.y = e->pointer.canvas_y;
		       ev.prev.output.x = px;
		       ev.prev.output.y = py;
		       ev.prev.canvas.x = pcx;
		       ev.prev.canvas.y = pcy;
		       ev.data = (void *)data;
		       ev.modifiers = &(e->modifiers);
		       ev.locks = &(e->locks);
		       if (!e->events_frozen)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_MOVE, &ev);
		    }
	       }
	     else
	       outs = evas_list_append(outs, obj);
	  }
	if (copy) copy = evas_list_free(copy);
	while (outs)
	  {
	     Evas_Object *obj;
	     
	     obj = outs->data;
	     outs = evas_list_remove(outs, obj);
	     e->pointer.object.in = evas_list_remove(e->pointer.object.in, obj);
	       {
		  Evas_Event_Mouse_Out ev;
		  
		  obj->mouse_in = 0;
		  ev.buttons = e->pointer.button;
		  ev.output.x = e->pointer.x;
		  ev.output.y = e->pointer.y;
		  ev.canvas.x = e->pointer.canvas_x;
		  ev.canvas.y = e->pointer.canvas_y;
		  ev.data = (void *)data;
		  ev.modifiers = &(e->modifiers);
		  ev.locks = &(e->locks);
		  if (!e->events_frozen)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
	       }	     
	  }
     }
   else
     {
	Evas_List *ins;
	Evas_List *l, *copy;
   
	/* get all new in objects */
	ins = evas_event_objects_event_list(e, NULL, x, y);
	/* go thru old list of in objects */
	copy = evas_event_list_copy(e->pointer.object.in);
	for (l = copy; l; l = l->next)
	  {
	     Evas_Object *obj;
	     
	     obj = l->data;
	     /* if its under the pointer and its visible and its in the new */
	     /* in list */
	     evas_object_clip_recalc(obj);
	     if (evas_object_is_in_output_rect(obj, x, y, 1, 1) && 
		 (obj->cur.visible) &&
		 (evas_object_clippers_is_visible(obj)) &&
		 (evas_list_find(ins, obj)) &&
		 (!evas_event_passes_through(obj)) &&
		 (!obj->smart.smart) &&
		 (!obj->clip.clipees))
	       {
		  if ((px != x) || (py != y))
		    {
		       Evas_Event_Mouse_Move ev;
		       
		       ev.buttons = e->pointer.button;
		       ev.cur.output.x = e->pointer.x;
		       ev.cur.output.y = e->pointer.y;
		       ev.cur.canvas.x = e->pointer.canvas_x;
		       ev.cur.canvas.y = e->pointer.canvas_y;
		       ev.prev.output.x = px;
		       ev.prev.output.y = py;
		       ev.prev.canvas.x = pcx;
		       ev.prev.canvas.y = pcy;
		       ev.data = (void *)data;
		       ev.modifiers = &(e->modifiers);
		       ev.locks = &(e->locks);		       
		       if (!e->events_frozen)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_MOVE, &ev);
		    }
	       }
	     /* otherwise it has left the object */
	     else
	       {
		  Evas_Event_Mouse_Out ev;
		  
		  obj->mouse_in = 0;
		  ev.buttons = e->pointer.button;
		  ev.output.x = e->pointer.x;
		  ev.output.y = e->pointer.y;
		  ev.canvas.x = e->pointer.canvas_x;
		  ev.canvas.y = e->pointer.canvas_y;
		  ev.data = (void *)data;
		  ev.modifiers = &(e->modifiers);
		  ev.locks = &(e->locks);		  
		  if (!e->events_frozen)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
	       }
	  }
	if (copy) copy = evas_list_free(copy);
	/* go thru our current list of ins */
	for (l = ins; l; l = l->next)
	  {
	     Evas_Object *obj;
	     
	     obj = l->data;
	     /* if its not in the old list of ins send an enter event */
	     if (!evas_list_find(e->pointer.object.in, obj))
	       {
		  Evas_Event_Mouse_In ev;
		  
		  obj->mouse_in = 1;
		  ev.buttons = e->pointer.button;
		  ev.output.x = e->pointer.x;
		  ev.output.y = e->pointer.y;
		  ev.canvas.x = e->pointer.canvas_x;
		  ev.canvas.y = e->pointer.canvas_y;
		  ev.data = (void *)data;
		  ev.modifiers = &(e->modifiers);
		  ev.locks = &(e->locks);		  
		  if (!e->events_frozen)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
	       }
	  }
	/* free our old list of ins */
	evas_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_mouse_in(Evas *e, const void *data)
{
   Evas_List *ins;	
   Evas_List *l;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 1;

   if (e->events_frozen > 0) return;

   if (e->pointer.mouse_grabbed != 0) return;
   
   /* get new list of ins */
   ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
   for (l = ins; l; l = l->next)
     {
	Evas_Object *obj;
	
	obj = l->data;
	
	if (!evas_list_find(e->pointer.object.in, obj))
	  {
	     Evas_Event_Mouse_In ev;
	     
	     obj->mouse_in = 1;
	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.canvas_x;
	     ev.canvas.y = e->pointer.canvas_y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     if (!e->events_frozen)
	       evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
	  }
     }
   /* free our old list of ins */
   e->pointer.object.in = evas_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   evas_event_feed_mouse_move(e, e->pointer.x, e->pointer.y, data);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_mouse_out(Evas *e, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 0;
   
   if (e->events_frozen > 0) return;
   
   /* if our mouse button is grabbed to any objects */   
   if (e->pointer.mouse_grabbed == 0)
     {
	/* go thru old list of in objects */
	Evas_List *l, *copy;

	copy = evas_event_list_copy(e->pointer.object.in);
	for (l = copy; l; l = l->next)
	  {
	     Evas_Object *obj;
	     
	     obj = l->data;
	       {
		  Evas_Event_Mouse_Out ev;
		  
		  obj->mouse_in = 0;
		  ev.buttons = e->pointer.button;
		  ev.output.x = e->pointer.x;
		  ev.output.y = e->pointer.y;
		  ev.canvas.x = e->pointer.canvas_x;
		  ev.canvas.y = e->pointer.canvas_y;
		  ev.data = (void *)data;
		  ev.modifiers = &(e->modifiers);
		  ev.locks = &(e->locks);
		  if (!e->events_frozen)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
	       }
	  }
	/* free our old list of ins */
	e->pointer.object.in =  evas_list_free(e->pointer.object.in);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_key_down(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
     {
	Evas_Event_Key_Down ev;
	int exclusive;
	
	exclusive = 0;
	ev.keyname = (char *)keyname;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.key = key;
	ev.string = string;
	ev.compose = compose;
	if (e->grabs)
	  {
	     Evas_List *l;
	     
	     e->walking_grabs++;
	     for (l = e->grabs; l; l= l->next)
	       {
		  Evas_Key_Grab *g;
		  
		  g = l->data;
		  if (g->just_added)
		    {
		       g->just_added = 0;
		       continue;
		    }
		  if (g->delete_me) continue;
		  if (((e->modifiers.mask & g->modifiers) ||
		       (g->modifiers == e->modifiers.mask)) &&
		      (!((e->modifiers.mask & g->not_modifiers) ||
			 (g->not_modifiers == ~e->modifiers.mask))) &&
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (!e->events_frozen)
			 evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_DOWN, &ev);
		       if (g->exclusive) exclusive = 1;
		    }
	       }
	     e->walking_grabs--;
	     if (e->walking_grabs <= 0)
	       {
		  while (e->delete_grabs > 0)
		    {
		       Evas_List *l;
		       
		       e->delete_grabs--;
		       for (l = e->grabs; l;)
			 {
			    Evas_Key_Grab *g;
			    
			    g = l->data;
			    l = l->next;
			    if (g->delete_me)
			      evas_key_grab_free(g->object, g->keyname, g->modifiers, g->not_modifiers);
			 }
		    }
	       }
	  }
	if ((e->focused) && (!exclusive))
	  {
	     if (!e->events_frozen)
	       evas_object_event_callback_call(e->focused, EVAS_CALLBACK_KEY_DOWN, &ev);
	  }
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_event_feed_key_up(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
     {
	Evas_Event_Key_Up ev;
	int exclusive;
	
	exclusive = 0;	
	ev.keyname = (char *)keyname;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.key = key;
	ev.string = string;
	ev.compose = compose;
	if (e->grabs)
	  {
	     Evas_List *l;
	     
	     e->walking_grabs++;
	     for (l = e->grabs; l; l= l->next)
	       {
		  Evas_Key_Grab *g;
		  
		  g = l->data;
		  if (g->just_added)
		    {
		       g->just_added = 0;
		       continue;
		    }
		  if (g->delete_me) continue;
		  if (((e->modifiers.mask & g->modifiers) ||
		       (g->modifiers == e->modifiers.mask)) &&
		      (!((e->modifiers.mask & g->not_modifiers) ||
			 (g->not_modifiers == ~e->modifiers.mask))) &&
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (!e->events_frozen)
			 evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_UP, &ev);
		       if (g->exclusive) exclusive = 1;
		    }		      
	       }
	     e->walking_grabs--;
	     if (e->walking_grabs <= 0)
	       {
		  while (e->delete_grabs > 0)
		    {
		       Evas_List *l;
		       
		       e->delete_grabs--;
		       for (l = e->grabs; l; l= l->next)
			 {
			    Evas_Key_Grab *g;
			    
			    g = l->data;
			    l = l->next;
			    if (g->delete_me)
			      evas_key_grab_free(g->object, g->keyname, g->modifiers, g->not_modifiers);
			 }
		    }
	       }
	  }
	if ((e->focused) && (!exclusive))
	  {
	     if (!e->events_frozen)
	       evas_object_event_callback_call(e->focused, EVAS_CALLBACK_KEY_UP, &ev);
	  }
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_pass_events_set(Evas_Object *obj, Evas_Bool pass)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->pass_events = pass;
   if (evas_object_is_in_output_rect(obj, 
				     obj->layer->evas->pointer.x, 
				     obj->layer->evas->pointer.y, 1, 1))
     evas_event_feed_mouse_move(obj->layer->evas, 
				obj->layer->evas->pointer.x, 
				obj->layer->evas->pointer.y,
				NULL);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Bool
evas_object_pass_events_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pass_events;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_repeat_events_set(Evas_Object *obj, Evas_Bool repeat)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->repeat_events = repeat;
   if (evas_object_is_in_output_rect(obj, 
				     obj->layer->evas->pointer.x, 
				     obj->layer->evas->pointer.y, 1, 1))
     evas_event_feed_mouse_move(obj->layer->evas, 
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				NULL);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Bool
evas_object_repeat_events_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->repeat_events;
}
