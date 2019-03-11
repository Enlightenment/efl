#ifndef _ELM_MULTIBUTTONENTRY_EO_LEGACY_H_
#define _ELM_MULTIBUTTONENTRY_EO_LEGACY_H_

#ifndef _ELM_MULTIBUTTONENTRY_EO_CLASS_TYPE
#define _ELM_MULTIBUTTONENTRY_EO_CLASS_TYPE

typedef Eo Elm_Multibuttonentry;

#endif

#ifndef _ELM_MULTIBUTTONENTRY_EO_TYPES
#define _ELM_MULTIBUTTONENTRY_EO_TYPES


#endif

/**
 * @brief Control if the multibuttonentry is to be editable or not.
 *
 * @param[in] obj The object.
 * @param[in] editable If @c true, user can add/delete item in
 * multibuttonentry, if not, the multibuttonentry is non-editable.
 *
 * @since 1.7
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_editable_set(Elm_Multibuttonentry *obj, Eina_Bool editable);

/**
 * @brief Control if the multibuttonentry is to be editable or not.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, user can add/delete item in multibuttonentry, if not,
 * the multibuttonentry is non-editable.
 *
 * @since 1.7
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Eina_Bool elm_multibuttonentry_editable_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Control the multibuttonentry to expanded state.
 *
 * In expanded state, the complete entry will be displayed. Otherwise, only
 * single line of the entry will be displayed.
 *
 * @param[in] obj The object.
 * @param[in] expanded The value of expanded state. Set this to @c true for
 * expanded state. Set this to @c false for single line state.
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_expanded_set(Elm_Multibuttonentry *obj, Eina_Bool expanded);

/**
 * @brief Control the multibuttonentry to expanded state.
 *
 * In expanded state, the complete entry will be displayed. Otherwise, only
 * single line of the entry will be displayed.
 *
 * @param[in] obj The object.
 *
 * @return The value of expanded state. Set this to @c true for expanded state.
 * Set this to @c false for single line state.
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Eina_Bool elm_multibuttonentry_expanded_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Set a function to format the string that will be used to display the
 * hidden items counter.
 *
 * If @c format_function is @c NULL, the default format will be used, which is
 * $"... + %d".
 *
 * @param[in] obj The object.
 * @param[in] format_function Format_function The actual format function
 * @param[in] data Data User data to passed to @c format_function
 *
 * @since 1.9
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_format_function_set(Elm_Multibuttonentry *obj, Elm_Multibuttonentry_Format_Cb format_function, const void *data);

/**
 * @brief Get a list of items in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The list of items, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI const Eina_List *elm_multibuttonentry_items_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Get the first item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The first item, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_first_item_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Get the last item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The last item, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_last_item_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Get the entry of the multibuttonentry object
 *
 * @param[in] obj The object.
 *
 * @return The entry object, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Efl_Canvas_Object *elm_multibuttonentry_entry_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Get the selected item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The selected item, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_selected_item_get(const Elm_Multibuttonentry *obj);

/**
 * @brief Prepend a new item to the multibuttonentry
 *
 * See @ref Efl.Object.invalidate to delete the item.
 *
 * @param[in] obj The object.
 * @param[in] label The label of new item
 * @param[in] func The callback function to be invoked when this item is
 * pressed.
 * @param[in] data The pointer to the data to be attached
 *
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_item_prepend(Elm_Multibuttonentry *obj, const char *label, Evas_Smart_Cb func, void *data);

/** Remove all items in the multibuttonentry.
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_clear(Elm_Multibuttonentry *obj);

/**
 * @brief Remove a filter from the list
 *
 * Removes the given callback from the filter list. See
 * elm_multibuttonentry_item_filter_append() for more information.
 *
 * @param[in] obj The object.
 * @param[in] func The filter function to remove
 * @param[in] data The user data passed when adding the function
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_item_filter_remove(Elm_Multibuttonentry *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * @brief Add a new item to the multibuttonentry before the indicated object
 * reference.
 *
 * See @ref Efl.Object.invalidate to delete the item.
 *
 * @param[in] obj The object.
 * @param[in] before The item before which to add it
 * @param[in] label The label of new item
 * @param[in] func The callback function to be invoked when this item is
 * pressed.
 * @param[in] data The pointer to the data to be attached
 *
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_item_insert_before(Elm_Multibuttonentry *obj, Elm_Widget_Item *before, const char *label, Evas_Smart_Cb func, void *data);

/**
 * @brief Append a new item to the multibuttonentry
 *
 * See @ref Efl.Object.invalidate to delete the item.
 *
 * @param[in] obj The object.
 * @param[in] label The label of new item
 * @param[in] func The callback function to be invoked when this item is
 * pressed.
 * @param[in] data The pointer to the data to be attached
 *
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_item_append(Elm_Multibuttonentry *obj, const char *label, Evas_Smart_Cb func, void *data);

/**
 * @brief Prepend a filter function for text inserted in the Multibuttonentry
 *
 * Prepend the given callback to the list. See
 * elm_multibuttonentry_item_filter_append() for more information
 *
 * @param[in] obj The object.
 * @param[in] func The function to use as text filter
 * @param[in] data User data to pass to @c func
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_item_filter_prepend(Elm_Multibuttonentry *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * @brief Append an item filter function for text inserted in the
 * Multibuttonentry
 *
 * Append the given callback to a list. This function is called whenever any
 * text is inserted into the Multibuttonentry, with the text to be inserted as
 * a parameter. The callback function is free to alter the text in any way it
 * wants but must free the given pointer and update it. If the new text is to
 * be discarded, the function can free it and set it text parameter to NULL.
 * This will also prevent any following filters from being called.
 *
 * @param[in] obj The object.
 * @param[in] func The function to use as item filter
 * @param[in] data User data to pass to @c func
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI void elm_multibuttonentry_item_filter_append(Elm_Multibuttonentry *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * @brief Add a new item to the multibuttonentry after the indicated object
 *
 * See @ref Efl.Object.invalidate to delete the item.
 *
 * @param[in] obj The object.
 * @param[in] after The item after which to add it
 * @param[in] label The label of new item
 * @param[in] func The callback function to be invoked when this item is
 * pressed.
 * @param[in] data The pointer to the data to be attached
 *
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Elm_Multibuttonentry_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_item_insert_after(Elm_Multibuttonentry *obj, Elm_Widget_Item *after, const char *label, Evas_Smart_Cb func, void *data);

#endif
