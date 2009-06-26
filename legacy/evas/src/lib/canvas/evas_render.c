#include "evas_common.h"
#include "evas_private.h"

static Eina_List *
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
   Eina_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   NEW_RECT(r, x, y, w, h);
   if (!r) return;
   e->damages = eina_list_append(e->damages, r);
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
   Eina_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   NEW_RECT(r, x, y, w, h);
   if (!r) return;
   e->obscures = eina_list_append(e->obscures, r);
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
   Eina_Rectangle *r;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   EINA_LIST_FREE(e->obscures, r)
     eina_rectangle_free(r);
}

static void
_evas_render_phase1_direct(Evas *e, Eina_Array *render_objects)
{
   unsigned int i;

   for (i = 0; i < render_objects->count; ++i)
     {
	Evas_Object *obj;

	obj = eina_array_data_get(render_objects, i);
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

static Eina_Bool
_evas_render_phase1_object_process(Evas *e, Evas_Object *obj, Eina_Array *active_objects, Eina_Array *restack_objects, Eina_Array *delete_objects, Eina_Array *render_objects, int restack)
{
   Eina_Bool clean_them = EINA_FALSE;
   int is_active;

   obj->rect_del = 0;
   obj->render_pre = 0;

/* if (obj->cur.cache.clip.dirty) */
   evas_object_clip_recalc(obj);
   /* because of clip objects - delete 2 cycles later */
   if (obj->delete_me == 2)
     eina_array_push(delete_objects, obj);
   else if (obj->delete_me != 0) obj->delete_me++;
   /* If the object will be removed, we should not cache anything during this run. */
   if (obj->delete_me != 0)
     clean_them = EINA_TRUE;

   /* build active object list */
   is_active = evas_object_is_active(obj);

   obj->is_active = is_active;
   if ((is_active) || (obj->delete_me != 0))
     eina_array_push(active_objects, obj);
   if (restack)
     {
	if (!obj->changed)
	  eina_array_push(&e->pending_objects, obj);
	obj->restack = 1;
	obj->changed = 1;
	clean_them = EINA_TRUE;
     }
   if (obj->changed)
     {
	if (obj->smart.smart)
	  {
	     Evas_Object *obj2;

	     eina_array_push(render_objects, obj);
	     obj->render_pre = 1;
	     EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
	       {
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
               eina_array_push(restack_objects, obj);
	     else if ((is_active) && (!obj->clip.clipees) &&
		      ((evas_object_is_visible(obj) && (!obj->cur.have_clipees)) ||
		       (evas_object_was_visible(obj) && (!obj->prev.have_clipees))))
	       {
		  eina_array_push(render_objects, obj);
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
		  Evas_Object *obj2;

		  eina_array_push(render_objects, obj);
		  obj->render_pre = 1;
		  EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
		    {
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
		       eina_array_push(render_objects, obj);
		       obj->rect_del = 1;
		    }
	       }
	  }
     }
   if (!is_active) obj->restack = 0;
   return clean_them;
}

static Eina_Bool
_evas_render_phase1_process(Evas *e, Eina_Array *active_objects, Eina_Array *restack_objects, Eina_Array *delete_objects, Eina_Array *render_objects)
{
   Evas_Layer *lay;
   Eina_Bool clean_them = EINA_FALSE;

   EINA_INLIST_FOREACH(e->layers, lay)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(lay->objects, obj)
	  {
	     clean_them |= _evas_render_phase1_object_process(e, obj,
							      active_objects, restack_objects,
							      delete_objects, render_objects,
							      0);
	  }
     }

   return clean_them;
}

static void
_evas_render_check_pending_objects(Eina_Array *pending_objects, Evas *e)
{
   unsigned int i;

   for (i = 0; i < pending_objects->count; ++i)
     {
	Evas_Object *obj;
	int ok = 0;
	int is_active;

	obj = eina_array_data_get(pending_objects, i);

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
	     eina_array_clean(&e->active_objects);
	     eina_array_clean(&e->render_objects);

	     eina_array_clean(&e->restack_objects);
	     eina_array_clean(&e->delete_objects);

	     e->invalidate = 1;

	     return ;
	  }
     }
}

Eina_Bool pending_change(void *data, __UNUSED__ void *gdata)
{
   Evas_Object *obj;

   obj = data;
   if (obj->delete_me) return EINA_FALSE;
   if (!obj->layer) obj->changed = 0;
   return obj->changed ? EINA_TRUE : EINA_FALSE;
}

static Eina_List *
evas_render_updates_internal(Evas *e, unsigned char make_updates, unsigned char do_draw)
{
   Eina_List *updates = NULL;
   Eina_List *ll;
   void *surface;
   Eina_Bool clean_them = EINA_FALSE;
   Eina_Bool alpha;
   Eina_Rectangle *r;
   int ux, uy, uw, uh;
   int cx, cy, cw, ch;
   unsigned int i, j;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!e->changed) return NULL;

   evas_call_smarts_calculate(e);

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

	obj = eina_array_data_get(&e->restack_objects, i);
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
   eina_array_clean(&e->restack_objects);
   /* phase 3. add exposes */
   EINA_LIST_FREE(e->damages, r)
     {
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
					       r->x, r->y, r->w, r->h);
	eina_rectangle_free(r);
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
   EINA_LIST_FOREACH(e->obscures, ll, r)
        e->engine.func->output_redraws_rect_del(e->engine.data.output,
					       r->x, r->y, r->w, r->h);
   /* build obscure objects list of active objects that obscure */
   for (i = 0; i < e->active_objects.count; ++i)
     {
	Evas_Object *obj;

	obj = eina_array_data_get(&e->active_objects, i);
	if (UNLIKELY((evas_object_is_opaque(obj) ||
                      ((obj->func->has_opaque_rect) &&
                       (obj->func->has_opaque_rect(obj)))) &&
                     evas_object_is_visible(obj) &&
                     (!obj->clip.clipees) &&
                     (obj->cur.visible) &&
                     (!obj->delete_me) &&
                     (obj->cur.cache.clip.visible) &&
                     (!obj->smart.smart)))
/* 	  obscuring_objects = eina_list_append(obscuring_objects, obj); */
	  eina_array_push(&e->obscuring_objects, obj);
     }
   /* save this list */
/*    obscuring_objects_orig = obscuring_objects; */
/*    obscuring_objects = NULL; */
   /* phase 6. go thru each update rect and render objects in it*/
   if (do_draw)
     {
	unsigned int offset = 0;

	alpha = e->engine.func->canvas_alpha_get(e->engine.data.output, e->engine.data.context);

	while ((surface =
		e->engine.func->output_redraws_next_update_get(e->engine.data.output,
							       &ux, &uy, &uw, &uh,
							       &cx, &cy, &cw, &ch)))
	  {
	     int off_x, off_y;

	     if (make_updates)
	       {
		  Eina_Rectangle *rect;

		  NEW_RECT(rect, ux, uy, uw, uh);
		  if (rect)
		    updates = eina_list_append(updates, rect);
	       }
	     off_x = cx - ux;
	     off_y = cy - uy;
	     /* build obscuring objects list (in order from bottom to top) */
	     for (i = 0; i < e->obscuring_objects.count; ++i)
	       {
		  Evas_Object *obj;

		  obj = (Evas_Object *) eina_array_data_get(&e->obscuring_objects, i);
		  if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh))
		    {
		       eina_array_push(&e->temporary_objects, obj);

		       /* reset the background of the area if needed (using cutout and engine alpha flag to help) */
		       if (alpha)
			 {
			    if (evas_object_is_opaque(obj))
			      e->engine.func->context_cutout_add(e->engine.data.output,
								 e->engine.data.context,
								 obj->cur.cache.clip.x + off_x,
								 obj->cur.cache.clip.y + off_y,
								 obj->cur.cache.clip.w,
								 obj->cur.cache.clip.h);
			    else
			      {
				 if (obj->func->get_opaque_rect)
				   {
				      Evas_Coord obx, oby, obw, obh;

				      obj->func->get_opaque_rect(obj, &obx, &oby, &obw, &obh);
				      if ((obw > 0) && (obh > 0))
					{
					   obx += off_x;
					   oby += off_y;
					   RECTS_CLIP_TO_RECT(obx, oby, obw, obh,
							      obj->cur.cache.clip.x + off_x,
							      obj->cur.cache.clip.y + off_y,
							      obj->cur.cache.clip.w,
							      obj->cur.cache.clip.h);
					   e->engine.func->context_cutout_add(e->engine.data.output,
									      e->engine.data.context,
									      obx, oby,
									      obw, obh);
					}
				   }
			      }
			 }
		    }
	       }
	     if (alpha)
	       {
		  e->engine.func->context_color_set(e->engine.data.output, e->engine.data.context, 0, 0, 0, 0);
		  e->engine.func->context_multiplier_unset(e->engine.data.output, e->engine.data.context);
		  e->engine.func->context_render_op_set(e->engine.data.output, e->engine.data.context, EVAS_RENDER_COPY);
		  e->engine.func->rectangle_draw(e->engine.data.output,
						 e->engine.data.context,
						 surface,
						 cx, cy, cw, ch);
		  e->engine.func->context_cutout_clear(e->engine.data.output,
						       e->engine.data.context);
	       }

	     /* render all object that intersect with rect */
             for (i = 0; i < e->active_objects.count; ++i)
	       {
		  Evas_Object *obj;

		  obj = eina_array_data_get(&e->active_objects, i);

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
			   (eina_array_data_get(&e->temporary_objects, offset) == obj))
			 offset++;
		       x = cx; y = cy; w = cw; h = ch;
		       RECTS_CLIP_TO_RECT(x, y, w, h,
					  obj->cur.cache.clip.x + off_x,
					  obj->cur.cache.clip.y + off_y,
					  obj->cur.cache.clip.w,
					  obj->cur.cache.clip.h);
		       if ((w > 0) && (h > 0))
			 {
			    e->engine.func->context_clip_set(e->engine.data.output,
							     e->engine.data.context,
							     x, y, w, h);
#if 1 /* FIXME: this can slow things down... figure out optimum... coverage */
			    for (j = offset; j < e->temporary_objects.count; ++j)
			      {
				 Evas_Object *obj2;

				 obj2 = (Evas_Object *) eina_array_data_get(&e->temporary_objects, j);
                                 if (evas_object_is_opaque(obj2))
                                   e->engine.func->context_cutout_add(e->engine.data.output,
                                                                      e->engine.data.context,
                                                                      obj2->cur.cache.clip.x + off_x,
                                                                      obj2->cur.cache.clip.y + off_y,
                                                                      obj2->cur.cache.clip.w,
                                                                      obj2->cur.cache.clip.h);
                                 else
                                   {
                                      if (obj2->func->get_opaque_rect)
                                        {
                                           Evas_Coord obx, oby, obw, obh;
                                           
                                           obj2->func->get_opaque_rect
                                             (obj2, &obx, &oby, &obw, &obh);
                                           if ((obw > 0) && (obh > 0))
                                             {
                                                obx += off_x;
                                                oby += off_y;
                                                RECTS_CLIP_TO_RECT(obx, oby, obw, obh,
                                                                   obj2->cur.cache.clip.x + off_x,
                                                                   obj2->cur.cache.clip.y + off_y,
                                                                   obj2->cur.cache.clip.w,
                                                                   obj2->cur.cache.clip.h);
                                                e->engine.func->context_cutout_add(e->engine.data.output,
                                                                                   e->engine.data.context,
                                                                                   obx, oby,
                                                                                   obw, obh);
                                             }
                                        }
                                   }
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
	     eina_array_clean(&e->temporary_objects);
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

	obj = eina_array_data_get(&e->active_objects, i);
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
	     delete_objects = eina_list_append(delete_objects, obj);
	  }
	else if (obj->delete_me != 0) obj->delete_me++;
 */
     }
   /* free our obscuring object list */
   eina_array_clean(&e->obscuring_objects);

   /* If some object are still marked as changed, do not remove
      them from the pending list. */
   eina_array_remove(&e->pending_objects, pending_change, NULL);

   /* delete all objects flagged for deletion now */
   for (i = 0; i < e->delete_objects.count; ++i)
     {
	Evas_Object *obj;

	obj = eina_array_data_get(&e->delete_objects, i);
	evas_object_free(obj, 1);
     }
   eina_array_clean(&e->delete_objects);

   e->changed = 0;
   e->viewport.changed = 0;
   e->output.changed = 0;
   e->invalidate = 0;

   /* If their are some object to restack or some object to delete, it's useless to keep the render object list around. */
   if (clean_them)
     {
	eina_array_clean(&e->active_objects);
	eina_array_clean(&e->render_objects);

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
evas_render_updates_free(Eina_List *updates)
{
   Eina_Rectangle *r;

   EINA_LIST_FREE(updates, r)
     eina_rectangle_free(r);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Eina_List *
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

   evas_fonts_zero_presure(e);

   if ((e->engine.func) && (e->engine.func->output_idle_flush) &&
       (e->engine.data.output))
     e->engine.func->output_idle_flush(e->engine.data.output);

   eina_array_flush(&e->delete_objects);
   eina_array_flush(&e->active_objects);
   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->render_objects);
   eina_array_flush(&e->clip_changes);

   e->invalidate = 1;
}

void
evas_render_invalidate(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   eina_array_clean(&e->active_objects);
   eina_array_clean(&e->render_objects);

   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->delete_objects);

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
	  eina_array_push(&e->pending_objects, obj);
	obj->changed = 1;
     }
}

