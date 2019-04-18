
Efl_Object *_elm_widget_item_static_focus_efl_object_constructor(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd);


void _elm_widget_item_static_focus_efl_object_destructor(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd);


void _elm_widget_item_static_focus_efl_ui_focus_object_setup_order_non_recursive(Eo *obj, Elm_Widget_Item_Static_Focus_Data *pd);


static Eina_Bool
_elm_widget_item_static_focus_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_WIDGET_ITEM_STATIC_FOCUS_EXTRA_OPS
#define ELM_WIDGET_ITEM_STATIC_FOCUS_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_widget_item_static_focus_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_widget_item_static_focus_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_setup_order_non_recursive, _elm_widget_item_static_focus_efl_ui_focus_object_setup_order_non_recursive),
      ELM_WIDGET_ITEM_STATIC_FOCUS_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_widget_item_static_focus_class_desc = {
   EO_VERSION,
   "Elm.Widget.Item.Static_Focus",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Widget_Item_Static_Focus_Data),
   _elm_widget_item_static_focus_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_widget_item_static_focus_class_get, &_elm_widget_item_static_focus_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_FOCUS_OBJECT_MIXIN, NULL);
