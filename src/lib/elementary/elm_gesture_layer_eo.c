
void _elm_gesture_layer_zoom_step_set(Eo *obj, Elm_Gesture_Layer_Data *pd, double step);


static Eina_Error
__eolian_elm_gesture_layer_zoom_step_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gesture_layer_zoom_step_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_zoom_step_set, EFL_FUNC_CALL(step), double step);

double _elm_gesture_layer_zoom_step_get(const Eo *obj, Elm_Gesture_Layer_Data *pd);


static Eina_Value
__eolian_elm_gesture_layer_zoom_step_get_reflect(Eo *obj)
{
   double val = elm_obj_gesture_layer_zoom_step_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gesture_layer_zoom_step_get, double, 0);

void _elm_gesture_layer_tap_finger_size_set(Eo *obj, Elm_Gesture_Layer_Data *pd, int sz);


static Eina_Error
__eolian_elm_gesture_layer_tap_finger_size_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gesture_layer_tap_finger_size_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_tap_finger_size_set, EFL_FUNC_CALL(sz), int sz);

int _elm_gesture_layer_tap_finger_size_get(const Eo *obj, Elm_Gesture_Layer_Data *pd);


static Eina_Value
__eolian_elm_gesture_layer_tap_finger_size_get_reflect(Eo *obj)
{
   int val = elm_obj_gesture_layer_tap_finger_size_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gesture_layer_tap_finger_size_get, int, 0);

void _elm_gesture_layer_hold_events_set(Eo *obj, Elm_Gesture_Layer_Data *pd, Eina_Bool hold_events);


static Eina_Error
__eolian_elm_gesture_layer_hold_events_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gesture_layer_hold_events_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_hold_events_set, EFL_FUNC_CALL(hold_events), Eina_Bool hold_events);

Eina_Bool _elm_gesture_layer_hold_events_get(const Eo *obj, Elm_Gesture_Layer_Data *pd);


static Eina_Value
__eolian_elm_gesture_layer_hold_events_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_gesture_layer_hold_events_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gesture_layer_hold_events_get, Eina_Bool, 0);

void _elm_gesture_layer_rotate_step_set(Eo *obj, Elm_Gesture_Layer_Data *pd, double step);


static Eina_Error
__eolian_elm_gesture_layer_rotate_step_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_gesture_layer_rotate_step_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_rotate_step_set, EFL_FUNC_CALL(step), double step);

double _elm_gesture_layer_rotate_step_get(const Eo *obj, Elm_Gesture_Layer_Data *pd);


static Eina_Value
__eolian_elm_gesture_layer_rotate_step_get_reflect(Eo *obj)
{
   double val = elm_obj_gesture_layer_rotate_step_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_gesture_layer_rotate_step_get, double, 0);

void _elm_gesture_layer_cb_set(Eo *obj, Elm_Gesture_Layer_Data *pd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_cb_set, EFL_FUNC_CALL(idx, cb_type, cb, data), Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

Eina_Bool _elm_gesture_layer_attach(Eo *obj, Elm_Gesture_Layer_Data *pd, Efl_Canvas_Object *target);

EOAPI EFL_FUNC_BODYV(elm_obj_gesture_layer_attach, Eina_Bool, 0, EFL_FUNC_CALL(target), Efl_Canvas_Object *target);

void _elm_gesture_layer_cb_del(Eo *obj, Elm_Gesture_Layer_Data *pd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_cb_del, EFL_FUNC_CALL(idx, cb_type, cb, data), Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

void _elm_gesture_layer_cb_add(Eo *obj, Elm_Gesture_Layer_Data *pd, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_gesture_layer_cb_add, EFL_FUNC_CALL(idx, cb_type, cb, data), Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

Efl_Object *_elm_gesture_layer_efl_object_constructor(Eo *obj, Elm_Gesture_Layer_Data *pd);


void _elm_gesture_layer_efl_ui_widget_disabled_set(Eo *obj, Elm_Gesture_Layer_Data *pd, Eina_Bool disabled);


static Eina_Bool
_elm_gesture_layer_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_GESTURE_LAYER_EXTRA_OPS
#define ELM_GESTURE_LAYER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_zoom_step_set, _elm_gesture_layer_zoom_step_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_zoom_step_get, _elm_gesture_layer_zoom_step_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_tap_finger_size_set, _elm_gesture_layer_tap_finger_size_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_tap_finger_size_get, _elm_gesture_layer_tap_finger_size_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_hold_events_set, _elm_gesture_layer_hold_events_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_hold_events_get, _elm_gesture_layer_hold_events_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_rotate_step_set, _elm_gesture_layer_rotate_step_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_rotate_step_get, _elm_gesture_layer_rotate_step_get),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_cb_set, _elm_gesture_layer_cb_set),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_attach, _elm_gesture_layer_attach),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_cb_del, _elm_gesture_layer_cb_del),
      EFL_OBJECT_OP_FUNC(elm_obj_gesture_layer_cb_add, _elm_gesture_layer_cb_add),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_gesture_layer_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_disabled_set, _elm_gesture_layer_efl_ui_widget_disabled_set),
      ELM_GESTURE_LAYER_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"zoom_step", __eolian_elm_gesture_layer_zoom_step_set_reflect, __eolian_elm_gesture_layer_zoom_step_get_reflect},
      {"tap_finger_size", __eolian_elm_gesture_layer_tap_finger_size_set_reflect, __eolian_elm_gesture_layer_tap_finger_size_get_reflect},
      {"hold_events", __eolian_elm_gesture_layer_hold_events_set_reflect, __eolian_elm_gesture_layer_hold_events_get_reflect},
      {"rotate_step", __eolian_elm_gesture_layer_rotate_step_set_reflect, __eolian_elm_gesture_layer_rotate_step_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_gesture_layer_class_desc = {
   EO_VERSION,
   "Elm.Gesture_Layer",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Gesture_Layer_Data),
   _elm_gesture_layer_class_initializer,
   _elm_gesture_layer_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_gesture_layer_class_get, &_elm_gesture_layer_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_gesture_layer_eo.legacy.c"
