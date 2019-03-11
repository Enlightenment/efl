
void _elm_separator_horizontal_set(Eo *obj, Elm_Separator_Data *pd, Eina_Bool horizontal);


static Eina_Error
__eolian_elm_separator_horizontal_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_separator_horizontal_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_separator_horizontal_set, EFL_FUNC_CALL(horizontal), Eina_Bool horizontal);

Eina_Bool _elm_separator_horizontal_get(const Eo *obj, Elm_Separator_Data *pd);


static Eina_Value
__eolian_elm_separator_horizontal_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_separator_horizontal_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_separator_horizontal_get, Eina_Bool, 0);

Efl_Object *_elm_separator_efl_object_constructor(Eo *obj, Elm_Separator_Data *pd);


Eina_Error _elm_separator_efl_ui_widget_theme_apply(Eo *obj, Elm_Separator_Data *pd);


static Eina_Bool
_elm_separator_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SEPARATOR_EXTRA_OPS
#define ELM_SEPARATOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_separator_horizontal_set, _elm_separator_horizontal_set),
      EFL_OBJECT_OP_FUNC(elm_obj_separator_horizontal_get, _elm_separator_horizontal_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_separator_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_separator_efl_ui_widget_theme_apply),
      ELM_SEPARATOR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"horizontal", __eolian_elm_separator_horizontal_set_reflect, __eolian_elm_separator_horizontal_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_separator_class_desc = {
   EO_VERSION,
   "Elm.Separator",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Separator_Data),
   _elm_separator_class_initializer,
   _elm_separator_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_separator_class_get, &_elm_separator_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_separator_eo.legacy.c"
