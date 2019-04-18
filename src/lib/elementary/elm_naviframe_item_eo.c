
void _elm_naviframe_item_pop_to(Eo *obj, Elm_Naviframe_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_naviframe_item_pop_to);

Eina_Bool _elm_naviframe_item_title_enabled_get(const Eo *obj, Elm_Naviframe_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_item_title_enabled_get, Eina_Bool, 0);

void _elm_naviframe_item_title_enabled_set(Eo *obj, Elm_Naviframe_Item_Data *pd, Eina_Bool enable, Eina_Bool transition);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_naviframe_item_title_enabled_set, EFL_FUNC_CALL(enable, transition), Eina_Bool enable, Eina_Bool transition);

void _elm_naviframe_item_promote(Eo *obj, Elm_Naviframe_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_naviframe_item_promote);

void _elm_naviframe_item_pop_cb_set(Eo *obj, Elm_Naviframe_Item_Data *pd, Elm_Naviframe_Item_Pop_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_naviframe_item_pop_cb_set, EFL_FUNC_CALL(func, data), Elm_Naviframe_Item_Pop_Cb func, void *data);

Efl_Object *_elm_naviframe_item_efl_object_constructor(Eo *obj, Elm_Naviframe_Item_Data *pd);


void _elm_naviframe_item_efl_object_invalidate(Eo *obj, Elm_Naviframe_Item_Data *pd);


void _elm_naviframe_item_efl_object_destructor(Eo *obj, Elm_Naviframe_Item_Data *pd);


void _elm_naviframe_item_elm_widget_item_signal_emit(Eo *obj, Elm_Naviframe_Item_Data *pd, const char *emission, const char *source);


void _elm_naviframe_item_elm_widget_item_part_text_set(Eo *obj, Elm_Naviframe_Item_Data *pd, const char *part, const char *label);


const char *_elm_naviframe_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Naviframe_Item_Data *pd, const char *part);


void _elm_naviframe_item_elm_widget_item_part_content_set(Eo *obj, Elm_Naviframe_Item_Data *pd, const char *part, Efl_Canvas_Object *content);


Efl_Canvas_Object *_elm_naviframe_item_elm_widget_item_part_content_get(const Eo *obj, Elm_Naviframe_Item_Data *pd, const char *part);


Efl_Canvas_Object *_elm_naviframe_item_elm_widget_item_part_content_unset(Eo *obj, Elm_Naviframe_Item_Data *pd, const char *part);


void _elm_naviframe_item_elm_widget_item_style_set(Eo *obj, Elm_Naviframe_Item_Data *pd, const char *style);


Eina_List *_elm_naviframe_item_efl_access_object_access_children_get(const Eo *obj, Elm_Naviframe_Item_Data *pd);


static Eina_Bool
_elm_naviframe_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_NAVIFRAME_ITEM_EXTRA_OPS
#define ELM_NAVIFRAME_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_pop_to, _elm_naviframe_item_pop_to),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_title_enabled_get, _elm_naviframe_item_title_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_title_enabled_set, _elm_naviframe_item_title_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_promote, _elm_naviframe_item_promote),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_pop_cb_set, _elm_naviframe_item_pop_cb_set),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_naviframe_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_invalidate, _elm_naviframe_item_efl_object_invalidate),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_naviframe_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_naviframe_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_naviframe_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_naviframe_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_naviframe_item_elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_naviframe_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_unset, _elm_naviframe_item_elm_widget_item_part_content_unset),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_set, _elm_naviframe_item_elm_widget_item_style_set),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_naviframe_item_efl_access_object_access_children_get),
      ELM_NAVIFRAME_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_naviframe_item_class_desc = {
   EO_VERSION,
   "Elm.Naviframe.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Naviframe_Item_Data),
   _elm_naviframe_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_naviframe_item_class_get, &_elm_naviframe_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_naviframe_item_eo.legacy.c"
