#include "evas_common.h"
#include "evas_private.h"

static Evas_List *
evas_render_updates_internal(Evas *e, unsigned char make_updates, unsigned char do_draw);

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_damage_rectangle_add(Evas *e, int x, int y, int w, int h)
{
   Evas_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   r = malloc(sizeof(Evas_Rectangle));
   if (!r) return;
   r->x = x; r->y = y; r->w = w; r->h = h;
   e->damages = evas_list_append(e->damages, r);
   e->changed = 1;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_obscured_rectangle_add(Evas *e, int x, int y, int w, int h)
{
   Evas_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   r = malloc(sizeof(Evas_Rectangle));
   if (!r) return;
   r->x = x; r->y = y; r->w = w; r->h = h;
   e->obscures = evas_list_append(e->obscures, r);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_obscured_clear(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   while (e->obscures)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)e->obscures->data;
	e->obscures = evas_list_remove(e->obscures, r);
	free(r);
     }
}

static void
_evas_render_phase1_direct(Evas *e, Evas_Array *render_objects)
{
   unsigned int i;

   for (i = 0; i < render_objects->count; ++i)
     {
	Evas_Object *obj;

	obj = _evas_array_get(render_objects, i);
	if (obj->changed) obj->func->render_pre(obj);
	else
	  {
	     if (obj->smart.smart)
	       obj->func->render_pre(obj);
	     else
	       if (obj->rect_del)
		 {
		    e->engine.func->output_redraws_rect_del(e->engine.data.output,
							    obj->cur.cache.clip.x,
							    obj->cur.cache.clip.y,
							    obj->cur.cache.clip.w,
							    obj->cur.cache.clip.h);
		 }
	  }
     }
}

static Evas_Bool
_evas_render_phase1_object_process(Evas *e, Evas_Object *obj, Evas_Array *active_objects, Evas_Array *restack_objects, Evas_Array *delete_objects, Evas_Array *render_objects, int restack)
{
   int clean_them = 0;
   int is_active;

   obj->rect_del = 0;
   obj->render_pre = 0;

/* if (obj->cur.cache.clip.dirty) */
   evas_object_clip_recalc(obj);
   /* because of clip objects - delete 2 cycles later */
   if (obj->delete_me == 2)
     _evas_array_append(delete_objects, obj);
   else if (obj->delete_me != 0) obj->delete_me++;
   /* If the object will be removed, we should not cache anything during this run. */
   if (obj->delete_me != 0)
     clean_them = 1;

   /* build active object list */
   is_active = evas_object_is_active(obj);

   obj->is_active = is_active;
   if ((is_active) || (obj->delete_me != 0))
     _evas_array_append(active_objects, obj);
   if (restack)
     {
	if (!obj->changed)
	  _evas_array_append(&e->pending_objects, obj);
	obj->restack = 1;
	obj->changed = 1;
	clean_them = 1;
     }
   if (obj->changed)
     {
	if (obj->smart.smart)
	  {
	     const Evas_Object_List *l;

	     _evas_array_append(render_objects, obj);
	     obj->render_pre = 1;
	     for (l = evas_object_smart_members_get_direct(obj); l; l = l->next)
	       {
		  Evas_Object *obj2;

		  obj2 = (Evas_Object *)l;
		  _evas_render_phase1_object_process(e, obj2,
						     active_objects,
						     restack_objects,
						     delete_objects,
						     render_objects,
						     obj->restack);
	       }
	  }
	else
	  {
	     if ((is_active) && (obj->restack) && (!obj->clip.clipees) &&
		 ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
		  (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
               _evas_array_append(restack_objects, obj);
	     else if ((is_active) && (!obj->clip.clipees) &&
		      ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
		       (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
	       {
		  _evas_array_append(render_objects, obj);
		  obj->render_pre = 1;
	       }
	  }
     }
   else
     {
	if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
	    ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
	     (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
	  {
	     if (obj->smart.smart)
	       {
		  const Evas_Object_List *l;

		  _evas_array_append(render_objects, obj);
		  obj->render_pre = 1;
		  for (l = evas_object_smart_members_get_direct(obj); l; l = l->next)
		    {
		       Evas_Object *obj2;

		       obj2 = (Evas_Object *)l;
		       _evas_render_phase1_object_process(e, obj2,
							  active_objects,
							  restack_objects,
							  delete_objects,
							  render_objects,
							  restack);
		    }
	       }
	     else
	       {
		  if (evas_object_is_opaque(obj) &&
		      evas_object_is_visible(obj))
		    {
		       _evas_array_append(render_objects, obj);
		       obj->rect_del = 1;
		    }
	       }
	  }
     }
   if (!is_active) obj->restack = 0;
   return clean_them;
}

static Evas_Bool
_evas_render_phase1_process(Evas *e, Evas_Array *active_objects, Evas_Array *restack_objects, Evas_Array *delete_objects, Evas_Array *render_objects)
{
   Evas_Object_List *l;
   int clean_them = 0;

   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;

	lay = (Evas_Layer *)l;
	for (l2 = (Evas_Object_List *)lay->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;

	     obj = (Evas_Object *)l2;
	     clean_them |= _evas_render_phase1_object_process(e, obj,
							      active_objects, restack_objects,
							      delete_objects, render_objects,
							      0);
	  }
     }

   return clean_them;
}

static void
_evas_render_check_pending_objects(Evas_Array *pending_objects, Evas *e)
{
   unsigned int i;

   for (i = 0; i < pending_objects->count; ++i)
     {
	Evas_Object *obj;
	int ok = 0;
	int is_active;

	obj = _evas_array_get(pending_objects, i);

	if (!obj->layer) goto clean_stuff;

	evas_object_clip_recalc(obj);
	is_active = evas_object_is_active(obj);

	if (!is_active &&
	    !obj->is_active &&
	    !obj->render_pre &&
	    !obj->rect_del)
	  {
	     ok = 1;
	     goto clean_stuff;
	  }

	if (obj->is_active == is_active)
	  {
	     if (obj->changed)
	       {
		  if (obj->smart.smart)
		    {
		       if (obj->render_pre
			   || obj->rect_del)
			 ok = 1;
		    }
		  else
		    if ((is_active) && (obj->restack) && (!obj->clip.clipees) &&
			((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
			 (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
		      {
			 if (!(obj->render_pre
			       || obj->rect_del))
			   ok = 1;
		      }
		    else
		      if (is_active && (!obj->clip.clipees) &&
			  ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
			   (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
			{
			   if (obj->render_pre
			       || obj->rect_del)
			     ok = 1;
			}
	       }
	     else
	       {
		  if ((!obj->clip.clipees) && (obj->delete_me == 0) &&
		      (!obj->cur.have_clipees || (evas_object_was_visible(obj) && (!obj->prev.have_clipees)))
		      && evas_object_is_opaque(obj) && evas_object_is_visible(obj))
		    if (obj->rect_del || obj->smart.smart)
		      {
			 ok = 1;
		      }
	       }
	  }

     clean_stuff:
	if (!ok)
	  {
	     evas_array_clean(&e->active_objects);
	     evas_array_clean(&e->render_objects);

	     evas_array_clean(&e->restack_objects);
	     evas_array_clean(&e->delete_objects);

	     e->invalidate = 1;

	     return ;
	  }
     }
}

Evas_Bool pending_change(void *data, void *gdata)
{
   Evas_Object *obj;

   obj = data;
   if (obj->delete_me) return 0;
   if (!obj->layer) obj->changed = 0;
   return obj->changed;
}

static Evas_List *
evas_render_updates_internal(Evas *e, unsigned char make_updates, unsigned char do_draw)
{
   Evas_List *updates = NULL;
   Evas_List *ll;
   void *surface;
   Evas_Bool clean_them = 0;
   int ux, uy, uw, uh;
   int cx, cy, cw, ch;
   unsigned int i, j;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!e->changed) return NULL;

   /* Check if the modified object mean recalculating every thing */
   if (!e->invalidate)
     _evas_render_check_pending_objects(&e->pending_objects, e);

   /* phase 1. add extra updates for changed objects */
   if (e->invalidate || e->render_objects.count <= 0)
     clean_them = _evas_render_phase1_process(e, &e->active_objects, &e->restack_objects, &e->delete_objects, &e->render_objects);

   _evas_render_phase1_direct(e, &e->render_objects);

   /* phase 2. force updates for restacks */
   for (i = 0; i < e->restack_objects.count; ++i)
     {
	Evas_Object *obj;

	obj = _evas_array_get(&e->restack_objects, i);
	obj->func->render_pre(obj);
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
						obj->prev.cache.clip.x,
						obj->prev.cache.clip.y,
						obj->prev.cache.clip.w,
						obj->prev.cache.clip.h);
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
						obj->cur.cache.clip.x,
						obj->cur.cache.clip.y,
						obj->cur.cache.clip.w,
						obj->cur.cache.clip.h);
     }
   evas_array_clean(&e->restack_objects);
   /* phase 3. add exposes */
   while (e->damages)
     {
	Evas_Rectangle *r;

	r = e->damages->data;
	e->damages = evas_list_remove(e->damages, r);
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
					       r->x, r->y, r->w, r->h);
	free(r);
     }
   /* phase 4. output & viewport changes */
   if (e->viewport.changed)
     {
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
						0, 0,
						e->output.w, e->output.h);
     }
   if (e->output.changed)
     {
	e->engine.func->output_resize(e->engine.data.output,
				      e->output.w, e->output.h);
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
						0, 0,
						e->output.w, e->output.h);
     }
   if ((e->output.w != e->viewport.w) || (e->output.h != e->viewport.h))
     {
	printf("EVAS: error: viewport size != output size!\n");
     }
   /* phase 5. add obscures */
   for (ll = e->obscures; ll; ll = ll->next)
     {
	Evas_Rectangle *r;

	r = ll->data;
	e->engine.func->output_redraws_rect_del(e->engine.data.output,
					       r->x, r->y, r->w, r->h);
     }
   /* build obscure objects list of active objects that obscure */
   for (i = 0; i < e->active_objects.count; ++i)
     {
	Evas_Object *obj;

	obj = _evas_array_get(&e->active_objects, i);
	if (UNLIKELY(evas_object_is_opaque(obj) &&
                     evas_object_is_visible(obj) &&
                     (!obj->clip.clipees) &&
                     (obj->cur.visible) &&
                     (!obj->delete_me) &&
                     (obj->cur.cache.clip.visible) &&
                     (!obj->smart.smart)))
/* 	  obscuring_objects = evas_list_append(obscuring_objects, obj); */
	  _evas_array_append(&e->obscuring_objects, obj);
     }
   /* save this list */
/*    obscuring_objects_orig = obscuring_objects; */
/*    obscuring_objects = NULL; */
   /* phase 6. go thru each update rect and render objects in it*/
   if (do_draw)
     {
	unsigned int offset = 0;

	while ((surface =
		e->engine.func->output_redraws_next_update_get(e->engine.data.output,
							       &ux, &uy, &uw, &uh,
							       &cx, &cy, &cw, &ch)))
	  {
	     int off_x, off_y;

	     if (make_updates)
	       {
		  Evas_Rectangle *rect;

		  rect = malloc(sizeof(Evas_Rectangle));
		  if (rect)
		    {
		       rect->x = ux; rect->y = uy; rect->w = uw; rect->h = uh;
		       updates = evas_list_append(updates, rect);
		    }
	       }
	     off_x = cx - ux;
	     off_y = cy - uy;
	     /* build obscuring objects list (in order from bottom to top) */
	     for (i = 0; i < e->obscuring_objects.count; ++i)
	       {
		  Evas_Object *obj;

		  obj = (Evas_Object *) _evas_array_get(&e->obscuring_objects, i);
		  if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh))
		    _evas_array_append(&e->temporary_objects, obj);
	       }
	     /* render all object that intersect with rect */
             for (i = 0; i < e->active_objects.count; ++i)
	       {
		  Evas_Object *obj;

		  obj = _evas_array_get(&e->active_objects, i);

		  /* if it's in our outpout rect and it doesn't clip anything */
		  if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh) &&
		      (!obj->clip.clipees) &&
		      (obj->cur.visible) &&
		      (!obj->delete_me) &&
		      (obj->cur.cache.clip.visible) &&
		      (!obj->smart.smart) &&
		      (obj->cur.color.a > 0))
		    {
		       int x, y, w, h;

		       if ((e->temporary_objects.count > offset) &&
			   (_evas_array_get(&e->temporary_objects, offset) == obj))
			 offset++;
		       x = cx; y = cy; w = cw; h = ch;
		       RECTS_CLIP_TO_RECT(x, y, w, h,
					  obj->cur.cache.clip.x + off_x,
					  obj->cur.cache.clip.y + off_y,
					  obj->cur.cache.clip.w,
					  obj->cur.cache.clip.h);
		       if ((w > 0) && (h > 0))
			 {
///		       printf("CLIP: %p | %i %i, %ix%i | %p %i %i %ix%i\n",
///			      obj,
///			      x, y, w, h,
///			      obj->cur.clipper,
///			      obj->cur.cache.clip.x + off_x,
///			      obj->cur.cache.clip.y + off_y,
///			      obj->cur.cache.clip.w,
///			      obj->cur.cache.clip.h
///			      );
///			    if (((obj->cur.cache.clip.x + off_x) == 0) &&
///				((obj->cur.cache.clip.w) == 960))
///			      {
///				 abort();
///			      }

			    e->engine.func->context_clip_set(e->engine.data.output,
							     e->engine.data.context,
							     x, y, w, h);
#if 1 /* FIXME: this can slow things down... figure out optimum... coverage */
			    for (j = offset; j < e->temporary_objects.count; ++j)
			      {
				 Evas_Object *obj2;

				 obj2 = (Evas_Object *) _evas_array_get(&e->temporary_objects, j);
				 e->engine.func->context_cutout_add(e->engine.data.output,
								    e->engine.data.context,
								    obj2->cur.cache.clip.x + off_x,
								    obj2->cur.cache.clip.y + off_y,
								    obj2->cur.cache.clip.w,
								    obj2->cur.cache.clip.h);
			      }
#endif
			    obj->func->render(obj,
					      e->engine.data.output,
					      e->engine.data.context,
					      surface,
					      off_x, off_y);
			    e->engine.func->context_cutout_clear(e->engine.data.output,
								 e->engine.data.context);
			 }
		    }
	       }
	     /* punch rect out */
	     e->engine.func->output_redraws_next_update_push(e->engine.data.output,
							     surface,
							     ux, uy, uw, uh);
	     /* free obscuring objects list */
	     evas_array_clean(&e->temporary_objects);
	  }
	/* flush redraws */
	e->engine.func->output_flush(e->engine.data.output);
     }
   /* clear redraws */
   e->engine.func->output_redraws_clear(e->engine.data.output);
   /* and do a post render pass */
   for (i = 0; i < e->active_objects.count; ++i)
     {
	Evas_Object *obj;

	obj = _evas_array_get(&e->active_objects, i);
	obj->pre_render_done = 0;
	if ((obj->changed) && (do_draw))
	  {
	     obj->func->render_post(obj);
	     obj->restack = 0;
	     obj->changed = 0;
	  }
/* moved to other pre-process phase 1
	if (obj->delete_me == 2)
	  {
	     delete_objects = evas_list_append(delete_objects, obj);
	  }
	else if (obj->delete_me != 0) obj->delete_me++;
 */
     }
   /* free our obscuring object list */
   evas_array_clean(&e->obscuring_objects);

   /* If some object are still marked as changed, do not remove
      them from the pending list. */
   evas_array_remove(&e->pending_objects, pending_change, NULL);

   /* delete all objects flagged for deletion now */
   for (i = 0; i < e->delete_objects.count; ++i)
     {
	Evas_Object *obj;

	obj = _evas_array_get(&e->delete_objects, i);
	evas_object_free(obj, 1);
     }
   evas_array_clean(&e->delete_objects);

   e->changed = 0;
   e->viewport.changed = 0;
   e->output.changed = 0;
   e->invalidate = 0;

   /* If their are some object to restack or some object to delete, it's useless to keep the render object list around. */
   if (clean_them)
     {
	evas_array_clean(&e->active_objects);
	evas_array_clean(&e->render_objects);

	e->invalidate = 1;
     }

   evas_module_clean();

   return updates;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_render_updates_free(Evas_List *updates)
{
   while (updates)
     {
	free(updates->data);
	updates = evas_list_remove(updates, updates->data);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_List *
evas_render_updates(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e->changed) return NULL;
   return evas_render_updates_internal(e, 1, 1);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_render(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!e->changed) return;
   evas_render_updates_internal(e, 0, 1);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_norender(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

//   if (!e->changed) return;
   evas_render_updates_internal(e, 0, 0);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_render_idle_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((e->engine.func) && (e->engine.func->output_idle_flush) &&
       (e->engine.data.output))
     e->engine.func->output_idle_flush(e->engine.data.output);

   evas_array_flush(&e->delete_objects);
   evas_array_flush(&e->active_objects);
   evas_array_flush(&e->restack_objects);
   evas_array_flush(&e->render_objects);

   e->invalidate = 1;
}

void
evas_render_invalidate(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_array_clean(&e->active_objects);
   evas_array_clean(&e->render_objects);

   evas_array_flush(&e->restack_objects);
   evas_array_flush(&e->delete_objects);

   e->invalidate = 1;
}

void
evas_render_object_recalc(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if ((!obj->changed) && (obj->delete_me < 2))
     {
	Evas	*e;

	e = obj->layer->evas;
	if (!e || e->cleanup) return ;

	if (!obj->changed)
	  _evas_array_append(&e->pending_objects, obj);
	obj->changed = 1;
     }
}

