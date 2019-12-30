#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"
#include "efl_ui_position_manager_common.h"

#define MY_CLASS      EFL_UI_POSITION_MANAGER_GRID_CLASS
#define MY_DATA_GET(obj, pd) \
  Efl_Ui_Position_Manager_Grid_Data *pd = efl_data_scope_get(obj, MY_CLASS);

typedef struct {
   Api_Callbacks callbacks;

   Eina_Inarray *group_cache;
   int *size_cache;
   Eo *last_group;
   Eina_Future *rebuild_absolut_size;
   Efl_Ui_Win *window;
   Evas *canvas;

   Vis_Segment prev_run;

   Eina_Rect viewport;
   Eina_Vector2 scroll_position;
   Eina_Size2D max_min_size;
   Eina_Size2D last_viewport_size;
   Eina_Size2D prev_min_size;

   Efl_Ui_Layout_Orientation dir;

   unsigned int size;
   unsigned int groups;
   unsigned int prev_consumed_space;

   Eina_Bool group_cache_dirty;
   Eina_Bool size_cache_dirty;
} Efl_Ui_Position_Manager_Grid_Data;

typedef struct {
   Eina_Size2D group_header_size;
   int items;
   Eina_Bool real_group;
} Group_Cache_Line;

static inline void
_update_min_size(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int added_index EINA_UNUSED, Eina_Size2D min_size)
{
   pd->max_min_size.w = MAX(pd->max_min_size.w, min_size.w);
   pd->max_min_size.h = MAX(pd->max_min_size.h, min_size.h);
}

static void
_group_cache_require(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   unsigned int i;
   const int len = 100;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[len];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;
   Group_Cache_Line line = { 0 };

   if (!pd->group_cache_dirty)
     return;

   pd->max_min_size = EINA_SIZE2D(0, 0);

   pd->group_cache_dirty = EINA_FALSE;
   if (pd->group_cache)
     eina_inarray_free(pd->group_cache);
   pd->group_cache = eina_inarray_new(sizeof(Group_Cache_Line), 10);

   for (i = 0; i < pd->size; ++i)
     {
        int buffer_id = i % len;

        if (buffer_id == 0)
          {
             BATCH_ACCESS_SIZE(pd->callbacks, i, pd->size, MIN(len, pd->size - i), EINA_TRUE, size_buffer);
          }

        if (size_buffer[buffer_id].depth_leader)
          {
             eina_inarray_push(pd->group_cache, &line);
             line.real_group = EINA_TRUE;
             line.group_header_size = size_buffer[buffer_id].size;
             line.items = 1;
          }
        else if (size_buffer[buffer_id].element_depth > 0 ||
                (!line.real_group && size_buffer[buffer_id].element_depth == 0))
          {
             line.items ++;
          }
        else if (size_buffer[buffer_id].element_depth == 0 && line.real_group)
          {
             eina_inarray_push(pd->group_cache, &line);
             line.real_group = EINA_FALSE;
             line.group_header_size = EINA_SIZE2D(0, 0);
             line.items = 0;
          }
        _update_min_size(obj, pd, i, size_buffer[buffer_id].size);
     }
   eina_inarray_push(pd->group_cache, &line);
}

static inline void
_group_cache_invalidate(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
  pd->group_cache_dirty = EINA_TRUE;
  pd->size_cache_dirty = EINA_TRUE;
}

static void
_size_cache_require(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   if (!pd->size_cache_dirty) return;

   _group_cache_require(obj, pd);

   pd->size_cache_dirty = EINA_FALSE;
   if (pd->size_cache)
     free(pd->size_cache);
   pd->size_cache = calloc(sizeof(int), eina_inarray_count(pd->group_cache));

   for (unsigned int i = 0; i < eina_inarray_count(pd->group_cache); ++i)
     {
         Group_Cache_Line *line = eina_inarray_nth(pd->group_cache, i);
         int header_out = 0;
         if (line->real_group)
           header_out = 1;

         if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
           pd->size_cache[i] = line->group_header_size.h +
                        (ceil(
                          (double)(line->items - header_out)/ /* the number of real items in the group (- the group item) */
                          (int)(pd->viewport.w/pd->max_min_size.w))) /* devided by the number of items per row */
                        *pd->max_min_size.h;
         else
           pd->size_cache[i] = (ceil((double)(line->items - header_out)/
                                  (int)((pd->viewport.h-line->group_header_size.h)/pd->max_min_size.h)))*pd->max_min_size.w;
     }
}

static inline void
_size_cache_invalidate(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
  pd->size_cache_dirty = EINA_TRUE;
}

typedef struct {
  int resulting_id;
  int consumed_space;
} Search_Result;

static inline Search_Result
_search_id(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int relevant_space_size)
{
   int consumed_space = 0;
   int consumed_groups = -1;
   int consumed_ids = 0;
   int sub_ids = 0;
   Search_Result res;

   //first we search how many blocks we can skip
   for (unsigned int i = 0; i < eina_inarray_count(pd->group_cache); ++i)
     {
        Group_Cache_Line *line = eina_inarray_nth(pd->group_cache, i);
        if (consumed_space + pd->size_cache[i] > relevant_space_size)
          break;
        consumed_space += pd->size_cache[i];
        consumed_groups = i;
        consumed_ids += line->items;
     }
   Group_Cache_Line *line = NULL;
   if (consumed_groups > -1 && consumed_groups + 1 < (int)eina_inarray_count(pd->group_cache))
     line = eina_inarray_nth(pd->group_cache, consumed_groups + 1);
   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        //now we have relevant_space_size - consumed_space left maybe we are searching the group item

        if (line && line->real_group)
          {
             if (consumed_space + line->group_header_size.h > relevant_space_size)
               {
                  res.resulting_id = consumed_ids;
                  res.consumed_space = consumed_space;
                  return res;
               }
             else
               {
                   consumed_space += line->group_header_size.h;
                   consumed_ids += 1;
               }
          }
        //now we need to locate at which id we are starting
        int space_top = relevant_space_size - consumed_space;
        consumed_space += floor(space_top/pd->max_min_size.h)*pd->max_min_size.h;
        sub_ids = floor(space_top/pd->max_min_size.h)*(pd->viewport.w/pd->max_min_size.w);
     }
   else
     {
        int header_height = 0;
        if (line && line->real_group)
          {
             header_height = line->group_header_size.h;
          }
        //now we need to locate at which id we are starting
        const int space_left = relevant_space_size - consumed_space;
        consumed_space += floor(space_left/pd->max_min_size.w)*pd->max_min_size.w;
        sub_ids = floor(space_left/pd->max_min_size.w)*((pd->viewport.h-header_height)/pd->max_min_size.h);
        if (line && line->real_group &&
            sub_ids > 0) /* if we are in the first row, we need the group item to be visible, otherwise, we need to add that to the consumed ids */
          {
             sub_ids += 1;
          }
     }
   res.resulting_id = consumed_ids + sub_ids;
   res.consumed_space = consumed_space;
   return res;
}

static inline Eina_Bool
_search_start_end(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int relevant_viewport, int relevant_space_size, unsigned int step, Vis_Segment *cur, int *consumed_space)
{
   Search_Result start = _search_id(obj, pd, MAX(relevant_space_size, 0));
   Search_Result end = _search_id(obj, pd, MAX(relevant_space_size, 0)+relevant_viewport+step*2);
   cur->start_id = MIN(MAX(start.resulting_id, 0), (int)pd->size);
   cur->end_id = MAX(MIN(end.resulting_id, (int)pd->size), 0);

   *consumed_space = start.consumed_space;

   return EINA_TRUE;
}

typedef struct {
   int relevant_space_size;
   int consumed_space;
   Vis_Segment new;
   Eo *floating_group;
   Eina_Size2D floating_size;
   Eo *placed_item;
} Item_Position_Context;


static inline void
_place_grid_item(Eina_Rect *geom, Efl_Ui_Position_Manager_Grid_Data *pd, int x, int y)
{
   geom->x += x*pd->max_min_size.w;
   geom->y += y*pd->max_min_size.h;
   geom->size = pd->max_min_size;
}

static inline void
_position_items_vertical(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Item_Position_Context *ctx)
{
   Eina_Position2D start = pd->viewport.pos;
   unsigned int i;
   const int len = 100;
   int columns, last_block_start = ctx->new.start_id;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[len];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;
   Efl_Ui_Position_Manager_Object_Batch_Entity obj_buffer[len];
   Efl_Ui_Position_Manager_Object_Batch_Result object_result;

   if (!pd->viewport.w || !pd->viewport.h) return;

   start.y -= (ctx->relevant_space_size - ctx->consumed_space);
   columns = pd->viewport.w/pd->max_min_size.w;

   for (i = ctx->new.start_id; i < ctx->new.end_id; ++i)
     {
        int buffer_id = (i-ctx->new.start_id) % len;
        if (buffer_id == 0)
          {
             BATCH_ACCESS_SIZE(pd->callbacks, i, ctx->new.end_id, len, EINA_FALSE, size_buffer);
             BATCH_ACCESS_OBJECT(pd->callbacks, i, ctx->new.end_id, len, obj_buffer);

             if (i == ctx->new.start_id)
               {
                  ctx->floating_group = object_result.group;
                  ctx->floating_size = size_result.parent_size;
                  ctx->floating_size.w = pd->viewport.w;
               }
          }
        Eina_Rect geom;
        geom.pos = start;
        int x = (i - last_block_start)%columns;
        int y = (i - last_block_start)/columns;

        if (obj_buffer[buffer_id].entity == pd->last_group)
          pd->last_group = NULL;

        if (obj_buffer[buffer_id].depth_leader)
          {
             if (x != 0)
               y += 1;

             last_block_start = i + 1;
             start.y += size_buffer[buffer_id].size.h + y*pd->max_min_size.h;

             geom.size = pd->viewport.size;
             geom.h = size_buffer[buffer_id].size.h;
             geom.y += y*pd->max_min_size.h;
             if (!ctx->placed_item)
               ctx->placed_item = obj_buffer[buffer_id].entity;
          }
        else
          {
             _place_grid_item(&geom, pd, x, y);
          }
        Efl_Gfx_Entity *item = obj_buffer[buffer_id].entity;
        if (item)
          {
             efl_gfx_entity_geometry_set(item, geom);
             efl_gfx_entity_visible_set(item, EINA_TRUE);
          }
     }
}

static inline void
_position_items_horizontal(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Item_Position_Context *ctx)
{
   Eina_Position2D start = pd->viewport.pos;
   unsigned int i;
   const int len = 100;
   int columns, last_block_start = ctx->new.start_id;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[len];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;
   Efl_Ui_Position_Manager_Object_Batch_Entity obj_buffer[len];
   Efl_Ui_Position_Manager_Object_Batch_Result object_result;

   if (!pd->viewport.w || !pd->viewport.h) return;

   start.x -= (ctx->relevant_space_size - ctx->consumed_space);
   columns = (pd->viewport.h)/pd->max_min_size.h;

   for (i = ctx->new.start_id; i < ctx->new.end_id; ++i)
     {
        int buffer_id = (i-ctx->new.start_id) % len;
        if (buffer_id == 0)
          {
             BATCH_ACCESS_SIZE(pd->callbacks, i, ctx->new.end_id, len, EINA_FALSE, size_buffer);
             BATCH_ACCESS_OBJECT(pd->callbacks, i, ctx->new.end_id, len, obj_buffer);

             if (i == ctx->new.start_id)
               {
                  ctx->floating_group = object_result.group;
                  ctx->floating_size = size_result.parent_size;
                  ctx->floating_size.w = pd->viewport.w;
                  start.y += size_result.parent_size.h;
                  columns = (pd->viewport.h - size_result.parent_size.h)/pd->max_min_size.h;
               }
          }
        Eina_Rect geom;
        geom.pos = start;

        int x = (i - last_block_start)/columns;
        int y = (i - last_block_start)%columns;

        if (obj_buffer[buffer_id].entity == pd->last_group)
          pd->last_group = NULL;

        if (obj_buffer[buffer_id].depth_leader)
          {
             last_block_start = i + 1;
             start.y = pd->viewport.y + size_buffer[buffer_id].size.h;
             start.x += x*pd->max_min_size.w;

             geom.size.h = size_buffer[buffer_id].size.h;
             geom.size.w = pd->viewport.w;
             geom.x += x*pd->max_min_size.w;
             geom.y = pd->viewport.y;

             columns = (pd->viewport.h - size_buffer[buffer_id].size.h)/pd->max_min_size.h;
             if (!ctx->placed_item)
               ctx->placed_item = obj_buffer[buffer_id].entity;
          }
        else
          {
             _place_grid_item(&geom, pd, x, y);
          }
        Efl_Gfx_Entity *item = obj_buffer[buffer_id].entity;
        if (item)
          {
             efl_gfx_entity_geometry_set(item, geom);
             efl_gfx_entity_visible_set(item, EINA_TRUE);
          }
     }
}

static inline void
_position_group_items(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Item_Position_Context *ctx)
{
   //floating group is not yet positioned, in case it is there, we need to position it there
   Eina_Rect geom;

   if (!ctx->floating_group && pd->last_group)
     {
        efl_gfx_entity_visible_set(pd->last_group, EINA_FALSE);
        pd->last_group = NULL;
     }

   if (ctx->floating_group)
     {
        geom.pos = pd->viewport.pos;
        geom.size = ctx->floating_size;

        if (ctx->placed_item)
          {
            Eina_Rect placed = efl_gfx_entity_geometry_get(ctx->placed_item);
             if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
               {
                  geom.y = MIN(geom.y, placed.y-geom.h);
              }
             else
               {
                  geom.x = MIN(geom.x, placed.x-geom.w);
               }
          }

        if (pd->last_group != ctx->floating_group)
          {
             efl_gfx_entity_visible_set(pd->last_group, EINA_FALSE);
             pd->last_group = ctx->floating_group;
          }

        efl_gfx_entity_visible_set(ctx->floating_group, EINA_TRUE);
        efl_gfx_stack_raise_to_top(ctx->floating_group);
        efl_gfx_entity_geometry_set(ctx->floating_group, geom);
     }
   else if (ctx->placed_item)
     {
        Eina_Rect placed = efl_gfx_entity_geometry_get(ctx->placed_item);

        placed.x = MAX(placed.x, pd->viewport.x);
        placed.y = MAX(placed.y, pd->viewport.y);
        efl_gfx_entity_geometry_set(ctx->placed_item, placed);
        efl_gfx_stack_raise_to_top(ctx->placed_item);
     }
}

static void
_reposition_content(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   Eina_Size2D space_size;
   int relevant_space_size, relevant_viewport, consumed_space;
   Vis_Segment cur;
   unsigned int step;
   Efl_Ui_Position_Manager_Range_Update ev;
   Item_Position_Context ctx;

   if (!pd->size) return;
   if (pd->max_min_size.w <= 0 || pd->max_min_size.h <= 0) return;
   if (!eina_inarray_count(pd->group_cache)) return;

   _size_cache_require(obj, pd);

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->last_viewport_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->last_viewport_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
        relevant_viewport = pd->viewport.h;
        step = pd->max_min_size.h;
     }
   else
     {
        relevant_space_size = space_size.w;
        relevant_viewport = pd->viewport.w;
        step = pd->max_min_size.w;
     }
   if (!_search_start_end(obj, pd, relevant_viewport, relevant_space_size, step, &cur, &consumed_space))
     return;

   //to performance optimize the whole widget, we are setting the objects that are outside the viewport to visibility false
   //The code below ensures that things outside the viewport are always hidden, and things inside the viewport are visible
   vis_segment_swap(pd->callbacks, cur, pd->prev_run);

   ctx.new = cur;
   ctx.consumed_space = consumed_space;
   ctx.relevant_space_size = relevant_space_size;
   ctx.floating_group = NULL;
   ctx.placed_item = NULL;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        _position_items_vertical(obj, pd, &ctx);
        _position_group_items(obj, pd, &ctx);
     }
   else
     {
        _position_items_horizontal(obj, pd, &ctx);
        _position_group_items(obj, pd, &ctx);
     }

   if (pd->prev_run.start_id != cur.start_id || pd->prev_run.end_id != cur.end_id)
     {
        ev.start_id = pd->prev_run.start_id = cur.start_id;
        ev.end_id = pd->prev_run.end_id = cur.end_id;
        pd->prev_consumed_space = consumed_space;
        efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_VISIBLE_RANGE_CHANGED, &ev);
     }
}

static inline void
_flush_abs_size(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   Eina_Size2D vp_size;
   int sum_of_cache = 0;

   if (!pd->size) return;
   if (pd->max_min_size.w <= 0 || pd->max_min_size.h <= 0) return;

   _size_cache_require(obj, pd);
   for (unsigned int i = 0; i < eina_inarray_count(pd->group_cache); ++i)
     {
        sum_of_cache += pd->size_cache[i];
     }

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        vp_size.w = pd->viewport.w;
        vp_size.h = sum_of_cache;
     }
   else
     {
        vp_size.h = pd->viewport.h;
        vp_size.w = sum_of_cache;
     }
   if (vp_size.h != pd->last_viewport_size.h || vp_size.w != pd->last_viewport_size.w)
     {
        pd->last_viewport_size = vp_size;
        efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, &vp_size);
     }
}

static inline void
_flush_min_size(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   Eina_Size2D min_size = pd->max_min_size;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     min_size.h = -1;
   else
     min_size.w = -1;

   if (pd->prev_min_size.w != min_size.w || pd->prev_min_size.h != min_size.h)
     {
        pd->prev_min_size = min_size;
        efl_event_callback_call(obj, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, &min_size);
     }
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_viewport_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Eina_Rect viewport)
{
   _size_cache_invalidate(obj, pd);
   pd->viewport = viewport;
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_scroll_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, double x, double y)
{
   pd->scroll_position.x = x;
   pd->scroll_position.y = y;
   _reposition_content(obj, pd);
}

static Eina_Value
_rebuild_job_cb(void *data, Eina_Value v EINA_UNUSED, const Eina_Future *f EINA_UNUSED)
{
   MY_DATA_GET(data, pd);

   if (!efl_alive_get(data)) return EINA_VALUE_EMPTY;

   _flush_abs_size(data, pd);
   _reposition_content(data, pd);
   pd->rebuild_absolut_size = NULL;

   return EINA_VALUE_EMPTY;
}

static void
_schedule_recalc_abs_size(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   if (pd->rebuild_absolut_size) return;

   pd->rebuild_absolut_size = efl_loop_job(efl_app_main_get());
   eina_future_then(pd->rebuild_absolut_size, _rebuild_job_cb, obj);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_item_added(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd, int added_index, Efl_Gfx_Entity *subobj EINA_UNUSED)
{
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[1];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;
   pd->size ++;

   efl_gfx_entity_visible_set(subobj, EINA_FALSE);
   _group_cache_invalidate(obj, pd);
   BATCH_ACCESS_SIZE(pd->callbacks, added_index, added_index + 1, 1, EINA_TRUE, size_buffer);
   _update_min_size(obj, pd, added_index, size_buffer[0].size);
   _flush_min_size(obj, pd);
   _schedule_recalc_abs_size(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_item_removed(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int removed_index EINA_UNUSED, Efl_Gfx_Entity *subobj EINA_UNUSED)
{
   //we ignore here that we might loose the item giving the current max min size
   EINA_SAFETY_ON_FALSE_RETURN(pd->size > 0);
   pd->size --;
   _group_cache_invalidate(obj, pd);
   pd->prev_run.start_id = MIN(pd->prev_run.start_id, pd->size);
   pd->prev_run.end_id = MIN(pd->prev_run.end_id, pd->size);
   _schedule_recalc_abs_size(obj, pd);
   efl_gfx_entity_visible_set(subobj, EINA_TRUE);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_item_size_changed(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd, int start_id, int end_id)
{
   const int len = 50;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[len];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;

   for (int i = start_id; i <= end_id; ++i)
     {
        int buffer_id = (i-start_id) % len;
        if (buffer_id == 0)
          {
             BATCH_ACCESS_SIZE(pd->callbacks, i, end_id + 1, len, EINA_TRUE, size_buffer);
          }
        _update_min_size(obj, pd, i, size_buffer[buffer_id].size);
     }
   _size_cache_invalidate(obj, pd);
   _flush_min_size(obj, pd);
   _schedule_recalc_abs_size(obj, pd);
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   pd->dir = dir;
   _flush_min_size(obj, pd);
   _flush_abs_size(obj, pd);
   _reposition_content(obj, pd); //FIXME we could check if this is needed or not
}


EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_position_manager_grid_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   return pd->dir;
}

EOLIAN static Eina_Rect
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_position_single_item(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, int idx)
{
   Eina_Rect geom;
   Eina_Size2D space_size;
   unsigned int relevant_space_size;
   unsigned int group_consumed_size = 0;
   unsigned int group_consumed_ids = 0;
   Efl_Ui_Position_Manager_Size_Batch_Entity size_buffer[1];
   Efl_Ui_Position_Manager_Size_Batch_Result size_result;

   if (!pd->size) return EINA_RECT(0, 0, 0, 0);
   if (pd->max_min_size.w <= 0 || pd->max_min_size.h <= 0) return EINA_RECT(0, 0, 0, 0);
   BATCH_ACCESS_SIZE_VAL(pd->callbacks, idx, idx + 1, 1, EINA_TRUE, size_buffer, EINA_RECT_EMPTY());

   _size_cache_require(obj, pd);
   _flush_abs_size(obj, pd);

   space_size.w = (MAX(pd->last_viewport_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->last_viewport_size.h - pd->viewport.h, 0))*pd->scroll_position.y;
   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     relevant_space_size = space_size.h;
   else
     relevant_space_size = space_size.w;

   geom.size = pd->max_min_size;
   geom.pos = pd->viewport.pos;

   for (unsigned int i = 0; i < eina_inarray_count(pd->group_cache); ++i)
     {
        Group_Cache_Line *line = eina_inarray_nth(pd->group_cache, i);
        if ((int)group_consumed_ids + line->items > idx)
          break;

        group_consumed_size += pd->size_cache[i];
        group_consumed_ids += line->items;
        if (line->real_group && idx == (int)group_consumed_ids + 1)
          {
             geom.y = (relevant_space_size - group_consumed_size);
             geom.size = size_buffer[0].size;

             return geom;
          }
        else if (line->real_group)
          group_consumed_size += line->group_header_size.h;
     }

   if (idx > 0)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(group_consumed_ids < (unsigned int)idx, EINA_RECT(0, 0, 0, 0));
   else if (idx == 0)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(group_consumed_ids == 0, EINA_RECT(0, 0, 0, 0));

   int columns = pd->viewport.w/pd->max_min_size.w;
   if (columns == 0) return EINA_RECT(0, 0, 0, 0);
   int sub_pos_id = idx - group_consumed_ids;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        const int x = (sub_pos_id)%columns;
        const int y = (sub_pos_id)/columns;

        geom.y -= relevant_space_size;
        geom.x += pd->max_min_size.w*x;
        geom.y += group_consumed_size + pd->max_min_size.h*y;
     }
   else
     {
        const int x = (sub_pos_id)/columns;
        const int y = (sub_pos_id)%columns;

        geom.x -= relevant_space_size;
        geom.y += pd->max_min_size.h*y;
        geom.x += group_consumed_size + pd->max_min_size.w*x;
     }

   return geom;
}

EOLIAN static Eina_Bool
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_relative_item(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd, unsigned int current_id, Efl_Ui_Focus_Direction direction, unsigned int *index)
{
   switch(direction)
     {
        case EFL_UI_FOCUS_DIRECTION_RIGHT:
        case EFL_UI_FOCUS_DIRECTION_NEXT:
           if (current_id + 1 >= pd->size) return EINA_FALSE;
           current_id += 1;
           break;
        case EFL_UI_FOCUS_DIRECTION_LEFT:
        case EFL_UI_FOCUS_DIRECTION_PREVIOUS:
           if (current_id == 0) return EINA_FALSE;
           current_id -= 1;
           break;
        case EFL_UI_FOCUS_DIRECTION_UP:
           //FIXME
           break;
        case EFL_UI_FOCUS_DIRECTION_DOWN:
           //FIXME
           break;
        default:
           ERR("Uncaught case!");
           return EINA_FALSE;
     }

   if (index) *index = current_id;
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_version(Eo *obj EINA_UNUSED, Efl_Ui_Position_Manager_Grid_Data *pd EINA_UNUSED, int max EINA_UNUSED)
{
   return 1;
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_data_access_v1_data_access_set(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd, Efl_Ui_Win *canvas, void *obj_access_data, Efl_Ui_Position_Manager_Object_Batch_Callback obj_access, Eina_Free_Cb obj_access_free_cb, void *size_access_data, Efl_Ui_Position_Manager_Size_Batch_Callback size_access, Eina_Free_Cb size_access_free_cb, int size)
{
   // Cleanup cache first
   _group_cache_invalidate(obj, pd);

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
   _group_cache_require(obj, pd);
   _schedule_recalc_abs_size(obj, pd);

}

EOLIAN static void
_efl_ui_position_manager_grid_efl_object_invalidate(Eo *obj,
                                                    Efl_Ui_Position_Manager_Grid_Data *pd EINA_UNUSED)
{
   efl_ui_position_manager_data_access_v1_data_access_set(obj, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0);

   efl_invalidate(efl_super(obj, EFL_UI_POSITION_MANAGER_GRID_CLASS));
}

EOLIAN static Efl_Object*
_efl_ui_position_manager_grid_efl_object_finalize(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   pd->group_cache_dirty = EINA_TRUE;
   _group_cache_require(obj, pd);
   _schedule_recalc_abs_size(obj, pd);

   return obj;
}

EOLIAN static void
_efl_ui_position_manager_grid_efl_ui_position_manager_entity_entities_ready(Eo *obj, Efl_Ui_Position_Manager_Grid_Data *pd, unsigned int start_id, unsigned int end_id)
{
   Eina_Size2D space_size;
   int relevant_space_size;
   Item_Position_Context ctx;

   if (end_id < pd->prev_run.start_id || start_id > pd->prev_run.end_id)
     return;

   space_size.w = (MAX(pd->last_viewport_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->last_viewport_size.h - pd->viewport.h, 0))*pd->scroll_position.y;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        relevant_space_size = space_size.h;
     }
   else
     {
        relevant_space_size = space_size.w;
     }

   ctx.new = pd->prev_run;
   ctx.consumed_space = pd->prev_consumed_space;
   ctx.relevant_space_size = relevant_space_size;
   ctx.floating_group = NULL;
   ctx.placed_item = NULL;

   if (pd->dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        _position_items_vertical(obj, pd, &ctx);
        _position_group_items(obj, pd, &ctx);
     }
   else
     {
        _position_items_horizontal(obj, pd, &ctx);
        _position_group_items(obj, pd, &ctx);
     }
}



#include "efl_ui_position_manager_grid.eo.c"
