EOAPI EFL_FUNC_BODY_CONST(elm_widget_item_container_focused_item_get, Elm_Widget_Item *, NULL);

static Eina_Bool
_elm_widget_item_container_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_WIDGET_ITEM_CONTAINER_EXTRA_OPS
#define ELM_WIDGET_ITEM_CONTAINER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_widget_item_container_focused_item_get, NULL),
      ELM_WIDGET_ITEM_CONTAINER_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_widget_item_container_class_desc = {
   EO_VERSION,
   "Elm.Widget_Item_Container",
   EFL_CLASS_TYPE_INTERFACE,
   0,
   _elm_widget_item_container_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_widget_item_container_interface_get, &_elm_widget_item_container_class_desc, NULL, NULL);
