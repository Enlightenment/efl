/**
 * Add a new layout to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_layout_file_set()
 * @see elm_layout_theme_set()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_add(Evas_Object *parent);

/**
 * Set the file that will be used as layout
 *
 * @param obj The layout object
 * @param file The path to file (edj) that will be used as layout
 * @param group The group that the layout belongs in edje file
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Freezes the Elementary layout object.
 *
 * @param obj A handle to an Elementary layout object.
 * @return The frozen state or 0 on Error
 *
 * This function puts all changes on hold. Successive freezes will
 * nest, requiring an equal number of thaws.
 *
 * @see elm_layout_thaw()
 */
EAPI int elm_layout_freeze(Evas_Object *obj);

/**
 * @brief Thaws the Elementary object.
 *
 * @param obj A handle to an Elementary layout object.
 * @return The frozen state or 0 if the object is not frozen or on error.
 *
 * This function thaws the given Edje object and the Elementary sizing calc.
 *
 * @note: If sucessives freezes were done, an equal number of
 *        thaws will be required.
 *
 * @see elm_layout_freeze()
 */
EAPI int elm_layout_thaw(Evas_Object *obj);

/**
 * Set the edje group from the elementary theme that will be used as layout
 *
 * @param obj The layout object
 * @param clas the class of the group
 * @param group the group
 * @param style the style to used
 *
 * @return (1 = success, 0 = error)
 *
 * Note that @a style will be the new style of @a obj too, as in an
 * elm_object_style_set() call.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_theme_set(Evas_Object *obj, const char *clas, const char *group, const char *style);

/**
 * Send a (Edje) signal to a given layout widget's underlying Edje
 * object.
 *
 * @param obj The layout object handle
 * @param emission The signal's name string
 * @param source The signal's source string
 *
 * This function sends a signal to the underlying Edje object of @a
 * obj. An Edje program on that Edje object's definition can respond
 * to a signal by specifying matching 'signal' and 'source' fields.
 *
 * @ingroup Layout
 */
EAPI void elm_layout_signal_emit(Evas_Object *obj, const char *emission, const char *source);

/**
 * Add a callback for a (Edje) signal emitted by a layout widget's
 * underlying Edje object.
 *
 * @param obj The layout object handle
 * @param emission The signal's name string
 * @param source The signal's source string
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass in to the callback function.
 *
 * This function connects a callback function to a signal emitted by
 * the underlying Edje object of @a obj. Globs are accepted in either
 * the emission or source strings (see @c
 * edje_object_signal_callback_add()).
 *
 * @ingroup Layout
 */
EAPI void elm_layout_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * Remove a signal-triggered callback from a given layout widget.
 *
 * @param obj The layout object handle
 * @param emission The signal's name string
 * @param source The signal's source string
 * @param func The callback function being executed when the signal
 * was emitted.
 * @return The data pointer of the signal callback (passed on
 * elm_layout_signal_callback_add()) or @c NULL, on errors.
 *
 * This function removes the @b last callback attached to a signal
 * emitted by the undelying Edje object of @a obj, with parameters @a
 * emission, @a source and @c func matching exactly those passed to a
 * previous call to elm_layout_signal_callback_add(). The data pointer
 * that was passed to this call will be returned.
 *
 * @ingroup Layout
 */
EAPI void *elm_layout_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * Append child to layout box part.
 *
 * @param obj the layout object
 * @param part the box part to which the object will be appended.
 * @param child the child object to append to box.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is appended, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_prepend()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_insert_at()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_append(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * Prepend child to layout box part.
 *
 * @param obj the layout object
 * @param part the box part to prepend.
 * @param child the child object to prepend to box.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is prepended, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_insert_at()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * Insert child to layout box part before a reference object.
 *
 * @param obj the layout object
 * @param part the box part to insert.
 * @param child the child object to insert into box.
 * @param reference another reference object to insert before in box.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is inserted, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_prepend()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * Insert child to layout box part at a given position.
 *
 * @param obj the layout object
 * @param part the box part to insert.
 * @param child the child object to insert into box.
 * @param pos the numeric position >=0 to insert the child.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is inserted, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_prepend()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * Remove a child of the given part box.
 *
 * @param obj The layout object
 * @param part The box part name to remove child.
 * @param child The object to remove from box.
 * @return The object that was being used, or NULL if not found.
 *
 * The object will be removed from the box part and its lifetime will
 * not be handled by the layout anymore. This is equivalent to
 * elm_layout_content_unset() for box.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_remove_all()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_box_remove(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * Remove all children of the given part box.
 *
 * @param obj The layout object
 * @param part The box part name to remove child.
 * @param clear If EINA_TRUE, then all objects will be deleted as
 *        well, otherwise they will just be removed and will be
 *        dangling on the canvas.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * The objects will be removed from the box part and their lifetime will
 * not be handled by the layout anymore. This is equivalent to
 * elm_layout_box_remove() for all box children.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * Insert child to layout table part.
 *
 * @param obj the layout object
 * @param part the box part to pack child.
 * @param child_obj the child object to pack into table.
 * @param col the column to which the child should be added. (>= 0)
 * @param row the row to which the child should be added. (>= 0)
 * @param colspan how many columns should be used to store this object. (>=
 *        1)
 * @param rowspan how many rows should be used to store this object. (>= 1)
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is inserted, it will become child of the table. Its
 * lifetime will be bound to the layout, and whenever the layout dies the
 * child will be deleted automatically. One should use
 * elm_layout_table_unpack() to make this layout forget about the object.
 *
 * If @p colspan or @p rowspan are bigger than 1, that object will occupy
 * more space than a single cell. For instance, the following code:
 * @code
 * elm_layout_table_pack(layout, "table_part", child, 0, 1, 3, 1);
 * @endcode
 *
 * Would result in an object being added like the following picture:
 *
 * @image html layout_colspan.png
 * @image latex layout_colspan.eps width=\textwidth
 *
 * @see elm_layout_table_unpack()
 * @see elm_layout_table_clear()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_table_pack(Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * Unpack (remove) a child of the given part table.
 *
 * @param obj The layout object
 * @param part The table part name to remove child.
 * @param child_obj The object to remove from table.
 * @return The object that was being used, or NULL if not found.
 *
 * The object will be unpacked from the table part and its lifetime
 * will not be handled by the layout anymore. This is equivalent to
 * elm_layout_content_unset() for table.
 *
 * @see elm_layout_table_pack()
 * @see elm_layout_table_clear()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * Remove all the child objects of the given part table.
 *
 * @param obj The layout object
 * @param part The table part name to remove child.
 * @param clear If EINA_TRUE, then all objects will be deleted as
 *        well, otherwise they will just be removed and will be
 *        dangling on the canvas.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * The objects will be removed from the table part and their lifetime will
 * not be handled by the layout anymore. This is equivalent to
 * elm_layout_table_unpack() for all table children.
 *
 * @see elm_layout_table_pack()
 * @see elm_layout_table_unpack()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * Get the edje layout
 *
 * @param obj The layout object
 *
 * @return A Evas_Object with the edje layout settings loaded
 * with function elm_layout_file_set
 *
 * This returns the edje object. It is not expected to be used to then
 * swallow objects via edje_object_part_swallow() for example. Use
 * elm_layout_content_set() instead so child object handling and sizing is
 * done properly.
 *
 * @note This function should only be used if you really need to call some
 * low level Edje function on this edje object. All the common stuff (setting
 * text, emitting signals, hooking callbacks to signals, etc.) can be done
 * with proper elementary functions.
 *
 * @see elm_layout_signal_callback_add()
 * @see elm_layout_signal_emit()
 * @see elm_layout_text_set()
 * @see elm_layout_content_set()
 * @see elm_layout_box_append()
 * @see elm_layout_table_pack()
 * @see elm_layout_data_get()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_edje_get(const Evas_Object *obj);

/**
 * Get the edje data from the given layout
 *
 * @param obj The layout object
 * @param key The data key
 *
 * @return The edje data string
 *
 * This function fetches data specified inside the edje theme of this layout.
 * This function return NULL if data is not found.
 *
 * In EDC this comes from a data block within the group block that @p
 * obj was loaded from. E.g.
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     data {
 *       item: "key1" "value1";
 *       item: "key2" "value2";
 *     }
 *   }
 * }
 * @endcode
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_data_get(const Evas_Object *obj, const char *key);

/**
 * Eval sizing
 *
 * @param obj The layout object
 *
 * Manually forces a sizing re-evaluation. This is useful when the minimum
 * size required by the edje theme of this layout has changed. The change on
 * the minimum size required by the edje theme is not immediately reported to
 * the elementary layout, so one needs to call this function in order to tell
 * the widget (layout) that it needs to reevaluate its own size.
 *
 * The minimum size of the theme is calculated based on minimum size of
 * parts, the size of elements inside containers like box and table, etc. All
 * of this can change due to state changes, and that's when this function
 * should be called.
 *
 * Also note that a standard signal of "size,eval" "elm" emitted from the
 * edje object will cause this to happen too.
 *
 * @ingroup Layout
 */
EAPI void                         elm_layout_sizing_eval(Evas_Object *obj);

/**
 * Request sizing reevaluation, restricted to current width and/or height
 *
 * Useful mostly when there are TEXTBLOCK parts defining the height of the
 * object and nothing else restricting it to a minimum width. Calling this
 * function will restrict the minimum size in the Edje calculation to whatever
 * size it the layout has at the moment.
 *
 * @param obj The layout object
 * @param w   Restrict minimum size to the current width
 * @param h   Restrict minimum size ot the current height
 *
 * @since 1.8
 *
 * @ingroup Layout
 */
EAPI void                         elm_layout_sizing_restricted_eval(Evas_Object *obj, Eina_Bool w, Eina_Bool h);

/**
 * Sets a specific cursor for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @param cursor cursor name to use, see Elementary_Cursor.h
 *
 * @return EINA_TRUE on success or EINA_FALSE on failure, that may be
 *         part not exists or it has "mouse_events: 0".
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_set(Evas_Object *obj, const char *part_name, const char *cursor);

/**
 * Get the cursor to be shown when mouse is over an edje part
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @return the cursor name.
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_part_cursor_get(const Evas_Object *obj, const char *part_name);

/**
 * Unsets a cursor previously set with elm_layout_part_cursor_set().
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group, that had a cursor set
 *        with elm_layout_part_cursor_set().
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_unset(Evas_Object *obj, const char *part_name);

/**
 * Sets a specific cursor style for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @return EINA_TRUE on success or EINA_FALSE on failure, that may be
 *         part not exists or it did not had a cursor set.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_style_set(Evas_Object *obj, const char *part_name, const char *style);

/**
 * Get a specific cursor style for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 *
 * @return the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then NULL is returned.
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_part_cursor_style_get(const Evas_Object *obj, const char *part_name);

/**
 * Sets if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a
 * cursor with elm_layout_part_cursor_set(). By default it will only
 * look for cursors provided by the engine.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @param engine_only if cursors should be just provided by the engine (EINA_TRUE)
 *        or should also search on widget's theme as well (EINA_FALSE)
 *
 * @return EINA_TRUE on success or EINA_FALSE on failure, that may be
 *         part not exists or it did not had a cursor set.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_engine_only_set(Evas_Object *obj, const char *part_name, Eina_Bool engine_only);

/*
* Set accessibility to all texblock(text) parts in the layout object
*
* @param obj The layout object.
* @param can_access makes all textblock(text) parts in the layout @p obj possible
* to have accessibility. @c EINA_TRUE means textblock(text) parts can be accessible
*
* @return @c EINA_TRUE on success or @c EINA_FALSE on failure. If @p obj is not
* a proper layout object, @c EINA_FALSE is returned.
*
* @since 1.7
*
* @ingroup Layout
*/
EAPI Eina_Bool                    elm_layout_edje_object_can_access_set(Evas_Object *obj, Eina_Bool can_access);

/*
* Get accessibility state of texblock(text) parts in the layout object
*
* @param obj The layout object.
*
* @return @c EINA_TRUE, if all textblock(text) parts in the layout can be accessible
* @c EINA_FALSET if those cannot be accessible. If @p obj is not a proper layout
* object, @c EINA_FALSE is returned.
*
* @see elm_layout_edje_object_access_set()
*
* @since 1.7
*
* @ingroup Layout
*/
EAPI Eina_Bool                    elm_layout_edje_object_can_access_get(Evas_Object *obj);

/**
 * Get a specific cursor engine_only for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 *
 * @return whenever the cursor is just provided by engine or also from theme.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_engine_only_get(const Evas_Object *obj, const char *part_name);

/**
 * Set the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @param content The child that will be added in this layout object
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_layout_content_unset() function.
 *
 * @note In an Edje theme, the part used as a content container is called
 * @c SWALLOW. This is why the parameter name is called @p swallow, but it is
 * expected to be a part name just like the second parameter of
 * elm_layout_box_append().
 *
 * @see elm_layout_box_append()
 * @see elm_layout_content_get()
 * @see elm_layout_content_unset()
 *
 * @see @ref secBox
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);

/**
 * Get the child object in the given content part.
 *
 * @param obj The layout object
 * @param swallow The SWALLOW part to get its content
 *
 * @return The swallowed object or NULL if none or an error occurred
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_content_get(const Evas_Object *obj, const char *swallow);

/**
 * Unset the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this part.
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_content_unset(Evas_Object *obj, const char *swallow);

/**
 * Set the text of the given part
 *
 * @param obj The layout object
 * @param part The TEXT part where to set the text
 * @param text The text to set
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_text_set(Evas_Object *obj, const char *part, const char *text);

/**
 * Get the text set in the given part
 *
 * @param obj The layout object
 * @param part The TEXT part to retrieve the text off
 *
 * @return The text set in @p part
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_text_get(const Evas_Object *obj, const char *part);

/**
 * Get the list of swallow parts of a given container widget
 *
 * @param obj The layout object
 *
 * @return list of swallow parts which should be freed by the user program with elm_list_free()
 *
 * @since 1.9
 *
 * @ingroup Layout
 */
EAPI Eina_List                   *elm_layout_content_swallow_list_get(const Evas_Object *obj);
