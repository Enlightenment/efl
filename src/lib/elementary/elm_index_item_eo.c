
void _elm_index_item_selected_set(Eo *obj, Elm_Index_Item_Data *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_index_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_index_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_index_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

void _elm_index_item_priority_set(Eo *obj, Elm_Index_Item_Data *pd, int priority);


static Eina_Error
__eolian_elm_index_item_priority_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_index_item_priority_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_index_item_priority_set, EFL_FUNC_CALL(priority), int priority);

const char *_elm_index_item_letter_get(const Eo *obj, Elm_Index_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_index_item_letter_get, const char *, NULL);

Efl_Object *_elm_index_item_efl_object_constructor(Eo *obj, Elm_Index_Item_Data *pd);


void _elm_index_item_efl_object_destructor(Eo *obj, Elm_Index_Item_Data *pd);


Efl_Canvas_Object *_elm_index_item_elm_widget_item_access_register(Eo *obj, Elm_Index_Item_Data *pd);


const char *_elm_index_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_Index_Item_Data *pd);


const Efl_Access_Action_Data *_elm_index_item_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Index_Item_Data *pd);


static Eina_Bool
_elm_index_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_INDEX_ITEM_EXTRA_OPS
#define ELM_INDEX_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_index_item_selected_set, _elm_index_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_index_item_priority_set, _elm_index_item_priority_set),
      EFL_OBJECT_OP_FUNC(elm_obj_index_item_letter_get, _elm_index_item_letter_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_index_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_index_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_register, _elm_index_item_elm_widget_item_access_register),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_index_item_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_index_item_efl_access_widget_action_elm_actions_get),
      ELM_INDEX_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"selected", __eolian_elm_index_item_selected_set_reflect, NULL},
      {"priority", __eolian_elm_index_item_priority_set_reflect, NULL},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_index_item_class_desc = {
   EO_VERSION,
   "Elm.Index.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Index_Item_Data),
   _elm_index_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_index_item_class_get, &_elm_index_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_index_item_eo.legacy.c"
