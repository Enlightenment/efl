#include "evas_common.h"
#include "evas_private.h"

void
evas_object_clip_dirty(Evas_Object *obj)
{
   Evas_List *l;

   obj->cur.cache.clip.dirty = 1;
   for (l = obj->clip.clipees; l; l = l->next)
     evas_object_clip_dirty(l->data);
}

void
evas_object_recalc_clippees(Evas_Object *obj)
{
   Evas_List *l;

   if (obj->cur.cache.clip.dirty)
     {
	evas_object_clip_recalc(obj);
	for (l = obj->clip.clipees; l; l = l->next)
	  evas_object_recalc_clippees(l->data);
     }
}

int
evas_object_clippers_was_visible(Evas_Object *obj)
{
   if (obj->prev.visible)
     {
	if (obj->prev.clipper)
	  return evas_object_clippers_is_visible(obj->prev.clipper);
	return 1;
     }
   return 0;
}

/* public functions */

/**
 * @defgroup Evas_Clip_Group Clip Functions
 *
 * Functions that manage the clipping of objects by other objects.
 */

/**
 * Clip one object to another.
 * @param obj The object to be clipped
 * @param clip The object to clip @p obj by
 *
 * This function will clip the object @p obj to the area occupied by the
 * object @p clipper. This means the object @p obj will only be visible within
 * the area occupied by the clipping object (@p clip). The color of the object
 * being clipped will be multiplied by the color of the clipping object, so
 * the resulting color for the clipped object is
 * RESULT = (OBJ * CLIP) / (255 * 255) per color element (red, green, blue and
 * alpha). Clipping is recursive, so clip objects may be clipped by other
 * objects, and their color will in tern be multiplied. You may NOT set up
 * circular clipping lists (i.e. object 1 clips object 2 which clips object 1).
 * The behavior of Evas is undefined in this case. Objects which do not clip
 * others are visible as normal, those that clip 1 or more objects become
 * invisible themselves, only affecting what they clip. If an object ceases to
 * have other objects being clipped by it, it will become visible again. The
 * visibility of an object affects the objects that are clipped by it, so if
 * the object clipping others is not shown, the objects clipped will not be
 * shown either. If the object was being clipped by another object when this
 * function is called, it is implicitly removed from the clipper it is being
 * clipped to, and now is made to clip its new clipper.
 *
 * At the moment the only objects that can validly be used to clip other
 * objects are rectangle objects. All other object types are invalid and the
 * result of using them is undefined.
 *
 * The clip object @p clip must be a valid object, but may also be NULL in
 * which case the effect of this function is the same as calling
 * evas_object_clip_unset() on the @p obj object.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 *
 * clipper = evas_object_rectangle_add(evas);
 * evas_object_color_set(clipper, 255, 255, 255, 255);
 * evas_object_move(clipper, 10, 10);
 * evas_object_resize(clipper, 20, 50);
 * evas_object_clip_set(obj, clipper);
 * evas_object_show(clipper);
 * @endcode
 *
 * @ingroup Evas_Clip_Group
 */
EAPI void
evas_object_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!clip)
     {
	evas_object_clip_unset(obj);
	return;
     }
   MAGIC_CHECK(clip, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->cur.clipper == clip) return;
   if (obj == clip) return;
   if (evas_object_intercept_call_clip_set(obj, clip)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->clip_set)
	  obj->smart.smart->smart_class->clip_set(obj, clip);
     }
   if (obj->cur.clipper)
     {
	/* unclip */
	obj->cur.clipper->clip.clipees = evas_list_remove(obj->cur.clipper->clip.clipees, obj);
	if (!obj->cur.clipper->clip.clipees) obj->cur.clipper->cur.have_clipees = 0;
	evas_object_change(obj->cur.clipper);
	evas_object_change(obj);
	obj->cur.clipper = NULL;
     }
   /* clip me */
   if ((clip->clip.clipees == NULL) && (clip->cur.visible))
     {
	/* Basically it just went invisible */
	clip->changed = 1;
	clip->layer->evas->changed = 1;
	evas_damage_rectangle_add(clip->layer->evas,
				  clip->cur.geometry.x, clip->cur.geometry.y,
				  clip->cur.geometry.w, clip->cur.geometry.h);
     }
   obj->cur.clipper = clip;
   clip->clip.clipees = evas_list_append(clip->clip.clipees, obj);
   if (clip->clip.clipees) clip->cur.have_clipees = 1;
   evas_object_change(clip);
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_recalc_clippees(obj);
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

/**
 * Get the object clipping this one (if any).
 * @param obj The object to get the clipper from
 *
 * This function returns the the object clipping @p obj. If @p obj not being
 * clipped, NULL is returned. The object @p obj must be a valid object.
 *
 * See also evas_object_clip_set(), evas_object_clip_unset() and
 * evas_object_clipees_get().
 *
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 *
 * clipper = evas_object_clip_get(obj);
 * if (clipper) evas_object_show(clipper);
 * @endcode
 *
 * @ingroup Evas_Clip_Group
 */
EAPI Evas_Object *
evas_object_clip_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->cur.clipper;
}

/**
 * Disable clipping for an object.
 *
 * @param obj The object to cease clipping on
 *
 * This function disables clipping for the object @p obj, if it was already
 * clipped. If it wasn't, this has no effect. The object @p obj must be a
 * valid object.
 *
 * See also evas_object_clip_set(), evas_object_clipees_get() and
 * evas_object_clip_get().
 *
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 *
 * clipper = evas_object_clip_get(obj);
 * if (clipper)
 *   {
 *     evas_object_clip_unset(obj);
 *     evas_object_hide(obj);
 *   }
 * @endcode
 *
 * @ingroup Evas_Clip_Group
 */
EAPI void
evas_object_clip_unset(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!obj->cur.clipper) return;
   /* unclip */
   if (evas_object_intercept_call_clip_unset(obj)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->clip_unset)
	  obj->smart.smart->smart_class->clip_unset(obj);
     }
   if (obj->cur.clipper) 
     {
        obj->cur.clipper->clip.clipees = evas_list_remove(obj->cur.clipper->clip.clipees, obj);
	if (!obj->cur.clipper->clip.clipees) 
	  obj->cur.clipper->cur.have_clipees = 0;
	evas_object_change(obj->cur.clipper);
     }
   obj->cur.clipper = NULL;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_recalc_clippees(obj);
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

/**
 * Return a list of objects currently clipped by a specific object.
 *
 * @param obj The object to get a list of clippees from
 *
 * This returns the inernal list handle that contains all objects clipped by
 * the object @p obj. If none are clipped, it returns NULL. This list is only
 * valid until the clip list is changed and should be fetched again with another
 * call to evas_object_clipees_get() if any objects being clipped by this object
 * are unclipped, clipped by a new object, are deleted or the clipper is
 * deleted.  These operations will invalidate the list returned so it should
 * not be used anymore after that point. Any use of the list after this may have
 * undefined results, not limited just to strange behavior but possible
 * segfaults and other strange memory errors. The object @p obj must be a valid
 * object.
 *
 * See also evas_object_clip_set(), evas_object_clip_unset() and
 * evas_object_clip_get().
 *
 * Example:
 * @code
 * extern Evas_Object *obj;
 * Evas_Object *clipper;
 *
 * clipper = evas_object_clip_get(obj);
 * if (clipper)
 *   {
 *     Evas_List *clippees, *l;
 *
 *     clippees = evas_object_clipees_get(clipper);
 *     printf("Clipper clips %i objects\n", evas_list_count(clippees));
 *     for (l = clippees; l; l = l->next)
 *       {
 *         Evas_Object *obj_tmp;
 *
 *         obj_tmp = l->data;
 *         evas_object_show(obj_tmp);
 *       }
 *   }
 * @endcode
 * @ingroup Evas_Clip_Group
 */
EAPI const Evas_List *
evas_object_clipees_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->clip.clipees;
}
