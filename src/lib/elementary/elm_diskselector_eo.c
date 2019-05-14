
void _elm_diskselector_side_text_max_length_set(Eo *obj, Elm_Diskselector_Data *pd, int len);


static Eina_Error
__eolian_elm_diskselector_side_text_max_length_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_diskselector_side_text_max_length_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_diskselector_side_text_max_length_set, EFL_FUNC_CALL(len), int len);

int _elm_diskselector_side_text_max_length_get(const Eo *obj, Elm_Diskselector_Data *pd);


static Eina_Value
__eolian_elm_diskselector_side_text_max_length_get_reflect(const Eo *obj)
{
   int val = elm_obj_diskselector_side_text_max_length_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_side_text_max_length_get, int, 0);

void _elm_diskselector_round_enabled_set(Eo *obj, Elm_Diskselector_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_diskselector_round_enabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_diskselector_round_enabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_diskselector_round_enabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

Eina_Bool _elm_diskselector_round_enabled_get(const Eo *obj, Elm_Diskselector_Data *pd);


static Eina_Value
__eolian_elm_diskselector_round_enabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_diskselector_round_enabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_round_enabled_get, Eina_Bool, 0);

void _elm_diskselector_display_item_num_set(Eo *obj, Elm_Diskselector_Data *pd, int num);


static Eina_Error
__eolian_elm_diskselector_display_item_num_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_diskselector_display_item_num_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_diskselector_display_item_num_set, EFL_FUNC_CALL(num), int num);

int _elm_diskselector_display_item_num_get(const Eo *obj, Elm_Diskselector_Data *pd);


static Eina_Value
__eolian_elm_diskselector_display_item_num_get_reflect(const Eo *obj)
{
   int val = elm_obj_diskselector_display_item_num_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_display_item_num_get, int, 0);

Elm_Widget_Item *_elm_diskselector_first_item_get(const Eo *obj, Elm_Diskselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_first_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_diskselector_items_get(const Eo *obj, Elm_Diskselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_diskselector_last_item_get(const Eo *obj, Elm_Diskselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_last_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_diskselector_selected_item_get(const Eo *obj, Elm_Diskselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_diskselector_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_diskselector_item_append(Eo *obj, Elm_Diskselector_Data *pd, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_diskselector_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, func, data), const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

void _elm_diskselector_clear(Eo *obj, Elm_Diskselector_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_diskselector_clear);

Efl_Object *_elm_diskselector_efl_object_constructor(Eo *obj, Elm_Diskselector_Data *pd);


void _elm_diskselector_efl_gfx_entity_position_set(Eo *obj, Elm_Diskselector_Data *pd, Eina_Position2D pos);


void _elm_diskselector_efl_gfx_entity_size_set(Eo *obj, Elm_Diskselector_Data *pd, Eina_Size2D size);


void _elm_diskselector_efl_canvas_group_group_member_add(Eo *obj, Elm_Diskselector_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_diskselector_efl_ui_widget_on_access_update(Eo *obj, Elm_Diskselector_Data *pd, Eina_Bool enable);


Eina_Error _elm_diskselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Diskselector_Data *pd);


Eina_Bool _elm_diskselector_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Diskselector_Data *pd);


void _elm_diskselector_efl_ui_l10n_translation_update(Eo *obj, Elm_Diskselector_Data *pd);


Eina_Bool _elm_diskselector_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Diskselector_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_diskselector_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Diskselector_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


void _elm_diskselector_elm_interface_scrollable_policy_set(Eo *obj, Elm_Diskselector_Data *pd, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar);


void _elm_diskselector_elm_interface_scrollable_policy_get(const Eo *obj, Elm_Diskselector_Data *pd, Elm_Scroller_Policy *hbar, Elm_Scroller_Policy *vbar);


const Efl_Access_Action_Data *_elm_diskselector_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Diskselector_Data *pd);


static Eina_Bool
_elm_diskselector_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_DISKSELECTOR_EXTRA_OPS
#define ELM_DISKSELECTOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_side_text_max_length_set, _elm_diskselector_side_text_max_length_set),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_side_text_max_length_get, _elm_diskselector_side_text_max_length_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_round_enabled_set, _elm_diskselector_round_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_round_enabled_get, _elm_diskselector_round_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_display_item_num_set, _elm_diskselector_display_item_num_set),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_display_item_num_get, _elm_diskselector_display_item_num_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_first_item_get, _elm_diskselector_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_items_get, _elm_diskselector_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_last_item_get, _elm_diskselector_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_selected_item_get, _elm_diskselector_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_item_append, _elm_diskselector_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_diskselector_clear, _elm_diskselector_clear),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_diskselector_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_diskselector_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_diskselector_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_diskselector_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_diskselector_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_diskselector_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_diskselector_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_diskselector_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_diskselector_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_diskselector_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_set, _elm_diskselector_elm_interface_scrollable_policy_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_get, _elm_diskselector_elm_interface_scrollable_policy_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_diskselector_efl_access_widget_action_elm_actions_get),
      ELM_DISKSELECTOR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"side_text_max_length", __eolian_elm_diskselector_side_text_max_length_set_reflect, __eolian_elm_diskselector_side_text_max_length_get_reflect},
      {"round_enabled", __eolian_elm_diskselector_round_enabled_set_reflect, __eolian_elm_diskselector_round_enabled_get_reflect},
      {"display_item_num", __eolian_elm_diskselector_display_item_num_set_reflect, __eolian_elm_diskselector_display_item_num_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_diskselector_class_desc = {
   EO_VERSION,
   "Elm.Diskselector",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Diskselector_Data),
   _elm_diskselector_class_initializer,
   _elm_diskselector_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_diskselector_class_get, &_elm_diskselector_class_desc, EFL_UI_WIDGET_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_CLICKABLE_MIXIN, EFL_UI_SCROLLABLE_INTERFACE, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_diskselector_eo.legacy.c"
