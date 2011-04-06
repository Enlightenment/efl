#include "evas_common.h"
#include "evas_private.h"

void
evas_object_clip_dirty(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Object *data;

   if (obj->cur.cache.clip.dirty) return ;

   obj->cur.cache.clip.dirty = 1;
   EINA_LIST_FOREACH(obj->clip.clipees, l, data)
     evas_object_clip_dirty(data);
}

void
evas_object_recalc_clippees(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Object *data;

   if (obj->cur.cache.clip.dirty)
     {
	evas_object_clip_recalc(obj);
	EINA_LIST_FOREACH(obj->clip.clipees, l, data)
	  evas_object_recalc_clippees(data);
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

/* aaaaargh (pirate voice) ... notes!
 * 
 * we have a big problem until now that's gone undetected... until yesterday.
 * that problem involves clips and maps and smart objects. hooray! 3 of the
 * more complex bits of evas - and maps and smart objects  being one of the
 * nastiest ones.
 * 
 * what is the problem? when a clip crosses a map boundary. that is to say
 * that when the clipper and clippee are not within the child tree of the
 * mapped object. in this case "bad stuff" happens. basically as clips are
 * then used to render objects, but they no longer apply as you'd expect as
 * the map transfomr the objects to-be-clipped separately from the objects
 * that clip them and this whole relationship is broken by maps. it somehow
 * managed to not break with the advent of smart objects. lucky me... but
 * maps killed it. now... what do we do? that is a good question. detect
 * such a broken link and "turn off clipping" in that event - sure. but this
 * isn't going to be cheap as ANY addition or deletion of a map to an object
 * or any change in clipper of an object or any change in smart object
 * membership needs to walk the obj tree both up and down from the changed
 * object and probably walk entire object trees to find these and mark them.
 * thats silly-expensive and i was about to fix it that way but it has since
 * dawned on me that that is just going to kill performance in some critical
 * areas like during object setup and manipulation, as well as teardown.
 * 
 * aaaaagh! best for now is to document this as a "don't do it damnit!" thing
 * and have the apps avoid it. but even then - how to do this? this is not
 * easy. everywhere i turn so far i come up to either expensive operations,
 * breaks in logic, or nasty re-work of apps or4 the whole concept of clipping,
 * smart objects and maps... and that will have to wait for evas 2.0
 * 
 * the below does clip fixups etc. in the even a clip spans a map boundary.
 * not pretty, but necessary.
 */

#define MAP_ACROSS 1
static void
evas_object_child_map_across_mark(Evas_Object *obj, Evas_Object *map_obj, Eina_Bool force)
{
#ifdef MAP_ACROSS
   if ((obj->cur.map_parent != map_obj) || force)
     {
        obj->cur.map_parent = map_obj;
        obj->cur.cache.clip.dirty = 1;
        evas_object_clip_recalc(obj);
        if (obj->smart.smart)
          {
             Evas_Object *obj2;
             
             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
               {
                  // if obj has its own map - skip it. already done
                  if ((obj2->cur.map) && (obj2->cur.usemap)) continue;
                  evas_object_child_map_across_mark(obj2, map_obj, force);
               }
          }
        else if (obj->clip.clipees)
          {
             Eina_List *l;
             Evas_Object *obj2;
             
             EINA_LIST_FOREACH(obj->clip.clipees, l, obj2)
                evas_object_child_map_across_mark(obj2, map_obj, force);
          }
     }
#endif   
}

void
evas_object_clip_across_check(Evas_Object *obj)
{
#ifdef MAP_ACROSS
   if (!obj->cur.clipper) return;
   if (obj->cur.clipper->cur.map_parent != obj->cur.map_parent)
      evas_object_child_map_across_mark(obj, obj->cur.map_parent, 1);
#endif   
}

void
evas_object_clip_across_clippees_check(Evas_Object *obj)
{
#ifdef MAP_ACROSS
   Eina_List *l;
   Evas_Object *obj2;

   if (!obj->clip.clipees) return;
// schloooooooooooow:
//   evas_object_child_map_across_mark(obj, obj->cur.map_parent, 1);
// buggy:
   evas_object_child_map_across_mark(obj, obj->cur.map_parent, 0);
   if (obj->cur.cache.clip.dirty)
     {
	EINA_LIST_FOREACH(obj->clip.clipees, l, obj2)
           evas_object_clip_across_clippees_check(obj2);
     }
#endif   
}

// this function is called on an object when map is enabled or disabled on it
// thus creating a "map boundary" at that point.
// 
// FIXME: flip2 test broken in elm - might be show/hide of clips
void
evas_object_mapped_clip_across_mark(Evas_Object *obj)
{
#ifdef MAP_ACROSS
   if ((obj->cur.map) && (obj->cur.usemap))
      evas_object_child_map_across_mark(obj, obj, 0);
   else
     {
        if (obj->smart.parent)
           evas_object_child_map_across_mark
           (obj, obj->smart.parent->cur.map_parent, 0);
        else
           evas_object_child_map_across_mark(obj, NULL, 0); 
    }
#endif   
}

/* public functions */

/**
 * @addtogroup Evas_Object_Group_Basic
 * @{
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
	obj->cur.clipper->clip.clipees = eina_list_remove(obj->cur.clipper->clip.clipees, obj);
	if (!obj->cur.clipper->clip.clipees)
	  {
	     obj->cur.clipper->cur.have_clipees = 0;
	     if (obj->cur.clipper->cur.visible)
	       evas_damage_rectangle_add(obj->cur.clipper->layer->evas,
					 obj->cur.clipper->cur.geometry.x,
					 obj->cur.clipper->cur.geometry.y,
					 obj->cur.clipper->cur.geometry.w,
					 obj->cur.clipper->cur.geometry.h);
	  }
	evas_object_change(obj->cur.clipper);
	evas_object_change(obj);
	obj->cur.clipper = NULL;
     }
   /* clip me */
   if ((!clip->clip.clipees) && (clip->cur.visible))
     {
	/* Basically it just went invisible */
	clip->changed = 1;
	clip->layer->evas->changed = 1;
	evas_damage_rectangle_add(clip->layer->evas,
				  clip->cur.geometry.x, clip->cur.geometry.y,
				  clip->cur.geometry.w, clip->cur.geometry.h);
     }
   obj->cur.clipper = clip;
   clip->clip.clipees = eina_list_append(clip->clip.clipees, obj);
   if (clip->clip.clipees) clip->cur.have_clipees = 1;

   /* If it's NOT a rectangle set the mask bits too */
   /* FIXME: Optmz ths chck */
   if (strcmp(evas_object_type_get(clip),"rectangle") == 0)
      obj->cur.mask = NULL;
   else
     {
	void *engdata;
        obj->cur.mask = clip;
	engdata = clip->func->engine_data_get(clip);
	/* FIXME: Images only */
	clip->layer->evas->engine.func->image_mask_create(
				      clip->layer->evas->engine.data.output,
				      engdata);
     }
   evas_object_change(clip);
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_recalc_clippees(obj);
   if ((!obj->smart.smart) && 
       (!((obj->cur.map) && (obj->cur.usemap))))
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
   evas_object_clip_across_check(obj);
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
        obj->cur.clipper->clip.clipees = eina_list_remove(obj->cur.clipper->clip.clipees, obj);
	if (!obj->cur.clipper->clip.clipees)
	  {
	     obj->cur.clipper->cur.have_clipees = 0;
	     if (obj->cur.clipper->cur.visible)
	       evas_damage_rectangle_add(obj->cur.clipper->layer->evas,
					 obj->cur.clipper->cur.geometry.x,
					 obj->cur.clipper->cur.geometry.y,
					 obj->cur.clipper->cur.geometry.w,
					 obj->cur.clipper->cur.geometry.h);
	  }
	evas_object_change(obj->cur.clipper);
     }
   obj->cur.clipper = NULL;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_recalc_clippees(obj);
   if ((!obj->smart.smart) && 
       (!((obj->cur.map) && (obj->cur.usemap))))
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
   evas_object_clip_across_check(obj);
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
 *     Eina_List *clippees, *l;
 *     Evas_Object *obj_tmp;
 *
 *     clippees = evas_object_clipees_get(clipper);
 *     printf("Clipper clips %i objects\n", eina_list_count(clippees));
 *     EINA_LIST_FOREACH(clippees, l, obj_tmp)
 *         evas_object_show(obj_tmp);
 *   }
 * @endcode
 */
EAPI const Eina_List *
evas_object_clipees_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->clip.clipees;
}

/**
 * @}
 */
