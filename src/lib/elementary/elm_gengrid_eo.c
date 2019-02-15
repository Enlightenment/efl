EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_REALIZED =
   EFL_EVENT_DESCRIPTION("realized");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_UNREALIZED =
   EFL_EVENT_DESCRIPTION("unrealized");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_INDEX_UPDATE =
   EFL_EVENT_DESCRIPTION("index,update");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_SCROLL_PAGE_CHANGED =
   EFL_EVENT_DESCRIPTION("scroll,page,changed");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_EDGE_BOTTOM =
   EFL_EVENT_DESCRIPTION("edge,bottom");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_EDGE_TOP =
   EFL_EVENT_DESCRIPTION("edge,top");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_EDGE_RIGHT =
   EFL_EVENT_DESCRIPTION("edge,right");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_EDGE_LEFT =
   EFL_EVENT_DESCRIPTION("edge,left");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_ITEM_FOCUSED =
   EFL_EVENT_DESCRIPTION("item,focused");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_ITEM_UNFOCUSED =
   EFL_EVENT_DESCRIPTION("item,unfocused");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_ITEM_REORDER_ANIM_START =
   EFL_EVENT_DESCRIPTION("item,reorder,anim,start");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_ITEM_REORDER_ANIM_STOP =
   EFL_EVENT_DESCRIPTION("item,reorder,anim,stop");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_ACTIVATED =
   EFL_EVENT_DESCRIPTION("activated");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_HIGHLIGHTED =
   EFL_EVENT_DESCRIPTION("highlighted");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_UNHIGHLIGHTED =
   EFL_EVENT_DESCRIPTION("unhighlighted");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_RELEASED =
   EFL_EVENT_DESCRIPTION("released");
EWAPI const Efl_Event_Description _ELM_GENGRID_EVENT_MOVED =
   EFL_EVENT_DESCRIPTION("moved");

void _elm_gengrid_align_set(Eo *obj, Elm_Gengrid_Data *pd, double align_x, double align_y);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_align_set, EFL_FUNC_CALL(align_x, align_y), double align_x, double align_y);

void _elm_gengrid_align_get(const Eo *obj, Elm_Gengrid_Data *pd, double *align_x, double *align_y);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_gengrid_align_get, EFL_FUNC_CALL(align_x, align_y), double *align_x, double *align_y);

void _elm_gengrid_filled_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool fill);


static Eina_Error
__eolian_elm_gengrid_filled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gengrid_filled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_filled_set, EFL_FUNC_CALL(fill), Eina_Bool fill);

Eina_Bool _elm_gengrid_filled_get(const Eo *obj, Elm_Gengrid_Data *pd);


static Eina_Value
__eolian_elm_gengrid_filled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_gengrid_filled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_filled_get, Eina_Bool, 0);

void _elm_gengrid_multi_select_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool multi);


static Eina_Error
__eolian_elm_gengrid_multi_select_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gengrid_multi_select_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_multi_select_set, EFL_FUNC_CALL(multi), Eina_Bool multi);

Eina_Bool _elm_gengrid_multi_select_get(const Eo *obj, Elm_Gengrid_Data *pd);


static Eina_Value
__eolian_elm_gengrid_multi_select_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_gengrid_multi_select_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_multi_select_get, Eina_Bool, 0);

void _elm_gengrid_group_item_size_set(Eo *obj, Elm_Gengrid_Data *pd, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_group_item_size_set, EFL_FUNC_CALL(w, h), int w, int h);

void _elm_gengrid_group_item_size_get(const Eo *obj, Elm_Gengrid_Data *pd, int *w, int *h);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_gengrid_group_item_size_get, EFL_FUNC_CALL(w, h), int *w, int *h);

void _elm_gengrid_select_mode_set(Eo *obj, Elm_Gengrid_Data *pd, Elm_Object_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Select_Mode mode);

Elm_Object_Select_Mode _elm_gengrid_select_mode_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_select_mode_get, Elm_Object_Select_Mode, 4 /* Elm.Object.Select_Mode.max */);

void _elm_gengrid_reorder_mode_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool reorder_mode);


static Eina_Error
__eolian_elm_gengrid_reorder_mode_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gengrid_reorder_mode_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_reorder_mode_set, EFL_FUNC_CALL(reorder_mode), Eina_Bool reorder_mode);

Eina_Bool _elm_gengrid_reorder_mode_get(const Eo *obj, Elm_Gengrid_Data *pd);


static Eina_Value
__eolian_elm_gengrid_reorder_mode_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_gengrid_reorder_mode_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_reorder_mode_get, Eina_Bool, 0);

void _elm_gengrid_highlight_mode_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool highlight);


static Eina_Error
__eolian_elm_gengrid_highlight_mode_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gengrid_highlight_mode_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_highlight_mode_set, EFL_FUNC_CALL(highlight), Eina_Bool highlight);

Eina_Bool _elm_gengrid_highlight_mode_get(const Eo *obj, Elm_Gengrid_Data *pd);


static Eina_Value
__eolian_elm_gengrid_highlight_mode_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_gengrid_highlight_mode_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_highlight_mode_get, Eina_Bool, 0);

void _elm_gengrid_reorder_type_set(Eo *obj, Elm_Gengrid_Data *pd, Elm_Gengrid_Reorder_Type type);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_reorder_type_set, EFL_FUNC_CALL(type), Elm_Gengrid_Reorder_Type type);

void _elm_gengrid_item_size_set(Eo *obj, Elm_Gengrid_Data *pd, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_item_size_set, EFL_FUNC_CALL(w, h), int w, int h);

void _elm_gengrid_item_size_get(const Eo *obj, Elm_Gengrid_Data *pd, int *w, int *h);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_gengrid_item_size_get, EFL_FUNC_CALL(w, h), int *w, int *h);

void _elm_gengrid_multi_select_mode_set(Eo *obj, Elm_Gengrid_Data *pd, Elm_Object_Multi_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_multi_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Multi_Select_Mode mode);

Elm_Object_Multi_Select_Mode _elm_gengrid_multi_select_mode_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_multi_select_mode_get, Elm_Object_Multi_Select_Mode, 2 /* Elm.Object.Multi_Select_Mode.max */);

void _elm_gengrid_horizontal_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool horizontal);


static Eina_Error
__eolian_elm_gengrid_horizontal_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gengrid_horizontal_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_horizontal_set, EFL_FUNC_CALL(horizontal), Eina_Bool horizontal);

Eina_Bool _elm_gengrid_horizontal_get(const Eo *obj, Elm_Gengrid_Data *pd);


static Eina_Value
__eolian_elm_gengrid_horizontal_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_gengrid_horizontal_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_horizontal_get, Eina_Bool, 0);

Elm_Widget_Item *_elm_gengrid_selected_item_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_selected_item_get, Elm_Widget_Item *, NULL);

Eina_List *_elm_gengrid_realized_items_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_realized_items_get, Eina_List *, NULL);

Elm_Widget_Item *_elm_gengrid_first_item_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_first_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_gengrid_selected_items_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_selected_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_gengrid_last_item_get(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_last_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_gengrid_item_insert_before(Eo *obj, Elm_Gengrid_Data *pd, const Elm_Gengrid_Item_Class *itc, const void *data, Elm_Widget_Item *relative, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_gengrid_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, relative, func, func_data), const Elm_Gengrid_Item_Class *itc, const void *data, Elm_Widget_Item *relative, Evas_Smart_Cb func, const void *func_data);

void _elm_gengrid_realized_items_update(Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_gengrid_realized_items_update);

Elm_Widget_Item *_elm_gengrid_item_insert_after(Eo *obj, Elm_Gengrid_Data *pd, const Elm_Gengrid_Item_Class *itc, const void *data, Elm_Widget_Item *relative, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_gengrid_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, relative, func, func_data), const Elm_Gengrid_Item_Class *itc, const void *data, Elm_Widget_Item *relative, Evas_Smart_Cb func, const void *func_data);

unsigned int _elm_gengrid_items_count(const Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gengrid_items_count, unsigned int, 0);

Elm_Widget_Item *_elm_gengrid_at_xy_item_get(const Eo *obj, Elm_Gengrid_Data *pd, int x, int y, int *xposret, int *yposret);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_gengrid_at_xy_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(x, y, xposret, yposret), int x, int y, int *xposret, int *yposret);

Elm_Widget_Item *_elm_gengrid_item_append(Eo *obj, Elm_Gengrid_Data *pd, const Elm_Gengrid_Item_Class *itc, const void *data, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_gengrid_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, func, func_data), const Elm_Gengrid_Item_Class *itc, const void *data, Evas_Smart_Cb func, const void *func_data);

Elm_Widget_Item *_elm_gengrid_item_prepend(Eo *obj, Elm_Gengrid_Data *pd, const Elm_Gengrid_Item_Class *itc, const void *data, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_gengrid_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, func, func_data), const Elm_Gengrid_Item_Class *itc, const void *data, Evas_Smart_Cb func, const void *func_data);

void _elm_gengrid_clear(Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_gengrid_clear);

Elm_Widget_Item *_elm_gengrid_item_sorted_insert(Eo *obj, Elm_Gengrid_Data *pd, const Elm_Gengrid_Item_Class *itc, const void *data, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_gengrid_item_sorted_insert, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, comp, func, func_data), const Elm_Gengrid_Item_Class *itc, const void *data, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

Elm_Widget_Item *_elm_gengrid_search_by_text_item_get(Eo *obj, Elm_Gengrid_Data *pd, Elm_Widget_Item *item_to_search_from, const char *part_name, const char *pattern, Elm_Glob_Match_Flags flags);

EOAPI EFL_FUNC_BODYV(elm_obj_gengrid_search_by_text_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(item_to_search_from, part_name, pattern, flags), Elm_Widget_Item *item_to_search_from, const char *part_name, const char *pattern, Elm_Glob_Match_Flags flags);

void _elm_gengrid_reorder_mode_start(Eo *obj, Elm_Gengrid_Data *pd, Ecore_Pos_Map tween_mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gengrid_reorder_mode_start, EFL_FUNC_CALL(tween_mode), Ecore_Pos_Map tween_mode);

void _elm_gengrid_reorder_mode_stop(Eo *obj, Elm_Gengrid_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_gengrid_reorder_mode_stop);

Efl_Object *_elm_gengrid_efl_object_constructor(Eo *obj, Elm_Gengrid_Data *pd);


Efl_Object *_elm_gengrid_efl_object_provider_find(const Eo *obj, Elm_Gengrid_Data *pd, const Efl_Class *klass);


void _elm_gengrid_efl_gfx_entity_position_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Position2D pos);


void _elm_gengrid_efl_gfx_entity_size_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Size2D size);


void _elm_gengrid_efl_canvas_group_group_member_add(Eo *obj, Elm_Gengrid_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Error _elm_gengrid_efl_ui_widget_theme_apply(Eo *obj, Elm_Gengrid_Data *pd);


void _elm_gengrid_efl_ui_widget_on_access_update(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool enable);


Eina_Rect _elm_gengrid_efl_ui_widget_interest_region_get(const Eo *obj, Elm_Gengrid_Data *pd);


Eina_Bool _elm_gengrid_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Gengrid_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Eina_Rect _elm_gengrid_efl_ui_widget_focus_highlight_geometry_get(const Eo *obj, Elm_Gengrid_Data *pd);


Elm_Widget_Item *_elm_gengrid_elm_widget_item_container_focused_item_get(const Eo *obj, Elm_Gengrid_Data *pd);


void _elm_gengrid_elm_interface_scrollable_item_loop_enabled_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool enable);


Eina_Bool _elm_gengrid_elm_interface_scrollable_item_loop_enabled_get(const Eo *obj, Elm_Gengrid_Data *pd);


void _elm_gengrid_elm_interface_scrollable_bounce_allow_set(Eo *obj, Elm_Gengrid_Data *pd, Eina_Bool horiz, Eina_Bool vert);


void _elm_gengrid_elm_interface_scrollable_policy_set(Eo *obj, Elm_Gengrid_Data *pd, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar);


void _elm_gengrid_elm_interface_scrollable_policy_get(const Eo *obj, Elm_Gengrid_Data *pd, Elm_Scroller_Policy *hbar, Elm_Scroller_Policy *vbar);


Eina_List *_elm_gengrid_efl_access_object_access_children_get(const Eo *obj, Elm_Gengrid_Data *pd);


Efl_Access_State_Set _elm_gengrid_efl_access_object_state_set_get(const Eo *obj, Elm_Gengrid_Data *pd);


const Efl_Access_Action_Data *_elm_gengrid_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Gengrid_Data *pd);


int _elm_gengrid_efl_access_selection_selected_children_count_get(const Eo *obj, Elm_Gengrid_Data *pd);


Efl_Object *_elm_gengrid_efl_access_selection_selected_child_get(const Eo *obj, Elm_Gengrid_Data *pd, int selected_child_index);


Eina_Bool _elm_gengrid_efl_access_selection_selected_child_deselect(Eo *obj, Elm_Gengrid_Data *pd, int child_index);


Eina_Bool _elm_gengrid_efl_access_selection_child_select(Eo *obj, Elm_Gengrid_Data *pd, int child_index);


Eina_Bool _elm_gengrid_efl_access_selection_child_deselect(Eo *obj, Elm_Gengrid_Data *pd, int child_index);


Eina_Bool _elm_gengrid_efl_access_selection_is_child_selected(Eo *obj, Elm_Gengrid_Data *pd, int child_index);


Eina_Bool _elm_gengrid_efl_access_selection_all_children_select(Eo *obj, Elm_Gengrid_Data *pd);


Eina_Bool _elm_gengrid_efl_access_selection_access_selection_clear(Eo *obj, Elm_Gengrid_Data *pd);


void _elm_gengrid_efl_ui_focus_composition_prepare(Eo *obj, Elm_Gengrid_Data *pd);


Eina_Bool _elm_gengrid_efl_ui_widget_focus_state_apply(Eo *obj, Elm_Gengrid_Data *pd, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect);


void _elm_gengrid_efl_ui_focus_manager_setup_on_first_touch(Eo *obj, Elm_Gengrid_Data *pd, Efl_Ui_Focus_Direction direction, Efl_Ui_Focus_Object *entry);


Efl_Ui_Focus_Object *_elm_gengrid_efl_ui_focus_manager_manager_focus_get(const Eo *obj, Elm_Gengrid_Data *pd);


Efl_Ui_Focus_Object *_elm_gengrid_efl_ui_focus_manager_move(Eo *obj, Elm_Gengrid_Data *pd, Efl_Ui_Focus_Direction direction);


static Eina_Bool
_elm_gengrid_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_GENGRID_EXTRA_OPS
#define ELM_GENGRID_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_align_set, _elm_gengrid_align_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_align_get, _elm_gengrid_align_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_filled_set, _elm_gengrid_filled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_filled_get, _elm_gengrid_filled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_multi_select_set, _elm_gengrid_multi_select_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_multi_select_get, _elm_gengrid_multi_select_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_group_item_size_set, _elm_gengrid_group_item_size_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_group_item_size_get, _elm_gengrid_group_item_size_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_select_mode_set, _elm_gengrid_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_select_mode_get, _elm_gengrid_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_reorder_mode_set, _elm_gengrid_reorder_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_reorder_mode_get, _elm_gengrid_reorder_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_highlight_mode_set, _elm_gengrid_highlight_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_highlight_mode_get, _elm_gengrid_highlight_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_reorder_type_set, _elm_gengrid_reorder_type_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_size_set, _elm_gengrid_item_size_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_size_get, _elm_gengrid_item_size_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_multi_select_mode_set, _elm_gengrid_multi_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_multi_select_mode_get, _elm_gengrid_multi_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_horizontal_set, _elm_gengrid_horizontal_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_horizontal_get, _elm_gengrid_horizontal_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_selected_item_get, _elm_gengrid_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_realized_items_get, _elm_gengrid_realized_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_first_item_get, _elm_gengrid_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_selected_items_get, _elm_gengrid_selected_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_last_item_get, _elm_gengrid_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_insert_before, _elm_gengrid_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_realized_items_update, _elm_gengrid_realized_items_update),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_insert_after, _elm_gengrid_item_insert_after),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_items_count, _elm_gengrid_items_count),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_at_xy_item_get, _elm_gengrid_at_xy_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_append, _elm_gengrid_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_prepend, _elm_gengrid_item_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_clear, _elm_gengrid_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_item_sorted_insert, _elm_gengrid_item_sorted_insert),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_search_by_text_item_get, _elm_gengrid_search_by_text_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_reorder_mode_start, _elm_gengrid_reorder_mode_start),
      EFL_OBJECT_OP_FUNC(elm_obj_gengrid_reorder_mode_stop, _elm_gengrid_reorder_mode_stop),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_gengrid_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_provider_find, _elm_gengrid_efl_object_provider_find),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_gengrid_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_gengrid_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_gengrid_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_gengrid_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_gengrid_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_interest_region_get, _elm_gengrid_efl_ui_widget_interest_region_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_gengrid_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_highlight_geometry_get, _elm_gengrid_efl_ui_widget_focus_highlight_geometry_get),
      EFL_OBJECT_OP_FUNC(elm_widget_item_container_focused_item_get, _elm_gengrid_elm_widget_item_container_focused_item_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_item_loop_enabled_set, _elm_gengrid_elm_interface_scrollable_item_loop_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_item_loop_enabled_get, _elm_gengrid_elm_interface_scrollable_item_loop_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_bounce_allow_set, _elm_gengrid_elm_interface_scrollable_bounce_allow_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_set, _elm_gengrid_elm_interface_scrollable_policy_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_get, _elm_gengrid_elm_interface_scrollable_policy_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_gengrid_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_gengrid_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_gengrid_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_children_count_get, _elm_gengrid_efl_access_selection_selected_children_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_get, _elm_gengrid_efl_access_selection_selected_child_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_deselect, _elm_gengrid_efl_access_selection_selected_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_select, _elm_gengrid_efl_access_selection_child_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_deselect, _elm_gengrid_efl_access_selection_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_is_child_selected, _elm_gengrid_efl_access_selection_is_child_selected),
      EFL_OBJECT_OP_FUNC(efl_access_selection_all_children_select, _elm_gengrid_efl_access_selection_all_children_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_clear, _elm_gengrid_efl_access_selection_access_selection_clear),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_composition_prepare, _elm_gengrid_efl_ui_focus_composition_prepare),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_state_apply, _elm_gengrid_efl_ui_widget_focus_state_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_setup_on_first_touch, _elm_gengrid_efl_ui_focus_manager_setup_on_first_touch),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_focus_get, _elm_gengrid_efl_ui_focus_manager_manager_focus_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_move, _elm_gengrid_efl_ui_focus_manager_move),
      ELM_GENGRID_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"filled", __eolian_elm_gengrid_filled_set_reflect, __eolian_elm_gengrid_filled_get_reflect},
      {"multi_select", __eolian_elm_gengrid_multi_select_set_reflect, __eolian_elm_gengrid_multi_select_get_reflect},
      {"reorder_mode", __eolian_elm_gengrid_reorder_mode_set_reflect, __eolian_elm_gengrid_reorder_mode_get_reflect},
      {"highlight_mode", __eolian_elm_gengrid_highlight_mode_set_reflect, __eolian_elm_gengrid_highlight_mode_get_reflect},
      {"horizontal", __eolian_elm_gengrid_horizontal_set_reflect, __eolian_elm_gengrid_horizontal_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_gengrid_class_desc = {
   EO_VERSION,
   "Elm.Gengrid",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Gengrid_Data),
   _elm_gengrid_class_initializer,
   _elm_gengrid_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_gengrid_class_get, &_elm_gengrid_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_ACCESS_SELECTION_INTERFACE, EFL_UI_LEGACY_INTERFACE, ELM_WIDGET_ITEM_CONTAINER_INTERFACE, NULL);

#include "elm_gengrid_eo.legacy.c"
