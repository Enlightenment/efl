#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif


#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"
#include "efl_ui_position_manager_common.h"

#define MY_CLASS      EFL_UI_POSITION_MANAGER_LIST_CLASS
#define MY_DATA_GET(obj, pd) \
  Efl_Ui_Position_Manager_List_Data *pd = efl_data_scope_get(obj, MY_CLASS);

typedef struct {
   Api_Callbacks callbacks;

   Eina_Future *rebuild_absolut_size;
   int *size_cache;
   Efl_Gfx_Entity *last_group;
   Efl_Ui_Win *window;
   Evas *canvas;

   Vis_Segment prev_run;

   Eina_Rect viewport;
   Eina_Size2D abs_size;
   Eina_Vector2 scroll_position;

   Efl_Ui_Layout_Orientation dir;

   unsigned int size;
   int average_item_size;
   int maximum_min_size;
} Efl_Ui_Position_Manager_List_Data;

/*
 * The here used cache is a sum map
 * Every element in the cache contains the sum of the previous element, and the size of the current item
 * This is usefull as a lookup of all previous items is O(1).
 * The tradeoff that makes the cache performant here is, that we only need to walk the whole list of items once in the beginning.
 * Every other walk of the items is at max the maximum number of items you get into the maximum distance between the average item size and a actaul item size.
 */

static void
cache_invalidate(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd)
{
   if (pd->size_cache)
     free(pd->size_cache);
   pd->size_cache = NULL;
}

static void
cache_require(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd)
{
   unsigned int i;
   const int len = 100;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[len];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;

   if (pd->size_cache) return;

   if (pd->size == 0)
     {
        pd->size_cache = NULL;
        pd->average_item_size = 0;
        return;
     }

   pd->size_cache = calloc(pd->size + 1, sizeof(int));
   if (!pd->size_cache) return;
   pd->size_cache[0] = 0;
   pd->maximum_min_size = 0;

   for (i = 0; i < pd->size; ++i)
     {
        Eina_Size2D size;
        int step;
        int min;
        int buffer_id = i % len;

        if (buffer_id == 0)
          {
             BATCH_ACCESS_SIZE(pd->callbacks, i, pd->size, MIN(len, pd->size - i), EINA_TRUE, size_buffer);
          }
       size = size_buffer[buffer_id].size;

        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          {
             step = size.h;
             min = size.w;
          }
        else
          {
             step = size.w;
             min = size.h;
          }
        pd->size_cache[i + 1] = pd->size_cache[i] + step;
        pd->maximum_min_size = MAX(pd->maximum_min_size, min);
        /* no point iterating further if size calc can't be done yet */
        //if ((!i) && (!pd->maximum_min_size)) break;
     }
   pd->average_item_size = pd->size_cache[pd->size]/pd->size;
   if ((!pd->average_item_size) && (!pd->maximum_min_size))
     cache_invalidate(obj, pd);
}

static int
cache_access(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd, unsigned int idx)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(idx <= pd->size, 0);
   return pd->size_cache[idx];
}

static void
recalc_absolut_size(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd)
{
   Eina_Size2D min_size = EINA_SIZE2D(-1, -1);
   Eina_Size2D pabs_size = pd->abs_size;
   int pmin_size = pd->maximum_min_size;

   cache_require(obj, pd);
   /* deferred */
   if (!pd->size_cache) return;

   pd->abs_size = pd->viewport.size;

   if (pd->size)
     {
        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          pd->abs_size.h = MAX(cache_access(obj, pd, pd->size), pd->abs_size.h);
        else
          pd->abs_size.w = MAX(cache_access(obj, pd, pd->size), pd->abs_size.w);
     }
   if ((pabs_size.w != pd->abs_size.w) || (pabs_size.h != pd->abs_size.h))
     efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, &pd->abs_size);

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        min_size.w = pd->maximum_min_size;
     }
   else
     {
        min_size.h = pd->maximum_min_size;
     }
   if ((pd->maximum_min_size > 0) && (pd->maximum_min_size != pmin_size))
     efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, &min_size);
}

static inline Vis_Segment
_search_visual_segment(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, int relevant_space_size, int relevant_viewport)
{
   Vis_Segment cur;
   //based on the average item size, we jump somewhere into the sum cache.
   //After beeing in there, we are walking back, until we have less space then viewport size
   cur.start_id = MIN((unsigned int)(relevant_space_size / pd->average_item_size), pd->size);
   for (; cache_access(obj, pd, cur.start_id) >= relevant_space_size && cur.start_id > 0; cur.start_id --) { }

   //starting on the start id, we are walking down until the sum of elements is bigger than the lower part of the viewport.
   cur.end_id = cur.start_id;
   for (; cur.end_id <= pd->size && cache_access(obj, pd, cur.end_id) <= relevant_space_size + relevant_viewport ; cur.end_id ++) { }
   cur.end_id = MAX(cur.end_id, cur.start_id + 1);
   cur.end_id = MIN(cur.end_id, pd->size);

   #ifdef DEBUG
   printf("space_size %d : starting point : %d : cached_space_starting_point %d end point : %d cache_space_end_point %d\n", space_size.h, cur.start_id, pd->size_cache[cur.start_id], cur.end_id, pd->size_cache[cur.end_id]);
   #endif
   if (relevant_space_size > 0)
     EINA_SAFETY_ON_FALSE_GOTO(cache_access(obj, pd, cur.start_id) <= relevant_space_size, err);
   if (cur.end_id != pd->size)
     EINA_SAFETY_ON_FALSE_GOTO(cache_access(obj, pd, cur.end_id) >= relevant_space_size + relevant_viewport, err);
   EINA_SAFETY_ON_FALSE_GOTO(cur.start_id <= cur.end_id, err);

   return cur;

err:
   cur.start_id = cur.end_id = 0;

   return cur;
}

static inline void
_position_items(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd, Vis_Segment new, int relevant_space_size)
{
   Efl_Gfx_Entity *first_group = NULL, *first_fully_visual_group = NULL;
   Eina_Size2D first_group_size;
   Eina_Rect geom;
   const int len = 100;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[len];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;
   Efl_Ui_Position_Manager_Object_Batch_Entity obj_buffer[len];
   Efl_Ui_Position_Manager_Object_Batch_Result object_result;
   unsigned int i;

   //placement of the plain items
   geom = pd->viewport;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     geom.y -= (relevant_space_size - cache_access(obj, pd, new.start_id));
   else
     geom.x -= (relevant_space_size - cache_access(obj, pd, new.start_id));

   evas_event_freeze(pd->canvas);

   for (i = new.start_id; i < new.end_id; ++i)
     {
        Eina_Size2D size;
        Efl_Gfx_Entity *ent = NULL;
        int buffer_id = (i-new.start_id) % len;

        if (buffer_id == 0)
          {
             BATCH_ACCESS_SIZE(pd->callbacks, i, new.end_id, len, EINA_FALSE, size_buffer);
             BATCH_ACCESS_OBJECT(pd->callbacks, i, new.end_id, len, obj_buffer);

             if (i == new.start_id)
               {
                  first_group = object_result.group;
                  first_group_size = size_result.parent_size;
                  if (obj_buffer[0].depth_leader)
                    {
                       first_group = obj_buffer[0].entity;
                       first_group_size = size_buffer[0].size;
                    }
               }
          }

        size = size_buffer[buffer_id].size;
        ent = obj_buffer[buffer_id].entity;

        int diff = cache_access(obj, pd, i + 1) - cache_access(obj, pd, i);
        int real_diff = 0;
        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          real_diff = size.h;
        else
          real_diff = size.w;
        if (real_diff != diff)
          ERR("Reported sizes changed during caching and placement %d %d %d", i, real_diff, diff);

        if (ent == pd->last_group)
          {
             pd->last_group = NULL;
          }

        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          geom.h = size.h;
        else
          geom.w = size.w;

        if (!first_fully_visual_group && obj_buffer[buffer_id].depth_leader &&
             eina_spans_intersect(geom.x, geom.w, pd->viewport.x, pd->viewport.w) &&
             eina_spans_intersect(geom.y, geom.h, pd->viewport.y, pd->viewport.h))
          {
             first_fully_visual_group = obj_buffer[buffer_id].entity;
          }

        if (ent)
          {
             const char *signal;
             efl_gfx_entity_visible_set(ent, EINA_TRUE);
             efl_gfx_entity_geometry_set(ent, geom);
             if (i % 2 == 0)
               signal = "efl,state,even";
             else
               signal = "efl,state,odd";
             efl_layout_signal_emit(ent, signal, "efl");
          }
        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          geom.y += size.h;
        else
          geom.x += size.w;
     }
   //Now place group items
   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     first_group_size.w = pd->viewport.w;
   else
     first_group_size.h = pd->viewport.h;

   //if there is a new group item, display the new one, and hide the old one
   if (first_group != pd->last_group)
     {
        efl_gfx_entity_visible_set(pd->last_group, EINA_FALSE);
        efl_gfx_stack_raise_to_top(first_group);
        pd->last_group = first_group;
     }
   //we have to set the visibility again here, as changing the visual segments might overwrite our visibility state
   efl_gfx_entity_visible_set(first_group, EINA_TRUE);

   //in case there is another group item coming in, the new group item (which is placed as normal item) moves the group item to the top
   Eina_Position2D first_group_pos = EINA_POSITION2D(pd->viewport.x, pd->viewport.y);
   if (first_fully_visual_group && first_fully_visual_group != first_group)
     {
        Eina_Position2D first_visual_group;
        first_visual_group = efl_gfx_entity_position_get(first_fully_visual_group);
        if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
          first_group_pos.y = MIN(first_group_pos.y, first_visual_group.y - first_group_size.h);
        else
          first_group_pos.x = MIN(first_group_pos.x, first_visual_group.x - first_group_size.w);
     }

   efl_gfx_entity_position_set(first_group, first_group_pos);
   efl_gfx_entity_size_set(first_group, first_group_size);

   evas_event_thaw(pd->canvas);
   evas_event_thaw_eval(pd->canvas);
}


static void
position_content(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd)
{
   Eina_Size2D space_size;
   Vis_Segment cur;
   int relevant_space_size, relevant_viewport;
   Efl_Ui_Position_Manager_Range_Update ev;

   if (!pd->size) return;
   if (pd->average_item_size <= 0) return;

   cache_require(obj, pd);

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->abs_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->abs_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
        relevant_viewport = pd->viewport.h;
     }
   else
     {
        relevant_space_size = space_size.w;
        relevant_viewport = pd->viewport.w;
     }

   cur = _search_visual_segment(obj, pd, relevant_space_size, relevant_viewport);
   //to performance optimize the whole widget, we are setting the objects that are outside the viewport to visibility false
   //The code below ensures that things outside the viewport are always hidden, and things inside the viewport are visible
   vis_segment_swap(pd->callbacks, cur, pd->prev_run);

   _position_items(obj, pd, cur, relevant_space_size);

   if (pd->prev_run.start_id != cur.start_id || pd->prev_run.end_id != cur.end_id)
     {
        ev.start_id = pd->prev_run.start_id = cur.start_id;
        ev.end_id = pd->prev_run.end_id = cur.end_id;
        efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_VISIBLE_RANGE_CHANGED, &ev);
     }

}

static Eina_Value
_rebuild_job_cb(Eo *obj, void *data, const Eina_Value v)
{
   Efl_Ui_Position_Manager_List_Data *pd = data;

   cache_require(obj, pd);
   recalc_absolut_size(obj, pd);
   position_content(obj, pd);

   return v;
}

static void
_rebuild_job_free(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Position_Manager_List_Data *pd = data;

   pd->rebuild_absolut_size = NULL;
}

static void
schedule_recalc_absolut_size(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd)
{
   if (pd->rebuild_absolut_size) return;

   pd->rebuild_absolut_size = efl_future_then(obj, efl_loop_job(efl_app_main_get()),
                                              .success = _rebuild_job_cb,
                                              .data = pd,
                                              .free = _rebuild_job_free);
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_entity_viewport_set(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, Eina_Rect size)
{
   if (pd->viewport.x == size.x &&
       pd->viewport.y == size.y &&
       pd->viewport.w == size.w &&
       pd->viewport.h == size.h)
     return;

   pd->viewport = size;

   recalc_absolut_size(obj, pd);
   position_content(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_entity_scroll_position_set(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, double x, double y)
{
   pd->scroll_position.x = x;
   pd->scroll_position.y = y;
   position_content(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_entity_item_added(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, int added_index EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   if (pd->size == 0)
     {
        pd->prev_run.start_id = 0;
        pd->prev_run.end_id = 0;
     }
   pd->size ++;
   if (subobj)
     {
        efl_gfx_entity_visible_set(subobj, EINA_FALSE);
     }
   cache_invalidate(obj, pd);
   schedule_recalc_absolut_size(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_entity_item_removed(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, int removed_index EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   pd->size --;
   if (subobj)
     {
        efl_gfx_entity_visible_set(subobj, EINA_TRUE);
     }
   cache_invalidate(obj, pd);
   schedule_recalc_absolut_size(obj, pd);
}

EOLIAN static Eina_Rect
_efl_ui_position_manager_list_efl_ui_position_manager_entity_position_single_item(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, int idx)
{
   Eina_Rect geom;
   Eina_Size2D space_size;
   int relevant_space_size;
   Eina_Size2D size;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[1];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;

   if (!pd->size) return EINA_RECT(0,0,0,0);

   cache_require(obj, pd);

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->abs_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->abs_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(space_size.w >= 0 && space_size.h >= 0, EINA_RECT(0, 0, 0, 0));
   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
     }
   else
     {
        relevant_space_size = space_size.w;
     }

   geom = pd->viewport;

   BATCH_ACCESS_SIZE_VAL(pd->callbacks, idx, idx + 1, 1, EINA_FALSE, size_buffer, EINA_RECT_EMPTY());

   size = size_buffer[0].size;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        geom.y -= (relevant_space_size - cache_access(obj, pd, idx));
        geom.h = size.h;
     }
   else
     {
        geom.x -= (relevant_space_size - cache_access(obj, pd, idx));
        geom.w = size.w;
     }
   return geom;
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_entity_item_size_changed(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, int start_id EINA_UNUSED, int end_id EINA_UNUSED)
{
   cache_invalidate(obj, pd);
   schedule_recalc_absolut_size(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   pd->dir = dir;
   //in order to reset the state of the visible items, just hide everything and set the old segment accordingly
   vis_change_segment(pd->callbacks, pd->prev_run.start_id, pd->prev_run.end_id, EINA_FALSE);
   pd->prev_run.start_id = 0;
   pd->prev_run.end_id = 0;

   cache_invalidate(obj, pd);
   cache_require(obj,pd);
   if (!efl_finalized_get(obj)) return;
   recalc_absolut_size(obj, pd);
   position_content(obj, pd);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_position_manager_list_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_position_manager_list_efl_object_invalidate(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd)
{
   if (pd->rebuild_absolut_size)
     eina_future_cancel(pd->rebuild_absolut_size);

   efl_ui_position_manager_data_access_v1_data_access_set(obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0);

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_position_manager_list_efl_ui_position_manager_entity_relative_item(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd, unsigned int current_id, Efl_Ui_Focus_Direction direction, unsigned int *index)
{
   switch(direction)
     {
        case EFL_UI_FOCUS_DIRECTION_RIGHT:
        case EFL_UI_FOCUS_DIRECTION_NEXT:
        case EFL_UI_FOCUS_DIRECTION_DOWN:
           if (current_id + 1 >= pd->size) return EINA_FALSE;
           current_id +=  1;
           break;
        case EFL_UI_FOCUS_DIRECTION_LEFT:
        case EFL_UI_FOCUS_DIRECTION_PREVIOUS:
        case EFL_UI_FOCUS_DIRECTION_UP:
           if (current_id == 0) return EINA_FALSE;
           current_id -=  1;
           break;
        default:
           ERR("Uncaught case!");
           return EINA_FALSE;
     }

   if (index) *index = current_id;
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_position_manager_list_efl_ui_position_manager_entity_version(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_List_Data *pd EINA_UNUSED, int max EINA_UNUSED)
{
   return 1;
}

EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_data_access_v1_data_access_set(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, Efl_Ui_Win *canvas, void *obj_access_data, Efl_Ui_Position_Manager_Object_Batch_Callback obj_access, Eina_Free_Cb obj_access_free_cb, void *size_access_data, Efl_Ui_Position_Manager_Size_Batch_Callback size_access, Eina_Free_Cb size_access_free_cb, int size)
{
   // Cleanup cache first
   cache_invalidate(obj, pd);

   // Clean callback if they were set
   if (pd->callbacks.object.free_cb)
     pd->callbacks.object.free_cb(pd->callbacks.object.data);
   if (pd->callbacks.size.free_cb)
     pd->callbacks.size.free_cb(pd->callbacks.size.data);

   // Set them
   efl_replace(&pd->window, canvas);
   efl_replace(&pd->canvas, canvas ? evas_object_evas_get(canvas) : NULL);

   pd->callbacks.object.data = obj_access_data;
   pd->callbacks.object.access = obj_access;
   pd->callbacks.object.free_cb = obj_access_free_cb;
   pd->callbacks.size.data = size_access_data;
   pd->callbacks.size.access = size_access;
   pd->callbacks.size.free_cb = size_access_free_cb;
   pd->size = size;
}


EOLIAN static void
_efl_ui_position_manager_list_efl_ui_position_manager_entity_entities_ready(Eo *obj, Efl_Ui_Position_Manager_List_Data *pd, unsigned int start_id, unsigned int end_id)
{
   Eina_Size2D space_size;
   int relevant_space_size;

   if (end_id < pd->prev_run.start_id || start_id > pd->prev_run.end_id)
     return;

   if (!pd->size) return;
   if (pd->average_item_size <= 0) return;

   cache_require(obj, pd);

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->abs_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->abs_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
     }
   else
     {
        relevant_space_size = space_size.w;
     }
   _position_items(obj, pd, pd->prev_run, relevant_space_size);
}


#include "efl_ui_position_manager_list.eo.c"
