
int _elm_segment_control_item_index_get(const Eo *obj, Elm_Segment_Control_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_segment_control_item_index_get, int, -1 /* +1 */);

Efl_Canvas_Object *_elm_segment_control_item_object_get(const Eo *obj, Elm_Segment_Control_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_segment_control_item_object_get, Efl_Canvas_Object *, NULL);

void _elm_segment_control_item_selected_set(Eo *obj, Elm_Segment_Control_Item_Data *pd, Eina_Bool selected);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_segment_control_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Efl_Object *_elm_segment_control_item_efl_object_constructor(Eo *obj, Elm_Segment_Control_Item_Data *pd);


void _elm_segment_control_item_efl_object_destructor(Eo *obj, Elm_Segment_Control_Item_Data *pd);


Efl_Canvas_Object *_elm_segment_control_item_elm_widget_item_access_register(Eo *obj, Elm_Segment_Control_Item_Data *pd);


void _elm_segment_control_item_elm_widget_item_part_text_set(Eo *obj, Elm_Segment_Control_Item_Data *pd, const char *part, const char *label);


const char *_elm_segment_control_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Segment_Control_Item_Data *pd, const char *part);


void _elm_segment_control_item_elm_widget_item_part_content_set(Eo *obj, Elm_Segment_Control_Item_Data *pd, const char *part, Efl_Canvas_Object *content);


Efl_Canvas_Object *_elm_segment_control_item_elm_widget_item_part_content_get(const Eo *obj, Elm_Segment_Control_Item_Data *pd, const char *part);


Eina_Rect _elm_segment_control_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj, Elm_Segment_Control_Item_Data *pd);


Efl_Ui_Focus_Object *_elm_segment_control_item_efl_ui_focus_object_focus_parent_get(const Eo *obj, Elm_Segment_Control_Item_Data *pd);


static Eina_Bool
_elm_segment_control_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SEGMENT_CONTROL_ITEM_EXTRA_OPS
#define ELM_SEGMENT_CONTROL_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_index_get, _elm_segment_control_item_index_get),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_object_get, _elm_segment_control_item_object_get),
      EFL_OBJECT_OP_FUNC(elm_obj_segment_control_item_selected_set, _elm_segment_control_item_selected_set),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_segment_control_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_segment_control_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_register, _elm_segment_control_item_elm_widget_item_access_register),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_segment_control_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_segment_control_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_segment_control_item_elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_segment_control_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_geometry_get, _elm_segment_control_item_efl_ui_focus_object_focus_geometry_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_parent_get, _elm_segment_control_item_efl_ui_focus_object_focus_parent_get),
      ELM_SEGMENT_CONTROL_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_segment_control_item_class_desc = {
   EO_VERSION,
   "Elm.Segment_Control.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Segment_Control_Item_Data),
   _elm_segment_control_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_segment_control_item_class_get, &_elm_segment_control_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_FOCUS_OBJECT_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_segment_control_item_eo.legacy.c"
