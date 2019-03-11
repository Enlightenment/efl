#ifndef _ELM_GESTURE_LAYER_EO_LEGACY_H_
#define _ELM_GESTURE_LAYER_EO_LEGACY_H_

#ifndef _ELM_GESTURE_LAYER_EO_CLASS_TYPE
#define _ELM_GESTURE_LAYER_EO_CLASS_TYPE

typedef Eo Elm_Gesture_Layer;

#endif

#ifndef _ELM_GESTURE_LAYER_EO_TYPES
#define _ELM_GESTURE_LAYER_EO_TYPES

/** Enum of supported gesture types.
 *
 * @ingroup Elm_Gesture
 */
typedef enum
{
  ELM_GESTURE_FIRST = 0, /**< First type used for iteration over the enum */
  ELM_GESTURE_N_TAPS, /**< N fingers single taps */
  ELM_GESTURE_N_LONG_TAPS, /**< N fingers single long-taps */
  ELM_GESTURE_N_DOUBLE_TAPS, /**< N fingers double-single taps */
  ELM_GESTURE_N_TRIPLE_TAPS, /**< N fingers triple-single taps */
  ELM_GESTURE_MOMENTUM, /**< Reports momentum in the direction of move */
  ELM_GESTURE_N_LINES, /**< N fingers line gesture */
  ELM_GESTURE_N_FLICKS, /**< N fingers flick gesture */
  ELM_GESTURE_ZOOM, /**< Zoom */
  ELM_GESTURE_ROTATE, /**< Rotate */
  ELM_GESTURE_LAST /**< Sentinel value to indicate last enum field during
                    * iteration */
} Elm_Gesture_Type;

/** Enum of gesture states.
 *
 * @ingroup Elm_Gesture
 */
typedef enum
{
  ELM_GESTURE_STATE_UNDEFINED = -1 /* +1 */, /**< Gesture not started */
  ELM_GESTURE_STATE_START, /**< Gesture started */
  ELM_GESTURE_STATE_MOVE, /**< Gesture is ongoing */
  ELM_GESTURE_STATE_END, /**< Gesture completed */
  ELM_GESTURE_STATE_ABORT /**< Ongoing gesture was aborted */
} Elm_Gesture_State;


#endif

/**
 * @brief Control step value for zoom action.
 *
 * When recognizing the zoom gesture, it should be recognized as zooming larger
 * than step.
 *
 * @param[in] obj The object.
 * @param[in] step The zoom step value.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_zoom_step_set(Elm_Gesture_Layer *obj, double step);

/**
 * @brief Control step value for zoom action.
 *
 * @param[in] obj The object.
 *
 * @return The zoom step value.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI double elm_gesture_layer_zoom_step_get(const Elm_Gesture_Layer *obj);

/**
 * @brief This function sets the gesture layer finger-size for taps.
 *
 * If not set, it's taken from elm_config. Set to 0 if you want GLayer to use
 * the system finger size value (default).
 *
 * @param[in] obj The object.
 * @param[in] sz The finger size.
 *
 * @since 1.8
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_tap_finger_size_set(Elm_Gesture_Layer *obj, int sz);

/**
 * @brief This function returns the gesture layer finger-size for taps
 *
 * @param[in] obj The object.
 *
 * @return The finger size.
 *
 * @since 1.8
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI int elm_gesture_layer_tap_finger_size_get(const Elm_Gesture_Layer *obj);

/**
 * @brief This function makes gesture-layer repeat events.
 *
 * Set this if you like to get the raw events only if gestures were not
 * detected.
 *
 * Clear this if you like gesture layer to forward events as testing gestures.
 *
 * @param[in] obj The object.
 * @param[in] hold_events If @c true get events only if gesture was not
 * detected, @c false otherwise
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_hold_events_set(Elm_Gesture_Layer *obj, Eina_Bool hold_events);

/**
 * @brief Get the repeat-events setting.
 *
 * @param[in] obj The object.
 *
 * @return If @c true get events only if gesture was not detected, @c false
 * otherwise
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI Eina_Bool elm_gesture_layer_hold_events_get(const Elm_Gesture_Layer *obj);

/**
 * @brief This function returns step-value for rotate action.
 *
 * Set to 0 to cancel step setting.
 *
 * When recognizing the rotate gesture, the rotated size must be larger than
 * step.
 *
 * @param[in] obj The object.
 * @param[in] step New rotate step value.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_rotate_step_set(Elm_Gesture_Layer *obj, double step);

/**
 * @brief This function returns step-value for rotate action.
 *
 * @param[in] obj The object.
 *
 * @return New rotate step value.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI double elm_gesture_layer_rotate_step_get(const Elm_Gesture_Layer *obj);

/**
 * @brief Set the gesture state change callback.
 *
 * When all callbacks for the gesture are set to @c null, it means this gesture
 * is disabled.
 *
 * @param[in] obj The object.
 * @param[in] idx The gesture you want to track state of.
 * @param[in] cb_type The event the callback tracks (START, MOVE, END, ABORT).
 * @param[in] cb The callback itself.
 * @param[in] data Custom data to be passed.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_cb_set(Elm_Gesture_Layer *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

/**
 * @brief Attach a gesture layer widget to an Evas object (setting the widget's
 * target).
 *
 * A gesture layer's target may be any Evas object. This object will be used to
 * listen to mouse and key events.
 *
 * @param[in] obj The object.
 * @param[in] target The object to attach.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI Eina_Bool elm_gesture_layer_attach(Elm_Gesture_Layer *obj, Efl_Canvas_Object *target);

/**
 * @brief Remove a gesture callback.
 *
 * @param[in] obj The object.
 * @param[in] idx The gesture you want to track state of.
 * @param[in] cb_type The event the callback tracks (START, MOVE, END, ABORT).
 * @param[in] cb The callback itself.
 * @param[in] data Custom callback data.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_cb_del(Elm_Gesture_Layer *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

/**
 * @brief Add a gesture state change callback.
 *
 * When all callbacks for the gesture are set to @c null, it means this gesture
 * is disabled.
 *
 * If a function was already set for this gesture/type/state, it will be
 * replaced by the new one. For ABI compat, callbacks added by
 * @ref elm_gesture_layer_cb_add will be removed. It is recommended to use only
 * one of these functions for a gesture object.
 *
 * @param[in] obj The object.
 * @param[in] idx The gesture you want to track state of.
 * @param[in] cb_type The event the callback tracks (START, MOVE, END, ABORT).
 * @param[in] cb The callback itself.
 * @param[in] data Custom data to be passed.
 *
 * @ingroup Elm_Gesture_Layer_Group
 */
EAPI void elm_gesture_layer_cb_add(Elm_Gesture_Layer *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

#endif
