EWAPI const Efl_Event_Description _ELM_HOVER_EVENT_SMART_CHANGED =
   EFL_EVENT_DESCRIPTION("smart,changed");
EWAPI const Efl_Event_Description _ELM_HOVER_EVENT_DISMISSED =
   EFL_EVENT_DESCRIPTION("dismissed");

void _elm_hover_target_set(Eo *obj, Elm_Hover_Data *pd, Efl_Canvas_Object *target);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_hover_target_set, EFL_FUNC_CALL(target), Efl_Canvas_Object *target);

Efl_Canvas_Object *_elm_hover_target_get(const Eo *obj, Elm_Hover_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_hover_target_get, Efl_Canvas_Object *, NULL);

const char *_elm_hover_best_content_location_get(const Eo *obj, Elm_Hover_Data *pd, Elm_Hover_Axis pref_axis);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_hover_best_content_location_get, const char *, NULL, EFL_FUNC_CALL(pref_axis), Elm_Hover_Axis pref_axis);

void _elm_hover_dismiss(Eo *obj, Elm_Hover_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_hover_dismiss);

Efl_Object *_elm_hover_efl_object_constructor(Eo *obj, Elm_Hover_Data *pd);


void _elm_hover_efl_gfx_entity_visible_set(Eo *obj, Elm_Hover_Data *pd, Eina_Bool v);


void _elm_hover_efl_gfx_entity_position_set(Eo *obj, Elm_Hover_Data *pd, Eina_Position2D pos);


void _elm_hover_efl_gfx_entity_size_set(Eo *obj, Elm_Hover_Data *pd, Eina_Size2D size);


Eina_Error _elm_hover_efl_ui_widget_theme_apply(Eo *obj, Elm_Hover_Data *pd);


Eina_Bool _elm_hover_efl_ui_widget_widget_sub_object_add(Eo *obj, Elm_Hover_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_hover_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Hover_Data *pd, Efl_Ui_Widget *parent);


Efl_Ui_Widget *_elm_hover_efl_ui_widget_widget_parent_get(const Eo *obj, Elm_Hover_Data *pd);


Eina_Bool _elm_hover_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Hover_Data *pd, Efl_Canvas_Object *sub_obj);


const Efl_Access_Action_Data *_elm_hover_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Hover_Data *pd);


Efl_Access_State_Set _elm_hover_efl_access_object_state_set_get(const Eo *obj, Elm_Hover_Data *pd);


Efl_Object *_elm_hover_efl_part_part_get(const Eo *obj, Elm_Hover_Data *pd, const char *name);


static Eina_Bool
_elm_hover_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_HOVER_EXTRA_OPS
#define ELM_HOVER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_hover_target_set, _elm_hover_target_set),
      EFL_OBJECT_OP_FUNC(elm_obj_hover_target_get, _elm_hover_target_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hover_best_content_location_get, _elm_hover_best_content_location_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hover_dismiss, _elm_hover_dismiss),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_hover_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_hover_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_hover_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_hover_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_hover_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_add, _elm_hover_efl_ui_widget_widget_sub_object_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_set, _elm_hover_efl_ui_widget_widget_parent_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_get, _elm_hover_efl_ui_widget_widget_parent_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_hover_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_hover_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_hover_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_hover_efl_part_part_get),
      ELM_HOVER_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_hover_class_desc = {
   EO_VERSION,
   "Elm.Hover",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Hover_Data),
   _elm_hover_class_initializer,
   _elm_hover_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_hover_class_get, &_elm_hover_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_LAYER_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_hover_eo.legacy.c"
