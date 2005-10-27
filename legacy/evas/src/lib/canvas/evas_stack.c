#include "evas_common.h"
#include "evas_private.h"

static Evas_Object *evas_object_above_get_internal(Evas_Object *obj);
static Evas_Object *evas_object_below_get_internal(Evas_Object *obj);

static Evas_Object *
evas_object_above_get_internal(Evas_Object *obj)
{
   if (((Evas_Object_List *)obj)->next)
     return (Evas_Object *)(((Evas_Object_List *)obj)->next);
   else
     {
	if (((Evas_Object_List *)(((Evas_Object *)obj)->layer))->next)
	  {
	     Evas_Layer *l;

	     l = (Evas_Layer *)(((Evas_Object_List *)(((Evas_Object *)obj)->layer))->next);
	     return l->objects;
	  }
     }
   return NULL;
}

static Evas_Object *
evas_object_below_get_internal(Evas_Object *obj)
{
   if (((Evas_Object_List *)obj)->prev)
     return (Evas_Object *)(((Evas_Object_List *)obj)->prev);
   else
     {
        if (((Evas_Object_List *)(((Evas_Object *)obj)->layer))->prev)
	  {
	     Evas_Layer *l;

	     l = (Evas_Layer *)(((Evas_Object_List *)(((Evas_Object *)obj)->layer))->prev);
	     return (Evas_Object *)(((Evas_Object_List *)(l->objects))->last);
	  }
     }
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
void
evas_object_raise(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_raise(obj)) return;
   if (!(((Evas_Object_List *)obj)->next))
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->smart.parent)
     {
	obj->smart.parent->smart.contained = evas_object_list_remove(obj->smart.parent->smart.contained, obj);
	obj->smart.parent->smart.contained = evas_object_list_append(obj->smart.parent->smart.contained, obj);
     }
   else
     {
	if (obj->in_layer)
	  {
	     obj->layer->objects = evas_object_list_remove(obj->layer->objects, obj);
	     obj->layer->objects = evas_object_list_append(obj->layer->objects, obj);
	  }
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   obj->restack = 1;
   evas_object_change(obj);
   evas_object_inform_call_restack(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1) &&
		      obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
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
evas_object_lower(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_lower(obj)) return;
   if (!(((Evas_Object_List *)obj)->prev))
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->smart.parent)
     {
	obj->smart.parent->smart.contained = evas_object_list_remove(obj->smart.parent->smart.contained, obj);
	obj->smart.parent->smart.contained = evas_object_list_prepend(obj->smart.parent->smart.contained, obj);
     }
   else
     {
	if (obj->in_layer)
	  {
	     obj->layer->objects = evas_object_list_remove(obj->layer->objects, obj);
	     obj->layer->objects = evas_object_list_prepend(obj->layer->objects, obj);
	  }
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   obj->restack = 1;
   evas_object_change(obj);
   evas_object_inform_call_restack(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1) &&
		      obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
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
evas_object_stack_above(Evas_Object *obj, Evas_Object *above)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   MAGIC_CHECK(above, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_stack_above(obj, above)) return;
   if (!above)
     {
	evas_object_raise(obj);
	return;
     }
   if (((Evas_Object_List *)obj)->prev == (Evas_Object_List *)above)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->smart.parent)
     {
	if (obj->smart.parent != above->smart.parent)
	  {
//	     printf("BITCH! evas_object_stack_above(), %p not inside same smart as %p!\n", obj, above);
	     return;
	  }
	obj->smart.parent->smart.contained = evas_object_list_remove(obj->smart.parent->smart.contained, obj);
	obj->smart.parent->smart.contained = evas_object_list_append_relative(obj->smart.parent->smart.contained, obj, above);
     }
   else
     {
	if (obj->layer != above->layer)
	  {
	     return;
	  }
	if (obj->in_layer)
	  {
	     obj->layer->objects = evas_object_list_remove(obj->layer->objects, obj);
	     obj->layer->objects = evas_object_list_append_relative(obj->layer->objects, obj, above);
	  }
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   obj->restack = 1;
   evas_object_change(obj);
   evas_object_inform_call_restack(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1) &&
		      obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
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
evas_object_stack_below(Evas_Object *obj, Evas_Object *below)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   MAGIC_CHECK(below, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_stack_below(obj, below)) return;
   if (!below)
     {
	evas_object_lower(obj);
	return;
     }
   if (((Evas_Object_List *)obj)->next == (Evas_Object_List *)below)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->smart.parent)
     {
	if (obj->smart.parent != below->smart.parent)
	  {
//	     printf("BITCH! evas_object_stack_below(), %p not inside same smart as %p!\n", obj, below);
	     return;
	  }
	obj->smart.parent->smart.contained = evas_object_list_remove(obj->smart.parent->smart.contained, obj);
	obj->smart.parent->smart.contained = evas_object_list_prepend_relative(obj->smart.parent->smart.contained, obj, below);
     }
   else
     {
	if (obj->layer != below->layer)
	  {
	     return;
	  }
	if (obj->in_layer)
	  {
	     obj->layer->objects = evas_object_list_remove(obj->layer->objects, obj);
	     obj->layer->objects = evas_object_list_prepend_relative(obj->layer->objects, obj, below);
	  }
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   obj->restack = 1;
   evas_object_change(obj);
   evas_object_inform_call_restack(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1) &&
		      obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Object *
evas_object_above_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->smart.parent)
     return (Evas_Object *)(((Evas_Object_List *)(obj))->next);
   return evas_object_above_get_internal(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Object *
evas_object_below_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->smart.parent)
     return (Evas_Object *)(((Evas_Object_List *)(obj))->prev);
   return evas_object_below_get_internal(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Object *
evas_object_bottom_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (e->layers)
     return e->layers->objects;
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Object *
evas_object_top_get(Evas *e)
{
   Evas_Object *obj2 = NULL;
   Evas_Object_List *list;
   Evas_Layer *layer;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   list = (Evas_Object_List *) e->layers;
   if (!list) return NULL;

   layer = (Evas_Layer *) list->last;
   if (!layer) return NULL;

   list = (Evas_Object_List *) layer->objects;
   if (!list) return NULL;

   obj2 = (Evas_Object *) list->last;
   if (!obj2) return NULL;

   return obj2;
}
