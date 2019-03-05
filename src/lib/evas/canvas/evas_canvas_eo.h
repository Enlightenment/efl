#ifndef _EVAS_CANVAS_EO_H_
#define _EVAS_CANVAS_EO_H_

#ifndef _EVAS_CANVAS_EO_CLASS_TYPE
#define _EVAS_CANVAS_EO_CLASS_TYPE

typedef Eo Evas_Canvas;

#endif

#ifndef _EVAS_CANVAS_EO_TYPES
#define _EVAS_CANVAS_EO_TYPES


#endif
/** Evas canvas class
 *
 * @ingroup Evas_Canvas
 */
#define EVAS_CANVAS_CLASS evas_canvas_class_get()

EWAPI const Efl_Class *evas_canvas_class_get(void);

/**
 * @brief Set the image cache.
 *
 * This function sets the image cache of canvas in bytes.
 *
 * @param[in] obj The object.
 * @param[in] size The cache size.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_image_cache_set(Eo *obj, int size);

/**
 * @brief Get the image cache.
 *
 * This function returns the image cache size of canvas in bytes.
 *
 * @param[in] obj The object.
 *
 * @return The cache size.
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_image_cache_get(const Eo *obj);

/**
 * @brief Set the default set of flags an event begins with
 *
 * Events in evas can have an event_flags member. This starts out with an
 * initial value (no flags). This lets you set the default flags that an event
 * begins with to @c flags.
 *
 * @param[in] obj The object.
 * @param[in] flags The default flags to use.
 *
 * @since 1.2
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_event_default_flags_set(Eo *obj, Efl_Input_Flags flags);

/**
 * @brief Get the default set of flags an event begins with
 *
 * This gets the default event flags events are produced with when fed in.
 *
 * @param[in] obj The object.
 *
 * @return The default flags to use.
 *
 * @since 1.2
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Input_Flags evas_canvas_event_default_flags_get(const Eo *obj);

/**
 * @brief Changes the size of font cache of the given evas.
 *
 * @param[in] obj The object.
 * @param[in] size The size in bytes.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_font_cache_set(Eo *obj, int size);

/**
 * @brief Get the size of font cache of the given evas in bytes.
 *
 * @param[in] obj The object.
 *
 * @return The size in bytes.
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_font_cache_get(const Eo *obj);

/**
 * @brief Attaches a specific pointer to the evas for fetching later.
 *
 * @param[in] obj The object.
 * @param[in] data The attached pointer.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_data_attach_set(Eo *obj, void *data);

/**
 * @brief Returns the pointer attached by @ref evas_canvas_data_attach_set.
 *
 * @param[in] obj The object.
 *
 * @return The attached pointer.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void *evas_canvas_data_attach_get(const Eo *obj);

/**
 * @brief Retrieve the object focused by the default seat.
 *
 * Focused objects will be the ones having key events delivered to, which the
 * programmer can act upon by means of @ref evas_object_event_callback_add
 * usage.
 *
 * @note Most users wouldn't be dealing directly with Evas' focused objects.
 * Instead, they would be using a higher level library for that (like a
 * toolkit, as Elementary) to handle focus and who's receiving input for them.
 *
 * This call returns the object that currently has focus on the canvas @c e or
 * @c null, if none.
 *
 * See also @ref evas_object_focus_set, @ref evas_object_focus_get, @ref
 * evas_object_key_grab, @ref evas_object_key_ungrab,
 * @ref evas_canvas_seat_focus_get, @ref efl_canvas_object_seat_focus_check,
 * @ref efl_canvas_object_seat_focus_add,
 * @ref efl_canvas_object_seat_focus_del.
 *
 * @param[in] obj The object.
 *
 * @return The object that has focus or @c null if there is not one.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Canvas_Object *evas_canvas_focus_get(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Return the focused object by a given seat.
 *
 * @param[in] obj The object.
 * @param[in] seat The seat to fetch the focused object or @c null for the
 * default seat.
 *
 * @return The object that has the focus or @c null if the seat has no focused
 * object.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Canvas_Object *evas_canvas_seat_focus_get(const Eo *obj, Efl_Input_Device *seat);

/**
 * @brief Get the highest (stacked) Evas object on the canvas @c e.
 *
 * This function will take all populated layers in the canvas into account,
 * getting the highest object for the highest layer, naturally.
 *
 * @warning This function will skip objects parented by smart objects, acting
 * only on the ones at the "top level", with regard to object parenting.
 *
 * See also @ref evas_object_layer_get, @ref evas_object_layer_set, @ref
 * evas_object_below_get, @ref evas_object_above_get.
 *
 * @param[in] obj The object.
 *
 * @return A pointer to the highest object on it (if any) or @c null otherwise.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Canvas_Object *evas_canvas_object_top_get(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief This function returns the current known default pointer coordinates.
 *
 * @param[in] obj The object.
 * @param[in] dev The pointer device.
 * @param[out] x The pointer to a Evas_Coord to be filled in.
 * @param[out] y The pointer to a Evas_Coord to be filled in.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_pointer_canvas_xy_by_device_get(const Eo *obj, Efl_Input_Device *dev, int *x, int *y);

/**
 * @brief This function returns the current known default pointer coordinates
 *
 * This function returns the current known canvas unit coordinates of the mouse
 * pointer and sets the contents of the Evas_Coords pointed to by @c x and @c y
 * to contain these coordinates. If @c e is not a valid canvas the results of
 * this function are undefined.
 *
 * @param[in] obj The object.
 * @param[out] x The pointer to a Evas_Coord to be filled in.
 * @param[out] y The pointer to a Evas_Coord to be filled in.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_pointer_canvas_xy_get(const Eo *obj, int *x, int *y);

/**
 * @brief Get the number of mouse or multi presses currently active.
 *
 * @param[in] obj The object.
 *
 * @return Mouse or multi presses currently active
 *
 * @since 1.2
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_event_down_count_get(const Eo *obj);

/**
 * @brief This gets the internal counter that counts the number of smart
 * calculations.
 *
 * Whenever evas performs smart object calculations on the whole canvas it
 * increments a counter by 1. This function returns the value of the smart
 * object calculate counter. It starts with a value of 0 and will increase (and
 * eventually wrap around to negative values and so on) by 1 every time objects
 * are calculated. You can use this counter to ensure you don't re-do
 * calculations withint the same calculation generation/run if the calculations
 * maybe cause self-feeding effects.
 *
 * @param[in] obj The object.
 *
 * @return Number of smart calculations
 *
 * @since 1.1
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_smart_objects_calculate_count_get(const Eo *obj);

/**
 * @brief Get the focus state for the default seat.
 *
 * @param[in] obj The object.
 *
 * @return @c true if focused, @c false otherwise
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_Bool evas_canvas_focus_state_get(const Eo *obj);

/**
 * @brief Get the focus state by a given seat.
 *
 * @param[in] obj The object.
 * @param[in] seat The seat to check the focus state. Use @c null for the
 * default seat.
 *
 * @return @c true if the seat has the canvas focus, @c false otherwise.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_Bool evas_canvas_seat_focus_state_get(const Eo *obj, Efl_Input_Device *seat);

/**
 * @brief Get the changed marker for the canvas.
 *
 * @param[in] obj The object.
 *
 * @return @c true if changed, @c false otherwise
 *
 * @since 1.11
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_Bool evas_canvas_changed_get(const Eo *obj);

/**
 * @brief This function returns the current known pointer coordinates.
 *
 * @param[in] obj The object.
 * @param[in] dev The mouse device.
 * @param[out] x The pointer to an integer to be filled in.
 * @param[out] y The pointer to an integer to be filled in.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_pointer_output_xy_by_device_get(const Eo *obj, Efl_Input_Device *dev, int *x, int *y);

/**
 * @brief This function returns the current known default pointer coordinates.
 *
 * This function returns the current known screen/output coordinates of the
 * mouse pointer and sets the contents of the integers pointed to by @c x and
 * @c y to contain these coordinates. If @c e is not a valid canvas the results
 * of this function are undefined.
 *
 * @param[in] obj The object.
 * @param[out] x The pointer to an integer to be filled in.
 * @param[out] y The pointer to an integer to be filled in.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_pointer_output_xy_get(const Eo *obj, int *x, int *y);

/**
 * @brief Get the lowest (stacked) Evas object on the canvas @c e.
 *
 * This function will take all populated layers in the canvas into account,
 * getting the lowest object for the lowest layer, naturally.
 *
 * @warning This function will skip objects parented by smart objects, acting
 * only on the ones at the "top level", with regard to object parenting.
 *
 * See also @ref evas_object_layer_get, @ref evas_object_layer_set, @ref
 * evas_object_below_get, @ref evas_object_below_set.
 *
 * @param[in] obj The object.
 *
 * @return A pointer to the lowest object on it, if any, or @c null otherwise.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Canvas_Object *evas_canvas_object_bottom_get(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a bitmask with the mouse buttons currently pressed, set to 1.
 *
 * @param[in] obj The object.
 * @param[in] dev The mouse device.
 *
 * @return A bitmask of the currently depressed buttons on the canvas.
 *
 * @ingroup Evas_Canvas
 */
EOAPI unsigned int evas_canvas_pointer_button_down_mask_by_device_get(const Eo *obj, Efl_Input_Device *dev);

/**
 * @brief Returns a bitmask with the default mouse buttons currently pressed,
 * set to 1.
 *
 * Calling this function will return a 32-bit integer with the appropriate bits
 * set to 1, which correspond to a mouse button being depressed. This limits
 * Evas to a mouse devices with a maximum of 32 buttons, but that is generally
 * in excess of any host system's pointing device abilities.
 *
 * A canvas by default begins with no mouse buttons being pressed and only
 * pointer move events can alter that.
 *
 * The least significant bit corresponds to the first mouse button (button 1)
 * and the most significant bit corresponds to the last mouse button (button
 * 32).
 *
 * If @c e is not a valid canvas, the return value is undefined.
 *
 * @param[in] obj The object.
 *
 * @return A bitmask of the currently depressed buttons on the canvas.
 *
 * @ingroup Evas_Canvas
 */
EOAPI unsigned int evas_canvas_pointer_button_down_mask_get(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieve a list of Evas objects lying over a given position in a
 * canvas.
 *
 * This function will traverse all the layers of the given canvas, from top to
 * bottom, querying for objects with areas covering the given position. It will
 * enter the smart objects. It will not append to the list pass events as
 * hidden objects. Call eina_list_free on the returned list after usage.
 *
 * @param[in] obj The object.
 * @param[in] stop An Evas Object where to stop searching.
 * @param[in] x The horizontal coordinate of the position.
 * @param[in] y The vertical coordinate of the position.
 *
 * @return List of objects
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_List *evas_canvas_tree_objects_at_xy_get(Eo *obj, Efl_Canvas_Object *stop, int x, int y) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Enables or turns on programmatically the lock key with name
 * @c keyname for the default seat.
 *
 * The effect will be as if the key was put on its active state after this
 * call.
 *
 * See also @ref evas_canvas_key_lock_add, @ref evas_canvas_key_lock_del,
 * @ref evas_canvas_key_lock_del, @ref evas_canvas_key_lock_off,
 * @ref evas_canvas_seat_key_lock_on, @ref evas_canvas_seat_key_lock_off.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the lock to enable.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_lock_on(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Enables or turns on programmatically the lock key with name
 * @c keyname for a give seat.
 *
 * The effect will be as if the key was put on its active state after this
 * call.
 *
 * See also @ref evas_canvas_key_lock_add, @ref evas_canvas_key_lock_del,
 * @ref evas_canvas_key_lock_del, @ref evas_canvas_key_lock_off,
 * @ref evas_canvas_key_lock_on, @ref evas_canvas_seat_key_lock_off.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the lock to enable.
 * @param[in] seat The seat to enable the keylock. A @c null seat repesents the
 * default seat.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_seat_key_lock_on(Eo *obj, const char *keyname, Efl_Input_Device *seat) EINA_ARG_NONNULL(2);

/**
 * @brief Disables or turns off programmatically the lock key with name
 * @c keyname for a given seat.
 *
 * The effect will be as if the key was put on its inactive state after this
 * call.
 *
 * See also @ref evas_canvas_key_lock_on, @ref evas_canvas_seat_key_lock_on,
 * @ref evas_canvas_key_lock_off.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the lock to enable.
 * @param[in] seat The seat to disable the keylock. A @c null seat repesents
 * the default seat.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_seat_key_lock_off(Eo *obj, const char *keyname, Efl_Input_Device *seat) EINA_ARG_NONNULL(2);

/**
 * @brief Adds the @c keyname key to the current list of modifier keys.
 *
 * Modifiers are keys like shift, alt and ctrl, i.e. keys which are meant to be
 * pressed together with others, altering the behavior of the second keys
 * pressed. Evas is so that these keys can be user defined.
 *
 * This call allows custom modifiers to be added to the Evas system at run
 * time. It is then possible to set and unset modifier keys programmatically
 * for other parts of the program to check and act on. Programmers using Evas
 * would check for modifier keys on key event callbacks using @ref
 * evas_key_modifier_is_set.
 *
 * @note If the programmer instantiates the canvas by means of the @ref
 * ecore_evas_new family of helper functions, Ecore will take care of
 * registering on it all standard modifiers: "Shift", "Control", "Alt", "Meta",
 * "Hyper", "Super".
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the modifier key to add to the list of Evas
 * modifiers.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_modifier_add(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Disables or turns off programmatically the modifier key with name
 * @c keyname for the default seat.
 *
 * See also @ref evas_canvas_key_modifier_add, @ref evas_key_modifier_get,
 * @ref evas_canvas_key_modifier_on, @ref evas_canvas_seat_key_modifier_off,
 * @ref evas_canvas_seat_key_modifier_off, @ref evas_key_modifier_is_set, @ref
 * evas_seat_key_modifier_is_set.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the modifier to disable.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_modifier_off(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Render the given Evas canvas asynchronously.
 *
 * This function only returns @c true when a frame will be rendered. If the
 * previous frame is still rendering, @c false will be returned  so the users
 * know not to wait for the updates callback and just  return to their main
 * loop.
 *
 * If a @c func callback is given, a list of updated areas will be generated
 * and the function will be called from the main thread after the rendered
 * frame is flushed to the screen. The resulting list should be freed with @ref
 * evas_render_updates_free.
 *
 * The list is given in the @c event_info parameter of the callback function.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the canvas will render, @c false otherwise.
 *
 * @since 1.8
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_Bool evas_canvas_render_async(Eo *obj);

/** Inform the evas that it lost the focus from the default seat.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_focus_out(Eo *obj);

/**
 * @brief Update the canvas internal objects but not triggering immediate
 * renderization.
 *
 * This function updates the canvas internal objects not triggering
 * renderization. To force renderization function @ref evas_canvas_render
 * should be used.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_norender(Eo *obj);

/**
 * @brief Pop the nochange flag down 1.
 *
 * This tells evas, that while the nochange flag is greater than 0, do not mark
 * objects as "changed" when making changes.
 *
 * @warning Do not use this function unless you know what Evas exactly works
 * with "changed" state.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_nochange_pop(Eo *obj);

/**
 * @brief Disables or turns off programmatically the lock key with name
 * @c keyname for the default seat.
 *
 * The effect will be as if the key was put on its inactive state after this
 * call.
 *
 * See also @ref evas_canvas_key_lock_on, @ref evas_canvas_seat_key_lock_on,
 * @ref evas_canvas_seat_key_lock_off.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the lock to disable.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_lock_off(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Push the nochange flag up 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not mark
 * objects as "changed" when making changes.
 *
 * @warning Do not use this function unless you know what Evas exactly works
 * with "changed" state.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_nochange_push(Eo *obj);

/** Force the given evas and associated engine to flush its font cache.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_font_cache_flush(Eo *obj);

/**
 * @brief Enables or turns on programmatically the modifier key with name
 * @c keyname for the default seat.
 *
 * The effect will be as if the key was pressed for the whole time between this
 * call and a matching evas_key_modifier_off().
 *
 * See also @ref evas_canvas_key_modifier_off,
 * @ref evas_canvas_seat_key_modifier_on,
 * @ref evas_canvas_seat_key_modifier_off.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the modifier to enable.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_modifier_on(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Enables or turns on programmatically the modifier key with name
 * @c keyname for a given seat.
 *
 * The effect will be as if the key was pressed for the whole time between this
 * call and a matching @ref evas_canvas_seat_key_modifier_off.
 *
 * See also @ref evas_canvas_key_modifier_off,
 * @ref evas_canvas_seat_key_modifier_on,
 * @ref evas_canvas_seat_key_modifier_off.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the lock to enable.
 * @param[in] seat The seat to enable the modifier. A @c null seat repesents
 * the default seat.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_seat_key_modifier_on(Eo *obj, const char *keyname, Efl_Input_Device *seat) EINA_ARG_NONNULL(2);

/**
 * @brief Disables or turns off programmatically the modifier key with name
 * @c keyname for a given seat.
 *
 * See also @ref evas_canvas_key_modifier_add, @ref evas_key_modifier_get,
 * @ref evas_canvas_key_modifier_on, @ref evas_canvas_seat_key_modifier_off,
 * @ref evas_canvas_seat_key_modifier_off, @ref evas_key_modifier_is_set, @ref
 * evas_seat_key_modifier_is_set.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the lock to enable.
 * @param[in] seat The seat to disable the modifier. A @c null seat repesents
 * the default seat.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_seat_key_modifier_off(Eo *obj, const char *keyname, Efl_Input_Device *seat) EINA_ARG_NONNULL(2);

/**
 * @brief List of available font descriptions known or found by this evas.
 *
 * The list depends on Evas compile time configuration, such as fontconfig
 * support, and the paths provided at runtime as explained in @ref
 * Evas_Font_Path_Group.
 *
 * @param[in] obj The object.
 *
 * @return A newly allocated list of strings. Do not change the strings. Be
 * sure to call @ref evas_font_available_list_free after you're done.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_List *evas_canvas_font_available_list(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieves the object on the given evas with the given name.
 *
 * This looks for the evas object given a name by @ref evas_object_name_set. If
 * the name is not unique canvas-wide, then which one of the many objects with
 * that name is returned is undefined, so only use this if you can ensure the
 * object name is unique.
 *
 * @param[in] obj The object.
 * @param[in] name The given name.
 *
 * @return If successful, the Evas object with the given name. Otherwise,
 * @c null.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Canvas_Object *evas_canvas_object_name_find(const Eo *obj, const char *name) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Appends a font path to the list of font paths used by the given evas.
 *
 * @param[in] obj The object.
 * @param[in] path The new font path.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_font_path_append(Eo *obj, const char *path) EINA_ARG_NONNULL(2);

/** Removes all font paths loaded into memory for the given evas.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_font_path_clear(Eo *obj);

/**
 * @brief This function returns the nth touch point's coordinates.
 *
 * Touch point's coordinates is updated whenever moving that point on the
 * canvas.
 *
 * @param[in] obj The object.
 * @param[in] n The number of the touched point (0 being the first).
 * @param[out] x Last known X position in window coordinates
 * @param[out] y Last known Y position in window coordinates
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_touch_point_list_nth_xy_get(Eo *obj, unsigned int n, double *x, double *y);

/**
 * @brief Removes the @c keyname key from the current list of lock keys on
 * canvas @c e.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the key to remove from the locks list.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_lock_del(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Add a damage rectangle.
 *
 * This is the function by which one tells evas that a part of the canvas has
 * to be repainted.
 *
 * @note All newly created Evas rectangles get the default color values of 255
 * 255 255 255 (opaque white).
 *
 * @param[in] obj The object.
 * @param[in] x The rectangle's left position.
 * @param[in] y The rectangle's top position.
 * @param[in] w The rectangle's width.
 * @param[in] h The rectangle's height.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_damage_rectangle_add(Eo *obj, int x, int y, int w, int h);

/** Sync evas canvas
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_sync(Eo *obj);

/**
 * @brief Retrieves the list of font paths used by the given evas.
 *
 * @param[in] obj The object.
 *
 * @return The list of font paths used.
 *
 * @ingroup Evas_Canvas
 */
EOAPI const Eina_List *evas_canvas_font_path_list(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Reload the image cache.
 *
 * This function reloads the image cache of canvas.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_image_cache_reload(Eo *obj);

/**
 * @brief Convert/scale a canvas coordinate into output screen coordinates.
 *
 * This function takes in a horizontal coordinate as the @c x parameter and
 * converts it into output units, accounting for output size, viewport size and
 * location, returning it as the function  return value. If @c e is invalid,
 * the results are undefined.
 *
 * @param[in] obj The object.
 * @param[in] x The canvas X coordinate.
 *
 * @return The output/screen coordinate translated to output coordinates.
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_coord_world_x_to_screen(const Eo *obj, int x) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Force immediate renderization of the given Evas canvas.
 *
 * This function forces an immediate renderization update of the given canvas
 * @c e.
 *
 * @note This is a very low level function, which most of Evas' users wouldn't
 * care about. You might use it, for instance, to grab an Evas' canvas update
 * regions and paint them back, using the canvas' pixmap, on a displaying
 * system working below Evas.
 *
 * @note Evas is a stateful canvas. If no operations changing its state took
 * place since the last rendering action, you won't see any changes and this
 * call will be a no-op.
 *
 * @param[in] obj The object.
 *
 * @return A newly allocated list of updated rectangles of the canvas
 * (@Eina.Rect structs). Free this list with @ref evas_render_updates_free.
 *
 * @ingroup Evas_Canvas
 */
EOAPI Eina_List *evas_canvas_render_updates(Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Flush the image cache of the canvas.
 *
 * This function flushes image cache of canvas.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_image_cache_flush(Eo *obj);

/**
 * @brief Convert/scale an output screen coordinate into canvas coordinates.
 *
 * This function takes in a vertical coordinate as the @c y parameter and
 * converts it into canvas units, accounting for output size, viewport size and
 * location, returning it as the function return value. If @c e is invalid, the
 * results are undefined.
 *
 * @param[in] obj The object.
 * @param[in] y The screen/output y coordinate.
 *
 * @return The screen coordinate translated to canvas unit coordinates.
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_coord_screen_y_to_world(const Eo *obj, int y) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Removes the @c keyname key from the current list of modifier keys on
 * canvas @c e.
 *
 * See also @ref evas_canvas_key_modifier_add.
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the key to remove from the modifiers list.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_modifier_del(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/** Inform to the evas that it got the focus from the default seat.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_focus_in(Eo *obj);

/**
 * @brief Inform to the evas that it got the focus from a given seat.
 *
 * @param[in] obj The object.
 * @param[in] seat The seat or @c null for the default seat.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_seat_focus_in(Eo *obj, Efl_Input_Device *seat);

/**
 * @brief Inform to the evas that it lost the focus from a given seat.
 *
 * @param[in] obj The object.
 * @param[in] seat The seat or @c null for the default seat.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_seat_focus_out(Eo *obj, Efl_Input_Device *seat);

/**
 * @brief Add an "obscured region" to an Evas canvas.
 *
 * This is the function by which one tells an Evas canvas that a part of it
 * must not be repainted. The region must be rectangular and its coordinates
 * inside the canvas viewport are passed in the call. After this call, the
 * region specified won't participate in any form in Evas' calculations and
 * actions during its rendering updates, having its displaying content frozen
 * as it was just after this function took place.
 *
 * We call it "obscured region" because the most common use case for this
 * rendering (partial) freeze is something else (most probably other canvas)
 * being on top of the specified rectangular region, thus shading it completely
 * from the user's final scene in a display. To avoid unnecessary processing,
 * one should indicate to the obscured canvas not to bother about the
 * non-important area.
 *
 * The majority of users won't have to worry about this function, as they'll be
 * using just one canvas in their applications, with nothing inset or on top of
 * it in any form.
 *
 * To make this region one that has to be repainted again, call the function
 * @ref evas_obscured_clear.
 *
 * @note This is a very low level function, which most of Evas' users wouldn't
 * care about.
 *
 * @note This function does not flag the canvas as having its state changed. If
 * you want to re-render it afterwards expecting new contents, you have to add
 * "damage" regions yourself (see @ref evas_damage_rectangle_add).
 *
 * @param[in] obj The object.
 * @param[in] x The rectangle's top left corner's horizontal coordinate.
 * @param[in] y The rectangle's top left corner's vertical coordinate.
 * @param[in] w The rectangle's width.
 * @param[in] h The rectangle's height.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_obscured_rectangle_add(Eo *obj, int x, int y, int w, int h);

/**
 * @brief Make the canvas discard as much data as possible used by the engine
 * at runtime.
 *
 * This function will unload images, delete textures and much more where
 * possible. You may also want to call @ref evas_canvas_render_idle_flush
 * immediately prior to this to perhaps discard a little more, though this
 * function should implicitly delete most of what
 * @ref evas_canvas_render_idle_flush might discard too.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_render_dump(Eo *obj);

/** Force renderization of the given canvas.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_render(Eo *obj);

/**
 * @brief Prepends a font path to the list of font paths used by the given
 * evas.
 *
 * @param[in] obj The object.
 * @param[in] path The new font path.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_font_path_prepend(Eo *obj, const char *path) EINA_ARG_NONNULL(2);

/**
 * @brief Remove all "obscured regions" from an Evas canvas.
 *
 * This function removes all the rectangles from the obscured regions list of
 * the canvas @c e. It takes obscured areas added with
 * @ref evas_canvas_obscured_rectangle_add and make them again a regions that
 * have to be repainted on rendering updates.
 *
 * @note This is a very low level function, which most of Evas' users wouldn't
 * care about.
 *
 * @note This function does not flag the canvas as having its state changed. If
 * you want to re-render it afterwards expecting new contents, you have to add
 * "damage" regions yourself (see @ref evas_canvas_damage_rectangle_add).
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_obscured_clear(Eo *obj);

/**
 * @brief Convert/scale an output screen coordinate into canvas coordinates.
 *
 * This function takes in a horizontal coordinate as the @c x parameter and
 * converts it into canvas units, accounting for output size, viewport size and
 * location, returning it as the function return value. If @c e is invalid, the
 * results are undefined.
 *
 * @param[in] obj The object.
 * @param[in] x The screen/output x coordinate.
 *
 * @return The screen coordinate translated to canvas unit coordinates.
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_coord_screen_x_to_world(const Eo *obj, int x) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Adds the @c keyname key to the current list of lock keys.
 *
 * Locks are keys like caps lock, num lock or scroll lock, i.e., keys which are
 * meant to be pressed once -- toggling a binary state which is bound to it --
 * and thus altering the behavior of all  subsequently pressed keys somehow,
 * depending on its state. Evas is so that these keys can be defined by the
 * user.
 *
 * This allows custom locks to be added to the evas system at run time. It is
 * then possible to set and unset lock keys programmatically for other parts of
 * the program to check and act on. Programmers using Evas would check for lock
 * keys on key event callbacks using @ref evas_key_lock_is_set.
 *
 * @note If the programmer instantiates the canvas by means of the
 * ecore_evas_new() family of helper functions, Ecore will take care of
 * registering on it all standard lock keys: "Caps_Lock", "Num_Lock",
 * "Scroll_Lock".
 *
 * @param[in] obj The object.
 * @param[in] keyname The name of the key to add to the locks list.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_key_lock_add(Eo *obj, const char *keyname) EINA_ARG_NONNULL(2);

/**
 * @brief Make the canvas discard internally cached data used for rendering.
 *
 * This function flushes the arrays of delete, active and render objects. Other
 * things it may also discard are: shared memory segments, temporary scratch
 * buffers, cached data to avoid re-compute of that data etc.
 * @param[in] obj The object.
 *
 * @ingroup Evas_Canvas
 */
EOAPI void evas_canvas_render_idle_flush(Eo *obj);

/**
 * @brief Return the default device of a given type.
 *
 * @note Currently EFL only creates a seat, mouse and keyboard.
 *
 * @param[in] obj The object.
 * @param[in] type The class of the default device to fetch.
 *
 * @return The default device or @c null on error.
 *
 * @since 1.19
 *
 * @ingroup Evas_Canvas
 */
EOAPI Efl_Input_Device *evas_canvas_default_device_get(const Eo *obj, Efl_Input_Device_Type type);

/**
 * @brief Convert/scale a canvas coordinate into output screen coordinates.
 *
 * This function takes in a vertical coordinate as the @c x parameter and
 * converts it into output units, accounting for output size, viewport size and
 * location, returning it as the function return value. If @c e is invalid, the
 * results are undefined.
 *
 * @param[in] obj The object.
 * @param[in] y The canvas y coordinate.
 *
 * @return The output/screen coordinate translated to output coordinates.
 *
 * @ingroup Evas_Canvas
 */
EOAPI int evas_canvas_coord_world_y_to_screen(const Eo *obj, int y) EINA_WARN_UNUSED_RESULT;

#endif
