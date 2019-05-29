#include "evas_common_private.h"
#include "evas_private.h"

static void _clip_unset(Eo *eo_obj, Evas_Object_Protected_Data *obj);

void
evas_object_clip_dirty_do(Evas_Object_Protected_Data *obj)
{
   Evas_Object_Protected_Data *clipee;
   Eina_List *l;

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

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
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
                efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
             evas_object_child_map_across_mark
                (eo_obj, obj, smart_parent_obj->map->cur.map_parent, 0, NULL);
          }
        else
           evas_object_child_map_across_mark(eo_obj, obj, NULL, 0, NULL);
    }
#endif
}

static void
_efl_canvas_object_clipper_mask_unset(Evas_Object_Protected_Data *obj)
{
   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   if (!obj->mask->is_mask) return;
   if (obj->clip.clipees) return;

   /* this frees the clip surface. is this correct? */
   EINA_COW_WRITE_BEGIN(evas_object_mask_cow, obj->mask, Evas_Object_Mask_Data, mask)
     mask->is_mask = EINA_FALSE;
     mask->redraw = EINA_FALSE;
     mask->is_alpha = EINA_FALSE;
     if (mask->surface)
       {
          obj->layer->evas->engine.func->image_free(ENC, mask->surface);
          mask->surface = NULL;
       }
     mask->w = 0;
     mask->h = 0;
   EINA_COW_WRITE_END(evas_object_mask_cow, obj->mask, mask);
}

/* public functions */
extern const char *o_rect_type;
extern const char *o_image_type;

static void _clipper_invalidated_cb(void *data, const Efl_Event *event);

Eina_Bool
_efl_canvas_object_clipper_set_block(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                  Evas_Object *eo_clip, Evas_Object_Protected_Data *clip)
{
   if (!obj) obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (!clip) clip = efl_data_scope_get(eo_clip, EFL_CANVAS_OBJECT_CLASS);

   evas_object_async_block(obj);

   if (obj->cur->clipper && (obj->cur->clipper->object == eo_clip)) return EINA_TRUE;
   if (eo_obj == eo_clip) goto err_same;
   if (clip->delete_me) goto err_clip_deleted;
   if (obj->delete_me) goto err_obj_deleted;
   if (!obj->layer || !clip->layer) goto err_no_layer;
   if (obj->layer->evas != clip->layer->evas) goto err_diff_evas;
   if ((clip->type != o_rect_type) && (clip->type != o_image_type)) goto err_type;

   return EINA_FALSE;

err_same:
   CRI("Setting clip %p on itself", eo_obj);
   return EINA_TRUE;
err_clip_deleted:
   CRI("Setting deleted object %p as clip obj %p", eo_clip, eo_obj);
   return EINA_TRUE;
err_obj_deleted:
   CRI("Setting object %p as clip to deleted obj %p", eo_clip, eo_obj);
   return EINA_TRUE;
err_no_layer:
   CRI("Object %p or clip %p layer is not set !", obj, clip);;
   return EINA_TRUE;
err_diff_evas:
   CRI("Setting object %p from Evas (%p) to another Evas (%p)",
       obj, obj->layer->evas, clip->layer->evas);
   return EINA_TRUE;
err_type:
   CRI("A clipper can only be an evas rectangle or image (got %s)",
       efl_class_name_get(eo_clip));
   return EINA_TRUE;
}

static inline void
_efl_canvas_object_clipper_unset_common(Evas_Object_Protected_Data *obj, Eina_Bool warn)
{
   Evas_Object_Protected_Data *clip = obj->cur->clipper;

   if (!clip) return;
   if (EVAS_OBJECT_DATA_VALID(clip))
     {
        clip->clip.cache_clipees_answer = eina_list_free(clip->clip.cache_clipees_answer);
        clip->clip.clipees = eina_list_remove(clip->clip.clipees, obj);
        if (!clip->clip.clipees)
          {
             EINA_COW_STATE_WRITE_BEGIN(clip, state_write, cur)
               {
                  state_write->have_clipees = 0;
                  if (warn && clip->is_static_clip)
                    {
                       WRN("You override static clipper, it may be dangled! "
                           "obj(%p) type(%s) new clip(%p)",
                           obj->object, obj->type, clip->object);
                    }
               }
             EINA_COW_STATE_WRITE_END(clip, state_write, cur);
             /* i know this was to handle a case where a clip stops having
              * children and becomes a solid colored box - no one ever does
              * that... they hide the clip so dont add damages,
              * But, if the clipper could affect color to its clipees, the
              * clipped area should be redrawn. */
             if (((clip->cur) && (clip->cur->visible)) &&
                 (((clip->cur->color.r != 255) || (clip->cur->color.g != 255) ||
                   (clip->cur->color.b != 255) || (clip->cur->color.a != 255)) ||
                  (clip->mask->is_mask)) &&
                  efl_alive_get(clip->object))
               {
                  if (clip->layer)
                    {
                       Evas_Public_Data *e = clip->layer->evas;
                       evas_damage_rectangle_add(e->evas,
                                                 clip->cur->geometry.x + e->framespace.x,
                                                 clip->cur->geometry.y + e->framespace.y,
                                                 clip->cur->geometry.w,
                                                 clip->cur->geometry.h);
                    }
               }

             _efl_canvas_object_clipper_mask_unset(clip);
          }
        evas_object_change(clip->object, clip);
        if (obj->prev->clipper != clip)
          efl_event_callback_del(clip->object, EFL_EVENT_INVALIDATE, _clipper_invalidated_cb, obj->object);
     }

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     state_write->clipper = NULL;
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
}

EOLIAN void
_efl_canvas_object_clipper_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *eo_clip)
{
   Evas_Object_Protected_Data *clip;

   EVAS_OBJECT_DATA_ALIVE_CHECK(obj);

   if ((!obj->cur->clipper && !eo_clip) ||
       (obj->cur->clipper && obj->cur->clipper->object == eo_clip)) return;

   clip = EVAS_OBJECT_DATA_SAFE_GET(eo_clip);
   if (!EVAS_OBJECT_DATA_ALIVE(clip))
     {
        _clip_unset(eo_obj, obj);
        return;
     }

   if (_efl_canvas_object_clipper_set_block(eo_obj, obj, eo_clip, clip)) return;
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_CLIP_SET, 1, eo_clip)) return;

   if (obj->is_smart && obj->smart.smart && obj->smart.smart->smart_class &&
       obj->smart.smart->smart_class->clip_set)
     {
        obj->smart.smart->smart_class->clip_set(eo_obj, eo_clip);
     }

   /* unset cur clipper */
   _efl_canvas_object_clipper_unset_common(obj, EINA_TRUE);

   /* image object clipper */
   if (clip->type == o_image_type)
     {
        EINA_COW_WRITE_BEGIN(evas_object_mask_cow, clip->mask, Evas_Object_Mask_Data, mask)
          mask->is_mask = EINA_TRUE;
        EINA_COW_WRITE_END(evas_object_mask_cow, clip->mask, mask);
     }

   /* clip me */
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     state_write->clipper = clip;
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
   if (obj->prev->clipper != clip)
     efl_event_callback_add(clip->object, EFL_EVENT_INVALIDATE, _clipper_invalidated_cb, eo_obj);

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
          evas_object_update_bounding_box(eo_clip, clip, NULL);
     }

   evas_object_change(eo_clip, clip);
   evas_object_change(eo_obj, obj);
   evas_object_update_bounding_box(eo_obj, obj, NULL);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_recalc_clippees(obj);
   if ((!obj->is_smart) &&
       (!((obj->map->cur.map) && (obj->map->cur.usemap))) &&
       evas_object_is_visible(obj))
     {
        _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                           eo_obj,
                                                           obj, 1, 1,
                                                           EINA_FALSE,
                                                           NULL);
     }
   evas_object_clip_across_check(eo_obj, obj);
}

EOLIAN Evas_Object *
_efl_canvas_object_clipper_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   EVAS_OBJECT_DATA_ALIVE_CHECK(obj, NULL);
   if (obj->cur->clipper)
     return obj->cur->clipper->object;
   return NULL;
}

Eina_Bool
_efl_canvas_object_clipper_unset_block(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (!obj->cur->clipper)
     return EINA_TRUE;

   evas_object_async_block(obj);
   obj->clip.cache_clipees_answer = eina_list_free(obj->clip.cache_clipees_answer);

   return EINA_FALSE;
}

static void
_clip_unset(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (_efl_canvas_object_clipper_unset_block(eo_obj, obj)) return;
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_CLIP_SET, 1, NULL)) return;
   if (obj->is_smart && obj->smart.smart && obj->smart.smart->smart_class &&
       obj->smart.smart->smart_class->clip_unset)
     {
        obj->smart.smart->smart_class->clip_unset(eo_obj);
     }
   _efl_canvas_object_clipper_unset_common(obj, EINA_FALSE);
   evas_object_update_bounding_box(eo_obj, obj, NULL);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_recalc_clippees(obj);

   if ((!obj->is_smart) &&
       (!((obj->map->cur.map) && (obj->map->cur.usemap))) &&
       evas_object_is_visible(obj))
     {
        _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                           eo_obj,
                                                           obj, 1, 1,
                                                           EINA_FALSE,
                                                           NULL);
     }
   evas_object_clip_across_check(eo_obj, obj);
}

EAPI void
evas_object_clip_unset(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   EVAS_OBJECT_DATA_ALIVE_CHECK(obj);
   _clip_unset(eo_obj, obj);
}

static void
_clipper_invalidated_cb(void *data, const Efl_Event *event)
{
   Evas_Object *eo_obj = data;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *clip = efl_data_scope_get(event->object, EFL_CANVAS_OBJECT_CLASS);

   EVAS_OBJECT_DATA_ALIVE_CHECK(obj);

   if (EINA_LIKELY(obj->cur->clipper && (obj->cur->clipper == clip)))
     _clip_unset(eo_obj, obj);
   if (obj->prev->clipper && (obj->prev->clipper == clip))
     {
        // not removing cb since it's the del cb... it can't be called again!
        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, prev)
          state_write->clipper = NULL;
        EINA_COW_STATE_WRITE_END(obj, state_write, prev);
     }
}

void
_efl_canvas_object_clipper_prev_reset(Evas_Object_Protected_Data *obj, Eina_Bool cur_prev)
{
   if (obj->prev->clipper)
     {
        Evas_Object_Protected_Data *clip = obj->prev->clipper;
        if (!cur_prev)
          {
             EINA_COW_STATE_WRITE_BEGIN(obj, state_write, prev)
               state_write->clipper = NULL;
             EINA_COW_STATE_WRITE_END(obj, state_write, prev);
          }
        if (clip != obj->cur->clipper)
          efl_event_callback_del(clip->object, EFL_EVENT_INVALIDATE, _clipper_invalidated_cb, obj->object);
     }
}

EAPI const Eina_List *
evas_object_clipees_get(const Evas_Object *eo_obj)
{
   const Evas_Object_Protected_Data *tmp;
   Eina_List *l;
   Eina_List *answer = NULL;

   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, NULL);
   obj->clip.cache_clipees_answer = eina_list_free(obj->clip.cache_clipees_answer);

   EINA_LIST_FOREACH(obj->clip.clipees, l, tmp)
     answer = eina_list_append(answer, tmp->object);

   obj->clip.cache_clipees_answer = answer;
   return answer;
}

EAPI Eina_Bool
evas_object_clipees_has(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   return !!obj->clip.clipees;
}

typedef struct
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Evas_Object   *object;
} Clipee_Iterator;

static Eina_Bool
_clipee_iterator_next(Clipee_Iterator *it, void **data)
{
   Evas_Object_Protected_Data *sub;

   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub ? sub->object : NULL;
   return EINA_TRUE;
}

static void *
_clipee_iterator_get_container(Clipee_Iterator *it)
{
   return it->object;
}

static void
_clipee_iterator_free(Clipee_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

EOLIAN Eina_Iterator *
_efl_canvas_object_clipped_objects_get(const Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   Clipee_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = obj->clip.clipees;
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_clipee_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_clipee_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_clipee_iterator_free);
   it->object = (Evas_Object *)eo_obj;

   return &it->iterator;
}

EOLIAN unsigned int
_efl_canvas_object_clipped_objects_count(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return eina_list_count(obj->clip.clipees);
}

EOLIAN void
_efl_canvas_object_no_render_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool enable)
{
   obj->no_render = !!enable;
}

EOLIAN Eina_Bool
_efl_canvas_object_no_render_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->no_render;
}
