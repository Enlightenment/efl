EWAPI const Efl_Event_Description _ELM_ACTIONSLIDER_EVENT_POS_CHANGED =
   EFL_EVENT_DESCRIPTION("pos_changed");

void _elm_actionslider_indicator_pos_set(Eo *obj, Elm_Actionslider_Data *pd, Elm_Actionslider_Pos pos);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_actionslider_indicator_pos_set, EFL_FUNC_CALL(pos), Elm_Actionslider_Pos pos);

Elm_Actionslider_Pos _elm_actionslider_indicator_pos_get(const Eo *obj, Elm_Actionslider_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_actionslider_indicator_pos_get, Elm_Actionslider_Pos, 0);

void _elm_actionslider_magnet_pos_set(Eo *obj, Elm_Actionslider_Data *pd, Elm_Actionslider_Pos pos);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_actionslider_magnet_pos_set, EFL_FUNC_CALL(pos), Elm_Actionslider_Pos pos);

Elm_Actionslider_Pos _elm_actionslider_magnet_pos_get(const Eo *obj, Elm_Actionslider_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_actionslider_magnet_pos_get, Elm_Actionslider_Pos, 0);

void _elm_actionslider_enabled_pos_set(Eo *obj, Elm_Actionslider_Data *pd, Elm_Actionslider_Pos pos);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_actionslider_enabled_pos_set, EFL_FUNC_CALL(pos), Elm_Actionslider_Pos pos);

Elm_Actionslider_Pos _elm_actionslider_enabled_pos_get(const Eo *obj, Elm_Actionslider_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_actionslider_enabled_pos_get, Elm_Actionslider_Pos, 0);

const char *_elm_actionslider_selected_label_get(const Eo *obj, Elm_Actionslider_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_actionslider_selected_label_get, const char *, NULL);

Efl_Object *_elm_actionslider_efl_object_constructor(Eo *obj, Elm_Actionslider_Data *pd);


Eina_Error _elm_actionslider_efl_ui_widget_theme_apply(Eo *obj, Elm_Actionslider_Data *pd);


Efl_Object *_elm_actionslider_efl_part_part_get(const Eo *obj, Elm_Actionslider_Data *pd, const char *name);


static Eina_Bool
_elm_actionslider_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ACTIONSLIDER_EXTRA_OPS
#define ELM_ACTIONSLIDER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_indicator_pos_set, _elm_actionslider_indicator_pos_set),
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_indicator_pos_get, _elm_actionslider_indicator_pos_get),
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_magnet_pos_set, _elm_actionslider_magnet_pos_set),
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_magnet_pos_get, _elm_actionslider_magnet_pos_get),
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_enabled_pos_set, _elm_actionslider_enabled_pos_set),
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_enabled_pos_get, _elm_actionslider_enabled_pos_get),
      EFL_OBJECT_OP_FUNC(elm_obj_actionslider_selected_label_get, _elm_actionslider_selected_label_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_actionslider_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_actionslider_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_actionslider_efl_part_part_get),
      ELM_ACTIONSLIDER_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_actionslider_class_desc = {
   EO_VERSION,
   "Elm.Actionslider",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Actionslider_Data),
   _elm_actionslider_class_initializer,
   _elm_actionslider_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_actionslider_class_get, &_elm_actionslider_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_actionslider_eo.legacy.c"
