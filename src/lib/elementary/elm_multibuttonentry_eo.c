EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_SELECTED =
   EFL_EVENT_DESCRIPTION("item,selected");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_ADDED =
   EFL_EVENT_DESCRIPTION("item,added");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_DELETED =
   EFL_EVENT_DESCRIPTION("item,deleted");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_CLICKED =
   EFL_EVENT_DESCRIPTION("item,clicked");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_LONGPRESSED =
   EFL_EVENT_DESCRIPTION("item,longpressed");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_EXPANDED =
   EFL_EVENT_DESCRIPTION("expanded");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_CONTRACTED =
   EFL_EVENT_DESCRIPTION("contracted");
EWAPI const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_EXPAND_STATE_CHANGED =
   EFL_EVENT_DESCRIPTION("expand,state,changed");

void _elm_multibuttonentry_editable_set(Eo *obj, Elm_Multibuttonentry_Data *pd, Eina_Bool editable);


static Eina_Error
__eolian_elm_multibuttonentry_editable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_multibuttonentry_editable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_multibuttonentry_editable_set, EFL_FUNC_CALL(editable), Eina_Bool editable);

Eina_Bool _elm_multibuttonentry_editable_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);


static Eina_Value
__eolian_elm_multibuttonentry_editable_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_multibuttonentry_editable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_editable_get, Eina_Bool, 0);

void _elm_multibuttonentry_expanded_set(Eo *obj, Elm_Multibuttonentry_Data *pd, Eina_Bool expanded);


static Eina_Error
__eolian_elm_multibuttonentry_expanded_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_multibuttonentry_expanded_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_multibuttonentry_expanded_set, EFL_FUNC_CALL(expanded), Eina_Bool expanded);

Eina_Bool _elm_multibuttonentry_expanded_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);


static Eina_Value
__eolian_elm_multibuttonentry_expanded_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_multibuttonentry_expanded_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_expanded_get, Eina_Bool, 0);

void _elm_multibuttonentry_format_function_set(Eo *obj, Elm_Multibuttonentry_Data *pd, Elm_Multibuttonentry_Format_Cb format_function, const void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_multibuttonentry_format_function_set, EFL_FUNC_CALL(format_function, data), Elm_Multibuttonentry_Format_Cb format_function, const void *data);

const Eina_List *_elm_multibuttonentry_items_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_multibuttonentry_first_item_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_first_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_multibuttonentry_last_item_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_last_item_get, Elm_Widget_Item *, NULL);

Efl_Canvas_Object *_elm_multibuttonentry_entry_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_entry_get, Efl_Canvas_Object *, NULL);

Elm_Widget_Item *_elm_multibuttonentry_selected_item_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_multibuttonentry_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_multibuttonentry_item_prepend(Eo *obj, Elm_Multibuttonentry_Data *pd, const char *label, Evas_Smart_Cb func, void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_multibuttonentry_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, func, data), const char *label, Evas_Smart_Cb func, void *data);

void _elm_multibuttonentry_clear(Eo *obj, Elm_Multibuttonentry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_multibuttonentry_clear);

void _elm_multibuttonentry_item_filter_remove(Eo *obj, Elm_Multibuttonentry_Data *pd, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_multibuttonentry_item_filter_remove, EFL_FUNC_CALL(func, data), Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

Elm_Widget_Item *_elm_multibuttonentry_item_insert_before(Eo *obj, Elm_Multibuttonentry_Data *pd, Elm_Widget_Item *before, const char *label, Evas_Smart_Cb func, void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_multibuttonentry_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(before, label, func, data), Elm_Widget_Item *before, const char *label, Evas_Smart_Cb func, void *data);

Elm_Widget_Item *_elm_multibuttonentry_item_append(Eo *obj, Elm_Multibuttonentry_Data *pd, const char *label, Evas_Smart_Cb func, void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_multibuttonentry_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, func, data), const char *label, Evas_Smart_Cb func, void *data);

void _elm_multibuttonentry_item_filter_prepend(Eo *obj, Elm_Multibuttonentry_Data *pd, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_multibuttonentry_item_filter_prepend, EFL_FUNC_CALL(func, data), Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

void _elm_multibuttonentry_item_filter_append(Eo *obj, Elm_Multibuttonentry_Data *pd, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_multibuttonentry_item_filter_append, EFL_FUNC_CALL(func, data), Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

Elm_Widget_Item *_elm_multibuttonentry_item_insert_after(Eo *obj, Elm_Multibuttonentry_Data *pd, Elm_Widget_Item *after, const char *label, Evas_Smart_Cb func, void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_multibuttonentry_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(after, label, func, data), Elm_Widget_Item *after, const char *label, Evas_Smart_Cb func, void *data);

Efl_Object *_elm_multibuttonentry_efl_object_constructor(Eo *obj, Elm_Multibuttonentry_Data *pd);


Eina_Error _elm_multibuttonentry_efl_ui_widget_theme_apply(Eo *obj, Elm_Multibuttonentry_Data *pd);


void _elm_multibuttonentry_efl_ui_widget_on_access_update(Eo *obj, Elm_Multibuttonentry_Data *pd, Eina_Bool enable);


void _elm_multibuttonentry_efl_ui_l10n_translation_update(Eo *obj, Elm_Multibuttonentry_Data *pd);


Eina_Bool _elm_multibuttonentry_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Multibuttonentry_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Eina_List *_elm_multibuttonentry_efl_access_object_access_children_get(const Eo *obj, Elm_Multibuttonentry_Data *pd);


Efl_Object *_elm_multibuttonentry_efl_part_part_get(const Eo *obj, Elm_Multibuttonentry_Data *pd, const char *name);


static Eina_Bool
_elm_multibuttonentry_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_MULTIBUTTONENTRY_EXTRA_OPS
#define ELM_MULTIBUTTONENTRY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_editable_set, _elm_multibuttonentry_editable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_editable_get, _elm_multibuttonentry_editable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_expanded_set, _elm_multibuttonentry_expanded_set),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_expanded_get, _elm_multibuttonentry_expanded_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_format_function_set, _elm_multibuttonentry_format_function_set),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_items_get, _elm_multibuttonentry_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_first_item_get, _elm_multibuttonentry_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_last_item_get, _elm_multibuttonentry_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_entry_get, _elm_multibuttonentry_entry_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_selected_item_get, _elm_multibuttonentry_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_prepend, _elm_multibuttonentry_item_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_clear, _elm_multibuttonentry_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_filter_remove, _elm_multibuttonentry_item_filter_remove),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_insert_before, _elm_multibuttonentry_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_append, _elm_multibuttonentry_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_filter_prepend, _elm_multibuttonentry_item_filter_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_filter_append, _elm_multibuttonentry_item_filter_append),
      EFL_OBJECT_OP_FUNC(elm_obj_multibuttonentry_item_insert_after, _elm_multibuttonentry_item_insert_after),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_multibuttonentry_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_multibuttonentry_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_multibuttonentry_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_multibuttonentry_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_multibuttonentry_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_multibuttonentry_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_multibuttonentry_efl_part_part_get),
      ELM_MULTIBUTTONENTRY_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"editable", __eolian_elm_multibuttonentry_editable_set_reflect, __eolian_elm_multibuttonentry_editable_get_reflect},
      {"expanded", __eolian_elm_multibuttonentry_expanded_set_reflect, __eolian_elm_multibuttonentry_expanded_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_multibuttonentry_class_desc = {
   EO_VERSION,
   "Elm.Multibuttonentry",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Multibuttonentry_Data),
   _elm_multibuttonentry_class_initializer,
   _elm_multibuttonentry_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_multibuttonentry_class_get, &_elm_multibuttonentry_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_multibuttonentry_eo.legacy.c"
