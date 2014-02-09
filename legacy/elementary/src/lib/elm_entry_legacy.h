/**
 * This adds an entry to @p parent object.
 *
 * By default, entries are:
 * @li not scrolled
 * @li multi-line
 * @li word wrapped
 * @li autosave is enabled
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Entry
 */
EAPI Evas_Object       *elm_entry_add(Evas_Object *parent);

/**
 * This sets the text displayed within the entry to @p entry.
 *
 * @param obj The entry object
 * @param entry The text to be displayed
 *
 * @note Using this function bypasses text filters
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_set(Evas_Object *obj, const char *entry);

/**
 * This returns the text currently shown in object @p entry.
 * See also elm_entry_entry_set().
 *
 * @param obj The entry object
 * @return The currently displayed text or NULL on failure
 *
 * @ingroup Entry
 */
EAPI const char        *elm_entry_entry_get(const Evas_Object *obj);

/**
 * Push the style to the top of user style stack.
 * If there is styles in the user style stack, the properties in the top style
 * of user style stack will replace the properties in current theme.
 * The input style is specified in format tag='property=value' (i.e. DEFAULT='font=Sans font_size=60'hilight=' + font_weight=Bold').
 *
 * @param obj The entry object
 * @param style The style user to push
 *
 * @ingroup Entry
 * @since 1.7
 */
EAPI void      elm_entry_text_style_user_push(Evas_Object *obj, const char *style);

/**
 * Remove the style in the top of user style stack.
 *
 * @param  obj The entry object
 *
 * @see elm_entry_text_style_user_push()
 *
 * @ingroup Entry
 * @since 1.7
 */
EAPI void     elm_entry_text_style_user_pop(Evas_Object *obj);

/**
 * Retrieve the style on the top of user style stack.
 *
 * @param obj The entry object
 * @return style on the top of user style stack if exist, otherwise NULL.
 *
 * @see elm_entry_text_style_user_push()
 *
 * @ingroup Entry
 * @since 1.7
 */
EAPI const char*      elm_entry_text_style_user_peek(const Evas_Object *obj);

/**
 * Sets the entry to single line mode.
 *
 * In single line mode, entries don't ever wrap when the text reaches the
 * edge, and instead they keep growing horizontally. Pressing the @c Enter
 * key will generate an @c "activate" event instead of adding a new line.
 *
 * When @p single_line is @c EINA_FALSE, line wrapping takes effect again
 * and pressing enter will break the text into a different line
 * without generating any events.
 *
 * @param obj The entry object
 * @param single_line If true, the text in the entry
 * will be on a single line.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);

/**
 * Get whether the entry is set to be single line.
 *
 * @param obj The entry object
 * @return single_line If true, the text in the entry is set to display
 * on a single line.
 *
 * @see elm_entry_single_line_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_single_line_get(const Evas_Object *obj);

/**
 * Sets the entry to password mode.
 *
 * In password mode, entries are implicitly single line and the display of
 * any text in them is replaced with asterisks (*).
 *
 * @param obj The entry object
 * @param password If true, password mode is enabled.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_password_set(Evas_Object *obj, Eina_Bool password);

/**
 * Get whether the entry is set to password mode.
 *
 * @param obj The entry object
 * @return If true, the entry is set to display all characters
 * as asterisks (*).
 *
 * @see elm_entry_password_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_password_get(const Evas_Object *obj);

/**
 * Appends @p entry to the text of the entry.
 *
 * Adds the text in @p entry to the end of any text already present in the
 * widget.
 *
 * The appended text is subject to any filters set for the widget.
 *
 * @param obj The entry object
 * @param entry The text to be displayed
 *
 * @see elm_entry_markup_filter_append()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_append(Evas_Object *obj, const char *entry);

/**
 * Get whether the entry is empty.
 *
 * Empty means no text at all. If there are any markup tags, like an item
 * tag for which no provider finds anything, and no text is displayed, this
 * function still returns EINA_FALSE.
 *
 * @param obj The entry object
 * @return EINA_TRUE if the entry is empty, EINA_FALSE otherwise.
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_is_empty(const Evas_Object *obj);

/**
 * Returns the actual textblock object of the entry.
 *
 * This function exposes the internal textblock object that actually
 * contains and draws the text. This should be used for low-level
 * manipulations that are otherwise not possible.
 *
 * Changing the textblock directly from here will not notify edje/elm to
 * recalculate the textblock size automatically, so any modifications
 * done to the textblock returned by this function should be followed by
 * a call to elm_entry_calc_force().
 *
 * The return value is marked as const as an additional warning.
 * One should not use the returned object with any of the generic evas
 * functions (geometry_get/resize/move and etc), but only with the textblock
 * functions; The former will either not work at all, or break the correct
 * functionality.
 *
 * IMPORTANT: Many functions may change (i.e delete and create a new one)
 * the internal textblock object. Do NOT cache the returned object, and try
 * not to mix calls on this object with regular elm_entry calls (which may
 * change the internal textblock object). This applies to all cursors
 * returned from textblock calls, and all the other derivative values.
 *
 * @param obj The entry object
 * @return The textblock object.
 *
 * @ingroup Entry
 */
EAPI Evas_Object *      elm_entry_textblock_get(Evas_Object *obj);

/**
 * Forces calculation of the entry size and text layouting.
 *
 * This should be used after modifying the textblock object directly. See
 * elm_entry_textblock_get() for more information.
 *
 * @param obj The entry object
 *
 * @see elm_entry_textblock_get()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_calc_force(Evas_Object *obj);

/**
 * Get any selected text within the entry.
 *
 * If there's any selected text in the entry, this function returns it as
 * a string in markup format. NULL is returned if no selection exists or
 * if an error occurred.
 *
 * The returned value points to an internal string and should not be freed
 * or modified in any way. If the @p entry object is deleted or its
 * contents are changed, the returned pointer should be considered invalid.
 *
 * @param obj The entry object
 * @return The selected text within the entry or NULL on failure
 *
 * @ingroup Entry
 */
EAPI const char        *elm_entry_selection_get(const Evas_Object *obj);

/**
  * This disabled the entry's selection handlers.
  *
  * @param obj The entry object
  * @param disabled If true, the selection handlers are disabled.
  *
  * @ingroup Entry
  */
EAPI void              elm_entry_selection_handler_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
  * This returns whether the entry's selection handlers are disabled.
  *
  * @param obj The entry object
  * @return If true, the selection handlers are disabled.
  *
  * @ingroup Entry
  */
EAPI void              elm_entry_selection_handler_disabled_set(Evas_Object *obj, Eina_Bool disabled);


/**
 * Inserts the given text into the entry at the current cursor position.
 *
 * This inserts text at the cursor position as if it was typed
 * by the user (note that this also allows markup which a user
 * can't just "type" as it would be converted to escaped text, so this
 * call can be used to insert things like emoticon items or bold push/pop
 * tags, other font and color change tags etc.)
 *
 * If any selection exists, it will be replaced by the inserted text.
 *
 * The inserted text is subject to any filters set for the widget.
 *
 * @param obj The entry object
 * @param entry The text to insert
 *
 * @see elm_entry_markup_filter_append()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_entry_insert(Evas_Object *obj, const char *entry);

/**
 * Set the line wrap type to use on multi-line entries.
 *
 * Sets the wrap type used by the entry to any of the specified in
 * Elm_Wrap_Type. This tells how the text will be implicitly cut into a new
 * line (without inserting a line break or paragraph separator) when it
 * reaches the far edge of the widget.
 *
 * Note that this only makes sense for multi-line entries. A widget set
 * to be single line will never wrap.
 *
 * @param obj The entry object
 * @param wrap The wrap mode to use. See Elm_Wrap_Type for details on them
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap);

/**
 * Get the wrap mode the entry was set to use.
 *
 * @param obj The entry object
 * @return Wrap type
 *
 * @see also elm_entry_line_wrap_set()
 *
 * @ingroup Entry
 */
EAPI Elm_Wrap_Type      elm_entry_line_wrap_get(const Evas_Object *obj);

/**
 * Sets if the entry is to be editable or not.
 *
 * By default, entries are editable and when focused, any text input by the
 * user will be inserted at the current cursor position. But calling this
 * function with @p editable as EINA_FALSE will prevent the user from
 * inputting text into the entry.
 *
 * The only way to change the text of a non-editable entry is to use
 * elm_object_text_set(), elm_entry_entry_insert() and other related
 * functions.
 *
 * @param obj The entry object
 * @param editable If EINA_TRUE, user input will be inserted in the entry,
 * if not, the entry is read-only and no user input is allowed.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_editable_set(Evas_Object *obj, Eina_Bool editable);

/**
 * Get whether the entry is editable or not.
 *
 * @param obj The entry object
 * @return If true, the entry is editable by the user.
 * If false, it is not editable by the user
 *
 * @see elm_entry_editable_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_editable_get(const Evas_Object *obj);

/**
 * This drops any existing text selection within the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_select_none(Evas_Object *obj);

/**
 * This selects all text within the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_select_all(Evas_Object *obj);

/**
 * This selects a region of text within the entry.
 *
 * @param obj The entry object
 * @param start The starting position
 * @param end The end position
 *
 * @ingroup Entry
 * @since 1.9
 */
EAPI void               elm_entry_select_region_set(Evas_Object *obj, int start, int end);

/**
 * This function returns the geometry of the cursor.
 *
 * It's useful if you want to draw something on the cursor (or where it is),
 * or for example in the case of scrolled entry where you want to show the
 * cursor.
 *
 * @param obj The entry object
 * @param x returned geometry
 * @param y returned geometry
 * @param w returned geometry
 * @param h returned geometry
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * This moves the cursor one place to the right within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_next(Evas_Object *obj);

/**
 * This moves the cursor one place to the left within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_prev(Evas_Object *obj);

/**
 * This moves the cursor one line up within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_up(Evas_Object *obj);

/**
 * This moves the cursor one line down within the entry.
 *
 * @param obj The entry object
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_down(Evas_Object *obj);

/**
 * This moves the cursor to the beginning of the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_begin_set(Evas_Object *obj);

/**
 * This moves the cursor to the end of the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_end_set(Evas_Object *obj);

/**
 * This moves the cursor to the beginning of the current line.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_line_begin_set(Evas_Object *obj);

/**
 * This moves the cursor to the end of the current line.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_line_end_set(Evas_Object *obj);

/**
 * This begins a selection within the entry as though
 * the user were holding down the mouse button to make a selection.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_selection_begin(Evas_Object *obj);

/**
 * This ends a selection within the entry as though
 * the user had just released the mouse button while making a selection.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_selection_end(Evas_Object *obj);

/**
 * Get whether a format node exists at the current cursor position.
 *
 * A format node is anything that defines how the text is rendered. It can
 * be a visible format node, such as a line break or a paragraph separator,
 * or an invisible one, such as bold begin or end tag.
 * This function returns whether any format node exists at the current
 * cursor position.
 *
 * @param obj The entry object
 * @return EINA_TRUE if the current cursor position contains a format node,
 * EINA_FALSE otherwise.
 *
 * @see elm_entry_cursor_is_visible_format_get()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_is_format_get(const Evas_Object *obj);

/**
 * Get if the current cursor position holds a visible format node.
 *
 * @param obj The entry object
 * @return EINA_TRUE if the current cursor is a visible format, EINA_FALSE
 * if it's an invisible one or no format exists.
 *
 * @see elm_entry_cursor_is_format_get()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_is_visible_format_get(const Evas_Object *obj);

/**
 * Get the character pointed by the cursor at its current position.
 *
 * This function returns a string with the utf8 character stored at the
 * current cursor position.
 * Only the text is returned, any format that may exist will not be part
 * of the return value. You must free the string when done with free().
 *
 * @param obj The entry object
 * @return The text pointed by the cursors.
 *
 * @ingroup Entry
 */
EAPI char              *elm_entry_cursor_content_get(const Evas_Object *obj);

/**
 * Sets the cursor position in the entry to the given value
 *
 * The value in @p pos is the index of the character position within the
 * contents of the string as returned by elm_entry_cursor_pos_get().
 *
 * @param obj The entry object
 * @param pos The position of the cursor
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_cursor_pos_set(Evas_Object *obj, int pos);

/**
 * Retrieves the current position of the cursor in the entry
 *
 * @param obj The entry object
 * @return The cursor position
 *
 * @ingroup Entry
 */
EAPI int                elm_entry_cursor_pos_get(const Evas_Object *obj);

/**
 * This executes a "cut" action on the selected text in the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_selection_cut(Evas_Object *obj);

/**
 * This executes a "copy" action on the selected text in the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_selection_copy(Evas_Object *obj);

/**
 * This executes a "paste" action in the entry.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_selection_paste(Evas_Object *obj);

/**
 * This clears and frees the items in a entry's contextual (longpress)
 * menu.
 *
 * @param obj The entry object
 *
 * @see elm_entry_context_menu_item_add()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_clear(Evas_Object *obj);

/**
 * This adds an item to the entry's contextual menu.
 *
 * A longpress on an entry will make the contextual menu show up, if this
 * hasn't been disabled with elm_entry_context_menu_disabled_set().
 * By default, this menu provides a few options like enabling selection mode,
 * which is useful on embedded devices that need to be explicit about it,
 * and when a selection exists it also shows the copy and cut actions.
 *
 * With this function, developers can add other options to this menu to
 * perform any action they deem necessary.
 *
 * @param obj The entry object
 * @param label The item's text label
 * @param icon_file The item's icon file
 * @param icon_type The item's icon type
 * @param func The callback to execute when the item is clicked
 * @param data The data to associate with the item for related functions
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

/**
 * This disables the entry's contextual (longpress) menu.
 *
 * @param obj The entry object
 * @param disabled If true, the menu is disabled
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * This returns whether the entry's contextual (longpress) menu is
 * disabled.
 *
 * @param obj The entry object
 * @return If true, the menu is disabled
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_context_menu_disabled_get(const Evas_Object *obj);

/**
 * This appends a custom item provider to the list for that entry
 *
 * This appends the given callback. The list is walked from beginning to end
 * with each function called given the item href string in the text. If the
 * function returns an object handle other than NULL (it should create an
 * object to do this), then this object is used to replace that item. If
 * not the next provider is called until one provides an item object, or the
 * default provider in entry does.
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @see @ref entry-items
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_append(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * This prepends a custom item provider to the list for that entry
 *
 * This prepends the given callback. See elm_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_prepend(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * This removes a custom item provider to the list for that entry
 *
 * This removes the given callback. See elm_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_remove(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * Append a markup filter function for text inserted in the entry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the entry, with the text to be inserted
 * as a parameter. The type of given text is always markup.
 * The callback function is free to alter the text in any way it wants, but
 * it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set its
 * text parameter to NULL. This will also prevent any following filters from
 * being called.
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_append(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Prepend a markup filter function for text inserted in the entry
 *
 * Prepend the given callback to the list. See elm_entry_markup_filter_append()
 * for more information
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_prepend(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Remove a markup filter from the list
 *
 * Removes the given callback from the filter list. See
 * elm_entry_markup_filter_append() for more information.
 *
 * @param obj The entry object
 * @param func The filter function to remove
 * @param data The user data passed when adding the function
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_remove(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay if
 * the entry object is set to autosave (which is the default).
 *
 * If the entry had any other file set previously, any changes made to it
 * will be saved if the autosave feature is enabled, otherwise, the file
 * will be silently discarded and any non-saved changes will be lost.
 *
 * @param obj The entry object
 * @param file The path to the file to load and save
 * @param format The file format
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);

/**
 * Get the file being edited by the entry.
 *
 * This function can be used to retrieve any file set on the entry for
 * edition, along with the format used to load and save it.
 *
 * @param obj The entry object
 * @param file The path to the file to load and save
 * @param format The file format
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format);

/**
 * This function writes any changes made to the file set with
 * elm_entry_file_set()
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_file_save(Evas_Object *obj);

/**
 * This sets the entry object to 'autosave' the loaded text file or not.
 *
 * @param obj The entry object
 * @param autosave Autosave the loaded file or not
 *
 * @see elm_entry_file_set()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave);

/**
 * This gets the entry object's 'autosave' status.
 *
 * @param obj The entry object
 * @return Autosave the loaded file or not
 *
 * @see elm_entry_file_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_autosave_get(const Evas_Object *obj);

/**
 * Control pasting of text and images for the widget.
 *
 * Normally the entry allows both text and images to be pasted.
 * By setting cnp_mode to be #ELM_CNP_MODE_NO_IMAGE, this prevents images from being copy or past.
 * By setting cnp_mode to be #ELM_CNP_MODE_PLAINTEXT, this remove all tags in text .
 *
 * @note this only changes the behaviour of text.
 *
 * @param obj The entry object
 * @param cnp_mode One of #Elm_Cnp_Mode: #ELM_CNP_MODE_MARKUP, #ELM_CNP_MODE_NO_IMAGE, #ELM_CNP_MODE_PLAINTEXT.
 *
 * @ingroup Entry
 */
EAPI void         elm_entry_cnp_mode_set(Evas_Object *obj, Elm_Cnp_Mode cnp_mode);

/**
 * Getting elm_entry text paste/drop mode.
 *
 * Normally the entry allows both text and images to be pasted.
 * This gets the copy & paste mode of the entry.
 *
 * @param obj The entry object
 * @return mode One of #Elm_Cnp_Mode: #ELM_CNP_MODE_MARKUP, #ELM_CNP_MODE_NO_IMAGE, #ELM_CNP_MODE_PLAINTEXT.
 *
 * @ingroup Entry
 */
EAPI Elm_Cnp_Mode elm_entry_cnp_mode_get(const Evas_Object *obj);

/**
 * Enable or disable scrolling in entry
 *
 * Normally the entry is not scrollable unless you enable it with this call.
 *
 * @param obj The entry object
 * @param scroll EINA_TRUE if it is to be scrollable, EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_scrollable_set(Evas_Object *obj, Eina_Bool scroll);

/**
 * Get the scrollable state of the entry
 *
 * Normally the entry is not scrollable. This gets the scrollable state
 * of the entry. See elm_entry_scrollable_set() for more information.
 *
 * @param obj The entry object
 * @return The scrollable state
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_scrollable_get(const Evas_Object *obj);

/**
 * Sets the visibility of the left-side widget of the entry,
 * set by elm_object_part_content_set().
 *
 * @param obj The entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting);

/**
 * Sets the visibility of the end widget of the entry, set by
 * elm_object_part_content_set(ent, "end", content).
 *
 * @param obj The entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting);

/**
 * Set the input panel layout of the entry
 *
 * @param obj The entry object
 * @param layout layout type
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_layout_set(Evas_Object *obj, Elm_Input_Panel_Layout layout);

/**
 * Get the input panel layout of the entry
 *
 * @param obj The entry object
 * @return layout type
 *
 * @see elm_entry_input_panel_layout_set
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Layout elm_entry_input_panel_layout_get(const Evas_Object *obj);

/**
 * Set the autocapitalization type on the immodule.
 *
 * @param obj The entry object
 * @param autocapital_type The type of autocapitalization
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_autocapital_type_set(Evas_Object *obj, Elm_Autocapital_Type autocapital_type);

/**
 * Retrieve the autocapitalization type on the immodule.
 *
 * @param obj The entry object
 * @return autocapitalization type
 *
 * @ingroup Entry
 */
EAPI Elm_Autocapital_Type   elm_entry_autocapital_type_get(const Evas_Object *obj);

/**
 * Set whether the entry should allow to use the text prediction.
 *
 * @param obj The entry object
 * @param prediction Whether the entry should allow to use the text prediction.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_prediction_allow_set(Evas_Object *obj, Eina_Bool prediction);

/**
 * Get whether the entry should allow to use the text prediction.
 *
 * @param obj The entry object
 * @return EINA_TRUE if it allows to use the text prediction, otherwise EINA_FALSE.
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_prediction_allow_get(const Evas_Object *obj);

/**
 * Reset the input method context of the entry if needed.
 *
 * This can be necessary in the case where modifying the buffer would confuse on-going input method behavior.
 * This will typically cause the Input Method Context to clear the preedit state.
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_imf_context_reset(Evas_Object *obj);

/**
 * Sets the attribute to show the input panel automatically.
 *
 * @param obj The entry object
 * @param enabled If true, the input panel is appeared when entry is clicked or has a focus
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Retrieve the attribute to show the input panel automatically.
 *
 * @param obj The entry object
 * @return EINA_TRUE if input panel will be appeared when the entry is clicked or has a focus, EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_enabled_get(const Evas_Object *obj);

/**
 * Show the input panel (virtual keyboard) based on the input panel property of entry such as layout, autocapital types, and so on.
 *
 * Note that input panel is shown or hidden automatically according to the focus state of entry widget.
 * This API can be used in the case of manually controlling by using elm_entry_input_panel_enabled_set(en, EINA_FALSE).
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_show(Evas_Object *obj);

/**
 * Hide the input panel (virtual keyboard).
 *
 * Note that input panel is shown or hidden automatically according to the focus state of entry widget.
 * This API can be used in the case of manually controlling by using elm_entry_input_panel_enabled_set(en, EINA_FALSE)
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_hide(Evas_Object *obj);

/**
 * Set the language mode of the input panel.
 *
 * This API can be used if you want to show the alphabet keyboard mode.
 *
 * @param obj The entry object
 * @param lang language to be set to the input panel.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_language_set(Evas_Object *obj, Elm_Input_Panel_Lang lang);

/**
 * Get the language mode of the input panel.
 *
 * See @ref elm_entry_input_panel_language_set for more details.
 *
 * @param obj The entry object
 * @return input panel language type
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Lang   elm_entry_input_panel_language_get(const Evas_Object *obj);

/**
 * Set the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input panel.
 * The data format MUST be negotiated by both application and the input panel.
 * The size and format of data are defined by the input panel.
 *
 * @param obj The entry object
 * @param data The specific data to be set to the input panel.
 * @param len the length of data, in bytes, to send to the input panel
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_imdata_set(Evas_Object *obj, const void *data, int len);

/**
 * Get the specific data of the current input panel.
 *
 * See @ref elm_entry_input_panel_imdata_set for more details.
 *
 * @param obj The entry object
 * @param data The specific data to be got from the input panel
 * @param len The length of data
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_imdata_get(const Evas_Object *obj, void *data, int *len);

/**
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type
 *
 * @param obj The entry object
 * @param return_key_type The type of "return" key on the input panel
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_type_set(Evas_Object *obj, Elm_Input_Panel_Return_Key_Type return_key_type);

/**
 * Get the "return" key type.
 *
 * @see elm_entry_input_panel_return_key_type_set() for more details
 *
 * @param obj The entry object
 * @return The type of "return" key on the input panel
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Return_Key_Type elm_entry_input_panel_return_key_type_get(const Evas_Object *obj);

/**
 * Set the return key on the input panel to be disabled.
 *
 * @param obj The entry object
 * @param disabled The state to put in in: @c EINA_TRUE for
 *        disabled, @c EINA_FALSE for enabled
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param obj The entry object
 * @return EINA_TRUE if it should be disabled
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_return_key_disabled_get(const Evas_Object *obj);

/**
 * Set whether the return key on the input panel is disabled automatically when entry has no text.
 *
 * If @p enabled is EINA_TRUE, The return key on input panel is disabled when the entry has no text.
 * The return key on the input panel is automatically enabled when the entry has text.
 * The default value is EINA_FALSE.
 *
 * @param obj The entry object
 * @param enabled If @p enabled is EINA_TRUE, the return key is automatically disabled when the entry has no text.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_autoenabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Returns the input method context of the entry.
 *
 * This function exposes the internal input method context.
 *
 * IMPORTANT: Many functions may change (i.e delete and create a new one)
 * the internal input method context. Do NOT cache the returned object.
 *
 * @param obj The entry object
 * @return The input method context (Ecore_IMF_Context *) in entry.
 *
 * @ingroup Entry
 */
EAPI void                  *elm_entry_imf_context_get(Evas_Object *obj);

/**
 * Set the parent of the hover popup
 *
 * Sets the parent object to use by the hover created by the entry
 * when an anchor is clicked. See @ref Hover for more details on this.
 *
 * @param obj The entry object
 * @param parent The object to use as parent for the hover
 *
 * @ingroup Entry
 */
EAPI void                        elm_entry_anchor_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * Get the parent of the hover popup
 *
 * Get the object used as parent for the hover created by the entry
 * widget. See @ref Hover for more details on this.
 * If no parent is set, the same entry object will be used.
 *
 * @param obj The entry object
 * @return The object used as parent for the hover, NULL if none is set.
 *
 * @ingroup Entry
 */
EAPI Evas_Object                *elm_entry_anchor_hover_parent_get(const Evas_Object *obj);

/**
 * Set the style that the hover should use
 *
 * When creating the popup hover, entry will request that it's
 * themed according to @p style.
 *
 * Setting style no @c NULL means disabling automatic hover.
 *
 * @param obj The entry object
 * @param style The style to use for the underlying hover
 *
 * @see elm_object_style_set()
 *
 * @ingroup Entry
 */
EAPI void                        elm_entry_anchor_hover_style_set(Evas_Object *obj, const char *style);

/**
 * Get the style that the hover should use
 *
 * Get the style, the hover created by entry will use.
 *
 * @param obj The entry object
 * @return The style to use by the hover. @c NULL means the default is used.
 *
 * @see elm_object_style_set()
 *
 * @ingroup Entry
 */
EAPI const char                 *elm_entry_anchor_hover_style_get(const Evas_Object *obj);

/**
 * Ends the hover popup in the entry
 *
 * When an anchor is clicked, the entry widget will create a hover
 * object to use as a popup with user provided content. This function
 * terminates this popup, returning the entry to its normal state.
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                        elm_entry_anchor_hover_end(Evas_Object *obj);

/**
 * Set the input panel layout variation of the entry
 *
 * @param obj The entry object
 * @param variation layout variation type
 *
 * @ingroup Entry
 * @since 1.8
 */
EAPI void                   elm_entry_input_panel_layout_variation_set(Evas_Object *obj, int variation);

/**
 * Get the input panel layout variation of the entry
 *
 * @param obj The entry object
 * @return layout variation type
 *
 * @see elm_entry_input_panel_layout_variation_set
 *
 * @ingroup Entry
 * @since 1.8
 */
EAPI int                    elm_entry_input_panel_layout_variation_get(const Evas_Object *obj);

/**
 * Set the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 * It doesn't request to show the input panel even though it has focus.
 *
 * @param obj The entry object
 * @param ondemand If true, the input panel will be shown in case of only Mouse up event.
 *                 (Focus event will be ignored.)
 * @since 1.9
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_show_on_demand_set(Evas_Object *obj, Eina_Bool ondemand);

/**
 * Get the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param obj The entry object
 * @return @c EINA_TRUE if the input panel will be shown in case of only Mouse up event.
 * @since 1.9
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_show_on_demand_get(const Evas_Object *obj);
