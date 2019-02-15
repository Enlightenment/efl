
void _elm_list_item_separator_set(Eo *obj, Elm_List_Item_Data *pd, Eina_Bool setting);


static Eina_Error
__eolian_elm_list_item_separator_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_list_item_separator_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_item_separator_set, EFL_FUNC_CALL(setting), Eina_Bool setting);

Eina_Bool _elm_list_item_separator_get(const Eo *obj, Elm_List_Item_Data *pd);


static Eina_Value
__eolian_elm_list_item_separator_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_list_item_separator_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_item_separator_get, Eina_Bool, 0);

void _elm_list_item_selected_set(Eo *obj, Elm_List_Item_Data *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_list_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_list_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_list_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Eina_Bool _elm_list_item_selected_get(const Eo *obj, Elm_List_Item_Data *pd);


static Eina_Value
__eolian_elm_list_item_selected_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_list_item_selected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_item_selected_get, Eina_Bool, 0);

Efl_Canvas_Object *_elm_list_item_object_get(const Eo *obj, Elm_List_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_item_object_get, Efl_Canvas_Object *, NULL);

Elm_Widget_Item *_elm_list_item_prev_get(const Eo *obj, Elm_List_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_item_prev_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_list_item_next_get(const Eo *obj, Elm_List_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_list_item_next_get, Elm_Widget_Item *, NULL);

void _elm_list_item_show(Eo *obj, Elm_List_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_list_item_show);

void _elm_list_item_bring_in(Eo *obj, Elm_List_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_list_item_bring_in);

Efl_Object *_elm_list_item_efl_object_constructor(Eo *obj, Elm_List_Item_Data *pd);


void _elm_list_item_efl_object_invalidate(Eo *obj, Elm_List_Item_Data *pd);


void _elm_list_item_efl_object_destructor(Eo *obj, Elm_List_Item_Data *pd);


void _elm_list_item_elm_widget_item_disable(Eo *obj, Elm_List_Item_Data *pd);


void _elm_list_item_elm_widget_item_del_pre(Eo *obj, Elm_List_Item_Data *pd);


void _elm_list_item_elm_widget_item_signal_emit(Eo *obj, Elm_List_Item_Data *pd, const char *emission, const char *source);


void _elm_list_item_elm_widget_item_item_focus_set(Eo *obj, Elm_List_Item_Data *pd, Eina_Bool focused);


Eina_Bool _elm_list_item_elm_widget_item_item_focus_get(const Eo *obj, Elm_List_Item_Data *pd);


void _elm_list_item_elm_widget_item_part_text_set(Eo *obj, Elm_List_Item_Data *pd, const char *part, const char *label);


const char *_elm_list_item_elm_widget_item_part_text_get(const Eo *obj, Elm_List_Item_Data *pd, const char *part);


void _elm_list_item_elm_widget_item_part_content_set(Eo *obj, Elm_List_Item_Data *pd, const char *part, Efl_Canvas_Object *content);


Efl_Canvas_Object *_elm_list_item_elm_widget_item_part_content_get(const Eo *obj, Elm_List_Item_Data *pd, const char *part);


Efl_Canvas_Object *_elm_list_item_elm_widget_item_part_content_unset(Eo *obj, Elm_List_Item_Data *pd, const char *part);


const char *_elm_list_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_List_Item_Data *pd);


Efl_Access_State_Set _elm_list_item_efl_access_object_state_set_get(const Eo *obj, Elm_List_Item_Data *pd);


static Eina_Bool
_elm_list_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_LIST_ITEM_EXTRA_OPS
#define ELM_LIST_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_separator_set, _elm_list_item_separator_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_separator_get, _elm_list_item_separator_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_selected_set, _elm_list_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_selected_get, _elm_list_item_selected_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_object_get, _elm_list_item_object_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_prev_get, _elm_list_item_prev_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_next_get, _elm_list_item_next_get),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_show, _elm_list_item_show),
      EFL_OBJECT_OP_FUNC(elm_obj_list_item_bring_in, _elm_list_item_bring_in),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_list_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_invalidate, _elm_list_item_efl_object_invalidate),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_list_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_list_item_elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_del_pre, _elm_list_item_elm_widget_item_del_pre),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_list_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_set, _elm_list_item_elm_widget_item_item_focus_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_get, _elm_list_item_elm_widget_item_item_focus_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_list_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_list_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_list_item_elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_list_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_unset, _elm_list_item_elm_widget_item_part_content_unset),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_list_item_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_list_item_efl_access_object_state_set_get),
      ELM_LIST_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"separator", __eolian_elm_list_item_separator_set_reflect, __eolian_elm_list_item_separator_get_reflect},
      {"selected", __eolian_elm_list_item_selected_set_reflect, __eolian_elm_list_item_selected_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_list_item_class_desc = {
   EO_VERSION,
   "Elm.List.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_List_Item_Data),
   _elm_list_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_list_item_class_get, &_elm_list_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_list_item_eo.legacy.c"
