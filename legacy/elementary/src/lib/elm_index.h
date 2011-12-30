/**
 * @defgroup Index Index
 *
 * @image html img/widget/index/preview-00.png
 * @image latex img/widget/index/preview-00.eps
 *
 * An index widget gives you an index for fast access to whichever
 * group of other UI items one might have. It's a list of text
 * items (usually letters, for alphabetically ordered access).
 *
 * Index widgets are by default hidden and just appear when the
 * user clicks over it's reserved area in the canvas. In its
 * default theme, it's an area one @ref Fingers "finger" wide on
 * the right side of the index widget's container.
 *
 * When items on the index are selected, smart callbacks get
 * called, so that its user can make other container objects to
 * show a given area or child object depending on the index item
 * selected. You'd probably be using an index together with @ref
 * List "lists", @ref Genlist "generic lists" or @ref Gengrid
 * "general grids".
 *
 * Smart events one  can add callbacks for are:
 * - @c "changed" - When the selected index item changes. @c
 *      event_info is the selected item's data pointer.
 * - @c "delay,changed" - When the selected index item changes, but
 *      after a small idling period. @c event_info is the selected
 *      item's data pointer.
 * - @c "selected" - When the user releases a mouse button and
 *      selects an item. @c event_info is the selected item's data
 *      pointer.
 * - @c "level,up" - when the user moves a finger from the first
 *      level to the second level
 * - @c "level,down" - when the user moves a finger from the second
 *      level to the first level
 *
 * The @c "delay,changed" event is so that it'll wait a small time
 * before actually reporting those events and, moreover, just the
 * last event happening on those time frames will actually be
 * reported.
 *
 * Here are some examples on its usage:
 * @li @ref index_example_01
 * @li @ref index_example_02
 */

/**
 * @addtogroup Index
 * @{
 */

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
EAPI Evas_Object *
                           elm_index_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * Set whether a given index widget is or not visible,
 * programatically.
 *
 * @param obj The index object
 * @param active @c EINA_TRUE to show it, @c EINA_FALSE to hide it
 *
 * Not to be confused with visible as in @c evas_object_show() --
 * visible with regard to the widget's auto hiding feature.
 *
 * @see elm_index_active_get()
 *
 * @ingroup Index
 */
EAPI void                  elm_index_active_set(Evas_Object *obj, Eina_Bool active) EINA_ARG_NONNULL(1);

/**
 * Get whether a given index widget is currently visible or not.
 *
 * @param obj The index object
 * @return @c EINA_TRUE, if it's shown, @c EINA_FALSE otherwise
 *
 * @see elm_index_active_set() for more details
 *
 * @ingroup Index
 */
EAPI Eina_Bool             elm_index_active_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
EAPI void                  elm_index_item_level_set(Evas_Object *obj, int level) EINA_ARG_NONNULL(1);

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
EAPI int                   elm_index_item_level_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Returns the last selected item, for a given index widget.
 *
 * @param obj The index object.
 * @return The last item @b selected on @p obj (or @c NULL, on errors).
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_selected_get(const Evas_Object *obj, int level) EINA_ARG_NONNULL(1);

/**
 * Append a new item on a given index widget.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
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
EAPI void                  elm_index_item_append(Evas_Object *obj, const char *letter, const void *item) EINA_ARG_NONNULL(1);

/**
 * Prepend a new item on a given index widget.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
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
EAPI void                  elm_index_item_prepend(Evas_Object *obj, const char *letter, const void *item) EINA_ARG_NONNULL(1);

/**
 * Append a new item, on a given index widget, <b>after the item
 * having @p relative as data</b>.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
 * @param relative The index item to be the predecessor of this new one
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
EAPI void                  elm_index_item_append_relative(Evas_Object *obj, const char *letter, const void *item, const Elm_Object_Item *relative) EINA_ARG_NONNULL(1);

/**
 * Prepend a new item, on a given index widget, <b>after the item
 * having @p relative as data</b>.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
 * @param relative The index item to be the successor of this new one
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
EAPI void                  elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const Elm_Object_Item *relative) EINA_ARG_NONNULL(1);

/**
 * Insert a new item into the given index widget, using @p cmp_func
 * function to sort items (by item handles).
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
 * @param cmp_func The comparing function to be used to sort index
 * items <b>by #index item handles</b>
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
EAPI void                  elm_index_item_sorted_insert(Evas_Object *obj, const char *letter, const void *item, Eina_Compare_Cb cmp_func, Eina_Compare_Cb cmp_data_func) EINA_ARG_NONNULL(1);

/**
 * Remove an item from a given index widget, <b>to be referenced by
 * it's data value</b>.
 *
 * @param obj The index object
 * @param item The item to be removed from @p obj
 *
 * If a deletion callback is set, via elm_index_item_del_cb_set(),
 * that callback function will be called by this one.
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_del(Evas_Object *obj, Elm_Object_Item *item) EINA_ARG_NONNULL(1);

/**
 * Find a given index widget's item, <b>using item data</b>.
 *
 * @param obj The index object
 * @param item The item data pointed to by the desired index item
 * @return The index item handle, if found, or @c NULL otherwise
 *
 * @ingroup Index
 */
EAPI Elm_Object_Item      *elm_index_item_find(Evas_Object *obj, const void *item) EINA_ARG_NONNULL(1);

/**
 * Removes @b all items from a given index widget.
 *
 * @param obj The index object.
 *
 * If deletion callbacks are set, via elm_index_item_del_cb_set(),
 * that callback function will be called for each item in @p obj.
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Go to a given items level on a index widget
 *
 * @param obj The index object
 * @param level The index level (one of @c 0 or @c 1)
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_go(Evas_Object *obj, int level) EINA_ARG_NONNULL(1);

/**
 * Return the data associated with a given index widget item
 *
 * @param it The index widget item handle
 * @return The data associated with @p it
 *
 * @see elm_index_item_data_set()
 * @deprecated Use elm_object_item_data_get() instead
 *
 * @ingroup Index
 */
EINA_DEPRECATED EAPI void *elm_index_item_data_get(const Elm_Object_Item *item) EINA_ARG_NONNULL(1);

/**
 * Set the data associated with a given index widget item
 *
 * @param it The index widget item handle
 * @param data The new data pointer to set to @p it
 *
 * This sets new item data on @p it.
 *
 * @warning The old data pointer won't be touched by this function, so
 * the user had better to free that old data himself/herself.
 *
 * @deprecated Use elm_object_item_data_set() instead
 * @ingroup Index
 */
EINA_DEPRECATED EAPI void  elm_index_item_data_set(Elm_Object_Item *it, const void *data) EINA_ARG_NONNULL(1);

/**
 * Set the function to be called when a given index widget item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function to call on the item's deletion
 *
 * When called, @p func will have both @c data and @c event_info
 * arguments with the @p it item's data value and, naturally, the
 * @c obj argument with a handle to the parent index widget.
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func) EINA_ARG_NONNULL(1);

/**
 * Get the letter (string) set on a given index widget item.
 *
 * @param it The index item handle
 * @return The letter string set on @p it
 *
 * @ingroup Index
 */
EAPI const char           *elm_index_item_letter_get(const Elm_Object_Item *item) EINA_ARG_NONNULL(1);

/**
 * @}
 */
