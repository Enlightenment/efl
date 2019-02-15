
void _elm_flipselector_item_selected_set(Eo *obj, Elm_Flipselector_Item_Data *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_flipselector_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_flipselector_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_flipselector_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Eina_Bool _elm_flipselector_item_selected_get(const Eo *obj, Elm_Flipselector_Item_Data *pd);


static Eina_Value
__eolian_elm_flipselector_item_selected_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_flipselector_item_selected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_item_selected_get, Eina_Bool, 0);

Elm_Widget_Item *_elm_flipselector_item_prev_get(const Eo *obj, Elm_Flipselector_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_item_prev_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_flipselector_item_next_get(const Eo *obj, Elm_Flipselector_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_item_next_get, Elm_Widget_Item *, NULL);

Efl_Object *_elm_flipselector_item_efl_object_constructor(Eo *obj, Elm_Flipselector_Item_Data *pd);


void _elm_flipselector_item_efl_object_destructor(Eo *obj, Elm_Flipselector_Item_Data *pd);


void _elm_flipselector_item_elm_widget_item_signal_emit(Eo *obj, Elm_Flipselector_Item_Data *pd, const char *emission, const char *source);


void _elm_flipselector_item_elm_widget_item_part_text_set(Eo *obj, Elm_Flipselector_Item_Data *pd, const char *part, const char *label);


const char *_elm_flipselector_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Flipselector_Item_Data *pd, const char *part);


static Eina_Bool
_elm_flipselector_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_FLIPSELECTOR_ITEM_EXTRA_OPS
#define ELM_FLIPSELECTOR_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_item_selected_set, _elm_flipselector_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_item_selected_get, _elm_flipselector_item_selected_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_item_prev_get, _elm_flipselector_item_prev_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_item_next_get, _elm_flipselector_item_next_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_flipselector_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_flipselector_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_flipselector_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_flipselector_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_flipselector_item_elm_widget_item_part_text_get),
      ELM_FLIPSELECTOR_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"selected", __eolian_elm_flipselector_item_selected_set_reflect, __eolian_elm_flipselector_item_selected_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_flipselector_item_class_desc = {
   EO_VERSION,
   "Elm.Flipselector.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Flipselector_Item_Data),
   _elm_flipselector_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_flipselector_item_class_get, &_elm_flipselector_item_class_desc, ELM_WIDGET_ITEM_CLASS, NULL);

#include "elm_flipselector_item_eo.legacy.c"
