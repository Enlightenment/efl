
void _elm_hoversel_item_icon_set(Eo *obj, Elm_Hoversel_Item_Data *pd, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_hoversel_item_icon_set, EFL_FUNC_CALL(icon_file, icon_group, icon_type), const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type);

void _elm_hoversel_item_icon_get(const Eo *obj, Elm_Hoversel_Item_Data *pd, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_hoversel_item_icon_get, EFL_FUNC_CALL(icon_file, icon_group, icon_type), const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);

Efl_Object *_elm_hoversel_item_efl_object_constructor(Eo *obj, Elm_Hoversel_Item_Data *pd);


void _elm_hoversel_item_efl_object_destructor(Eo *obj, Elm_Hoversel_Item_Data *pd);


void _elm_hoversel_item_elm_widget_item_disable(Eo *obj, Elm_Hoversel_Item_Data *pd);


void _elm_hoversel_item_elm_widget_item_signal_emit(Eo *obj, Elm_Hoversel_Item_Data *pd, const char *emission, const char *source);


void _elm_hoversel_item_elm_widget_item_part_text_set(Eo *obj, Elm_Hoversel_Item_Data *pd, const char *part, const char *label);


const char *_elm_hoversel_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Hoversel_Item_Data *pd, const char *part);


void _elm_hoversel_item_elm_widget_item_style_set(Eo *obj, Elm_Hoversel_Item_Data *pd, const char *style);


const char *_elm_hoversel_item_elm_widget_item_style_get(const Eo *obj, Elm_Hoversel_Item_Data *pd);


void _elm_hoversel_item_elm_widget_item_item_focus_set(Eo *obj, Elm_Hoversel_Item_Data *pd, Eina_Bool focused);


Eina_Bool _elm_hoversel_item_elm_widget_item_item_focus_get(const Eo *obj, Elm_Hoversel_Item_Data *pd);


static Eina_Bool
_elm_hoversel_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_HOVERSEL_ITEM_EXTRA_OPS
#define ELM_HOVERSEL_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_item_icon_set, _elm_hoversel_item_icon_set),
      EFL_OBJECT_OP_FUNC(elm_obj_hoversel_item_icon_get, _elm_hoversel_item_icon_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_hoversel_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_hoversel_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_hoversel_item_elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_hoversel_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_hoversel_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_hoversel_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_set, _elm_hoversel_item_elm_widget_item_style_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_get, _elm_hoversel_item_elm_widget_item_style_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_set, _elm_hoversel_item_elm_widget_item_item_focus_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_get, _elm_hoversel_item_elm_widget_item_item_focus_get),
      ELM_HOVERSEL_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_hoversel_item_class_desc = {
   EO_VERSION,
   "Elm.Hoversel.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Hoversel_Item_Data),
   _elm_hoversel_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_hoversel_item_class_get, &_elm_hoversel_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_hoversel_item_eo.legacy.c"
