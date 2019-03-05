
void _elm_color_item_color_set(Eo *obj, Elm_Color_Item_Data *pd, int r, int g, int b, int a);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_color_item_color_set, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _elm_color_item_color_get(const Eo *obj, Elm_Color_Item_Data *pd, int *r, int *g, int *b, int *a);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_color_item_color_get, EFL_FUNC_CALL(r, g, b, a), int *r, int *g, int *b, int *a);

void _elm_color_item_selected_set(Eo *obj, Elm_Color_Item_Data *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_color_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_color_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_color_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Eina_Bool _elm_color_item_selected_get(const Eo *obj, Elm_Color_Item_Data *pd);


static Eina_Value
__eolian_elm_color_item_selected_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_color_item_selected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_color_item_selected_get, Eina_Bool, 0);

Efl_Object *_elm_color_item_efl_object_constructor(Eo *obj, Elm_Color_Item_Data *pd);


void _elm_color_item_efl_object_destructor(Eo *obj, Elm_Color_Item_Data *pd);


Efl_Canvas_Object *_elm_color_item_elm_widget_item_access_register(Eo *obj, Elm_Color_Item_Data *pd);


void _elm_color_item_elm_widget_item_signal_emit(Eo *obj, Elm_Color_Item_Data *pd, const char *emission, const char *source);


void _elm_color_item_elm_widget_item_item_focus_set(Eo *obj, Elm_Color_Item_Data *pd, Eina_Bool focused);


Eina_Bool _elm_color_item_elm_widget_item_item_focus_get(const Eo *obj, Elm_Color_Item_Data *pd);


void _elm_color_item_efl_ui_focus_object_focus_set(Eo *obj, Elm_Color_Item_Data *pd, Eina_Bool focus);


Eina_Rect _elm_color_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj, Elm_Color_Item_Data *pd);


Efl_Ui_Focus_Object *_elm_color_item_efl_ui_focus_object_focus_parent_get(const Eo *obj, Elm_Color_Item_Data *pd);


Efl_Ui_Focus_Manager *_elm_color_item_efl_ui_focus_object_focus_manager_get(const Eo *obj, Elm_Color_Item_Data *pd);


Efl_Access_State_Set _elm_color_item_efl_access_object_state_set_get(const Eo *obj, Elm_Color_Item_Data *pd);


const char *_elm_color_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_Color_Item_Data *pd);


const Efl_Access_Action_Data *_elm_color_item_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Color_Item_Data *pd);


static Eina_Bool
_elm_color_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_COLOR_ITEM_EXTRA_OPS
#define ELM_COLOR_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_color_item_color_set, _elm_color_item_color_set),
      EFL_OBJECT_OP_FUNC(elm_obj_color_item_color_get, _elm_color_item_color_get),
      EFL_OBJECT_OP_FUNC(elm_obj_color_item_selected_set, _elm_color_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_color_item_selected_get, _elm_color_item_selected_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_color_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_color_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_register, _elm_color_item_elm_widget_item_access_register),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_color_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_set, _elm_color_item_elm_widget_item_item_focus_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_get, _elm_color_item_elm_widget_item_item_focus_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_set, _elm_color_item_efl_ui_focus_object_focus_set),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_geometry_get, _elm_color_item_efl_ui_focus_object_focus_geometry_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_parent_get, _elm_color_item_efl_ui_focus_object_focus_parent_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_manager_get, _elm_color_item_efl_ui_focus_object_focus_manager_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_color_item_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_color_item_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_color_item_efl_access_widget_action_elm_actions_get),
      ELM_COLOR_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"selected", __eolian_elm_color_item_selected_set_reflect, __eolian_elm_color_item_selected_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_color_item_class_desc = {
   EO_VERSION,
   "Elm.Color.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Color_Item_Data),
   _elm_color_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_color_item_class_get, &_elm_color_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_FOCUS_OBJECT_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, NULL);

#include "elm_color_item_eo.legacy.c"
