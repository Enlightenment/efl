/**
 * @defgroup Transit Transit
 * @ingroup Elementary
 *
 * Transit is designed to apply various animated transition effects to @c
 * Evas_Object, such like translation, rotation, etc. For using these
 * effects, create an @ref Elm_Transit and add the desired transition effects.
 *
 * Once the effects are added into transit, they will be automatically
 * managed (their callback will be called for the set duration and
 * they will be deleted upon completion).
 *
 * Example:
 * @code
 * Elm_Transit *trans = elm_transit_add();
 * elm_transit_object_add(trans, obj);
 * elm_transit_effect_translation_add(trans, 0, 0, 280, 280
 * elm_transit_duration_set(transit, 1);
 * elm_transit_auto_reverse_set(transit, EINA_TRUE);
 * elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
 * elm_transit_repeat_times_set(transit, 3);
 * @endcode
 *
 * Some transition effects are used to change the properties of objects. They
 * are:
 * @li @ref elm_transit_effect_translation_add
 * @li @ref elm_transit_effect_color_add
 * @li @ref elm_transit_effect_rotation_add
 * @li @ref elm_transit_effect_wipe_add
 * @li @ref elm_transit_effect_zoom_add
 * @li @ref elm_transit_effect_resizing_add
 *
 * Other transition effects are used to make one object disappear and another
 * object appear on its place. These effects are:
 *
 * @li @ref elm_transit_effect_flip_add
 * @li @ref elm_transit_effect_resizable_flip_add
 * @li @ref elm_transit_effect_fade_add
 * @li @ref elm_transit_effect_blend_add
 *
 * It's also possible to make a transition chain with @ref
 * elm_transit_chain_transit_add.
 *
 * @warning We strongly recommend to use elm_transit just when edje can not do
 * the trick. Edje is better at handling transitions than Elm_Transit.
 * Edje has more flexibility and animations can be manipulated inside the theme.
 *
 * List of examples:
 * @li @ref transit_example_01_explained
 * @li @ref transit_example_02_explained
 * @li @ref transit_example_03_c
 * @li @ref transit_example_04_c
 *
 * @{
 */

/**
 * @enum Elm_Transit_Tween_Mode
 *
 * The type of acceleration used in the transition.
 */
typedef enum
{
   ELM_TRANSIT_TWEEN_MODE_LINEAR, /**< Constant speed */
   ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL, /**< Starts slow, increase speed
                                         over time, then decrease again
                                         and stop slowly, v1 being a power factor */
   ELM_TRANSIT_TWEEN_MODE_DECELERATE, /**< Starts fast and decrease
                                         speed over time, v1 being a power factor */
   ELM_TRANSIT_TWEEN_MODE_ACCELERATE, /**< Starts slow and increase speed
                                         over time, v1 being a power factor */
   ELM_TRANSIT_TWEEN_MODE_DIVISOR_INTERP, /**< Start at gradient v1,
                                             interpolated via power of v2 curve */
   ELM_TRANSIT_TWEEN_MODE_BOUNCE, /**< Start at 0.0 then "drop" like a ball
                                     bouncing to the ground at 1.0, and
                                     bounce v2 times, with decay factor of v1 */
   ELM_TRANSIT_TWEEN_MODE_SPRING /**< Start at 0.0 then "wobble" like a spring
                                    rest position 1.0, and wobble v2 times,
                                    with decay factor of v1 */
} Elm_Transit_Tween_Mode;

/**
 * @enum Elm_Transit_Effect_Flip_Axis
 *
 * The axis along which flip effect should be applied.
 */
typedef enum
{
   ELM_TRANSIT_EFFECT_FLIP_AXIS_X, /**< Flip on X axis */
   ELM_TRANSIT_EFFECT_FLIP_AXIS_Y /**< Flip on Y axis */
} Elm_Transit_Effect_Flip_Axis;

/**
 * @enum Elm_Transit_Effect_Wipe_Dir
 *
 * The direction in which the wipe effect should occur.
 */
typedef enum
{
   ELM_TRANSIT_EFFECT_WIPE_DIR_LEFT, /**< Wipe to the left */
   ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT, /**< Wipe to the right */
   ELM_TRANSIT_EFFECT_WIPE_DIR_UP, /**< Wipe up */
   ELM_TRANSIT_EFFECT_WIPE_DIR_DOWN /**< Wipe down */
} Elm_Transit_Effect_Wipe_Dir;

/** @enum Elm_Transit_Effect_Wipe_Type
 *
 * Whether the wipe effect should show or hide the object.
 */
typedef enum
{
   ELM_TRANSIT_EFFECT_WIPE_TYPE_HIDE, /**< Hide the object during the
                                         animation */
   ELM_TRANSIT_EFFECT_WIPE_TYPE_SHOW /**< Show the object during the
                                        animation */
} Elm_Transit_Effect_Wipe_Type;

/**
 * @typedef Elm_Transit
 *
 * The Transit created with elm_transit_add(). This type has the information
 * about the objects which the transition will be applied, and the
 * transition effects that will be used. It also contains info about
 * duration, number of repetitions, auto-reverse, etc.
 */
typedef struct _Elm_Transit Elm_Transit;
typedef void                Elm_Transit_Effect;

/**
 * @typedef Elm_Transit_Effect_Transition_Cb
 *
 * Transition callback called for this effect on each transition iteration.
 */
typedef void (*Elm_Transit_Effect_Transition_Cb)(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress);

/**
 * Elm_Transit_Effect_End_Cb
 *
 * Transition callback called for this effect when the transition is over.
 */
typedef void (*Elm_Transit_Effect_End_Cb)(Elm_Transit_Effect *effect, Elm_Transit *transit);

/**
 * Elm_Transit_Del_Cb
 *
 * A callback called when the transit is deleted.
 */
typedef void (*Elm_Transit_Del_Cb)(void *data, Elm_Transit *transit);

/**
 * Create new transit.
 *
 * @note It is not necessary to delete the transit object, it will be deleted at
 * the end of its operation.
 * @note The transit will start playing when the program enters the main loop.
 *
 * @return The transit object.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit           *elm_transit_add(void);

/**
 * Stops the animation and delete the @p transit object.
 *
 * Call this function if you want to stop the animation before the
 * transit time. Make sure the @p transit object is still alive with
 * elm_transit_del_cb_set() function.
 * All added effects will be deleted, calling its respective data_free_cb
 * functions. The function set by elm_transit_del_cb_set() will be called.
 *
 * @see elm_transit_del_cb_set()
 *
 * @param transit The transit object to be deleted.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_del(Elm_Transit *transit);

/**
 * Add a new effect to the transit.
 *
 * @note The cb function and the data are the key to the effect.
 * If you try to add an existing effect, nothing is done.
 * @note After the first addition of an effect to @p transit, if its
 * effect list become empty again, the @p transit will be killed by
 * elm_transit_del(transit) function.
 *
 * Example:
 * @code
 * Elm_Transit *transit = elm_transit_add();
 * elm_transit_effect_add(transit,
 *                        elm_transit_effect_blend_op,
 *                        elm_transit_effect_blend_context_new(),
 *                        elm_transit_effect_blend_context_free);
 * @endcode
 *
 * @param transit The transit object.
 * @param transition_cb The operation function. It is called when the
 * animation begins, it is the function that actually performs the animation.
 * It is called with the @p data, @p transit and the time progression of the
 * animation (a double value between 0.0 and 1.0).
 * @param effect The context data of the effect.
 * @param end_cb The function to free the context data, it will be called
 * at the end of the effect, it must finalize the animation and free the
 * @p data.
 *
 * @ingroup Transit
 * @warning The transit will free the context data at the and of the
 * transition with the data_free_cb function.
 * Do not share the context data in between different transit objects.
 */
EAPI void                   elm_transit_effect_add(Elm_Transit *transit, Elm_Transit_Effect_Transition_Cb transition_cb, Elm_Transit_Effect *effect, Elm_Transit_Effect_End_Cb end_cb);

/**
 * Delete an added effect.
 *
 * This function will remove the effect from the @p transit, calling the
 * data_free_cb to free the @p data.
 *
 * @see elm_transit_effect_add()
 *
 * @note If the effect is not found, nothing is done.
 * @note If the effect list become empty, this function will call
 * elm_transit_del(transit), i.e., it will kill the @p transit.
 *
 * @param transit The transit object.
 * @param transition_cb The operation function.
 * @param effect The context data of the effect.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_effect_del(Elm_Transit *transit, Elm_Transit_Effect_Transition_Cb transition_cb, Elm_Transit_Effect *effect);

/**
 * Add new object to apply the effects.
 *
 * @note After the first addition of an object to @p transit, if its
 * object list become empty again, the @p transit will be killed by
 * elm_transit_del(transit) function.
 * @note If the @p obj belongs to another transit, the @p obj will be
 * removed from it and it will only belong to the other @p transit.
 * If the old transit stays without objects, it will die.
 * @note When you add an object into the @p transit, its state from
 * evas_object_pass_events_get(obj) is saved, and it is applied when the
 * transit ends, if you change this state with evas_object_pass_events_set()
 * after add the object, this state will change again when @p transit stops.
 *
 * @param transit The transit object.
 * @param obj Object to be animated.
 *
 * @ingroup Transit
 * @warning It is not allowed to add a new object after transit begins.
 */
EAPI void                   elm_transit_object_add(Elm_Transit *transit, Evas_Object *obj);

/**
 * Removes an added object from the transit.
 *
 * @note If the @p obj is not in the @p transit, nothing is done.
 * @note If the list become empty, this function will call
 * elm_transit_del(transit), i.e., it will kill the @p transit.
 *
 * @param transit The transit object.
 * @param obj Object to be removed from @p transit.
 *
 * @ingroup Transit
 * @warning It is not allowed to remove objects after transit begins.
 */
EAPI void                   elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj);

/**
 * Get the objects of the transit.
 *
 * @param transit The transit object.
 * @return a Eina_List with the objects from the transit.
 *
 * @ingroup Transit
 */
EAPI const Eina_List       *elm_transit_objects_get(const Elm_Transit *transit);

/**
 * Enable/disable keeping up the objects states.
 * If it is not kept, the objects states will be reset when transition ends.
 *
 * @note @p transit can not be NULL.
 * @note One state includes geometry, color, map data.
 *
 * @param transit The transit object.
 * @param state_keep retain the state or not.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_objects_final_state_keep_set(Elm_Transit *transit, Eina_Bool state_keep);

/**
 * Get a value whether the objects states will be reset or not.
 *
 * @note @p transit can not be NULL
 *
 * @see elm_transit_objects_final_state_keep_set()
 *
 * @param transit The transit object.
 * @return EINA_TRUE means the states of the objects will be reset.
 * If @p transit is NULL, EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool              elm_transit_objects_final_state_keep_get(const Elm_Transit *transit);

/**
 * Set the event enabled when transit is operating.
 *
 * If @p enabled is EINA_TRUE, the objects of the transit will receive
 * events from mouse and keyboard during the animation.
 * @note When you add an object with elm_transit_object_add(), its state from
 * evas_object_freeze_events_get(obj) is saved, and it is applied when the
 * transit ends. If you change this state with evas_object_freeze_events_set()
 * after adding the object, this state will change again when @p transit stops
 * to run.
 *
 * @param transit The transit object.
 * @param enabled Events are received when enabled is @c EINA_TRUE, and
 * ignored otherwise.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_event_enabled_set(Elm_Transit *transit, Eina_Bool enabled);

/**
 * Get the value of event enabled status.
 *
 * @see elm_transit_event_enabled_set()
 *
 * @param transit The Transit object
 * @return EINA_TRUE, when event is enabled. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool              elm_transit_event_enabled_get(const Elm_Transit *transit);

/**
 * Set the user-callback function when the transit is deleted.
 *
 * @note Using this function twice will overwrite the first function set.
 * @note the @p transit object will be deleted after call @p cb function.
 *
 * @param transit The transit object.
 * @param cb Callback function pointer. This function will be called before
 * the deletion of the transit.
 * @param data Callback function user data. It is the @p op parameter.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_del_cb_set(Elm_Transit *transit, Elm_Transit_Del_Cb cb, void *data);

/**
 * Set reverse effect automatically.
 *
 * If auto reverse is set, after running the effects with the progress
 * parameter from 0 to 1, it will call the effects again with the progress
 * from 1 to 0. The transit will last for a time equal to (2 * duration * repeat),
 * where the duration was set with the function elm_transit_add and
 * the repeat with the function elm_transit_repeat_times_set().
 *
 * @param transit The transit object.
 * @param reverse EINA_TRUE means the auto_reverse is on.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_auto_reverse_set(Elm_Transit *transit, Eina_Bool reverse);

/**
 * Get if the auto reverse is on.
 *
 * @see elm_transit_auto_reverse_set()
 *
 * @param transit The transit object.
 * @return EINA_TRUE means auto reverse is on. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool              elm_transit_auto_reverse_get(const Elm_Transit *transit);

/**
 * Set the transit repeat count. Effect will be repeated by repeat count.
 *
 * This function sets the number of repetition the transit will run after
 * the first one, i.e., if @p repeat is 1, the transit will run 2 times.
 * If the @p repeat is a negative number, it will repeat infinite times.
 *
 * @note If this function is called during the transit execution, the transit
 * will run @p repeat times, ignoring the times it already performed.
 *
 * @param transit The transit object
 * @param repeat Repeat count
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_repeat_times_set(Elm_Transit *transit, int repeat);

/**
 * Get the transit repeat count.
 *
 * @see elm_transit_repeat_times_set()
 *
 * @param transit The Transit object.
 * @return The repeat count. If @p transit is NULL
 * 0 is returned
 *
 * @ingroup Transit
 */
EAPI int                    elm_transit_repeat_times_get(const Elm_Transit *transit);

/**
 * Set the transit animation acceleration type.
 *
 * This function sets the tween mode of the transit that can be:
 * ELM_TRANSIT_TWEEN_MODE_LINEAR - The default mode.
 * ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL - Starts in accelerate mode and ends
 * decelerating with factor.
 * ELM_TRANSIT_TWEEN_MODE_DECELERATE - The animation will be slowed over time
 * with factor.
 * ELM_TRANSIT_TWEEN_MODE_ACCELERATE - The animation will accelerate over time
 * with factor.
 * ELM_TRANSIT_TWEEN_MODE_DIVISOR_INTERP - Start at gradient v1, interpolated
 * via power of v2 curve.
 * ELM_TRANSIT_TWEEN_MODE_BOUNCE - Start at 0.0 then "drop" like a ball bouncing
 * to the ground at 1.0, and bounce v2 times, with decay factor of v1.
 * ELM_TRANSIT_TWEEN_MODE_SPRING - Start at 0.0 then "wobble" like a spring rest
 * position 1.0, and wobble v2 times, with decay factor of v1.
 *
 * @param transit The transit object.
 * @param tween_mode The tween type.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_tween_mode_set(Elm_Transit *transit, Elm_Transit_Tween_Mode tween_mode);

/**
 * Get the transit animation acceleration type.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @return The tween type. If @p transit is NULL
 * ELM_TRANSIT_TWEEN_MODE_LINEAR is returned.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Tween_Mode elm_transit_tween_mode_get(const Elm_Transit *transit);

/**
 * Set the transit animation acceleration factor.
 *
 * This function sets the tween mode factor of the transit that can be:
 * If you use the below tween modes, you have to set the factor using this API.
 * ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL - Start slow, speed up then slow down
 * at end, v1 being a power factor, 0.0 being linear, 1.0 being
 * ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL default, 2.0 being much more pronounced
 * sinusoidal(squared), 3.0 being cubed, etc.
 * ELM_TRANSIT_TWEEN_MODE_DECELERATE - Start fast then slow down, v1 being a
 * power factor, 0.0 being linear, 1.0 being ELM_TRANSIT_TWEEN_MODE_DECELERATE
 * default, 2.0 being much more pronounced decelerate (squared), 3.0 being
 * cubed, etc.
 * ELM_TRANSIT_TWEEN_MODE_ACCELERATE - Start slow then speed up, v1 being a
 * power factor, 0.0 being linear, 1.0 being ELM_TRANSIT_TWEEN_MODE_ACCELERATE
 * default, 2.0 being much more pronounced accelerate (squared), 3.0 being
 * cubed, etc.
 * ELM_TRANSIT_TWEEN_MODE_DIVISOR_INTERP - Start at gradient * v1, interpolated
 * via power of v2 curve
 * ELM_TRANSIT_TWEEN_MODE_BOUNCE - Start at 0.0 then "drop" like a ball bouncing
 * to the ground at 1.0, and bounce v2 times, with decay factor of v1
 * ELM_TRANSIT_TWEEN_MODE_SPRING - Start at 0.0 then "wobble" like a spring rest
 * position 1.0, and wobble v2 times, with decay factor of v1
 *
 * @param transit The transit object.
 * @param v1 A parameter use by the mapping (default is 1.0)
 * @param v2 A parameter use by the mapping (default is 0.0)
 *
 * @see elm_transit_tween_mode_factor_get()
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_tween_mode_factor_set(Elm_Transit *transit, double v1, double v2);

/**
 * Get the transit animation acceleration factor.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @param v1      Pointer to an double in which to store the factor value.
 * @param v2      Pointer to an double in which to store the factor value2.
 *
 * @see elm_transit_tween_mode_factor_set()
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_tween_mode_factor_get(const Elm_Transit *transit, double *v1, double *v2);

/**
 * Set the transit animation time
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @param duration The animation time.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_duration_set(Elm_Transit *transit, double duration);

/**
 * Get the transit animation time
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @return The transit animation time.
 *
 * @ingroup Transit
 */
EAPI double                 elm_transit_duration_get(const Elm_Transit *transit);

/**
 * Starts the transition.
 * Once this API is called, the transit begins to measure the time.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_go(Elm_Transit *transit);

/**
 * Pause/Resume the transition.
 *
 * If you call elm_transit_go again, the transit will be started from the
 * beginning, and will be played.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @param paused Whether the transition should be paused or not.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_paused_set(Elm_Transit *transit, Eina_Bool paused);

/**
 * Get the value of paused status.
 *
 * @see elm_transit_paused_set()
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @return EINA_TRUE means transition is paused. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool              elm_transit_paused_get(const Elm_Transit *transit);

/**
 * Get the time progression of the animation (a double value between 0.0 and 1.0).
 *
 * The value returned is a fraction (current time / total time). It
 * represents the progression position relative to the total.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @return The time progression value. If @p transit is NULL
 * 0 is returned
 *
 * @ingroup Transit
 */
EAPI double                 elm_transit_progress_value_get(const Elm_Transit *transit);

/**
 * Makes the chain relationship between two transits.
 *
 * @note @p transit can not be NULL. Transit would have multiple chain transits.
 * @note @p chain_transit can not be NULL. Chain transits could be chained to the only one transit.
 *
 * @param transit The transit object.
 * @param chain_transit The chain transit object. This transit will be operated
 *        after transit is done.
 *
 * This function adds @p chain_transit transition to a chain after the @p
 * transit, and will be started as soon as @p transit ends. See @ref
 * transit_example_02_explained for a full example.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_chain_transit_add(Elm_Transit *transit, Elm_Transit *chain_transit);

/**
 * Cut off the chain relationship between two transits.
 *
 * @note @p transit can not be NULL. Transit would have the chain relationship with @p chain transit.
 * @note @p chain_transit can not be NULL. Chain transits should be chained to the @p transit.
 *
 * @param transit The transit object.
 * @param chain_transit The chain transit object.
 *
 * This function remove the @p chain_transit transition from the @p transit.
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_chain_transit_del(Elm_Transit *transit, Elm_Transit *chain_transit);

/**
 * Get the current chain transit list.
 *
 * @note @p transit can not be NULL.
 *
 * @param transit The transit object.
 * @return chain transit list.
 *
 * @ingroup Transit
 */
EAPI Eina_List             *elm_transit_chain_transits_get(const Elm_Transit *transit);

/**
 * Set the smooth effect for a transit.
 *
 * @param obj The transit object
 * @param enabled enable or disable smooth map rendering
 *
 * This sets smoothing for transit map rendering. If the object added in a
 * transit is a type that has its own smoothing settings, then both the smooth
 * settings for this object and the map must be turned off. By default smooth
 * maps are enabled.
 *
 * @see evas_map_smooth_set()
 * @since 1.8
 *
 * @ingroup Transit
 */
EAPI void                   elm_transit_smooth_set(Elm_Transit *transit, Eina_Bool smooth);

/**
 * Get the smooth scaling for transit map rendering
 *
 * This gets smooth scaling for transit map rendering.
 *
 * @param obj The transit object
 * @return @c EINA_TRUE if the smooth is enabled, @c EINA_FALSE otherwise.
 *
 * @see elm_transit_smooth_set()
 * @since 1.8
 *
 */
Eina_Bool                   elm_transit_smooth_get(const Elm_Transit *transit);

/**
 * Add the Resizing Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates resizing effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_w Object width size when effect begins.
 * @param from_h Object height size when effect begins.
 * @param to_w Object width size when effect ends.
 * @param to_h Object height size when effect ends.
 * @return Resizing effect context data.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_resizing_add(Elm_Transit *transit, Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h);

/**
 * Add the Translation Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates translation effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_dx X Position variation when effect begins.
 * @param from_dy Y Position variation when effect begins.
 * @param to_dx X Position variation when effect ends.
 * @param to_dy Y Position variation when effect ends.
 * @return Translation effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_translation_add(Elm_Transit *transit, Evas_Coord from_dx, Evas_Coord from_dy, Evas_Coord to_dx, Evas_Coord to_dy);

/**
 * Add the Zoom Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates zoom effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_rate Scale rate when effect begins (1 is current rate).
 * @param to_rate Scale rate when effect ends.
 * @return Zoom effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_zoom_add(Elm_Transit *transit, float from_rate, float to_rate);

/**
 * Add the Flip Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates flip effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "front" object and the second will be the "back" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param axis Flipping Axis(X or Y).
 * @param cw Flipping Direction. EINA_TRUE is clock-wise.
 * @return Flip effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_flip_add(Elm_Transit *transit, Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw);

/**
 * Add the Resizeable Flip Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates resizable flip effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "front" object and the second will be the "back" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param axis Flipping Axis(X or Y).
 * @param cw Flipping Direction. EINA_TRUE is clock-wise.
 * @return Resizeable flip effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_resizable_flip_add(Elm_Transit *transit, Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw);

/**
 * Add the Wipe Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates wipe effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param type Wipe type. Hide or show.
 * @param dir Wipe Direction.
 * @return Wipe effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_wipe_add(Elm_Transit *transit, Elm_Transit_Effect_Wipe_Type type, Elm_Transit_Effect_Wipe_Dir dir);

/**
 * Add the Color Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates color effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit        Transit object.
 * @param  from_r        RGB R when effect begins.
 * @param  from_g        RGB G when effect begins.
 * @param  from_b        RGB B when effect begins.
 * @param  from_a        RGB A when effect begins.
 * @param  to_r          RGB R when effect ends.
 * @param  to_g          RGB G when effect ends.
 * @param  to_b          RGB B when effect ends.
 * @param  to_a          RGB A when effect ends.
 * @return               Color effect context data.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_color_add(Elm_Transit *transit, unsigned int from_r, unsigned int from_g, unsigned int from_b, unsigned int from_a, unsigned int to_r, unsigned int to_g, unsigned int to_b, unsigned int to_a);

/**
 * Add the Fade Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates fade effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "before" object and the second will be the "after" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @return Fade effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the color information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_fade_add(Elm_Transit *transit);

/**
 * Add the Blend Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates blend effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "before" object and the second will be the "after" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @return Blend effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the color information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_blend_add(Elm_Transit *transit);

/**
 * Add the Rotation Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates rotation effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_degree Degree when effect begins.
 * @param to_degree Degree when effect is ends.
 * @return Rotation effect context data.
 *
 * @ingroup Transit
 * @warning It is highly recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_rotation_add(Elm_Transit *transit, float from_degree, float to_degree);

/**
 * Add the ImageAnimation Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates image animation effect context
 * and add it's required APIs to elm_transit_effect_add.
 * The @p images parameter is a list images paths. This list and
 * its contents will be deleted at the end of the effect by
 * elm_transit_effect_image_animation_context_free() function.
 *
 * Example:
 * @code
 * char buf[PATH_MAX];
 * Eina_List *images = NULL;
 * Elm_Transit *transi = elm_transit_add();
 *
 * snprintf(buf, sizeof(buf), "%s/images/icon_11.png", PACKAGE_DATA_DIR);
 * images = eina_list_append(images, eina_stringshare_add(buf));
 *
 * snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
 * images = eina_list_append(images, eina_stringshare_add(buf));
 * elm_transit_effect_image_animation_add(transi, images);
 *
 * @endcode
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param images Eina_List of images file paths. This list and
 * its contents will be deleted at the end of the effect by
 * elm_transit_effect_image_animation_context_free() function.
 * @return Image Animation effect context data.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Effect    *elm_transit_effect_image_animation_add(Elm_Transit *transit, Eina_List *images);
/**
 * @}
 */
