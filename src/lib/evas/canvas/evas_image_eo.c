
Eina_Error _evas_image_efl_file_load(Eo *obj, void *pd);


static Eina_Bool
_evas_image_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EVAS_IMAGE_EXTRA_OPS
#define EVAS_IMAGE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_file_load, _evas_image_efl_file_load),
      EVAS_IMAGE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _evas_image_class_desc = {
   EO_VERSION,
   "Evas.Image",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _evas_image_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(evas_image_class_get, &_evas_image_class_desc, EFL_CANVAS_IMAGE_INTERNAL_CLASS, EFL_FILE_MIXIN, NULL);
