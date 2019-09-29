
static Eina_Error
__eolian_efl2_input_text_input_panel_show_on_demand_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl2_input_text_input_panel_show_on_demand_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_show_on_demand_set, EFL_FUNC_CALL(ondemand), Eina_Bool ondemand);

static Eina_Value
__eolian_efl2_input_text_input_panel_show_on_demand_get_reflect(const Eo *obj)
{
   Eina_Bool val = efl2_input_text_input_panel_show_on_demand_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_show_on_demand_get, Eina_Bool, 0);
EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_language_set, EFL_FUNC_CALL(lang), Efl2_Input_Text_Panel_Lang lang);
EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_language_get, Efl2_Input_Text_Panel_Lang, 0);

static Eina_Error
__eolian_efl2_input_text_input_panel_layout_variation_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl2_input_text_input_panel_layout_variation_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_layout_variation_set, EFL_FUNC_CALL(variation), int variation);

static Eina_Value
__eolian_efl2_input_text_input_panel_layout_variation_get_reflect(const Eo *obj)
{
   int val = efl2_input_text_input_panel_layout_variation_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_layout_variation_get, int, 0);
EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_autocapital_type_set, EFL_FUNC_CALL(autocapital_type), Efl2_Input_Text_Autocapital_Type autocapital_type);
EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_autocapital_type_get, Efl2_Input_Text_Autocapital_Type, 0);

static Eina_Error
__eolian_efl2_input_text_input_panel_return_key_disabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl2_input_text_input_panel_return_key_disabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_return_key_disabled_set, EFL_FUNC_CALL(disabled), Eina_Bool disabled);

static Eina_Value
__eolian_efl2_input_text_input_panel_return_key_disabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = efl2_input_text_input_panel_return_key_disabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_return_key_disabled_get, Eina_Bool, 0);

static Eina_Error
__eolian_efl2_input_text_prediction_allow_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl2_input_text_prediction_allow_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_prediction_allow_set, EFL_FUNC_CALL(prediction), Eina_Bool prediction);

static Eina_Value
__eolian_efl2_input_text_prediction_allow_get_reflect(const Eo *obj)
{
   Eina_Bool val = efl2_input_text_prediction_allow_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_prediction_allow_get, Eina_Bool, 0);
EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_hint_set, EFL_FUNC_CALL(hints), Efl2_Input_Text_Hints hints);
EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_hint_get, Efl2_Input_Text_Hints, 0);
EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_layout_set, EFL_FUNC_CALL(layout), Efl2_Input_Text_Panel_Layout layout);
EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_layout_get, Efl2_Input_Text_Panel_Layout, 8 /* Efl2.Input.Text_Panel.Layout.invalid */);
EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_return_key_type_set, EFL_FUNC_CALL(return_key_type), Efl2_Input_Text_Panel_Return_Key_Type return_key_type);
EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_return_key_type_get, Efl2_Input_Text_Panel_Return_Key_Type, 0);

static Eina_Error
__eolian_efl2_input_text_input_panel_enabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl2_input_text_input_panel_enabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_enabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

static Eina_Value
__eolian_efl2_input_text_input_panel_enabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = efl2_input_text_input_panel_enabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(efl2_input_text_input_panel_enabled_get, Eina_Bool, 0);

static Eina_Error
__eolian_efl2_input_text_input_panel_return_key_autoenabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl2_input_text_input_panel_return_key_autoenabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_return_key_autoenabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);
EOAPI EFL_VOID_FUNC_BODY(efl2_input_text_input_panel_show);
EOAPI EFL_VOID_FUNC_BODYV(efl2_input_text_input_panel_imdata_set, EFL_FUNC_CALL(data, len), const void *data, int len);
EOAPI EFL_VOID_FUNC_BODYV_CONST(efl2_input_text_input_panel_imdata_get, EFL_FUNC_CALL(data, len), void *data, int *len);
EOAPI EFL_VOID_FUNC_BODY(efl2_input_text_input_panel_hide);

static Eina_Bool
_efl2_input_text_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EFL2_INPUT_TEXT_EXTRA_OPS
#define EFL2_INPUT_TEXT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_show_on_demand_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_show_on_demand_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_language_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_language_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_layout_variation_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_layout_variation_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_autocapital_type_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_autocapital_type_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_return_key_disabled_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_return_key_disabled_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_prediction_allow_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_prediction_allow_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_hint_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_hint_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_layout_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_layout_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_return_key_type_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_return_key_type_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_enabled_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_enabled_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_return_key_autoenabled_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_show, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_imdata_set, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_imdata_get, NULL),
      EFL_OBJECT_OP_FUNC(efl2_input_text_input_panel_hide, NULL),
      EFL2_INPUT_TEXT_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"input_panel_show_on_demand", __eolian_efl2_input_text_input_panel_show_on_demand_set_reflect, __eolian_efl2_input_text_input_panel_show_on_demand_get_reflect},
      {"input_panel_layout_variation", __eolian_efl2_input_text_input_panel_layout_variation_set_reflect, __eolian_efl2_input_text_input_panel_layout_variation_get_reflect},
      {"input_panel_return_key_disabled", __eolian_efl2_input_text_input_panel_return_key_disabled_set_reflect, __eolian_efl2_input_text_input_panel_return_key_disabled_get_reflect},
      {"prediction_allow", __eolian_efl2_input_text_prediction_allow_set_reflect, __eolian_efl2_input_text_prediction_allow_get_reflect},
      {"input_panel_enabled", __eolian_efl2_input_text_input_panel_enabled_set_reflect, __eolian_efl2_input_text_input_panel_enabled_get_reflect},
      {"input_panel_return_key_autoenabled", __eolian_efl2_input_text_input_panel_return_key_autoenabled_set_reflect, NULL},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _efl2_input_text_class_desc = {
   EO_VERSION,
   "Efl2.Input.Text",
   EFL_CLASS_TYPE_INTERFACE,
   0,
   _efl2_input_text_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(efl2_input_text_interface_get, &_efl2_input_text_class_desc, NULL, NULL);
