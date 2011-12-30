/**
 * @defgroup Cursors Cursors
 *
 * The Elementary cursor is an internal smart object used to
 * customize the mouse cursor displayed over objects (or
 * widgets). In the most common scenario, the cursor decoration
 * comes from the graphical @b engine Elementary is running
 * on. Those engines may provide different decorations for cursors,
 * and Elementary provides functions to choose them (think of X11
 * cursors, as an example).
 *
 * There's also the possibility of, besides using engine provided
 * cursors, also use ones coming from Edje theming files. Both
 * globally and per widget, Elementary makes it possible for one to
 * make the cursors lookup to be held on engines only or on
 * Elementary's theme file, too. To set cursor's hot spot,
 * two data items should be added to cursor's theme: "hot_x" and
 * "hot_y", that are the offset from upper-left corner of the cursor
 * (coordinates 0,0).
 *
 * @{
 */

/**
 * Set the cursor to be shown when mouse is over the object
 *
 * Set the cursor that will be displayed when mouse is over the
 * object. The object can have only one cursor set to it, so if
 * this function is called twice for an object, the previous set
 * will be unset.
 * If using X cursors, a definition of all the valid cursor names
 * is listed on Elementary_Cursors.h. If an invalid name is set
 * the default cursor will be used.
 *
 * @param obj the object being set a cursor.
 * @param cursor the cursor name to be used.
 *
 * @ingroup Cursors
 */
EAPI void        elm_object_cursor_set(Evas_Object *obj, const char *cursor);

/**
 * Get the cursor to be shown when mouse is over the object
 *
 * @param obj an object with cursor already set.
 * @return the cursor name.
 *
 * @ingroup Cursors
 */
EAPI const char *elm_object_cursor_get(const Evas_Object *obj);

/**
 * Unset cursor for object
 *
 * Unset cursor for object, and set the cursor to default if the mouse
 * was over this object.
 *
 * @param obj Target object
 * @see elm_object_cursor_set()
 *
 * @ingroup Cursors
 */
EAPI void        elm_object_cursor_unset(Evas_Object *obj);

/**
 * Sets a different style for this object cursor.
 *
 * @note before you set a style you should define a cursor with
 *       elm_object_cursor_set()
 *
 * @param obj an object with cursor already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Cursors
 */
EAPI void        elm_object_cursor_style_set(Evas_Object *obj, const char *style);

/**
 * Get the style for this object cursor.
 *
 * @param obj an object with cursor already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then NULL is returned.
 *
 * @ingroup Cursors
 */
EAPI const char *elm_object_cursor_style_get(const Evas_Object *obj);

/**
 * Set if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a cursor
 * with elm_object_cursor_set(). By default it will only look for cursors
 * provided by the engine.
 *
 * @param obj an object with cursor already set.
 * @param engine_only boolean to define it cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well.
 *
 * @ingroup Cursors
 */
EAPI void        elm_object_cursor_engine_only_set(Evas_Object *obj, Eina_Bool engine_only);

/**
 * Get the cursor engine only usage for this object cursor.
 *
 * @param obj an object with cursor already set.
 * @return engine_only boolean to define it cursors should be
 * looked only between the provided by the engine or searched on
 * widget's theme as well. If the object does not have a cursor
 * set, then EINA_FALSE is returned.
 *
 * @ingroup Cursors
 */
EAPI Eina_Bool   elm_object_cursor_engine_only_get(const Evas_Object *obj);

/**
 * Get the configured cursor engine only usage
 *
 * This gets the globally configured exclusive usage of engine cursors.
 *
 * @return 1 if only engine cursors should be used
 * @ingroup Cursors
 */
EAPI int         elm_cursor_engine_only_get(void);

/**
 * Set the configured cursor engine only usage
 *
 * This sets the globally configured exclusive usage of engine cursors.
 * It won't affect cursors set before changing this value.
 *
 * @param engine_only If 1 only engine cursors will be enabled, if 0 will
 * look for them on theme before.
 * @return EINA_TRUE if value is valid and setted (0 or 1)
 * @ingroup Cursors
 */
EAPI Eina_Bool   elm_cursor_engine_only_set(int engine_only);

/**
 * @}
 */
