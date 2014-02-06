/**
 * @defgroup Cursors Cursors
 * @ingroup Elementary
 *
 * The Elementary cursor is an internal smart object used to
 * customize the mouse cursor displayed over objects (or
 * widgets). In the most common scenario, the cursor decoration
 * comes from the graphical @b engine Elementary is running
 * on. Those engines may provide different decorations for cursors,
 * and Elementary provides functions to choose them (think of X11
 * cursors, as an example).
 *
 * By default, Elementary searches cursors only from engine.
 * There's also the possibility of, besides using engine provided
 * cursors, also use the ones coming from Edje theme files. Both
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
 * @note before you set theme_search you should define a cursor with
 * elm_object_cursor_set(). By default it will only look for cursors
 * provided by the engine.
 *
 * @param obj an object with cursor already set.
 * @param theme_search boolean to define if cursors should be searched
 * on widget's theme.
 *
 * @ingroup Cursors
 */
EAPI void elm_object_cursor_theme_search_enabled_set(Evas_Object *obj, Eina_Bool theme_search);

/**
 * Get if the cursor set should be searched on the theme for this object cursor.
 *
 * @param obj an object with cursor already set.
 * @return @c EINA_TRUE if the cursor set should be searched on widget's theme,
 * EINA_FALSE otherwise.
 *
 * @ingroup Cursors
 */
EAPI Eina_Bool elm_object_cursor_theme_search_enabled_get(const Evas_Object *obj);

/**
 * @}
 */
