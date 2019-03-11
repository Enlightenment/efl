
Efl_Object *_elm_dayselector_item_efl_object_constructor(Eo *obj, Elm_Dayselector_Item_Data *pd);


static Eina_Bool
_elm_dayselector_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_DAYSELECTOR_ITEM_EXTRA_OPS
#define ELM_DAYSELECTOR_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_dayselector_item_efl_object_constructor),
      ELM_DAYSELECTOR_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_dayselector_item_class_desc = {
   EO_VERSION,
   "Elm.Dayselector.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Dayselector_Item_Data),
   _elm_dayselector_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_dayselector_item_class_get, &_elm_dayselector_item_class_desc, ELM_WIDGET_ITEM_CLASS, NULL);
