
Efl_Object *_edje_edit_efl_object_constructor(Eo *obj, Edje_Edit *pd);


void _edje_edit_efl_object_destructor(Eo *obj, Edje_Edit *pd);


Eina_Error _edje_edit_efl_file_load(Eo *obj, Edje_Edit *pd);


static Eina_Bool
_edje_edit_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EDJE_EDIT_EXTRA_OPS
#define EDJE_EDIT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _edje_edit_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _edje_edit_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_file_load, _edje_edit_efl_file_load),
      EDJE_EDIT_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _edje_edit_class_desc = {
   EO_VERSION,
   "Edje.Edit",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Edje_Edit),
   _edje_edit_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(edje_edit_class_get, &_edje_edit_class_desc, EFL_CANVAS_LAYOUT_CLASS, NULL);
