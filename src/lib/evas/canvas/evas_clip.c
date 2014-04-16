#include "evas_common_private.h"
#include "evas_private.h"

void
evas_object_clip_dirty(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   Evas_Object_Protected_Data *clipee;
   Eina_List *l;

   if (obj->cur->cache.clip.dirty) return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
       state_write->cache.clip.dirty = EINA_TRUE;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   EINA_LIST_FOREACH(obj->clip.clipees, l, clipee)
     {
        evas_object_clip_dirty(clipee->object, clipee);
     }
}

void
evas_object_recalc_clippees(Evas_Object_Protected_Data *obj)
{
   Evas_Object_Protected_Data *clipee;
   Eina_List *l;

   if (obj->cur->cache.clip.dirty)
     {
        evas_object_clip_recalc(obj);
        EINA_LIST_FOREACH(obj->clip.clipees, l, clipee)
          {
             evas_object_recalc_clippees(clipee);
          }
     }
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
evas_object_child_map_across_mark(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *map_obj, Eina_Bool force, Eina_Hash *visited)
{
#ifdef MAP_ACROSS
   Eina_Bool clear_visited = EINA_FALSE;

   if (!visited)
     {
        visited = eina_hash_pointer_new(NULL);
        clear_visited = EINA_TRUE;
     }
   if (eina_hash_find(visited, &eo_obj) == (void *)1) goto end;
   else eina_hash_add(visited, &eo_obj, (void *)1);
   
   if ((obj->map->cur.map_parent != map_obj) || force)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          map_write->cur.map_parent = map_obj;
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);

	EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
	  {
	    state_write->cache.clip.dirty = 1;
	  }
	EINA_COW_STATE_WRITE_END(obj, state_write, cur);

        evas_object_clip_recalc(obj);
        if (obj->is_smart)
          {
             Evas_Object_Protected_Data *obj2;

             EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), obj2)
               {
                  // if obj has its own map - skip it. already done
                  if ((obj2->map->cur.map) && (obj2->map->cur.usemap)) continue;
                  Evas_Object *eo_obj2 = obj2->object;
                  evas_object_child_map_across_mark(eo_obj2, obj2, map_obj,
                                                    force, visited);
               }
          }
        else if (obj->clip.clipees)
          {
             Evas_Object_Protected_Data *obj2;
             Eina_List *l;

             EINA_LIST_FOREACH(obj->clip.clipees, l, obj2)
               {
                  evas_object_child_map_across_mark(obj2->object, obj2,
                                                    map_obj, force, visited);
               }
          }
     }
end:   
   if (clear_visited) eina_hash_free(visited);
#endif
}

void
evas_object_clip_across_check(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
#ifdef MAP_ACROSS
   if (!obj->cur->clipper) return;
   if (obj->cur->clipper->map->cur.map_parent != obj->map->cur.map_parent)
      evas_object_child_map_across_mark(eo_obj, obj, obj->map->cur.map_parent,
                                        1, NULL);
#endif
}

void
evas_object_clip_across_clippees_check(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
#ifdef MAP_ACROSS
   Evas_Object_Protected_Data *obj2;
   Eina_List *l;

   if (!obj->clip.clipees) return;
// schloooooooooooow:
//   evas_object_child_map_across_mark(eo_obj, obj->map->cur.map_parent, 1, NULL);
// buggy:
   evas_object_child_map_across_mark(eo_obj, obj, obj->map->cur.map_parent, 0,
                                    NULL);
   if (obj->cur->cache.clip.dirty)
     {
	EINA_LIST_FOREACH(obj->clip.clipees, l, obj2)
          {
             evas_object_clip_across_clippees_check(obj2->object, obj2);
          }
     }
#endif
}

// this function is called on an object when map is enabled or disabled on it
// thus creating a "map boundary" at that point.
//
// FIXME: flip2 test broken in elm - might be show/hide of clips
void
evas_object_mapped_clip_across_mark(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
#ifdef MAP_ACROSS
   if ((obj->map->cur.map) && (obj->map->cur.usemap))
      evas_object_child_map_across_mark(eo_obj, obj, eo_obj, 0, NULL);
   else
     {
        if (obj->smart.parent)
          {
             Evas_Object_Protected_Data *smart_parent_obj =
                eo_data_scope_get(obj->smart.parent, EVAS_OBJ_CLASS);
             evas_object_child_map_across_mark
                (eo_obj, obj, smart_parent_obj->map->cur.map_parent, 0, NULL);
          }
        else
           evas_object_child_map_across_mark(eo_obj, obj, NULL, 0, NULL);
    }
#endif
}

/* public functions */
extern const char *o_rect_type;

EOLIAN void
_evas_object_clip_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *eo_clip)
{
   Evas_Object_Protected_Data *clip;
   Evas_Public_Data *e;

   if (!eo_clip)
     {
        evas_object_clip_unset(eo_obj);
        return;
     }

   MAGIC_CHECK(eo_clip, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   clip = eo_data_scope_get(eo_clip, EVAS_OBJ_CLASS);
   if (obj->cur->clipper && obj->cur->clipper->object == eo_clip) return;
   if (eo_obj == eo_clip)
     {
        CRI("Setting clip %p on itself", eo_obj);
        return;
     }
   if (clip->delete_me)
     {
        CRI("Setting deleted object %p as clip obj %p", eo_clip, eo_obj);
        return;
     }
   if (obj->delete_me)
     {
        CRI("Setting object %p as clip to deleted obj %p", eo_clip, eo_obj);
        return;
     }
   if (!obj->layer)
     {
        CRI("No evas surface associated with object (%p)", eo_obj);
        return;
     }
   if ((obj->layer && clip->layer) &&
       (obj->layer->evas != clip->layer->evas))
     {
        CRI("Setting object %p from Evas (%p) to another Evas (%p)", obj, obj->layer->evas, clip->layer->evas);
        return;
     }
   if (!obj->layer || !clip->layer)
     {
        CRI("Object %p or clip %p layer is not set !", obj, clip);
        return;
     }

   if (evas_object_intercept_call_clip_set(eo_obj, obj, eo_clip)) return;
   // illegal to set anything but a rect as a clip
   if (clip->type != o_rect_type)
     {
        ERR("For now a clip on other object than a rectangle is disabled");
        return;
     }
   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_clip_set(eo_clip));
     }
   if (obj->cur->clipper)
     {
	/* unclip */
        obj->cur->clipper->clip.cache_clipees_answer = eina_list_free(obj->cur->clipper->clip.cache_clipees_answer);
        obj->cur->clipper->clip.clipees = eina_list_remove(obj->cur->clipper->clip.clipees, obj);
        if (!obj->cur->clipper->clip.clipees)
          {
             EINA_COW_STATE_WRITE_BEGIN(obj->cur->clipper, state_write, cur)
               {
                  state_write->have_clipees = 0;
               }
             EINA_COW_STATE_WRITE_END(obj->cur->clipper, state_write, cur);
/* i know this was to handle a case where a clip stops having children and
 * becomes a solid colored box - no one ever does that... they hide the clip
 * so dont add damages
             if ((obj->cur->clipper->cur) && (obj->cur->clipper->cur->visible))
               {
                  if (obj->cur->clipper->layer)
                    {
                       e = obj->cur->clipper->layer->evas;
                       evas_damage_rectangle_add(e->evas,
                                                 obj->cur->clipper->cur->geometry.x + e->framespace.x,
                                                 obj->cur->clipper->cur->geometry.y + e->framespace.y,
                                                 obj->cur->clipper->cur->geometry.w,
                                                 obj->cur->clipper->cur->geometry.h);
                    }
               }
 */
          }
        evas_object_change(obj->cur->clipper->object, obj->cur->clipper);
        evas_object_change(eo_obj, obj);

        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          {
             state_write->clipper = NULL;
          }
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);
     }
   /* clip me */
   if ((!clip->clip.clipees) && (clip->cur->visible))
     {
        /* Basically it just went invisible */
        clip->changed = 1;
        e = clip->layer->evas;
        e->changed = 1;
/* i know this was to handle a case where a clip starts having children and
 * stops being a solid colored box - no one ever does that... they hide the clp
 * so dont add damages
        evas_damage_rectangle_add(e->evas,
                                  clip->cur->geometry.x + e->framespace.x,
                                  clip->cur->geometry.y + e->framespace.y,
                                  clip->cur->geometry.w, clip->cur->geometry.h);
 */
     }
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->clipper = clip;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   clip->clip.cache_clipees_answer = eina_list_free(clip->clip.cache_clipees_answer);
   clip->clip.clipees = eina_list_append(clip->clip.clipees, obj);
   if (clip->clip.clipees)
     {
        EINA_COW_STATE_WRITE_BEGIN(clip, state_write, cur)
          {
             state_write->have_clipees = 1;
          }
        EINA_COW_STATE_WRITE_END(clip, state_write, cur);

        if (clip->changed)
          evas_object_update_bounding_box(eo_clip, clip);
     }

   /* If it's NOT a rectangle set the mask bits too */
   /* FIXME: Optmz ths chck */
   if (clip->type != o_rect_type)
     {
        ERR("Not supported clipping to type '%s', just rectangles.",
            clip->type);
     }
   evas_object_change(eo_clip, clip);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_recalc_clippees(obj);
   if ((!obj->is_smart) &&
       (!((obj->map->cur.map) && (obj->map->cur.usemap))))
     {
        if (evas_object_is_in_output_rect(eo_obj, obj,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y, 1, 1))
          evas_event_feed_mouse_move(obj->layer->evas->evas,
                                     obj->layer->evas->pointer.x,
                                     obj->layer->evas->pointer.y,
                                     obj->layer->evas->last_timestamp,
                                     NULL);
     }
   evas_object_clip_across_check(eo_obj, obj);
}

EOLIAN Evas_Object *
_evas_object_clip_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->cur->clipper)
     return obj->cur->clipper->object;
   return NULL;
}

EOLIAN void
_evas_object_clip_unset(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (!obj->cur->clipper) return;

   obj->clip.cache_clipees_answer = eina_list_free(obj->clip.cache_clipees_answer);

   /* unclip */
   if (evas_object_intercept_call_clip_unset(eo_obj, obj)) return;
   if (obj->is_smart)
     {
        eo_do(eo_obj, evas_obj_smart_clip_unset());
     }
   if (obj->cur->clipper)
     {
        obj->cur->clipper->clip.clipees = eina_list_remove(obj->cur->clipper->clip.clipees, obj);
        if (!obj->cur->clipper->clip.clipees)
          {
             EINA_COW_STATE_WRITE_BEGIN(obj->cur->clipper, state_write, cur)
               {
                  state_write->have_clipees = 0;
               }
             EINA_COW_STATE_WRITE_END(obj->cur->clipper, state_write, cur);
/* i know this was to handle a case where a clip stops having children and
 * becomes a solid colored box - no one ever does that... they hide the clip
 * so dont add damages
             if ((obj->cur->clipper->cur) && (obj->cur->clipper->cur->visible))
               {
                  if (obj->cur->clipper->layer)
                    {
                       Evas_Public_Data *e = obj->cur->clipper->layer->evas;
                       evas_damage_rectangle_add(e->evas,
                                                 obj->cur->clipper->cur->geometry.x + e->framespace.x,
                                                 obj->cur->clipper->cur->geometry.y + e->framespace.y,
                                                 obj->cur->clipper->cur->geometry.w,
                                                 obj->cur->clipper->cur->geometry.h);
                    }
               }
 */
          }
	evas_object_change(obj->cur->clipper->object, obj->cur->clipper);
     }
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->clipper = NULL;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_recalc_clippees(obj);
   if ((!obj->is_smart) &&
       (!((obj->map->cur.map) && (obj->map->cur.usemap))))
     {
        if (evas_object_is_in_output_rect(eo_obj, obj,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y, 1, 1))
          evas_event_feed_mouse_move(obj->layer->evas->evas,
                                     obj->layer->evas->pointer.x,
                                     obj->layer->evas->pointer.y,
                                     obj->layer->evas->last_timestamp,
                                     NULL);
     }
   evas_object_clip_across_check(eo_obj, obj);
}

EOLIAN Eina_List *
_evas_object_clipees_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   const Evas_Object_Protected_Data *tmp;
   Eina_List *l;
   Eina_List *answer = NULL;

   obj->clip.cache_clipees_answer = eina_list_free(obj->clip.cache_clipees_answer);

   EINA_LIST_FOREACH(obj->clip.clipees, l, tmp)
     answer = eina_list_append(answer, tmp);

   obj->clip.cache_clipees_answer = answer;
   return answer;
}

EOLIAN Eina_Bool
_evas_object_clipees_has(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return (obj->clip.clipees ? EINA_TRUE : EINA_FALSE);
}

