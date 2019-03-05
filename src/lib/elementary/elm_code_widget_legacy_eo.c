
static Eina_Bool
_elm_code_widget_legacy_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifdef ELM_CODE_WIDGET_LEGACY_EXTRA_OPS
   EFL_OPS_DEFINE(ops, ELM_CODE_WIDGET_LEGACY_EXTRA_OPS);
   opsp = &ops;
#endif

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_code_widget_legacy_class_desc = {
   EO_VERSION,
   "Elm.Code_Widget_Legacy",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_code_widget_legacy_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_code_widget_legacy_class_get, &_elm_code_widget_legacy_class_desc, ELM_CODE_WIDGET_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);
