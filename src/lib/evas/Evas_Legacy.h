#ifndef _EVAS_H
# error You shall not include this header directly
#endif

/**
 * @ingroup Evas_Canvas
 *
 * @{
 */
/**
 * Creates a new empty evas.
 *
 * Note that before you can use the evas, you will to at a minimum:
 * @li Set its render method with @ref evas_output_method_set .
 * @li Set its viewport size with @ref evas_output_viewport_set .
 * @li Set its size of the canvas with @ref evas_output_size_set .
 * @li Ensure that the render engine is given the correct settings
 *     with @ref evas_engine_info_set .
 *
 * This function should only fail if the memory allocation fails
 *
 * @note this function is very low level. Instead of using it
 *       directly, consider using the high level functions in
 *       @ref Ecore_Evas_Group such as @c ecore_evas_new(). See
 *       @ref Ecore.
 *
 * @attention it is recommended that one calls evas_init() before
 *       creating new canvas.
 *
 * @return A new uninitialised Evas canvas on success. Otherwise, @c NULL.
 * @ingroup Evas_Canvas
 */
EAPI Evas             *evas_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Frees the given evas and any objects created on it.
 *
 * Any objects with 'free' callbacks will have those callbacks called
 * in this function.
 *
 * @param   e The given evas.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_free(Evas *e)  EINA_ARG_NONNULL(1);

/**
 * Inform to the evas that it got the focus.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_in(Evas *e);

/**
 * Inform to the evas that it lost the focus.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_focus_out(Evas *e);

/**
 * Get the focus state known by the given evas
 *
 * @param e The evas to query information.
 * @return @c EINA_TRUE if it got the focus, @c EINA_FALSE otherwise.
 * @ingroup Evas_Canvas
 */
EAPI Eina_Bool         evas_focus_state_get(const Evas *e);

/**
 * Push the nochange flag up 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not
 * mark objects as "changed" when making changes.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_push(Evas *e);

/**
 * Pop the nochange flag down 1
 *
 * This tells evas, that while the nochange flag is greater than 0, do not
 * mark objects as "changed" when making changes.
 *
 * @param e The evas to change information.
 * @ingroup Evas_Canvas
 */
EAPI void              evas_nochange_pop(Evas *e);

/**
 * Attaches a specific pointer to the evas for fetching later
 *
 * @param e The canvas to attach the pointer to
 * @param data The pointer to attach
 * @ingroup Evas_Canvas
 */
EAPI void              evas_data_attach_set(Evas *e, void *data) EINA_ARG_NONNULL(1);

/**
 * Returns the pointer attached by evas_data_attach_set()
 *
 * @param e The canvas to attach the pointer to
 * @return The pointer attached
 * @ingroup Evas_Canvas
 */
EAPI void             *evas_data_attach_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Add a damage rectangle.
 *
 * @param e The given canvas pointer.
 * @param x The rectangle's left position.
 * @param y The rectangle's top position.
 * @param w The rectangle's width.
 * @param h The rectangle's height.
 *
 * This is the function by which one tells evas that a part of the
 * canvas has to be repainted.
 *
 * @note All newly created Evas rectangles get the default color values of 255 255 255 255 (opaque white).
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_damage_rectangle_add(Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Add an "obscured region" to an Evas canvas.
 *
 * @param e The given canvas pointer.
 * @param x The rectangle's top left corner's horizontal coordinate.
 * @param y The rectangle's top left corner's vertical coordinate
 * @param w The rectangle's width.
 * @param h The rectangle's height.
 *
 * This is the function by which one tells an Evas canvas that a part
 * of it <b>must not</b> be repainted. The region must be
 * rectangular and its coordinates inside the canvas viewport are
 * passed in the call. After this call, the region specified won't
 * participate in any form in Evas' calculations and actions during
 * its rendering updates, having its displaying content frozen as it
 * was just after this function took place.
 *
 * We call it "obscured region" because the most common use case for
 * this rendering (partial) freeze is something else (most probably
 * other canvas) being on top of the specified rectangular region,
 * thus shading it completely from the user's final scene in a
 * display. To avoid unnecessary processing, one should indicate to the
 * obscured canvas not to bother about the non-important area.
 *
 * The majority of users won't have to worry about this function, as
 * they'll be using just one canvas in their applications, with
 * nothing inset or on top of it in any form.
 *
 * To make this region one that @b has to be repainted again, call the
 * function evas_obscured_clear().
 *
 * @note This is a <b>very low level function</b>, which most of
 * Evas' users wouldn't care about.
 *
 * @note This function does @b not flag the canvas as having its state
 * changed. If you want to re-render it afterwards expecting new
 * contents, you have to add "damage" regions yourself (see
 * evas_damage_rectangle_add()).
 *
 * @see evas_obscured_clear()
 * @see evas_render_updates()
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip add an obscured
 * @until evas_obscured_clear(evas);
 *
 * In that example, pressing the "Ctrl" and "o" keys will impose or
 * remove an obscured region in the middle of the canvas. You'll get
 * the same contents at the time the key was pressed, if toggling it
 * on, until you toggle it off again (make sure the animation is
 * running on to get the idea better). See the full @ref
 * Example_Evas_Events "example".
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_obscured_rectangle_add(Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Remove all "obscured regions" from an Evas canvas.
 *
 * @param e The given canvas pointer.
 *
 * This function removes all the rectangles from the obscured regions
 * list of the canvas @p e. It takes obscured areas added with
 * evas_obscured_rectangle_add() and make them again a regions that @b
 * have to be repainted on rendering updates.
 *
 * @note This is a <b>very low level function</b>, which most of
 * Evas' users wouldn't care about.
 *
 * @note This function does @b not flag the canvas as having its state
 * changed. If you want to re-render it afterwards expecting new
 * contents, you have to add "damage" regions yourself (see
 * evas_damage_rectangle_add()).
 *
 * @see evas_obscured_rectangle_add() for an example
 * @see evas_render_updates()
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_obscured_clear(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Render the given Evas canvas asynchronously.
 *
 * @param e The canvas to render.
 * @param func Optional function to call with the list of updated areas.
 * @param data User data to pass to @p func.
 *
 * @return EINA_TRUE if the canvas will render, EINA_FALSE otherwise.
 *
 * This function only returns EINA_TRUE when a frame will be rendered. If the
 * previous frame is still rendering, EINA_FALSE will be returned so the users
 * know not to wait for the updates callback and just return to their main
 * loop.
 *
 * If a @p func callback is given, a list of updated areas will be generated
 * and the function will be called from the main thread after the rendered
 * frame is flushed to the screen. The resulting list should be freed with
 * @f evas_render_updates_free().
 * The list is given in the @p event_info parameter of the callback function.
 *
 * @ingroup Evas_Canvas
 * @since 1.8
 */
EAPI Eina_Bool         evas_render_async(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Force immediate renderization of the given Evas canvas.
 *
 * @param e The given canvas pointer.
 * @return A newly allocated list of updated rectangles of the canvas
 *        (@c Eina_Rectangle structs). Free this list with
 *        evas_render_updates_free().
 *
 * This function forces an immediate renderization update of the given
 * canvas @p e.
 *
 * @note This is a <b>very low level function</b>, which most of
 * Evas' users wouldn't care about. One would use it, for example, to
 * grab an Evas' canvas update regions and paint them back, using the
 * canvas' pixmap, on a displaying system working below Evas.
 *
 * @note Evas is a stateful canvas. If no operations changing its
 * state took place since the last rendering action, you won't see no
 * changes and this call will be a no-op.
 *
 * Example code follows.
 * @dontinclude evas-events.c
 * @skip add an obscured
 * @until d.obscured = !d.obscured;
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Canvas
 */
EAPI Eina_List        *evas_render_updates(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Force renderization of the given canvas.
 *
 * @param e The given canvas pointer.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Update the canvas internal objects but not triggering immediate
 * renderization.
 *
 * @param e The given canvas pointer.
 *
 * This function updates the canvas internal objects not triggering
 * renderization. To force renderization function evas_render() should
 * be used.
 *
 * @see evas_render.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_norender(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Make the canvas discard internally cached data used for rendering.
 *
 * @param e The given canvas pointer.
 *
 * This function flushes the arrays of delete, active and render objects.
 * Other things it may also discard are: shared memory segments,
 * temporary scratch buffers, cached data to avoid re-compute of that data etc.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_idle_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Make the canvas discard as much data as possible used by the engine at
 * runtime.
 *
 * @param e The given canvas pointer.
 *
 * This function will unload images, delete textures and much more, where
 * possible. You may also want to call evas_render_idle_flush() immediately
 * prior to this to perhaps discard a little more, though evas_render_dump()
 * should implicitly delete most of what evas_render_idle_flush() might
 * discard too.
 *
 * @ingroup Evas_Canvas
 */
EAPI void              evas_render_dump(Evas *e) EINA_ARG_NONNULL(1);


/**
 * @}
 */


/**
 * @ingroup Evas_Output_Method
 *
 * @{
 */

/**
 * Sets the output engine for the given evas.
 *
 * Once the output engine for an evas is set, any attempt to change it
 * will be ignored.  The value for @p render_method can be found using
 * @ref evas_render_method_lookup .
 *
 * @param   e             The given evas.
 * @param   render_method The numeric engine value to use.
 *
 * @attention it is mandatory that one calls evas_init() before
 *       setting the output method.
 *
 * @ingroup Evas_Output_Method
 */
EAPI void              evas_output_method_set(Evas *e, int render_method) EINA_ARG_NONNULL(1);

/**
 * Retrieves the number of the output engine used for the given evas.
 * @param   e The given evas.
 * @return  The ID number of the output engine being used.  @c 0 is
 *          returned if there is an error.
 * @ingroup Evas_Output_Method
 */
EAPI int               evas_output_method_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves the current render engine info struct from the given evas.
 *
 * The returned structure is publicly modifiable.  The contents are
 * valid until either @ref evas_engine_info_set or @ref evas_render
 * are called.
 *
 * This structure does not need to be freed by the caller.
 *
 * @param   e The given evas.
 * @return  A pointer to the Engine Info structure.  @c NULL is returned if
 *          an engine has not yet been assigned.
 * @ingroup Evas_Output_Method
 */
EAPI Evas_Engine_Info *evas_engine_info_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Applies the engine settings for the given evas from the given @c
 * Evas_Engine_Info structure.
 *
 * To get the Evas_Engine_Info structure to use, call @ref
 * evas_engine_info_get .  Do not try to obtain a pointer to an
 * @c Evas_Engine_Info structure in any other way.
 *
 * You will need to call this function at least once before you can
 * create objects on an evas or render that evas.  Some engines allow
 * their settings to be changed more than once.
 *
 * Once called, the @p info pointer should be considered invalid.
 *
 * @param   e    The pointer to the Evas Canvas
 * @param   info The pointer to the Engine Info to use
 * @return  @c EINA_TRUE if no error occurred, @c EINA_FALSE otherwise.
 * @ingroup Evas_Output_Method
 */
EAPI Eina_Bool         evas_engine_info_set(Evas *e, Evas_Engine_Info *info) EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @ingroup Evas_Output_Size
 *
 * @{
 */
/**
 * Sets the output size of the render engine of the given evas.
 *
 * The evas will render to a rectangle of the given size once this
 * function is called.  The output size is independent of the viewport
 * size.  The viewport will be stretched to fill the given rectangle.
 *
 * The units used for @p w and @p h depend on the engine used by the
 * evas.
 *
 * @param   e The given evas.
 * @param   w The width in output units, usually pixels.
 * @param   h The height in output units, usually pixels.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_size_set(Evas *e, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieve the output size of the render engine of the given evas.
 *
 * The output size is given in whatever the output units are for the
 * engine.
 *
 * If either @p w or @p h is @c NULL, then it is ignored.  If @p e is
 * invalid, the returned results are undefined.
 *
 * @param   e The given evas.
 * @param   w The pointer to an integer to store the width in.
 * @param   h The pointer to an integer to store the height in.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_size_get(const Evas *e, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Sets the output viewport of the given evas in evas units.
 *
 * The output viewport is the area of the evas that will be visible to
 * the viewer.  The viewport will be stretched to fit the output
 * target of the evas when rendering is performed.
 *
 * @note The coordinate values do not have to map 1-to-1 with the output
 *       target.  However, it is generally advised that it is done for ease
 *       of use.
 *
 * @param   e The given evas.
 * @param   x The top-left corner x value of the viewport.
 * @param   y The top-left corner y value of the viewport.
 * @param   w The width of the viewport.  Must be greater than 0.
 * @param   h The height of the viewport.  Must be greater than 0.
 * @ingroup Evas_Output_Size
 */
EAPI void              evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Get the render engine's output viewport co-ordinates in canvas units.
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a x variable to be filled in
 * @param y The pointer to a y variable to be filled in
 * @param w The pointer to a width variable to be filled in
 * @param h The pointer to a height variable to be filled in
 * @ingroup Evas_Output_Size
 *
 * Calling this function writes the current canvas output viewport
 * size and location values into the variables pointed to by @p x, @p
 * y, @p w and @p h.  On success the variables have the output
 * location and size values written to them in canvas units. Any of @p
 * x, @p y, @p w or @p h that are @c NULL will not be written to. If @p e
 * is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord x, y, width, height;
 *
 * evas_output_viewport_get(evas, &x, &y, &w, &h);
 * @endcode
 */
EAPI void              evas_output_viewport_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the output framespace size of the render engine of the given evas.
 *
 * The framespace size is used in the Wayland engines to denote space where
 * the output is not drawn. This is mainly used in ecore_evas to draw borders
 *
 * The units used for @p w and @p h depend on the engine used by the
 * evas.
 *
 * @param   e The given evas.
 * @param   x The left coordinate in output units, usually pixels.
 * @param   y The top coordinate in output units, usually pixels.
 * @param   w The width in output units, usually pixels.
 * @param   h The height in output units, usually pixels.
 * @ingroup Evas_Output_Size
 * @since 1.1
 */
EAPI void              evas_output_framespace_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * Get the render engine's output framespace co-ordinates in canvas units.
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a x variable to be filled in
 * @param y The pointer to a y variable to be filled in
 * @param w The pointer to a width variable to be filled in
 * @param h The pointer to a height variable to be filled in
 * @ingroup Evas_Output_Size
 * @since 1.1
 */
EAPI void              evas_output_framespace_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
/**
 * @}
 */

/**
 * Convert/scale an output screen co-ordinate into canvas co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The screen/output x co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a horizontal co-ordinate as the @p x
 * parameter and converts it into canvas units, accounting for output
 * size, viewport size and location, returning it as the function
 * return value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern int screen_x;
 * Evas_Coord canvas_x;
 *
 * canvas_x = evas_coord_screen_x_to_world(evas, screen_x);
 * @endcode
 */
EAPI Evas_Coord        evas_coord_screen_x_to_world(const Evas *e, int x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Convert/scale an output screen co-ordinate into canvas co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param y The screen/output y co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a vertical co-ordinate as the @p y parameter
 * and converts it into canvas units, accounting for output size,
 * viewport size and location, returning it as the function return
 * value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern int screen_y;
 * Evas_Coord canvas_y;
 *
 * canvas_y = evas_coord_screen_y_to_world(evas, screen_y);
 * @endcode
 */
EAPI Evas_Coord        evas_coord_screen_y_to_world(const Evas *e, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The canvas x co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a horizontal co-ordinate as the @p x
 * parameter and converts it into output units, accounting for output
 * size, viewport size and location, returning it as the function
 * return value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int screen_x;
 * extern Evas_Coord canvas_x;
 *
 * screen_x = evas_coord_world_x_to_screen(evas, canvas_x);
 * @endcode
 */
EAPI int               evas_coord_world_x_to_screen(const Evas *e, Evas_Coord x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param y The canvas y co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a vertical co-ordinate as the @p x parameter
 * and converts it into output units, accounting for output size,
 * viewport size and location, returning it as the function return
 * value. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int screen_y;
 * extern Evas_Coord canvas_y;
 *
 * screen_y = evas_coord_world_y_to_screen(evas, canvas_y);
 * @endcode
 */
EAPI int               evas_coord_world_y_to_screen(const Evas *e, Evas_Coord y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Pointer_Group Pointer (Mouse) Functions
 *
 * Functions that deal with the status of the pointer (mouse cursor).
 *
 * @ingroup Evas_Canvas
 */

/**
 * This function returns the current known pointer co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to an integer to be filled in
 * @param y The pointer to an integer to be filled in
 * @ingroup Evas_Pointer_Group
 *
 * This function returns the current known screen/output co-ordinates
 * of the mouse pointer and sets the contents of the integers pointed
 * to by @p x and @p y to contain these co-ordinates. If @p e is not a
 * valid canvas the results of this function are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at screen position %i, %i\n", mouse_x, mouse_y);
 * @endcode
 */
EAPI void              evas_pointer_output_xy_get(const Evas *e, int *x, int *y) EINA_ARG_NONNULL(1);

/**
 * This function returns the current known pointer co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a Evas_Coord to be filled in
 * @param y The pointer to a Evas_Coord to be filled in
 * @ingroup Evas_Pointer_Group
 *
 * This function returns the current known canvas unit co-ordinates of
 * the mouse pointer and sets the contents of the Evas_Coords pointed
 * to by @p x and @p y to contain these co-ordinates. If @p e is not a
 * valid canvas the results of this function are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at canvas position %d, %d\n", mouse_x, mouse_y);
 * @endcode
 */
EAPI void              evas_pointer_canvas_xy_get(const Evas *e, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * Returns a bitmask with the mouse buttons currently pressed, set to 1
 *
 * @param e The pointer to the Evas Canvas
 * @return A bitmask of the currently depressed buttons on the canvas
 * @ingroup Evas_Pointer_Group
 *
 * Calling this function will return a 32-bit integer with the
 * appropriate bits set to 1 that correspond to a mouse button being
 * depressed. This limits Evas to a mouse devices with a maximum of 32
 * buttons, but that is generally in excess of any host system's
 * pointing device abilities.
 *
 * A canvas by default begins with no mouse buttons being pressed and
 * only calls to evas_event_feed_mouse_down(),
 * evas_event_feed_mouse_down_data(), evas_event_feed_mouse_up() and
 * evas_event_feed_mouse_up_data() will alter that.
 *
 * The least significant bit corresponds to the first mouse button
 * (button 1) and the most significant bit corresponds to the last
 * mouse button (button 32).
 *
 * If @p e is not a valid canvas, the return value is undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int button_mask, i;
 *
 * button_mask = evas_pointer_button_down_mask_get(evas);
 * printf("Buttons currently pressed:\n");
 * for (i = 0; i < 32; i++)
 *   {
 *     if ((button_mask & (1 << i)) != 0) printf("Button %i\n", i + 1);
 *   }
 * @endcode
 */
EAPI int               evas_pointer_button_down_mask_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Returns whether the mouse pointer is logically inside the canvas
 *
 * @param e The pointer to the Evas Canvas
 * @return An integer that is 1 if the mouse is inside the canvas, 0 otherwise
 * @ingroup Evas_Pointer_Group
 *
 * When this function is called it will return a value of either 0 or
 * 1, depending on if evas_event_feed_mouse_in(),
 * evas_event_feed_mouse_in_data(), or evas_event_feed_mouse_out(),
 * evas_event_feed_mouse_out_data() have been called to feed in a
 * mouse enter event into the canvas.
 *
 * A return value of 1 indicates the mouse is logically inside the
 * canvas, and 0 implies it is logically outside the canvas.
 *
 * A canvas begins with the mouse being assumed outside (0).
 *
 * If @p e is not a valid canvas, the return value is undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 *
 * if (evas_pointer_inside_get(evas)) printf("Mouse is in!\n");
 * else printf("Mouse is out!\n");
 * @endcode
 */
EAPI Eina_Bool         evas_pointer_inside_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void              evas_sync(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @ingroup Evas_Canvas_Events
 *
 * @{
 */

/**
 * Add (register) a callback function to a given canvas event.
 *
 * @param e Canvas to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param func The (callback) function to be called when the event is
 *        triggered
 * @param data The data pointer to be passed to @p func
 *
 * This function adds a function callback to the canvas @p e when the
 * event of type @p type occurs on it. The function pointer is @p
 * func.
 *
 * In the event of a memory allocation error during the addition of
 * the callback to the canvas, evas_alloc_error() should be used to
 * determine the nature of the error, if any, and the program should
 * sensibly try and recover.
 *
 * A callback function must have the ::Evas_Event_Cb prototype
 * definition. The first parameter (@p data) in this definition will
 * have the same value passed to evas_event_callback_add() as the @p
 * data parameter, at runtime. The second parameter @p e is the canvas
 * pointer on which the event occurred. The third parameter @p
 * event_info is a pointer to a data structure that may or may not be
 * passed to the callback, depending on the event type that triggered
 * the callback. This is so because some events don't carry extra
 * context with them, but others do.
 *
 * The event type @p type to trigger the function may be one of
 * #EVAS_CALLBACK_RENDER_FLUSH_PRE, #EVAS_CALLBACK_RENDER_FLUSH_POST,
 * #EVAS_CALLBACK_CANVAS_FOCUS_IN, #EVAS_CALLBACK_CANVAS_FOCUS_OUT,
 * #EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN and
 * #EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT. This determines the kind of
 * event that will trigger the callback to be called. Only the last
 * two of the event types listed here provide useful event information
 * data -- a pointer to the recently focused Evas object. For the
 * others the @p event_info pointer is going to be @c NULL.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_event_callback_add(d.canvas, EVAS_CALLBACK_RENDER_FLUSH_PRE
 * @until two canvas event callbacks
 *
 * Looking to the callbacks registered above,
 * @dontinclude evas-events.c
 * @skip called when our rectangle gets focus
 * @until let's have our events back
 *
 * we see that the canvas flushes its rendering pipeline
 * (#EVAS_CALLBACK_RENDER_FLUSH_PRE) whenever the @c _resize_cb
 * routine takes place: it has to redraw that image at a different
 * size. Also, the callback on an object being focused comes just
 * after we focus it explicitly, on code.
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @note Be careful not to add the same callback multiple times, if
 * that's not what you want, because Evas won't check if a callback
 * existed before exactly as the one being registered (and thus, call
 * it more than once on the event, in this case). This would make
 * sense if you passed different functions and/or callback data, only.
 */
EAPI void  evas_event_callback_add(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Add (register) a callback function to a given canvas event with a
 * non-default priority set. Except for the priority field, it's exactly the
 * same as @ref evas_event_callback_add
 *
 * @param e Canvas to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param priority The priority of the callback, lower values called first.
 * @param func The (callback) function to be called when the event is
 *        triggered
 * @param data The data pointer to be passed to @p func
 *
 * @see evas_event_callback_add
 * @since 1.1
 */
EAPI void  evas_event_callback_priority_add(Evas *e, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 4);

/**
 * Delete a callback function from the canvas.
 *
 * @param e Canvas to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * canvas @p e which was triggered by the event type @p type and was
 * calling the function @p func when triggered. If the removal is
 * successful it will also return the data pointer that was passed to
 * evas_event_callback_add() when the callback was added to the
 * canvas. If not successful @c NULL will be returned.
 *
 * Example:
 * @code
 * extern Evas *e;
 * void *my_data;
 * void focus_in_callback(void *data, Evas *e, void *event_info);
 *
 * my_data = evas_event_callback_del(ebject, EVAS_CALLBACK_CANVAS_FOCUS_IN, focus_in_callback);
 * @endcode
 */
EAPI void *evas_event_callback_del(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * Delete (unregister) a callback function registered to a given
 * canvas event.
 *
 * @param e Canvas to remove an event callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was
 *        triggered
 * @param data The data pointer that was to be passed to the callback
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes <b>the first</b> added callback from the
 * canvas @p e matching the event type @p type, the registered
 * function pointer @p func and the callback data pointer @p data. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_event_callback_add() (that will be the same as
 * the parameter) when the callback(s) was(were) added to the
 * canvas. If not successful @c NULL will be returned. A common use
 * would be to remove an exact match of a callback.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_event_callback_del_full(evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
 * @until _object_focus_in_cb, NULL);
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @note For deletion of canvas events callbacks filtering by just
 * type and function pointer, user evas_event_callback_del().
 */
EAPI void *evas_event_callback_del_full(Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Push a callback on the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 * @param data The data pointer to be passed to the callback
 *
 * Evas has a stack of callbacks that get called after all the callbacks for
 * an event have triggered (all the objects it triggers on and all the callbacks
 * in each object triggered). When all these have been called, the stack is
 * unwond from most recently to least recently pushed item and removed from the
 * stack calling the callback set for it.
 *
 * This is intended for doing reverse logic-like processing, example - when a
 * child object that happens to get the event later is meant to be able to
 * "steal" functions from a parent and thus on unwind of this stack have its
 * function called first, thus being able to set flags, or return 0 from the
 * post-callback that stops all other post-callbacks in the current stack from
 * being called (thus basically allowing a child to take control, if the event
 * callback prepares information ready for taking action, but the post callback
 * actually does the action).
 *
 */
EAPI void  evas_post_event_callback_push(Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * Remove a callback from the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 *
 * This removes a callback from the stack added with
 * evas_post_event_callback_push(). The first instance of the function in
 * the callback stack is removed from being executed when the stack is
 * unwound. Further instances may still be run on unwind.
 */
EAPI void  evas_post_event_callback_remove(Evas *e, Evas_Object_Event_Post_Cb func);

/**
 * Remove a callback from the post-event callback stack
 *
 * @param e Canvas to push the callback on
 * @param func The function that to be called when the stack is unwound
 * @param data The data pointer to be passed to the callback
 *
 * This removes a callback from the stack added with
 * evas_post_event_callback_push(). The first instance of the function and data
 * in the callback stack is removed from being executed when the stack is
 * unwound. Further instances may still be run on unwind.
 */
EAPI void  evas_post_event_callback_remove_full(Evas *e, Evas_Object_Event_Post_Cb func, const void *data);

/**
 * @}
 */

/**
 * @ingroup Evas_Event_Feeding_Group
 *
 * @{
 */

/**
 * Set the default set of flags an event begins with
 *
 * @param e The canvas to set the default event flags of
 * @param flags The default flags to use
 *
 * Events in evas can have an event_flags member. This starts out with
 * and initial value (no flags). This lets you set the default flags that
 * an event begins with to be @p flags
 *
 * @since 1.2
 */
EAPI void             evas_event_default_flags_set(Evas *e, Evas_Event_Flags flags) EINA_ARG_NONNULL(1);

/**
 * Get the default set of flags an event begins with
 *
 * @param e The canvas to get the default event flags from
 * @return The default event flags for that canvas
 *
 * This gets the default event flags events are produced with when fed in.
 *
 * @see evas_event_default_flags_set()
 * @since 1.2
 */
EAPI Evas_Event_Flags evas_event_default_flags_get(const Evas *e) EINA_ARG_NONNULL(1);

/**
 * Freeze all input events processing.
 *
 * @param e The canvas to freeze input events processing on.
 *
 * This function will indicate to Evas that the canvas @p e is to have
 * all input event processing frozen until a matching
 * evas_event_thaw() function is called on the same canvas. All events
 * of this kind during the freeze will get @b discarded. Every freeze
 * call must be matched by a thaw call in order to completely thaw out
 * a canvas (i.e. these calls may be nested). The most common use is
 * when you don't want the user to interact with your user interface
 * when you're populating a view or changing the layout.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip freeze input for 3 seconds
 * @until }
 * @dontinclude evas-events.c
 * @skip let's have our events back
 * @until }
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * If you run that example, you'll see the canvas ignoring all input
 * events for 3 seconds, when the "f" key is pressed. In a more
 * realistic code we would be freezing while a toolkit or Edje was
 * doing some UI changes, thawing it back afterwards.
 */
EAPI void             evas_event_freeze(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Thaw a canvas out after freezing (for input events).
 *
 * @param e The canvas to thaw out.
 *
 * This will thaw out a canvas after a matching evas_event_freeze()
 * call. If this call completely thaws out a canvas, i.e., there's no
 * other unbalanced call to evas_event_freeze(), events will start to
 * be processed again, but any "missed" events will @b not be
 * evaluated.
 *
 * See evas_event_freeze() for an example.
 */
EAPI void             evas_event_thaw(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Return the freeze count on input events of a given canvas.
 *
 * @param e The canvas to fetch the freeze count from.
 *
 * This returns the number of times the canvas has been told to freeze
 * input events. It is possible to call evas_event_freeze() multiple
 * times, and these must be matched by evas_event_thaw() calls. This
 * call allows the program to discover just how many times things have
 * been frozen in case it may want to break out of a deep freeze state
 * where the count is high.
 *
 * Example:
 * @code
 * extern Evas *evas;
 *
 * while (evas_event_freeze_get(evas) > 0) evas_event_thaw(evas);
 * @endcode
 *
 */
EAPI int              evas_event_freeze_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * After thaw of a canvas, re-evaluate the state of objects and call callbacks
 *
 * @param e The canvas to evaluate after a thaw
 *
 * This is normally called after evas_event_thaw() to re-evaluate mouse
 * containment and other states and thus also call callbacks for mouse in and
 * out on new objects if the state change demands it.
 */
EAPI void             evas_event_thaw_eval(Evas *e) EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @ingroup Evas_Event_Feeding_Group
 *
 * @{
 */

/**
 * Get the number of mouse or multi presses currently active
 *
 * @p e The given canvas pointer.
 * @return The number of presses (0 if none active).
 *
 * @since 1.2
 */
EAPI int  evas_event_down_count_get(const Evas *e) EINA_ARG_NONNULL(1);

/**
 * Mouse down event feed.
 *
 * @param e The given canvas pointer.
 * @param b The button number.
 * @param flags The evas button flags.
 * @param timestamp The timestamp of the mouse down event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse button is pressed. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void evas_event_feed_mouse_down(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse up event feed.
 *
 * @param e The given canvas pointer.
 * @param b The button number.
 * @param flags evas button flags.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse button is released. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void evas_event_feed_mouse_up(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse move event feed from input.
 *
 * @param e The given canvas pointer.
 * @param x The horizontal position of the mouse pointer relative to the 0,0 of
 * the window/surface.
 * @param y The vertical position of the mouse pointer relative to the 0,0 of
 * the window/surface.
 * @param timestamp The timestamp of the mouse move event.
 * @param data The data for canvas.
 *
 * Similar to the evas_event_feed_mouse_move(), this function will inform Evas
 * about mouse move events which were received by the input system, relative to
 * the 0,0 of the window, not to the canvas 0,0. It will take care of doing any
 * special transformation like adding the framespace offset to the mouse event.
 *
 * @since 1.8
 * @see evas_event_feed_mouse_move
 */
EAPI void evas_event_input_mouse_move(Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse move event feed.
 *
 * @param e The given canvas pointer.
 * @param x The horizontal position of the mouse pointer.
 * @param y The vertical position of the mouse pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse is moved from its last position. It prepares information
 * to be treated by the callback function.
 *
 */
EAPI void evas_event_feed_mouse_move(Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse in event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse in event happens. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void evas_event_feed_mouse_in(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse out event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp Timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse out event happens. It prepares information to be treated
 * by the callback function.
 *
 */
EAPI void evas_event_feed_mouse_out(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

EAPI void evas_event_feed_multi_down(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_feed_multi_up(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_feed_multi_move(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);

EAPI void evas_event_input_multi_down(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_input_multi_up(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
EAPI void evas_event_input_multi_move(Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);
/**
 * Mouse cancel event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will call evas_event_feed_mouse_up() when a
 * mouse cancel event happens.
 *
 */
EAPI void evas_event_feed_mouse_cancel(Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Mouse wheel event feed.
 *
 * @param e The given canvas pointer.
 * @param direction The wheel mouse direction.
 * @param z How much mouse wheel was scrolled up or down.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse wheel is scrolled up or down. It prepares information to
 * be treated by the callback function.
 *
 */
EAPI void evas_event_feed_mouse_wheel(Evas *e, int direction, int z, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Key down event feed
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key pressed.
 * @param string A String
 * @param compose The compose string
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is pressed. It prepares information to be treated by the
 * callback function.
 *
 */
EAPI void evas_event_feed_key_down(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Key up event feed
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key released.
 * @param string string
 * @param compose compose
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is released. It prepares information to be treated by the
 * callback function.
 *
 */
EAPI void evas_event_feed_key_up(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Key down event feed with keycode
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key pressed.
 * @param string A String
 * @param compose The compose string
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 * @param keycode Key scan code numeric value for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is pressed. It prepares information to be treated by the
 * callback function.
 *
 * @since 1.10
 */
EAPI void evas_event_feed_key_down_with_keycode(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode) EINA_ARG_NONNULL(1);

/**
 * Key up event feed with keycode
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key released.
 * @param string string
 * @param compose compose
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 * @param keycode Key scan code numeric value for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is released. It prepares information to be treated by the
 * callback function.
 *
 * @since 1.10
 */
EAPI void evas_event_feed_key_up_with_keycode(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data, unsigned int keycode) EINA_ARG_NONNULL(1);

/**
 * Hold event feed
 *
 * @param e The given canvas pointer.
 * @param hold The hold.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function makes the object to stop sending events.
 *
 */
EAPI void evas_event_feed_hold(Evas *e, int hold, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * Re feed event.
 *
 * @param e The given canvas pointer.
 * @param event_copy the event to refeed
 * @param event_type Event type
 *
 * This function re-feeds the event pointed by event_copy
 *
 * This function call evas_event_feed_* functions, so it can
 * cause havoc if not used wisely. Please use it responsibly.
 */
EAPI void evas_event_refeed_event(Evas *e, void *event_copy, Evas_Callback_Type event_type) EINA_ARG_NONNULL(1);

/**
 * Retrieve a list of Evas objects lying over a given position in
 * a canvas.
 *
 * @param e A handle to the canvas.
 * @param stop An Evas Object where to stop searching.
 * @param x The horizontal coordinate of the position.
 * @param y The vertical coordinate of the position.
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * given position. It will enter the smart objects.
 * It will not append to the list pass events as hidden objects.
 * Call eina_list_free on the returned list after usage.
 *
 */
EAPI Eina_List *evas_tree_objects_at_xy_get(Evas *e, Evas_Object *stop, int x, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @ingroup Evas_Image_Group
 *
 * @{
 */

/**
 * Flush the image cache of the canvas.
 *
 * @param e The given evas pointer.
 *
 * This function flushes image cache of canvas.
 *
 */
EAPI void      evas_image_cache_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Reload the image cache
 *
 * @param e The given evas pointer.
 *
 * This function reloads the image cache of canvas.
 *
 */
EAPI void      evas_image_cache_reload(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Set the image cache.
 *
 * @param e The given evas pointer.
 * @param size The cache size.
 *
 * This function sets the image cache of canvas in bytes.
 *
 */
EAPI void      evas_image_cache_set(Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * Get the image cache
 *
 * @param e The given evas pointer.
 *
 * This function returns the image cache size of canvas in bytes.
 *
 */
EAPI int       evas_image_cache_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the maximum image size evas can possibly handle
 *
 * @param e The given evas pointer.
 * @param maxw Pointer to hold the return value in pixels of the maximum width
 * @param maxh Pointer to hold the return value in pixels of the maximum height
 *
 * This function returns the larges image or surface size that evas can handle
 * in pixels, and if there is one, returns @c EINA_TRUE. It returns
 * @c EINA_FALSE if no extra constraint on maximum image size exists. You still
 * should check the return values of @p maxw and @p maxh as there may still be
 * a limit, just a much higher one.
 *
 * @since 1.1
 */
EAPI Eina_Bool evas_image_max_size_get(const Evas *e, int *maxw, int *maxh) EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @ingroup Evas_Font_Group
 *
 * @{
 */

/**
 * Changes the font hinting for the given evas.
 *
 * @param e The given evas.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_hinting_set(Evas *e, Evas_Font_Hinting_Flags hinting) EINA_ARG_NONNULL(1);

/**
 * Retrieves the font hinting used by the given evas.
 *
 * @param e The given evas to query.
 * @return The hinting in use, one of #EVAS_FONT_HINTING_NONE,
 *         #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI Evas_Font_Hinting_Flags evas_font_hinting_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Checks if the font hinting is supported by the given evas.
 *
 * @param e The given evas to query.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @return @c EINA_TRUE if it is supported, @c EINA_FALSE otherwise.
 * @ingroup Evas_Font_Group
 */
EAPI Eina_Bool               evas_font_hinting_can_hint(const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Force the given evas and associated engine to flush its font cache.
 *
 * @param e The given evas to flush font cache.
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_cache_flush(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @param size The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_cache_set(Evas *e, int size) EINA_ARG_NONNULL(1);

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @return The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI int                     evas_font_cache_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * List of available font descriptions known or found by this evas.
 *
 * The list depends on Evas compile time configuration, such as
 * fontconfig support, and the paths provided at runtime as explained
 * in @ref Evas_Font_Path_Group.
 *
 * @param e The evas instance to query.
 * @return a newly allocated list of strings. Do not change the
 *         strings.  Be sure to call evas_font_available_list_free()
 *         after you're done.
 *
 * @ingroup Evas_Font_Group
 */
EAPI Eina_List              *evas_font_available_list(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Free list of font descriptions returned by evas_font_dir_available_list().
 *
 * @param e The evas instance that returned such list.
 * @param available the list returned by evas_font_dir_available_list().
 *
 * @ingroup Evas_Font_Group
 */
EAPI void                    evas_font_available_list_free(Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Font_Path_Group Font Path Functions
 *
 * Functions that edit the paths being used to load fonts.
 *
 * @ingroup Evas_Font_Group
 */

/**
 * Removes all font paths loaded into memory for the given evas.
 * @param   e The given evas.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_clear(Evas *e) EINA_ARG_NONNULL(1);

/**
 * Appends a font path to the list of font paths used by the given evas.
 * @param   e    The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_append(Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * Prepends a font path to the list of font paths used by the given evas.
 * @param   e The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void                    evas_font_path_prepend(Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);

/**
 * Retrieves the list of font paths used by the given evas.
 * @param   e The given evas.
 * @return  The list of font paths used.
 * @ingroup Evas_Font_Path_Group
 */
EAPI const Eina_List        *evas_font_path_list(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Basic
 *
 * @{
 */




/**
 * Increments object reference count to defer its deletion.
 *
 * @param obj The given Evas object to reference
 *
 * This increments the reference count of an object, which if greater
 * than 0 will defer deletion by evas_object_del() until all
 * references are released back (counter back to 0). References cannot
 * go below 0 and unreferencing past that will result in the reference
 * count being limited to 0. References are limited to <c>2^32 - 1</c>
 * for an object. Referencing it more than this will result in it
 * being limited to this value.
 *
 * @see evas_object_unref()
 * @see evas_object_del()
 *
 * @note This is a <b>very simple</b> reference counting mechanism! For
 * instance, Evas is not ready to check for pending references on a
 * canvas deletion, or things like that. This is useful on scenarios
 * where, inside a code block, callbacks exist which would possibly
 * delete an object we are operating on afterwards. Then, one would
 * evas_object_ref() it on the beginning of the block and
 * evas_object_unref() it on the end. It would then be deleted at this
 * point, if it should be.
 *
 * Example:
 * @code
 *  evas_object_ref(obj);
 *
 *  // action here...
 *  evas_object_smart_callback_call(obj, SIG_SELECTED, NULL);
 *  // more action here...
 *  evas_object_unref(obj);
 * @endcode
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.1
 */
EAPI void             evas_object_ref(Evas_Object *obj);

/**
 * Decrements object reference count.
 *
 * @param obj The given Evas object to unreference
 *
 * This decrements the reference count of an object. If the object has
 * had evas_object_del() called on it while references were more than
 * 0, it will be deleted at the time this function is called and puts
 * the counter back to 0. See evas_object_ref() for more information.
 *
 * @see evas_object_ref() (for an example)
 * @see evas_object_del()
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.1
 */
EAPI void             evas_object_unref(Evas_Object *obj);

/**
 * Get the object reference count.
 *
 * @param obj The given Evas object to query
 *
 * This gets the reference count for an object (normally 0 until it is
 * referenced). Values of 1 or greater mean that someone is holding a
 * reference to this object that needs to be unreffed before it can be
 * deleted.
 *
 * @see evas_object_ref()
 * @see evas_object_unref()
 * @see evas_object_del()
 *
 * @ingroup Evas_Object_Group_Basic
 * @since 1.2
 */
EAPI int              evas_object_ref_get(const Evas_Object *obj);

/**
 * Marks the given Evas object for deletion (when Evas will free its
 * memory).
 *
 * @param obj The given Evas object.
 *
 * This call will mark @p obj for deletion, which will take place
 * whenever it has no more references to it (see evas_object_ref() and
 * evas_object_unref()).
 *
 * At actual deletion time, which may or may not be just after this
 * call, ::EVAS_CALLBACK_DEL and ::EVAS_CALLBACK_FREE callbacks will
 * be called. If the object currently had the focus, its
 * ::EVAS_CALLBACK_FOCUS_OUT callback will also be called.
 *
 * @see evas_object_ref()
 * @see evas_object_unref()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_del(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Retrieves the position and (rectangular) size of the given Evas
 * object.
 *
 * @param obj The given Evas object.
 * @param x Pointer to an integer in which to store the X coordinate
 *          of the object.
 * @param y Pointer to an integer in which to store the Y coordinate
 *          of the object.
 * @param w Pointer to an integer in which to store the width of the
 *          object.
 * @param h Pointer to an integer in which to store the height of the
 *          object.
 *
 * The position, naturally, will be relative to the top left corner of
 * the canvas' viewport.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip int w, h, cw, ch;
 * @until return
 *
 * See the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Set the position and (rectangular) size of the given Evas object.
 *
 * @param obj The given Evas object.
 * @param x   X position to move the object to, in canvas units.
 * @param y   Y position to move the object to, in canvas units.
 * @param w   The new width of the Evas object.
 * @param h   The new height of the Evas object.
 *
 * The position, naturally, will be relative to the top left corner of
 * the canvas' viewport.
 *
 * If the object get moved, the object's ::EVAS_CALLBACK_MOVE callback
 * will be called.
 *
 * If the object get resized, the object's ::EVAS_CALLBACK_RESIZE callback
 * will be called.
 *
 * @see evas_object_move()
 * @see evas_object_resize()
 * @see evas_object_geometry_get
 *
 * @since 1.8
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_geometry_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);


/**
 * Makes the given Evas object visible.
 *
 * @param obj The given Evas object.
 *
 * Besides becoming visible, the object's ::EVAS_CALLBACK_SHOW
 * callback will be called.
 *
 * @see evas_object_hide() for more on object visibility.
 * @see evas_object_visible_get()
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_show(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Makes the given Evas object invisible.
 *
 * @param obj The given Evas object.
 *
 * Hidden objects, besides not being shown at all in your canvas,
 * won't be checked for changes on the canvas rendering
 * process. Furthermore, they will not catch input events. Thus, they
 * are much ligher (in processing needs) than an object that is
 * invisible due to indirect causes, such as being clipped or out of
 * the canvas' viewport.
 *
 * Besides becoming hidden, @p obj object's ::EVAS_CALLBACK_SHOW
 * callback will be called.
 *
 * @note All objects are created in the hidden state! If you want them
 * shown, use evas_object_show() after their creation.
 *
 * @see evas_object_show()
 * @see evas_object_visible_get()
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip if (evas_object_visible_get(d.clipper))
 * @until return
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void             evas_object_hide(Evas_Object *obj) EINA_ARG_NONNULL(1);


#include "canvas/evas_common_interface.eo.legacy.h"
#include "canvas/evas_object.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Events
 *
 * @{
 */

/**
 * Add (register) a callback function to a given Evas object event.
 *
 * @param obj Object to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param func The function to be called when the event is triggered
 * @param data The data pointer to be passed to @p func
 *
 * This function adds a function callback to an object when the event
 * of type @p type occurs on object @p obj. The function is @p func.
 *
 * In the event of a memory allocation error during addition of the
 * callback to the object, evas_alloc_error() should be used to
 * determine the nature of the error, if any, and the program should
 * sensibly try and recover.
 *
 * A callback function must have the ::Evas_Object_Event_Cb prototype
 * definition. The first parameter (@p data) in this definition will
 * have the same value passed to evas_object_event_callback_add() as
 * the @p data parameter, at runtime. The second parameter @p e is the
 * canvas pointer on which the event occurred. The third parameter is
 * a pointer to the object on which event occurred. Finally, the
 * fourth parameter @p event_info is a pointer to a data structure
 * that may or may not be passed to the callback, depending on the
 * event type that triggered the callback. This is so because some
 * events don't carry extra context with them, but others do.
 *
 * The event type @p type to trigger the function may be one of
 * #EVAS_CALLBACK_MOUSE_IN, #EVAS_CALLBACK_MOUSE_OUT,
 * #EVAS_CALLBACK_MOUSE_DOWN, #EVAS_CALLBACK_MOUSE_UP,
 * #EVAS_CALLBACK_MOUSE_MOVE, #EVAS_CALLBACK_MOUSE_WHEEL,
 * #EVAS_CALLBACK_MULTI_DOWN, #EVAS_CALLBACK_MULTI_UP,
 * #EVAS_CALLBACK_MULTI_MOVE, #EVAS_CALLBACK_FREE,
 * #EVAS_CALLBACK_KEY_DOWN, #EVAS_CALLBACK_KEY_UP,
 * #EVAS_CALLBACK_FOCUS_IN, #EVAS_CALLBACK_FOCUS_OUT,
 * #EVAS_CALLBACK_SHOW, #EVAS_CALLBACK_HIDE, #EVAS_CALLBACK_MOVE,
 * #EVAS_CALLBACK_RESIZE, #EVAS_CALLBACK_RESTACK, #EVAS_CALLBACK_DEL,
 * #EVAS_CALLBACK_HOLD, #EVAS_CALLBACK_CHANGED_SIZE_HINTS,
 * #EVAS_CALLBACK_IMAGE_PRELOADED or #EVAS_CALLBACK_IMAGE_UNLOADED.
 *
 * This determines the kind of event that will trigger the callback.
 * What follows is a list explaining better the nature of each type of
 * event, along with their associated @p event_info pointers:
 *
 * - #EVAS_CALLBACK_MOUSE_IN: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_In struct\n\n
 *   This event is triggered when the mouse pointer enters the area
 *   (not shaded by other objects) of the object @p obj. This may
 *   occur by the mouse pointer being moved by
 *   evas_event_feed_mouse_move() calls, or by the object being shown,
 *   raised, moved, resized, or other objects being moved out of the
 *   way, hidden or lowered, whatever may cause the mouse pointer to
 *   get on top of @p obj, having been on top of another object
 *   previously.
 *
 * - #EVAS_CALLBACK_MOUSE_OUT: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Out struct\n\n
 *   This event is triggered exactly like #EVAS_CALLBACK_MOUSE_IN is,
 *   but it occurs when the mouse pointer exits an object's area. Note
 *   that no mouse out events will be reported if the mouse pointer is
 *   implicitly grabbed to an object (mouse buttons are down, having
 *   been pressed while the pointer was over that object). In these
 *   cases, mouse out events will be reported once all buttons are
 *   released, if the mouse pointer has left the object's area. The
 *   indirect ways of taking off the mouse pointer from an object,
 *   like cited above, for #EVAS_CALLBACK_MOUSE_IN, also apply here,
 *   naturally.
 *
 * - #EVAS_CALLBACK_MOUSE_DOWN: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Down struct\n\n
 *   This event is triggered by a mouse button being pressed while the
 *   mouse pointer is over an object. If the pointer mode for Evas is
 *   #EVAS_OBJECT_POINTER_MODE_AUTOGRAB (default), this causes this
 *   object to <b>passively grab the mouse</b> until all mouse buttons
 *   have been released: all future mouse events will be reported to
 *   only this object until no buttons are down. That includes mouse
 *   move events, mouse in and mouse out events, and further button
 *   presses. When all buttons are released, event propagation will
 *   occur as normal (see #Evas_Object_Pointer_Mode).
 *
 * - #EVAS_CALLBACK_MOUSE_UP: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Up struct\n\n
 *   This event is triggered by a mouse button being released while
 *   the mouse pointer is over an object's area (or when passively
 *   grabbed to an object).
 *
 * - #EVAS_CALLBACK_MOUSE_MOVE: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Move struct\n\n
 *   This event is triggered by the mouse pointer being moved while
 *   over an object's area (or while passively grabbed to an object).
 *
 * - #EVAS_CALLBACK_MOUSE_WHEEL: @p event_info is a pointer to an
 *   #Evas_Event_Mouse_Wheel struct\n\n
 *   This event is triggered by the mouse wheel being rolled while the
 *   mouse pointer is over an object (or passively grabbed to an
 *   object).
 *
 * - #EVAS_CALLBACK_MULTI_DOWN: @p event_info is a pointer to an
 *   #Evas_Event_Multi_Down struct
 *
 * - #EVAS_CALLBACK_MULTI_UP: @p event_info is a pointer to an
 *   #Evas_Event_Multi_Up struct
 *
 * - #EVAS_CALLBACK_MULTI_MOVE: @p event_info is a pointer to an
 *   #Evas_Event_Multi_Move struct
 *
 * - #EVAS_CALLBACK_FREE: @p event_info is @c NULL \n\n
 *   This event is triggered just before Evas is about to free all
 *   memory used by an object and remove all references to it. This is
 *   useful for programs to use if they attached data to an object and
 *   want to free it when the object is deleted. The object is still
 *   valid when this callback is called, but after it returns, there
 *   is no guarantee on the object's validity.
 *
 * - #EVAS_CALLBACK_KEY_DOWN: @p event_info is a pointer to an
 *   #Evas_Event_Key_Down struct\n\n
 *   This callback is called when a key is pressed and the focus is on
 *   the object, or a key has been grabbed to a particular object
 *   which wants to intercept the key press regardless of what object
 *   has the focus.
 *
 * - #EVAS_CALLBACK_KEY_UP: @p event_info is a pointer to an
 *   #Evas_Event_Key_Up struct \n\n
 *   This callback is called when a key is released and the focus is
 *   on the object, or a key has been grabbed to a particular object
 *   which wants to intercept the key release regardless of what
 *   object has the focus.
 *
 * - #EVAS_CALLBACK_FOCUS_IN: @p event_info is @c NULL \n\n
 *   This event is called when an object gains the focus. When it is
 *   called the object has already gained the focus.
 *
 * - #EVAS_CALLBACK_FOCUS_OUT: @p event_info is @c NULL \n\n
 *   This event is triggered when an object loses the focus. When it
 *   is called the object has already lost the focus.
 *
 * - #EVAS_CALLBACK_SHOW: @p event_info is @c NULL \n\n
 *   This event is triggered by the object being shown by
 *   evas_object_show().
 *
 * - #EVAS_CALLBACK_HIDE: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being hidden by
 *   evas_object_hide().
 *
 * - #EVAS_CALLBACK_MOVE: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being
 *   moved. evas_object_move() can trigger this, as can any
 *   object-specific manipulations that would mean the object's origin
 *   could move.
 *
 * - #EVAS_CALLBACK_RESIZE: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being resized. Resizes can
 *   be triggered by evas_object_resize() or by any object-specific
 *   calls that may cause the object to resize.
 *
 * - #EVAS_CALLBACK_RESTACK: @p event_info is @c NULL \n\n
 *   This event is triggered by an object being re-stacked. Stacking
 *   changes can be triggered by
 *   evas_object_stack_below()/evas_object_stack_above() and others.
 *
 * - #EVAS_CALLBACK_DEL: @p event_info is @c NULL.
 *
 * - #EVAS_CALLBACK_HOLD: @p event_info is a pointer to an
 *   #Evas_Event_Hold struct
 *
 * - #EVAS_CALLBACK_CHANGED_SIZE_HINTS: @p event_info is @c NULL.
 *
 * - #EVAS_CALLBACK_IMAGE_PRELOADED: @p event_info is @c NULL.
 *
 * - #EVAS_CALLBACK_IMAGE_UNLOADED: @p event_info is @c NULL.
 *
 * @note Be careful not to add the same callback multiple times, if
 * that's not what you want, because Evas won't check if a callback
 * existed before exactly as the one being registered (and thus, call
 * it more than once on the event, in this case). This would make
 * sense if you passed different functions and/or callback data, only.
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_object_event_callback_add(
 * @until }
 *
 * See the full example @ref Example_Evas_Events "here".
 *
 */
EAPI void      evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * Add (register) a callback function to a given Evas object event with a
 * non-default priority set. Except for the priority field, it's exactly the
 * same as @ref evas_object_event_callback_add
 *
 * @param obj Object to attach a callback to
 * @param type The type of event that will trigger the callback
 * @param priority The priority of the callback, lower values called first.
 * @param func The function to be called when the event is triggered
 * @param data The data pointer to be passed to @p func
 *
 * @see evas_object_event_callback_add
 * @since 1.1
 */
EAPI void      evas_object_event_callback_priority_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Callback_Priority priority, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 4);

/**
 * Delete a callback function from an object
 *
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was triggered
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * object @p obj which was triggered by the event type @p type and was
 * calling the function @p func when triggered. If the removal is
 * successful it will also return the data pointer that was passed to
 * evas_object_event_callback_add() when the callback was added to the
 * object. If not successful @c NULL will be returned.
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del(object, EVAS_CALLBACK_MOUSE_UP, up_callback);
 * @endcode
 */
EAPI void     *evas_object_event_callback_del(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func) EINA_ARG_NONNULL(1, 3);

/**
 * Delete (unregister) a callback function registered to a given
 * Evas object event.
 *
 * @param obj Object to remove a callback from
 * @param type The type of event that was triggering the callback
 * @param func The function that was to be called when the event was
 * triggered
 * @param data The data pointer that was to be passed to the callback
 * @return The data pointer that was to be passed to the callback
 *
 * This function removes the most recently added callback from the
 * object @p obj, which was triggered by the event type @p type and was
 * calling the function @p func with data @p data, when triggered. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_object_event_callback_add() (that will be the
 * same as the parameter) when the callback was added to the
 * object. In errors, @c NULL will be returned.
 *
 * @note For deletion of Evas object events callbacks filtering by
 * just type and function pointer, user
 * evas_object_event_callback_del().
 *
 * Example:
 * @code
 * extern Evas_Object *object;
 * void *my_data;
 * void up_callback(void *data, Evas *e, Evas_Object *obj, void *event_info);
 *
 * my_data = evas_object_event_callback_del_full(object, EVAS_CALLBACK_MOUSE_UP, up_callback, data);
 * @endcode
 */
EAPI void     *evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);


/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Map
 *
 * @{
 */

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Size_Hints
 *
 * @{
 */


/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Extras
 *
 * @{
 */

/**
 * Set an attached data pointer to an object with a given string key.
 *
 * @param obj The object to attach the data pointer to
 * @param key The string key for the data to access it
 * @param data The pointer to the data to be attached
 *
 * This attaches the pointer @p data to the object @p obj, given the
 * access string @p key. This pointer will stay "hooked" to the object
 * until a new pointer with the same string key is attached with
 * evas_object_data_set() or it is deleted with
 * evas_object_data_del(). On deletion of the object @p obj, the
 * pointers will not be accessible from the object anymore.
 *
 * You can find the pointer attached under a string key using
 * evas_object_data_get(). It is the job of the calling application to
 * free any data pointed to by @p data when it is no longer required.
 *
 * If @p data is @c NULL, the old value stored at @p key will be
 * removed but no new value will be stored. This is synonymous with
 * calling evas_object_data_del() with @p obj and @p key.
 *
 * @note This function is very handy when you have data associated
 * specifically to an Evas object, being of use only when dealing with
 * it. Than you don't have the burden to a pointer to it elsewhere,
 * using this family of functions.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = malloc(500);
 * evas_object_data_set(obj, "name_of_data", my_data);
 * printf("The data that was attached was %p\n", evas_object_data_get(obj, "name_of_data"));
 * @endcode
 */
EAPI void                     evas_object_data_set(Evas_Object *obj, const char *key, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Return an attached data pointer on an Evas object by its given
 * string key.
 *
 * @param obj The object to which the data was attached
 * @param key The string key the data was stored under
 * @return The data pointer stored, or @c NULL if none was stored
 *
 * This function will return the data pointer attached to the object
 * @p obj, stored using the string key @p key. If the object is valid
 * and a data pointer was stored under the given key, that pointer
 * will be returned. If this is not the case, @c NULL will be
 * returned, signifying an invalid object or a non-existent key. It is
 * possible that a @c NULL pointer was stored given that key, but this
 * situation is non-sensical and thus can be considered an error as
 * well. @c NULL pointers are never stored as this is the return value
 * if an error occurs.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_get(obj, "name_of_my_data");
 * if (my_data) printf("Data stored was %p\n", my_data);
 * else printf("No data was stored on the object\n");
 * @endcode
 */
EAPI void                    *evas_object_data_get(const Evas_Object *obj, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Delete an attached data pointer from an object.
 *
 * @param obj The object to delete the data pointer from
 * @param key The string key the data was stored under
 * @return The original data pointer stored at @p key on @p obj
 *
 * This will remove the stored data pointer from @p obj stored under
 * @p key and return this same pointer, if actually there was data
 * there, or @c NULL, if nothing was stored under that key.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_del(obj, "name_of_my_data");
 * @endcode
 */
EAPI void                    *evas_object_data_del(Evas_Object *obj, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Find
 *
 * @{
 */

/**
 * Retrieve the object that currently has focus.
 *
 * @param e The Evas canvas to query for focused object on.
 * @return The object that has focus or @c NULL if there is not one.
 *
 * Evas can have (at most) one of its objects focused at a time.
 * Focused objects will be the ones having <b>key events</b> delivered
 * to, which the programmer can act upon by means of
 * evas_object_event_callback_add() usage.
 *
 * @note Most users wouldn't be dealing directly with Evas' focused
 * objects. Instead, they would be using a higher level library for
 * that (like a toolkit, as Elementary) to handle focus and who's
 * receiving input for them.
 *
 * This call returns the object that currently has focus on the canvas
 * @p e or @c NULL, if none.
 *
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 *
 * Example:
 * @dontinclude evas-events.c
 * @skip evas_event_callback_add(d.canvas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
 * @until evas_object_focus_set(d.bg, EINA_TRUE);
 * @dontinclude evas-events.c
 * @skip called when our rectangle gets focus
 * @until }
 *
 * In this example the @c event_info is exactly a pointer to that
 * focused rectangle. See the full @ref Example_Evas_Events "example".
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_focus_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves the object on the given evas with the given name.
 * @param   e    The given evas.
 * @param   name The given name.
 * @return  If successful, the Evas object with the given name.  Otherwise,
 *          @c NULL.
 *
 * This looks for the evas object given a name by evas_object_name_set(). If
 * the name is not unique canvas-wide, then which one of the many objects
 * with that name is returned is undefined, so only use this if you can ensure
 * the object name is unique.
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_object_name_find(const Evas *e, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves the object from children of the given object with the given name.
 * @param   obj  The parent (smart) object whose children to search.
 * @param   name The given name.
 * @param   recurse Set to the number of child levels to recurse (0 == don't recurse, 1 == only look at the children of @p obj or their immediate children, but no further etc.).
 * @return  If successful, the Evas object with the given name.  Otherwise,
 *          @c NULL.
 *
 * This looks for the evas object given a name by evas_object_name_set(), but
 * it ONLY looks at the children of the object *p obj, and will only recurse
 * into those children if @p recurse is greater than 0. If the name is not
 * unique within immediate children (or the whole child tree) then it is not
 * defined which child object will be returned. If @p recurse is set to -1 then
 * it will recurse without limit.
 *
 * @since 1.2
 *
 * @ingroup Evas_Object_Group_Find
 */
EAPI Evas_Object *evas_object_name_child_find(const Evas_Object *obj, const char *name, int recurse) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve the Evas object stacked at the top of a given position in
 * a canvas
 *
 * @param   e A handle to the canvas.
 * @param   x The horizontal coordinate of the position
 * @param   y The vertical coordinate of the position
 * @param   include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation
 * @param   include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation
 * @return  The Evas object that is over all other objects at the given
 * position.
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * given position. The user can remove from the query
 * objects which are hidden and/or which are set to pass events.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve the Evas object stacked at the top at the position of the
 * mouse cursor, over a given canvas
 *
 * @param   e A handle to the canvas.
 * @return  The Evas object that is over all other objects at the mouse
 * pointer's position
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * mouse pointer's position, over @p e.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_at_pointer_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve the Evas object stacked at the top of a given rectangular
 * region in a canvas
 *
 * @param   e A handle to the canvas.
 * @param   x The top left corner's horizontal coordinate for the
 * rectangular region
 * @param   y The top left corner's vertical coordinate for the
 * rectangular region
 * @param   w The width of the rectangular region
 * @param   h The height of the rectangular region
 * @param   include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation
 * @param   include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation
 * @return  The Evas object that is over all other objects at the given
 * rectangular region.
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas overlapping
 * with the given rectangular region inside @p e. The user can remove
 * from the query objects which are hidden and/or which are set to
 * pass events.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieve a list of Evas objects lying over a given position in
 * a canvas
 *
 * @param   e A handle to the canvas.
 * @param   x The horizontal coordinate of the position
 * @param   y The vertical coordinate of the position
 * @param   include_pass_events_objects Boolean flag to include or not
 * objects which pass events in this calculation
 * @param   include_hidden_objects Boolean flag to include or not hidden
 * objects in this calculation
 * @return  The list of Evas objects that are over the given position
 * in @p e
 *
 * This function will traverse all the layers of the given canvas,
 * from top to bottom, querying for objects with areas covering the
 * given position. The user can remove from query
 * objects which are hidden and/or which are set to pass events.
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Eina_List   *evas_objects_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Eina_List   *evas_objects_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the lowest (stacked) Evas object on the canvas @p e.
 *
 * @param e a valid canvas pointer
 * @return a pointer to the lowest object on it, if any, or @c NULL,
 * otherwise
 *
 * This function will take all populated layers in the canvas into
 * account, getting the lowest object for the lowest layer, naturally.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 * @see evas_object_above_get()
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_bottom_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the highest (stacked) Evas object on the canvas @p e.
 *
 * @param e a valid canvas pointer
 * @return a pointer to the highest object on it, if any, or @c NULL,
 * otherwise
 *
 * This function will take all populated layers in the canvas into
 * account, getting the highest object for the highest layer,
 * naturally.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 * @see evas_object_above_get()
 *
 * @warning This function will @b skip objects parented by smart
 * objects, acting only on the ones at the "top level", with regard to
 * object parenting.
 */
EAPI Evas_Object *evas_object_top_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Interceptors
 *
 * @{
 */

/**
 * Set the callback function that intercepts a show event of a object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a show event
 * of a canvas object.
 *
 * @see evas_object_intercept_show_callback_del().
 *
 */
EAPI void  evas_object_intercept_show_callback_add(Evas_Object *obj, Evas_Object_Intercept_Show_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a show event of a
 * object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a show event
 * of a canvas object.
 *
 * @see evas_object_intercept_show_callback_add().
 *
 */
EAPI void *evas_object_intercept_show_callback_del(Evas_Object *obj, Evas_Object_Intercept_Show_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a hide event of a object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a hide event
 * of a canvas object.
 *
 * @see evas_object_intercept_hide_callback_del().
 *
 */
EAPI void  evas_object_intercept_hide_callback_add(Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a hide event of a
 * object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a hide event
 * of a canvas object.
 *
 * @see evas_object_intercept_hide_callback_add().
 *
 */
EAPI void *evas_object_intercept_hide_callback_del(Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a move event of a object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a move event
 * of a canvas object.
 *
 * @see evas_object_intercept_move_callback_del().
 *
 */
EAPI void  evas_object_intercept_move_callback_add(Evas_Object *obj, Evas_Object_Intercept_Move_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a move event of a
 * object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a move event
 * of a canvas object.
 *
 * @see evas_object_intercept_move_callback_add().
 *
 */
EAPI void *evas_object_intercept_move_callback_del(Evas_Object *obj, Evas_Object_Intercept_Move_Cb func) EINA_ARG_NONNULL(1, 2);

EAPI void  evas_object_intercept_resize_callback_add(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_resize_callback_del(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_raise_callback_add(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_raise_callback_del(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_lower_callback_add(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_lower_callback_del(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_stack_above_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_stack_above_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_stack_below_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_stack_below_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_layer_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_layer_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_color_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_color_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_clip_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_clip_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_clip_unset_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_clip_unset_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func) EINA_ARG_NONNULL(1, 2);
EAPI void  evas_object_intercept_focus_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Focus_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
EAPI void *evas_object_intercept_focus_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Focus_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Rectangle
 *
 * @{
 */

/**
 * Adds a rectangle to the given evas.
 * @param   e The given evas.
 * @return  The new rectangle object.
 *
 * @ingroup Evas_Object_Rectangle
 */
EAPI Evas_Object *evas_object_rectangle_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/evas_rectangle.eo.legacy.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Image
 *
 * @{
 */
/**
 * Creates a new image object on the given Evas @p e canvas.
 *
 * @param e The given canvas.
 * @return The created image object handle.
 *
 * @note If you intend to @b display an image somehow in a GUI,
 * besides binding it to a real image file/source (with
 * evas_object_image_file_set(), for example), you'll have to tell
 * this image object how to fill its space with the pixels it can get
 * from the source. See evas_object_image_filled_add(), for a helper
 * on the common case of scaling up an image source to the whole area
 * of the image object.
 *
 * @see evas_object_image_fill_set()
 *
 * Example:
 * @code
 * img = evas_object_image_add(canvas);
 * evas_object_image_file_set(img, "/path/to/img", NULL);
 * @endcode
 */
EAPI Evas_Object                  *evas_object_image_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Creates a new image object that @b automatically scales its bound
 * image to the object's area, on both axis.
 *
 * @param e The given canvas.
 * @return The created image object handle.
 *
 * This is a helper function around evas_object_image_add() and
 * evas_object_image_filled_set(). It has the same effect of applying
 * those functions in sequence, which is a very common use case.
 *
 * @note Whenever this object gets resized, the bound image will be
 * rescaled, too.
 *
 * @see evas_object_image_add()
 * @see evas_object_image_filled_set()
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Object                  *evas_object_image_filled_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Sets the data for an image from memory to be loaded
 *
 * This is the same as evas_object_image_file_set() but the file to be loaded
 * may exist at an address in memory (the data for the file, not the filename
 * itself). The @p data at the address is copied and stored for future use, so
 * no @p data needs to be kept after this call is made. It will be managed and
 * freed for you when no longer needed. The @p size is limited to 2 gigabytes
 * in size, and must be greater than 0. A @c NULL @p data pointer is also
 * invalid. Set the filename to @c NULL to reset to empty state and have the
 * image file data freed from memory using evas_object_image_file_set().
 *
 * The @p format is optional (pass @c NULL if you don't need/use it). It is
 * used to help Evas guess better which loader to use for the data. It may
 * simply be the "extension" of the file as it would normally be on disk
 * such as "jpg" or "png" or "gif" etc.
 *
 * @param obj The given image object.
 * @param data The image file data address
 * @param size The size of the image file data in bytes
 * @param format The format of the file (optional), or @c NULL if not needed
 * @param key The image key in file, or @c NULL.
 */
EAPI void                          evas_object_image_memfile_set(Evas_Object *obj, void *data, int size, char *format, char *key) EINA_ARG_NONNULL(1, 2);

/**
 * Set the source file from where an image object must fetch the real
 * image data (it may be an Eet file, besides pure image ones).
 *
 * @param obj The given image object.
 * @param file The image file path.
 * @param key The image key in @p file (if its an Eet one), or @c
 * NULL, otherwise.
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the image in
 * this file.
 *
 * Example:
 * @code
 * img = evas_object_image_add(canvas);
 * evas_object_image_file_set(img, "/path/to/img", NULL);
 * err = evas_object_image_load_error_get(img);
 * if (err != EVAS_LOAD_ERROR_NONE)
 *   {
 *      fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
 *              valid_path, evas_load_error_str(err));
 *   }
 * else
 *   {
 *      evas_object_image_fill_set(img, 0, 0, w, h);
 *      evas_object_resize(img, w, h);
 *      evas_object_show(img);
 *   }
 * @endcode
 */
EAPI void                          evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key) EINA_ARG_NONNULL(1);

/**
 * Set the source mmaped file from where an image object must fetch the real
 * image data (it must be an Eina_File).
 *
 * @param obj The given image object.
 * @param f The mmaped file
 * @param key The image key in @p file (if its an Eet one), or @c
 * NULL, otherwise.
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the image in
 * this file.
 *
 * @since 1.8
 */
EAPI void                          evas_object_image_mmap_set(Evas_Object *obj, const Eina_File *f, const char *key);

/**
 * Get the source mmaped file from where an image object must fetch the real
 * image data (it must be an Eina_File).
 *
 * @param obj The given image object.
 * @param f The mmaped file
 * @param key The image key in @p file (if its an Eet one), or @c
 * NULL, otherwise.
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can get the key to be used as the index of the image in
 * this file.
 *
 * @since 1.10
 */
EAPI void                          evas_object_image_mmap_get(const Evas_Object *obj, const Eina_File **f, const char **key);

/**
 * Retrieve the source file from where an image object is to fetch the
 * real image data (it may be an Eet file, besides pure image ones).
 *
 * @param obj The given image object.
 * @param file Location to store the image file path.
 * @param key Location to store the image key (if @p file is an Eet
 * one).
 *
 * You must @b not modify the strings on the returned pointers.
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 */
EAPI void                          evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key) EINA_ARG_NONNULL(1);

/**
 * Set the dimensions for an image object's border, a region which @b
 * won't ever be scaled together with its center.
 *
 * @param obj The given image object.
 * @param l The border's left width.
 * @param r The border's right width.
 * @param t The border's top width.
 * @param b The border's bottom width.
 *
 * When Evas is rendering, an image source may be scaled to fit the
 * size of its image object. This function sets an area from the
 * borders of the image inwards which is @b not to be scaled. This
 * function is useful for making frames and for widget theming, where,
 * for example, buttons may be of varying sizes, but their border size
 * must remain constant.
 *
 * The units used for @p l, @p r, @p t and @p b are canvas units.
 *
 * @note The border region itself @b may be scaled by the
 * evas_object_image_border_scale_set() function.
 *
 * @note By default, image objects have no borders set, i. e. @c l, @c
 * r, @c t and @c b start as @c 0.
 *
 * See the following figures for visual explanation:\n
 * @htmlonly
 * <img src="image-borders.png" style="max-width: 100%;" />
 * <a href="image-borders.png">Full-size</a>
 * @endhtmlonly
 * @image rtf image-borders.png
 * @image latex image-borders.eps width=\textwidth
 * @htmlonly
 * <img src="border-effect.png" style="max-width: 100%;" />
 * <a href="border-effect.png">Full-size</a>
 * @endhtmlonly
 * @image rtf border-effect.png
 * @image latex border-effect.eps width=\textwidth
 *
 * @see evas_object_image_border_get()
 * @see evas_object_image_border_center_fill_set()
 */
EAPI void                          evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b) EINA_ARG_NONNULL(1);

/**
 * Retrieve the dimensions for an image object's border, a region
 * which @b won't ever be scaled together with its center.
 *
 * @param obj The given image object.
 * @param l Location to store the border's left width in.
 * @param r Location to store the border's right width in.
 * @param t Location to store the border's top width in.
 * @param b Location to store the border's bottom width in.
 *
 * @note Use @c NULL pointers on the border components you're not
 * interested in: they'll be ignored by the function.
 *
 * See @ref evas_object_image_border_set() for more details.
 */
EAPI void                          evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * Sets @b how the center part of the given image object (not the
 * borders) should be drawn when Evas is rendering it.
 *
 * @param obj The given image object.
 * @param fill Fill mode of the center region of @p obj (a value in
 * #Evas_Border_Fill_Mode).
 *
 * This function sets how the center part of the image object's source
 * image is to be drawn, which must be one of the values in
 * #Evas_Border_Fill_Mode. By center we mean the complementary part of
 * that defined by evas_object_image_border_set(). This one is very
 * useful for making frames and decorations. You would most probably
 * also be using a filled image (as in evas_object_image_filled_set())
 * to use as a frame.
 *
 * @see evas_object_image_border_center_fill_get()
 */
EAPI void                          evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill) EINA_ARG_NONNULL(1);

/**
 * Retrieves @b how the center part of the given image object (not the
 * borders) is to be drawn when Evas is rendering it.
 *
 * @param obj The given image object.
 * @return fill Fill mode of the center region of @p obj (a value in
 * #Evas_Border_Fill_Mode).
 *
 * See @ref evas_object_image_fill_set() for more details.
 */
EAPI Evas_Border_Fill_Mode         evas_object_image_border_center_fill_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set whether the image object's fill property should track the
 * object's size.
 *
 * @param obj The given image object.
 * @param setting @c EINA_TRUE, to make the fill property follow
 *        object size or @c EINA_FALSE, otherwise.
 *
 * If @p setting is @c EINA_TRUE, then every evas_object_resize() will
 * @b automatically trigger a call to evas_object_image_fill_set()
 * with the that new size (and @c 0, @c 0 as source image's origin),
 * so the bound image will fill the whole object's area.
 *
 * @see evas_object_image_filled_add()
 * @see evas_object_image_fill_get()
 */
EAPI void                          evas_object_image_filled_set(Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);

/**
 * Retrieve whether the image object's fill property should track the
 * object's size.
 *
 * @param obj The given image object.
 * @return @c EINA_TRUE if it is tracking, @c EINA_FALSE, if not (and
 *         evas_object_fill_set() must be called manually).
 *
 * @see evas_object_image_filled_set() for more information
 */
EAPI Eina_Bool                     evas_object_image_filled_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the scaling factor (multiplier) for the borders of an image
 * object.
 *
 * @param obj The given image object.
 * @param scale The scale factor (default is @c 1.0 - i.e. no scaling)
 *
 * @see evas_object_image_border_set()
 * @see evas_object_image_border_scale_get()
 */
EAPI void                          evas_object_image_border_scale_set(Evas_Object *obj, double scale);

/**
 * Retrieves the scaling factor (multiplier) for the borders of an
 * image object.
 *
 * @param obj The given image object.
 * @return The scale factor set for its borders
 *
 * @see evas_object_image_border_set()
 * @see evas_object_image_border_scale_set()
 */
EAPI double                        evas_object_image_border_scale_get(const Evas_Object *obj);

/**
 * Set how to fill an image object's drawing rectangle given the
 * (real) image bound to it.
 *
 * @param obj The given image object to operate on.
 * @param x The x coordinate (from the top left corner of the bound
 *          image) to start drawing from.
 * @param y The y coordinate (from the top left corner of the bound
 *          image) to start drawing from.
 * @param w The width the bound image will be displayed at.
 * @param h The height the bound image will be displayed at.
 *
 * Note that if @p w or @p h are smaller than the dimensions of
 * @p obj, the displayed image will be @b tiled around the object's
 * area. To have only one copy of the bound image drawn, @p x and @p y
 * must be 0 and @p w and @p h need to be the exact width and height
 * of the image object itself, respectively.
 *
 * See the following image to better understand the effects of this
 * call. On this diagram, both image object and original image source
 * have @c a x @c a dimensions and the image itself is a circle, with
 * empty space around it:
 *
 * @image html image-fill.png
 * @image rtf image-fill.png
 * @image latex image-fill.eps
 *
 * @warning The default values for the fill parameters are @p x = 0,
 * @p y = 0, @p w = 0 and @p h = 0. Thus, if you're not using the
 * evas_object_image_filled_add() helper and want your image
 * displayed, you'll have to set valid values with this function on
 * your object.
 *
 * @note evas_object_image_filled_set() is a helper function which
 * will @b override the values set here automatically, for you, in a
 * given way.
 */
EAPI void                          evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);

/**
 * Retrieve how an image object is to fill its drawing rectangle,
 * given the (real) image bound to it.
 *
 * @param obj The given image object.
 * @param x Location to store the x coordinate (from the top left
 *          corner of the bound image) to start drawing from.
 * @param y Location to store the y coordinate (from the top left
 *          corner of the bound image) to start drawing from.
 * @param w Location to store the width the bound image is to be
 *          displayed at.
 * @param h Location to store the height the bound image is to be
 *          displayed at.
 *
 * @note Use @c NULL pointers on the fill components you're not
 * interested in: they'll be ignored by the function.
 *
 * See @ref evas_object_image_fill_set() for more details.
 */
EAPI void                          evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * Sets the tiling mode for the given evas image object's fill.
 * @param   obj   The given evas image object.
 * @param   spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 */
EAPI void                          evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread) EINA_ARG_NONNULL(1);

/**
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @param   obj The given evas image object.
 * @return  The current spread mode of the image object.
 */
EAPI Evas_Fill_Spread              evas_object_image_fill_spread_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the size of the given image object.
 *
 * @param obj The given image object.
 * @param w The new width of the image.
 * @param h The new height of the image.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 */
EAPI void                          evas_object_image_size_set(Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the size of the given image object.
 *
 * @param obj The given image object.
 * @param w Location to store the width of the image in, or @c NULL.
 * @param h Location to store the height of the image in, or @c NULL.
 *
 * See @ref evas_object_image_size_set() for more details.
 */
EAPI void                          evas_object_image_size_get(const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Retrieves the row stride of the given image object.
 *
 * @param obj The given image object.
 * @return The stride of the image (<b>in bytes</b>).
 *
 * The row stride is the number of bytes between the start of a row
 * and the start of the next row for image data.
 */
EAPI int                           evas_object_image_stride_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Retrieves a number representing any error that occurred during the
 * last loading of the given image object's source image.
 *
 * @param obj The given image object.
 * @return A value giving the last error that occurred. It should be
 *         one of the #Evas_Load_Error values. #EVAS_LOAD_ERROR_NONE
 *         is returned if there was no error.
 */
EAPI Evas_Load_Error               evas_object_image_load_error_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets the raw image data of the given image object.
 *
 * @param obj The given image object.
 * @param data The raw data, or @c NULL.
 *
 * Note that the raw data must be of the same size (see
 * evas_object_image_size_set(), which has to be called @b before this
 * one) and colorspace (see evas_object_image_colorspace_set()) of the
 * image. If data is @c NULL, the current image data will be
 * freed. Naturally, if one does not set an image object's data
 * manually, it will still have one, allocated by Evas.
 *
 * @see evas_object_image_data_get()
 */
EAPI void                          evas_object_image_data_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Get a pointer to the raw image data of the given image object.
 *
 * @param obj The given image object.
 * @param for_writing Whether the data being retrieved will be
 *        modified (@c EINA_TRUE) or not (@c EINA_FALSE).
 * @return The raw image data.
 *
 * This function returns a pointer to an image object's internal pixel
 * buffer, for reading only or read/write. If you request it for
 * writing, the image will be marked dirty so that it gets redrawn at
 * the next update.
 *
 * Each time you call this function on an image object, its data
 * buffer will have an internal reference counter
 * incremented. Decrement it back by using
 * evas_object_image_data_set().
 *
 * This is best suited for when you want to modify an existing image,
 * without changing its dimensions.
 *
 * @note The contents' format returned by it depend on the color
 * space of the given image object.
 *
 * @note You may want to use evas_object_image_data_update_add() to
 * inform data changes, if you did any.
 *
 * @see evas_object_image_data_set()
 */
EAPI void                         *evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * Note that this function does not modify the raw image data.  If the
 * requested colorspace is the same as the image colorspace nothing is
 * done and @c NULL is returned. You should use
 * evas_object_image_colorspace_get() to check the current image
 * colorspace.
 *
 * See @ref evas_object_image_colorspace_get.
 *
 * @param obj The given image object.
 * @param to_cspace The colorspace to which the image raw data will be converted.
 * @return data A newly allocated data in the format specified by to_cspace.
 */
EAPI void                         *evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Replaces the raw image data of the given image object.
 *
 * @param obj The given image object.
 * @param data The raw data to replace.
 *
 * This function lets the application replace an image object's
 * internal pixel buffer with an user-allocated one. For best results,
 * you should generally first call evas_object_image_size_set() with
 * the width and height for the new buffer.
 *
 * This call is best suited for when you will be using image data with
 * different dimensions than the existing image data, if any. If you
 * only need to modify the existing image in some fashion, then using
 * evas_object_image_data_get() is probably what you are after.
 *
 * Note that the caller is responsible for freeing the buffer when
 * finished with it, as user-set image data will not be automatically
 * freed when the image object is deleted.
 *
 * See @ref evas_object_image_data_get() for more details.
 *
 */
EAPI void                          evas_object_image_data_copy_set(Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);

/**
 * Mark a sub-region of the given image object to be redrawn.
 *
 * @param obj The given image object.
 * @param x X-offset of the region to be updated.
 * @param y Y-offset of the region to be updated.
 * @param w Width of the region to be updated.
 * @param h Height of the region to be updated.
 *
 * This function schedules a particular rectangular region of an image
 * object to be updated (redrawn) at the next rendering cycle.
 */
EAPI void                          evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Enable or disable alpha channel usage on the given image object.
 *
 * @param obj The given image object.
 * @param has_alpha Whether to use alpha channel (@c EINA_TRUE) data
 * or not (@c EINA_FALSE).
 *
 * This function sets a flag on an image object indicating whether or
 * not to use alpha channel data. A value of @c EINA_TRUE makes it use
 * alpha channel data, and @c EINA_FALSE makes it ignore that
 * data. Note that this has nothing to do with an object's color as
 * manipulated by evas_object_color_set().
 *
 * @see evas_object_image_alpha_get()
 */
EAPI void                          evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool has_alpha) EINA_ARG_NONNULL(1);

/**
 * Retrieve whether alpha channel data is being used on the given
 * image object.
 *
 * @param obj The given image object.
 * @return Whether the alpha channel data is being used (@c EINA_TRUE)
 * or not (@c EINA_FALSE).
 *
 * This function returns @c EINA_TRUE if the image object's alpha
 * channel is being used, or @c EINA_FALSE otherwise.
 *
 * See @ref evas_object_image_alpha_set() for more details.
 */
EAPI Eina_Bool                     evas_object_image_alpha_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Sets whether to use high-quality image scaling algorithm on the
 * given image object.
 *
 * @param obj The given image object.
 * @param smooth_scale Whether to use smooth scale or not.
 *
 * When enabled, a higher quality image scaling algorithm is used when
 * scaling images to sizes other than the source image's original
 * one. This gives better results but is more computationally
 * expensive.
 *
 * @note Image objects get created originally with smooth scaling @b
 * on.
 *
 * @see evas_object_image_smooth_scale_get()
 */
EAPI void                          evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether the given image object is using high-quality
 * image scaling algorithm.
 *
 * @param obj The given image object.
 * @return Whether smooth scale is being used.
 *
 * See @ref evas_object_image_smooth_scale_set() for more details.
 */
EAPI Eina_Bool                     evas_object_image_smooth_scale_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Preload an image object's image data in the background
 *
 * @param obj The given image object.
 * @param cancel @c EINA_FALSE will add it the preloading work queue,
 *               @c EINA_TRUE will remove it (if it was issued before).
 *
 * This function requests the preload of the data image in the
 * background. The work is queued before being processed (because
 * there might be other pending requests of this type).
 *
 * Whenever the image data gets loaded, Evas will call
 * #EVAS_CALLBACK_IMAGE_PRELOADED registered callbacks on @p obj (what
 * may be immediately, if the data was already preloaded before).
 *
 * Use @c EINA_TRUE for @p cancel on scenarios where you don't need
 * the image data preloaded anymore.
 *
 * @note Any evas_object_show() call after evas_object_image_preload()
 * will make the latter to be @b cancelled, with the loading process
 * now taking place @b synchronously (and, thus, blocking the return
 * of the former until the image is loaded). It is highly advisable,
 * then, that the user preload an image with it being @b hidden, just
 * to be shown on the #EVAS_CALLBACK_IMAGE_PRELOADED event's callback.
 */
EAPI void                          evas_object_image_preload(Evas_Object *obj, Eina_Bool cancel) EINA_ARG_NONNULL(1);

/**
 * Reload an image object's image data.
 *
 * @param obj The given image object pointer.
 *
 * This function reloads the image data bound to image object @p obj.
 */
EAPI void                          evas_object_image_reload(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Save the given image object's contents to an (image) file.
 *
 * @param obj The given image object.
 * @param file The filename to be used to save the image (extension
 *        obligatory).
 * @param key The image key in the file (if an Eet one), or @c NULL,
 *        otherwise.
 * @param flags String containing the flags to be used (@c NULL for
 *        none).
 *
 * The extension suffix on @p file will determine which <b>saver
 * module</b> Evas is to use when saving, thus the final file's
 * format. If the file supports multiple data stored in it (Eet ones),
 * you can specify the key to be used as the index of the image in it.
 *
 * You can specify some flags when saving the image.  Currently
 * acceptable flags are @c quality and @c compress. Eg.: @c
 * "quality=100 compress=9"
 */
EAPI Eina_Bool                     evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)  EINA_ARG_NONNULL(1, 2);

/**
 * Import pixels from given source to a given canvas image object.
 *
 * @param obj The given canvas object.
 * @param pixels The pixel's source to be imported.
 *
 * This function imports pixels from a given source to a given canvas image.
 *
 */
EAPI Eina_Bool                     evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function to get pixels from a canvas' image.
 *
 * @param obj The given canvas pointer.
 * @param func The callback function.
 * @param data The data pointer to be passed to @a func.
 *
 * This functions sets a function to be the callback function that get
 * pixels from a image of the canvas.
 *
 */
EAPI void                          evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Mark whether the given image object is dirty and needs to request its pixels.
 *
 * @param obj The given image object.
 * @param dirty Whether the image is dirty.
 *
 * This function will only properly work if a pixels get callback has been set.
 *
 * @warning use this function if you really know what you are doing.
 *
 * @see evas_object_image_pixels_get_callback_set()
 */
EAPI void                          evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty) EINA_ARG_NONNULL(1);

/**
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @return Whether the image is dirty.
 */
EAPI Eina_Bool                     evas_object_image_pixels_dirty_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the DPI resolution of an image object's source image.
 *
 * @param obj The given canvas pointer.
 * @param dpi The new DPI resolution.
 *
 * This function sets the DPI resolution of a given loaded canvas
 * image. Most useful for the SVG image loader.
 *
 * @see evas_object_image_load_dpi_get()
 */
EAPI void                          evas_object_image_load_dpi_set(Evas_Object *obj, double dpi) EINA_ARG_NONNULL(1);

/**
 * Get the DPI resolution of a loaded image object in the canvas.
 *
 * @param obj The given canvas pointer.
 * @return The DPI resolution of the given canvas image.
 *
 * This function returns the DPI resolution of the given canvas image.
 *
 * @see evas_object_image_load_dpi_set() for more details
 */
EAPI double                        evas_object_image_load_dpi_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the size of a given image object's source image, when loading
 * it.
 *
 * @param obj The given canvas object.
 * @param w The new width of the image's load size.
 * @param h The new height of the image's load size.
 *
 * This function sets a new (loading) size for the given canvas
 * image.
 *
 * @see evas_object_image_load_size_get()
 */
EAPI void                          evas_object_image_load_size_set(Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the size of a given image object's source image, when loading
 * it.
 *
 * @param obj The given image object.
 * @param w Where to store the new width of the image's load size.
 * @param h Where to store the new height of the image's load size.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see evas_object_image_load_size_set() for more details
 */
EAPI void                          evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Set the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param obj The given image object pointer.
 * @param scale_down The scale down factor.
 *
 * This function sets the scale down factor of a given canvas
 * image. Most useful for the SVG image loader.
 *
 * @see evas_object_image_load_scale_down_get()
 */
EAPI void                          evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down) EINA_ARG_NONNULL(1);

/**
 * get the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @param obj The given image object pointer.
 *
 * @see evas_object_image_load_scale_down_set() for more details
 */
EAPI int                           evas_object_image_load_scale_down_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Inform a given image object to load a selective region of its
 * source image.
 *
 * @param obj The given image object pointer.
 * @param x X-offset of the region to be loaded.
 * @param y Y-offset of the region to be loaded.
 * @param w Width of the region to be loaded.
 * @param h Height of the region to be loaded.
 *
 * This function is useful when one is not showing all of an image's
 * area on its image object.
 *
 * @note The image loader for the image format in question has to
 * support selective region loading in order to this function to take
 * effect.
 *
 * @see evas_object_image_load_region_get()
 */
EAPI void                          evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Retrieve the coordinates of a given image object's selective
 * (source image) load region.
 *
 * @param obj The given image object pointer.
 * @param x Where to store the X-offset of the region to be loaded.
 * @param y Where to store the Y-offset of the region to be loaded.
 * @param w Where to store the width of the region to be loaded.
 * @param h Where to store the height of the region to be loaded.
 *
 * @note Use @c NULL pointers on the coordinates you're not interested
 * in: they'll be ignored by the function.
 *
 * @see evas_object_image_load_region_get()
 */
EAPI void                          evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Define if the orientation information in the image file should be honored.
 *
 * @param obj The given image object pointer.
 * @param enable @c EINA_TRUE means that it should honor the orientation information
 * @since 1.1
 */
EAPI void                          evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * Get if the orientation information in the image file should be honored.
 *
 * @param obj The given image object pointer.
 * @since 1.1
 */
EAPI Eina_Bool                     evas_object_image_load_orientation_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param cspace The new color space.
 *
 * This function sets the colorspace of given canvas image.
 *
 */
EAPI void                          evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace) EINA_ARG_NONNULL(1);

/**
 * Get the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @return The colorspace of the image.
 *
 * This function returns the colorspace of given canvas image.
 *
 */
EAPI Evas_Colorspace               evas_object_image_colorspace_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the support state of a given image
 *
 * @param obj The given image object pointer
 * @return The region support state
 * @since 1.2
 *
 * This function returns the state of the region support of given image
 */
EAPI Eina_Bool                     evas_object_image_region_support_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @param surf The new native surface.
 *
 * This function sets a native surface of a given canvas image.
 *
 */
EAPI void                          evas_object_image_native_surface_set(Evas_Object *obj, Evas_Native_Surface *surf) EINA_ARG_NONNULL(1, 2);

/**
 * Get the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @return The native surface of the given canvas image.
 *
 * This function returns the native surface of a given canvas image.
 *
 */
EAPI Evas_Native_Surface          *evas_object_image_native_surface_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the video surface linked to a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @param surf The new video surface.
 * @since 1.1
 *
 * This function links a video surface to a given canvas image.
 *
 */
EAPI void                          evas_object_image_video_surface_set(Evas_Object *obj, Evas_Video_Surface *surf) EINA_ARG_NONNULL(1);

/**
 * Get the video surface linekd to a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @return The video surface of the given canvas image.
 * @since 1.1
 *
 * This function returns the video surface linked to a given canvas image.
 *
 */
EAPI const Evas_Video_Surface     *evas_object_image_video_surface_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI void evas_object_image_video_surface_caps_set(Evas_Object *obj, unsigned int caps) EINA_ARG_NONNULL(1);
EAPI unsigned int evas_object_image_video_surface_caps_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the scale hint of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param hint The scale hint, a value in
 * #Evas_Image_Scale_Hint.
 *
 * This function sets the scale hint value of the given image object
 * in the canvas, which will affect how Evas is to cache scaled
 * versions of its original source image.
 *
 * @see evas_object_image_scale_hint_get()
 */
EAPI void                          evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint) EINA_ARG_NONNULL(1);

/**
 * Get the scale hint of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @return The scale hint value set on @p obj, a value in
 * #Evas_Image_Scale_Hint.
 *
 * This function returns the scale hint value of the given image
 * object of the canvas.
 *
 * @see evas_object_image_scale_hint_set() for more details.
 */
EAPI Evas_Image_Scale_Hint         evas_object_image_scale_hint_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Set the content hint setting of a given image object of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param hint The content hint value, one of the
 * #Evas_Image_Content_Hint ones.
 *
 * This function sets the content hint value of the given image of the
 * canvas. For example, if you're on the GL engine and your driver
 * implementation supports it, setting this hint to
 * #EVAS_IMAGE_CONTENT_HINT_DYNAMIC will make it need @b zero copies
 * at texture upload time, which is an "expensive" operation.
 *
 * @see evas_object_image_content_hint_get()
 */
EAPI void                          evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint) EINA_ARG_NONNULL(1);

/**
 * Get the content hint setting of a given image object of the canvas.
 *
 * @param obj The given canvas pointer.
 * @return hint The content hint value set on it, one of the
 * #Evas_Image_Content_Hint ones (#EVAS_IMAGE_CONTENT_HINT_NONE means
 * an error).
 *
 * This function returns the content hint value of the given image of
 * the canvas.
 *
 * @see evas_object_image_content_hint_set()
 */
EAPI Evas_Image_Content_Hint       evas_object_image_content_hint_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Enable an image to be used as an alpha mask.
 *
 * This will set any flags, and discard any excess image data not used as an
 * alpha mask.
 *
 * Note there is little point in using a image as alpha mask unless it has an
 * alpha channel.
 *
 * @param obj Object to use as an alpha mask.
 * @param ismask Use image as alphamask, must be true.
 */
EAPI void                          evas_object_image_alpha_mask_set(Evas_Object *obj, Eina_Bool ismask) EINA_ARG_NONNULL(1);

/**
 * Set the source object on an image object to used as a @b proxy.
 *
 * @param obj Proxy (image) object.
 * @param src Source object to use for the proxy.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * If an image object is set to behave as a @b proxy, it will mirror
 * the rendering contents of a given @b source object in its drawing
 * region, without affecting that source in any way. The source must
 * be another valid Evas object. Other effects may be applied to the
 * proxy, such as a map (see evas_object_map_set()) to create a
 * reflection of the original object (for example).
 *
 * Any existing source object on @p obj will be removed after this
 * call. Setting @p src to @c NULL clears the proxy object (not in
 * "proxy state" anymore).
 *
 * @warning You cannot set a proxy as another proxy's source.
 *
 * @see evas_object_image_source_get()
 * @see evas_object_image_source_unset()
 * @see evas_object_image_source_visible_set()
 */
EAPI Eina_Bool                     evas_object_image_source_set(Evas_Object *obj, Evas_Object *src) EINA_ARG_NONNULL(1);

/**
 * Get the current source object of an image object.
 *
 * @param obj Image object
 * @return Source object (if any), or @c NULL, if not in "proxy mode"
 * (or on errors).
 *
 * @see evas_object_image_source_set() for more details
 */
EAPI Evas_Object                  *evas_object_image_source_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Clear the source object on a proxy image object.
 *
 * @param obj Image object to clear source of.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * This is equivalent to calling evas_object_image_source_set() with a
 * @c NULL source.
 */
EAPI Eina_Bool                     evas_object_image_source_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the source object to be visible or not.
 *
 * @param obj Proxy (image) object.
 * @param visible @c EINA_TRUE is source object to be shown, @c EINA_FALSE 
 * otherwise.
 *
 * If the @p visible set to @c EINA_FALSE, the source object of the proxy(@p obj
 * ) will be invisible.
 *
 * This API works differently to evas_object_show() and evas_object_hide().
 * Once source object is hidden by evas_object_hide() then the proxy object will
 * be hidden as well. Actually in this case both objects are excluded from the
 * Evas internal update circle.
 *
 * By this API, instead, one can toggle the visibility of a proxy's source
 * object remaining the proxy visibility untouched.
 *
 * @warning If the all of proxies are deleted, then the source visibility of the
 * source object will be cancelled.
 *
 * @see evas_object_image_source_visible_get()
 * @see evas_object_image_source_set()
 * @see evas_object_show()
 * @see evas_object_hide()
 * @since 1.8
 */
EAPI void                          evas_object_image_source_visible_set(Evas_Object *obj, Eina_Bool visible) EINA_ARG_NONNULL(1);

/**
 * Get the state of the source object visibility.
 *
 * @param obj Proxy (image) object.
 * @return @c EINA_TRUE if source object is visible, @c EINA_FALSE otherwise.
 *
 * @see evas_object_image_source_visible_set()
 * @see evas_object_image_source_set()
 * @see evas_object_show()
 * @see evas_object_hide()
 * @since 1.8
 */
EAPI Eina_Bool                     evas_object_image_source_visible_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set whether an Evas object is to source events.
 *
 * @param obj Proxy (image) object.
 * @param source whether @p obj is to pass events (@c EINA_TRUE) or not
 * (@c EINA_FALSE)
 *
 * Set whether an Evas object is to repeat events to source.
 *
 * If @p source is @c EINA_TRUE, it will make events on @p obj to also be
 * repeated for the source object (see evas_object_image_source_set()). Even the
 * @p obj and source geometries are different, the event position will be
 * transformed to the source object's space.
 *
 * If @p source is @c EINA_FALSE, events occurring on @p obj will be
 * processed only on it.
 *
 * @see evas_object_image_source_get()
 * @see evas_object_image_source_visible_set()
 * @see evas_object_image_source_events_get()
 * @since 1.8
 */
EAPI void evas_object_image_source_events_set(Evas_Object *obj, Eina_Bool source) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is set to source events.
 *
 * @param obj Proxy (image) object.
 * @return @c EINA_TRUE if source object has events, @c EINA_FALSE otherwise.
 *
 * @see evas_object_image_source_set()
 * @see evas_object_image_source_visible_set()
 * @see evas_object_image_source_events_set()
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_events_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Clip the proxy object with the source object's clipper.
 *
 * @param obj Proxy (image) object.
 * @param source_clip whether @p obj is clipped by the source clipper.
 * (@c EINA_TRUE) or not (@c EINA_FALSE)
 *
 * @see evas_object_clip_set()
 * @see evas_object_image_source_set()
 * @since 1.8
 */
EAPI void evas_object_image_source_clip_set(Evas_Object *obj, Eina_Bool source_clip) EINA_ARG_NONNULL(1);

/**
 * Determine whether an object is clipped by source object's clipper.
 *
 * @param obj Proxy (image) object.
 * @return @c EINA_TRUE if source clip is enabled, @c EINA_FALSE otherwise.
 *
 * @see evas_object_clip_set()
 * @see evas_object_image_source_set()
 * @see evas_object_image_source_clip_set()
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_clip_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Check if an image object can be animated (have multiple frames)
 *
 * @param obj Image object
 * @return whether obj support animation
 *
 * This returns if the image file of an image object is capable of animation
 * such as an animated gif file might. This is only useful to be called once
 * the image object file has been set.
 *
 * Example:
 * @code
 * extern Evas_Object *obj;
 *
 * if (evas_object_image_animated_get(obj))
 *   {
 *     int frame_count;
 *     int loop_count;
 *     Evas_Image_Animated_Loop_Hint loop_type;
 *     double duration;
 *
 *     frame_count = evas_object_image_animated_frame_count_get(obj);
 *     printf("This image has %d frames\n",frame_count);
 *
 *     duration = evas_object_image_animated_frame_duration_get(obj,1,0);
 *     printf("Frame 1's duration is %f. You had better set object's frame to 2 after this duration using timer\n");
 *
 *     loop_count = evas_object_image_animated_loop_count_get(obj);
 *     printf("loop count is %d. You had better run loop %d times\n",loop_count,loop_count);
 *
 *     loop_type = evas_object_image_animated_loop_type_get(obj);
 *     if (loop_type == EVAS_IMAGE_ANIMATED_HINT_LOOP)
 *       printf("You had better set frame like 1->2->3->1->2->3...\n");
 *     else if (loop_type == EVAS_IMAGE_ANIMATED_HINT_PINGPONG)
 *       printf("You had better set frame like 1->2->3->2->1->2...\n");
 *     else
 *       printf("Unknown loop type\n");
 *
 *     evas_object_image_animated_frame_set(obj,1);
 *     printf("You set image object's frame to 1. You can see frame 1\n");
 *   }
 * @endcode
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI Eina_Bool                     evas_object_image_animated_get(const Evas_Object *obj);

/**
 * Get the total number of frames of the image object.
 *
 * @param obj Image object
 * @return The number of frames
 *
 * This returns total number of frames the image object supports (if animated)
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI int                           evas_object_image_animated_frame_count_get(const Evas_Object *obj);

/**
 * Get the kind of looping the image object does.
 *
 * @param obj Image object
 * @return Loop type of the image object
 *
 * This returns the kind of looping the image object wants to do.
 *
 * If it returns EVAS_IMAGE_ANIMATED_HINT_LOOP, you should display frames in a sequence like:
 * 1->2->3->1->2->3->1...
 * If it returns EVAS_IMAGE_ANIMATED_HINT_PINGPONG, it is better to
 * display frames in a sequence like: 1->2->3->2->1->2->3->1...
 *
 * The default type is EVAS_IMAGE_ANIMATED_HINT_LOOP.
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI Evas_Image_Animated_Loop_Hint evas_object_image_animated_loop_type_get(const Evas_Object *obj);

/**
 * Get the number times the animation of the object loops.
 *
 * @param obj Image object
 * @return The number of loop of an animated image object
 *
 * This returns loop count of image. The loop count is the number of times
 * the animation will play fully from first to last frame until the animation
 * should stop (at the final frame).
 *
 * If 0 is returned, then looping should happen indefinitely (no limit to
 * the number of times it loops).
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI int                           evas_object_image_animated_loop_count_get(const Evas_Object *obj);

/**
 * Get the duration of a sequence of frames.
 *
 * @param obj Image object
 * @param start_frame The first frame
 * @param fram_num Number of frames in the sequence
 *
 * This returns total duration that the specified sequence of frames should
 * take in seconds.
 *
 * If you set start_frame to 1 and frame_num 0, you get frame 1's duration
 * If you set start_frame to 1 and frame_num 1, you get frame 1's duration +
 * frame2's duration
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI double                        evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int fram_num);

/**
 * Set the frame to current frame of an image object
 *
 * @param obj The given image object.
 * @param frame_num The index of current frame
 *
 * This set image object's current frame to frame_num with 1 being the first
 * frame.
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 */
EAPI void                          evas_object_image_animated_frame_set(Evas_Object *obj, int frame_num);
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Text
 *
 * @{
 */

/**
 * Creates a new text object on the provided canvas.
 *
 * @param e The canvas to create the text object on.
 * @return @c NULL on error, a pointer to a new text object on
 * success.
 *
 * Text objects are for simple, single line text elements. If you want
 * more elaborated text blocks, see @ref Evas_Object_Textblock.
 *
 * @see evas_object_text_font_source_set()
 * @see evas_object_text_font_set()
 * @see evas_object_text_text_set()
 */
EAPI Evas_Object         *evas_object_text_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/evas_text.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Textblock
 *
 * @{
 */

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 */
EAPI Evas_Object                             *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Return the plain version of the markup.
 *
 * Works as if you set the markup to a textblock and then retrieve the plain
 * version of the text. i.e: <br> and <\n> will be replaced with \n, &...; with
 * the actual char and etc.
 *
 * @param obj The textblock object to work with. (if @c NULL, tries the
 * default).
 * @param text The markup text (if @c NULL, return @c NULL).
 * @return An allocated plain text version of the markup.
 * @since 1.2
 */
EAPI char                                    *evas_textblock_text_markup_to_utf8(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * Return the markup version of the plain text.
 *
 * Replaces \\n -\> \<br/\> \\t -\> \<tab/\> and etc. Generally needed before you pass
 * plain text to be set in a textblock.
 *
 * @param obj the textblock object to work with (if @c NULL, it just does the
 * default behaviour, i.e with no extra object information).
 * @param text The plain text (if @c NULL, return @c NULL).
 * @return An allocated markup version of the plain text.
 * @since 1.2
 */
EAPI char                                    *evas_textblock_text_utf8_to_markup(const Evas_Object *obj, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

#include "canvas/evas_textblock.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
/**
 * @brief Add a textgrid to the given Evas.
 *
 * @param e The given evas.
 * @return The new textgrid object.
 *
 * This function adds a new textgrid object to the Evas @p e and returns the object.
 *
 * @since 1.7
 */
EAPI Evas_Object *evas_object_textgrid_add(Evas *e);

#include "canvas/evas_textgrid.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Line
 *
 * @{
 */
/**
 * Adds a new evas line object to the given evas.
 * @param   e The given evas.
 * @return  The new evas line object.
 */
EAPI Evas_Object *evas_object_line_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/evas_line.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Polygon
 *
 * @{
 */
/**
 * Adds a new evas polygon object to the given evas.
 * @param   e The given evas.
 * @return  A new evas polygon object.
 */
EAPI Evas_Object *evas_object_polygon_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/evas_polygon.eo.legacy.h"
/**
 * @}
 */

/* @since 1.2 */
//EAPI void         evas_object_is_frame_object_set(Evas_Object *obj, Eina_Bool is_frame);

/* @since 1.2 */
//EAPI Eina_Bool    evas_object_is_frame_object_get(const Evas_Object *obj);

/**
 * @}
 */

/**
 * @ingroup Evas_Group_Smart
 *
 * @{
 */

/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Group
 *
 * @{
 */
/**
 * Instantiates a new smart object described by @p s.
 *
 * @param e the canvas on which to add the object
 * @param s the #Evas_Smart describing the smart object
 * @return a new #Evas_Object handle
 *
 * This is the function one should use when defining the public
 * function @b adding an instance of the new smart object to a given
 * canvas. It will take care of setting all of its internals to work
 * as they should, if the user set things properly, as seem on the
 * #EVAS_SMART_SUBCLASS_NEW, for example.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI Evas_Object *evas_object_smart_add(Evas *e, Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;

/**
 * Set an Evas object as a member of a given smart object.
 *
 * @param obj The member object
 * @param smart_obj The smart object
 *
 * Members will automatically be stacked and layered together with the
 * smart object. The various stacking functions will operate on
 * members relative to the other members instead of the entire canvas,
 * since they now live on an exclusive layer (see
 * evas_object_stack_above(), for more details).
 *
 * Any @p smart_obj object's specific implementation of the @c
 * member_add() smart function will take place too, naturally.
 *
 * @see evas_object_smart_member_del()
 * @see evas_object_smart_members_get()
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_member_add(Evas_Object *obj, Evas_Object *smart_obj) EINA_ARG_NONNULL(1, 2);

/**
 * Removes a member object from a given smart object.
 *
 * @param obj the member object
 * @ingroup Evas_Smart_Object_Group
 *
 * This removes a member object from a smart object, if it was added
 * to any. The object will still be on the canvas, but no longer
 * associated with whichever smart object it was associated with.
 *
 * @see evas_object_smart_member_add() for more details
 * @see evas_object_smart_members_get()
 */
EAPI void         evas_object_smart_member_del(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Add (register) a callback function to the smart event specified by
 * @p event on the smart object @p obj.
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param func the callback function
 * @param data user data to be passed to the callback function
 *
 * Smart callbacks look very similar to Evas callbacks, but are
 * implemented as smart object's custom ones.
 *
 * This function adds a function callback to an smart object when the
 * event named @p event occurs in it. The function is @p func.
 *
 * In the event of a memory allocation error during addition of the
 * callback to the object, evas_alloc_error() should be used to
 * determine the nature of the error, if any, and the program should
 * sensibly try and recover.
 *
 * A smart callback function must have the ::Evas_Smart_Cb prototype
 * definition. The first parameter (@p data) in this definition will
 * have the same value passed to evas_object_smart_callback_add() as
 * the @p data parameter, at runtime. The second parameter @p obj is a
 * handle to the object on which the event occurred. The third
 * parameter, @p event_info, is a pointer to data which is totally
 * dependent on the smart object's implementation and semantic for the
 * given event.
 *
 * There is an infrastructure for introspection on smart objects'
 * events (see evas_smart_callbacks_descriptions_get()), but no
 * internal smart objects on Evas implement them yet.
 *
 * @see @ref Evas_Smart_Object_Group_Callbacks for more details.
 *
 * @see evas_object_smart_callback_del()
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Add (register) a callback function to the smart event specified by
 * @p event on the smart object @p obj. Except for the priority field,
 * it's exactly the same as @ref evas_object_smart_callback_add
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param priority The priority of the callback, lower values called first.
 * @param func the callback function
 * @param data user data to be passed to the callback function
 *
 * @see evas_object_smart_callback_add
 * @since 1.1
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callback_priority_add(Evas_Object *obj, const char *event, Evas_Callback_Priority priority, Evas_Smart_Cb func, const void *data);

/**
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object @p obj.
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param func the callback function
 * @return the data pointer
 *
 * This function removes <b>the first</b> added smart callback on the
 * object @p obj matching the event name @p event and the registered
 * function pointer @p func. If the removal is successful it will also
 * return the data pointer that was passed to
 * evas_object_smart_callback_add() (that will be the same as the
 * parameter) when the callback(s) was(were) added to the canvas. If
 * not successful @c NULL will be returned.
 *
 * @see evas_object_smart_callback_add() for more details.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void        *evas_object_smart_callback_del(Evas_Object *obj, const char *event, Evas_Smart_Cb func) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Delete (unregister) a callback function from the smart event
 * specified by @p event on the smart object @p obj.
 *
 * @param obj a smart object
 * @param event the event's name string
 * @param func the callback function
 * @param data the data pointer that was passed to the callback
 * @return the data pointer
 *
 * This function removes <b>the first</b> added smart callback on the
 * object @p obj matching the event name @p event, the registered
 * function pointer @p func and the callback data pointer @p data. If
 * the removal is successful it will also return the data pointer that
 * was passed to evas_object_smart_callback_add() (that will be the same
 * as the parameter) when the callback(s) was(were) added to the canvas.
 * If not successful @c NULL will be returned. A common use would be to
 * remove an exact match of a callback
 *
 * @see evas_object_smart_callback_add() for more details.
 * @since 1.2
 * @ingroup Evas_Smart_Object_Group
 *
 * @note To delete all smart event callbacks which match @p type and @p func,
 * use evas_object_smart_callback_del().
 */
EAPI void        *evas_object_smart_callback_del_full(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Call a given smart callback on the smart object @p obj.
 *
 * @param obj the smart object
 * @param event the event's name string
 * @param event_info pointer to an event specific struct or information to
 * pass to the callback functions registered on this smart event
 *
 * This should be called @b internally, from the smart object's own
 * code, when some specific event has occurred and the implementor
 * wants is to pertain to the object's events API (see @ref
 * Evas_Smart_Object_Group_Callbacks). The documentation for the smart
 * object should include a list of possible events and what type of @p
 * event_info to expect for each of them. Also, when defining an
 * #Evas_Smart_Class, smart object implementors are strongly
 * encouraged to properly set the Evas_Smart_Class::callbacks
 * callbacks description array, so that the users of the smart object
 * can have introspection on its events API <b>at run time</b>.
 *
 * @ingroup Evas_Smart_Object_Group
 */
EAPI void         evas_object_smart_callback_call(Evas_Object *obj, const char *event, void *event_info) EINA_ARG_NONNULL(1, 2);

/**
 * Retrieve an Evas smart object's interface, by name string pointer.
 *
 * @param obj An Evas smart object.
 * @param name Name string of the desired interface, which must be the
 *             same pointer used at the interface's declarion, when
 *             creating the smart object @a obj.
 *
 * @since 1.7
 *
 * @return The interface's handle pointer, if found, @c NULL
 * otherwise.
 */
EAPI const void       *evas_object_smart_interface_get(const Evas_Object *obj, const char *name);

/**
 * Retrieve an Evas smart object interface's <b>private data</b>.
 *
 * @param obj An Evas smart object.
 * @param iface The given object's interface handle.
 *
 * @since 1.7
 *
 * @return The object interface's private data blob pointer, if found,
 * @c NULL otherwise.
 */
EAPI void             *evas_object_smart_interface_data_get(const Evas_Object *obj, const Evas_Smart_Interface *iface);

/**
 * This gets the internal counter that counts the number of smart calculations
 *
 * @param e The canvas to get the calculate counter from
 *
 * Whenever evas performs smart object calculations on the whole canvas
 * it increments a counter by 1. This is the smart object calculate counter
 * that this function returns the value of. It starts at the value of 0 and
 * will increase (and eventually wrap around to negative values and so on) by
 * 1 every time objects are calculated. You can use this counter to ensure
 * you don't re-do calculations withint the same calculation generation/run
 * if the calculations maybe cause self-feeding effects.
 *
 * @ingroup Evas_Smart_Object_Group
 * @since 1.1
 */
EAPI int          evas_smart_objects_calculate_count_get(const Evas *e);

#include "canvas/evas_smart.eo.legacy.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Smart_Object_Clipped
 *
 * @{
 */
/**
 * Get the clipper object for the given clipped smart object.
 *
 * @param obj the clipped smart object to retrieve associated clipper
 * from.
 * @return the clipper object.
 *
 * Use this function if you want to change any of this clipper's
 * properties, like colors.
 *
 * @see evas_object_smart_clipped_smart_add()
 */
EAPI Evas_Object            *evas_object_smart_clipped_clipper_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

#include "canvas/evas_smart_clipped.eo.legacy.h"
/**
 * @}
 */

/**
 * @ingroup Evas_Object_Box
 *
 * @{
 */
/**
 * Set a new layouting function to a given box object
 *
 * @param o The box object to operate on.
 * @param cb The new layout function to set on @p o.
 * @param data Data pointer to be passed to @p cb.
 * @param free_data Function to free @p data, if need be.
 *
 * A box layout function affects how a box object displays child
 * elements within its area. The list of pre-defined box layouts
 * available in Evas is:
 * - evas_object_box_layout_horizontal()
 * - evas_object_box_layout_vertical()
 * - evas_object_box_layout_homogeneous_horizontal()
 * - evas_object_box_layout_homogeneous_vertical()
 * - evas_object_box_layout_homogeneous_max_size_horizontal()
 * - evas_object_box_layout_homogeneous_max_size_vertical()
 * - evas_object_box_layout_flow_horizontal()
 * - evas_object_box_layout_flow_vertical()
 * - evas_object_box_layout_stack()
 *
 * Refer to each of their documentation texts for details on them.
 *
 * @note A box layouting function will be triggered by the @c
 * 'calculate' smart callback of the box's smart class.
 */
EAPI void                       evas_object_box_layout_set(Evas_Object *o, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data)) EINA_ARG_NONNULL(1, 2);

/**
 * Add a new box object on the provided canvas.
 *
 * @param evas The canvas to create the box object on.
 * @return @c NULL on error, a pointer to a new box object on
 * success.
 *
 * After instantiation, if a box object hasn't its layout function
 * set, via evas_object_box_layout_set(), it will have it by default
 * set to evas_object_box_layout_horizontal(). The remaining
 * properties of the box must be set/retrieved via
 * <c>evas_object_box_{h,v}_{align,padding}_{get,set)()</c>.
 */
EAPI Evas_Object               *evas_object_box_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Add a new box as a @b child of a given smart object.
 *
 * @param parent The parent smart object to put the new box in.
 * @return @c NULL on error, a pointer to a new box object on
 * success.
 *
 * This is a helper function that has the same effect of putting a new
 * box object into @p parent by use of evas_object_smart_member_add().
 *
 * @see evas_object_box_add()
 */
EAPI Evas_Object               *evas_object_box_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Layout function which sets the box @a o to a (basic) horizontal box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In this layout, the box object's overall behavior is controlled by
 * its padding/alignment properties, which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions. The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c align_h controls the horizontal alignment of the child objects
 * relative to the containing box. When set to @c 0.0, children are
 * aligned to the left. A value of @c 1.0 makes them aligned to the
 * right border. Values in between align them proportionally. Note
 * that if the size required by the children, which is given by their
 * widths and the @c padding_h property of the box, is bigger than the
 * their container's width, the children will be displayed out of the
 * box's bounds. A negative value of @c align_h makes the box to
 * @b justify its children. The padding between them, in this case, is
 * corrected so that the leftmost one touches the left border and the
 * rightmost one touches the right border (even if they must
 * overlap). The @c align_v and @c padding_v properties of the box
 * @b don't contribute to its behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c align_x does @b not influence the box's behavior. @c padding_l
 * and @c padding_r sum up to the container's horizontal padding
 * between elements. The child's @c padding_t, @c padding_b and
 * @c align_y properties apply for padding/alignment relative to the
 * overall height of the box. Finally, there is the @c weight_x
 * property, which, if set to a non-zero value, tells the container
 * that the child width is @b not pre-defined. If the container can't
 * accommodate all its children, it sets the widths of the ones
 * <b>with weights</b> to sizes as small as they can all fit into
 * it. If the size required by the children is less than the
 * available, the box increases its childrens' (which have weights)
 * widths as to fit the remaining space. The @c weight_x property,
 * besides telling the element is resizable, gives a @b weight for the
 * resizing process.  The parent box will try to distribute (or take
 * off) widths accordingly to the @b normalized list of weights: most
 * weighted children remain/get larger in this process than the least
 * ones. @c weight_y does not influence the layout.
 *
 * If one desires that, besides having weights, child elements must be
 * resized bounded to a minimum or maximum size, those size hints must
 * be set, by the <c>evas_object_size_hint_{min,max}_set()</c>
 * functions.
 */
EAPI void                       evas_object_box_layout_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a (basic) vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_horizontal(). The description of its
 * behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b homogeneous
 * vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_horizontal().  The description
 * of its behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_homogeneous_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b homogeneous
 * horizontal box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a homogeneous horizontal box, its width is divided @b equally
 * between the contained objects. The box's overall behavior is
 * controlled by its padding/alignment properties, which are set by
 * the <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c align_h has no influence on the box for this layout.
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the (equal) child objects' cells. The @c align_v
 * and @c padding_v properties of the box don't contribute to its
 * behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to
 * @c align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the minimum and maximum size hints on
 * the child's width and accounting for its horizontal padding
 * hints). The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding/alignment relative to the overall
 * height of the box. A value of @c -1.0 to @c align_y makes the box
 * try to resize this child element to the exact height of its parent
 * (respecting the maximum size hint on the child's height).
 */
EAPI void                       evas_object_box_layout_homogeneous_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a <b>maximum size,
 * homogeneous</b> horizontal box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a maximum size, homogeneous horizontal box, besides having cells
 * of <b>equal size</b> reserved for the child objects, this size will
 * be defined by the size of the @b largest child in the box (in
 * width). The box's overall behavior is controlled by its properties,
 * which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects' cells. @c align_h controls the
 * horizontal alignment of the child objects, relative to the
 * containing box. When set to @c 0.0, children are aligned to the
 * left. A value of @c 1.0 lets them aligned to the right
 * border. Values in between align them proportionally. A negative
 * value of @c align_h makes the box to @b justify its children
 * cells. The padding between them, in this case, is corrected so that
 * the leftmost one touches the left border and the rightmost one
 * touches the right border (even if they must overlap). The
 * @c align_v and @c padding_v properties of the box don't contribute to
 * its behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to
 * @c align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the minimum and maximum size hints on
 * the child's width and accounting for its horizontal padding
 * hints). The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding/alignment relative to the overall
 * height of the box. A value of @c -1.0 to @c align_y makes the box
 * try to resize this child element to the exact height of its parent
 * (respecting the max hint on the child's height).
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a <b>maximum size,
 * homogeneous</b> vertical box
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_max_size_horizontal(). The
 * description of its behaviour can be derived from that function's
 * documentation.
 */
EAPI void                       evas_object_box_layout_homogeneous_max_size_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b flow horizontal
 * box.
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a flow horizontal box, the box's child elements are placed in
 * @b rows (think of text as an analogy). A row has as much elements as
 * can fit into the box's width. The box's overall behavior is
 * controlled by its properties, which are set by the
 * <c>evas_object_box_{h,v}_{align,padding}_set()</c> family of
 * functions.  The size hints of the elements in the box -- set by the
 * <c>evas_object_size_hint_{align,padding,weight}_set()</c> functions
 * -- also control the way this function works.
 *
 * \par Box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects' cells. @c align_h dictates the
 * horizontal alignment of the rows (@c 0.0 to left align them, @c 1.0
 * to right align). A value of @c -1.0 to @c align_h lets the rows
 * @b justified horizontally. @c align_v controls the vertical alignment
 * of the entire set of rows (@c 0.0 to top, @c 1.0 to bottom). A
 * value of @c -1.0 to @c align_v makes the box to @b justify the rows
 * vertically. The padding between them, in this case, is corrected so
 * that the first row touches the top border and the last one touches
 * the bottom border (even if they must overlap). @c padding_v has no
 * influence on the layout.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property has no influence on the
 * layout. The child's @c padding_t and @c padding_b sum up to the
 * required height of the child element and is the only means (besides
 * row justifying) of setting space between rows. Note, however, that
 * @c align_y dictates positioning relative to the <b>largest
 * height</b> required by a child object in the actual row.
 */
EAPI void                       evas_object_box_layout_flow_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b flow vertical box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_flow_horizontal(). The description of its
 * behaviour can be derived from that function's documentation.
 */
EAPI void                       evas_object_box_layout_flow_vertical(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Layout function which sets the box @a o to a @b stacking box
 *
 * @param o The box object in question
 * @param priv The smart data of the @p o
 * @param data The data pointer passed on
 * evas_object_box_layout_set(), if any
 *
 * In a stacking box, all children will be given the same size -- the
 * box's own size -- and they will be stacked one above the other, so
 * that the first object in @p o's internal list of child elements
 * will be the bottommost in the stack.
 *
 * \par Box's properties:
 * No box properties are used.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (@c 0.0 to
 * extreme left, @c 1.0 to extreme right). A value of @c -1.0 to @c
 * align_x makes the box try to resize this child element to the exact
 * width of its cell (respecting the min and max hints on the child's
 * width and accounting for its horizontal padding properties). The
 * same applies to the vertical axis.
 */
EAPI void                       evas_object_box_layout_stack(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Set the alignment of the whole bounding box of contents, for a
 * given box object.
 *
 * @param o The given box object to set alignment from
 * @param horizontal The horizontal alignment, in pixels
 * @param vertical the vertical alignment, in pixels
 *
 * This will influence how a box object is to align its bounding box
 * of contents within its own area. The values @b must be in the range
 * @c 0.0 - @c 1.0, or undefined behavior is expected. For horizontal
 * alignment, @c 0.0 means to the left, with @c 1.0 meaning to the
 * right. For vertical alignment, @c 0.0 means to the top, with @c 1.0
 * meaning to the bottom.
 *
 * @note The default values for both alignments is @c 0.5.
 *
 * @see evas_object_box_align_get()
 */
EAPI void                       evas_object_box_align_set(Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);

/**
 * Get the alignment of the whole bounding box of contents, for a
 * given box object.
 *
 * @param o The given box object to get alignment from
 * @param horizontal Pointer to a variable where to store the
 * horizontal alignment
 * @param vertical Pointer to a variable where to store the vertical
 * alignment
 *
 * @see evas_object_box_align_set() for more information
 */
EAPI void                       evas_object_box_align_get(const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

/**
 * Set the (space) padding between cells set for a given box object.
 *
 * @param o The given box object to set padding from
 * @param horizontal The horizontal padding, in pixels
 * @param vertical the vertical padding, in pixels
 *
 * @note The default values for both padding components is @c 0.
 *
 * @see evas_object_box_padding_get()
 */
EAPI void                       evas_object_box_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);

/**
 * Get the (space) padding between cells set for a given box object.
 *
 * @param o The given box object to get padding from
 * @param horizontal Pointer to a variable where to store the
 * horizontal padding
 * @param vertical Pointer to a variable where to store the vertical
 * padding
 *
 * @see evas_object_box_padding_set()
 */
EAPI void                       evas_object_box_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

/**
 * Append a new @a child object to the given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it. For example, on horizontal layouts
 * an item in the end of the box's list of children will appear on its
 * right.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api::append
 * smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_append(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Prepend a new @a child object to the given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it. For example, on horizontal layouts
 * an item in the beginning of the box's list of children will appear
 * on its left.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::prepend smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_prepend(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Insert a new @a child object <b>before another existing one</b>, in
 * a given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @param reference The child object to place this new one before
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note This function will fail if @p reference is not a member of @p
 * o.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::insert_before smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Insert a new @a child object <b>after another existing one</b>, in
 * a given box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @param reference The child object to place this new one after
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note This function will fail if @p reference is not a member of @p
 * o.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::insert_after smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_after(Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Insert a new @a child object <b>at a given position</b>, in a given
 * box object @a o.
 *
 * @param o The given box object
 * @param child A child Evas object to be made a member of @p o
 * @param pos The numeric position (starting from @c 0) to place the
 * new child object at
 * @return A box option bound to the recently added box item or @c
 * NULL, on errors
 *
 * On success, the @c "child,added" smart event will take place.
 *
 * @note This function will fail if the given position is invalid,
 * given @p o's internal list of elements.
 *
 * @note The actual placing of the item relative to @p o's area will
 * depend on the layout set to it.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::insert_at smart function.
 */
EAPI Evas_Object_Box_Option    *evas_object_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos) EINA_ARG_NONNULL(1, 2);

/**
 * Remove a given object from a box object, unparenting it again.
 *
 * @param o The box object to remove a child object from
 * @param child The handle to the child object to be removed
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * On removal, you'll get an unparented object again, just as it was
 * before you inserted it in the box. The
 * _Evas_Object_Box_Api::option_free box smart callback will be called
 * automatically for you and, also, the @c "child,removed" smart event
 * will take place.
 *
 * @note This call will trigger the box's _Evas_Object_Box_Api::remove
 * smart function.
 */
EAPI Eina_Bool                  evas_object_box_remove(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Remove an object, <b>bound to a given position</b> in a box object,
 * unparenting it again.
 *
 * @param o The box object to remove a child object from
 * @param pos The numeric position (starting from @c 0) of the child
 * object to be removed
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * On removal, you'll get an unparented object again, just as it was
 * before you inserted it in the box. The @c option_free() box smart
 * callback will be called automatically for you and, also, the
 * @c "child,removed" smart event will take place.
 *
 * @note This function will fail if the given position is invalid,
 * given @p o's internal list of elements.
 *
 * @note This call will trigger the box's
 * _Evas_Object_Box_Api::remove_at smart function.
 */
EAPI Eina_Bool                  evas_object_box_remove_at(Evas_Object *o, unsigned int pos) EINA_ARG_NONNULL(1);

/**
 * Remove @b all child objects from a box object, unparenting them
 * again.
 *
 * @param o The box object to remove a child object from
 * @param clear if true, it will delete just removed children.
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * This has the same effect of calling evas_object_box_remove() on
 * each of @p o's child objects, in sequence. If, and only if, all
 * those calls succeed, so does this one.
 */
EAPI Eina_Bool                  evas_object_box_remove_all(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * Get an iterator to walk the list of children of a given box object.
 *
 * @param o The box to retrieve an items iterator from
 * @return An iterator on @p o's child objects, on success, or @c NULL,
 * on errors
 *
 * @note Do @b not remove or delete objects while walking the list.
 */
EAPI Eina_Iterator             *evas_object_box_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor (a structure providing random items access) to the
 * list of children of a given box object.
 *
 * @param o The box to retrieve an items iterator from
 * @return An accessor on @p o's child objects, on success, or @c NULL,
 * on errors
 *
 * @note Do not remove or delete objects while walking the list.
 */
EAPI Eina_Accessor             *evas_object_box_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children objects in a given box object.
 *
 * @param o The box to retrieve an items list from
 * @return A list of @p o's child objects, on success, or @c NULL,
 * on errors (or if it has no child objects)
 *
 * The returned list should be freed with @c eina_list_free() when you
 * no longer need it.
 *
 * @note This is a duplicate of the list kept by the box internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the box when walking
 *       this list, but these removals won't be reflected on it.
 */
EAPI Eina_List                 *evas_object_box_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the name of the property of the child elements of the box @a o
 * which have @a id as identifier
 *
 * @param o The box to search child options from
 * @param property The numerical identifier of the option being searched,
 * for its name
 * @return The name of the given property or @c NULL, on errors.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, setting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and set it to be the
 * _Evas_Object_Box_Api::property_name_get smart class function of the
 * box, which is originally set to @c NULL.
 */
EAPI const char                *evas_object_box_option_property_name_get(const Evas_Object *o, int property) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Get the numerical identifier of the property of the child elements
 * of the box @a o which have @a name as name string
 *
 * @param o The box to search child options from
 * @param name The name string of the option being searched, for
 * its ID
 * @return The numerical ID of the given property or @c -1, on
 * errors.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, setting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and set it to be the
 * _Evas_Object_Box_Api::property_id_get smart class function of the
 * box, which is originally set to @c NULL.
 */
EAPI int                        evas_object_box_option_property_id_get(const Evas_Object *o, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * Set a property value (by its given numerical identifier), on a
 * given box child element
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to set a property on
 * @param property The numerical ID of the given property
 * @param ... (List of) actual value(s) to be set for this
 * property. It (they) @b must be of the same type the user has
 * defined for it (them).
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, setting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and set it to be the
 * _Evas_Object_Box_Api::property_set smart class function of the box,
 * which is originally set to @c NULL.
 *
 * @note This function will internally create a variable argument
 * list, with the values passed after @p property, and call
 * evas_object_box_option_property_vset() with this list and the same
 * previous arguments.
 */
EAPI Eina_Bool                  evas_object_box_option_property_set(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * Set a property value (by its given numerical identifier), on a
 * given box child element -- by a variable argument list
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to set a property on
 * @param property The numerical ID of the given property
 * @param args The variable argument list implementing the value to
 * be set for this property. It @b must be of the same type the user has
 * defined for it.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * This is a variable argument list variant of the
 * evas_object_box_option_property_set(). See its documentation for
 * more details.
 */
EAPI Eina_Bool                  evas_object_box_option_property_vset(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * Get a property's value (by its given numerical identifier), on a
 * given box child element
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to get a property from
 * @param property The numerical ID of the given property
 * @param ... (List of) pointer(s) where to store the value(s) set for
 * this property. It (they) @b must point to variable(s) of the same
 * type the user has defined for it (them).
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * @note This call won't do anything for a canonical Evas box. Only
 * users which have @b subclassed it, getting custom box items options
 * (see #Evas_Object_Box_Option) on it, would benefit from this
 * function. They'd have to implement it and get it to be the
 * _Evas_Object_Box_Api::property_get smart class function of the
 * box, which is originally get to @c NULL.
 *
 * @note This function will internally create a variable argument
 * list, with the values passed after @p property, and call
 * evas_object_box_option_property_vget() with this list and the same
 * previous arguments.
 */
EAPI Eina_Bool                  evas_object_box_option_property_get(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);

/**
 * Get a property's value (by its given numerical identifier), on a
 * given box child element -- by a variable argument list
 *
 * @param o The box parenting the child element
 * @param opt The box option structure bound to the child box element
 * to get a property from
 * @param property The numerical ID of the given property
 * @param args The variable argument list with pointers to where to
 * store the values of this property. They @b must point to variables
 * of the same type the user has defined for them.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * This is a variable argument list variant of the
 * evas_object_box_option_property_get(). See its documentation for
 * more details.
 */
EAPI Eina_Bool                  evas_object_box_option_property_vget(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Table
 *
 * @{
 */
/**
 * @brief Create a new table.
 *
 * @param evas Canvas in which table will be added.
 */
EAPI Evas_Object                       *evas_object_table_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the child of the table at the given coordinates
 *
 * @note This does not take into account col/row spanning
 */
EAPI Evas_Object                       *evas_object_table_child_get(const Evas_Object *o, unsigned short col, unsigned short row) EINA_ARG_NONNULL(1);

#include "canvas/evas_table.eo.legacy.h"

/**
 * @}
 */

/**
 * @ingroup Evas_Object_Grid
 *
 * @{
 */
/**
 * Create a new grid.
 *
 * It's set to a virtual size of 1x1 by default and add children with
 * evas_object_grid_pack().
 * @since 1.1
 */
EAPI Evas_Object   *evas_object_grid_add(Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Create a grid that is child of a given element @a parent.
 *
 * @see evas_object_grid_add()
 * @since 1.1
 */
EAPI Evas_Object   *evas_object_grid_add_to(Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Set the virtual resolution for the grid
 *
 * @param o The grid object to modify
 * @param w The virtual horizontal size (resolution) in integer units
 * @param h The virtual vertical size (resolution) in integer units
 * @since 1.1
 */
EAPI void           evas_object_grid_size_set(Evas_Object *o, int w, int h) EINA_ARG_NONNULL(1);

/**
 * Get the current virtual resolution
 *
 * @param o The grid object to query
 * @param w A pointer to an integer to store the virtual width
 * @param h A pointer to an integer to store the virtual height
 * @see evas_object_grid_size_set()
 * @since 1.1
 */
EAPI void           evas_object_grid_size_get(const Evas_Object *o, int *w, int *h) EINA_ARG_NONNULL(1);

/**
 * Sets the mirrored mode of the grid. In mirrored mode the grid items go
 * from right to left instead of left to right. That is, 0,0 is top right, not
 * to left.
 *
 * @param o The grid object.
 * @param mirrored the mirrored mode to set
 * @since 1.1
 */
EAPI void           evas_object_grid_mirrored_set(Evas_Object *o, Eina_Bool mirrored) EINA_ARG_NONNULL(1);

/**
 * Gets the mirrored mode of the grid.
 *
 * @param o The grid object.
 * @return @c EINA_TRUE if it's a mirrored grid, @c EINA_FALSE otherwise.
 * @see evas_object_grid_mirrored_set()
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_mirrored_get(const Evas_Object *o) EINA_ARG_NONNULL(1);

/**
 * Add a new child to a grid object.
 *
 * @param o The given grid object.
 * @param child The child object to add.
 * @param x The virtual x coordinate of the child
 * @param y The virtual y coordinate of the child
 * @param w The virtual width of the child
 * @param h The virtual height of the child
 * @return 1 on success, 0 on failure.
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_pack(Evas_Object *o, Evas_Object *child, int x, int y, int w, int h) EINA_ARG_NONNULL(1, 2);

/**
 * Remove child from grid.
 *
 * @note removing a child will immediately call a walk over children in order
 *       to recalculate numbers of columns and rows. If you plan to remove
 *       all children, use evas_object_grid_clear() instead.
 *
 * @return 1 on success, 0 on failure.
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_unpack(Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);

/**
 * Faster way to remove all child objects from a grid object.
 *
 * @param o The given grid object.
 * @param clear if true, it will delete just removed children.
 * @since 1.1
 */
EAPI void           evas_object_grid_clear(Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

/**
 * Get the pack options for a grid child
 *
 * Get the pack x, y, width and height in virtual coordinates set by
 * evas_object_grid_pack()
 * @param o The grid object
 * @param child The grid child to query for coordinates
 * @param x The pointer to where the x coordinate will be returned
 * @param y The pointer to where the y coordinate will be returned
 * @param w The pointer to where the width will be returned
 * @param h The pointer to where the height will be returned
 * @return 1 on success, 0 on failure.
 * @since 1.1
 */
EAPI Eina_Bool      evas_object_grid_pack_get(const Evas_Object *o, Evas_Object *child, int *x, int *y, int *w, int *h);

/**
 * Get an iterator to walk the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 * @since 1.1
 */
EAPI Eina_Iterator *evas_object_grid_iterator_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get an accessor to get random access to the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 * @since 1.1
 */
EAPI Eina_Accessor *evas_object_grid_accessor_new(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Get the list of children for the grid.
 *
 * @note This is a duplicate of the list kept by the grid internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the grid when walking this
 *       list, but these removals won't be reflected on it.
 * @since 1.1
 */
EAPI Eina_List     *evas_object_grid_children_get(const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @}
 */

/**
 * @ingroup Evas_Keys
 *
 * @{
 */
/**
 * Returns a handle to the list of modifier keys registered in the
 * canvas @p e. This is required to check for which modifiers are set
 * at a given time with the evas_key_modifier_is_set() function.
 *
 * @param e The pointer to the Evas canvas
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 *
 * @return An ::Evas_Modifier handle to query Evas' keys subsystem
 *	with evas_key_modifier_is_set(), or @c NULL on error.
 */
EAPI const Evas_Modifier *evas_key_modifier_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Returns a handle to the list of lock keys registered in the canvas
 * @p e. This is required to check for which locks are set at a given
 * time with the evas_key_lock_is_set() function.
 *
 * @param e The pointer to the Evas canvas
 *
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 *
 * @return An ::Evas_Lock handle to query Evas' keys subsystem with
 *	evas_key_lock_is_set(), or @c NULL on error.
 */
EAPI const Evas_Lock     *evas_key_lock_get(const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Adds the @p keyname key to the current list of modifier keys.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the modifier key to add to the list of
 *        Evas modifiers.
 *
 * Modifiers are keys like shift, alt and ctrl, i.e., keys which are
 * meant to be pressed together with others, altering the behavior of
 * the secondly pressed keys somehow. Evas is so that these keys can
 * be user defined.
 *
 * This call allows custom modifiers to be added to the Evas system at
 * run time. It is then possible to set and unset modifier keys
 * programmatically for other parts of the program to check and act
 * on. Programmers using Evas would check for modifier keys on key
 * event callbacks using evas_key_modifier_is_set().
 *
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 *
 * @note If the programmer instantiates the canvas by means of the
 *       ecore_evas_new() family of helper functions, Ecore will take
 *       care of registering on it all standard modifiers: "Shift",
 *       "Control", "Alt", "Meta", "Hyper", "Super".
 */
EAPI void                 evas_key_modifier_add(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Removes the @p keyname key from the current list of modifier keys
 * on canvas @p e.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to remove from the modifiers list.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_del(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Adds the @p keyname key to the current list of lock keys.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to add to the locks list.
 *
 * Locks are keys like caps lock, num lock or scroll lock, i.e., keys
 * which are meant to be pressed once -- toggling a binary state which
 * is bound to it -- and thus altering the behavior of all
 * subsequently pressed keys somehow, depending on its state. Evas is
 * so that these keys can be defined by the user.
 *
 * This allows custom locks to be added to the evas system at run
 * time. It is then possible to set and unset lock keys
 * programmatically for other parts of the program to check and act
 * on. Programmers using Evas would check for lock keys on key event
 * callbacks using evas_key_lock_is_set().
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * @see evas_key_lock_is_set
 *
 * @note If the programmer instantiates the canvas by means of the
 *       ecore_evas_new() family of helper functions, Ecore will take
 *       care of registering on it all standard lock keys: "Caps_Lock",
 *       "Num_Lock", "Scroll_Lock".
 */
EAPI void                 evas_key_lock_add(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Removes the @p keyname key from the current list of lock keys on
 * canvas @p e.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the key to remove from the locks list.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_del(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Enables or turns on programmatically the modifier key with name @p
 * keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the modifier to enable.
 *
 * The effect will be as if the key was pressed for the whole time
 * between this call and a matching evas_key_modifier_off().
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_on(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Disables or turns off programmatically the modifier key with name
 * @p keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the modifier to disable.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_is_set
 */
EAPI void                 evas_key_modifier_off(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Enables or turns on programmatically the lock key with name @p
 * keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the lock to enable.
 *
 * The effect will be as if the key was put on its active state after
 * this call.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_off
 */
EAPI void                 evas_key_lock_on(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Disables or turns off programmatically the lock key with name @p
 * keyname.
 *
 * @param e The pointer to the Evas canvas
 * @param keyname The name of the lock to disable.
 *
 * The effect will be as if the key was put on its inactive state
 * after this call.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 */
EAPI void                 evas_key_lock_off(Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);

/**
 * Creates a bit mask from the @p keyname @b modifier key. Values
 * returned from different calls to it may be ORed together,
 * naturally.
 *
 * @param e The canvas whom to query the bit mask from.
 * @param keyname The name of the modifier key to create the mask for.
 *
 * @returns the bit mask or 0 if the @p keyname key wasn't registered as a
 *          modifier for canvas @p e.
 *
 * This function is meant to be using in conjunction with
 * evas_object_key_grab()/evas_object_key_ungrab(). Go check their
 * documentation for more information.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI Evas_Modifier_Mask   evas_key_modifier_mask_get(const Evas *e, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @ingroup Evas_Touch_Point_List
 *
 * @{
 */
/**
 * Get the number of touched point in the evas.
 *
 * @param e The pointer to the Evas canvas.
 * @return The number of touched point on the evas.
 *
 * New touched point is added to the list whenever touching the evas
 * and point is removed whenever removing touched point from the evas.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int count;
 *
 * count = evas_touch_point_list_count(evas);
 * printf("The count of touch points: %i\n", count);
 * @endcode
 *
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_id_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI unsigned int           evas_touch_point_list_count(Evas *e) EINA_ARG_NONNULL(1);

/**
 * This function returns the nth touch point's co-ordinates.
 *
 * @param e The pointer to the Evas canvas.
 * @param n The number of the touched point (0 being the first).
 * @param x The pointer to a Evas_Coord to be filled in.
 * @param y The pointer to a Evas_Coord to be filled in.
 *
 * Touch point's co-ordinates is updated whenever moving that point
 * on the canvas.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord x, y;
 *
 * if (evas_touch_point_list_count(evas))
 *   {
 *      evas_touch_point_nth_xy_get(evas, 0, &x, &y);
 *      printf("The first touch point's co-ordinate: (%i, %i)\n", x, y);
 *   }
 * @endcode
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_id_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI void                   evas_touch_point_list_nth_xy_get(Evas *e, unsigned int n, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);

/**
 * This function returns the @p id of nth touch point.
 *
 * @param e The pointer to the Evas canvas.
 * @param n The number of the touched point (0 being the first).
 * @return id of nth touch point, if the call succeeded, -1 otherwise.
 *
 * The point which comes from Mouse Event has @p id 0 and The point
 * which comes from Multi Event has @p id that is same as Multi
 * Event's device id.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int id;
 *
 * if (evas_touch_point_list_count(evas))
 *   {
 *      id = evas_touch_point_nth_id_get(evas, 0);
 *      printf("The first touch point's id: %i\n", id);
 *   }
 * @endcode
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_state_get()
 */
EAPI int                    evas_touch_point_list_nth_id_get(Evas *e, unsigned int n) EINA_ARG_NONNULL(1);

/**
 * This function returns the @p state of nth touch point.
 *
 * @param e The pointer to the Evas canvas.
 * @param n The number of the touched point (0 being the first).
 * @return @p state of nth touch point, if the call succeeded,
 *         EVAS_TOUCH_POINT_CANCEL otherwise.
 *
 * The point's @p state is EVAS_TOUCH_POINT_DOWN when pressed,
 * EVAS_TOUCH_POINT_STILL when the point is not moved after pressed,
 * EVAS_TOUCH_POINT_MOVE when moved at least once after pressed and
 * EVAS_TOUCH_POINT_UP when released.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Touch_Point_State state;
 *
 * if (evas_touch_point_list_count(evas))
 *   {
 *      state = evas_touch_point_nth_state_get(evas, 0);
 *      printf("The first touch point's state: %i\n", state);
 *   }
 * @endcode
 *
 * @see evas_touch_point_list_count()
 * @see evas_touch_point_list_nth_xy_get()
 * @see evas_touch_point_list_nth_id_get()
 */
EAPI Evas_Touch_Point_State evas_touch_point_list_nth_state_get(Evas *e, unsigned int n) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @since 1.8
 *
 * Adds an output to the canvas
 * 
 * @parem e The canvas to add the output to
 * @return The output
 *
 * @see evas_out_engine_info_set
 * @see evas_output_viewport_set
 * @see evas_output_size_set
 */
EAPI Evas_Out *evas_out_add(Evas *e);

/**
 * @since 1.8
 *
 * Deletes an output
 * 
 * @parem evo The output object
 *
 * @see evas_out_add
 */
EAPI void evas_output_del(Evas_Out *evo);

/**
 * @since 1.8
 *
 * Sets the viewport region of the canvas that the output displays
 * 
 * This sets both the viewport region in the canvas that displays on the
 * given output, but also the viewport size will match the output
 * resolution 1:1.
 *
 * @parem evo The output object
 * @param x The X coordinate of the viewport
 * @param y The Y coordinate of the viewport
 * @param w The Width of the viewport
 * @param h The Height of the viewport
 *
 * @see evas_out_engine_info_set
 * @see evas_output_viewport_set
 * @see evas_output_size_set
 */
EAPI void evas_output_view_set(Evas_Out *evo, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @since 1.8
 *
 * Gets the viewport region of the canvas that the output displays
 *
 * @parem evo The output object
 * @param x Pointer to X return value
 * @param y Pointer to Y return value
 * @param w Pointer to Width return value
 * @param h Pointer to Height return value
 *
 * @see evas_out_engine_info_get
 * @see evas_out_view_set
 * @see evas_output_viewport_get
 * @see evas_output_size_get
 */
EAPI void evas_output_view_get(const Evas_Out *evo, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @since 1.8
 *
 * Sets the engine specific output parameters for a given output.
 *
 * @parem evo The output object
 * @param info The engine parameters
 * @return EINA_TRUE for success.
 *
 * @see evas_out_engine_info_get
 * @see evas_output_viewport_set
 * @see evas_output_size_set
 */
EAPI Eina_Bool evas_output_engine_info_set(Evas_Out *evo, Evas_Engine_Info *info);

/**
 * @since 1.8
 *
 * Gets the engine specific output parameters for a given output.
 *
 * @param evo The output object
 * @return info The engine parameters return (NULL on failure)
 *
 * @see evas_out_engine_info_set
 * @see evas_output_viewport_get
 * @see evas_output_size_get
 */
EAPI Evas_Engine_Info *evas_output_engine_info_get(const Evas_Out *evo);

/*
 * XXX: need output add/del events on evas
 * XXX: need output name/id, label, data
 */

