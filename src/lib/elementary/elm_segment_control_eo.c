EWAPI const Efl_Event_Description _ELM_SEGMENT_CONTROL_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");

int _elm_segment_control_item_count_get(const Eo *obj, Elm_Segment_Control_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_segment_control_item_count_get, int, 0);

Elm_Widget_Item *_elm_segment_control_item_selected_get(const Eo *obj, Elm_Segment_Control_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_segment_control_item_selected_get, Elm_Widget_Item *, NULL);

const char *_elm_segment_control_item_label_get(const Eo *obj, Elm_Segment_Control_Data *pd, int idx);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_segment_control_item_label_get, const char *, NULL, EFL_FUNC_CALL(idx), int idx);

Elm_Widget_Item *_elm_segment_control_item_insert_at(Eo *obj, Elm_Segment_Control_Data *pd, Efl_Canvas_Object *icon, const char *label, int idx);

EOAPI EFL_FUNC_BODYV(elm_obj_segment_control_item_insert_at, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(icon, label, idx), Efl_Canvas_Object *icon, const char *label, int idx);

Elm_Widget_Item *_elm_segment_control_item_get(const Eo *obj, Elm_Segment_Control_Data *pd, int idx);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_segment_control_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(idx), int idx);

void _elm_segment_control_item_del_at(Eo *obj, Elm_Segment_Control_Data *pd, int idx);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_segment_control_item_del_at, EFL_FUNC_CALL(idx), int idx);

Elm_Widget_Item *_elm_segment_control_item_add(Eo *obj, Elm_Segment_Control_Data *pd, Efl_Canvas_Object *icon, const char *label);

EOAPI EFL_FUNC_BODYV(elm_obj_segment_control_item_add, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(icon, label), Efl_Canvas_Object *icon, const char *label);

Efl_Canvas_Object *_elm_segment_control_item_icon_get(const Eo *obj, Elm_Segment_Control_Data *pd, int idx);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_segment_control_item_icon_get, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(idx), int idx);

Efl_Object *_elm_segment_control_efl_object_constructor(Eo *obj, Elm_Segment_Control_Data *pd);


Eina_Error _elm_segment_control_efl_ui_widget_theme_apply(Eo *obj, Elm_Segment_Control_Data *pd);


void _elm_segment_control_efl_ui_widget_on_access_update(Eo *obj, Elm_Segment_Control_Data *pd, Eina_Bool enable);


void _elm_segment_control_efl_ui_widget_disabled_set(Eo *obj, Elm_Segment_Control_Data *pd, Eina_Bool disabled);


void _elm_segment_control_efl_ui_l10n_translation_update(Eo *obj, Elm_Segment_Control_Data *pd);


static Eina_Bool
_elm_segment_control_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SEGMENT_CONTROL_EXTRA_OPS
#define ELM_SEGMENT_CONTROL_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_count_get, _elm_segment_control_item_count_get),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_selected_get, _elm_segment_control_item_selected_get),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_label_get, _elm_segment_control_item_label_get),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_insert_at, _elm_segment_control_item_insert_at),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_get, _elm_segment_control_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_del_at, _elm_segment_control_item_del_at),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_add, _elm_segment_control_item_add),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_icon_get, _elm_segment_control_item_icon_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_segment_control_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_segment_control_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_segment_control_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_disabled_set, _elm_segment_control_efl_ui_widget_disabled_set),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_segment_control_efl_ui_l10n_translation_update),
      ELM_SEGMENT_CONTROL_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_segment_control_class_desc = {
   EO_VERSION,
   "Elm.Segment_Control",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Segment_Control_Data),
   _elm_segment_control_class_initializer,
   _elm_segment_control_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_segment_control_class_get, &_elm_segment_control_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_segment_control_eo.legacy.c"
