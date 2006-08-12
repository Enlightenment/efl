#include "evas_common.h"
#include "evas_private.h"

static Evas_Object_List* get_layer_objects_last( Evas_Layer* l )
{
   if( !l || !l->objects )
	return NULL;

   return ((Evas_Object_List *)(l->objects))->last;
}

/* evas internal stuff */
Evas_Object *
evas_object_new(void)
{
   Evas_Object *obj;

   obj = calloc(1, sizeof(Evas_Object));
   if (!obj) return NULL;

   obj->magic = MAGIC_OBJ;

   return obj;
}

void
evas_object_free(Evas_Object *obj, int clean_layer)
{
   int was_smart_child = 0;

   evas_object_grabs_cleanup(obj);
   evas_object_intercept_cleanup(obj);
   if (obj->smart.parent) was_smart_child = 1;
   evas_object_smart_cleanup(obj);
   obj->func->free(obj);
   if (!was_smart_child) evas_object_release(obj, clean_layer);
   if (obj->clip.clipees)
     evas_list_free(obj->clip.clipees);
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   evas_object_event_callback_cleanup(obj);
   while (obj->data.elements)
     {
	Evas_Data_Node *node;

	node = obj->data.elements->data;
	obj->data.elements = evas_list_remove(obj->data.elements, node);
	free(node);
     }
   obj->magic = 0;
   free(obj);
}

void
evas_object_change(Evas_Object *obj)
{
   Evas_List *l;

   obj->layer->evas->changed = 1;
   if (obj->changed) return;
   obj->changed = 1;
   /* set changed flag on all objects this one clips too */
   for (l = obj->clip.clipees; l; l = l->next)
     {
	Evas_Object *o;

	o = (Evas_Object *)l->data;
	evas_object_change(o);
     }
   if (obj->smart.parent) evas_object_change(obj->smart.parent);
}

Evas_List *
evas_object_render_pre_visible_change(Evas_List *updates, Evas_Object *obj, int is_v, int was_v)
{
   Evas_Rectangle *r;

   if (obj->smart.smart) return updates;
   if (is_v == was_v) return updates;
   if (is_v)
     {
	NEW_RECT(r,
		 obj->cur.cache.clip.x,
		 obj->cur.cache.clip.y,
		 obj->cur.cache.clip.w,
		 obj->cur.cache.clip.h);
	if (r) updates = evas_list_append(updates, r);
     }
   else
     {
	NEW_RECT(r,
		 obj->prev.cache.clip.x,
		 obj->prev.cache.clip.y,
		 obj->prev.cache.clip.w,
		 obj->prev.cache.clip.h);
	if (r) updates = evas_list_append(updates, r);
     }
   return updates;
}

Evas_List *
evas_object_render_pre_clipper_change(Evas_List *updates, Evas_Object *obj)
{
   Evas_Rectangle *r;
   Evas_List *rl;

   if (obj->smart.smart) return updates;
   if (obj->cur.clipper == obj->prev.clipper) return updates;
   if ((obj->cur.clipper) && (obj->prev.clipper))
     {
	/* get difference rects between clippers */
	rl = evas_rects_return_difference_rects(obj->cur.clipper->cur.cache.clip.x,
						obj->cur.clipper->cur.cache.clip.y,
						obj->cur.clipper->cur.cache.clip.w,
						obj->cur.clipper->cur.cache.clip.h,
						obj->prev.clipper->prev.cache.clip.x,
						obj->prev.clipper->prev.cache.clip.y,
						obj->prev.clipper->prev.cache.clip.w,
						obj->prev.clipper->prev.cache.clip.h);
	/* go thru every difference rect */
	while (rl)
	  {
	     r = rl->data;
	     rl = evas_list_remove(rl, r);
	     updates = evas_list_append(updates, r);
	  }
     }
   else if (obj->cur.clipper)
     {
	rl = evas_rects_return_difference_rects(obj->cur.cache.geometry.x,
						obj->cur.cache.geometry.y,
						obj->cur.cache.geometry.w,
						obj->cur.cache.geometry.h,
						obj->cur.clipper->cur.cache.clip.x,
						obj->cur.clipper->cur.cache.clip.y,
						obj->cur.clipper->cur.cache.clip.w,
						obj->cur.clipper->cur.cache.clip.h);
	while (rl)
	  {
	     r = rl->data;
	     rl = evas_list_remove(rl, r);
	     updates = evas_list_append(updates, r);
	  }
     }
   else if (obj->prev.clipper)
     {
	rl = evas_rects_return_difference_rects(obj->prev.cache.geometry.x,
						obj->prev.cache.geometry.y,
						obj->prev.cache.geometry.w,
						obj->prev.cache.geometry.h,
						obj->prev.clipper->prev.cache.clip.x,
						obj->prev.clipper->prev.cache.clip.y,
						obj->prev.clipper->prev.cache.clip.w,
						obj->prev.clipper->prev.cache.clip.h);
	while (rl)
	  {
	     r = rl->data;
	     rl = evas_list_remove(rl, r);
	     updates = evas_list_append(updates, r);
	  }
     }
   return updates;
}

Evas_List *
evas_object_render_pre_prev_cur_add(Evas_List *updates, Evas_Object *obj)
{
   Evas_Rectangle *r;

   NEW_RECT(r,
	    obj->cur.cache.geometry.x,
	    obj->cur.cache.geometry.y,
	    obj->cur.cache.geometry.w,
	    obj->cur.cache.geometry.h);
   if (r) updates = evas_list_append(updates, r);
   NEW_RECT(r,
	    obj->prev.cache.geometry.x,
	    obj->prev.cache.geometry.y,
	    obj->prev.cache.geometry.w,
	    obj->prev.cache.geometry.h);
   if (r) updates = evas_list_append(updates, r);
   return updates;
}

void
evas_object_render_pre_effect_updates(Evas_List *updates, Evas_Object *obj, int is_v, int was_v)
{
   Evas_Rectangle *r;
   int x, y, w, h;
   Evas_Object *clipper;
   Evas_List *l;

   if (obj->smart.smart) return;
   /* FIXME: was_v isn't used... why? */
   was_v = 0;
   if (!obj->clip.clipees)
     {
	while (updates)
	  {
	     r = (Evas_Rectangle *)(updates->data);
	     updates = evas_list_remove(updates, r);
	     /* get updates and clip to current clip */
	     x = r->x; y = r->y; w = r->w; h = r->h;
	     RECTS_CLIP_TO_RECT(x, y, w, h,
				obj->cur.cache.clip.x,
				obj->cur.cache.clip.y,
				obj->cur.cache.clip.w,
				obj->cur.cache.clip.h);
	     if ((w > 0) && (h > 0))
	       obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
								      x, y, w, h);
	     /* get updates and clip to previous clip */
	     x = r->x; y = r->y; w = r->w; h = r->h;
	     RECTS_CLIP_TO_RECT(x, y, w, h,
				obj->prev.cache.clip.x,
				obj->prev.cache.clip.y,
				obj->prev.cache.clip.w,
				obj->prev.cache.clip.h);
	     if ((w > 0) && (h > 0))
	       obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
								      x, y, w, h);
	     free(r);
	     r = NULL;
	  }
	/* if the object is actually visible, take any parent clip changes */
	if (is_v)
	  {
	     clipper = obj->cur.clipper;
	     while (clipper)
	       {
		  for (l = clipper->clip.changes; l; l = l->next)
		    {
		       r = (Evas_Rectangle *)(l->data);
		       /* get updates and clip to current clip */
		       x = r->x; y = r->y; w = r->w; h = r->h;
		       RECTS_CLIP_TO_RECT(x, y, w, h,
					  obj->cur.cache.clip.x,
					  obj->cur.cache.clip.y,
					  obj->cur.cache.clip.w,
					  obj->cur.cache.clip.h);
		       if ((w > 0) && (h > 0))
			 obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
										x, y, w, h);
		       /* get updates and clip to previous clip */
		       x = r->x; y = r->y; w = r->w; h = r->h;
		       RECTS_CLIP_TO_RECT(x, y, w, h,
					  obj->prev.cache.clip.x,
					  obj->prev.cache.clip.y,
					  obj->prev.cache.clip.w,
					  obj->prev.cache.clip.h);
		       if ((w > 0) && (h > 0))
			 obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
										x, y, w, h);
		    }
		  clipper = clipper->cur.clipper;
	       }
	  }
     }
   else
     {
	while (obj->clip.changes)
	  {
	     free(obj->clip.changes->data);
	     obj->clip.changes = evas_list_remove(obj->clip.changes, obj->clip.changes->data);
	  }
	obj->clip.changes = updates;
     }
}

void
evas_object_coords_recalc(Evas_Object *obj)
{
   if (obj->smart.smart) return;
   if (obj->cur.cache.geometry.validity == obj->layer->evas->output_validity)
     return;
   obj->cur.cache.geometry.x =
     evas_coord_world_x_to_screen(obj->layer->evas, obj->cur.geometry.x);
   obj->cur.cache.geometry.y =
     evas_coord_world_y_to_screen(obj->layer->evas, obj->cur.geometry.y);
   obj->cur.cache.geometry.w =
     evas_coord_world_x_to_screen(obj->layer->evas, obj->cur.geometry.w) -
     evas_coord_world_x_to_screen(obj->layer->evas, 0);
   obj->cur.cache.geometry.h =
     evas_coord_world_y_to_screen(obj->layer->evas, obj->cur.geometry.h) -
     evas_coord_world_y_to_screen(obj->layer->evas, 0);
   if (obj->func->coords_recalc) obj->func->coords_recalc(obj);
   obj->cur.cache.geometry.validity = obj->layer->evas->output_validity;
}

int
evas_object_is_active(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if ((evas_object_is_visible(obj) || evas_object_was_visible(obj)) &&
       (evas_object_is_in_output_rect(obj, 0, 0, obj->layer->evas->output.w,
				      obj->layer->evas->output.h) ||
	evas_object_was_in_output_rect(obj, 0, 0, obj->layer->evas->output.w,
				       obj->layer->evas->output.h)))
     return 1;
   return 0;
}

int
evas_object_is_in_output_rect(Evas_Object *obj, int x, int y, int w, int h)
{
   if (obj->smart.smart) return 0;
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
			obj->cur.cache.clip.x,
			obj->cur.cache.clip.y,
			obj->cur.cache.clip.w,
			obj->cur.cache.clip.h)))
     return 1;
   return 0;
}

int
evas_object_was_in_output_rect(Evas_Object *obj, int x, int y, int w, int h)
{
   if (obj->smart.smart) return 0;
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
			obj->prev.cache.clip.x,
			obj->prev.cache.clip.y,
			obj->prev.cache.clip.w,
			obj->prev.cache.clip.h)))
     return 1;
   return 0;
}

int
evas_object_is_visible(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if ((obj->cur.visible) &&
       (obj->cur.cache.clip.visible) &&
       (obj->cur.cache.clip.a > 0))
     {
	if (obj->func->is_visible)
	  return obj->func->is_visible(obj);
	return 1;
     }
   return 0;
}

int
evas_object_was_visible(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if ((obj->prev.visible) &&
       (obj->prev.cache.clip.visible) &&
       (obj->prev.cache.clip.a > 0))
     {
	if (obj->func->was_visible)
	  return obj->func->was_visible(obj);
	return 1;
     }
   return 0;
}

int
evas_object_is_opaque(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if (obj->cur.cache.clip.a == 255)
     {
	if (obj->func->is_opaque)
	  return obj->func->is_opaque(obj);
	return 1;
     }
   return 0;
}

int
evas_object_was_opaque(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if (obj->prev.cache.clip.a == 255)
     {
	if (obj->func->was_opaque)
	  return obj->func->was_opaque(obj);
	return 1;
     }
   return 0;
}

/* routines apps will call */

/**
 * @defgroup Evas_Object_Group Generic Object Functions
 *
 * Functions that manipulate generic evas objects.
 */

/**
 * Deletes the given evas object and frees its memory.
 *
 * The object's 'free' callback is called when this function is called.
 * If the object currently has the focus, its 'focus out' callback is
 * also called.
 *
 * @param   obj The given evas object.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_del(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;

   if (obj->name) evas_object_name_set(obj, NULL);
   if (!obj->layer)
     {
	evas_object_free(obj, 1);
	return;
     }
   if (obj->focused)
     {
	obj->focused = 0;
	obj->layer->evas->focused = NULL;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_OUT, NULL);
     }
   obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
   obj->mouse_grabbed = 0;
   evas_object_hide(obj);
   evas_object_event_callback_call(obj, EVAS_CALLBACK_FREE, NULL);
   evas_object_grabs_cleanup(obj);
   while (obj->clip.clipees) evas_object_clip_unset(obj->clip.clipees->data);
   if (obj->cur.clipper) evas_object_clip_unset(obj);
   if (obj->smart.smart) evas_object_smart_del(obj);
   evas_object_smart_cleanup(obj);
   obj->delete_me = 1;
   evas_object_change(obj);
}

/**
 * Moves the given evas object to the given location.
 * @param   obj The given evas object.
 * @param   x   X position to move the object to, in canvas units.
 * @param   y   Y position to move the object to, in canvas units.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   int is, was = 0, pass = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (evas_object_intercept_call_move(obj, x, y)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->move)
	  obj->smart.smart->smart_class->move(obj, x, y);
     }
   if ((obj->cur.geometry.x == x) &&
       (obj->cur.geometry.y == y))
     {
	evas_object_inform_call_move(obj);
	return;
     }
   if (obj->layer->evas->events_frozen <= 0)
     {
	pass = evas_event_passes_through(obj);
	if (!pass)
	  was = evas_object_is_in_output_rect(obj,
					      obj->layer->evas->pointer.x,
					      obj->layer->evas->pointer.y, 1, 1);
     }
   obj->cur.geometry.x = x;
   obj->cur.geometry.y = y;
   obj->cur.cache.geometry.validity = 0;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	evas_object_recalc_clippees(obj);
	if (!pass)
	  {
	     if (!obj->smart.smart)
	       {
		  is = evas_object_is_in_output_rect(obj,
						     obj->layer->evas->pointer.x,
						     obj->layer->evas->pointer.y, 1, 1);
		  if ((is ^ was) && obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_move(obj);
}

/**
 * Changes the size of the given evas object.
 * @param   obj The given evas object.
 * @param   w   The new width of the evas object.
 * @param   h   The new height of the evas object.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   int is, was = 0, pass = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (w < 0.0) w = 0.0; if (h < 0.0) h = 0.0;
   if (evas_object_intercept_call_resize(obj, w, h)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->resize)
	  obj->smart.smart->smart_class->resize(obj, w, h);
     }
   if ((obj->cur.geometry.w == w) &&
       (obj->cur.geometry.h == h))
     {
	evas_object_inform_call_resize(obj);
	return;
     }
   if (obj->layer->evas->events_frozen <= 0)
     {
	pass = evas_event_passes_through(obj);
	if (!pass)
	  was = evas_object_is_in_output_rect(obj,
					      obj->layer->evas->pointer.x,
					      obj->layer->evas->pointer.y, 1, 1);
     }
   obj->cur.geometry.w = w;
   obj->cur.geometry.h = h;
   obj->cur.cache.geometry.validity = 0;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_recalc_clippees(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	//   if (obj->func->coords_recalc) obj->func->coords_recalc(obj);
	if (!pass)
	  {
	     if (!obj->smart.smart)
	       {
		  is = evas_object_is_in_output_rect(obj,
						     obj->layer->evas->pointer.x,
						     obj->layer->evas->pointer.y, 1, 1);
		  if ((is ^ was) && (obj->cur.visible))
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_resize(obj);
}

/**
 * Retrieves the position and rectangular size of the given evas object.
 *
 * Note that if any of @p x, @p y, @p w or @p h are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param obj The given evas object.
 * @param   x   Pointer to an integer in which to store the X coordinate of
 *              the object.
 * @param   y   Pointer to an integer in which to store the Y coordinate of
 *              the object.
 * @param   w   Pointer to an integer in which to store the width of the
 *              object.
 * @param   h   Pointer to an integer in which to store the height of the
 *              object.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_geometry_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0; if (y) *y = 0; if (w) *w = 0; if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     {
	if (x) *x = 0; if (y) *y = 0; if (w) *w = 0; if (h) *h = 0;
	return;
     }
   if (x) *x = obj->cur.geometry.x;
   if (y) *y = obj->cur.geometry.y;
   if (w) *w = obj->cur.geometry.w;
   if (h) *h = obj->cur.geometry.h;
}

/**
 * @defgroup Evas_Object_Visibility_Group Generic Object Visibility Functions
 *
 * Functions that deal with the visibility of evas objects.
 */

/**
 * Makes the given evas object visible.
 * @param   obj The given evas object.
 * @ingroup Evas_Object_Visibility_Group
 */
EAPI void
evas_object_show(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (evas_object_intercept_call_show(obj)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->show)
	  obj->smart.smart->smart_class->show(obj);
     }
   if (obj->cur.visible)
     {
	evas_object_inform_call_show(obj);
	return;
     }
   obj->cur.visible = 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	evas_object_recalc_clippees(obj);
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1))
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_show(obj);
}

/**
 * Makes the given evas object invisible.
 * @param   obj The given evas object.
 * @ingroup Evas_Object_Visibility_Group
 */
EAPI void
evas_object_hide(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (evas_object_intercept_call_hide(obj)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->hide)
	  obj->smart.smart->smart_class->hide(obj);
     }
   if (!obj->cur.visible)
     {
	evas_object_inform_call_hide(obj);
	return;
     }
   obj->cur.visible = 0;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	evas_object_recalc_clippees(obj);
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1))
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
		  if (obj->delete_me) return;
		  if (obj->mouse_grabbed > 0)
		    {
//		       if (obj->layer->evas->pointer.mouse_grabbed >= obj->mouse_grabbed)
			 obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
		    }
		    {
		       if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
			 {
			    obj->layer->evas->pointer.object.in = evas_list_remove(obj->layer->evas->pointer.object.in, obj);
			 }
		       obj->mouse_grabbed = 0;
		       if (obj->layer->evas->events_frozen > 0)
			 {
			    obj->mouse_in = 0;
			    return;
			 }
		       if (obj->mouse_in)
			 {
			    Evas_Event_Mouse_Out ev;
			    
			    obj->mouse_in = 0;
			    ev.buttons = obj->layer->evas->pointer.button;
			    ev.output.x = obj->layer->evas->pointer.x;
			    ev.output.y = obj->layer->evas->pointer.y;
			    ev.canvas.x = obj->layer->evas->pointer.canvas_x;
			    ev.canvas.y = obj->layer->evas->pointer.canvas_y;
			    ev.data = NULL;
			    ev.modifiers = &(obj->layer->evas->modifiers);
			    ev.locks = &(obj->layer->evas->locks);
			    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
			 }
		    }
	       }
	  }
     }
   else
     {
	if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
	  obj->layer->evas->pointer.object.in = evas_list_remove(obj->layer->evas->pointer.object.in, obj);
	obj->mouse_grabbed = 0;
	obj->mouse_in = 0;
     }
   evas_object_inform_call_hide(obj);
}

/**
 * Retrieves whether or not the given evas object is visible.
 * @param   obj The given evas object.
 * @return  @c 1 if the object is visible.  @c 0 otherwise.
 * @ingroup Evas_Object_Visibility_Group
 */
EAPI Evas_Bool
evas_object_visible_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 0;
   return obj->cur.visible;
}

/**
 * Sets the general colour of the given evas object to the given colour.
 * @param obj The given evas object.
 * @param r   The red component of the given colour.
 * @param g   The green component of the given colour.
 * @param b   The blue component of the given colour.
 * @param a   The alpha component of the given colour.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (r > 255) r = 255; if (r < 0) r = 0;
   if (g > 255) g = 255; if (g < 0) g = 0;
   if (b > 255) b = 255; if (b < 0) b = 0;
   if (a > 255) a = 255; if (a < 0) a = 0;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->color_set)
	  obj->smart.smart->smart_class->color_set(obj, r, g, b, a);
     }
   if ((obj->cur.color.r == r) &&
       (obj->cur.color.g == g) &&
       (obj->cur.color.b == b) &&
       (obj->cur.color.a == a)) return;
   obj->cur.color.r = r;
   obj->cur.color.g = g;
   obj->cur.color.b = b;
   if ((obj->cur.color.a == 0) && (a == 0)) return;
   obj->cur.color.a = a;
   evas_object_change(obj);
}

/**
 * Retrieves the general colour of the given evas object.
 *
 * Note that if any of @p r, @p g, @p b or @p a are @c NULL, then the
 * @c NULL parameters are ignored.
 *
 * @param   obj The given evas object.
 * @param   r   Pointer to an integer in which to store the red component of
 *              the colour.
 * @param   g   Pointer to an integer in which to store the green component of
 *              the colour.
 * @param   b   Pointer to an integer in which to store the blue component of
 *              the colour.
 * @param   a   Pointer to an integer in which to store the alpha component of
 *              the colour.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_color_get(Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (r) *r = 0; if (g) *g = 0; if (b) *b = 0; if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     {
	if (r) *r = 0; if (g) *g = 0; if (b) *b = 0; if (a) *a = 0;
	return;
     }
   if (r) *r = obj->cur.color.r;
   if (g) *g = obj->cur.color.g;
   if (b) *b = obj->cur.color.b;
   if (a) *a = obj->cur.color.a;
}

/**
 * Sets whether or not the given evas object is to be drawn anti_aliased.
 * @param   obj The given evas object.
 * @param   anti_alias. 1 if the object is to be anti_aliased, 0 otherwise.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_anti_alias_set(Evas_Object *obj, Evas_Bool anti_alias)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (obj->cur.anti_alias == !!anti_alias)
   	return;
   obj->cur.anti_alias = !!anti_alias;
   evas_object_change(obj);
}


/**
 * Retrieves whether or not the given evas object is to be drawn anti_aliased.
 * @param   obj The given evas object.
 * @return  @c 1 if the object is to be anti_aliased.  @c 0 otherwise.
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Bool
evas_object_anti_alias_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 0;
   return obj->cur.anti_alias;
}

/**
 * Sets the color_space to be used for linear interpolation of colors.
 * @param   obj The given evas object.
 * @param   color_space, one of EVAS_COLOR_SPACE_ARGB or EVAS_COLOR_SPACE_AHSV.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_color_interpolation_set(Evas_Object *obj, int color_space)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (obj->cur.interpolation.color_space == color_space)
   	return;
   obj->cur.interpolation.color_space = color_space;
   evas_object_change(obj);
}


/**
 * Retrieves the current value of the color space used for linear interpolation.
 * @param   obj The given evas object.
 * @return  @c EVAS_COLOR_SPACE_ARGB or EVAS_COLOR_SPACE_AHSV.
 * @ingroup Evas_Object_Group
 */
EAPI int
evas_object_color_interpolation_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 0;
   return obj->cur.interpolation.color_space;
}

/**
 * Sets the render_op to be used for rendering the evas object.
 * @param   obj The given evas object.
 * @param   render_op one of the Evas_Render_Op values.
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_render_op_set(Evas_Object *obj, Evas_Render_Op render_op)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if ((Evas_Render_Op)obj->cur.render_op == render_op)
   	return;
   obj->cur.render_op = render_op;
   evas_object_change(obj);
}


/**
 * Retrieves the current value of the operation used for rendering the evas object.
 * @param   obj The given evas object.
 * @return  one of the enumerated values in Evas_Render_Op.
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Render_Op
evas_object_render_op_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return EVAS_RENDER_BLEND;
   return obj->cur.render_op;
}

/**
 * Retrieves the evas that the given evas object is on.
 * @param   obj The given evas object.
 * @return  The evas that the object is on.
 * @ingroup Evas_Object_Group
 */
EAPI Evas *
evas_object_evas_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 0;
   return obj->layer->evas;
}

/**
 * @defgroup Evas_Object_Finders Object Finder Functions
 *
 * Functions that determine what evas objects are at a given location
 * or within a given region of an evas.
 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * @ingroup Evas_Object_Finders
 */
EAPI Evas_Object *
evas_object_top_at_xy_get(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects)
{
   Evas_Object_List *l;
   int xx, yy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = evas_coord_world_x_to_screen(e, x);
   yy = evas_coord_world_y_to_screen(e, y);
   for (l = ((Evas_Object_List *)(e->layers))->last; l; l = l->prev)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;

	lay = (Evas_Layer *)l;
	for (l2 = get_layer_objects_last(lay); l2; l2 = l2->prev)
	  {
	     Evas_Object *obj;

	     obj = (Evas_Object *)l2;
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, 1, 1)) &&
		 (!obj->clip.clipees))
	       return obj;
	  }
     }
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * @ingroup Evas_Object_Finders
 */
EAPI Evas_Object *
evas_object_top_at_pointer_get(Evas *e)
{
   return evas_object_top_at_xy_get(e, e->pointer.canvas_x, e->pointer.canvas_y, 0, 0);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * @ingroup Evas_Object_Finders
 */
EAPI Evas_Object *
evas_object_top_in_rectangle_get(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects)
{
   Evas_Object_List *l;
   int xx, yy, ww, hh;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = evas_coord_world_x_to_screen(e, x);
   yy = evas_coord_world_y_to_screen(e, y);
   ww = evas_coord_world_x_to_screen(e, w);
   hh = evas_coord_world_y_to_screen(e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   for (l = ((Evas_Object_List *)(e->layers))->last; l; l = l->prev)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;

	lay = (Evas_Layer *)l;
	for (l2 = get_layer_objects_last(lay); l2; l2 = l2->prev)
	  {
	     Evas_Object *obj;

	     obj = (Evas_Object *)l2;
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, ww, hh)) &&
		 (!obj->clip.clipees))
	       return obj;
	  }
     }
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * @ingroup Evas_Object_Finders
 */
EAPI Evas_List *
evas_objects_at_xy_get(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects)
{
   Evas_List *in = NULL;
   Evas_Object_List *l;
   int xx, yy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = evas_coord_world_x_to_screen(e, x);
   yy = evas_coord_world_y_to_screen(e, y);
   for (l = ((Evas_Object_List *)(e->layers))->last; l; l = l->prev)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;

	lay = (Evas_Layer *)l;
	for (l2 = get_layer_objects_last(lay); l2; l2 = l2->prev)
	  {
	     Evas_Object *obj;

	     obj = (Evas_Object *)l2;
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, 1, 1)) &&
		 (!obj->clip.clipees))
	       in = evas_list_prepend(in, obj);
	  }
     }
   return in;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * @ingroup Evas_Object_Finders
 */
EAPI Evas_List *
evas_objects_in_rectangle_get(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects)
{
   Evas_List *in = NULL;
   Evas_Object_List *l;
   int xx, yy, ww, hh;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = evas_coord_world_x_to_screen(e, x);
   yy = evas_coord_world_y_to_screen(e, y);
   ww = evas_coord_world_x_to_screen(e, w);
   hh = evas_coord_world_y_to_screen(e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   for (l = ((Evas_Object_List *)(e->layers))->last; l; l = l->prev)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;

	lay = (Evas_Layer *)l;
	for (l2 = get_layer_objects_last(lay); l2; l2 = l2->prev)
	  {
	     Evas_Object *obj;

	     obj = (Evas_Object *)l2;
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, ww, hh)) &&
		 (!obj->clip.clipees))
	       in = evas_list_prepend(in, obj);
	  }
     }
   return in;
}

/**
 * Retrieves the name of the type of the given evas object.
 * @param   obj The given object.
 * @return  The name.
 * @ingroup Evas_Object_Group
 */
EAPI const char *
evas_object_type_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->delete_me) return "";
   return obj->type;
}
