EWAPI const Efl_Event_Description _ELM_POPUP_EVENT_BLOCK_CLICKED =
   EFL_EVENT_DESCRIPTION("block,clicked");
EWAPI const Efl_Event_Description _ELM_POPUP_EVENT_TIMEOUT =
   EFL_EVENT_DESCRIPTION("timeout");
EWAPI const Efl_Event_Description _ELM_POPUP_EVENT_ITEM_FOCUSED =
   EFL_EVENT_DESCRIPTION("item,focused");
EWAPI const Efl_Event_Description _ELM_POPUP_EVENT_ITEM_UNFOCUSED =
   EFL_EVENT_DESCRIPTION("item,unfocused");
EWAPI const Efl_Event_Description _ELM_POPUP_EVENT_DISMISSED =
   EFL_EVENT_DESCRIPTION("dismissed");

void _elm_popup_align_set(Eo *obj, Elm_Popup_Data *pd, double horizontal, double vertical);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_popup_align_set, EFL_FUNC_CALL(horizontal, vertical), double horizontal, double vertical);

void _elm_popup_align_get(const Eo *obj, Elm_Popup_Data *pd, double *horizontal, double *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_popup_align_get, EFL_FUNC_CALL(horizontal, vertical), double *horizontal, double *vertical);

void _elm_popup_allow_events_set(Eo *obj, Elm_Popup_Data *pd, Eina_Bool allow);


static Eina_Error
__eolian_elm_popup_allow_events_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_popup_allow_events_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_popup_allow_events_set, EFL_FUNC_CALL(allow), Eina_Bool allow);

Eina_Bool _elm_popup_allow_events_get(const Eo *obj, Elm_Popup_Data *pd);


static Eina_Value
__eolian_elm_popup_allow_events_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_popup_allow_events_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_popup_allow_events_get, Eina_Bool, 0);

void _elm_popup_content_text_wrap_type_set(Eo *obj, Elm_Popup_Data *pd, Elm_Wrap_Type wrap);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_popup_content_text_wrap_type_set, EFL_FUNC_CALL(wrap), Elm_Wrap_Type wrap);

Elm_Wrap_Type _elm_popup_content_text_wrap_type_get(const Eo *obj, Elm_Popup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_popup_content_text_wrap_type_get, Elm_Wrap_Type, 0);

void _elm_popup_orient_set(Eo *obj, Elm_Popup_Data *pd, Elm_Popup_Orient orient);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_popup_orient_set, EFL_FUNC_CALL(orient), Elm_Popup_Orient orient);

Elm_Popup_Orient _elm_popup_orient_get(const Eo *obj, Elm_Popup_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_popup_orient_get, Elm_Popup_Orient, 0);

void _elm_popup_timeout_set(Eo *obj, Elm_Popup_Data *pd, double timeout);


static Eina_Error
__eolian_elm_popup_timeout_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_popup_timeout_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_popup_timeout_set, EFL_FUNC_CALL(timeout), double timeout);

double _elm_popup_timeout_get(const Eo *obj, Elm_Popup_Data *pd);


static Eina_Value
__eolian_elm_popup_timeout_get_reflect(const Eo *obj)
{
   double val = elm_obj_popup_timeout_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_popup_timeout_get, double, 0);

void _elm_popup_scrollable_set(Eo *obj, Elm_Popup_Data *pd, Eina_Bool scroll);


static Eina_Error
__eolian_elm_popup_scrollable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_popup_scrollable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_popup_scrollable_set, EFL_FUNC_CALL(scroll), Eina_Bool scroll);

Eina_Bool _elm_popup_scrollable_get(const Eo *obj, Elm_Popup_Data *pd);


static Eina_Value
__eolian_elm_popup_scrollable_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_popup_scrollable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_popup_scrollable_get, Eina_Bool, 0);

Elm_Widget_Item *_elm_popup_item_append(Eo *obj, Elm_Popup_Data *pd, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_popup_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, icon, func, data), const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

void _elm_popup_dismiss(Eo *obj, Elm_Popup_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_popup_dismiss);

Efl_Object *_elm_popup_efl_object_constructor(Eo *obj, Elm_Popup_Data *pd);


Eina_Error _elm_popup_efl_ui_widget_theme_apply(Eo *obj, Elm_Popup_Data *pd);


void _elm_popup_efl_ui_widget_on_access_update(Eo *obj, Elm_Popup_Data *pd, Eina_Bool enable);


void _elm_popup_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Popup_Data *pd, Efl_Ui_Widget *parent);


void _elm_popup_efl_ui_l10n_translation_update(Eo *obj, Elm_Popup_Data *pd);


Eina_Bool _elm_popup_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Popup_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_popup_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Popup_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


void _elm_popup_efl_layout_signal_signal_emit(Eo *obj, Elm_Popup_Data *pd, const char *emission, const char *source);


const Efl_Access_Action_Data *_elm_popup_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Popup_Data *pd);


Efl_Access_State_Set _elm_popup_efl_access_object_state_set_get(const Eo *obj, Elm_Popup_Data *pd);


const char *_elm_popup_efl_access_object_i18n_name_get(const Eo *obj, Elm_Popup_Data *pd);


Efl_Object *_elm_popup_efl_part_part_get(const Eo *obj, Elm_Popup_Data *pd, const char *name);


static Eina_Bool
_elm_popup_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_POPUP_EXTRA_OPS
#define ELM_POPUP_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_popup_align_set, _elm_popup_align_set),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_align_get, _elm_popup_align_get),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_allow_events_set, _elm_popup_allow_events_set),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_allow_events_get, _elm_popup_allow_events_get),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_content_text_wrap_type_set, _elm_popup_content_text_wrap_type_set),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_content_text_wrap_type_get, _elm_popup_content_text_wrap_type_get),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_orient_set, _elm_popup_orient_set),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_orient_get, _elm_popup_orient_get),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_timeout_set, _elm_popup_timeout_set),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_timeout_get, _elm_popup_timeout_get),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_scrollable_set, _elm_popup_scrollable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_scrollable_get, _elm_popup_scrollable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_item_append, _elm_popup_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_popup_dismiss, _elm_popup_dismiss),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_popup_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_popup_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_popup_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_set, _elm_popup_efl_ui_widget_widget_parent_set),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_popup_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_popup_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_popup_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_layout_signal_emit, _elm_popup_efl_layout_signal_signal_emit),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_popup_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_popup_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_popup_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_popup_efl_part_part_get),
      ELM_POPUP_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"allow_events", __eolian_elm_popup_allow_events_set_reflect, __eolian_elm_popup_allow_events_get_reflect},
      {"timeout", __eolian_elm_popup_timeout_set_reflect, __eolian_elm_popup_timeout_get_reflect},
      {"scrollable", __eolian_elm_popup_scrollable_set_reflect, __eolian_elm_popup_scrollable_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_popup_class_desc = {
   EO_VERSION,
   "Elm.Popup",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Popup_Data),
   _elm_popup_class_initializer,
   _elm_popup_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_popup_class_get, &_elm_popup_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_popup_eo.legacy.c"
