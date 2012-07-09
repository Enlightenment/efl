/**
 * @defgroup Multibuttonentry Multibuttonentry
 * @ingroup Elementary
 *
 * A Multibuttonentry is a widget to allow a user enter text and manage it as a number
 * of buttons. Each text button is inserted by pressing the "return" key.
 * If there is no space in the current row, a new button is added to the next row.
 * When a text button is pressed, it will become focused.
 * Backspace removes the focus.
 * When the Multibuttonentry loses focus items longer than one line are shrunk
 * to one line.
 *
 * Typical use case of multibuttonentry is, composing emails/messages to a group
 * of addresses, each of which is an item that can be clicked for further actions.
 *
 * Smart callbacks one can register:
 * - @c "item,selected" - this is called when an item is selected by api, user
 *       interaction, and etc. this is also called when a user press back space
 *       while cursor is on the first field of entry.
 * - @c "item,added" - when a new multibuttonentry item is added.
 * - @c "item,deleted" - when a multibuttonentry item is deleted.
 * - @c "item,clicked" - this is called when an item is clicked by user
 *       interaction. Both "item,selected" and "item,clicked" are needed.
 * - @c "clicked" - when multibuttonentry is clicked.
 * - @c "focused" - when multibuttonentry is focused.
 * - @c "unfocused" - when multibuttonentry is unfocused.
 * - @c "expanded" - when multibuttonentry is expanded.
 * - @c "contracted" - when multibuttonentry is contracted.
 * - @c "expand,state,changed" - when shrink mode state of multibuttonentry is changed.
 *
 * Default text parts of the multibuttonentry widget that you can use for are:
 * @li "default" - A label of the multibuttonentry
 *
 * Default text parts of the multibuttonentry items that you can use for are:
 * @li "default" - A label of the multibuttonentry item
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 *
 */

/**
 * @addtogroup Multibuttonentry
 * @{
 */

/**
 * @brief Callback to be invoked when an item is added to the multibuttonentry.
 *
 * @param obj The parent object
 * @param item_label The label corresponding to the added item.
 * @param item_data data specific to this item.
 * @param data data specific to the multibuttonentry.
 *
 * @return EINA_TRUE
 *         EINA_FALSE otherwise.
 *
 * @ingroup Multibuttonentry
 */
typedef Eina_Bool                   (*Elm_Multibuttonentry_Item_Filter_Cb)(Evas_Object *obj, const char *item_label, void *item_data, void *data);

/**
 * @brief Add a new multibuttonentry to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 *
 * @ingroup Multibuttonentry
 */
EAPI Evas_Object               *elm_multibuttonentry_add(Evas_Object *parent);


/**
 * Get the entry of the multibuttonentry object
 *
 * @param obj The multibuttonentry object
 * @return The entry object, or NULL if none
 *
 * @ingroup Multibuttonentry
 */
EAPI Evas_Object               *elm_multibuttonentry_entry_get(const Evas_Object *obj);

/**
 * Get the value of expanded state.
 * In expanded state, the complete entry will be displayed.
 * Otherwise, only single line of the entry will be displayed.
 *
 * @param obj The multibuttonentry object
 * @return EINA_TRUE if the widget is in expanded state. EINA_FALSE if not.
 *
 * @ingroup Multibuttonentry
 */
EAPI Eina_Bool                  elm_multibuttonentry_expanded_get(const Evas_Object *obj);

/**
 * Set/Unset the multibuttonentry to expanded state.
 * In expanded state, the complete entry will be displayed.
 * Otherwise, only single line of the entry will be displayed.
 *
 * @param obj The multibuttonentry object
 * @param expanded the value of expanded state.
 *        Set this to EINA_TRUE for expanded state.
 *        Set this to EINA_FALSE for single line state.
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_expanded_set(Evas_Object *obj, Eina_Bool expanded);

/**
 * Prepend a new item to the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @param label The label of new item
 * @param func The callback function to be invoked when this item is pressed.
 * @param data The pointer to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 * @see Use elm_object_item_del() to delete the item.
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_prepend(Evas_Object *obj, const char *label, Evas_Smart_Cb func, void *data);

/**
 * Append a new item to the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @param label The label of new item
 * @param func The callback function to be invoked when this item is pressed.
 * @param data The pointer to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 * @see Use elm_object_item_del() to delete the item.
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_append(Evas_Object *obj, const char *label, Evas_Smart_Cb func, void *data);

/**
 * Add a new item to the multibuttonentry before the indicated object
 *
 * reference.
 * @param obj The multibuttonentry object
 * @param before The item before which to add it
 * @param label The label of new item
 * @param func The callback function to be invoked when this item is pressed.
 * @param data The pointer to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 * @see Use elm_object_item_del() to delete the item.
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *label, Evas_Smart_Cb func, void *data);

/**
 * Add a new item to the multibuttonentry after the indicated object
 *
 * @param obj The multibuttonentry object
 * @param after The item after which to add it
 * @param label The label of new item
 * @param func The callback function to be invoked when this item is pressed.
 * @param data The pointer to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 * @see Use elm_object_item_del() to delete the item.
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *label, Evas_Smart_Cb func, void *data);

/**
 * Get a list of items in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The list of items, or NULL if none
 *
 * @ingroup Multibuttonentry
 */
EAPI const Eina_List           *elm_multibuttonentry_items_get(const Evas_Object *obj);

/**
 * Get the first item in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The first item, or NULL if none
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_first_item_get(const Evas_Object *obj);

/**
 * Get the last item in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The last item, or NULL if none
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_last_item_get(const Evas_Object *obj);

/**
 * Get the selected item in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The selected item, or NULL if none
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_selected_item_get(const Evas_Object *obj);

/**
 * Set the selected state of an item
 *
 * @param it The item
 * @param selected if it's EINA_TRUE, select the item otherwise, unselect the item
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);


/**
 * Get the selected state of an item
 *
 * @param it The item
 * @return EINA_TRUE if the item is selected, EINA_FALSE otherwise.
 *
 * @ingroup Multibuttonentry
 */
EAPI Eina_Bool elm_multibuttonentry_item_selected_get(const Elm_Object_Item *it);

/**
 * Remove all items in the multibuttonentry.
 *
 * @param obj The multibuttonentry object
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_clear(Evas_Object *obj);

/**
 * Get the previous item in the multibuttonentry
 *
 * @param it The item
 * @return The item before the item @p it
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_prev_get(const Elm_Object_Item *it);

/**
 * Get the next item in the multibuttonentry
 *
 * @param it The item
 * @return The item after the item @p it
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_next_get(const Elm_Object_Item *it);

/**
 * Append an item filter function for text inserted in the Multibuttonentry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the Multibuttonentry, with the text to be inserted
 * as a parameter. The callback function is free to alter the text in any way
 * it wants, but it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set it text
 * parameter to NULL. This will also prevent any following filters from being
 * called.
 *
 * @param obj The multibuttonentry object
 * @param func The function to use as item filter
 * @param data User data to pass to @p func
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_item_filter_append(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * Prepend a filter function for text inserted in the Multibuttonentry
 *
 * Prepend the given callback to the list. See elm_multibuttonentry_item_filter_append()
 * for more information
 *
 * @param obj The multibuttonentry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_item_filter_prepend(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * Remove a filter from the list
 *
 * Removes the given callback from the filter list. See elm_multibuttonentry_item_filter_append()
 * for more information.
 *
 * @param obj The multibuttonentry object
 * @param func The filter function to remove
 * @param data The user data passed when adding the function
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_item_filter_remove(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * Sets if the multibuttonentry is to be editable or not.
 *
 * @param obj The multibuttonentry object
 * @param editable If EINA_TRUE, user can add/delete item in multibuttonentry, if not, the multibuttonentry is non-editable.
 *
 * @ingroup Multibuttonentry
 *
 * @since 1.7
 */
EAPI void elm_multibuttonentry_editable_set(Evas_Object *obj, Eina_Bool editable);

/**
 * Gets whether the multibuttonentry is editable or not.
 *
 * @param obj The multibuttonentry object
 * @return EINA_TRUE if the multibuttonentry is editable by the user. EINA_FALSE if not.
 *
 * @ingroup Multibuttonentry
 *
 * @since 1.7
 */
EAPI Eina_Bool elm_multibuttonentry_editable_get(const Evas_Object *obj);

/**
 * @}
 */
