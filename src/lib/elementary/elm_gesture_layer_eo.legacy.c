
EAPI void
elm_gesture_layer_zoom_step_set(Elm_Gesture_Layer *obj, double step)
{
   elm_obj_gesture_layer_zoom_step_set(obj, step);
}

EAPI double
elm_gesture_layer_zoom_step_get(const Elm_Gesture_Layer *obj)
{
   return elm_obj_gesture_layer_zoom_step_get(obj);
}

EAPI void
elm_gesture_layer_tap_finger_size_set(Elm_Gesture_Layer *obj, int sz)
{
   elm_obj_gesture_layer_tap_finger_size_set(obj, sz);
}

EAPI int
elm_gesture_layer_tap_finger_size_get(const Elm_Gesture_Layer *obj)
{
   return elm_obj_gesture_layer_tap_finger_size_get(obj);
}

EAPI void
elm_gesture_layer_hold_events_set(Elm_Gesture_Layer *obj, Eina_Bool hold_events)
{
   elm_obj_gesture_layer_hold_events_set(obj, hold_events);
}

EAPI Eina_Bool
elm_gesture_layer_hold_events_get(const Elm_Gesture_Layer *obj)
{
   return elm_obj_gesture_layer_hold_events_get(obj);
}

EAPI void
elm_gesture_layer_rotate_step_set(Elm_Gesture_Layer *obj, double step)
{
   elm_obj_gesture_layer_rotate_step_set(obj, step);
}

EAPI double
elm_gesture_layer_rotate_step_get(const Elm_Gesture_Layer *obj)
{
   return elm_obj_gesture_layer_rotate_step_get(obj);
}

EAPI void
elm_gesture_layer_cb_set(Elm_Gesture_Layer *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   elm_obj_gesture_layer_cb_set(obj, idx, cb_type, cb, data);
}

EAPI Eina_Bool
elm_gesture_layer_attach(Elm_Gesture_Layer *obj, Efl_Canvas_Object *target)
{
   return elm_obj_gesture_layer_attach(obj, target);
}

EAPI void
elm_gesture_layer_cb_del(Elm_Gesture_Layer *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   elm_obj_gesture_layer_cb_del(obj, idx, cb_type, cb, data);
}

EAPI void
elm_gesture_layer_cb_add(Elm_Gesture_Layer *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data)
{
   elm_obj_gesture_layer_cb_add(obj, idx, cb_type, cb, data);
}
