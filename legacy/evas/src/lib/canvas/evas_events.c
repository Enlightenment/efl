#include "evas_common.h"
#include "evas_private.h"

static Evas_List *
_evas_event_object_list_in_get(Evas *e, Evas_List *in, const Evas_Object_List *list, Evas_Object *stop, int x, int y, int *no_rep)
{
   const Evas_Object_List *l;

   if (!list) return in;
   for (l = list->last; l; l = l->prev)
     {
	Evas_Object *obj;

	obj = (Evas_Object *)l;
	if (obj == stop)
	  {
	     *no_rep = 1;
	     return in;
	  }
	if (!evas_event_passes_through(obj))
	  {
	     if ((obj->cur.visible) && (obj->delete_me == 0) &&
		 (!obj->clip.clipees) &&
		 (evas_object_clippers_is_visible(obj)))
	       {
		  if (obj->smart.smart)
		    {
		       int norep;

		       norep = 0;
		       in = _evas_event_object_list_in_get(e, in,
							   evas_object_smart_members_get_direct(obj),
							   stop, x, y, &norep);
		       if (norep)
			 {
			    *no_rep = 1;
			    return in;
			 }
		    }
		  else
		    {
		       if (evas_object_is_in_output_rect(obj, x, y, 1, 1) &&
		           ((!obj->precise_is_inside) ||
			    (evas_object_is_inside(obj, x, y))))
			 {
			    in = evas_list_append(in, obj);
			    if (!obj->repeat_events)
			      {
				 *no_rep = 1;
				 return in;
			      }
			 }
		    }
	       }
	  }
     }
   *no_rep = 0;
   return in;
}

Evas_List *
evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y)
{
   Evas_Object_List *l;
   Evas_List *in = NULL;

   if (!e->layers) return NULL;
   for (l = ((Evas_Object_List *)(e->layers))->last; l; l = l->prev)
     {
	Evas_Layer *lay;
	int norep;

	lay = (Evas_Layer *)l;
	norep = 0;
	in = _evas_event_object_list_in_get(e, in, (Evas_Object_List *)lay->objects, stop,
					    x, y, &norep);
	if (norep) return in;
     }
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
 * Freeze all event processing
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
EAPI void
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
EAPI void
evas_event_thaw(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen--;
   if (e->events_frozen == 0)
     {
	Evas_Object_List *l;

	for (l = (Evas_Object_List *)e->layers; l; l = l->next)
	  {
	     Evas_Object_List *l2;
	     Evas_Layer *lay;

	     lay = (Evas_Layer *)l;
	     for (l2 = (Evas_Object_List *)lay->objects; l2; l2 = l2->next)
	       {
		  Evas_Object *obj;

		  obj = (Evas_Object *)l2;
		  evas_object_clip_recalc(obj);
		  evas_object_recalc_clippees(obj);
	       }
	  }
     }
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
EAPI int
evas_event_freeze_get(const Evas *e)
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
EAPI void
evas_event_feed_mouse_down(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Evas_List *l, *copy;
   Evas_Event_Mouse_Down ev;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((b < 1) || (b > 32)) return;

   e->pointer.button |= (1 << (b - 1));

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   ev.button = b;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   for (l = copy; l; l = l->next)
     {
	Evas_Object *obj;

	obj = l->data;
	if (obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
	  {
	     obj->mouse_grabbed++;
	     e->pointer.mouse_grabbed++;
	  }

	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_DOWN, &ev);
	if (e->delete_me) break;
     }
   if (copy) copy = evas_list_free(copy);
   e->last_mouse_down_counter++;
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_mouse_up(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Evas_List *l, *copy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((b < 1) || (b > 32)) return;

   e->pointer.button &= ~(1 << (b - 1));

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

     {
	Evas_Event_Mouse_Up ev;

	ev.button = b;
	ev.output.x = e->pointer.x;
	ev.output.y = e->pointer.y;
	ev.canvas.x = e->pointer.x;
	ev.canvas.y = e->pointer.y;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.flags = flags;
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;

	_evas_walk(e);
	copy = evas_event_list_copy(e->pointer.object.in);
	for (l = copy; l; l = l->next)
	  {
	     Evas_Object *obj;

	     obj = l->data;
	     if ((obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB) &&
		 (obj->mouse_in) && (obj->mouse_grabbed > 0))
	       {
		  obj->mouse_grabbed--;
		  e->pointer.mouse_grabbed--;
	       }
	     if (e->events_frozen <= 0)
	       evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_UP, &ev);
	     if (e->delete_me) break;
	  }
	if (copy) copy = evas_list_free(copy);
	e->last_mouse_up_counter++;
     }

   if (!e->pointer.button)
     {
	Evas_List *ins;
	Evas_List *l;

	  {
	     Evas_Event_Mouse_Out ev;

	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.x;
	     ev.canvas.y = e->pointer.y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;

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

		       obj->mouse_in = 0;
		       if (e->events_frozen <= 0)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
		    }
		  if (e->delete_me) break;
	       }
	  }
	if (copy) copy = evas_list_free(copy);
	if (e->pointer.inside)
	  {
	     Evas_Event_Mouse_In ev;

	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.x;
	     ev.canvas.y = e->pointer.y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;

	     for (l = ins; l; l = l->next)
	       {
		  Evas_Object *obj;

		  obj = l->data;

		  if (!evas_list_find(e->pointer.object.in, obj))
		    {

		       obj->mouse_in = 1;
		       if (e->events_frozen <= 0)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
		    }
		  if (e->delete_me) break;
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
	  evas_event_feed_mouse_move(e, e->pointer.x, e->pointer.y, timestamp, data);
     }

   if (e->pointer.mouse_grabbed < 0)
     fprintf(stderr, "BUG? e->pointer.mouse_grabbed (=%d) < 0!\n",
	     e->pointer.mouse_grabbed);

   if ((e->pointer.button == 0) && (e->pointer.mouse_grabbed))
     {
	e->pointer.mouse_grabbed = 0;
     }
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_mouse_cancel(Evas *e, unsigned int timestamp, const void *data)
{
   int i;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;

   _evas_walk(e);
   for (i = 0; i < 32; i++)
     {
	if ((e->pointer.button & (1 << i)))
	  evas_event_feed_mouse_up(e, i + 1, 0, timestamp, data);
     }
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_mouse_wheel(Evas *e, int direction, int z, unsigned int timestamp, const void *data)
{
   Evas_List *l, *copy;
   Evas_Event_Mouse_Wheel ev;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   ev.direction = direction;
   ev.z = z;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *) data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);

   for (l = copy; l; l = l->next)
     {
	Evas_Object *obj = l->data;

	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_WHEEL, &ev);
	if (e->delete_me) break;
     }
   if (copy) copy = evas_list_free(copy);

   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_mouse_move(Evas *e, int x, int y, unsigned int timestamp, const void *data)
{
   int px, py;
////   Evas_Coord pcx, pcy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   px = e->pointer.x;
   py = e->pointer.y;
////   pcx = e->pointer.canvas_x;
////   pcy = e->pointer.canvas_y;

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   e->pointer.x = x;
   e->pointer.y = y;
////   e->pointer.canvas_x = x;
////   e->pointer.canvas_y = y;
////   e->pointer.canvas_x = evas_coord_screen_x_to_world(e, x);
////   e->pointer.canvas_y = evas_coord_screen_y_to_world(e, y);
   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;
   _evas_walk(e);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
	/* go thru old list of in objects */
	Evas_List *outs = NULL;
	Evas_List *l, *copy;

	  {
	     Evas_Event_Mouse_Move ev;

	     ev.buttons = e->pointer.button;
	     ev.cur.output.x = e->pointer.x;
	     ev.cur.output.y = e->pointer.y;
	     ev.cur.canvas.x = e->pointer.x;
	     ev.cur.canvas.y = e->pointer.y;
	     ev.prev.output.x = px;
	     ev.prev.output.y = py;
	     ev.prev.canvas.x = px;
	     ev.prev.canvas.y = py;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;
	     copy = evas_event_list_copy(e->pointer.object.in);
	     for (l = copy; l; l = l->next)
	       {
		  Evas_Object *obj;

		  obj = l->data;
		  if ((obj->cur.visible) &&
		      (evas_object_clippers_is_visible(obj)) &&
		      (!evas_event_passes_through(obj)) &&
		      (!obj->clip.clipees))
		    {
		       if ((px != x) || (py != y))
			 {
			    if (e->events_frozen <= 0)
			      evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_MOVE, &ev);
			 }
		    }
		  else
		    outs = evas_list_append(outs, obj);
		  if (e->delete_me) break;
	       }
	  }
	  {
	     Evas_Event_Mouse_Out ev;

	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.x;
	     ev.canvas.y = e->pointer.y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;

	     if (copy) copy = evas_list_free(copy);
	     while (outs)
	       {
		  Evas_Object *obj;

		  obj = outs->data;
		  outs = evas_list_remove(outs, obj);
		  if ((!obj->mouse_grabbed) && (!e->delete_me))
		    {
		       e->pointer.object.in = evas_list_remove(e->pointer.object.in, obj);
			 {
			    obj->mouse_in = 0;
			    if (e->events_frozen <= 0)
			      evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
			 }
		    }
	       }
	  }
     }
   else
     {
	Evas_List *ins;
	Evas_List *l, *copy;
	Evas_Event_Mouse_Move ev;
	Evas_Event_Mouse_Out ev2;
	Evas_Event_Mouse_In ev3;

	ev.buttons = e->pointer.button;
	ev.cur.output.x = e->pointer.x;
	ev.cur.output.y = e->pointer.y;
	ev.cur.canvas.x = e->pointer.x;
	ev.cur.canvas.y = e->pointer.y;
	ev.prev.output.x = px;
	ev.prev.output.y = py;
	ev.prev.canvas.x = px;
	ev.prev.canvas.y = py;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;

	ev2.buttons = e->pointer.button;
	ev2.output.x = e->pointer.x;
	ev2.output.y = e->pointer.y;
	ev2.canvas.x = e->pointer.x;
	ev2.canvas.y = e->pointer.y;
	ev2.data = (void *)data;
	ev2.modifiers = &(e->modifiers);
	ev2.locks = &(e->locks);
	ev2.timestamp = timestamp;
	ev2.event_flags = EVAS_EVENT_FLAG_NONE;

	ev3.buttons = e->pointer.button;
	ev3.output.x = e->pointer.x;
	ev3.output.y = e->pointer.y;
	ev3.canvas.x = e->pointer.x;
	ev3.canvas.y = e->pointer.y;
	ev3.data = (void *)data;
	ev3.modifiers = &(e->modifiers);
	ev3.locks = &(e->locks);
	ev3.timestamp = timestamp;
	ev3.event_flags = EVAS_EVENT_FLAG_NONE;

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
// FIXME: i don't think we need this
//	     evas_object_clip_recalc(obj);
	     if (evas_object_is_in_output_rect(obj, x, y, 1, 1) &&
		 (obj->cur.visible) &&
		 (evas_object_clippers_is_visible(obj)) &&
		 (evas_list_find(ins, obj)) &&
		 (!evas_event_passes_through(obj)) &&
		 (!obj->clip.clipees) &&
		 ((!obj->precise_is_inside) ||
		  (evas_object_is_inside(obj, x, y))))
	       {
		  if ((px != x) || (py != y))
		    {
		       if (e->events_frozen <= 0)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_MOVE, &ev);
		    }
	       }
	     /* otherwise it has left the object */
	     else
	       {
		  obj->mouse_in = 0;
		  if (e->events_frozen <= 0)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev2);
	       }
	     if (e->delete_me) break;
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
		  obj->mouse_in = 1;

		  if (e->events_frozen <= 0)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev3);
	       }
	     if (e->delete_me) break;
	  }
	/* free our old list of ins */
	evas_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
     }
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_mouse_in(Evas *e, unsigned int timestamp, const void *data)
{
   Evas_List *ins;
   Evas_List *l;
   Evas_Event_Mouse_In ev;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 1;

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   if (e->pointer.mouse_grabbed != 0) return;

   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   /* get new list of ins */
   ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
   for (l = ins; l; l = l->next)
     {
	Evas_Object *obj;

	obj = l->data;

	if (!evas_list_find(e->pointer.object.in, obj))
	  {

	     obj->mouse_in = 1;
	     if (e->events_frozen <= 0)
	       evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
	  }
	if (e->delete_me) break;
     }
   /* free our old list of ins */
   e->pointer.object.in = evas_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   evas_event_feed_mouse_move(e, e->pointer.x, e->pointer.y, timestamp, data);
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_mouse_out(Evas *e, unsigned int timestamp, const void *data)
{
   Evas_Event_Mouse_Out ev;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 0;

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
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

		  obj->mouse_in = 0;
		  if (e->events_frozen <= 0)
		    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
	       }
	     if (e->delete_me) break;
	  }
	if (copy) copy = evas_list_free(copy);
	/* free our old list of ins */
	e->pointer.object.in =  evas_list_free(e->pointer.object.in);
     }
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_key_down(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;
   _evas_walk(e);
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
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;
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
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (!(e->modifiers.mask & g->not_modifiers))
			 {
			    if (e->events_frozen <= 0)
			      evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_DOWN, &ev);
			    if (g->exclusive) exclusive = 1;
			 }
		    }
		  if (e->delete_me) break;
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
	     if (e->events_frozen <= 0)
	       evas_object_event_callback_call(e->focused, EVAS_CALLBACK_KEY_DOWN, &ev);
	  }
     }
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_key_up(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;
   _evas_walk(e);
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
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;
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
		       if (e->events_frozen <= 0)
			 evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_UP, &ev);
		       if (g->exclusive) exclusive = 1;
		    }
		  if (e->delete_me) break;
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
	     if (e->events_frozen <= 0)
	       evas_object_event_callback_call(e->focused, EVAS_CALLBACK_KEY_UP, &ev);
	  }
     }
   _evas_unwalk(e);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_event_feed_hold(Evas *e, int hold, unsigned int timestamp, const void *data)
{
   Evas_List *l, *copy;
   Evas_Event_Hold ev;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   ev.hold = hold;
   ev.data = (void *)data;
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   for (l = copy; l; l = l->next)
     {
	Evas_Object *obj;

	obj = l->data;
	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_HOLD, &ev);
	if (e->delete_me) break;
     }
   if (copy) copy = evas_list_free(copy);
   _evas_unwalk(e);
}

/**
 * @defgroup Evas_Object_Event_Flags_Group Evas Object Event Flag Functions
 *
 * Functions that deal with how events on an Evas Object are processed.
 */

/**
 * Set an object's pass events state.
 * @param obj the evas object
 * @param pass whether to pass events or not
 *
 * If @p pass is true, this will cause events on @p obj to be ignored.
 * They will be triggered on the next lower object (that is not set to
 * pass events) instead.
 *
 * If @p pass is false, events will be processed as normal.
 *
 * @ingroup Evas_Object_Event_Flags_Group
 */
EAPI void
evas_object_pass_events_set(Evas_Object *obj, Evas_Bool pass)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->pass_events = pass;
   evas_object_smart_member_cache_invalidate(obj);
   if (evas_object_is_in_output_rect(obj,
				     obj->layer->evas->pointer.x,
				     obj->layer->evas->pointer.y, 1, 1) &&
       ((!obj->precise_is_inside) ||
	(evas_object_is_inside(obj,
                               obj->layer->evas->pointer.x,
                               obj->layer->evas->pointer.y))))
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
}

/**
 * Determine whether an object is set to pass events.
 * @param obj
 * @return pass events state
 *
 * @ingroup Evas_Object_Event_Flags_Group
 */
EAPI Evas_Bool
evas_object_pass_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pass_events;
}

/**
 * Set an object's repeat events state.
 * @param obj the object
 * @param repeat wheter to repeat events or not
 *
 * If @p repeat is true, this will cause events on @p obj to trigger
 * callbacks, but also to be repeated on the next lower object in the
 * stack.
 *
 * If @p repeat is false, events occuring on @p obj will be processed
 * normally.
 *
 * @ingroup Evas_Object_Event_Flags_Group
 */
EAPI void
evas_object_repeat_events_set(Evas_Object *obj, Evas_Bool repeat)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->repeat_events = repeat;
   if (evas_object_is_in_output_rect(obj,
				     obj->layer->evas->pointer.x,
				     obj->layer->evas->pointer.y, 1, 1) &&
       ((!obj->precise_is_inside) ||
	(evas_object_is_inside(obj,
                               obj->layer->evas->pointer.x,
                               obj->layer->evas->pointer.y))))
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
}

/**
 * Determine whether an object is set to repeat events.
 * @param obj
 * @return repeat events state
 *
 * @ingroup Evas_Object_Event_Flags_Group
 */
EAPI Evas_Bool
evas_object_repeat_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->repeat_events;
}

/**
 * Set whether events on a smart member object should propagate to its parent.
 *
 * @param obj the smart member object
 * @param prop wheter to propagate events or not
 *
 * This function has no effect if @p obj is not a member of a smart
 * object.
 *
 * If @p prop is true, events occuring on this object will propagate on
 * to the smart object of which @p obj is a member.
 *
 * If @p prop is false, events for which callbacks are set on the member
 * object, @p obj, will not be passed on to the parent smart object.
 *
 * The default value is true.
 * @ingroup Evas_Object_Event_Flags_Group
 */
EAPI void
evas_object_propagate_events_set(Evas_Object *obj, Evas_Bool prop)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->no_propagate = !prop;
}

/**
 * Determine whether an object is set to propagate events.
 * @param obj
 * @return propogate events state
 *
 * @ingroup Evas_Object_Event_Flags_Group
 */
EAPI Evas_Bool
evas_object_propagate_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return !(obj->no_propagate);
}

/**
 * Set pointer behavior.
 *
 * @param obj
 * @param setting desired behavior.
 *
 * This function has direct effect on event callbacks related to mouse.
 *
 * If @p setting is EVAS_OBJECT_POINTER_MODE_AUTOGRAB, then when mouse is
 * down at this object, events will be restricted to it as source, mouse
 * moves, for example, will be emitted even if outside this object area.
 *
 * If @p setting is EVAS_OBJECT_POINTER_MODE_NOGRAB, then events will be
 * emitted just when inside this object area.
 *
 * The default value is EVAS_OBJECT_POINTER_MODE_AUTOGRAB.
 */
EAPI void
evas_object_pointer_mode_set(Evas_Object *obj, Evas_Object_Pointer_Mode setting)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->pointer_mode = setting;
}

/**
 * Determine how pointer will behave.
 * @param obj
 * @return pointer behavior.
 */
EAPI Evas_Object_Pointer_Mode
evas_object_pointer_mode_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pointer_mode;
}
