#ifndef _ELM_INDEX_EO_LEGACY_H_
#define _ELM_INDEX_EO_LEGACY_H_

#ifndef _ELM_INDEX_EO_CLASS_TYPE
#define _ELM_INDEX_EO_CLASS_TYPE

typedef Eo Elm_Index;

#endif

#ifndef _ELM_INDEX_EO_TYPES
#define _ELM_INDEX_EO_TYPES


#endif

/**
 * @brief Enable or disable auto hiding feature for a given index widget.
 *
 * @param[in] obj The object.
 * @param[in] disabled @c true to disable auto hiding, @c false to enable
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_autohide_disabled_set(Elm_Index *obj, Eina_Bool disabled);

/**
 * @brief Enable or disable auto hiding feature for a given index widget.
 *
 * @param[in] obj The object.
 *
 * @return @c true to disable auto hiding, @c false to enable
 *
 * @ingroup Elm_Index_Group
 */
EAPI Eina_Bool elm_index_autohide_disabled_get(const Elm_Index *obj);

/**
 * @brief Enable or disable omit feature for a given index widget.
 *
 * @param[in] obj The object.
 * @param[in] enabled @c true to enable omit feature, @c false to disable
 *
 * @since 1.8
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_omit_enabled_set(Elm_Index *obj, Eina_Bool enabled);

/**
 * @brief Enable or disable omit feature for a given index widget.
 *
 * @param[in] obj The object.
 *
 * @return @c true to enable omit feature, @c false to disable
 *
 * @since 1.8
 *
 * @ingroup Elm_Index_Group
 */
EAPI Eina_Bool elm_index_omit_enabled_get(const Elm_Index *obj);

/**
 * @brief Control standard_priority group of index. Priority group will be
 * shown as many items as it can, and other group will be shown one character
 * only.
 *
 * @param[in] obj The object.
 * @param[in] priority @c priority target priority value in index
 *
 * @since 1.16
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_standard_priority_set(Elm_Index *obj, int priority);

/**
 * @brief Control standard_priority group of index. Priority group will be
 * shown as many items as it can, and other group will be shown one character
 * only.
 *
 * @param[in] obj The object.
 *
 * @return @c priority target priority value in index
 *
 * @since 1.16
 *
 * @ingroup Elm_Index_Group
 */
EAPI int elm_index_standard_priority_get(const Elm_Index *obj);

/**
 * @brief Set a delay change time for index object.
 *
 * @note delay time is 0.2 sec by default.
 *
 * @param[in] obj The object.
 * @param[in] dtime The delay change time to set.
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_delay_change_time_set(Elm_Index *obj, double dtime);

/**
 * @brief Set a delay change time for index object.
 *
 * @note delay time is 0.2 sec by default.
 *
 * @param[in] obj The object.
 *
 * @return The delay change time to set.
 *
 * @ingroup Elm_Index_Group
 */
EAPI double elm_index_delay_change_time_get(const Elm_Index *obj);

/**
 * @brief Control the indicator as to be disabled.
 *
 * In Index widget, Indicator notes popup text, which shows a letter has been
 * selecting.
 *
 * @param[in] obj The object.
 * @param[in] disabled @c true to disable it, @c false to enable it
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_indicator_disabled_set(Elm_Index *obj, Eina_Bool disabled);

/**
 * @brief Control the indicator as to be disabled.
 *
 * In Index widget, Indicator notes popup text, which shows a letter has been
 * selecting.
 *
 * @param[in] obj The object.
 *
 * @return @c true to disable it, @c false to enable it
 *
 * @ingroup Elm_Index_Group
 */
EAPI Eina_Bool elm_index_indicator_disabled_get(const Elm_Index *obj);

/**
 * @brief Set the items level for a given index widget.
 *
 * See: elm_index_item_level_get()
 *
 * @param[in] obj The object.
 * @param[in] level $0 or $1, the currently implemented levels.
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_item_level_set(Elm_Index *obj, int level);

/**
 * @brief Set the items level for a given index widget.
 *
 * See: elm_index_item_level_get()
 *
 * @param[in] obj The object.
 *
 * @return $0 or $1, the currently implemented levels.
 *
 * @ingroup Elm_Index_Group
 */
EAPI int elm_index_item_level_get(const Elm_Index *obj);

/**
 * @brief Flush the changes made to the index items so they work correctly
 *
 * This flushes any changes made to items indicating the object is ready to go.
 * You should call this before any changes you expect to work. This is similar
 * to elm_list_go().
 *
 * @warning If not called, it won't display the index properly.
 *
 * @param[in] obj The object.
 * @param[in] level The index level (one of $0 or $1) where changes were made
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_level_go(Elm_Index *obj, int level);

/**
 * @brief Prepend a new item on a given index widget.
 *
 * Despite the most common usage of the @c letter argument is for single char
 * strings, one could use arbitrary strings as index entries.
 *
 * @c item will be the pointer returned back on $"changed", "delay,changed" and
 * $"selected" smart events.
 *
 * @param[in] obj The object.
 * @param[in] letter Letter under which the item should be indexed
 * @param[in] func The function to call when the item is selected.
 * @param[in] data The item data to set for the index's item
 *
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_item_prepend(Elm_Index *obj, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * @brief Removes all items from a given index widget.
 *
 * If deletion callbacks are set, via elm_object_item_del_cb_set(), that
 * callback function will be called for each item in @c obj.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Index_Group
 */
EAPI void elm_index_item_clear(Elm_Index *obj);

/**
 * @brief Insert a new item into the index object after item @c after.
 *
 * Despite the most common usage of the @c letter argument is for single char
 * strings, one could use arbitrary strings as index entries.
 *
 * @c item will be the pointer returned back on $"changed", "delay,changed" and
 * $"selected" smart events.
 *
 * @note If @c after is @c NULL this function will behave as
 * elm_index_item_append().
 *
 * @param[in] obj The object.
 * @param[in] after The index item to insert after.
 * @param[in] letter Letter under which the item should be indexed
 * @param[in] func The function to call when the item is clicked.
 * @param[in] data The item data to set for the index's item
 *
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_item_insert_after(Elm_Index *obj, Elm_Widget_Item *after, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * @brief Find a given index widget's item, using item data.
 *
 * @param[in] obj The object.
 * @param[in] data The item data pointed to by the desired index item
 *
 * @return The index item handle, if found, or @c NULL otherwise
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_item_find(Elm_Index *obj, const void *data);

/**
 * @brief Insert a new item into the index object before item @c before.
 *
 * Despite the most common usage of the @c letter argument is for single char
 * strings, one could use arbitrary strings as index entries.
 *
 * @c item will be the pointer returned back on $"changed", "delay,changed" and
 * $"selected" smart events.
 *
 * @note If @c before is @c NULL this function will behave as
 * elm_index_item_prepend().
 *
 * @param[in] obj The object.
 * @param[in] before The index item to insert after.
 * @param[in] letter Letter under which the item should be indexed
 * @param[in] func The function to call when the item is clicked.
 * @param[in] data The item data to set for the index's item
 *
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_item_insert_before(Elm_Index *obj, Elm_Widget_Item *before, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * @brief Append a new item on a given index widget.
 *
 * Despite the most common usage of the @c letter argument is for single char
 * strings, one could use arbitrary strings as index entries.
 *
 * @c item will be the pointer returned back on $"changed", "delay,changed" and
 * $"selected" smart events.
 *
 * @param[in] obj The object.
 * @param[in] letter Letter under which the item should be indexed
 * @param[in] func The function to call when the item is selected.
 * @param[in] data The item data to set for the index's item
 *
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_item_append(Elm_Index *obj, const char *letter, Evas_Smart_Cb func, const void *data);

/**
 * @brief Returns the last selected item, for a given index widget.
 *
 * @param[in] obj The object.
 * @param[in] level $0 or $1, the currently implemented levels.
 *
 * @return The last item selected on @c obj (or @c NULL, on errors).
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_selected_item_get(const Elm_Index *obj, int level);

/**
 * @brief Insert a new item into the given index widget, using @c cmp_func
 * function to sort items (by item handles).
 *
 * Despite the most common usage of the @c letter argument is for single char
 * strings, one could use arbitrary strings as index entries.
 *
 * @c item will be the pointer returned back on $"changed", "delay,changed" and
 * $"selected" smart events.
 *
 * @param[in] obj The object.
 * @param[in] letter Letter under which the item should be indexed
 * @param[in] func The function to call when the item is clicked.
 * @param[in] data The item data to set for the index's item
 * @param[in] cmp_func The comparing function to be used to sort index items by
 * index item handles
 * @param[in] cmp_data_func A fallback function to be called for the sorting of
 * index items by item data). It will be used when @c cmp_func returns $0
 * (equality), which means an index item with provided item data already
 * exists. To decide which data item should be pointed to by the index item in
 * question, @c cmp_data_func will be used. If @c cmp_data_func returns a
 * non-negative value, the previous index item data will be replaced by the
 * given @c item pointer. If the previous data need to be freed, it should be
 * done by the @c cmp_data_func function, because all references to it will be
 * lost. If this function is not provided ($NULL is given), index items will be
 * duplicated, if @c cmp_func returns $0.
 *
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Elm_Index_Group
 */
EAPI Elm_Widget_Item *elm_index_item_sorted_insert(Elm_Index *obj, const char *letter, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func, Eina_Compare_Cb cmp_data_func);

#endif
