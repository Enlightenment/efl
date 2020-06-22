#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"
#include "efl_ui_collection_focus_manager.eo.h"

typedef struct {
   Eo *collection;
} Efl_Ui_Collection_Focus_Manager_Data;

typedef struct {
   unsigned int last_index;
   const Eina_List *current;
   Eina_List **items;
} Fast_Accessor;

static const Eina_List*
_fast_accessor_get_at(Fast_Accessor *accessor, unsigned int idx)
{
   const Eina_List *over;
   unsigned int middle;
   unsigned int i;

   if (idx >= eina_list_count(*accessor->items))
     return NULL;

   if (accessor->last_index == idx)
     over = accessor->current;
   else if (idx > accessor->last_index)
     {
        /* After current position. */
        middle = ((eina_list_count(*accessor->items) - accessor->last_index))/2;

        if (idx > middle)
          /* Go backward from the end. */
          for (i = eina_list_count(*accessor->items) - 1,
               over = eina_list_last(*accessor->items);
               i > idx && over;
               --i, over = eina_list_prev(over))
            ;
        else
          /* Go forward from current. */
          for (i = accessor->last_index, over = accessor->current;
               i < idx && over;
               ++i, over = eina_list_next(over))
            ;
     }
   else
     {
        /* Before current position. */
        middle = accessor->last_index/2;

        if (idx > middle)
          /* Go backward from current. */
          for (i = accessor->last_index, over = accessor->current;
               i > idx && over;
               --i, over = eina_list_prev(over))
            ;
        else
          /* Go forward from start. */
          for (i = 0, over = *accessor->items;
               i < idx && over;
               ++i, over = eina_list_next(over))
            ;
     }

   if (!over)
     return NULL;

   accessor->last_index = idx;
   accessor->current = over;

   return over;
}

static void
_fast_accessor_init(Fast_Accessor *accessor, Eina_List **items)
{
   //this is the accessor for accessing the items
   //we have to workaround here the problem that
   //no accessor can be created for a not yet created list.
   accessor->items = items;
}

static void
_fast_accessor_remove(Fast_Accessor *accessor, const Eina_List *removed_elem)
{
   if (accessor->current == removed_elem)
     {
        Eina_List *next;
        Eina_List *prev;

        next = eina_list_next(removed_elem);
        prev = eina_list_prev(removed_elem);
        if (next)
          {
             accessor->current = next;
             accessor->last_index ++;
          }
        else if (prev)
          {
             accessor->current = prev;
             accessor->last_index --;
          }
        else
          {
             //everything >= length is invalid, and we need that.
             accessor->last_index = eina_list_count(*accessor->items);
             accessor->current = NULL;
          }

     }

}

#define MY_CLASS      EFL_UI_COLLECTION_CLASS

#define MY_DATA_GET(obj, pd) \
  Efl_Ui_Collection_Data *pd = efl_data_scope_get(obj, MY_CLASS);

typedef struct {
   Efl_Ui_Scroll_Manager *smanager;
   Efl_Ui_Pan *pan;
   Eina_List *selected;
   Eina_List *items;
   Efl_Ui_Selection *fallback;
   Efl_Ui_Select_Mode mode;
   Efl_Ui_Layout_Orientation dir;
   Eina_Size2D content_min_size;
   Efl_Ui_Position_Manager_Entity *pos_man;
   Eina_Future *selection_changed_job;
   struct {
      Eina_Bool w;
      Eina_Bool h;
   } match_content;
   Fast_Accessor obj_accessor;
   Fast_Accessor size_accessor;
   Efl_Gfx_Entity *sizer;
   unsigned int start_id, end_id;
   Eina_Bool allow_manual_deselection : 1;
} Efl_Ui_Collection_Data;

static Eina_Bool register_item(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Item *item);
static Eina_Bool unregister_item(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Item *item);

static void
flush_min_size(Eo *obj, Efl_Ui_Collection_Data *pd)
{
   Eina_Size2D tmp = pd->content_min_size;

   if (!pd->match_content.w)
     tmp.w = -1;

   if (!pd->match_content.h)
     tmp.h = -1;

   efl_gfx_hint_size_restricted_min_set(obj, tmp);
}

static int
clamp_index(Efl_Ui_Collection_Data *pd, int index)
{
   if (index < ((int)eina_list_count(pd->items)) * -1)
     return -1;
   else if (index > (int)eina_list_count(pd->items) - 1)
     return 1;
   return 0;
}

static int
index_adjust(Efl_Ui_Collection_Data *pd, int index)
{
   int c = eina_list_count(pd->items);
   if (index < c * -1)
     return 0;
   else if (index > c - 1)
     return c - 1;
   else if (index < 0)
     return index + c;
   return index;
}

static void
_pan_viewport_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Rect rect = efl_ui_scrollable_viewport_geometry_get(data);

   efl_ui_position_manager_entity_viewport_set(pd->pos_man, rect);
}

static void
_pan_position_changed_cb(void *data, const Efl_Event *ev)
{
   MY_DATA_GET(data, pd);
   Eina_Position2D *pos = ev->info;
   Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);
   Eina_Vector2 rpos = {0.0, 0.0};

   if (max.x > 0.0)
     rpos.x = (double)pos->x/(double)max.x;
   if (max.y > 0.0)
     rpos.y = (double)pos->y/(double)max.y;

   efl_ui_position_manager_entity_scroll_position_set(pd->pos_man, rpos.x, rpos.y);
}

EFL_CALLBACKS_ARRAY_DEFINE(pan_events_cb,
  {EFL_UI_PAN_EVENT_PAN_CONTENT_POSITION_CHANGED, _pan_position_changed_cb},
  {EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _pan_viewport_changed_cb},
  {EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _pan_viewport_changed_cb},
)

static void
_item_scroll_internal(Eo *obj EINA_UNUSED,
                      Efl_Ui_Collection_Data *pd,
                      Efl_Ui_Item *item,
                      double align EINA_UNUSED,
                      Eina_Bool anim)
{
   Eina_Rect ipos, view;
   Eina_Position2D vpos;

   if (!pd->smanager) return;

   ipos = efl_ui_position_manager_entity_position_single_item(pd->pos_man, eina_list_data_idx(pd->items, item));
   view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
   vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

   ipos.x = ipos.x + vpos.x - view.x;
   ipos.y = ipos.y + vpos.y - view.y;

   //FIXME scrollable needs some sort of align, the docs do not even garantee to completely move in the element
   efl_ui_scrollable_scroll(pd->smanager, ipos, anim);
}

EOLIAN static void
_efl_ui_collection_item_scroll(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Item *item, Eina_Bool animation)
{
   _item_scroll_internal(obj, pd, item, -1.0, animation);
}

EOLIAN static void
_efl_ui_collection_item_scroll_align(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Item *item, double align, Eina_Bool animation)
{
   _item_scroll_internal(obj, pd, item, align, animation);
}

EOLIAN static Efl_Ui_Selectable*
_efl_ui_collection_efl_ui_single_selectable_last_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return eina_list_last_data_get(pd->selected);
}

EOLIAN static Eina_Iterator*
_efl_ui_collection_efl_ui_multi_selectable_object_range_selected_iterator_new(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return eina_list_iterator_new(pd->selected);
}

static inline void
_fill_depth(Eo *item, unsigned char *depth, Eina_Bool *leader)
{
   if (efl_isa(item, EFL_UI_GROUP_ITEM_CLASS))
     {
        *depth = 1;
        *leader = EINA_TRUE;
     }
   else if (efl_ui_item_parent_get(item))
     {
        *depth = 1;
        *leader = EINA_FALSE;
     }
   else
     {
        *leader = EINA_FALSE;
        *depth = 0;
     }
}

static Efl_Ui_Position_Manager_Size_Batch_Result
_size_accessor_get_at(void *data, Efl_Ui_Position_Manager_Size_Call_Config conf, Eina_Rw_Slice memory)
{
   Fast_Accessor *accessor = data;
   size_t i;
   const Eina_List *lst = _fast_accessor_get_at(accessor, conf.range.start_id);
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = {0};

   EINA_SAFETY_ON_NULL_RETURN_VAL(lst, result);

   for (i = 0; i < (conf.range.end_id - conf.range.start_id); ++i)
     {
         Efl_Gfx_Entity *geom = eina_list_data_get(lst), *parent;
         Eina_Size2D size = efl_gfx_hint_size_combined_min_get(geom);

         parent = efl_ui_item_parent_get(geom);
         sizes[i].size = size;
         _fill_depth(geom, &sizes[i].element_depth, &sizes[i].depth_leader);
         if (i == 0 && !sizes[0].depth_leader && parent)
           {
              result.parent_size = efl_gfx_hint_size_combined_min_get(parent);
           }
         lst = eina_list_next(lst);
         if (!lst)
           {
              i++;
              break;
           }
     }
   result.filled_items = i;

   return result;
}

static Efl_Ui_Position_Manager_Object_Batch_Result
_obj_accessor_get_at(void *data, Efl_Ui_Position_Manager_Request_Range range, Eina_Rw_Slice memory)
{
   Fast_Accessor *accessor = data;
   size_t i;
   const Eina_List *lst = _fast_accessor_get_at(accessor, range.start_id);
   Efl_Ui_Position_Manager_Object_Batch_Entity *objs = memory.mem;
   Efl_Ui_Position_Manager_Object_Batch_Result result = {0};

   for (i = 0; i < range.end_id - range.start_id; ++i)
     {
         Efl_Gfx_Entity *geom = eina_list_data_get(lst), *parent;

         parent = efl_ui_item_parent_get(geom);
         objs[i].entity = geom;
         _fill_depth(geom, &objs[i].element_depth, &objs[i].depth_leader);
         if (i == 0 && !objs[0].depth_leader && parent)
           {
              result.group = parent;
           }

         lst = eina_list_next(lst);
         if (!lst)
           {
              i++;
              break;
           }
     }
   result.filled_items = i;

   return result;
}


EOLIAN static Efl_Object*
_efl_ui_collection_efl_object_constructor(Eo *obj, Efl_Ui_Collection_Data *pd EINA_UNUSED)
{
   Eo *o;

   efl_ui_selectable_allow_manual_deselection_set(obj, EINA_TRUE);

   pd->dir = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;

   _fast_accessor_init(&pd->obj_accessor, &pd->items);
   _fast_accessor_init(&pd->size_accessor, &pd->items);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "collection");

   o = efl_constructor(efl_super(obj, MY_CLASS));

   pd->sizer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(pd->sizer, 0, 0, 0, 0);

   pd->pan = efl_add(EFL_UI_PAN_CLASS, obj);
   efl_content_set(pd->pan, pd->sizer);
   efl_event_callback_array_add(pd->pan, pan_events_cb(), obj);

   pd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_composite_attach(obj, pd->smanager);
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_ui_scroll_manager_pan_set(pd->smanager, pd->pan);

   efl_ui_scroll_connector_bind(obj, pd->smanager);

   return o;
}

EOLIAN static Efl_Object*
_efl_ui_collection_efl_object_finalize(Eo *obj, Efl_Ui_Collection_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->pos_man, NULL);

   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_ui_collection_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Collection_Data *pd)
{
   Eina_Error res;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);
   res = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (res == EFL_UI_THEME_APPLY_ERROR_GENERIC) return res;
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_content_set(efl_part(wd->resize_obj, "efl.content"), pd->pan);

   return res;
}

EOLIAN static void
_efl_ui_collection_efl_object_destructor(Eo *obj, Efl_Ui_Collection_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
deselect_all(Efl_Ui_Collection_Data *pd)
{
   while(pd->selected)
     {
        Eo *item = eina_list_data_get(pd->selected);
        efl_ui_selectable_selected_set(item, EINA_FALSE);
        EINA_SAFETY_ON_TRUE_RETURN(eina_list_data_get(pd->selected) == item);
     }
}

EOLIAN static void
_efl_ui_collection_efl_object_invalidate(Eo *obj, Efl_Ui_Collection_Data *pd EINA_UNUSED)
{
   efl_ui_collection_position_manager_set(obj, NULL);

   efl_ui_selectable_fallback_selection_set(obj, NULL);

   deselect_all(pd);

   while(pd->items)
     efl_del(pd->items->data);

   // pan is given to edje, which reparents it, which forces us to manually deleting it
   efl_del(pd->pan);

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Iterator*
_efl_ui_collection_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return eina_list_iterator_new(pd->items);
}

EOLIAN static int
_efl_ui_collection_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return eina_list_count(pd->items);
}

EOLIAN static void
_efl_ui_collection_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   if (pd->dir == dir) return;

   pd->dir = dir;
   if (pd->pos_man)
     efl_ui_layout_orientation_set(pd->pos_man, dir);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_collection_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_collection_efl_ui_scrollable_match_content_set(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Eina_Bool w, Eina_Bool h)
{
   if (pd->match_content.w == w && pd->match_content.h == h)
     return;

   pd->match_content.w = w;
   pd->match_content.h = h;

   efl_ui_scrollable_match_content_set(pd->smanager, w, h);
   flush_min_size(obj, pd);
}

EOLIAN static void
_efl_ui_collection_efl_ui_multi_selectable_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Efl_Ui_Select_Mode mode)
{
   pd->mode = mode;
   if ((mode == EFL_UI_SELECT_MODE_SINGLE) &&
       eina_list_count(pd->selected) > 0)
     {
        Efl_Ui_Item *last = eina_list_last_data_get(pd->selected);

        pd->selected = eina_list_remove_list(pd->selected, eina_list_last(pd->selected));
        deselect_all(pd);
        pd->selected = eina_list_append(pd->selected, last);
     }
   else if (mode == EFL_UI_SELECT_MODE_NONE && pd->selected)
     {
        deselect_all(pd);
     }
}

EOLIAN static Efl_Ui_Select_Mode
_efl_ui_collection_efl_ui_multi_selectable_select_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return pd->mode;
}

static Eina_Value
_schedule_selection_job_cb(Eo *o, void *data EINA_UNUSED, const Eina_Value value EINA_UNUSED)
{
   MY_DATA_GET(o, pd);

   pd->selection_changed_job = NULL;

   efl_event_callback_call(o, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, NULL);

   return EINA_VALUE_EMPTY;
}

static void
_schedule_selection_changed(Eo *obj, Efl_Ui_Collection_Data *pd)
{
   Eina_Future *f;

   if (pd->selection_changed_job) return;

   f = efl_loop_job(efl_main_loop_get());
   pd->selection_changed_job = efl_future_then(obj, f, _schedule_selection_job_cb);

}

static void
_apply_fallback(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   if (pd->fallback && !pd->selected)
     {
        efl_ui_selectable_selected_set(pd->fallback, EINA_TRUE);
     }
}

static inline void
_single_selection_behaviour(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Efl_Ui_Selectable *new_selection)
{
   //we might get the situation that the item is already in the list and selected again, so just free the list, it will be rebuild below
   if (eina_list_data_get(pd->selected) == new_selection)
     {
       pd->selected = eina_list_free(pd->selected);
     }
   else
     {
        deselect_all(pd);
     }
}

static void
_selection_changed(void *data, const Efl_Event *ev)
{
   Eina_Bool selection = *((Eina_Bool*) ev->info);
   Eo *obj = data;
   MY_DATA_GET(obj, pd);
   Efl_Ui_Selection *fallback;

   //if this is the highest call in the tree of selection changes, safe the fallback and apply it later
   //this way we ensure that we are not accidentally adding fallback even if we just want to have a empty selection list
   fallback = pd->fallback;
   pd->fallback = NULL;

   if (selection)
     {
        if (pd->mode == EFL_UI_SELECT_MODE_SINGLE)
          {
             _single_selection_behaviour(obj, pd, ev->object);
          }
        else if (pd->mode == EFL_UI_SELECT_MODE_MULTI && _elm_config->desktop_entry)
          {
             const Evas_Modifier *mod = evas_key_modifier_get(evas_object_evas_get(ev->object));
             if (!(efl_input_clickable_interaction_get(ev->object)
                   && evas_key_modifier_is_set(mod, "Control")))
               _single_selection_behaviour(obj, pd, ev->object);
          }
        else if (pd->mode == EFL_UI_SELECT_MODE_NONE)
          {
             ERR("Selection while mode is NONE, uncaught state!");
             return;
          }
        pd->selected = eina_list_append(pd->selected, ev->object);
     }
   else
     {
        pd->selected = eina_list_remove(pd->selected, ev->object);
     }

   pd->fallback = fallback;
   _apply_fallback(obj, pd);
   _schedule_selection_changed(obj, pd);
}

static void
_invalidate_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   unregister_item(obj, pd, ev->object);
}

static void
_hints_changed_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);
   int idx = eina_list_data_idx(pd->items, ev->object);

   efl_ui_position_manager_entity_item_size_changed(pd->pos_man, idx, idx);
}

static void
_redirect_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;

#define REDIRECT_EVT(Desc, Item_Desc)                           \
   if (Desc == ev->desc)                                        \
     {                                                          \
        Efl_Ui_Item_Clickable_Clicked item_clicked;             \
        Efl_Input_Clickable_Clicked *clicked = ev->info;        \
                                                                \
        item_clicked.clicked = *clicked;                        \
        item_clicked.item = ev->object;                         \
                                                                \
        efl_event_callback_call(obj, Item_Desc, &item_clicked); \
     }
#define REDIRECT_EVT_PRESS(Desc, Item_Desc)                           \
   if (Desc == ev->desc)                                        \
     {                                                          \
        Efl_Ui_Item_Clickable_Pressed item_pressed;             \
        int *button = ev->info;        \
                                                                \
        item_pressed.button = *button;                        \
        item_pressed.item = ev->object;                         \
                                                                \
        efl_event_callback_call(obj, Item_Desc, &item_pressed); \
     }

   REDIRECT_EVT_PRESS(EFL_INPUT_EVENT_PRESSED, EFL_UI_EVENT_ITEM_PRESSED);
   REDIRECT_EVT_PRESS(EFL_INPUT_EVENT_UNPRESSED, EFL_UI_EVENT_ITEM_UNPRESSED);
   REDIRECT_EVT_PRESS(EFL_INPUT_EVENT_LONGPRESSED, EFL_UI_EVENT_ITEM_LONGPRESSED);
   REDIRECT_EVT(EFL_INPUT_EVENT_CLICKED_ANY, EFL_UI_EVENT_ITEM_CLICKED_ANY);
   REDIRECT_EVT(EFL_INPUT_EVENT_CLICKED, EFL_UI_EVENT_ITEM_CLICKED);
#undef REDIRECT_EVT
#undef REDIRECT_EVT_PRESS
}

EFL_CALLBACKS_ARRAY_DEFINE(active_item,
  {EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _hints_changed_cb},
  {EFL_UI_EVENT_SELECTED_CHANGED, _selection_changed},
  {EFL_INPUT_EVENT_PRESSED, _redirect_cb},
  {EFL_INPUT_EVENT_UNPRESSED, _redirect_cb},
  {EFL_INPUT_EVENT_LONGPRESSED, _redirect_cb},
  {EFL_INPUT_EVENT_CLICKED, _redirect_cb},
  {EFL_INPUT_EVENT_CLICKED_ANY, _redirect_cb},
  {EFL_EVENT_INVALIDATE, _invalidate_cb},
)

static Eina_Bool
register_item(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Item *item)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(item, EFL_UI_ITEM_CLASS), EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_list_data_find(pd->items, item), EINA_FALSE);

   if (!efl_ui_widget_sub_object_add(obj, item))
     return EINA_FALSE;

   efl_ui_item_container_set(item, obj);
   efl_canvas_group_member_add(pd->pan, item);
   efl_event_callback_array_add(item, active_item(), obj);
   efl_ui_mirrored_set(item, efl_ui_mirrored_get(obj));

   return EINA_TRUE;
}

static Eina_Bool
unregister_item(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Item *item)
{
   Eina_List *elem = eina_list_data_find_list(pd->items, item);
   if (!elem)
     {
        ERR("Item %p is not part of this widget", item);
        return EINA_FALSE;
     }

   if (!efl_ui_widget_sub_object_del(obj, item))
     return EINA_FALSE;

   unsigned int id = eina_list_data_idx(pd->items, item);

   _fast_accessor_remove(&pd->obj_accessor, elem);
   _fast_accessor_remove(&pd->size_accessor, elem);

   pd->items = eina_list_remove(pd->items, item);
   pd->selected = eina_list_remove(pd->selected, item);
   efl_event_callback_array_del(item, active_item(), obj);
   efl_ui_position_manager_entity_item_removed(pd->pos_man, id, item);
   efl_ui_item_container_set(item, NULL);
   efl_canvas_group_member_remove(pd->pan, item);

   return EINA_TRUE;
}

static void
update_pos_man(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj)
{
   int id = eina_list_data_idx(pd->items, subobj);
   if (id == 0)
     {
        pd->obj_accessor.last_index = id;
        pd->obj_accessor.current = pd->items;
        pd->size_accessor.last_index = id;
        pd->size_accessor.current = pd->items;
     }
   efl_ui_position_manager_entity_item_added(pd->pos_man, id, subobj);
}

static inline Efl_Ui_Item*
fetch_rep_parent(Eina_List *lst)
{
   if (!lst)
     return NULL;

   Efl_Ui_Item *it = eina_list_data_get(lst);

   return efl_ui_item_parent_get(it);
}

static Eina_Bool
check_group_integrity(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_List *carrier_list = eina_list_data_find_list(pd->items, subobj), *prev_lst;
   Efl_Ui_Item *next, *prev, *carrier;

   prev_lst = eina_list_prev(carrier_list);
   next = fetch_rep_parent(eina_list_next(carrier_list));
   prev = fetch_rep_parent(prev_lst);
   carrier = fetch_rep_parent(carrier_list);

   if (next && next == prev && carrier != prev)
     {
        //a item got inserted into the middle of one group, but does not have the correct group header, that is a bug
        ERR("Inserting a item with the wrong group into another group(%p,%p,%p)", prev, carrier, next);
        unregister_item(obj, pd, subobj);
        return EINA_FALSE;
     }

   if (prev_lst && eina_list_data_get(prev_lst) == next && carrier != next)
     {
        //a item got inserted between group header and group children, also a error
        ERR("Inserting a item between group header, and group elements(%p,%p,%p)", prev_lst, eina_list_data_get(prev_lst), next);
        unregister_item(obj, pd, subobj);
        return EINA_FALSE;
     }
   if (!next && !prev && carrier && prev_lst && eina_list_data_get(prev_lst) != carrier)
     {
        ERR("Tried to insert a item with group, outside its group(%p,%p,%p)", next, prev, carrier);
        unregister_item(obj, pd, subobj);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   while(pd->items)
     {
        efl_del(pd->items->data);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_unpack_all(Eo *obj, Efl_Ui_Collection_Data *pd)
{
   while(pd->items)
     {
        if (!unregister_item(obj, pd, pd->items->data))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_collection_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Collection_Data *pd, int index)
{
   Efl_Ui_Item *it = eina_list_nth(pd->items, index_adjust(pd, index));

   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   if (!unregister_item(obj, pd, it))
     return NULL;

   return it;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_unpack(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj)
{
   return unregister_item(obj, pd, subobj);
}


EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_pack(Eo *obj, Efl_Ui_Collection_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_end(obj, subobj);
}


EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_append(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}


EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_prepend(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, existing);
   if (existing)
     EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);

   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_prepend_relative_list(pd->items, subobj, subobj_list);
   if (!check_group_integrity(obj, pd, subobj)) return EINA_FALSE;
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, existing);
   if (existing)
     EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);

   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_append_relative_list(pd->items, subobj, subobj_list);
   if (!check_group_integrity(obj, pd, subobj)) return EINA_FALSE;
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   Eina_List *subobj_list;
   int clamp;

   clamp = clamp_index(pd, index);
   index = index_adjust(pd, index);
   subobj_list = eina_list_nth_list(pd->items, index);
   if (pd->items)
     EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   if (clamp == 0)
     pd->items = eina_list_prepend_relative_list(pd->items, subobj, subobj_list);
   else if (clamp == 1)
     pd->items = eina_list_append(pd->items, subobj);
   else
     pd->items = eina_list_prepend(pd->items, subobj);
   if (!check_group_integrity(obj, pd, subobj)) return EINA_FALSE;
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_collection_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->items, (void*)subobj);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_collection_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, int index)
{
   return eina_list_nth(pd->items, index_adjust(pd, index));
}

static void
_pos_content_size_changed_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   efl_gfx_entity_size_set(pd->sizer, *size);
}

static void
_pos_content_min_size_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   pd->content_min_size = *size;

   flush_min_size(data, pd);
}

static void
_visible_range_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Position_Manager_Range_Update *info = ev->info;
   MY_DATA_GET(data, pd);

   pd->start_id = info->start_id;
   pd->end_id = info->end_id;
}

EFL_CALLBACKS_ARRAY_DEFINE(pos_manager_cbs,
  {EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, _pos_content_size_changed_cb},
  {EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, _pos_content_min_size_changed_cb},
  {EFL_UI_POSITION_MANAGER_ENTITY_EVENT_VISIBLE_RANGE_CHANGED, _visible_range_changed_cb}
)

EOLIAN static void
_efl_ui_collection_position_manager_set(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Position_Manager_Entity *layouter)
{
   if (layouter)
     EINA_SAFETY_ON_FALSE_RETURN(efl_isa(layouter, EFL_UI_POSITION_MANAGER_ENTITY_INTERFACE));

   if (pd->pos_man)
     {
        efl_event_callback_array_del(pd->pos_man, pos_manager_cbs(), obj);
        efl_del(pd->pos_man);
     }
   pd->pos_man = layouter;
   if (pd->pos_man)
     {
        efl_parent_set(pd->pos_man, obj);
        efl_event_callback_array_add(pd->pos_man, pos_manager_cbs(), obj);
        switch(efl_ui_position_manager_entity_version(pd->pos_man, 1))
          {
            case 1:
              efl_ui_position_manager_data_access_v1_data_access_set(pd->pos_man,
                efl_provider_find(obj, EFL_UI_WIN_CLASS),
                &pd->obj_accessor, _obj_accessor_get_at, NULL,
                &pd->size_accessor, _size_accessor_get_at, NULL,
                eina_list_count(pd->items));
            break;
          }

        efl_ui_position_manager_entity_viewport_set(pd->pos_man, efl_ui_scrollable_viewport_geometry_get(obj));
        efl_ui_layout_orientation_set(pd->pos_man, pd->dir);
     }
}

EOLIAN static Efl_Ui_Position_Manager_Entity*
_efl_ui_collection_position_manager_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
  return pd->pos_man;
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_collection_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Efl_Ui_Collection_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   Eo *man = efl_add(EFL_UI_COLLECTION_FOCUS_MANAGER_CLASS, obj,
                 efl_ui_focus_manager_root_set(efl_added, root));
   Efl_Ui_Collection_Focus_Manager_Data *fm_pd = efl_data_scope_safe_get(man, EFL_UI_COLLECTION_FOCUS_MANAGER_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fm_pd, NULL);
   fm_pd->collection = obj;
   return man;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_Collection_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect EINA_UNUSED)
{
   return efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, obj);
}

static Efl_Ui_Item *
_find_item(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd EINA_UNUSED, Eo *focused_element)
{
   if (!focused_element) return NULL;

   while (focused_element && !efl_isa(focused_element, EFL_UI_ITEM_CLASS))
     {
        focused_element = efl_ui_widget_parent_get(focused_element);
     }

   return focused_element;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_collection_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *new_obj, *focus;
   Eina_Size2D step;

   focus = efl_ui_focus_manager_focus_get(obj);
   new_obj = efl_ui_focus_manager_move(efl_super(obj, MY_CLASS), direction);
   step = efl_gfx_hint_size_combined_min_get(focus);

   if (new_obj)
     {
        /* if this is outside the viewport, then we must bring that in first */
        Eina_Rect viewport;
        Eina_Rect element;
        element = efl_gfx_entity_geometry_get(focus);
        viewport = efl_gfx_entity_geometry_get(obj);
        if (!eina_spans_intersect(element.x, element.w, viewport.x, viewport.w) &&
            !eina_spans_intersect(element.y, element.h, viewport.y, viewport.h))
          {
             efl_ui_scrollable_scroll(obj, element, EINA_TRUE);
             return focus;
          }
     }

   if (!new_obj)
     {
        Eina_Rect pos = efl_gfx_entity_geometry_get(focus);
        Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
        Eina_Position2D vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

        pos.x = pos.x + vpos.x - view.x;
        pos.y = pos.y + vpos.y - view.y;
        Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);

        if (direction == EFL_UI_FOCUS_DIRECTION_RIGHT)
          {
             if (pos.x < max.x)
               {
                  pos.x = MIN(max.x, pos.x + step.w);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_LEFT)
          {
             if (pos.x > 0)
               {
                  pos.x = MAX(0, pos.x - step.w);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_UP)
          {
             if (pos.y > 0)
               {
                  pos.y = MAX(0, pos.y - step.h);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_DOWN)
          {
             if (pos.y < max.y)
               {
                  pos.y = MAX(0, pos.y + step.h);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
     }
   else
     {
        _item_scroll_internal(obj, pd, efl_provider_find(new_obj, EFL_UI_ITEM_CLASS), .0, EINA_TRUE);
     }

   return new_obj;
}

static void
_selectable_range_apply(Eina_List *start, Eina_Bool flag)
{
   Efl_Ui_Selectable *sel;
   Eina_List *n;

   EINA_LIST_FOREACH(start, n, sel)
     {
        efl_ui_selectable_selected_set(sel, flag);
     }
}

EOLIAN static void
_efl_ui_collection_efl_ui_multi_selectable_all_select(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   _selectable_range_apply(pd->items, EINA_TRUE);
}

EOLIAN static void
_efl_ui_collection_efl_ui_multi_selectable_all_unselect(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   _selectable_range_apply(pd->items, EINA_FALSE);
}

static void
_range_selection_find(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Selectable *a, Efl_Ui_Selectable *b, Eina_Bool flag)
{
   Eina_List *n;
   Efl_Ui_Selectable *c;
   Eina_List *start = NULL, *end = NULL;

   EINA_SAFETY_ON_FALSE_RETURN(efl_ui_widget_parent_get(a) == obj);
   EINA_SAFETY_ON_FALSE_RETURN(efl_ui_widget_parent_get(b) == obj);

   EINA_LIST_FOREACH(pd->items, n, c)
     {
        if (!start)
          {
             if (c == a)
               start = n;
             else if (c == b)
               start = n;
          }
        else if (!end)
          {
             if (c == a)
               end = n;
             else if (c == b)
               end = n;
          }
        /* if we have found the first element, start applying the flag */
        if (start)
          efl_ui_selectable_selected_set(c, flag);
        if (end)
          break;
     }
}

EOLIAN static void
_efl_ui_collection_efl_ui_multi_selectable_object_range_range_select(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Selectable *a, Efl_Ui_Selectable *b)
{
   _range_selection_find(obj, pd, a, b, EINA_TRUE);
}

EOLIAN static void
_efl_ui_collection_efl_ui_multi_selectable_object_range_range_unselect(Eo *obj, Efl_Ui_Collection_Data *pd, Efl_Ui_Selectable *a, Efl_Ui_Selectable *b)
{
   _range_selection_find(obj, pd, a, b, EINA_FALSE);
}

EOLIAN static void
_efl_ui_collection_efl_ui_single_selectable_fallback_selection_set(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Efl_Ui_Selectable *fallback)
{
   pd->fallback = fallback;
   _apply_fallback(obj, pd);
}

EOLIAN static Efl_Ui_Selectable*
_efl_ui_collection_efl_ui_single_selectable_fallback_selection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return pd->fallback;
}

EOLIAN static void
_efl_ui_collection_efl_ui_single_selectable_allow_manual_deselection_set(Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd, Eina_Bool allow_manual_deselection)
{
   pd->allow_manual_deselection = !!allow_manual_deselection;
}

EOLIAN static Eina_Bool
_efl_ui_collection_efl_ui_single_selectable_allow_manual_deselection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_Data *pd)
{
   return pd->allow_manual_deselection;
}


#include "efl_ui_collection.eo.c"

#define ITEM_IS_OUTSIDE_VISIBLE(id) id < collection_pd->start_id || id > collection_pd->end_id

static inline void
_assert_item_available(Eo *item, unsigned int new_id, Efl_Ui_Collection_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN(new_id < eina_list_count(pd->items));
   efl_gfx_entity_visible_set(item, EINA_TRUE);
   efl_gfx_entity_geometry_set(item, efl_ui_position_manager_entity_position_single_item(pd->pos_man, new_id));
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_collection_focus_manager_efl_ui_focus_manager_request_move(Eo *obj, Efl_Ui_Collection_Focus_Manager_Data *pd, Efl_Ui_Focus_Direction direction, Efl_Ui_Focus_Object *child, Eina_Bool logical)
{
   MY_DATA_GET(pd->collection, collection_pd);
   Efl_Ui_Item *new_item, *item;
   unsigned int item_id;

   if (!child)
     child = efl_ui_focus_manager_focus_get(obj);

   item = _find_item(obj, collection_pd, child);

   //if this is NULL then we are before finalize, we cannot serve any sane value here
   if (!collection_pd->pos_man) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, NULL);

   item_id = efl_ui_item_index_get(item);

   if (ITEM_IS_OUTSIDE_VISIBLE(item_id))
     {
        unsigned int new_id;

        if (!efl_ui_position_manager_entity_relative_item(collection_pd->pos_man, efl_ui_item_index_get(item), direction, &new_id))
          {
             new_item = NULL;
          }
        else
          {
             new_item = eina_list_nth(collection_pd->items, new_id);;
             _assert_item_available(new_item, new_id, collection_pd);
          }
     }
   else
     {
        new_item = efl_ui_focus_manager_request_move(efl_super(obj, EFL_UI_COLLECTION_FOCUS_MANAGER_CLASS), direction, child, logical);
     }

   return new_item;
}


EOLIAN static void
_efl_ui_collection_focus_manager_efl_ui_focus_manager_manager_focus_set(Eo *obj, Efl_Ui_Collection_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *focus)
{
   MY_DATA_GET(pd->collection, collection_pd);
   Efl_Ui_Item *item;
   unsigned int item_id;

   if (focus == efl_ui_focus_manager_root_get(obj))
     {
        item = eina_list_data_get(collection_pd->items);
     }
   else
     {
        item = _find_item(obj, collection_pd, focus);
     }

   //if this is NULL then we are before finalize, we cannot serve any sane value here
   if (!collection_pd->pos_man) return;
   EINA_SAFETY_ON_NULL_RETURN(item);

   item_id = efl_ui_item_index_get(item);

   if (ITEM_IS_OUTSIDE_VISIBLE(item_id))
     {
        _assert_item_available(item, item_id, collection_pd);
     }
   efl_ui_focus_manager_focus_set(efl_super(obj, EFL_UI_COLLECTION_FOCUS_MANAGER_CLASS), focus);
}


#include "efl_ui_collection_focus_manager.eo.c"
