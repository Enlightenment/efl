
Efl_Object *_efl_ui_image_zoomable_legacy_efl_object_constructor(Eo *obj, void *pd);


static Eina_Bool
_efl_ui_image_zoomable_legacy_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EFL_UI_IMAGE_ZOOMABLE_LEGACY_EXTRA_OPS
#define EFL_UI_IMAGE_ZOOMABLE_LEGACY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _efl_ui_image_zoomable_legacy_efl_object_constructor),
      EFL_UI_IMAGE_ZOOMABLE_LEGACY_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _efl_ui_image_zoomable_legacy_class_desc = {
   EO_VERSION,
   "Efl.Ui.Image_Zoomable_Legacy",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _efl_ui_image_zoomable_legacy_class_initializer,
   _efl_ui_image_zoomable_legacy_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(efl_ui_image_zoomable_legacy_class_get, &_efl_ui_image_zoomable_legacy_class_desc, EFL_UI_IMAGE_ZOOMABLE_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);
