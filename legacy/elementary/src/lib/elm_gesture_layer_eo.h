#define ELM_OBJ_GESTURE_LAYER_CLASS elm_obj_gesture_layer_class_get()

const Eo_Class *elm_obj_gesture_layer_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_GESTURE_LAYER_BASE_ID;

enum
{
   ELM_OBJ_GESTURE_LAYER_SUB_ID_HOLD_EVENTS_GET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_HOLD_EVENTS_SET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_ZOOM_STEP_GET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_ZOOM_STEP_SET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_ROTATE_STEP_GET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_ROTATE_STEP_SET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_ATTACH,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_CB_SET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_TAP_FINGER_SIZE_SET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_TAP_FINGER_SIZE_GET,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_CB_ADD,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_CB_DEL,
   ELM_OBJ_GESTURE_LAYER_SUB_ID_LAST
};

#define ELM_OBJ_GESTURE_LAYER_ID(sub_id) (ELM_OBJ_GESTURE_LAYER_BASE_ID + sub_id)


/**
 * @def elm_obj_gesture_layer_hold_events_get
 * @since 1.8
 *
 * Call this function to get repeat-events settings.
 *
 * @param[out] ret
 *
 * @see elm_gesture_layer_hold_events_get
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_hold_events_get(ret) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_HOLD_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gesture_layer_hold_events_set
 * @since 1.8
 *
 * This function is to make gesture-layer repeat events.
 *
 * @param[in] hold_events
 *
 * @see elm_gesture_layer_hold_events_set
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_hold_events_set(hold_events) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_HOLD_EVENTS_SET), EO_TYPECHECK(Eina_Bool, hold_events)

/**
 * @def elm_obj_gesture_layer_zoom_step_get
 * @since 1.8
 *
 * This function returns step-value for zoom action.
 *
 * @param[out] ret
 *
 * @see elm_gesture_layer_zoom_step_get
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_zoom_step_get(ret) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_ZOOM_STEP_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_gesture_layer_zoom_step_set
 * @since 1.8
 *
 * This function sets step-value for zoom action.
 *
 * @param[in] step
 *
 * @see elm_gesture_layer_zoom_step_set
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_zoom_step_set(step) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_ZOOM_STEP_SET), EO_TYPECHECK(double, step)

/**
 * @def elm_obj_gesture_layer_rotate_step_get
 * @since 1.8
 *
 * This function returns step-value for rotate action.
 *
 * @param[out] ret
 *
 * @see elm_gesture_layer_rotate_step_get
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_rotate_step_get(ret) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_ROTATE_STEP_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_gesture_layer_rotate_step_set
 * @since 1.8
 *
 * This function sets step-value for rotate action.
 *
 * @param[in] step
 *
 * @see elm_gesture_layer_rotate_step_set
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_rotate_step_set(step) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_ROTATE_STEP_SET), EO_TYPECHECK(double, step)

/**
 * @def elm_obj_gesture_layer_attach
 * @since 1.8
 *
 * Attach a given gesture layer widget to an Evas object, thus setting
 * the widget's @b target.
 *
 * @param[in] target
 * @param[out] ret
 *
 * @see elm_gesture_layer_attach
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_attach(target, ret) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_ATTACH), EO_TYPECHECK(Evas_Object *, target), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gesture_layer_cb_set
 * @since 1.8
 *
 * Use function to set callbacks to be notified about
 * change of state of gesture. If a function was already
 * set for this gesture/type/state, it will be replaced by the new one.
 * For ABI compat, callbacks added by elm_obj_gesture_layer_cb_add will be removed. It is recommended
 * to use only one of these functions for a gesture object.
 *
 * @param[in] idx
 * @param[in] cb_type
 * @param[in] cb
 * @param[in] data
 *
 * @see elm_gesture_layer_cb_set
 * @see elm_gesture_layer_cb_add
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_cb_set(idx, cb_type, cb, data) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_CB_SET), EO_TYPECHECK(Elm_Gesture_Type, idx), EO_TYPECHECK(Elm_Gesture_State, cb_type), EO_TYPECHECK(Elm_Gesture_Event_Cb, cb), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_gesture_layer_cb_add
 * @since 1.8
 *
 * Use function to add a callback to be notified about
 * change of state of gesture.
 *
 * @param[in] idx
 * @param[in] cb_type
 * @param[in] cb
 * @param[in] data
 *
 * @see elm_gesture_layer_cb_add
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_cb_add(idx, cb_type, cb, data) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_CB_ADD), EO_TYPECHECK(Elm_Gesture_Type, idx), EO_TYPECHECK(Elm_Gesture_State, cb_type), EO_TYPECHECK(Elm_Gesture_Event_Cb, cb), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_gesture_layer_cb_del
 * @since 1.8
 *
 * Use function to remove a callback that has been added
 * to be notified about change of state of gesture.
 *
 * @param[in] idx
 * @param[in] cb_type
 * @param[in] cb
 * @param[in] data
 *
 * @see elm_gesture_layer_cb_del
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_cb_del(idx, cb_type, cb, data) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_CB_DEL), EO_TYPECHECK(Elm_Gesture_Type, idx), EO_TYPECHECK(Elm_Gesture_State, cb_type), EO_TYPECHECK(Elm_Gesture_Event_Cb, cb), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_gesture_layer_tap_finger_size_set
 * @since 1.8
 *
 * This function sets the gesture layer finger-size for taps
 * If not set, this size is taken from elm_config.
 * Set to ZERO if you want GLayer to use system finger size value (default)
 * It is recommended to not set a too much big or little value to avoid weird
 * behaviors.
 *
 * @param[in] fsize Finger size
 *
 * @see elm_gesture_layer_tap_finger_size_get
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_tap_finger_size_set(sz) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_TAP_FINGER_SIZE_SET), EO_TYPECHECK(Evas_Coord, sz)

/**
 * @def elm_obj_gesture_layer_tap_finger_size_get
 * @since 1.8
 *
 * This function returns the gesture layer finger-size for taps
 *
 * @param[out] ret
 * @return Finger size that is currently used by Gesture Layer for taps.
 *
 * @see elm_gesture_layer_tap_finger_size_set
 *
 * @ingroup Elm_Gesture_Layer
 */
#define elm_obj_gesture_layer_tap_finger_size_get(ret) ELM_OBJ_GESTURE_LAYER_ID(ELM_OBJ_GESTURE_LAYER_SUB_ID_TAP_FINGER_SIZE_GET), EO_TYPECHECK(Evas_Coord *, ret)
