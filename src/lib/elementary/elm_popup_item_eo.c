
Efl_Object *_elm_popup_item_efl_object_constructor(Eo *obj, Elm_Popup_Item_Data *pd);


void _elm_popup_item_efl_object_destructor(Eo *obj, Elm_Popup_Item_Data *pd);


void _elm_popup_item_elm_widget_item_disable(Eo *obj, Elm_Popup_Item_Data *pd);


void _elm_popup_item_elm_widget_item_signal_emit(Eo *obj, Elm_Popup_Item_Data *pd, const char *emission, const char *source);


void _elm_popup_item_elm_widget_item_part_text_set(Eo *obj, Elm_Popup_Item_Data *pd, const char *part, const char *label);


const char *_elm_popup_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Popup_Item_Data *pd, const char *part);


void _elm_popup_item_elm_widget_item_part_content_set(Eo *obj, Elm_Popup_Item_Data *pd, const char *part, Efl_Canvas_Object *content);


Efl_Canvas_Object *_elm_popup_item_elm_widget_item_part_content_get(const Eo *obj, Elm_Popup_Item_Data *pd, const char *part);


Efl_Canvas_Object *_elm_popup_item_elm_widget_item_part_content_unset(Eo *obj, Elm_Popup_Item_Data *pd, const char *part);


static Eina_Bool
_elm_popup_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_POPUP_ITEM_EXTRA_OPS
#define ELM_POPUP_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_popup_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_popup_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_popup_item_elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_popup_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_popup_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_popup_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_popup_item_elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_popup_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_unset, _elm_popup_item_elm_widget_item_part_content_unset),
      ELM_POPUP_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_popup_item_class_desc = {
   EO_VERSION,
   "Elm.Popup.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Popup_Item_Data),
   _elm_popup_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_popup_item_class_get, &_elm_popup_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);
