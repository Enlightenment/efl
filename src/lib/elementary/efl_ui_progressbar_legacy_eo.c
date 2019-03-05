
Efl_Object *_efl_ui_progressbar_legacy_efl_object_constructor(Eo *obj, void *pd);


Eina_Error _efl_ui_progressbar_legacy_efl_ui_widget_theme_apply(Eo *obj, void *pd);


Eina_Bool _efl_ui_progressbar_legacy_efl_ui_widget_widget_sub_object_del(Eo *obj, void *pd, Efl_Canvas_Object *sub_obj);


Efl_Object *_efl_ui_progressbar_legacy_efl_part_part_get(const Eo *obj, void *pd, const char *name);


static Eina_Bool
_efl_ui_progressbar_legacy_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EFL_UI_PROGRESSBAR_LEGACY_EXTRA_OPS
#define EFL_UI_PROGRESSBAR_LEGACY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _efl_ui_progressbar_legacy_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _efl_ui_progressbar_legacy_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _efl_ui_progressbar_legacy_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_part_get, _efl_ui_progressbar_legacy_efl_part_part_get),
      EFL_UI_PROGRESSBAR_LEGACY_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _efl_ui_progressbar_legacy_class_desc = {
   EO_VERSION,
   "Efl.Ui.Progressbar_Legacy",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _efl_ui_progressbar_legacy_class_initializer,
   _efl_ui_progressbar_legacy_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(efl_ui_progressbar_legacy_class_get, &_efl_ui_progressbar_legacy_class_desc, EFL_UI_PROGRESSBAR_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);
