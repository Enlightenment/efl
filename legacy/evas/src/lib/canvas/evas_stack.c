#include "evas_common.h"
#include "evas_private.h"

static Evas_Object *
evas_object_above_get_internal(const Evas_Object *obj)
{
   if ((EINA_INLIST_GET(obj))->next)
     return (Evas_Object *)((EINA_INLIST_GET(obj))->next);
   else
     {
	if ((EINA_INLIST_GET(((Evas_Object*)(obj))->layer))->next)
	  {
	     Evas_Layer *l;

	     l = (Evas_Layer *)((EINA_INLIST_GET((((Evas_Object *)obj)->layer)))->next);
	     return l->objects;
	  }
     }
   return NULL;
}

static Evas_Object *
evas_object_below_get_internal(const Evas_Object *obj)
{
   if ((EINA_INLIST_GET(obj))->prev)
     return (Evas_Object *)((EINA_INLIST_GET(obj))->prev);
   else
     {
        if ((EINA_INLIST_GET((((Evas_Object *)obj)->layer)))->prev)
	  {
	     Evas_Layer *l;

	     l = (Evas_Layer *)((EINA_INLIST_GET((((Evas_Object *)obj)->layer)))->prev);
	     return (Evas_Object *)((EINA_INLIST_GET((l->objects)))->last);
	  }
     }
   return NULL;
}

/**
 * Raise @p obj to the top of its layer.
 *
 * @param obj the object to raise
 *
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_raise(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_raise(obj)) return;
   if (!((EINA_INLIST_GET(obj))->next))
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->smart.parent)
     evas_object_smart_member_raise(obj);
   else
     {
	if (obj->in_layer)
	  obj->layer->objects = (Evas_Object *)eina_inlist_demote(EINA_INLIST_GET(obj->layer->objects),
								  EINA_INLIST_GET(obj));
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas);
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
 * Lower @p obj to the bottom of its layer.
 *
 * @param obj the object to lower
 *
 * @ingroup Evas_Object_Group
 */
EAPI void
evas_object_lower(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_lower(obj)) return;
   if (!((EINA_INLIST_GET(obj))->prev))
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->smart.parent)
     evas_object_smart_member_lower(obj);
   else
     {
	if (obj->in_layer)
	  obj->layer->objects = (Evas_Object *)eina_inlist_promote(EINA_INLIST_GET(obj->layer->objects),
								   EINA_INLIST_GET(obj));
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas);
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
 * Stack @p obj immediately above @p above 
 *
 * If @p obj is a member of a smart object, then @p above must also be
 * a member of the same smart object.
 *
 * Similarly, if @p obj is not a member of smart object, @p above may 
 * not either. 
 *
 * @param obj the object to stack
 * @param above the object above which to stack
 *
 * @ingroup Evas_Object_Group
 */
EAPI void
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
   if ((EINA_INLIST_GET(obj))->prev == EINA_INLIST_GET(above))
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
	evas_object_smart_member_stack_above(obj, above);
     }
   else
     {
	if (above->smart.parent) return;
	if (obj->layer != above->layer)
	  {
	     return;
	  }
	if (obj->in_layer)
	  {
 	     obj->layer->objects = (Evas_Object *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects),
								     EINA_INLIST_GET(obj));
	     obj->layer->objects = (Evas_Object *)eina_inlist_append_relative(EINA_INLIST_GET(obj->layer->objects),
									      EINA_INLIST_GET(obj),
									      EINA_INLIST_GET(above));
	  }
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas);
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
 * Stack @p obj immediately below @p below 
 *
 * If @p obj is a member of a smart object, then @p below must also be
 * a member of the same smart object.
 *
 * Similarly, if @p obj is not a member of smart object, @p below may 
 * not either. 
 *
 * @param obj the object to stack
 * @param below the object below which to stack
 *
 * @ingroup Evas_Object_Group
 */
EAPI void
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
   if ((EINA_INLIST_GET(obj))->next == EINA_INLIST_GET(below))
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
	evas_object_smart_member_stack_below(obj, below);
     }
   else
     {
	if (below->smart.parent) return;
	if (obj->layer != below->layer)
	  {
	     return;
	  }
	if (obj->in_layer)
	  {
	     obj->layer->objects = (Evas_Object *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects),
								     EINA_INLIST_GET(obj));
	     obj->layer->objects = (Evas_Object *)eina_inlist_prepend_relative(EINA_INLIST_GET(obj->layer->objects),
									       EINA_INLIST_GET(obj),
									       EINA_INLIST_GET(below));
	  }
     }
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (obj->layer) evas_render_invalidate(obj->layer->evas);
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
 * Get the evas object above @p obj
 *
 * @param obj an Evas_Object
 * @return the Evas_Object directly above
 *
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Object *
evas_object_above_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->smart.parent)
     {
	do
	  {
	     obj = (Evas_Object *)((EINA_INLIST_GET(obj))->next);
	     if ((obj) && (!obj->delete_me)) return (Evas_Object *)obj;
	  }
	while (obj);
	return NULL;
     }
   obj = evas_object_above_get_internal(obj);
   while (obj)
     {
	if (!obj->delete_me) return (Evas_Object *)obj;
	obj = evas_object_above_get_internal(obj);
     }
   return NULL;
}

/**
 * Get the evas object below @p obj
 *
 * @param obj an Evas_Object
 * @return the Evas_Object directly below
 *
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Object *
evas_object_below_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->smart.parent)
     {
	do
	  {
	     obj = (Evas_Object *)((EINA_INLIST_GET(obj))->prev);
	     if ((obj) && (!obj->delete_me)) return (Evas_Object *)obj;
	  }
	while (obj);
	return NULL;
     }
   obj = evas_object_below_get_internal(obj);
   while (obj)
     {
	if (!obj->delete_me) return (Evas_Object *)obj;
	obj = evas_object_below_get_internal(obj);
     }
   return NULL;
}

/**
 * Get the lowest evas object on the Evas @p e
 *
 * @param e an Evas
 * @return the lowest object
 *
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Object *
evas_object_bottom_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (e->layers)
     {
	Evas_Object *obj;
	
	obj = e->layers->objects;
	while (obj)
	  {
	     if (!obj->delete_me) return obj;
	     obj = evas_object_above_get_internal(obj);
	  }
     }
   return NULL;
}

/**
 * Get the highest evas object on the Evas @p e
 *
 * @param e an Evas
 * @return the highest object
 *
 * @ingroup Evas_Object_Group
 */
EAPI Evas_Object *
evas_object_top_get(const Evas *e)
{
   Evas_Object *obj = NULL;
   Eina_Inlist *list;
   Evas_Layer *layer;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   list = EINA_INLIST_GET(e->layers);
   if (!list) return NULL;

   layer = (Evas_Layer *) list->last;
   if (!layer) return NULL;

   list = EINA_INLIST_GET(layer->objects);
   if (!list) return NULL;

   obj = (Evas_Object *) list->last;
   if (!obj) return NULL;

   while (obj)
     {
	if (!obj->delete_me) return obj;
	obj = evas_object_below_get_internal(obj);
     }

   return obj;
}
