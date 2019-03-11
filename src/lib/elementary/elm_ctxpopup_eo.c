EWAPI const Efl_Event_Description _ELM_CTXPOPUP_EVENT_DISMISSED =
   EFL_EVENT_DESCRIPTION("dismissed");
EWAPI const Efl_Event_Description _ELM_CTXPOPUP_EVENT_GEOMETRY_UPDATE =
   EFL_EVENT_DESCRIPTION("geometry,update");

Elm_Widget_Item *_elm_ctxpopup_selected_item_get(const Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_ctxpopup_first_item_get(const Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_first_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_ctxpopup_last_item_get(const Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_last_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_ctxpopup_items_get(const Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_items_get, const Eina_List *, NULL);

void _elm_ctxpopup_horizontal_set(Eo *obj, Elm_Ctxpopup_Data *pd, Eina_Bool horizontal);


static Eina_Error
__eolian_elm_ctxpopup_horizontal_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_ctxpopup_horizontal_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_ctxpopup_horizontal_set, EFL_FUNC_CALL(horizontal), Eina_Bool horizontal);

Eina_Bool _elm_ctxpopup_horizontal_get(const Eo *obj, Elm_Ctxpopup_Data *pd);


static Eina_Value
__eolian_elm_ctxpopup_horizontal_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_ctxpopup_horizontal_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_horizontal_get, Eina_Bool, 0);

void _elm_ctxpopup_auto_hide_disabled_set(Eo *obj, Elm_Ctxpopup_Data *pd, Eina_Bool disabled);


static Eina_Error
__eolian_elm_ctxpopup_auto_hide_disabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_ctxpopup_auto_hide_disabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_ctxpopup_auto_hide_disabled_set, EFL_FUNC_CALL(disabled), Eina_Bool disabled);

Eina_Bool _elm_ctxpopup_auto_hide_disabled_get(const Eo *obj, Elm_Ctxpopup_Data *pd);


static Eina_Value
__eolian_elm_ctxpopup_auto_hide_disabled_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_ctxpopup_auto_hide_disabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_auto_hide_disabled_get, Eina_Bool, 0);

void _elm_ctxpopup_hover_parent_set(Eo *obj, Elm_Ctxpopup_Data *pd, Efl_Canvas_Object *parent);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_ctxpopup_hover_parent_set, EFL_FUNC_CALL(parent), Efl_Canvas_Object *parent);

Efl_Canvas_Object *_elm_ctxpopup_hover_parent_get(const Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_hover_parent_get, Efl_Canvas_Object *, NULL);

void _elm_ctxpopup_direction_priority_set(Eo *obj, Elm_Ctxpopup_Data *pd, Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_ctxpopup_direction_priority_set, EFL_FUNC_CALL(first, second, third, fourth), Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth);

void _elm_ctxpopup_direction_priority_get(const Eo *obj, Elm_Ctxpopup_Data *pd, Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_ctxpopup_direction_priority_get, EFL_FUNC_CALL(first, second, third, fourth), Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth);

Elm_Ctxpopup_Direction _elm_ctxpopup_direction_get(const Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_ctxpopup_direction_get, Elm_Ctxpopup_Direction, 4 /* Elm.Ctxpopup.Direction.unknown */);

void _elm_ctxpopup_dismiss(Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_ctxpopup_dismiss);

void _elm_ctxpopup_clear(Eo *obj, Elm_Ctxpopup_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_ctxpopup_clear);

Elm_Widget_Item *_elm_ctxpopup_item_insert_before(Eo *obj, Elm_Ctxpopup_Data *pd, Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_ctxpopup_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(before, label, icon, func, data), Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_ctxpopup_item_insert_after(Eo *obj, Elm_Ctxpopup_Data *pd, Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_ctxpopup_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(after, label, icon, func, data), Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_ctxpopup_item_append(Eo *obj, Elm_Ctxpopup_Data *pd, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_ctxpopup_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, func, data), const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

Elm_Widget_Item *_elm_ctxpopup_item_prepend(Eo *obj, Elm_Ctxpopup_Data *pd, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_ctxpopup_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, func, data), const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

Efl_Object *_elm_ctxpopup_efl_object_constructor(Eo *obj, Elm_Ctxpopup_Data *pd);


void _elm_ctxpopup_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Ctxpopup_Data *pd, Efl_Ui_Widget *parent);


Eina_Bool _elm_ctxpopup_efl_ui_widget_widget_sub_object_add(Eo *obj, Elm_Ctxpopup_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_ctxpopup_efl_ui_l10n_translation_update(Eo *obj, Elm_Ctxpopup_Data *pd);


Eina_Error _elm_ctxpopup_efl_ui_widget_theme_apply(Eo *obj, Elm_Ctxpopup_Data *pd);


Eina_Bool _elm_ctxpopup_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Ctxpopup_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Elm_Widget_Item *_elm_ctxpopup_elm_widget_item_container_focused_item_get(const Eo *obj, Elm_Ctxpopup_Data *pd);


const Efl_Access_Action_Data *_elm_ctxpopup_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Ctxpopup_Data *pd);


Efl_Access_State_Set _elm_ctxpopup_efl_access_object_state_set_get(const Eo *obj, Elm_Ctxpopup_Data *pd);


Efl_Object *_elm_ctxpopup_efl_part_part_get(const Eo *obj, Elm_Ctxpopup_Data *pd, const char *name);


static Eina_Bool
_elm_ctxpopup_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_CTXPOPUP_EXTRA_OPS
#define ELM_CTXPOPUP_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_selected_item_get, _elm_ctxpopup_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_first_item_get, _elm_ctxpopup_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_last_item_get, _elm_ctxpopup_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_items_get, _elm_ctxpopup_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_horizontal_set, _elm_ctxpopup_horizontal_set),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_horizontal_get, _elm_ctxpopup_horizontal_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_auto_hide_disabled_set, _elm_ctxpopup_auto_hide_disabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_auto_hide_disabled_get, _elm_ctxpopup_auto_hide_disabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_hover_parent_set, _elm_ctxpopup_hover_parent_set),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_hover_parent_get, _elm_ctxpopup_hover_parent_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_direction_priority_set, _elm_ctxpopup_direction_priority_set),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_direction_priority_get, _elm_ctxpopup_direction_priority_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_direction_get, _elm_ctxpopup_direction_get),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_dismiss, _elm_ctxpopup_dismiss),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_clear, _elm_ctxpopup_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_item_insert_before, _elm_ctxpopup_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_item_insert_after, _elm_ctxpopup_item_insert_after),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_item_append, _elm_ctxpopup_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_ctxpopup_item_prepend, _elm_ctxpopup_item_prepend),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_ctxpopup_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_set, _elm_ctxpopup_efl_ui_widget_widget_parent_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_add, _elm_ctxpopup_efl_ui_widget_widget_sub_object_add),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_ctxpopup_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_ctxpopup_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_ctxpopup_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(elm_widget_item_container_focused_item_get, _elm_ctxpopup_elm_widget_item_container_focused_item_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_ctxpopup_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_ctxpopup_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_ctxpopup_efl_part_part_get),
      ELM_CTXPOPUP_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"horizontal", __eolian_elm_ctxpopup_horizontal_set_reflect, __eolian_elm_ctxpopup_horizontal_get_reflect},
      {"auto_hide_disabled", __eolian_elm_ctxpopup_auto_hide_disabled_set_reflect, __eolian_elm_ctxpopup_auto_hide_disabled_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_ctxpopup_class_desc = {
   EO_VERSION,
   "Elm.Ctxpopup",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Ctxpopup_Data),
   _elm_ctxpopup_class_initializer,
   _elm_ctxpopup_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_ctxpopup_class_get, &_elm_ctxpopup_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_LAYER_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, ELM_WIDGET_ITEM_CONTAINER_INTERFACE, NULL);

#include "elm_ctxpopup_eo.legacy.c"
