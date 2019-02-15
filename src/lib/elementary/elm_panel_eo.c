EWAPI const Efl_Event_Description _ELM_PANEL_EVENT_TOGGLED =
   EFL_EVENT_DESCRIPTION("toggled");

void _elm_panel_orient_set(Eo *obj, Elm_Panel_Data *pd, Elm_Panel_Orient orient);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_panel_orient_set, EFL_FUNC_CALL(orient), Elm_Panel_Orient orient);

Elm_Panel_Orient _elm_panel_orient_get(const Eo *obj, Elm_Panel_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_panel_orient_get, Elm_Panel_Orient, 2 /* Elm.Panel.Orient.left */);

void _elm_panel_hidden_set(Eo *obj, Elm_Panel_Data *pd, Eina_Bool hidden);


static Eina_Error
__eolian_elm_panel_hidden_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_panel_hidden_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_panel_hidden_set, EFL_FUNC_CALL(hidden), Eina_Bool hidden);

Eina_Bool _elm_panel_hidden_get(const Eo *obj, Elm_Panel_Data *pd);


static Eina_Value
__eolian_elm_panel_hidden_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_panel_hidden_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_panel_hidden_get, Eina_Bool, 0);

void _elm_panel_scrollable_set(Eo *obj, Elm_Panel_Data *pd, Eina_Bool scrollable);


static Eina_Error
__eolian_elm_panel_scrollable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_panel_scrollable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_panel_scrollable_set, EFL_FUNC_CALL(scrollable), Eina_Bool scrollable);

Eina_Bool _elm_panel_scrollable_get(const Eo *obj, Elm_Panel_Data *pd);


static Eina_Value
__eolian_elm_panel_scrollable_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_panel_scrollable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_panel_scrollable_get, Eina_Bool, 0);

void _elm_panel_scrollable_content_size_set(Eo *obj, Elm_Panel_Data *pd, double ratio);


static Eina_Error
__eolian_elm_panel_scrollable_content_size_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_panel_scrollable_content_size_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_panel_scrollable_content_size_set, EFL_FUNC_CALL(ratio), double ratio);

double _elm_panel_scrollable_content_size_get(const Eo *obj, Elm_Panel_Data *pd);


static Eina_Value
__eolian_elm_panel_scrollable_content_size_get_reflect(const Eo *obj)
{
   double val = elm_obj_panel_scrollable_content_size_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_panel_scrollable_content_size_get, double, 0);

void _elm_panel_toggle(Eo *obj, Elm_Panel_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_panel_toggle);

Efl_Object *_elm_panel_efl_object_constructor(Eo *obj, Elm_Panel_Data *pd);


void _elm_panel_efl_gfx_entity_position_set(Eo *obj, Elm_Panel_Data *pd, Eina_Position2D pos);


void _elm_panel_efl_gfx_entity_size_set(Eo *obj, Elm_Panel_Data *pd, Eina_Size2D size);


void _elm_panel_efl_canvas_group_group_member_add(Eo *obj, Elm_Panel_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Error _elm_panel_efl_ui_widget_theme_apply(Eo *obj, Elm_Panel_Data *pd);


void _elm_panel_efl_ui_widget_disabled_set(Eo *obj, Elm_Panel_Data *pd, Eina_Bool disabled);


void _elm_panel_efl_ui_widget_on_access_update(Eo *obj, Elm_Panel_Data *pd, Eina_Bool enable);


Eina_Bool _elm_panel_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Panel_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Eina_Rect _elm_panel_efl_ui_widget_interest_region_get(const Eo *obj, Elm_Panel_Data *pd);


const Efl_Access_Action_Data *_elm_panel_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Panel_Data *pd);


void _elm_panel_efl_ui_i18n_mirrored_set(Eo *obj, Elm_Panel_Data *pd, Eina_Bool rtl);


Efl_Object *_elm_panel_efl_part_part_get(const Eo *obj, Elm_Panel_Data *pd, const char *name);


static Eina_Bool
_elm_panel_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_PANEL_EXTRA_OPS
#define ELM_PANEL_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_panel_orient_set, _elm_panel_orient_set),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_orient_get, _elm_panel_orient_get),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_hidden_set, _elm_panel_hidden_set),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_hidden_get, _elm_panel_hidden_get),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_scrollable_set, _elm_panel_scrollable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_scrollable_get, _elm_panel_scrollable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_scrollable_content_size_set, _elm_panel_scrollable_content_size_set),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_scrollable_content_size_get, _elm_panel_scrollable_content_size_get),
      EFL_OBJECT_OP_FUNC(elm_obj_panel_toggle, _elm_panel_toggle),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_panel_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_panel_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_panel_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_panel_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_panel_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_disabled_set, _elm_panel_efl_ui_widget_disabled_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_panel_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_panel_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_interest_region_get, _elm_panel_efl_ui_widget_interest_region_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_panel_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_ui_mirrored_set, _elm_panel_efl_ui_i18n_mirrored_set),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_panel_efl_part_part_get),
      ELM_PANEL_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"hidden", __eolian_elm_panel_hidden_set_reflect, __eolian_elm_panel_hidden_get_reflect},
      {"scrollable", __eolian_elm_panel_scrollable_set_reflect, __eolian_elm_panel_scrollable_get_reflect},
      {"scrollable_content_size", __eolian_elm_panel_scrollable_content_size_set_reflect, __eolian_elm_panel_scrollable_content_size_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_panel_class_desc = {
   EO_VERSION,
   "Elm.Panel",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Panel_Data),
   _elm_panel_class_initializer,
   _elm_panel_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_panel_class_get, &_elm_panel_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_LAYER_MIXIN, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_panel_eo.legacy.c"
