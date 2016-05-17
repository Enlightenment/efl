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

#include "canvas/evas_canvas.eo.legacy.h"

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
 * unwound from most recently to least recently pushed item and removed from the
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
 * @ingroup Evas_Font_Group
 *
 * @{
 */

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

/**
 *
 * Sets the general/main color of the given Evas object to the given
 * one.
 *
 * @see evas_object_color_get() (for an example)
 * @note These color values are expected to be premultiplied by @p a.
 *
 * @ingroup Evas_Object_Group_Basic
 *
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 */
EAPI void evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a);

/**
 *
 * Retrieves the general/main color of the given Evas object.
 *
 * Retrieves the “main” color's RGB component (and alpha channel)
 * values, <b>which range from 0 to 255</b>. For the alpha channel,
 * which defines the object's transparency level, 0 means totally
 * transparent, while 255 means opaque. These color values are
 * premultiplied by the alpha value.
 *
 * Usually you’ll use this attribute for text and rectangle objects,
 * where the “main” color is their unique one. If set for objects
 * which themselves have colors, like the images one, those colors get
 * modulated by this one.
 *
 * @note All newly created Evas rectangles get the default color
 * values of <code>255 255 255 255</code> (opaque white).
 *
 * @note Use @c NULL pointers on the components you're not interested
 * in: they'll be ignored by the function.
 *
 * Example:
 * @dontinclude evas-object-manipulation.c
 * @skip int alpha, r, g, b;
 * @until return
 *
 * See the full @ref Example_Evas_Object_Manipulation "example".
 *
 * @ingroup Evas_Object_Group_Basic
 *
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 */
EAPI void evas_object_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a);

/**
 *
 * Move the given Evas object to the given location inside its canvas' viewport.
 *
 * @param[in] x in
 * @param[in] y in
 */
EAPI void evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/**
 *
 * Changes the size of the given Evas object.
 *
 * @param[in] w in
 * @param[in] h in
 */
EAPI void evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 *
 * Retrieves whether or not the given Evas object is visible.
 *
 */
EAPI Eina_Bool evas_object_visible_get(const Evas_Object *obj);

/**
 *
 * Sets the layer of its canvas that the given object will be part of.
 *
 * If you don't use this function, you'll be dealing with an @b unique
 * layer of objects, the default one. Additional layers are handy when
 * you don't want a set of objects to interfere with another set with
 * regard to @b stacking. Two layers are completely disjoint in that
 * matter.
 *
 * This is a low-level function, which you'd be using when something
 * should be always on top, for example.
 *
 * @warning Be careful, it doesn't make sense to change the layer of
 * smart objects' children. Smart objects have a layer of their own,
 * which should contain all their children objects.
 *
 * @see evas_object_layer_get()
 *
 * @param[in] l The number of the layer to place the object on.
Must be between #EVAS_LAYER_MIN and #EVAS_LAYER_MAX.
 */
EAPI void evas_object_layer_set(Evas_Object *obj, short l);

/**
 *
 * Retrieves the layer of its canvas that the given object is part of.
 *
 * @return  Number of its layer
 *
 * @see evas_object_layer_set()
 *
 */
EAPI short evas_object_layer_get(const Evas_Object *obj);

/**
 *
 * Get the Evas object stacked right below @p obj
 *
 * @return the #Evas_Object directly below @p obj, if any, or @c NULL,
 * if none
 *
 * This function will traverse layers in its search, if there are
 * objects on layers below the one @p obj is placed at.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 *
 */
EAPI Evas_Object *evas_object_below_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 *
 * Get the Evas object stacked right above @p obj
 *
 * @return the #Evas_Object directly above @p obj, if any, or @c NULL,
 * if none
 *
 * This function will traverse layers in its search, if there are
 * objects on layers above the one @p obj is placed at.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_below_get()
 *
 */
EAPI Evas_Object *evas_object_above_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 *
 * Stack @p obj immediately below @p below
 *
 * Objects, in a given canvas, are stacked in the order they get added
 * to it.  This means that, if they overlap, the highest ones will
 * cover the lowest ones, in that order. This function is a way to
 * change the stacking order for the objects.
 *
 * This function is intended to be used with <b>objects belonging to
 * the same layer</b> in a given canvas, otherwise it will fail (and
 * accomplish nothing).
 *
 * If you have smart objects on your canvas and @p obj is a member of
 * one of them, then @p below must also be a member of the same
 * smart object.
 *
 * Similarly, if @p obj is not a member of a smart object, @p below
 * must not be either.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 *
 *
 * @param[in] below the object below which to stack
 */
EAPI void evas_object_stack_below(Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(2);

/**
 *
 * Raise @p obj to the top of its layer.
 *
 * @p obj will, then, be the highest one in the layer it belongs
 * to. Object on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_lower()
 *
 *
 */
EAPI void evas_object_raise(Evas_Object *obj);

/**
 *
 * Stack @p obj immediately above @p above
 *
 * Objects, in a given canvas, are stacked in the order they get added
 * to it.  This means that, if they overlap, the highest ones will
 * cover the lowest ones, in that order. This function is a way to
 * change the stacking order for the objects.
 *
 * This function is intended to be used with <b>objects belonging to
 * the same layer</b> in a given canvas, otherwise it will fail (and
 * accomplish nothing).
 *
 * If you have smart objects on your canvas and @p obj is a member of
 * one of them, then @p above must also be a member of the same
 * smart object.
 *
 * Similarly, if @p obj is not a member of a smart object, @p above
 * must not be either.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 *
 *
 * @param[in] above the object above which to stack
 */
EAPI void evas_object_stack_above(Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(2);

/**
 *
 * Lower @p obj to the bottom of its layer.
 *
 * @p obj will, then, be the lowest one in the layer it belongs
 * to. Objects on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_raise()
 *
 *
 */
EAPI void evas_object_lower(Evas_Object *obj);

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
 * #EVAS_CALLBACK_AXIS_UPDATE,
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
 * - #EVAS_CALLBACK_AXIS_UPDATE: @p event_info is a pointer to an
 *   #Evas_Event_Axis_Update struct
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
 * just type and function pointer, use
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
 * @}
 */

/**
 * @ingroup Evas_Object_Group_Interceptors
 *
 * @{
 */

/**
 * Set the callback function that intercepts a show event of an object.
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
 * Unset the callback function that intercepts a show event of a object.
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
 * Unset the callback function that intercepts a hide event of a object.
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
 * Set the callback function that intercepts a move event of an object.
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
 * Unset the callback function that intercepts a move event of an object.
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

/**
 * Set the callback function that intercepts a resize event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a resize event
 * of a canvas object.
 *
 * @see evas_object_intercept_resize_callback_del().
 *
 */
EAPI void  evas_object_intercept_resize_callback_add(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a resize event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a resize event
 * of a canvas object.
 *
 * @see evas_object_intercept_resize_callback_add().
 *
 */
EAPI void *evas_object_intercept_resize_callback_del(Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a raise event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a raise event
 * of a canvas object.
 *
 * @see evas_object_intercept_raise_callback_del().
 *
 */
EAPI void  evas_object_intercept_raise_callback_add(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a raise event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a raise event
 * of a canvas object.
 *
 * @see evas_object_intercept_raise_callback_add().
 *
 */
EAPI void *evas_object_intercept_raise_callback_del(Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a lower event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a lower event
 * of a canvas object.
 *
 * @see evas_object_intercept_lower_callback_del().
 *
 */
EAPI void  evas_object_intercept_lower_callback_add(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a lower event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a lower event
 * of a canvas object.
 *
 * @see evas_object_intercept_lower_callback_add().
 *
 */
EAPI void *evas_object_intercept_lower_callback_del(Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a stack above event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a stack above event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_above_callback_del().
 *
 */
EAPI void  evas_object_intercept_stack_above_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a stack above event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a stack above event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_above_callback_add().
 *
 */
EAPI void *evas_object_intercept_stack_above_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a stack below event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a stack below event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_below_callback_del().
 *
 */
EAPI void  evas_object_intercept_stack_below_callback_add(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a stack below event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a stack below event
 * of a canvas object.
 *
 * @see evas_object_intercept_stack_below_callback_add().
 *
 */
EAPI void *evas_object_intercept_stack_below_callback_del(Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a layer set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a layer set event
 * of a canvas object.
 *
 * @see evas_object_intercept_layer_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_layer_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a layer set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a layer set event
 * of a canvas object.
 *
 * @see evas_object_intercept_layer_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_layer_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a color set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a color set event
 * of a canvas object.
 *
 * @see evas_object_intercept_color_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_color_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a color set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a color set event
 * of a canvas object.
 *
 * @see evas_object_intercept_color_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_color_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a clip set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a clip set event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_clip_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a clip set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a clip set event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_set_callback_add().
 *
 */
EAPI void *evas_object_intercept_clip_set_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a clip unset event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a clip unset event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_unset_callback_del().
 *
 */
EAPI void  evas_object_intercept_clip_unset_callback_add(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a clip unset event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a clip unset event
 * of a canvas object.
 *
 * @see evas_object_intercept_clip_unset_callback_add().
 *
 */
EAPI void *evas_object_intercept_clip_unset_callback_del(Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func) EINA_ARG_NONNULL(1, 2);

/**
 * Set the callback function that intercepts a focus set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given function to be the callback function.
 * @param data The data passed to the callback function.
 *
 * This function sets a callback function to intercepts a focus set event
 * of a canvas object.
 *
 * @see evas_object_intercept_focus_set_callback_del().
 *
 */
EAPI void  evas_object_intercept_focus_set_callback_add(Evas_Object *obj, Evas_Object_Intercept_Focus_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Unset the callback function that intercepts a focus set event of an object.
 *
 * @param obj The given canvas object pointer.
 * @param func The given callback function.
 *
 * This function sets a callback function to intercepts a focus set event
 * of a canvas object.
 *
 * @see evas_object_intercept_focus_set_callback_add().
 *
 */
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
 * @defgroup Evas_Object_Vg
 * @ingroup Evas
 *
 * Evas_Object_Vg is the scene graph for managing vector graphics  objects.
 * User can create shape objects as well as fill objects and give it to the
 * Evas_Object_Vg for drawing on the screen as well as managing the lifecycle
 * of the objects. enabling reuse of shape objects.
 *
 * As Evas_Object_Vg is a Evas_Object all the operation that applicable to
 * a Evas_Object can be performed on it(clipping , map, etc).
 *
 * To create any complex vector graphics you can create a hirarchy of shape
 * and fill objects and give the hirarchy to Evas_Object which  will be
 * responsible for drawing and showing on the screen.
 *
 * As the shape object and fill object (linear and radial gradient) have
 * retain mode API, you only have to create it once and set the properties
 * and give it to evas_object_vg.
 *
 * Any change in the property of shape/fill object will automaticaly notified
 * to the evas_object_vg which will trigger a redrawing to reflect the change.
 *
 * To create a vector path, you can give list of path commands to the shape
 * object using efl_gfx_shape_path_set() API.
 *
 * Enabling graphical shapes to be constructed and reused.
 *
 * Below are the list of feature currently supported by Vector object.
 *
 * @li Drawing SVG Path.
 *     You can construct a path by using api in efl_gfx_utils.h
 *
 * @li Gradient filling and stroking.
 *     You can fill or stroke the path using linear or radial gradient.
 *     @see Evas_Vg_Gradient_Linear and Evas_Vg_Gradient_Radial
 *
 * @li Transformation support for path and gradient fill. You can apply
       affin transformation on path object.
 *     @see Eina_Matrix.
 *
 * @note Below are the list of interface, classes can be used to draw vector
 *       graphics using vector object.
 *
 * @li Efl.Gfx.Shape
 * @li Evas.VG_Shape
 * @li Evas.VG_Node
 * @li Efl.Gfx.Gradient
 * @li Efl.Gfx.Gradient_Radial
 * @li Efl.Gfx.Gradient_Linear
 *
 * Example:
 * @code
 * vector = evas_object_vg_add(canvas);
 * root = evas_obj_vg_root_node_get(vector);
 * shape = eo_add(EVAS_VG_SHAPE_CLASS, root);
 * Efl_Gfx_Path_Command *path_cmd = NULL;
 * double *points = NULL;
 * efl_gfx_path_append_circle(&path_cmd, &points);
 * *       evas_vg_node_origin_set(shape, 10, 10);
 * *       efl_gfx_shape_stroke_width_set(shape, 1.0);
 * *       evas_vg_node_color_set(shape, 128, 128, 128, 80);
 * *       efl_gfx_shape_path_set(shape, path_cmd, points); 
 * @endcode
 *
 * @since 1.14
 */

#ifdef EFL_BETA_API_SUPPORT

/**
 * Creates a new vector object on the given Evas @p e canvas.
 *
 * @param e The given canvas.
 * @return The created vector object handle.
 *
 * The shape object hirarchy can be added to the evas_object_vg by accessing
 * the rootnode of the vg canvas and adding the hirarchy as child to the root
 * node.
 *
 * @see evas_obj_vg_root_node_get()
 * @since 1.14
 */
EAPI Evas_Object *evas_object_vg_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "canvas/evas_vg.eo.legacy.h"

/**
 * Creates a new vector shape object \.
 *
 * @param parent The given vector container object.
 * @return The created vector shape object handle.
 *
 * @since 1.14
 */
EAPI Efl_VG* evas_vg_shape_add(Efl_VG *parent);

/**
 * Creates a new vector container object \.
 *
 * @param parent The given vector container object.
 * @return The created vector container object handle.
 *
 * @since 1.14
 */

EAPI Efl_VG* evas_vg_container_add(Efl_VG *parent);

/**
 *
 * Retrieves whether or not the given Efl_Vg object is visible.
 *
 *
 */
EAPI Eina_Bool evas_vg_node_visible_get(Eo *obj);

/**
 *
 * Makes the given Efl_Vg object visible or invisible.
 *
 * @param[in] v @c EINA_TRUE if to make the object visible, @c EINA_FALSE otherwise
 *
 */
EAPI void evas_vg_node_visible_set(Eo *obj, Eina_Bool v);

/**
 *
 * Retrieves the general/main color of the given Efl_Vg object.
 *
 * Retrieves the “main” color's RGB component (and alpha channel)
 * values, <b>which range from 0 to 255</b>. For the alpha channel,
 * which defines the object's transparency level, 0 means totally
 * transparent, while 255 means opaque. These color values are
 * premultiplied by the alpha value.
 *
 *
 * @note Use @c NULL pointers on the components you're not interested
 * in: they'll be ignored by the function.
 *
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_node_color_get(Eo *obj, int *r, int *g, int *b, int *a);

/**
 *
 * Sets the general/main color of the given Efl_Vg object to the given
 * one.
 *
 * @see evas_vg_node_color_get() (for an example)
 * @note These color values are expected to be premultiplied by @p a.
 *
 * @ingroup Evas_Object_Group_Basic
 *
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_node_color_set(Eo *obj, int r, int g, int b, int a);

/**
 *
 * Retrieves the geometry of the given Efl_Vg object.
 *
 * @param[out] x in
 * @param[out] y in
 * @param[out] w in
 * @param[out] h in
 *
 */
EAPI void evas_vg_node_geometry_get(Eo *obj, int *x, int *y, int *w, int *h);

/**
 *
 * Changes the geometry of the given Efl_Vg object.
 *
 * @param[in] x in
 * @param[in] y in
 * @param[in] w in
 * @param[in] h in
 *
 */
EAPI void evas_vg_node_geometry_set(Eo *obj, int x, int y, int w, int h);

/**
 *
 * Stack @p obj immediately below @p below
 *
 * Objects, in a given canvas, are stacked in the order they get added
 * to it.  This means that, if they overlap, the highest ones will
 * cover the lowest ones, in that order. This function is a way to
 * change the stacking order for the objects.
 *
 * This function is intended to be used with <b>objects belonging to
 * the same layer</b> in a given canvas, otherwise it will fail (and
 * accomplish nothing).
 *
 * If you have smart objects on your canvas and @p obj is a member of
 * one of them, then @p below must also be a member of the same
 * smart object.
 *
 * Similarly, if @p obj is not a member of a smart object, @p below
 * must not be either.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 * 
 *
 * @param[in] below the object below which to stack
 *
 */
EAPI void evas_vg_node_stack_below(Eo *obj, Eo *below);

/**
 *
 * Stack @p obj immediately above @p above
 *
 * Objects, in a given canvas, are stacked in the order they get added
 * to it.  This means that, if they overlap, the highest ones will
 * cover the lowest ones, in that order. This function is a way to
 * change the stacking order for the objects.
 *
 * This function is intended to be used with <b>objects belonging to
 * the same layer</b> in a given canvas, otherwise it will fail (and
 * accomplish nothing).
 *
 * If you have smart objects on your canvas and @p obj is a member of
 * one of them, then @p above must also be a member of the same
 * smart object.
 *
 * Similarly, if @p obj is not a member of a smart object, @p above
 * must not be either.
 *
 * @see evas_object_layer_get()
 * @see evas_object_layer_set()
 * @see evas_object_stack_below()
 * 
 *
 * @param[in] above the object above which to stack
 *
 */
EAPI void evas_vg_node_stack_above(Eo *obj, Eo *above);

/**
 *
 * Raise @p obj to the top of its layer.
 *
 * @p obj will, then, be the highest one in the layer it belongs
 * to. Object on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_lower()
 * 
 */
EAPI void evas_vg_node_raise(Eo *obj);

/**
 *
 * Lower @p obj to the bottom of its layer.
 *
 * @p obj will, then, be the lowest one in the layer it belongs
 * to. Objects on other layers won't get touched.
 *
 * @see evas_object_stack_above()
 * @see evas_object_stack_below()
 * @see evas_object_raise()
 * 
 *
 *
 */
EAPI void evas_vg_node_lower(Eo *obj);

#include "canvas/efl_vg.eo.legacy.h"

/**
 *
 * Get the stroke scaling factor used for stroking this path.
 * @since 1.14
 *
 *
 */
EAPI double evas_vg_shape_stroke_scale_get(Eo *obj);

/**
 *
 * Sets the stroke scale to be used for stroking the path.
 * the scale property will be used along with stroke width property.
 * @since 1.14
 *
 * @param[in] s stroke scale value
 *
 */
EAPI void evas_vg_shape_stroke_scale_set(Eo *obj, double s);

/**
 *
 * Gets the color used for stroking the path.
 * @since 1.14
 *
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_shape_stroke_color_get(Eo *obj, int *r, int *g, int *b, int *a);

/**
 *
 * Sets the color to be used for stroking the path.
 * @since 1.14
 *
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 */
EAPI void evas_vg_shape_stroke_color_set(Eo *obj, int r, int g, int b, int a);

/**
 *
 * Gets the stroke width to be used for stroking the path.
 * @since 1.14
 *
 *
 */
EAPI double evas_vg_shape_stroke_width_get(Eo *obj);

/**
 *
 * Sets the stroke width to be used for stroking the path.
 * @since 1.14
 *
 * @param[in] w stroke width to be used
 *
 */
EAPI void evas_vg_shape_stroke_width_set(Eo *obj, double w);

/**
 *
 * Not Implemented
 *
 *
 */
EAPI double evas_vg_shape_stroke_location_get(Eo *obj);

/**
 *
 * Not Implemented
 *
 * @param[in] centered
 *
 */
EAPI void evas_vg_shape_stroke_location_set(Eo *obj, double centered);

/**
 *
 * Not Implemented
 *
 * @param[out] dash
 * @param[out] length
 *
 */
EAPI void evas_vg_shape_stroke_dash_get(Eo *obj, const Efl_Gfx_Dash **dash, unsigned int *length);

/**
 *
 * Not Implemented
 *
 * @param[in] dash
 * @param[in] length
 *
 */
EAPI void evas_vg_shape_stroke_dash_set(Eo *obj, const Efl_Gfx_Dash *dash, unsigned int length);

/**
 *
 * Gets the cap style used for stroking path.
 * @since 1.14
 *
 *
 */
EAPI Efl_Gfx_Cap evas_vg_shape_stroke_cap_get(Eo *obj);

/**
 *
 * Sets the cap style to be used for stroking the path.
 * The cap will be used for capping the end point of a 
 * open subpath.
 *
 * @see Efl_Gfx_Cap
 * @since 1.14
 *
 * @param[in] c cap style to use , default is EFL_GFX_CAP_BUTT
 *
 */
EAPI void evas_vg_shape_stroke_cap_set(Eo *obj, Efl_Gfx_Cap c);

/**
 *
 * Gets the join style used for stroking path.
 * @since 1.14
 *
 *
 */
EAPI Efl_Gfx_Join evas_vg_shape_stroke_join_get(Eo *obj);

/**
 *
 * Sets the join style to be used for stroking the path.
 * The join style will be used for joining the two line segment
 * while stroking the path.
 *
 * @see Efl_Gfx_Join
 * @since 1.14
 *
 * @param[in] j join style to use , default is
EFL_GFX_JOIN_MITER
 *
 */
EAPI void evas_vg_shape_stroke_join_set(Eo *obj, Efl_Gfx_Join j);

/**
 *
 * Set the list of commands and points to be used to create the
 * content of shape.
 *
 * @note see efl_gfx_path interface for how to create a command list.
 * @see Efl_Gfx_Path_Command
 * @since 1.14
 *
 * @param[in] op command list
 * @param[in] points point list
 *
 */
EAPI void evas_vg_shape_shape_path_set(Eo *obj, const Efl_Gfx_Path_Command *op, const double *points);

/**
 *
 * Gets the command and points list
 * @since 1.14
 *
 * @param[out] op command list
 * @param[out] points point list
 *
 */

EAPI void evas_vg_shape_shape_path_get(Eo *obj, const Efl_Gfx_Path_Command **op, const double **points);
EAPI void evas_vg_shape_shape_path_length_get(Eo *obj, unsigned int *commands, unsigned int *points);
EAPI void evas_vg_shape_shape_current_get(Eo *obj, double *x, double *y);
EAPI void evas_vg_shape_shape_current_ctrl_get(Eo *obj, double *x, double *y);

/**
 *
 * Copy the shape data from the object specified .
 *
 * @since 1.14
 *
 *
 * @param[in] dup_from Shape object from where data will be copied.
 *
 */
EAPI void evas_vg_shape_shape_dup(Eo *obj, Eo *dup_from);

/**
 *
 * Reset the shape data of the shape object.
 *
 * @since 1.14
 * 
 *
 *
 */
EAPI void evas_vg_shape_shape_reset(Eo *obj);

/**
 *
 * Moves the current point to the given point, 
 * implicitly starting a new subpath and closing the previous one.
 *
 * @see efl_gfx_path_append_close()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of the current point.
 * @param[in] y Y co-ordinate of the current point.
 *
 */
EAPI void evas_vg_shape_shape_append_move_to(Eo *obj, double x, double y);

/**
 *
 * Adds a straight line from the current position to the given endPoint.
 * After the line is drawn, the current position is updated to be at the end
 * point of the line.
 *
 * @note if no current position present, it draws a line to itself, basically
 * a point.
 *
 * @see efl_gfx_path_append_move_to()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 *
 */
EAPI void evas_vg_shape_shape_append_line_to(Eo *obj, double x, double y);

/**
 *
 * Adds a quadratic Bezier curve between the current position and the
 * given end point (x,y) using the control points specified by (ctrl_x, ctrl_y).
 * After the path is drawn, the current position is updated to be at the end
 * point of the path.
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 * @param[in] ctrl_x X co-ordinate of control point.
 * @param[in] ctrl_y Y co-ordinate of control point.
 *
 */
EAPI void evas_vg_shape_shape_append_quadratic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y);

/**
 *
 * Same as efl_gfx_path_append_quadratic_to() api only difference is that it
 * uses the current control point to draw the bezier.
 *
 * @see efl_gfx_path_append_quadratic_to()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 *
 */
EAPI void evas_vg_shape_shape_append_squadratic_to(Eo *obj, double x, double y);

/**
 *
 * Adds a cubic Bezier curve between the current position and the
 * given end point (x,y) using the control points specified by
 * (ctrl_x0, ctrl_y0), and (ctrl_x1, ctrl_y1). After the path is drawn,
 * the current position is updated to be at the end point of the path.
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 * @param[in] ctrl_x0 X co-ordinate of 1st control point.
 * @param[in] ctrl_y0 Y co-ordinate of 1st control point.
 * @param[in] ctrl_x1 X co-ordinate of 2nd control point.
 * @param[in] ctrl_y1 Y co-ordinate of 2nd control point.
 *
 */
EAPI void evas_vg_shape_shape_append_cubic_to(Eo *obj, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1);

/**
 *
 * Same as efl_gfx_path_append_cubic_to() api only difference is that it uses
 * the current control point to draw the bezier.
 *
 * @see efl_gfx_path_append_cubic_to()
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the line.
 * @param[in] y Y co-ordinate of end point of the line.
 * @param[in] ctrl_x X co-ordinate of 2nd control point.
 * @param[in] ctrl_y Y co-ordinate of 2nd control point.
 *
 */
EAPI void evas_vg_shape_shape_append_scubic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y);

/**
 *
 * Append an arc that connects from the current point int the point list
 * to the given point (x,y). The arc is defined by the given radius in 
 * x-direction (rx) and radius in y direction (ry) . 
 *
 * @note Use this api if you know the end point's of the arc otherwise
 * use more convenient function efl_gfx_path_append_arc_to()
 *
 * @see efl_gfx_path_append_arc_to()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of end point of the arc.
 * @param[in] y Y co-ordinate of end point of the arc.
 * @param[in] rx radius of arc in x direction.
 * @param[in] ry radius of arc in y direction.
 * @param[in] angle x-axis rotation , normally 0.
 * @param[in] large_arc Defines whether to draw the larger arc or smaller arc joining two point.
 * @param[in] sweep Defines whether the arc will be drawn counter-clockwise or clockwise from current point to the end point taking into account the large_arc property.
 *
 */
EAPI void evas_vg_shape_shape_append_arc_to(Eo *obj, double x, double y, double rx, double ry, double angle, Eina_Bool large_arc, Eina_Bool sweep);

/**
 * @brief Append an arc that enclosed in the given rectangle (x, y, w, h). The
 * angle is defined in counter clock wise , use -ve angle for clockwise arc.
 *
 * @param[in] y Y co-ordinate of the rect.
 * @param[in] w width of the rect.
 * @param[in] h height of the rect.
 * @param[in] start_angle Angle at which the arc will start
 * @param[in] sweep_length @ Length of the arc.
 *
 * @since 1.18
 *
 * @ingroup Efl_Gfx_Shape
 */
EAPI void evas_vg_shape_shape_append_arc(Eo *obj, double x, double y, double w, double h, double start_angle, double sweep_length);

/**
 *
 * Closes the current subpath by drawing a line to the beginning of the subpath,
 * automatically starting a new path. The current point of the new path is
 * (0, 0).
 *
 * @note If the subpath does not contain any points, this function does nothing.
 *
 * @since 1.14
 *
 *
 *
 */
EAPI void evas_vg_shape_shape_append_close(Eo *obj);

/**
 *
 * Append a circle with given center and radius.
 *
 * @see efl_gfx_path_append_arc()
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of the center of the circle.
 * @param[in] y Y co-ordinate of the center of the circle.
 * @param[in] radius radius of the circle.
 *
 */
EAPI void evas_vg_shape_shape_append_circle(Eo *obj, double x, double y, double radius);

/**
 *
 * Append the given rectangle with rounded corner to the path.
 *
 * The xr and yr arguments specify the radii of the ellipses defining the
 * corners of the rounded rectangle.
 *
 * @note xr and yr are specified in terms of width and height respectively.
 *
 * @note if xr and yr are 0, then it will draw a rectangle without rounded corner.
 *
 * @since 1.14
 *
 *
 * @param[in] x X co-ordinate of the rectangle.
 * @param[in] y Y co-ordinate of the rectangle.
 * @param[in] w Width of the rectangle.
 * @param[in] h Height of the rectangle.
 * @param[in] rx The x radius of the rounded corner and should be in range [ 0 to w/2 ]
 * @param[in] ry The y radius of the rounded corner and should be in range [ 0 to h/2 ]
 *
 */
EAPI void evas_vg_shape_shape_append_rect(Eo *obj, double x, double y, double w, double h, double rx, double ry);

EAPI void evas_vg_shape_shape_append_svg_path(Eo *obj, const char *svg_path_data);
EAPI Eina_Bool evas_vg_shape_shape_interpolate(Eo *obj, const Eo *from, const Eo *to, double pos_map);
EAPI Eina_Bool evas_vg_shape_shape_equal_commands(Eo *obj, const Eo *with);

#include "canvas/efl_vg_shape.eo.legacy.h"

/**
 *
 * Set the list of color stops for the gradient
 * @since 1.14
 *
 * @param[in] colors color stops list
 * @param[in] length length of the list
 *
 */
EAPI void evas_vg_gradient_stop_set(Eo *obj, const Efl_Gfx_Gradient_Stop *colors, unsigned int length);

/**
 *
 * get the list of color stops.
 * @since 1.14
 *
 * @param[out] colors color stops list
 * @param[out] length length of the list
 *
 */
EAPI void evas_vg_gradient_stop_get(Eo *obj, const Efl_Gfx_Gradient_Stop **colors, unsigned int *length);

/**
 *
 * Specifies the spread method that should be used for this gradient.
 * @since 1.14
 *
 * @param[in] s spread type to be used
 *
 */
EAPI void evas_vg_gradient_spread_set(Eo *obj, Efl_Gfx_Gradient_Spread s);

/**
 *
 * Returns the spread method use by this gradient. The default is
 * EFL_GFX_GRADIENT_SPREAD_PAD.
 * @since 1.14
 *
 *
 */
EAPI Efl_Gfx_Gradient_Spread evas_vg_gradient_spread_get(Eo *obj);

#include "canvas/efl_vg_gradient.eo.legacy.h"

/**
 *
 * Sets the start point of this linear gradient.
 *
 * @param[in] x x co-ordinate of start point
 * @param[in] y y co-ordinate of start point
 *
 */
EAPI void evas_vg_gradient_linear_start_set(Eo *obj, double x, double y);

/**
 *
 * Gets the start point of this linear gradient.
 *
 * @param[out] x x co-ordinate of start point
 * @param[out] y y co-ordinate of start point
 *
 */
EAPI void evas_vg_gradient_linear_start_get(Eo *obj, double *x, double *y);

/**
 *
 * Sets the end point of this linear gradient.
 *
 * @param[in] x x co-ordinate of end point
 * @param[in] y y co-ordinate of end point
 *
 */
EAPI void evas_vg_gradient_linear_end_set(Eo *obj, double x, double y);

/**
 *
 * Gets the end point of this linear gradient.
 *
 * @param[out] x x co-ordinate of end point
 * @param[out] y y co-ordinate of end point
 *
 */
EAPI void evas_vg_gradient_linear_end_get(Eo *obj, double *x, double *y);

#include "canvas/efl_vg_gradient_linear.eo.legacy.h"

/**
 *
 * Sets the center of this radial gradient.
 *
 * @param[in] x x co-ordinate of center point
 * @param[in] y y co-ordinate of center point
 *
 */
EAPI void evas_vg_gradient_radial_center_set(Eo *obj, double x, double y);

/**
 *
 * Gets the center of this radial gradient.
 *
 * @param[out] x x co-ordinate of center point
 * @param[out] y y co-ordinate of center point
 *
 */
EAPI void evas_vg_gradient_radial_center_get(Eo *obj, double *x, double *y);

/**
 *
 * Sets the center radius of this radial gradient.
 *
 * @param[in] r center radius
 *
 */
EAPI void evas_vg_gradient_radial_radius_set(Eo *obj, double r);

/**
 *
 * Gets the center radius of this radial gradient.
 *
 *
 */
EAPI double evas_vg_gradient_radial_radius_get(Eo *obj);

/**
 *
 * Sets the focal point of this radial gradient.
 *
 * @param[in] x x co-ordinate of focal point
 * @param[in] y y co-ordinate of focal point
 *
 */
EAPI void evas_vg_gradient_radial_focal_set(Eo *obj, double x, double y);

/**
 *
 * Gets the focal point of this radial gradient.
 *
 * @param[out] x x co-ordinate of focal point
 * @param[out] y y co-ordinate of focal point
 *
 */
EAPI void evas_vg_gradient_radial_focal_get(Eo *obj, double *x, double *y);

#include "canvas/efl_vg_gradient_radial.eo.legacy.h"

#endif

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
 * @def EVAS_NATIVE_SURFACE_VERSION
 * Magic version number to know what the native surface struct looks like
 */

#define EVAS_NATIVE_SURFACE_VERSION 3

/**
 * Native surface types that image object supports
 *
 * @see Evas_Native_Surface
 * @see evas_object_image_native_surface_set()
 */
typedef enum _Evas_Native_Surface_Type
{
   EVAS_NATIVE_SURFACE_NONE, /**< No surface type */
   EVAS_NATIVE_SURFACE_X11,  /**< X Window system based type. pixmap id or visual of the pixmap */
   EVAS_NATIVE_SURFACE_OPENGL, /**< OpenGL system based type. texture or framebuffer id*/
   EVAS_NATIVE_SURFACE_WL, /**< Wayland system based type. buffer of surface */
   EVAS_NATIVE_SURFACE_TBM, /**< Tizen system based type. tbm surface @since 1.14  */
   EVAS_NATIVE_SURFACE_EVASGL, /**< Evas GL based type. evas gl surface @since 1.14 */
   EVAS_NATIVE_SURFACE_WL_DMABUF, /**< Wayland system based type. using dmabuf @since 1.18 */
} Evas_Native_Surface_Type;

/**
 * @brief A generic datatype for engine specific native surface information.
 *
 * Please fill up Evas_Native_Surface fields that regarded with current surface
 * type. If you want to set the native surface type to
 * EVAS_NATIVE_SURFACE_X11, you need to set union data with x11.visual or
 * x11.pixmap. If you need to set the native surface as
 * EVAS_NATIVE_SURFACE_OPENGL, on the other hand, you need to set union data
 * with opengl.texture_id or opengl.framebuffer_id and so on.
 * If you need to set the native surface as EVAS_NATIVE_SURFACE_WL,
 * you need to set union data with wl.legacy_buffer.
 * If you need to set the native surface as EVAS_NATIVE_SURFACE_TBM,
 * you need to set union data with tbm surface. The version field
 * should be set with EVAS_NATIVE_SURFACE_VERSION in order to check abi
 * break in your application on the different efl library versions.
 *
 * @warning Native surface types totally depend on the system. Please
 *          be aware that the types are supported on your system before using
 *          them.
 *
 * @note The information stored in an @c Evas_Native_Surface returned by
 *       @ref evas_gl_native_surface_get() is not meant to be used by
 *       applications except for passing it to
 *       @ref evas_object_image_native_surface_set().
 *
 * @see evas_object_image_native_surface_set()
 */
struct _Evas_Native_Surface
{
   int                      version; /**< Current Native Surface Version. Use EVAS_NATIVE_SURFACE_VERSION */
   Evas_Native_Surface_Type type; /**< Surface type. @see Evas_Native_Surface_Type */
   union {
      struct
      {
         void         *visual; /**< visual of the pixmap to use (Visual) */
         unsigned long pixmap; /**< pixmap id to use (Pixmap) */
      } x11; /**< Set this struct fields if surface data is X11 based. */

      struct
      {
         unsigned int texture_id; /**< opengl texture id to use from glGenTextures() */
         unsigned int framebuffer_id; /**< 0 if not a FBO, FBO id otherwise from glGenFramebuffers() */
         unsigned int internal_format; /**< same as 'internalFormat' for glTexImage2D() */
         unsigned int format; /**< same as 'format' for glTexImage2D() */
         unsigned int x, y, w, h; /**< region inside the texture to use (image size is assumed as texture size, with 0, 0 being the top-left and co-ordinates working down to the right and bottom being positive) */
      } opengl; /**< Set this struct fields if surface data is OpenGL based. */
      struct
      {
         void *legacy_buffer; /**< wayland client buffer to use */
      } wl; /**< Set this struct fields if surface data is Wayland based. */
      struct
      {
         void *attr; /**< Pointer to dmabuf attributes - contents copied */
         void *resource; /**< Wayland resource pointer, kept as is */
      } wl_dmabuf; /**< Set this struct fields if surface data is Wayland dmabuf based. @since 1.18 */
      struct
      {
         void *buffer; /**< tbm surface buffer to use @since 1.14 */
      } tbm; /**< Set this struct fields if surface data is Tizen based. @since 1.14 */
      struct
      {
         void *surface; /**< evas gl surface to use @since 1.14 */
      } evasgl; /**< Set this struct fields if surface data is Evas GL based. @since 1.14 */
   } data; /**< Choose one union data according to your surface. */
};

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
 * @brief Get the native surface of a given image of the canvas
 *
 * This function returns the native surface of a given canvas image.
 *
 * @return The native surface.
 */
EAPI Evas_Native_Surface          *evas_object_image_native_surface_get(const Evas_Object *obj);

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
 *
 * Set the source file from where an image object must fetch the real
 * image data (it may be an Eet file, besides pure image ones).
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
 * {
 * fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
 * valid_path, evas_load_error_str(err));
 * }
 * else
 * {
 * evas_object_image_fill_set(img, 0, 0, w, h);
 * evas_object_resize(img, w, h);
 * evas_object_show(img);
 * }
 * @endcode
 *
 * @param[in] file The image file path.
 * @param[in] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_file_set(Eo *obj, const char *file, const char *key);

/**
 *
 * Retrieve the source file from where an image object is to fetch the
 * real image data (it may be an Eet file, besides pure image ones).
 *
 * You must @b not modify the strings on the returned pointers.
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @param[out] file The image file path.
 * @param[out] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_file_get(const Eo *obj, const char **file, const char **key);

/**
 *
 * Set the source mmaped file from where an image object must fetch the real
 * image data (it must be an Eina_File).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the image in
 * this file.
 *
 * @since 1.8
 *
 * @param[in] f The mmaped file
 * @param[in] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_mmap_set(Eo *obj, const Eina_File *f, const char *key);

/**
 *
 * Get the source mmaped file from where an image object must fetch the real
 * image data (it must be an Eina_File).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can get the key to be used as the index of the image in
 * this file.
 *
 * @since 1.10
 *
 * @param[out] f The mmaped file
 * @param[out] key The image key in @p file (if its an Eet one), or @c
NULL, otherwise.
 */
EAPI void evas_object_image_mmap_get(const Eo *obj, const Eina_File **f, const char **key);

/**
 *
 * Save the given image object's contents to an (image) file.
 * Proxy object is image object, but it doesn't have contents. So you can't use this function for proxy objects.
 *
 * The extension suffix on @p file will determine which <b>saver
 * module</b> Evas is to use when saving, thus the final file's
 * format. If the file supports multiple data stored in it (Eet ones),
 * you can specify the key to be used as the index of the image in it.
 *
 * You can specify some flags when saving the image.  Currently
 * acceptable flags are @c quality and @c compress. Eg.: @c
 * "quality=100 compress=9"
 *
 * quality is hint for the quality of image,0-100. 0 means low quality
 * and saved image size is small. 100 means high quality and saved image size is big.
 *
 * compress is hint for the compression modes (Eet ones) or for the compression flags (Png one)
 * (1 == compress, 0 = don't compress).
 *
 * @param[in] file The filename to be used to save the image (extension
obligatory).
 * @param[in] key The image key in the file (if an Eet one), or @c NULL,
otherwise.
 * @param[in] flags String containing the flags to be used (@c NULL for
none).
 */
EAPI Eina_Bool evas_object_image_save(const Eo *obj, const char *file, const char *key, const char *flags) EINA_ARG_NONNULL(2);

/**
 *
 * Check if an image object can be animated (have multiple frames)
 *
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
 * {
 * int frame_count;
 * int loop_count;
 * Evas_Image_Animated_Loop_Hint loop_type;
 * double duration;
 *
 * frame_count = evas_object_image_animated_frame_count_get(obj);
 * printf("This image has %d frames\n",frame_count);
 *
 * duration = evas_object_image_animated_frame_duration_get(obj,1,0);
 * printf("Frame 1's duration is %f. You had better set object's frame to 2 after this duration using timer\n");
 *
 * loop_count = evas_object_image_animated_loop_count_get(obj);
 * printf("loop count is %d. You had better run loop %d times\n",loop_count,loop_count);
 *
 * loop_type = evas_object_image_animated_loop_type_get(obj);
 * if (loop_type == EVAS_IMAGE_ANIMATED_HINT_LOOP)
 * printf("You had better set frame like 1->2->3->1->2->3...\n");
 * else if (loop_type == EVAS_IMAGE_ANIMATED_HINT_PINGPONG)
 * printf("You had better set frame like 1->2->3->2->1->2...\n");
 * else
 * printf("Unknown loop type\n");
 *
 * evas_object_image_animated_frame_set(obj,1);
 * printf("You set image object's frame to 1. You can see frame 1\n");
 * }
 * @endcode
 *
 * @see evas_object_image_animated_get()
 * @see evas_object_image_animated_frame_count_get()
 * @see evas_object_image_animated_loop_type_get()
 * @see evas_object_image_animated_loop_count_get()
 * @see evas_object_image_animated_frame_duration_get()
 * @see evas_object_image_animated_frame_set()
 * @since 1.1
 *
 */
EAPI Eina_Bool evas_object_image_animated_get(const Eo *obj);

/**
 * @brief Set the frame to current frame of an image object.
 *
 * This set image object's current frame to frame_num with 1 being the first
 * frame.
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_loop_count_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @param[in] frame_index The index of current frame.
 *
 * @since 1.1
 */
EAPI void evas_object_image_animated_frame_set(Evas_Object *obj, int frame_index);

/**
 * @brief Get the total number of frames of the image object.
 *
 * This returns total number of frames the image object supports (if animated).
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_loop_count_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @return The number of frames.
 *
 * @since 1.1
 */
EAPI int evas_object_image_animated_frame_count_get(const Evas_Object *obj);

/**
 * @brief Get the kind of looping the image object does.
 *
 * This returns the kind of looping the image object wants to do.
 *
 * If it returns EVAS_IMAGE_ANIMATED_HINT_LOOP, you should display frames in a
 * sequence like: 1->2->3->1->2->3->1...
 *
 * If it returns EVAS_IMAGE_ANIMATED_HINT_PINGPONG, it is better to display
 * frames in a sequence like: 1->2->3->2->1->2->3->1...
 *
 * The default type is EVAS_IMAGE_ANIMATED_HINT_LOOP.
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_count_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @return Loop type of the image object.
 *
 * @since 1.1
 */
EAPI Evas_Image_Animated_Loop_Hint evas_object_image_animated_loop_type_get(const Evas_Object *obj);

/**
 * @brief Get the number times the animation of the object loops.
 *
 * This returns loop count of image. The loop count is the number of times the
 * animation will play fully from first to last frame until the animation
 * should stop (at the final frame).
 *
 * If 0 is returned, then looping should happen indefinitely (no limit to the
 * number of times it loops).
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_frame_duration_get.
 *
 * @return The number of loop of an animated image object.
 *
 * @since 1.1
 */
EAPI int evas_object_image_animated_loop_count_get(const Evas_Object *obj);

/**
 * @brief Get the duration of a sequence of frames.
 *
 * This returns total duration that the specified sequence of frames should
 * take in seconds.
 *
 * If you set start_frame to 1 and frame_num 0, you get frame 1's duration. If
 * you set start_frame to 1 and frame_num 1, you get frame 1's duration +
 * frame2's duration.
 *
 * See also @ref evas_object_image_animated_get,
 * @ref evas_object_image_animated_frame_count_get,
 * @ref evas_object_image_animated_loop_type_get,
 * @ref evas_object_image_animated_loop_count_get,
 *
 * @param[in] frame_num Number of frames in the sequence.
 *
 * @since 1.1
 */
EAPI double evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int frame_num);

/**
 * @brief Set the DPI resolution of an image object's source image.
 *
 * This function sets the DPI resolution of a given loaded canvas image. Most
 * useful for the SVG image loader.
 *
 * @param[in] dpi The DPI resolution.
 */
EAPI void evas_object_image_load_dpi_set(Evas_Object *obj, double dpi);

/**
 * @brief Get the DPI resolution of a loaded image object in the canvas.
 *
 * This function returns the DPI resolution of the given canvas image.
 *
 * @return The DPI resolution.
 */
EAPI double evas_object_image_load_dpi_get(const Evas_Object *obj);

/**
 *
 * Set the load size of a given image object's source image.
 *
 * This function sets a new geometry size for the given canvas image.
 * The image will be loaded into memory as if it was the set size instead of
 * the original size.
 *
 * @note The size of a given image object's source image will be less than or
 * equal to the size of @p w and @p h.
 *
 * @see evas_object_image_load_size_get()
 *
 * @param[in] w The new width of the image's load size.
 * @param[in] h The new height of the image's load size.
 */
EAPI void evas_object_image_load_size_set(Eo *obj, int w, int h);

/**
 *
 * Get the load size of a given image object's source image.
 *
 * This function gets the geometry size set manually for the given canvas image.
 *
 * @note Use @c NULL pointers on the size components you're not
 * interested in: they'll be ignored by the function.
 * @note @p w and @p h will be set with the image's loading size only if
 * the image's load size is set manually: if evas_object_image_load_size_set()
 * has not been called, @p w and @p h will be set with 0.
 *
 * @see evas_object_image_load_size_set() for more details
 *
 * @param[out] w The new width of the image's load size.
 * @param[out] h The new height of the image's load size.
 */
EAPI void evas_object_image_load_size_get(const Eo *obj, int *w, int *h);

/**
 * @brief Inform a given image object to load a selective region of its source
 * image.
 *
 * This function is useful when one is not showing all of an image's area on
 * its image object.
 *
 * @note The image loader for the image format in question has to support
 * selective region loading in order to this function to take effect.
 *
 * @param[in] x X-offset of the region to be loaded.
 * @param[in] y Y-offset of the region to be loaded.
 * @param[in] w Width of the region to be loaded.
 * @param[in] h Height of the region to be loaded.
 */
EAPI void evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief Retrieve the coordinates of a given image object's selective (source
 * image) load region.
 *
 * @note Use @c null pointers on the coordinates you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[out] x X-offset of the region to be loaded.
 * @param[out] y Y-offset of the region to be loaded.
 * @param[out] w Width of the region to be loaded.
 * @param[out] h Height of the region to be loaded.
 */
EAPI void evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

/**
 * @brief Get the support state of a given image.
 *
 * @return The region support state.
 *
 * @since 1.2
 *
 * @ingroup Evas_Image
 */
EAPI Eina_Bool evas_object_image_region_support_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Define if the orientation information in the image file should be
 * honored.
 *
 * @param[in] enable @c true means that it should honor the orientation
 * information.
 *
 * @since 1.1
 */
EAPI void evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable);

/**
 * @brief Get if the orientation information in the image file should be
 * honored.
 *
 * @return @c true means that it should honor the orientation information.
 *
 * @since 1.1
 */
EAPI Eina_Bool evas_object_image_load_orientation_get(const Evas_Object *obj);

/**
 * @brief Set the scale down factor of a given image object's source image,
 * when loading it.
 *
 * This function sets the scale down factor of a given canvas image. Most
 * useful for the SVG image loader.
 *
 * @param[in] scale_down The scale down factor.
 */
EAPI void evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down);

/**
 * @brief Get the scale down factor of a given image object's source image,
 * when loading it.
 *
 * @return The scale down factor.
 */
EAPI int evas_object_image_load_scale_down_get(const Evas_Object *obj);

/**
 * @brief Retrieves a number representing any error that occurred during the
 * last loading of the given image object's source image.
 *
 * @return A value giving the last error that occurred. It should be one of the
 * @ref Evas_Load_Error values. #EVAS_LOAD_ERROR_NONE is returned if there was
 * no error.
 */
EAPI Evas_Load_Error evas_object_image_load_error_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 *
 * Sets whether to use high-quality image scaling algorithm on the
 * given image object.
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
 *
 * @param[in] smooth_scale Whether to use smooth scale or not.
 */
EAPI void evas_object_image_smooth_scale_set(Eo *obj, Eina_Bool smooth_scale);

/**
 *
 * Retrieves whether the given image object is using high-quality
 * image scaling algorithm.
 *
 * @return Whether smooth scale is being used.
 *
 * See @ref evas_object_image_smooth_scale_set() for more details.
 *
 */
EAPI Eina_Bool evas_object_image_smooth_scale_get(const Eo *obj);

/**
 *
 * Sets the tiling mode for the given evas image object's fill.
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 *
 * @param[in] spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 */
EAPI void evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread);

/**
 *
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @return  The current spread mode of the image object.
 *
 */
EAPI Evas_Fill_Spread evas_object_image_fill_spread_get(const Evas_Object *obj);

/**
 *
 * Set how to fill an image object's drawing rectangle given the
 * (real) image bound to it.
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
 *
 * @param[in] x The x coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[in] y The y coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[in] w The width the bound image will be displayed at.
 * @param[in] h The height the bound image will be displayed at.
 */
EAPI void evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 *
 * Retrieve how an image object is to fill its drawing rectangle,
 * given the (real) image bound to it.
 *
 * @note Use @c NULL pointers on the fill components you're not
 * interested in: they'll be ignored by the function.
 *
 * See @ref evas_object_image_fill_set() for more details.
 *
 * @param[out] x The x coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[out] y The y coordinate (from the top left corner of the bound
image) to start drawing from.
 * @param[out] w The width the bound image will be displayed at.
 * @param[out] h The height the bound image will be displayed at.
 */
EAPI void evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Set whether the image object's fill property should track the
 * object's size.
 *
 * If @c setting is @c true, then every @ref evas_object_resize will
 * automatically trigger a call to @ref evas_object_image_fill_set with the
 * that new size (and 0, 0 as source image's origin), so the bound image will
 * fill the whole object's area.
 *
 * @param[in] filled @c true to make the fill property follow object size or
 * @c false otherwise.
 */
EAPI void evas_object_image_filled_set(Evas_Object *obj, Eina_Bool filled);

/**
 * @brief Retrieve whether the image object's fill property should track the
 * object's size.
 *
 * Returns @c true if it is tracking, @c false if not (and @ref
 * evas_object_fill_set must be called manually).
 *
 * @return @c true to make the fill property follow object size or @c false
 * otherwise.
 */
EAPI Eina_Bool evas_object_image_filled_get(const Evas_Object *obj);

/**
 * @brief Retrieve whether alpha channel data is being used on the given image
 * object.
 *
 * This function returns @c true if the image object's alpha channel is being
 * used, or @c false otherwise.
 *
 * @return Whether to use alpha channel ($true) data or not ($false).
 *
 * @ingroup Evas_Image
 */
EAPI Eina_Bool evas_object_image_alpha_get(const Evas_Object *obj);

/**
 * @brief Enable or disable alpha channel usage on the given image object.
 *
 * This function sets a flag on an image object indicating whether or not to
 * use alpha channel data. A value of @c true makes it use alpha channel data,
 * and @c false makes it ignore that data. Note that this has nothing to do
 * with an object's color as  manipulated by @ref evas_object_color_set.
 *
 * @param[in] alpha Whether to use alpha channel ($true) data or not ($false).
 */
EAPI void evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool alpha);

/**
 * @brief Dimensions of this image's border, a region that does not scale with
 * the center area.
 *
 * When EFL renders an image, its source may be scaled to fit the size of the
 * object. This function sets an area from the borders of the image inwards
 * which is not to be scaled. This function is useful for making frames and for
 * widget theming, where, for example, buttons may be of varying sizes, but
 * their border size must remain constant.
 *
 * The units used for @c l, @c r, @c t and @c b are canvas units (pixels).
 *
 * @note The border region itself may be scaled by the
 * @ref evas_object_image_border_scale_set function.
 *
 * @note By default, image objects have no borders set, i.e. @c l, @c r, @c t
 * and @c b start as 0.
 *
 * @note Similar to the concepts of 9-patch images or cap insets.
 *
 * @param[in] l The border's left width.
 * @param[in] r The border's right width.
 * @param[in] t The border's top height.
 * @param[in] b The border's bottom height.
 */
EAPI void evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b);

/**
 * @brief Dimensions of this image's border, a region that does not scale with
 * the center area.
 *
 * When EFL renders an image, its source may be scaled to fit the size of the
 * object. This function sets an area from the borders of the image inwards
 * which is not to be scaled. This function is useful for making frames and for
 * widget theming, where, for example, buttons may be of varying sizes, but
 * their border size must remain constant.
 *
 * The units used for @c l, @c r, @c t and @c b are canvas units (pixels).
 *
 * @note The border region itself may be scaled by the
 * @ref evas_object_image_border_scale_set function.
 *
 * @note By default, image objects have no borders set, i.e. @c l, @c r, @c t
 * and @c b start as 0.
 *
 * @note Similar to the concepts of 9-patch images or cap insets.
 *
 * @param[out] l The border's left width.
 * @param[out] r The border's right width.
 * @param[out] t The border's top height.
 * @param[out] b The border's bottom height.
 */
EAPI void evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b);

/**
 * @brief Scaling factor applied to the image borders.
 *
 * This value multiplies the size of the @ref evas_object_image_border_get when
 * scaling an object.
 *
 * Default value is 1.0 (no scaling).
 *
 * @param[in] scale The scale factor.
 */
EAPI void evas_object_image_border_scale_set(Evas_Object *obj, double scale);

/**
 * @brief Scaling factor applied to the image borders.
 *
 * This value multiplies the size of the @ref evas_object_image_border_get when
 * scaling an object.
 *
 * Default value is 1.0 (no scaling).
 *
 * @return The scale factor.
 */
EAPI double evas_object_image_border_scale_get(const Evas_Object *obj);

/**
 * @brief Specifies how the center part of the object (not the borders) should
 * be drawn when EFL is rendering it.
 *
 * This function sets how the center part of the image object's source image is
 * to be drawn, which must be one of the values in @ref Evas_Border_Fill_Mode.
 * By center we mean the complementary part of that defined by
 * @ref evas_object_image_border_set. This one is very useful for making frames
 * and decorations. You would most probably also be using a filled image (as in
 * @ref Efl.Gfx.Fill.fill_auto) to use as a frame.
 *
 * The default value is
 *
 * @param[in] fill Fill mode of the center region of @c obj (a value in
 *  #Evas_Border_Fill_Mode).
 */
EAPI void evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill);

/**
 * @brief Specifies how the center part of the object (not the borders) should
 * be drawn when EFL is rendering it.
 *
 * This function sets how the center part of the image object's source image is
 * to be drawn, which must be one of the values in @ref Evas_Border_Fill_Mode.
 * By center we mean the complementary part of that defined by
 * @ref evas_object_image_border_set. This one is very useful for making frames
 * and decorations. You would most probably also be using a filled image (as in
 * @ref Efl.Gfx.Fill.fill_auto) to use as a frame.
 *
 * The default value is
 *
 * @return Fill mode of the center region of @c obj (a value in
 *  #Evas_Border_Fill_Mode).
 */
EAPI Evas_Border_Fill_Mode evas_object_image_border_center_fill_get(const Evas_Object *obj);

/**
 * @brief Set the image orientation.
 *
 * This function allows to rotate or flip the image.
 *
 * @param[in] orient The image orientation @ref Evas_Image_Orient. Default is
 * #EVAS_IMAGE_ORIENT_NONE.
 *
 * @since 1.14
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_orient_set(Evas_Object *obj, Evas_Image_Orient orient);

/**
 * @brief Get the image orientation.
 *
 * @return The image orientation @ref Evas_Image_Orient. Default is
 * #EVAS_IMAGE_ORIENT_NONE.
 *
 * @since 1.14
 *
 * @ingroup Evas_Image
 */
EAPI Evas_Image_Orient evas_object_image_orient_get(const Evas_Object *obj);

/**
 * @brief Set the content hint setting of a given image object of the canvas.
 *
 * This function sets the content hint value of the given image of the canvas.
 * For example, if you're on the GL engine and your driver implementation
 * supports it, setting this hint to #EVAS_IMAGE_CONTENT_HINT_DYNAMIC will make
 * it need zero copies at texture upload time, which is an "expensive"
 * operation.
 *
 * @param[in] hint The content hint value, one of the
 * @ref Evas_Image_Content_Hint ones.
 */
EAPI void evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint);

/**
 * @brief Get the content hint setting of a given image object of the canvas.
 *
 * This returns #EVAS_IMAGE_CONTENT_HINT_NONE on error.
 *
 * @return The content hint value, one of the @ref Evas_Image_Content_Hint
 * ones.
 */
EAPI Evas_Image_Content_Hint evas_object_image_content_hint_get(const Evas_Object *obj);

/**
 * @brief Set the scale hint of a given image of the canvas.
 *
 * This function sets the scale hint value of the given image object in the
 * canvas, which will affect how Evas is to cache scaled versions of its
 * original source image.
 *
 * @param[in] hint The scale hint, a value in @ref Evas_Image_Scale_Hint.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint);

/**
 * @brief Get the scale hint of a given image of the canvas.
 *
 * This function returns the scale hint value of the given image object of the
 * canvas.
 *
 * @return The scale hint, a value in @ref Evas_Image_Scale_Hint.
 *
 * @ingroup Evas_Image
 */
EAPI Evas_Image_Scale_Hint evas_object_image_scale_hint_get(const Evas_Object *obj);

/**
 *
 * Sets the size of the given image object.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 *
 * @param[in] w The new width of the image.
 * @param[in] h The new height of the image.
 */
EAPI void evas_object_image_size_set(Evas_Object *obj, int w, int h);

/**
 *
 * Retrieves the size of the given image object.
 *
 * See @ref evas_object_image_size_set() for more details.
 *
 * @param[out] w The new width of the image.
 * @param[out] h The new height of the image.
 */
EAPI void evas_object_image_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Set the colorspace of a given image of the canvas.
 *
 * This function sets the colorspace of given canvas image.
 *
 * @param[in] cspace The new color space.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace);

/**
 * @brief Get the colorspace of a given image of the canvas.
 *
 * This function returns the colorspace of given canvas image.
 *
 * @return The new color space.
 *
 * @ingroup Evas_Image
 */
EAPI Evas_Colorspace evas_object_image_colorspace_get(const Evas_Object *obj);

/**
 * @brief Retrieves the row stride of the given image object.
 *
 * The row stride is the number of bytes between the start of a row and the
 * start of the next row for image data.
 *
 * @return The stride of the image (in bytes).
 *
 * @ingroup Evas_Image
 */
EAPI int evas_object_image_stride_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Replaces the raw image data of the given image object.
 *
 * This function lets the application replace an image object's internal pixel
 * buffer with an user-allocated one. For best results, you should generally
 * first call @ref evas_object_image_size_set with the width and height for the
 * new buffer.
 *
 * This call is best suited for when you will be using image data with
 * different dimensions than the existing image data, if any. If you only need
 * to modify the existing image in some fashion, then using
 * @ref evas_object_image_data_get is probably what you are after.
 *
 * Note that the caller is responsible for freeing the buffer when finished
 * with it, as user-set image data will not be automatically freed when the
 * image object is deleted.
 *
 * @param[in] data The raw data to replace.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_data_copy_set(Evas_Object *obj, void *data);

/**
 * @brief Sets the raw image data of the given image object.
 *
 * Note that the raw data must be of the same size (see @ref
 * evas_object_image_size_set, which has to be called before this one) and
 * colorspace (see @Evas.Image.colorspace.set) of the image. If data is
 * @c null, the current image data will be freed. Naturally, if one does not
 * set an image object's data manually, it will still have one, allocated by
 * Evas.
 *
 * @param[in] data The raw data, or @c null.
 *
 * @ingroup Evas_Image
 */
EAPI void evas_object_image_data_set(Evas_Object *obj, void *data);

/**
 * @brief Get a pointer to the raw image data of the given image object.
 *
 * This function returns a pointer to an image object's internal pixel buffer,
 * for reading only or read/write. If you request it for writing, the image
 * will be marked dirty so that it gets redrawn at the next update.
 *
 * Each time you call this function on an image object, its data buffer will
 * have an internal reference counter incremented. Decrement it back by using
 * @ref evas_object_image_data_set.
 *
 * This is best suited for when you want to modify an existing image, without
 * changing its dimensions.
 *
 * @note The contents' format returned by it depend on the color space of the
 * given image object.
 *
 * @note You may want to use @Evas.Image.data_update_add to inform data
 * changes, if you did any.
 *
 * @param[in] for_writing Whether the data being retrieved will be modified
 * ($true) or not ($false).
 *
 * @return The raw image data.
 *
 * @ingroup Evas_Image
 */
EAPI void *evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Mark a sub-region of the given image object to be redrawn.
 *
 * This function schedules a particular rectangular region of an image object
 * to be updated (redrawn) at the next rendering cycle.
 *
 * @param[in] y Y-offset of the region to be updated.
 * @param[in] w Width of the region to be updated.
 * @param[in] h Height of the region to be updated.
 */
EAPI void evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief The content below the Evas_Object_Image will be rendered inside it
 * and you can reuse it as a source for any kind of effect.
 *
 * @param[in] s Wether to put the content of the canvas below inside the
 * Evas_Object_Image.
 *
 * @since 1.15
 */
EAPI void evas_object_image_snapshot_set(Evas_Object *obj, Eina_Bool s);

/**
 * @brief Determine wether the Evas_Object_Image replicate the content of the
 * canvas below.
 *
 * @return Wether to put the content of the canvas below inside the
 * Evas_Object_Image.
 *
 * @since 1.15
 */
EAPI Eina_Bool evas_object_image_snapshot_get(const Evas_Object *obj);

/**
 * @brief Set the source object on an image object to used as a proxy.
 *
 * If an image object is set to behave as a proxy, it will mirror the rendering
 * contents of a given source object in its drawing region, without affecting
 * that source in any way. The source must be another valid Evas object. Other
 * effects may be applied to the proxy, such as a map (see @ref
 * evas_object_map_set) to create a reflection of the original object (for
 * example).
 *
 * Any existing source object on @c obj will be removed after this call.
 * Setting @c src to @c null clears the proxy object (not in "proxy state"
 * anymore).
 *
 * @warning You cannot set a proxy as another proxy's source.
 *
 * @param[in] src Source object to use for the proxy.
 */
EAPI Eina_Bool evas_object_image_source_set(Evas_Object *obj, Evas_Object *src);

/**
 * @brief Get the current source object of an image object.
 *
 * @return Source object to use for the proxy.
 */
EAPI Evas_Object *evas_object_image_source_get(const Evas_Object *obj);

/**
 * @brief Clip the proxy object with the source object's clipper.
 *
 * @param[in] source_clip Whether @c obj is clipped by the source clipper
 * ($true) or not ($false).
 *
 * @since 1.8
 */
EAPI void evas_object_image_source_clip_set(Evas_Object *obj, Eina_Bool source_clip);

/**
 * @brief Determine whether an object is clipped by source object's clipper.
 *
 * @return Whether @c obj is clipped by the source clipper ($true) or not
 * ($false).
 *
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_clip_get(const Evas_Object *obj);

/**
 * @brief Set whether an Evas object is to source events.
 *
 * Set whether an Evas object is to repeat events to source.
 *
 * If @c repeat is @c true, it will make events on @c obj to also be repeated
 * for the source object (see @ref evas_object_image_source_set). Even the
 * @c obj and source geometries are different, the event position will be
 * transformed to the source object's space.
 *
 * If @c repeat is @c false, events occurring on @c obj will be processed only
 * on it.
 *
 * @param[in] source Whether @c obj is to pass events ($true) or not ($false).
 *
 * @since 1.8
 */
EAPI void evas_object_image_source_events_set(Evas_Object *obj, Eina_Bool repeat);

/**
 * @brief Determine whether an object is set to source events.
 *
 * @return Whether @c obj is to pass events ($true) or not ($false).
 *
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_events_get(const Evas_Object *obj);

/**
 * @brief Set the source object to be visible or not.
 *
 * If @c visible is set to @c false, the source object of the proxy ($obj) will
 * be invisible.
 *
 * This API works differently to @ref evas_object_show and @ref
 * evas_object_hide. Once source object is hidden, the proxy object will be
 * hidden as well. Actually in this case both objects are excluded from the
 * Evas internal update circle.
 *
 * By this API, instead, one can toggle the visibility of a proxy's source
 *  object remaining the proxy visibility untouched.
 *
 * @warning If the all of proxies are deleted, then the source visibility of
 * the source object will be cancelled.
 *
 * @param[in] visible @c true is source object to be shown, @c false otherwise.
 *
 * @deprecated Please use evas_object_norender_get() on the source instead.
 *
 * @since 1.8
 */
EAPI void evas_object_image_source_visible_set(Evas_Object *obj, Eina_Bool visible);

/**
 * @brief Get the state of the source object visibility.
 *
 * @return @c true is source object to be shown, @c false otherwise.
 *
 * @deprecated Please use evas_object_norender_get() on the source instead.
 *
 * @since 1.8
 */
EAPI Eina_Bool evas_object_image_source_visible_get(const Evas_Object *obj);

/**
 * @brief Mark whether the given image object is dirty and needs to request its
 * pixels.
 *
 * This function will only properly work if a pixels get callback has been set.
 *
 * @warning Use this function if you really know what you are doing.
 *
 * @param[in] dirty Whether the image is dirty.
 */
EAPI void evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty);

/**
 * @brief Retrieves whether the given image object is dirty (needs to be
 * redrawn).
 *
 * @return Whether the image is dirty.
 */
EAPI Eina_Bool evas_object_image_pixels_dirty_get(const Evas_Object *obj);

/**
 * @brief Set the callback function to get pixels from a canvas' image.
 *
 * This functions sets a function to be the callback function that get pixels
 * from a image of the canvas.
 *
 * @param[in] func The callback function.
 * @param[in] data The data pointer to be passed to @c func.
 */
EAPI void evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(2);


/**
 * @typedef Evas_Video_Surface
 *
 * A generic datatype for video specific surface information
 * @see evas_object_image_video_surface_set
 * @see evas_object_image_video_surface_get
 * @since 1.1
 */
typedef struct _Evas_Video_Surface Evas_Video_Surface;

/**
 * @def EVAS_VIDEO_SURFACE_VERSION
 * Magic version number to know what the video surf struct looks like
 * @since 1.1
 */
#define EVAS_VIDEO_SURFACE_VERSION 1

typedef void (*Evas_Video_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface);  /**< Evas video callback function signature */
typedef void (*Evas_Video_Coord_Cb)(void *data, Evas_Object *obj, const Evas_Video_Surface *surface, Evas_Coord a, Evas_Coord b);  /**< Evas video coordinates callback function signature */

struct _Evas_Video_Surface
{
   int                 version; /**< The Evas Video surface version in use @see EVAS_VIDEO_SURFACE_VERSION*/

   Evas_Video_Coord_Cb move; /**< Move the video surface to this position */
   Evas_Video_Coord_Cb resize; /**< Resize the video surface to that size */
   Evas_Video_Cb       show; /**< Show the video overlay surface */
   Evas_Video_Cb       hide; /**< Hide the video overlay surface */
   Evas_Video_Cb       update_pixels; /**< Please update the Evas_Object_Image pixels when called */

   Evas_Object        *parent; /**< The parent object */
   void               *data;
};

/**
 * Enum values for the Video surface capabilities
 * @see evas_object_image_video_surface_caps_get()
 * @see evas_object_image_video_surface_caps_set()
 */

typedef enum _Evas_Video_Surface_Caps
{
   EVAS_VIDEO_SURFACE_MOVE = 1,   /**< Move capability */
   EVAS_VIDEO_SURFACE_RESIZE = 2,   /**< Resize capability */
   EVAS_VIDEO_SURFACE_CLIP = 4,   /**< Clip capability */
   EVAS_VIDEO_SURFACE_BELOW = 8,   /**< Below capability */
   EVAS_VIDEO_SURFACE_STACKING_CHECK = 16,   /**< Stacking capability */
   EVAS_VIDEO_SURFACE_IGNORE_WINDOW = 32,   /**< Ignore window capability */
} Evas_Video_Surface_Caps;

/**
 * @brief Set the video surface linked to a given image of the canvas.
 *
 * @param[in] surf The new video surface.
 *
 * @since 1.1
 */
EAPI void evas_object_image_video_surface_set(Evas_Object *obj, Evas_Video_Surface *surf);

/**
 * @brief Get the video surface linekd to a given image of the canvas.
 *
 * @return The new video surface.
 *
 * @since 1.1
 */
EAPI const Evas_Video_Surface *evas_object_image_video_surface_get(const Evas_Object *obj);

/**
 * @brief Set the video surface capabilities to a given image of the canvas.
 *
 * @param[in] caps
 */
EAPI void evas_object_image_video_surface_caps_set(Evas_Object *obj, unsigned int caps);

/** Get the video surface capabilities to a given image of the canvas.
 */
EAPI unsigned int evas_object_image_video_surface_caps_get(const Evas_Object *obj);


/*
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
 * @return data A newly allocated data in the format specified by to_cspace.
 *
 * @param[in] to_cspace The colorspace to which the image raw data will be converted.
 */
/** @deprecated evas_object_image_data_convert */
EAPI void *evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_DEPRECATED;

/*
 * Import pixels from given source to a given canvas image object.
 *
 * This function imports pixels from a given source to a given canvas image.
 *
 * @param[in] pixels The pixel's source to be imported.
 */
/** @deprecated evas_object_image_pixels_import */
EAPI Eina_Bool evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(2) EINA_DEPRECATED;

/*
 * Reload an image object's image data.
 *
 * This function reloads the image data bound to image object @p obj.
 */
/** @deprecated evas_object_image_reload */
EAPI void evas_object_image_reload(Evas_Object *obj) EINA_DEPRECATED;

/**
 * @deprecated This function has never been implemented. Please use
 *             evas_object_clip_set() with an alpha or RGBA image instead
 *             of setting this flag.
 */
EAPI void evas_object_image_alpha_mask_set(Evas_Object *obj, Eina_Bool ismask) EINA_ARG_NONNULL(1) EINA_DEPRECATED;

#include "canvas/evas_image.eo.legacy.h"

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

/**
 *
 * Sets the text string to be displayed by the given text object.
 *
 * @see evas_object_text_text_get()
 *
 * @param[in] text Text string to display on it.
 */
EAPI void evas_object_text_text_set(Eo *obj, const char *text);

/**
 *
 * Retrieves the text string currently being displayed by the given
 * text object.
 *
 * @return The text string currently being displayed on it.
 *
 * @note Do not free() the return value.
 *
 * @see evas_object_text_text_set()
 *
 */
EAPI const char *evas_object_text_text_get(const Eo *obj);

#include "canvas/evas_text.eo.legacy.h"

/**
 *
 * Set the font (source) file to be used on a given text object.
 *
 * This function allows the font file to be explicitly set for a given
 * text object, overriding system lookup, which will first occur in
 * the given file's contents.
 *
 * @see evas_object_text_font_get()
 *
 * @param[in] font_source The font file's path.
 */
EAPI void evas_object_text_font_source_set(Eo *obj, const char *font_source);

/**
 *
 * Get the font file's path which is being used on a given text
 * object.
 *
 * @return The font file's path.
 *
 * @see evas_object_text_font_get() for more details
 *
 */
EAPI const char *evas_object_text_font_source_get(const Eo *obj);

/**
 *
 * Set the font family or filename, and size on a given text object.
 *
 * This function allows the font name and size of a text object to be
 * set. The @p font string has to follow fontconfig's convention on
 * naming fonts, as it's the underlying library used to query system
 * fonts by Evas (see the @c fc-list command's output, on your system,
 * to get an idea). Alternatively, one can use a full path to a font file.
 *
 * @see evas_object_text_font_get()
 * @see evas_object_text_font_source_set()
 *
 * @param[in] font The font family name or filename.
 * @param[in] size The font size, in points.
 */
EAPI void evas_object_text_font_set(Eo *obj, const char *font, Evas_Font_Size size);

/**
 *
 * Retrieve the font family and size in use on a given text object.
 *
 * This function allows the font name and size of a text object to be
 * queried. Be aware that the font name string is still owned by Evas
 * and should @b not have free() called on it by the caller of the
 * function.
 *
 * @see evas_object_text_font_set()
 *
 * @param[out] font The font family name or filename.
 * @param[out] size The font size, in points.
 */
EAPI void evas_object_text_font_get(const Eo *obj, const char **font, Evas_Font_Size *size);


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

/**
 * Clear the textblock object.
 * @note Does *NOT* free the Evas object itself.
 *
 * @param obj the object to clear.
 * @return nothing.
 */
EAPI void                                     evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

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
 *
 * @brief Set the font (source) file to be used on a given textgrid object.
 *
 * This function allows the font file @p font_source to be explicitly
 * set for the textgrid object @p obj, overriding system lookup, which
 * will first occur in the given file's contents. If @p font_source is
 * @c NULL or is an empty string, or the same font_source has already
 * been set, or on error, this function does nothing.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 *
 * @param[in] font_source The font file's path.
 */
EAPI void evas_object_textgrid_font_source_set(Eo *obj, const char *font_source);

/**
 *
 * @brief Get the font file's path which is being used on a given textgrid object.
 *
 * @return The font file's path.
 *
 * This function returns the font source path of the textgrid object
 * @p obj. If the font source path has not been set, or on error,
 * @c NULL is returned.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_set()
 *
 * @since 1.7
 *
 */
EAPI const char *evas_object_textgrid_font_source_get(const Eo *obj);

/**
 *
 * @brief Set the font family and size on a given textgrid object.
 *
 * This function allows the font name @p font_name and size
 * @p font_size of the textgrid object @p obj to be set. The @p font_name
 * string has to follow fontconfig's convention on naming fonts, as
 * it's the underlying library used to query system fonts by Evas (see
 * the @c fc-list command's output, on your system, to get an
 * idea). It also has to be a monospace font. If @p font_name is
 * @c NULL, or if it is an empty string, or if @p font_size is less or
 * equal than 0, or on error, this function does nothing.
 *
 * @see evas_object_textgrid_font_get()
 * @see evas_object_textgrid_font_source_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 *
 * @param[in] font_name The font (family) name.
 * @param[in] font_size The font size, in points.
 */
EAPI void evas_object_textgrid_font_set(Eo *obj, const char *font_name, Evas_Font_Size font_size);

/**
 *
 * @brief Retrieve the font family and size in use on a given textgrid object.
 *
 * This function allows the font name and size of a textgrid object
 * @p obj to be queried and stored respectively in the buffers
 * @p font_name and @p font_size. Be aware that the font name string is
 * still owned by Evas and should @b not have free() called on it by
 * the caller of the function. On error, the font name is the empty
 * string and the font size is 0. @p font_name and @p font_source can
 * be @c NULL.
 *
 * @see evas_object_textgrid_font_set()
 * @see evas_object_textgrid_font_source_set()
 * @see evas_object_textgrid_font_source_get()
 *
 * @since 1.7
 *
 * @param[out] font_name The font (family) name.
 * @param[out] font_size The font size, in points.
 */
EAPI void evas_object_textgrid_font_get(const Eo *obj, const char **font_name, Evas_Font_Size *font_size);

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

/**
 * @brief Adds the given point to the given evas polygon object.
 *
 * @param[in] y The Y coordinate of the given point.
 *
 * @ingroup Evas_Polygon
 */
EAPI void evas_object_polygon_point_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/** Removes all of the points from the given evas polygon object.
 *
 * @ingroup Evas_Polygon
 */
EAPI void evas_object_polygon_points_clear(Evas_Object *obj);

/**
 * @}
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

#include "canvas/evas_object_smart.eo.legacy.h"

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

#include "canvas/evas_box.eo.legacy.h"

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

#include "canvas/evas_grid.eo.legacy.h"

/**
 * @}
 */

/**
 *
 * Adds an output to the canvas
 *
 * @parem e The canvas to add the output to
 * @return The output
 *
 * @see evas_out_engine_info_set
 * @see evas_output_viewport_set
 * @see evas_output_size_set
 *
 * @since 1.8
 */
EAPI Evas_Out *evas_out_add(Evas *e);

/**
 *
 * Deletes an output
 *
 * @parem evo The output object
 *
 * @see evas_out_add
 *
 * @since 1.8
 *
 */
EAPI void evas_output_del(Evas_Out *evo);

#include "canvas/evas_out.eo.legacy.h"
