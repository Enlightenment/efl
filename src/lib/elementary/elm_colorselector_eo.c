EWAPI const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_COLOR_ITEM_SELECTED =
   EFL_EVENT_DESCRIPTION("color,item,selected");
EWAPI const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_COLOR_ITEM_LONGPRESSED =
   EFL_EVENT_DESCRIPTION("color,item,longpressed");
EWAPI const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");
EWAPI const Efl_Event_Description _ELM_COLORSELECTOR_EVENT_CHANGED_USER =
   EFL_EVENT_DESCRIPTION("changed,user");

void _elm_colorselector_picked_color_set(Eo *obj, Elm_Colorselector_Data *pd, int r, int g, int b, int a);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_colorselector_picked_color_set, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _elm_colorselector_picked_color_get(const Eo *obj, Elm_Colorselector_Data *pd, int *r, int *g, int *b, int *a);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_colorselector_picked_color_get, EFL_FUNC_CALL(r, g, b, a), int *r, int *g, int *b, int *a);

void _elm_colorselector_palette_name_set(Eo *obj, Elm_Colorselector_Data *pd, const char *palette_name);


static Eina_Error
__eolian_elm_colorselector_palette_name_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_colorselector_palette_name_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_colorselector_palette_name_set, EFL_FUNC_CALL(palette_name), const char *palette_name);

const char *_elm_colorselector_palette_name_get(const Eo *obj, Elm_Colorselector_Data *pd);


static Eina_Value
__eolian_elm_colorselector_palette_name_get_reflect(Eo *obj)
{
   const char *val = elm_obj_colorselector_palette_name_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_colorselector_palette_name_get, const char *, NULL);

void _elm_colorselector_mode_set(Eo *obj, Elm_Colorselector_Data *pd, Elm_Colorselector_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_colorselector_mode_set, EFL_FUNC_CALL(mode), Elm_Colorselector_Mode mode);

Elm_Colorselector_Mode _elm_colorselector_mode_get(const Eo *obj, Elm_Colorselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_colorselector_mode_get, Elm_Colorselector_Mode, 0);

const Eina_List *_elm_colorselector_palette_items_get(const Eo *obj, Elm_Colorselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_colorselector_palette_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_colorselector_palette_selected_item_get(const Eo *obj, Elm_Colorselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_colorselector_palette_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_colorselector_palette_color_add(Eo *obj, Elm_Colorselector_Data *pd, int r, int g, int b, int a);

EOAPI EFL_FUNC_BODYV(elm_obj_colorselector_palette_color_add, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _elm_colorselector_palette_clear(Eo *obj, Elm_Colorselector_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_colorselector_palette_clear);

Efl_Object *_elm_colorselector_efl_object_constructor(Eo *obj, Elm_Colorselector_Data *pd);


Eina_Error _elm_colorselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Colorselector_Data *pd);


Eina_Rect _elm_colorselector_efl_ui_widget_focus_highlight_geometry_get(const Eo *obj, Elm_Colorselector_Data *pd);


void _elm_colorselector_efl_ui_widget_on_access_update(Eo *obj, Elm_Colorselector_Data *pd, Eina_Bool enable);


Eina_Bool _elm_colorselector_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Colorselector_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


const Efl_Access_Action_Data *_elm_colorselector_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Colorselector_Data *pd);


Eina_List *_elm_colorselector_efl_access_object_access_children_get(const Eo *obj, Elm_Colorselector_Data *pd);


static Eina_Bool
_elm_colorselector_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_COLORSELECTOR_EXTRA_OPS
#define ELM_COLORSELECTOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_picked_color_set, _elm_colorselector_picked_color_set),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_picked_color_get, _elm_colorselector_picked_color_get),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_palette_name_set, _elm_colorselector_palette_name_set),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_palette_name_get, _elm_colorselector_palette_name_get),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_mode_set, _elm_colorselector_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_mode_get, _elm_colorselector_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_palette_items_get, _elm_colorselector_palette_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_palette_selected_item_get, _elm_colorselector_palette_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_palette_color_add, _elm_colorselector_palette_color_add),
      EFL_OBJECT_OP_FUNC(elm_obj_colorselector_palette_clear, _elm_colorselector_palette_clear),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_colorselector_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_colorselector_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_highlight_geometry_get, _elm_colorselector_efl_ui_widget_focus_highlight_geometry_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_colorselector_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_colorselector_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_colorselector_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_colorselector_efl_access_object_access_children_get),
      ELM_COLORSELECTOR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"palette_name", __eolian_elm_colorselector_palette_name_set_reflect, __eolian_elm_colorselector_palette_name_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_colorselector_class_desc = {
   EO_VERSION,
   "Elm.Colorselector",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Colorselector_Data),
   _elm_colorselector_class_initializer,
   _elm_colorselector_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_colorselector_class_get, &_elm_colorselector_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_colorselector.eo.legacy.c"
