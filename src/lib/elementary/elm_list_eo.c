EWAPI const Efl_Event_Description _ELM_LIST_EVENT_ACTIVATED =
   EFL_EVENT_DESCRIPTION("activated");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_EDGE_TOP =
   EFL_EVENT_DESCRIPTION("edge,top");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_EDGE_BOTTOM =
   EFL_EVENT_DESCRIPTION("edge,bottom");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_EDGE_LEFT =
   EFL_EVENT_DESCRIPTION("edge,left");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_EDGE_RIGHT =
   EFL_EVENT_DESCRIPTION("edge,right");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_SWIPE =
   EFL_EVENT_DESCRIPTION("swipe");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_HIGHLIGHTED =
   EFL_EVENT_DESCRIPTION("highlighted");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_UNHIGHLIGHTED =
   EFL_EVENT_DESCRIPTION("unhighlighted");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_ITEM_FOCUSED =
   EFL_EVENT_DESCRIPTION("item,focused");
EWAPI const Efl_Event_Description _ELM_LIST_EVENT_ITEM_UNFOCUSED =
   EFL_EVENT_DESCRIPTION("item,unfocused");

void _elm_list_horizontal_set(Eo *obj, Elm_List_Data *pd, Eina_Bool horizontal);


static Eina_Error
__eolian_elm_list_horizontal_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_list_horizontal_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_horizontal_set, EFL_FUNC_CALL(horizontal), Eina_Bool horizontal);

Eina_Bool _elm_list_horizontal_get(const Eo *obj, Elm_List_Data *pd);


static Eina_Value
__eolian_elm_list_horizontal_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_list_horizontal_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_horizontal_get, Eina_Bool, 0);

void _elm_list_select_mode_set(Eo *obj, Elm_List_Data *pd, Elm_Object_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Select_Mode mode);

Elm_Object_Select_Mode _elm_list_select_mode_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_select_mode_get, Elm_Object_Select_Mode, 4 /* Elm.Object.Select_Mode.max */);

void _elm_list_focus_on_selection_set(Eo *obj, Elm_List_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_list_focus_on_selection_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_list_focus_on_selection_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_focus_on_selection_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

Eina_Bool _elm_list_focus_on_selection_get(const Eo *obj, Elm_List_Data *pd);


static Eina_Value
__eolian_elm_list_focus_on_selection_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_list_focus_on_selection_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_focus_on_selection_get, Eina_Bool, 0);

void _elm_list_multi_select_set(Eo *obj, Elm_List_Data *pd, Eina_Bool multi);


static Eina_Error
__eolian_elm_list_multi_select_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_list_multi_select_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_multi_select_set, EFL_FUNC_CALL(multi), Eina_Bool multi);

Eina_Bool _elm_list_multi_select_get(const Eo *obj, Elm_List_Data *pd);


static Eina_Value
__eolian_elm_list_multi_select_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_list_multi_select_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_multi_select_get, Eina_Bool, 0);

void _elm_list_multi_select_mode_set(Eo *obj, Elm_List_Data *pd, Elm_Object_Multi_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_multi_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Multi_Select_Mode mode);

Elm_Object_Multi_Select_Mode _elm_list_multi_select_mode_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_multi_select_mode_get, Elm_Object_Multi_Select_Mode, 2 /* Elm.Object.Multi_Select_Mode.max */);

void _elm_list_mode_set(Eo *obj, Elm_List_Data *pd, Elm_List_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_mode_set, EFL_FUNC_CALL(mode), Elm_List_Mode mode);

Elm_List_Mode _elm_list_mode_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_mode_get, Elm_List_Mode, 4 /* Elm.List.Mode.last */);

Elm_Widget_Item *_elm_list_selected_item_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_selected_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_list_items_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_list_first_item_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_first_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_list_selected_items_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_selected_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_list_last_item_get(const Eo *obj, Elm_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_last_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_list_item_insert_before(Eo *obj, Elm_List_Data *pd, Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_list_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(before, label, icon, end, func, data), Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

void _elm_list_go(Eo *obj, Elm_List_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_list_go);

Elm_Widget_Item *_elm_list_item_insert_after(Eo *obj, Elm_List_Data *pd, Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_list_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(after, label, icon, end, func, data), Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_list_at_xy_item_get(const Eo *obj, Elm_List_Data *pd, int x, int y, int *posret);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_list_at_xy_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(x, y, posret), int x, int y, int *posret);

Elm_Widget_Item *_elm_list_item_append(Eo *obj, Elm_List_Data *pd, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_list_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, end, func, data), const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_list_item_prepend(Eo *obj, Elm_List_Data *pd, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_list_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, end, func, data), const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data);

void _elm_list_clear(Eo *obj, Elm_List_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_list_clear);

Elm_Widget_Item *_elm_list_item_sorted_insert(Eo *obj, Elm_List_Data *pd, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func);

EOAPI EFL_FUNC_BODYV(elm_obj_list_item_sorted_insert, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, end, func, data, cmp_func), const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func);

Efl_Object *_elm_list_efl_object_constructor(Eo *obj, Elm_List_Data *pd);


void _elm_list_efl_gfx_entity_position_set(Eo *obj, Elm_List_Data *pd, Eina_Position2D pos);


void _elm_list_efl_gfx_entity_size_set(Eo *obj, Elm_List_Data *pd, Eina_Size2D size);


void _elm_list_efl_canvas_group_group_member_add(Eo *obj, Elm_List_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Error _elm_list_efl_ui_widget_theme_apply(Eo *obj, Elm_List_Data *pd);


void _elm_list_efl_ui_widget_on_access_update(Eo *obj, Elm_List_Data *pd, Eina_Bool enable);


Eina_Rect _elm_list_efl_ui_widget_focus_highlight_geometry_get(const Eo *obj, Elm_List_Data *pd);


void _elm_list_efl_ui_widget_disabled_set(Eo *obj, Elm_List_Data *pd, Eina_Bool disabled);


Eina_Bool _elm_list_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_List_Data *pd);


void _elm_list_efl_ui_l10n_translation_update(Eo *obj, Elm_List_Data *pd);


Eina_Bool _elm_list_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_List_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_list_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_List_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Elm_Widget_Item *_elm_list_elm_widget_item_container_focused_item_get(const Eo *obj, Elm_List_Data *pd);


Eina_Bool _elm_list_efl_ui_widget_focus_state_apply(Eo *obj, Elm_List_Data *pd, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect);


void _elm_list_elm_interface_scrollable_item_loop_enabled_set(Eo *obj, Elm_List_Data *pd, Eina_Bool enable);


Eina_Bool _elm_list_elm_interface_scrollable_item_loop_enabled_get(const Eo *obj, Elm_List_Data *pd);


void _elm_list_elm_interface_scrollable_policy_set(Eo *obj, Elm_List_Data *pd, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar);


const Efl_Access_Action_Data *_elm_list_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_List_Data *pd);


Eina_List *_elm_list_efl_access_object_access_children_get(const Eo *obj, Elm_List_Data *pd);


int _elm_list_efl_access_selection_selected_children_count_get(const Eo *obj, Elm_List_Data *pd);


Efl_Object *_elm_list_efl_access_selection_selected_child_get(const Eo *obj, Elm_List_Data *pd, int selected_child_index);


Eina_Bool _elm_list_efl_access_selection_selected_child_deselect(Eo *obj, Elm_List_Data *pd, int child_index);


Eina_Bool _elm_list_efl_access_selection_child_select(Eo *obj, Elm_List_Data *pd, int child_index);


Eina_Bool _elm_list_efl_access_selection_child_deselect(Eo *obj, Elm_List_Data *pd, int child_index);


Eina_Bool _elm_list_efl_access_selection_is_child_selected(Eo *obj, Elm_List_Data *pd, int child_index);


Eina_Bool _elm_list_efl_access_selection_all_children_select(Eo *obj, Elm_List_Data *pd);


Eina_Bool _elm_list_efl_access_selection_access_selection_clear(Eo *obj, Elm_List_Data *pd);


static Eina_Bool
_elm_list_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_LIST_EXTRA_OPS
#define ELM_LIST_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_list_horizontal_set, _elm_list_horizontal_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_horizontal_get, _elm_list_horizontal_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_select_mode_set, _elm_list_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_select_mode_get, _elm_list_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_focus_on_selection_set, _elm_list_focus_on_selection_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_focus_on_selection_get, _elm_list_focus_on_selection_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_multi_select_set, _elm_list_multi_select_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_multi_select_get, _elm_list_multi_select_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_multi_select_mode_set, _elm_list_multi_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_multi_select_mode_get, _elm_list_multi_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_mode_set, _elm_list_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_mode_get, _elm_list_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_selected_item_get, _elm_list_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_items_get, _elm_list_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_first_item_get, _elm_list_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_selected_items_get, _elm_list_selected_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_last_item_get, _elm_list_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_insert_before, _elm_list_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_list_go, _elm_list_go),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_insert_after, _elm_list_item_insert_after),
      EFL_OBJECT_OP_FUNC(elm_obj_list_at_xy_item_get, _elm_list_at_xy_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_append, _elm_list_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_prepend, _elm_list_item_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_list_clear, _elm_list_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_sorted_insert, _elm_list_item_sorted_insert),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_list_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_list_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_list_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_list_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_list_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_list_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_highlight_geometry_get, _elm_list_efl_ui_widget_focus_highlight_geometry_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_disabled_set, _elm_list_efl_ui_widget_disabled_set),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_list_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_list_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_list_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_list_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(elm_widget_item_container_focused_item_get, _elm_list_elm_widget_item_container_focused_item_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_state_apply, _elm_list_efl_ui_widget_focus_state_apply),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_item_loop_enabled_set, _elm_list_elm_interface_scrollable_item_loop_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_item_loop_enabled_get, _elm_list_elm_interface_scrollable_item_loop_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_set, _elm_list_elm_interface_scrollable_policy_set),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_list_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_list_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_children_count_get, _elm_list_efl_access_selection_selected_children_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_get, _elm_list_efl_access_selection_selected_child_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_deselect, _elm_list_efl_access_selection_selected_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_select, _elm_list_efl_access_selection_child_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_deselect, _elm_list_efl_access_selection_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_is_child_selected, _elm_list_efl_access_selection_is_child_selected),
      EFL_OBJECT_OP_FUNC(efl_access_selection_all_children_select, _elm_list_efl_access_selection_all_children_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_clear, _elm_list_efl_access_selection_access_selection_clear),
      ELM_LIST_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"horizontal", __eolian_elm_list_horizontal_set_reflect, __eolian_elm_list_horizontal_get_reflect},
      {"focus_on_selection", __eolian_elm_list_focus_on_selection_set_reflect, __eolian_elm_list_focus_on_selection_get_reflect},
      {"multi_select", __eolian_elm_list_multi_select_set_reflect, __eolian_elm_list_multi_select_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_list_class_desc = {
   EO_VERSION,
   "Elm.List",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_List_Data),
   _elm_list_class_initializer,
   _elm_list_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_list_class_get, &_elm_list_class_desc, EFL_UI_LAYOUT_BASE_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_ACCESS_SELECTION_INTERFACE, EFL_ACCESS_OBJECT_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, ELM_WIDGET_ITEM_CONTAINER_INTERFACE, NULL);

#include "elm_list_eo.legacy.c"
