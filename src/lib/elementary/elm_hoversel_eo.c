EWAPI const Efl_Event_Description _ELM_HOVERSEL_EVENT_DISMISSED =
   EFL_EVENT_DESCRIPTION("dismissed");
EWAPI const Efl_Event_Description _ELM_HOVERSEL_EVENT_EXPANDED =
   EFL_EVENT_DESCRIPTION("expanded");
EWAPI const Efl_Event_Description _ELM_HOVERSEL_EVENT_ITEM_FOCUSED =
   EFL_EVENT_DESCRIPTION("item,focused");
EWAPI const Efl_Event_Description _ELM_HOVERSEL_EVENT_ITEM_UNFOCUSED =
   EFL_EVENT_DESCRIPTION("item,unfocused");

void _elm_hoversel_horizontal_set(Eo *obj, Elm_Hoversel_Data *pd, Eina_Bool horizontal);


static Eina_Error
__eolian_elm_hoversel_horizontal_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_hoversel_horizontal_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_hoversel_horizontal_set, EFL_FUNC_CALL(horizontal), Eina_Bool horizontal);

Eina_Bool _elm_hoversel_horizontal_get(const Eo *obj, Elm_Hoversel_Data *pd);


static Eina_Value
__eolian_elm_hoversel_horizontal_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_hoversel_horizontal_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_hoversel_horizontal_get, Eina_Bool, 0);

void _elm_hoversel_hover_parent_set(Eo *obj, Elm_Hoversel_Data *pd, Efl_Canvas_Object *parent);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_hoversel_hover_parent_set, EFL_FUNC_CALL(parent), Efl_Canvas_Object *parent);

Efl_Canvas_Object *_elm_hoversel_hover_parent_get(const Eo *obj, Elm_Hoversel_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_hoversel_hover_parent_get, Efl_Canvas_Object *, NULL);

Eina_Bool _elm_hoversel_expanded_get(const Eo *obj, Elm_Hoversel_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_hoversel_expanded_get, Eina_Bool, 0);

const Eina_List *_elm_hoversel_items_get(const Eo *obj, Elm_Hoversel_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_hoversel_items_get, const Eina_List *, NULL);

void _elm_hoversel_auto_update_set(Eo *obj, Elm_Hoversel_Data *pd, Eina_Bool auto_update);


static Eina_Error
__eolian_elm_hoversel_auto_update_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_hoversel_auto_update_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_hoversel_auto_update_set, EFL_FUNC_CALL(auto_update), Eina_Bool auto_update);

Eina_Bool _elm_hoversel_auto_update_get(const Eo *obj, Elm_Hoversel_Data *pd);


static Eina_Value
__eolian_elm_hoversel_auto_update_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_hoversel_auto_update_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_hoversel_auto_update_get, Eina_Bool, 0);

void _elm_hoversel_hover_begin(Eo *obj, Elm_Hoversel_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_hoversel_hover_begin);

void _elm_hoversel_clear(Eo *obj, Elm_Hoversel_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_hoversel_clear);

void _elm_hoversel_hover_end(Eo *obj, Elm_Hoversel_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_hoversel_hover_end);

Elm_Widget_Item *_elm_hoversel_item_add(Eo *obj, Elm_Hoversel_Data *pd, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_hoversel_item_add, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon_file, icon_type, func, data), const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

Efl_Object *_elm_hoversel_efl_object_constructor(Eo *obj, Elm_Hoversel_Data *pd);


void _elm_hoversel_efl_object_destructor(Eo *obj, Elm_Hoversel_Data *pd);


void _elm_hoversel_efl_gfx_entity_visible_set(Eo *obj, Elm_Hoversel_Data *pd, Eina_Bool v);


Eina_Error _elm_hoversel_efl_ui_widget_theme_apply(Eo *obj, Elm_Hoversel_Data *pd);


void _elm_hoversel_efl_ui_l10n_translation_update(Eo *obj, Elm_Hoversel_Data *pd);


Eina_Bool _elm_hoversel_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Hoversel_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Eina_Bool _elm_hoversel_efl_ui_autorepeat_autorepeat_supported_get(const Eo *obj, Elm_Hoversel_Data *pd);


const Efl_Access_Action_Data *_elm_hoversel_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Hoversel_Data *pd);


Eina_List *_elm_hoversel_efl_access_object_access_children_get(const Eo *obj, Elm_Hoversel_Data *pd);


static Eina_Bool
_elm_hoversel_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_HOVERSEL_EXTRA_OPS
#define ELM_HOVERSEL_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_horizontal_set, _elm_hoversel_horizontal_set),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_horizontal_get, _elm_hoversel_horizontal_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_hover_parent_set, _elm_hoversel_hover_parent_set),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_hover_parent_get, _elm_hoversel_hover_parent_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_expanded_get, _elm_hoversel_expanded_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_items_get, _elm_hoversel_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_auto_update_set, _elm_hoversel_auto_update_set),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_auto_update_get, _elm_hoversel_auto_update_get),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_hover_begin, _elm_hoversel_hover_begin),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_clear, _elm_hoversel_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_hover_end, _elm_hoversel_hover_end),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_item_add, _elm_hoversel_item_add),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_hoversel_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_hoversel_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_hoversel_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_hoversel_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_hoversel_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_hoversel_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_autorepeat_enabled_set, _elm_hoversel_efl_ui_autorepeat_autorepeat_enabled_set),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_hoversel_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_hoversel_efl_access_object_access_children_get),
      ELM_HOVERSEL_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"horizontal", __eolian_elm_hoversel_horizontal_set_reflect, __eolian_elm_hoversel_horizontal_get_reflect},
      {"auto_update", __eolian_elm_hoversel_auto_update_set_reflect, __eolian_elm_hoversel_auto_update_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_hoversel_class_desc = {
   EO_VERSION,
   "Elm.Hoversel",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Hoversel_Data),
   _elm_hoversel_class_initializer,
   _elm_hoversel_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_hoversel_class_get, &_elm_hoversel_class_desc, EFL_UI_BUTTON_LEGACY_CLASS, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_CLICKABLE_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_hoversel_eo.legacy.c"
