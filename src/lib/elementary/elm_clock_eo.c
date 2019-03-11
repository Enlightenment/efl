EWAPI const Efl_Event_Description _ELM_CLOCK_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");

void _elm_clock_show_am_pm_set(Eo *obj, Elm_Clock_Data *pd, Eina_Bool am_pm);


static Eina_Error
__eolian_elm_clock_show_am_pm_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_clock_show_am_pm_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_show_am_pm_set, EFL_FUNC_CALL(am_pm), Eina_Bool am_pm);

Eina_Bool _elm_clock_show_am_pm_get(const Eo *obj, Elm_Clock_Data *pd);


static Eina_Value
__eolian_elm_clock_show_am_pm_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_clock_show_am_pm_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_clock_show_am_pm_get, Eina_Bool, 0);

void _elm_clock_first_interval_set(Eo *obj, Elm_Clock_Data *pd, double interval);


static Eina_Error
__eolian_elm_clock_first_interval_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_clock_first_interval_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_first_interval_set, EFL_FUNC_CALL(interval), double interval);

double _elm_clock_first_interval_get(const Eo *obj, Elm_Clock_Data *pd);


static Eina_Value
__eolian_elm_clock_first_interval_get_reflect(Eo *obj)
{
   double val = elm_obj_clock_first_interval_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_clock_first_interval_get, double, 0);

void _elm_clock_show_seconds_set(Eo *obj, Elm_Clock_Data *pd, Eina_Bool seconds);


static Eina_Error
__eolian_elm_clock_show_seconds_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_clock_show_seconds_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_show_seconds_set, EFL_FUNC_CALL(seconds), Eina_Bool seconds);

Eina_Bool _elm_clock_show_seconds_get(const Eo *obj, Elm_Clock_Data *pd);


static Eina_Value
__eolian_elm_clock_show_seconds_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_clock_show_seconds_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_clock_show_seconds_get, Eina_Bool, 0);

void _elm_clock_edit_set(Eo *obj, Elm_Clock_Data *pd, Eina_Bool edit);


static Eina_Error
__eolian_elm_clock_edit_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_clock_edit_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_edit_set, EFL_FUNC_CALL(edit), Eina_Bool edit);

Eina_Bool _elm_clock_edit_get(const Eo *obj, Elm_Clock_Data *pd);


static Eina_Value
__eolian_elm_clock_edit_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_clock_edit_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_clock_edit_get, Eina_Bool, 0);

void _elm_clock_pause_set(Eo *obj, Elm_Clock_Data *pd, Eina_Bool paused);


static Eina_Error
__eolian_elm_clock_pause_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_clock_pause_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_pause_set, EFL_FUNC_CALL(paused), Eina_Bool paused);

Eina_Bool _elm_clock_pause_get(const Eo *obj, Elm_Clock_Data *pd);


static Eina_Value
__eolian_elm_clock_pause_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_clock_pause_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_clock_pause_get, Eina_Bool, 0);

void _elm_clock_time_set(Eo *obj, Elm_Clock_Data *pd, int hrs, int min, int sec);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_time_set, EFL_FUNC_CALL(hrs, min, sec), int hrs, int min, int sec);

void _elm_clock_time_get(const Eo *obj, Elm_Clock_Data *pd, int *hrs, int *min, int *sec);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_clock_time_get, EFL_FUNC_CALL(hrs, min, sec), int *hrs, int *min, int *sec);

void _elm_clock_edit_mode_set(Eo *obj, Elm_Clock_Data *pd, Elm_Clock_Edit_Mode digedit);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_clock_edit_mode_set, EFL_FUNC_CALL(digedit), Elm_Clock_Edit_Mode digedit);

Elm_Clock_Edit_Mode _elm_clock_edit_mode_get(const Eo *obj, Elm_Clock_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_clock_edit_mode_get, Elm_Clock_Edit_Mode, 0);

Efl_Object *_elm_clock_efl_object_constructor(Eo *obj, Elm_Clock_Data *pd);


void _elm_clock_efl_ui_widget_on_access_update(Eo *obj, Elm_Clock_Data *pd, Eina_Bool enable);


Eina_Error _elm_clock_efl_ui_widget_theme_apply(Eo *obj, Elm_Clock_Data *pd);


static Eina_Bool
_elm_clock_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_CLOCK_EXTRA_OPS
#define ELM_CLOCK_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_clock_show_am_pm_set, _elm_clock_show_am_pm_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_show_am_pm_get, _elm_clock_show_am_pm_get),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_first_interval_set, _elm_clock_first_interval_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_first_interval_get, _elm_clock_first_interval_get),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_show_seconds_set, _elm_clock_show_seconds_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_show_seconds_get, _elm_clock_show_seconds_get),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_edit_set, _elm_clock_edit_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_edit_get, _elm_clock_edit_get),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_pause_set, _elm_clock_pause_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_pause_get, _elm_clock_pause_get),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_time_set, _elm_clock_time_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_time_get, _elm_clock_time_get),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_edit_mode_set, _elm_clock_edit_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_clock_edit_mode_get, _elm_clock_edit_mode_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_clock_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_clock_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_clock_efl_ui_widget_theme_apply),
      ELM_CLOCK_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"show_am_pm", __eolian_elm_clock_show_am_pm_set_reflect, __eolian_elm_clock_show_am_pm_get_reflect},
      {"first_interval", __eolian_elm_clock_first_interval_set_reflect, __eolian_elm_clock_first_interval_get_reflect},
      {"show_seconds", __eolian_elm_clock_show_seconds_set_reflect, __eolian_elm_clock_show_seconds_get_reflect},
      {"edit", __eolian_elm_clock_edit_set_reflect, __eolian_elm_clock_edit_get_reflect},
      {"pause", __eolian_elm_clock_pause_set_reflect, __eolian_elm_clock_pause_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_clock_class_desc = {
   EO_VERSION,
   "Elm.Clock",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Clock_Data),
   _elm_clock_class_initializer,
   _elm_clock_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_clock_class_get, &_elm_clock_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_clock_eo.legacy.c"
