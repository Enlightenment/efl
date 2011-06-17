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
extern const char *o_rect_type;

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
   // illegal to set anything but a rect as a clip
   if (clip->type != o_rect_type)
     {
        ERR("For now a clip on other object than a rectangle is disabled\n");
        return;
     }
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

EAPI Evas_Object *
evas_object_clip_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->cur.clipper;
}

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

EAPI const Eina_List *
evas_object_clipees_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->clip.clipees;
}
