/**
 * @defgroup Index Index
 * @ingroup Elementary
 *
 * @image html index_inheritance_tree.png
 * @image latex index_inheritance_tree.eps
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
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for index objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
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
 * Set the selected state of an item.
 *
 * @param it The index item
 * @param selected The selected state
 *
 * This sets the selected state of the given item @p it.
 * @c EINA_TRUE for selected, @c EINA_FALSE for not selected.
 *
 * If a new item is selected the previously selected will be unselected.
 * Previously selected item can be get with function
 * elm_index_selected_item_get().
 *
 * Selected items will be highlighted.
 *
 * @see elm_index_selected_item_get()
 *
 * @ingroup Index
 */
EAPI void                  elm_index_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

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
 * Go to a given items level on a index widget
 *
 * @param obj The index object
 * @param level The index level (one of @c 0 or @c 1)
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
 * @}
 */
