#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

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
	     if ((!obj->pass_events) && (!obj->smart.smart))
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

void
evas_event_freeze(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen++;
}

void
evas_event_thaw(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen--;
}

int
evas_event_freeze_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return e->events_frozen;
}

void
evas_event_feed_mouse_down_data(Evas *e, int b, const void *data)
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
	obj->mouse_grabbed = 1;
	e->pointer.mouse_grabbed = 1;
		       
	ev.button = b;
	ev.output.x = e->pointer.x;
	ev.output.y = e->pointer.y;
	ev.canvas.x = e->pointer.canvas_x;
	ev.canvas.y = e->pointer.canvas_y;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);	
	evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_DOWN, &ev);
     }
   if (copy) copy = evas_list_free(copy);
}

void
evas_event_feed_mouse_up_data(Evas *e, int b, const void *data)
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
	if (!e->pointer.button) obj->mouse_grabbed = 0;
	ev.button = b;
	ev.output.x = e->pointer.x;
	ev.output.y = e->pointer.y;
	ev.canvas.x = e->pointer.canvas_x;
	ev.canvas.y = e->pointer.canvas_y;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);	
	if (!e->events_frozen)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_UP, &ev);
     }
   if (copy) copy = evas_list_free(copy);
   if (!e->pointer.button) 
     {
	Evas_List *ins;	
	Evas_List *l;
   
	e->pointer.mouse_grabbed = 0;
	/* get new list of ins */
	ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
	/* go thru old list of in objects */
	copy = evas_event_list_copy(e->pointer.object.in);
	for (l = copy; l; l = l->next)
	  {
	     Evas_Object *obj;
	     
	     obj = l->data;
	     obj->mouse_grabbed = 0;
	     if (!evas_list_find(ins, obj))
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
	evas_event_feed_mouse_move_data(e, e->pointer.x, e->pointer.y, data);
     }
}

void
evas_event_feed_mouse_move_data(Evas *e, int x, int y, const void *data)
{
   int px, py;
   double pcx, pcy;
   
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
   e->pointer.canvas_y = evas_coord_screen_x_to_world(e, y);
   /* if our mouse button is grabbed to any objects */   
   if (e->pointer.mouse_grabbed)
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
		 (!obj->pass_events) &&
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
		 (!obj->pass_events) &&
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
	/* go thru out current list of ins */
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
	e->pointer.object.in = evas_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
     }
}

void
evas_event_feed_mouse_in_data(Evas *e, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 1;
}

void
evas_event_feed_mouse_out_data(Evas *e, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 0;
}

void
evas_event_feed_key_down_data(Evas *e, const char *keyname, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
     {
	Evas_Event_Key_Down ev;
	Evas_Object *focused_obj;

	focused_obj = e->focused;
	ev.keyname = (char *)keyname;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	if (e->grabs)
	  {
	     Evas_List *l;
	     
	     for (l = e->grabs; l; l= l->next)
	       {
		  Evas_Key_Grab *g;
		  
		  g = l->data;
		  if (((e->modifiers.mask & g->modifiers) ||
		       (g->modifiers == e->modifiers.mask)) &&
		      (!((e->modifiers.mask & g->not_modifiers) ||
			 (g->not_modifiers == ~e->modifiers.mask))) &&
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (!e->events_frozen)
			 evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_DOWN, &ev);
		       if (g->exclusive) return;
		    }		      
	       }
	  }
	if (focused_obj)
	  {
	     if (!e->events_frozen)
	       evas_object_event_callback_call(focused_obj, EVAS_CALLBACK_KEY_DOWN, &ev);
	  }
     }
}

void
evas_event_feed_key_up_data(Evas *e, const char *keyname, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
     {
	Evas_Event_Key_Up ev;
	Evas_Object *focused_obj;

	focused_obj = e->focused;
	ev.keyname = (char *)keyname;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	if (e->grabs)
	  {
	     Evas_List *l;
	     
	     for (l = e->grabs; l; l= l->next)
	       {
		  Evas_Key_Grab *g;
		  
		  g = l->data;
		  if (((e->modifiers.mask & g->modifiers) ||
		       (g->modifiers == e->modifiers.mask)) &&
		      (!((e->modifiers.mask & g->not_modifiers) ||
			 (g->not_modifiers == ~e->modifiers.mask))) &&
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (!e->events_frozen)
			 evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_UP, &ev);
		       if (g->exclusive) return;
		    }		      
	       }
	  }
	if (focused_obj)
	  {
	     if (!e->events_frozen)
	       evas_object_event_callback_call(focused_obj, EVAS_CALLBACK_KEY_UP, &ev);
	  }
     }
}

void
evas_event_feed_mouse_down(Evas *e, int b)
{
   evas_event_feed_mouse_down_data(e, b, NULL);
}

void
evas_event_feed_mouse_up(Evas *e, int b)
{
   evas_event_feed_mouse_up_data(e, b, NULL);
}

void
evas_event_feed_mouse_move(Evas *e, int x, int y)
{
   evas_event_feed_mouse_move_data(e, x, y, NULL);
}

void
evas_event_feed_mouse_in(Evas *e)
{
   evas_event_feed_mouse_in_data(e, NULL);
}

void
evas_event_feed_mouse_out(Evas *e)
{
   evas_event_feed_mouse_out_data(e, NULL);
}

void
evas_event_feed_key_down(Evas *e, const char *keyname)
{
   evas_event_feed_key_down_data(e, keyname, NULL);
}

void
evas_event_feed_key_up(Evas *e, const char *keyname)
{
   evas_event_feed_key_up_data(e, keyname, NULL);
}


void
evas_object_pass_events_set(Evas_Object *obj, int pass)
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
				obj->layer->evas->pointer.y);
}

int
evas_object_pass_events_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pass_events;
}

void
evas_object_repeat_events_set(Evas_Object *obj, int repeat)
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
				obj->layer->evas->pointer.y);
}

int
evas_object_repeat_events_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->repeat_events;
}
