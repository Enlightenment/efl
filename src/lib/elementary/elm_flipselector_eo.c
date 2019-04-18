EWAPI const Efl_Event_Description _ELM_FLIPSELECTOR_EVENT_UNDERFLOWED =
   EFL_EVENT_DESCRIPTION("underflowed");
EWAPI const Efl_Event_Description _ELM_FLIPSELECTOR_EVENT_OVERFLOWED =
   EFL_EVENT_DESCRIPTION("overflowed");

const Eina_List *_elm_flipselector_items_get(const Eo *obj, Elm_Flipselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_flipselector_first_item_get(const Eo *obj, Elm_Flipselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_first_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_flipselector_last_item_get(const Eo *obj, Elm_Flipselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_last_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_flipselector_selected_item_get(const Eo *obj, Elm_Flipselector_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_selected_item_get, Elm_Widget_Item *, NULL);

void _elm_flipselector_first_interval_set(Eo *obj, Elm_Flipselector_Data *pd, double interval);


static Eina_Error
__eolian_elm_flipselector_first_interval_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_flipselector_first_interval_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_flipselector_first_interval_set, EFL_FUNC_CALL(interval), double interval);

double _elm_flipselector_first_interval_get(const Eo *obj, Elm_Flipselector_Data *pd);


static Eina_Value
__eolian_elm_flipselector_first_interval_get_reflect(const Eo *obj)
{
   double val = elm_obj_flipselector_first_interval_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_flipselector_first_interval_get, double, 0);

Elm_Widget_Item *_elm_flipselector_item_prepend(Eo *obj, Elm_Flipselector_Data *pd, const char *label, Evas_Smart_Cb func, void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_flipselector_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, func, data), const char *label, Evas_Smart_Cb func, void *data);

void _elm_flipselector_flip_next(Eo *obj, Elm_Flipselector_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_flipselector_flip_next);

Elm_Widget_Item *_elm_flipselector_item_append(Eo *obj, Elm_Flipselector_Data *pd, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_flipselector_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(label, func, data), const char *label, Evas_Smart_Cb func, const void *data);

void _elm_flipselector_flip_prev(Eo *obj, Elm_Flipselector_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_flipselector_flip_prev);

Efl_Object *_elm_flipselector_efl_object_constructor(Eo *obj, Elm_Flipselector_Data *pd);


Eina_Error _elm_flipselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Flipselector_Data *pd);


Eina_Bool _elm_flipselector_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Flipselector_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


void _elm_flipselector_efl_ui_range_display_range_min_max_set(Eo *obj, Elm_Flipselector_Data *pd, double min, double max);


void _elm_flipselector_efl_ui_range_display_range_min_max_get(const Eo *obj, Elm_Flipselector_Data *pd, double *min, double *max);


void _elm_flipselector_efl_ui_range_interactive_range_step_set(Eo *obj, Elm_Flipselector_Data *pd, double step);


double _elm_flipselector_efl_ui_range_interactive_range_step_get(const Eo *obj, Elm_Flipselector_Data *pd);


void _elm_flipselector_efl_ui_range_display_range_value_set(Eo *obj, Elm_Flipselector_Data *pd, double val);


double _elm_flipselector_efl_ui_range_display_range_value_get(const Eo *obj, Elm_Flipselector_Data *pd);


const Efl_Access_Action_Data *_elm_flipselector_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Flipselector_Data *pd);


static Eina_Bool
_elm_flipselector_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_FLIPSELECTOR_EXTRA_OPS
#define ELM_FLIPSELECTOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_items_get, _elm_flipselector_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_first_item_get, _elm_flipselector_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_last_item_get, _elm_flipselector_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_selected_item_get, _elm_flipselector_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_first_interval_set, _elm_flipselector_first_interval_set),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_first_interval_get, _elm_flipselector_first_interval_get),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_item_prepend, _elm_flipselector_item_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_flip_next, _elm_flipselector_flip_next),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_item_append, _elm_flipselector_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_flipselector_flip_prev, _elm_flipselector_flip_prev),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_flipselector_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_flipselector_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_flipselector_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_range_min_max_set, _elm_flipselector_efl_ui_range_display_range_min_max_set),
      EFL_OBJECT_OP_FUNC(efl_ui_range_min_max_get, _elm_flipselector_efl_ui_range_display_range_min_max_get),
      EFL_OBJECT_OP_FUNC(efl_ui_range_step_set, _elm_flipselector_efl_ui_range_interactive_range_step_set),
      EFL_OBJECT_OP_FUNC(efl_ui_range_step_get, _elm_flipselector_efl_ui_range_interactive_range_step_get),
      EFL_OBJECT_OP_FUNC(efl_ui_range_value_set, _elm_flipselector_efl_ui_range_display_range_value_set),
      EFL_OBJECT_OP_FUNC(efl_ui_range_value_get, _elm_flipselector_efl_ui_range_display_range_value_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_flipselector_efl_access_widget_action_elm_actions_get),
      ELM_FLIPSELECTOR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"first_interval", __eolian_elm_flipselector_first_interval_set_reflect, __eolian_elm_flipselector_first_interval_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_flipselector_class_desc = {
   EO_VERSION,
   "Elm.Flipselector",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Flipselector_Data),
   _elm_flipselector_class_initializer,
   _elm_flipselector_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_flipselector_class_get, &_elm_flipselector_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_RANGE_INTERACTIVE_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_flipselector_eo.legacy.c"
