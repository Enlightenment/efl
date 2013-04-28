/**
 * Add a new index widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new index widget handle or @c NULL, on errors
 *
 * This function inserts a new index widget on the canvas.
 *
 * @ingroup Index
 */
EAPI Evas_Object          *elm_index_add(Evas_Object *parent);

/**
 * Enable or disable auto hiding feature for a given index widget.
 *
 * @param obj The index object
 * @param disabled @c EINA_TRUE to disable auto hiding, @c EINA_FALSE to enable
 *
 * @see elm_index_autohide_disabled_get()
 *
 * @ingroup Index
 */
EAPI void                  elm_index_autohide_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get whether auto hiding feature is enabled or not for a given index widget.
 *
 * @param obj The index object
 * @return @c EINA_TRUE, if auto hiding is disabled, @c EINA_FALSE otherwise
 *
 * @see elm_index_autohide_disabled_set() for more details
 *
 * @ingroup Index
 */
EAPI Eina_Bool             elm_index_autohide_disabled_get(const Evas_Object *obj);

/**
 * Set the items level for a given index widget.
 *
 * @param obj The index object.
 * @param level @c 0 or @c 1, the currently implemented levels.
 *
 * @see elm_index_item_level_get()
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_level_set(Evas_Object *obj, int level);

/**
 * Get the items level set for a given index widget.
 *
 * @param obj The index object.
 * @return @c 0 or @c 1, which are the levels @p obj might be at.
 *
 * @see elm_index_item_level_set() for more information
 *
 * @ingroup Index
 */
EAPI int                   elm_index_item_level_get(const Evas_Object *obj);

/**
 * Returns the last selected item, for a given index widget.
 *
 * @param obj The index object.
 * @param level @c 0 or @c 1, the currently implemented levels.
 * @return The last item @b selected on @p obj (or @c NULL, on errors).
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_selected_item_get(const Evas_Object *obj, int level);

/**
 * Append a new item on a given index widget.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param func The function to call when the item is selected.
 * @param data The item data to set for the index's item
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_append(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * Prepend a new item on a given index widget.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param func The function to call when the item is selected.
 * @param data The item data to set for the index's item
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_prepend(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the index object after item @p after.
 *
 * @param obj The index object.
 * @param after The index item to insert after.
 * @param letter Letter under which the item should be indexed
 * @param func The function to call when the item is clicked.
 * @param data The item data to set for the index's item
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @note If @p relative is @c NULL this function will behave as
 * elm_index_item_append().
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the index object before item @p before.
 *
 * @param obj The index object.
 * @param before The index item to insert after.
 * @param letter Letter under which the item should be indexed
 * @param func The function to call when the item is clicked.
 * @param data The item data to set for the index's item
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @note If @p relative is @c NULL this function will behave as
 * elm_index_item_prepend().
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the given index widget, using @p cmp_func
 * function to sort items (by item handles).
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param func The function to call when the item is clicked.
 * @param data The item data to set for the index's item
 * @param cmp_func The comparing function to be used to sort index
 * items <b>by index item handles</b>
 * @param cmp_data_func A @b fallback function to be called for the
 * sorting of index items <b>by item data</b>). It will be used
 * when @p cmp_func returns @c 0 (equality), which means an index
 * item with provided item data already exists. To decide which
 * data item should be pointed to by the index item in question, @p
 * cmp_data_func will be used. If @p cmp_data_func returns a
 * non-negative value, the previous index item data will be
 * replaced by the given @p item pointer. If the previous data need
 * to be freed, it should be done by the @p cmp_data_func function,
 * because all references to it will be lost. If this function is
 * not provided (@c NULL is given), index items will be @b
 * duplicated, if @p cmp_func returns @c 0.
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_sorted_insert(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func, Eina_Compare_Cb cmp_data_func);

/**
 * Find a given index widget's item, <b>using item data</b>.
 *
 * @param obj The index object
 * @param data The item data pointed to by the desired index item
 * @return The index item handle, if found, or @c NULL otherwise
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_find(Evas_Object *obj, const void *data);

/**
 * Removes @b all items from a given index widget.
 *
 * @param obj The index object.
 *
 * If deletion callbacks are set, via elm_object_item_del_cb_set(),
 * that callback function will be called for each item in @p obj.
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_clear(Evas_Object *obj);

/**
 * Flush the changes made to the index items so they work correctly
 *
 * This flushes any changes made to items indicating the object is ready to
 * go. You should call this before any changes you expect to work. This
 * is similar to elm_list_go().
 *
 * @param obj The index object
 * @param level The index level (one of @c 0 or @c 1) where changes were made
 *
 * @warning If not called, it won't display the index properly.
 *
 * @ingroup Index
 */
EAPI void                  elm_index_level_go(Evas_Object *obj, int level);

/**
 * Get the letter (string) set on a given index widget item.
 *
 * @param item The index item handle
 * @return The letter string set on @p it
 *
 * @ingroup Index
 */
EAPI const char           *elm_index_item_letter_get(const Elm_Object_Item *item);

/**
 * Set the indicator as to be disabled.
 *
 * @param obj The index object
 * @param disabled  @c EINA_TRUE to disable it, @c EINA_FALSE to enable it
 *
 * In Index widget, Indicator notes popup text, which shows a letter has been selecting.
 *
 * @see elm_index_indicator_disabled_get()
 *
 * @ingroup Index
 */
EAPI void                 elm_index_indicator_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get the value of indicator's disabled status.
 *
 * @param obj The index object
 * @return EINA_TRUE if the indicator is disabled.
 *
 * @see elm_index_indicator_disabled_set()
 *
 * @ingroup Index
 */
EAPI Eina_Bool            elm_index_indicator_disabled_get(const Evas_Object *obj);

/**
 * Enable or disable horizontal mode on the index object
 *
 * @param obj The index object.
 * @param horizontal @c EINA_TRUE to enable horizontal or @c EINA_FALSE to
 * disable it, i.e., to enable vertical mode. it's an area one @ref Fingers
 * "finger" wide on the bottom side of the index widget's container.
 *
 * @note Vertical mode is set by default.
 *
 * On horizontal mode items are displayed on index from left to right,
 * instead of from top to bottom. Also, the index will scroll horizontally.
 *
 * @see elm_index_horizontal_get()
 *
 * @ingroup Index
 */
EAPI void                      elm_index_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Get a value whether horizontal mode is enabled or not.
 *
 * @param obj The index object.
 * @return @c EINA_TRUE means horizontal mode selection is enabled.
 * @c EINA_FALSE indicates it's disabled. If @p obj is @c NULL,
 * @c EINA_FALSE is returned.
 *
 * @see elm_index_horizontal_set() for details.
 *
 * @ingroup Index
 */
EAPI Eina_Bool                 elm_index_horizontal_get(const Evas_Object *obj);

/**
 * Set a delay change time for index object.
 *
 * @param obj The index object.
 * @param delay_change_time The delay change time to set.
 *
 * @note delay time is 0.2 sec by default.
 *
 * @see elm_index_delay_change_time_get
 *
 * @ingroup Index
 */
EAPI void                      elm_index_delay_change_time_set(Evas_Object *obj, double delay_change_time);

/**
 * Get a delay change time for index object.
 *
 * @param obj The index object.
 * @return delay change time in seconds
 *
 * @see elm_index_delay_change_time_set
 *
 * @ingroup Index
 */
EAPI double                    elm_index_delay_change_time_get(const Evas_Object *obj);

/**
 * Enable or disable omit feature for a given index widget.
 *
 * @param obj The index object
 * @param enabled @c EINA_TRUE to enable omit feature, @c EINA_FALSE to disable
 *
 * @see elm_index_omit_enabled_get()
 *
 * @since 1.8
 *
 * @ingroup Index
 */
EAPI void                      elm_index_omit_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Get whether omit feature is enabled or not for a given index widget.
 *
 * @param obj The index object
 * @return @c EINA_TRUE, if omit feature is enabled, @c EINA_FALSE otherwise
 *
 * @see elm_index_omit_enabled_set()
 *
 * @since 1.8
 *
 * @ingroup Index
 */
EAPI Eina_Bool                 elm_index_omit_enabled_get(const Evas_Object *obj);

