
void _elm_dayselector_week_start_set(Eo *obj, Elm_Dayselector_Data *pd, Elm_Dayselector_Day day);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_dayselector_week_start_set, EFL_FUNC_CALL(day), Elm_Dayselector_Day day);

Elm_Dayselector_Day _elm_dayselector_week_start_get(const Eo *obj, Elm_Dayselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_dayselector_week_start_get, Elm_Dayselector_Day, 0);

void _elm_dayselector_weekend_length_set(Eo *obj, Elm_Dayselector_Data *pd, unsigned int length);


static Eina_Error
__eolian_elm_dayselector_weekend_length_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   unsigned int cval;
   if (!eina_value_uint_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_dayselector_weekend_length_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_dayselector_weekend_length_set, EFL_FUNC_CALL(length), unsigned int length);

unsigned int _elm_dayselector_weekend_length_get(const Eo *obj, Elm_Dayselector_Data *pd);


static Eina_Value
__eolian_elm_dayselector_weekend_length_get_reflect(const Eo *obj)
{
   unsigned int val = elm_obj_dayselector_weekend_length_get(obj);
   return eina_value_uint_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_dayselector_weekend_length_get, unsigned int, 0);

void _elm_dayselector_weekend_start_set(Eo *obj, Elm_Dayselector_Data *pd, Elm_Dayselector_Day day);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_dayselector_weekend_start_set, EFL_FUNC_CALL(day), Elm_Dayselector_Day day);

Elm_Dayselector_Day _elm_dayselector_weekend_start_get(const Eo *obj, Elm_Dayselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_dayselector_weekend_start_get, Elm_Dayselector_Day, 0);

void _elm_dayselector_weekdays_names_set(Eo *obj, Elm_Dayselector_Data *pd, const char **weekdays);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_dayselector_weekdays_names_set, EFL_FUNC_CALL(weekdays), const char **weekdays);

Eina_List *_elm_dayselector_weekdays_names_get(const Eo *obj, Elm_Dayselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_dayselector_weekdays_names_get, Eina_List *, NULL);

void _elm_dayselector_day_selected_set(Eo *obj, Elm_Dayselector_Data *pd, Elm_Dayselector_Day day, Eina_Bool selected);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_dayselector_day_selected_set, EFL_FUNC_CALL(day, selected), Elm_Dayselector_Day day, Eina_Bool selected);

Eina_Bool _elm_dayselector_day_selected_get(const Eo *obj, Elm_Dayselector_Data *pd, Elm_Dayselector_Day day);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_dayselector_day_selected_get, Eina_Bool, 0, EFL_FUNC_CALL(day), Elm_Dayselector_Day day);

Efl_Object *_elm_dayselector_efl_object_constructor(Eo *obj, Elm_Dayselector_Data *pd);


Eina_Error _elm_dayselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Dayselector_Data *pd);


void _elm_dayselector_efl_ui_l10n_translation_update(Eo *obj, Elm_Dayselector_Data *pd);


Efl_Object *_elm_dayselector_efl_part_part_get(const Eo *obj, Elm_Dayselector_Data *pd, const char *name);


static Eina_Bool
_elm_dayselector_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_DAYSELECTOR_EXTRA_OPS
#define ELM_DAYSELECTOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_week_start_set, _elm_dayselector_week_start_set),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_week_start_get, _elm_dayselector_week_start_get),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_weekend_length_set, _elm_dayselector_weekend_length_set),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_weekend_length_get, _elm_dayselector_weekend_length_get),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_weekend_start_set, _elm_dayselector_weekend_start_set),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_weekend_start_get, _elm_dayselector_weekend_start_get),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_weekdays_names_set, _elm_dayselector_weekdays_names_set),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_weekdays_names_get, _elm_dayselector_weekdays_names_get),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_day_selected_set, _elm_dayselector_day_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_dayselector_day_selected_get, _elm_dayselector_day_selected_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_dayselector_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_dayselector_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_dayselector_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_dayselector_efl_part_part_get),
      ELM_DAYSELECTOR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"weekend_length", __eolian_elm_dayselector_weekend_length_set_reflect, __eolian_elm_dayselector_weekend_length_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_dayselector_class_desc = {
   EO_VERSION,
   "Elm.Dayselector",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Dayselector_Data),
   _elm_dayselector_class_initializer,
   _elm_dayselector_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_dayselector_class_get, &_elm_dayselector_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_dayselector_eo.legacy.c"
