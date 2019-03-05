EWAPI const Efl_Event_Description _ELM_TOOLBAR_EVENT_ITEM_FOCUSED =
   EFL_EVENT_DESCRIPTION("item,focused");
EWAPI const Efl_Event_Description _ELM_TOOLBAR_EVENT_ITEM_UNFOCUSED =
   EFL_EVENT_DESCRIPTION("item,unfocused");

Elm_Widget_Item *_elm_toolbar_selected_item_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_toolbar_first_item_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_first_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_toolbar_last_item_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_last_item_get, Elm_Widget_Item *, NULL);

Eina_Iterator *_elm_toolbar_items_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_items_get, Eina_Iterator *, NULL);

void _elm_toolbar_homogeneous_set(Eo *obj, Elm_Toolbar_Data *pd, Eina_Bool homogeneous);


static Eina_Error
__eolian_elm_toolbar_homogeneous_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_homogeneous_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_homogeneous_set, EFL_FUNC_CALL(homogeneous), Eina_Bool homogeneous);

Eina_Bool _elm_toolbar_homogeneous_get(const Eo *obj, Elm_Toolbar_Data *pd);


static Eina_Value
__eolian_elm_toolbar_homogeneous_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_toolbar_homogeneous_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_homogeneous_get, Eina_Bool, 0);

void _elm_toolbar_align_set(Eo *obj, Elm_Toolbar_Data *pd, double align);


static Eina_Error
__eolian_elm_toolbar_align_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_align_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_align_set, EFL_FUNC_CALL(align), double align);

double _elm_toolbar_align_get(const Eo *obj, Elm_Toolbar_Data *pd);


static Eina_Value
__eolian_elm_toolbar_align_get_reflect(Eo *obj)
{
   double val = elm_obj_toolbar_align_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_align_get, double, 0);

void _elm_toolbar_select_mode_set(Eo *obj, Elm_Toolbar_Data *pd, Elm_Object_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Select_Mode mode);

Elm_Object_Select_Mode _elm_toolbar_select_mode_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_select_mode_get, Elm_Object_Select_Mode, 4 /* Elm.Object.Select_Mode.max */);

void _elm_toolbar_icon_size_set(Eo *obj, Elm_Toolbar_Data *pd, int icon_size);


static Eina_Error
__eolian_elm_toolbar_icon_size_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_icon_size_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_icon_size_set, EFL_FUNC_CALL(icon_size), int icon_size);

int _elm_toolbar_icon_size_get(const Eo *obj, Elm_Toolbar_Data *pd);


static Eina_Value
__eolian_elm_toolbar_icon_size_get_reflect(Eo *obj)
{
   int val = elm_obj_toolbar_icon_size_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_icon_size_get, int, 0);

void _elm_toolbar_shrink_mode_set(Eo *obj, Elm_Toolbar_Data *pd, Elm_Toolbar_Shrink_Mode shrink_mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_shrink_mode_set, EFL_FUNC_CALL(shrink_mode), Elm_Toolbar_Shrink_Mode shrink_mode);

Elm_Toolbar_Shrink_Mode _elm_toolbar_shrink_mode_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_shrink_mode_get, Elm_Toolbar_Shrink_Mode, 0);

void _elm_toolbar_menu_parent_set(Eo *obj, Elm_Toolbar_Data *pd, Efl_Canvas_Object *parent);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_menu_parent_set, EFL_FUNC_CALL(parent), Efl_Canvas_Object *parent);

Efl_Canvas_Object *_elm_toolbar_menu_parent_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_menu_parent_get, Efl_Canvas_Object *, NULL);

void _elm_toolbar_standard_priority_set(Eo *obj, Elm_Toolbar_Data *pd, int priority);


static Eina_Error
__eolian_elm_toolbar_standard_priority_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_standard_priority_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_standard_priority_set, EFL_FUNC_CALL(priority), int priority);

int _elm_toolbar_standard_priority_get(const Eo *obj, Elm_Toolbar_Data *pd);


static Eina_Value
__eolian_elm_toolbar_standard_priority_get_reflect(Eo *obj)
{
   int val = elm_obj_toolbar_standard_priority_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_standard_priority_get, int, 0);

Elm_Widget_Item *_elm_toolbar_more_item_get(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_more_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_toolbar_item_insert_before(Eo *obj, Elm_Toolbar_Data *pd, Elm_Widget_Item *before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(before, icon, label, func, data), Elm_Widget_Item *before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_toolbar_item_insert_after(Eo *obj, Elm_Toolbar_Data *pd, Elm_Widget_Item *after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(after, icon, label, func, data), Elm_Widget_Item *after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_toolbar_item_append(Eo *obj, Elm_Toolbar_Data *pd, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(icon, label, func, data), const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

unsigned int _elm_toolbar_items_count(const Eo *obj, Elm_Toolbar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_items_count, unsigned int, 0);

Elm_Widget_Item *_elm_toolbar_item_prepend(Eo *obj, Elm_Toolbar_Data *pd, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(icon, label, func, data), const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_toolbar_item_find_by_label(const Eo *obj, Elm_Toolbar_Data *pd, const char *label);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_toolbar_item_find_by_label, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label), const char *label);

Efl_Object *_elm_toolbar_efl_object_constructor(Eo *obj, Elm_Toolbar_Data *pd);


void _elm_toolbar_efl_gfx_entity_position_set(Eo *obj, Elm_Toolbar_Data *pd, Eina_Position2D pos);


void _elm_toolbar_efl_gfx_entity_size_set(Eo *obj, Elm_Toolbar_Data *pd, Eina_Size2D size);


void _elm_toolbar_efl_canvas_group_group_calculate(Eo *obj, Elm_Toolbar_Data *pd);


void _elm_toolbar_efl_canvas_group_group_member_add(Eo *obj, Elm_Toolbar_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_toolbar_efl_ui_widget_on_access_update(Eo *obj, Elm_Toolbar_Data *pd, Eina_Bool enable);


Eina_Error _elm_toolbar_efl_ui_widget_theme_apply(Eo *obj, Elm_Toolbar_Data *pd);


Eina_Bool _elm_toolbar_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Toolbar_Data *pd);


void _elm_toolbar_efl_ui_l10n_translation_update(Eo *obj, Elm_Toolbar_Data *pd);


Eina_Bool _elm_toolbar_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Toolbar_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Eina_Rect _elm_toolbar_efl_ui_widget_focus_highlight_geometry_get(const Eo *obj, Elm_Toolbar_Data *pd);


Elm_Widget_Item *_elm_toolbar_elm_widget_item_container_focused_item_get(const Eo *obj, Elm_Toolbar_Data *pd);


void _elm_toolbar_efl_ui_direction_direction_set(Eo *obj, Elm_Toolbar_Data *pd, Efl_Ui_Dir dir);


Efl_Ui_Dir _elm_toolbar_efl_ui_direction_direction_get(const Eo *obj, Elm_Toolbar_Data *pd);


Eina_Bool _elm_toolbar_efl_ui_widget_focus_state_apply(Eo *obj, Elm_Toolbar_Data *pd, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect);


const Efl_Access_Action_Data *_elm_toolbar_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Toolbar_Data *pd);


Eina_List *_elm_toolbar_efl_access_object_access_children_get(const Eo *obj, Elm_Toolbar_Data *pd);


Efl_Access_State_Set _elm_toolbar_efl_access_object_state_set_get(const Eo *obj, Elm_Toolbar_Data *pd);


int _elm_toolbar_efl_access_selection_selected_children_count_get(const Eo *obj, Elm_Toolbar_Data *pd);


Efl_Object *_elm_toolbar_efl_access_selection_selected_child_get(const Eo *obj, Elm_Toolbar_Data *pd, int selected_child_index);


Eina_Bool _elm_toolbar_efl_access_selection_selected_child_deselect(Eo *obj, Elm_Toolbar_Data *pd, int child_index);


Eina_Bool _elm_toolbar_efl_access_selection_child_select(Eo *obj, Elm_Toolbar_Data *pd, int child_index);


Eina_Bool _elm_toolbar_efl_access_selection_child_deselect(Eo *obj, Elm_Toolbar_Data *pd, int child_index);


Eina_Bool _elm_toolbar_efl_access_selection_is_child_selected(Eo *obj, Elm_Toolbar_Data *pd, int child_index);


Eina_Bool _elm_toolbar_efl_access_selection_all_children_select(Eo *obj, Elm_Toolbar_Data *pd);


Eina_Bool _elm_toolbar_efl_access_selection_access_selection_clear(Eo *obj, Elm_Toolbar_Data *pd);


void _elm_toolbar_efl_ui_focus_composition_prepare(Eo *obj, Elm_Toolbar_Data *pd);


static Eina_Bool
_elm_toolbar_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_TOOLBAR_EXTRA_OPS
#define ELM_TOOLBAR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_selected_item_get, _elm_toolbar_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_first_item_get, _elm_toolbar_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_last_item_get, _elm_toolbar_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_items_get, _elm_toolbar_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_homogeneous_set, _elm_toolbar_homogeneous_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_homogeneous_get, _elm_toolbar_homogeneous_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_align_set, _elm_toolbar_align_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_align_get, _elm_toolbar_align_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_select_mode_set, _elm_toolbar_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_select_mode_get, _elm_toolbar_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_icon_size_set, _elm_toolbar_icon_size_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_icon_size_get, _elm_toolbar_icon_size_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_shrink_mode_set, _elm_toolbar_shrink_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_shrink_mode_get, _elm_toolbar_shrink_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_menu_parent_set, _elm_toolbar_menu_parent_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_menu_parent_get, _elm_toolbar_menu_parent_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_standard_priority_set, _elm_toolbar_standard_priority_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_standard_priority_get, _elm_toolbar_standard_priority_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_more_item_get, _elm_toolbar_more_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_insert_before, _elm_toolbar_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_insert_after, _elm_toolbar_item_insert_after),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_append, _elm_toolbar_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_items_count, _elm_toolbar_items_count),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_prepend, _elm_toolbar_item_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_find_by_label, _elm_toolbar_item_find_by_label),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_toolbar_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_toolbar_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_toolbar_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _elm_toolbar_efl_canvas_group_group_calculate),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_toolbar_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_toolbar_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_toolbar_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_toolbar_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_toolbar_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_toolbar_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_highlight_geometry_get, _elm_toolbar_efl_ui_widget_focus_highlight_geometry_get),
      EFL_OBJECT_OP_FUNC(elm_widget_item_container_focused_item_get, _elm_toolbar_elm_widget_item_container_focused_item_get),
      EFL_OBJECT_OP_FUNC(efl_ui_direction_set, _elm_toolbar_efl_ui_direction_direction_set),
      EFL_OBJECT_OP_FUNC(efl_ui_direction_get, _elm_toolbar_efl_ui_direction_direction_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_state_apply, _elm_toolbar_efl_ui_widget_focus_state_apply),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_toolbar_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_toolbar_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_toolbar_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_children_count_get, _elm_toolbar_efl_access_selection_selected_children_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_get, _elm_toolbar_efl_access_selection_selected_child_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_deselect, _elm_toolbar_efl_access_selection_selected_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_select, _elm_toolbar_efl_access_selection_child_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_deselect, _elm_toolbar_efl_access_selection_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_is_child_selected, _elm_toolbar_efl_access_selection_is_child_selected),
      EFL_OBJECT_OP_FUNC(efl_access_selection_all_children_select, _elm_toolbar_efl_access_selection_all_children_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_clear, _elm_toolbar_efl_access_selection_access_selection_clear),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_composition_prepare, _elm_toolbar_efl_ui_focus_composition_prepare),
      ELM_TOOLBAR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"homogeneous", __eolian_elm_toolbar_homogeneous_set_reflect, __eolian_elm_toolbar_homogeneous_get_reflect},
      {"align", __eolian_elm_toolbar_align_set_reflect, __eolian_elm_toolbar_align_get_reflect},
      {"icon_size", __eolian_elm_toolbar_icon_size_set_reflect, __eolian_elm_toolbar_icon_size_get_reflect},
      {"standard_priority", __eolian_elm_toolbar_standard_priority_set_reflect, __eolian_elm_toolbar_standard_priority_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_toolbar_class_desc = {
   EO_VERSION,
   "Elm.Toolbar",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Toolbar_Data),
   _elm_toolbar_class_initializer,
   _elm_toolbar_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_toolbar_class_get, &_elm_toolbar_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_UI_DIRECTION_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_ACCESS_SELECTION_INTERFACE, EFL_ACCESS_OBJECT_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, ELM_WIDGET_ITEM_CONTAINER_INTERFACE, NULL);

#include "elm_toolbar_eo.legacy.c"
