/**
 * Call this function to construct a new gesture-layer object.
 *
 * @param parent The gesture layer's parent widget.
 *
 * @return A new gesture layer object.
 *
 * This does not activate the gesture layer. You have to
 * call elm_gesture_layer_attach() in order to 'activate' gesture-layer.
 *
 */
EAPI Evas_Object *elm_gesture_layer_add(Evas_Object *parent);

/**
 * Call this function to get repeat-events settings.
 *
 * @param obj gesture-layer.
 *
 * @return repeat events settings.
 * @see elm_gesture_layer_hold_events_set()
 */
EAPI Eina_Bool    elm_gesture_layer_hold_events_get(const Evas_Object *obj);

/**
 * This function is to make gesture-layer repeat events.
 * Set this if you like to get the raw events only if gestures were not
 * detected.
 * Clear this if you like gesture layer to forward events as testing gestures.
 *
 * @param obj gesture layer.
 * @param hold_events hold events or not.
 *
 */
EAPI void         elm_gesture_layer_hold_events_set(Evas_Object *obj, Eina_Bool hold_events);

/**
 * This function returns step-value for zoom action.
 *
 * @param obj gesture-layer.
 * @return zoom step value.
 *
 * @see elm_gesture_layer_zoom_step_set()
 */
EAPI double       elm_gesture_layer_zoom_step_get(const Evas_Object *obj);

/**
 * This function sets step-value for zoom action.
 * Set step to any positive value.
 * Cancel step setting by setting to 0
 *
 * @param obj gesture-layer.
 * @param step new zoom step value.
 *
 * @see elm_gesture_layer_zoom_step_get()
 */
EAPI void         elm_gesture_layer_zoom_step_set(Evas_Object *obj, double step);
/**
 * This function returns step-value for rotate action.
 *
 * @param obj gesture-layer.
 * @return rotate step value.
 *
 */
EAPI double       elm_gesture_layer_rotate_step_get(const Evas_Object *obj);


/**
 * This function sets step-value for rotate action.
 * Set step to any positive value.
 * Cancel step setting by setting to 0
 *
 * @param obj gesture-layer.
 * @param step new rotate step value.
 *
 */
EAPI void         elm_gesture_layer_rotate_step_set(Evas_Object *obj, double step);

/**
 * Attach a given gesture layer widget to an Evas object, thus setting
 * the widget's @b target.
 *
 * @param obj A gesture layer to attach an object to.
 * @param target Object to attach to @a obj (target)
 *
 * A gesture layer target may be whichever Evas object one
 * chooses. This will be object @a obj will listen all mouse and key
 * events from, to report the gestures made upon it back.
 *
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise.
 *
 */
EAPI Eina_Bool    elm_gesture_layer_attach(Evas_Object *obj, Evas_Object *target);

/**
 * Use function to set callbacks to be notified about
 * change of state of gesture.
 * When a user registers a callback with this function
 * this means this gesture has to be tested.
 *
 * When ALL callbacks for a gesture are set to NULL
 * it means user isn't interested in gesture-state
 * and it will not be tested.
 *
 * @param obj gesture-layer.
 * @param idx The gesture you would like to track its state.
 * @param cb callback function pointer.
 * @param cb_type what event this callback tracks: START, MOVE, END, ABORT.
 * @param data user info to be sent to callback (usually, Smart Data)
 *
 */
EAPI void         elm_gesture_layer_cb_set(Evas_Object *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

/**
 * Use function to add callbacks to be notified about
 * change of state of gesture.
 * When a user registers a callback with this function
 * this means this gesture has to be tested.
 *
 * When ALL callbacks for a gesture are set to NULL
 * it means user isn't interested in gesture-state
 * and it will not be tested.
 *
 * If a function was already set for this gesture/type/state, it will be
 * replaced by the new one. For ABI compat, callbacks added by
 * elm_gesture_layer_cb_add will be removed. It is recommended to
 * use only one of these functions for a gesture object.
 *
 * @param obj gesture-layer.
 * @param idx The gesture you would like to track its state.
 * @param cb callback function pointer.
 * @param cb_type what event this callback tracks: START, MOVE, END, ABORT.
 * @param data user info to be sent to callback (usually, Smart Data)
 *
 */
EAPI void elm_gesture_layer_cb_add(Evas_Object *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

/**
 * Use this function to remove a callback that has been added
 * to be notified about change of state of gesture.
 *
 * @param obj gesture-layer.
 * @param idx The gesture you would like to track its state.
 * @param cb callback function pointer.
 * @param cb_type what event this callback tracks: START, MOVE, END, ABORT.
 * @param data user info for the callback (usually, Smart Data)
 *
 */
EAPI void elm_gesture_layer_cb_del(Evas_Object *obj, Elm_Gesture_Type idx, Elm_Gesture_State cb_type, Elm_Gesture_Event_Cb cb, void *data);

/**
 * @since 1.8
 * This function sets the gesture layer finger-size for taps
 * If not set, this size taken from elm_config.
 * Set to ZERO if you want GLayer to use system finger size value (default)
 *
 * @param obj gesture-layer.
 * @param fsize Finger size
 *
 */
EAPI void elm_gesture_layer_tap_finger_size_set(Evas_Object *obj, Evas_Coord sz);

/**
 * @since 1.8
 * This function returns the gesture layer finger-size for taps
 *
 * @param obj gesture-layer.
 * @return Finger size that is currently used by Gesture Layer for taps.
 *
 */
EAPI Evas_Coord elm_gesture_layer_tap_finger_size_get(const Evas_Object *obj);

/**
 * @since 1.8
 * This function adds a callback called during Tap + Long Tap sequence.
 *
 * @param state state for the callback to add.
 * @param cb callback pointer
 * @param data user data for the callback.
 *
 * The callbacks will be called as followed:
 * - start cbs on single tap start
 * - move cbs on long press move
 * - end cbs on long press end
 * - abort cbs whenever in the sequence. The event info will be NULL, because it
 *   can be triggered from multiple events (timer expired, abort single/long taps).
 *
 * You can remove the callbacks by using elm_gesture_layer_tap_longpress_cb_del.
 *
 * @see elm_gesture_layer_tap_longpress_cb_del
 */
EAPI void elm_gesture_layer_tap_longpress_cb_add(Evas_Object *obj, Elm_Gesture_State state, Elm_Gesture_Event_Cb cb, void *data);

/**
 * @since 1.8
 * This function removes a callback called during Tap + Long Tap sequence.
 *
 * @param state state for the callback to add.
 * @param cb callback pointer
 * @param data user data for the callback.
 *
 * The internal data used for the sequence will be freed ONLY when all the
 * callbacks added via elm_gesture_layer_tap_longpress_cb_add are removed by
 * this function.
 *
 * @see elm_gesture_layer_tap_longpress_cb_add
 */
EAPI void elm_gesture_layer_tap_longpress_cb_del(Evas_Object *obj, Elm_Gesture_State state, Elm_Gesture_Event_Cb cb, void *data);
