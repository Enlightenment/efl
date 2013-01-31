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

#define ELM_OBJ_INDEX_CLASS elm_obj_index_class_get()

const Eo_Class *elm_obj_index_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_INDEX_BASE_ID;

enum
{
   ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_SET,
   ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_GET,
   ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_SET,
   ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_GET,
   ELM_OBJ_INDEX_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_INDEX_SUB_ID_ITEM_APPEND,
   ELM_OBJ_INDEX_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_INDEX_SUB_ID_ITEM_SORTED_INSERT,
   ELM_OBJ_INDEX_SUB_ID_ITEM_FIND,
   ELM_OBJ_INDEX_SUB_ID_ITEM_CLEAR,
   ELM_OBJ_INDEX_SUB_ID_LEVEL_GO,
   ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_SET,
   ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_GET,
   ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_SET,
   ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_GET,
   ELM_OBJ_INDEX_SUB_ID_LAST
};

#define ELM_OBJ_INDEX_ID(sub_id) (ELM_OBJ_INDEX_BASE_ID + sub_id)


/**
 * @def elm_obj_index_autohide_disabled_set
 * @since 1.8
 *
 * Enable or disable auto hiding feature for a given index widget.
 *
 * @param[in] disabled
 *
 * @see elm_index_autohide_disabled_set
 */
#define elm_obj_index_autohide_disabled_set(disabled) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_index_autohide_disabled_get
 * @since 1.8
 *
 * Get whether auto hiding feature is enabled or not for a given index widget.
 *
 * @param[out] ret
 *
 * @see elm_index_autohide_disabled_get
 */
#define elm_obj_index_autohide_disabled_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_index_item_level_set
 * @since 1.8
 *
 * Set the items level for a given index widget.
 *
 * @param[in] level
 *
 * @see elm_index_item_level_set
 */
#define elm_obj_index_item_level_set(level) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_SET), EO_TYPECHECK(int, level)

/**
 * @def elm_obj_index_item_level_get
 * @since 1.8
 *
 * Get the items level set for a given index widget.
 *
 * @param[out] ret
 *
 * @see elm_index_item_level_get
 */
#define elm_obj_index_item_level_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_index_selected_item_get
 * @since 1.8
 *
 * Returns the last selected item, for a given index widget.
 *
 * @param[in] level
 * @param[out] ret
 *
 * @see elm_index_selected_item_get
 */
#define elm_obj_index_selected_item_get(level, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(int, level), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_append
 * @since 1.8
 *
 * Append a new item on a given index widget.
 *
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_append
 */
#define elm_obj_index_item_append(letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_prepend
 * @since 1.8
 *
 * Prepend a new item on a given index widget.
 *
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_prepend
 */
#define elm_obj_index_item_prepend(letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_insert_after
 * @since 1.8
 *
 * Insert a new item into the index object after item after.
 *
 * @param[in] after
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_insert_after
 */
#define elm_obj_index_item_insert_after(after, letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_insert_before
 * @since 1.8
 *
 * Insert a new item into the index object before item before.
 *
 * @param[in] before
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_insert_before
 */
#define elm_obj_index_item_insert_before(before, letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_sorted_insert
 * @since 1.8
 *
 * Insert a new item into the given index widget, using cmp_func
 *
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[in] cmp_func
 * @param[in] cmp_data_func
 * @param[out] ret
 *
 * @see elm_index_item_sorted_insert
 */
#define elm_obj_index_item_sorted_insert(letter, func, data, cmp_func, cmp_data_func, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_SORTED_INSERT), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Compare_Cb, cmp_func), EO_TYPECHECK(Eina_Compare_Cb, cmp_data_func), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_find
 * @since 1.8
 *
 * Find a given index widget's item, <b>using item data</b>.
 *
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_find
 */
#define elm_obj_index_item_find(data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_FIND), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_clear
 * @since 1.8
 *
 * Removes all items from a given index widget.
 *
 *
 * @see elm_index_item_clear
 */
#define elm_obj_index_item_clear() ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_CLEAR)

/**
 * @def elm_obj_index_level_go
 * @since 1.8
 *
 * Go to a given items level on a index widget
 *
 * @param[in] level
 *
 * @see elm_index_level_go
 */
#define elm_obj_index_level_go(level) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_LEVEL_GO), EO_TYPECHECK(int, level)

/**
 * @def elm_obj_index_indicator_disabled_set
 * @since 1.8
 *
 * Set the indicator as to be disabled.
 *
 * @param[in] disabled
 *
 * @see elm_index_indicator_disabled_set
 */
#define elm_obj_index_indicator_disabled_set(disabled) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_index_indicator_disabled_get
 * @since 1.8
 *
 * Get the value of indicator's disabled status.
 *
 * @param[out] ret
 *
 * @see elm_index_indicator_disabled_get
 */
#define elm_obj_index_indicator_disabled_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_index_horizontal_set
 * @since 1.8
 *
 * Enable or disable horizontal mode on the index object
 *
 * @param[in] horizontal
 *
 * @see elm_index_horizontal_set
 */
#define elm_obj_index_horizontal_set(horizontal) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_index_horizontal_get
 * @since 1.8
 *
 * Get a value whether horizontal mode is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_index_horizontal_get
 */
#define elm_obj_index_horizontal_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_index_delay_change_time_set
 * @since 1.8
 *
 * Change index delay change time.
 *
 * @param[in] ret
 *
 * @see elm_index_delay_change_time_get
 */
#define elm_obj_index_delay_change_time_set(dtime) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_SET), EO_TYPECHECK(double, dtime)

/**
 * @def elm_obj_index_delay_change_time_get
 * @since 1.8
 *
 * Get index delay change time.
 *
 * @param[out] ret
 *
 * @see elm_index_delay_change_time_set
 */
#define elm_obj_index_delay_change_time_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_GET), EO_TYPECHECK(double *, ret)

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
 * @}
 */
