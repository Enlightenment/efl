/**
 * @defgroup Multibuttonentry Multibuttonentry
 *
 * A Multibuttonentry is a widget to allow a user enter text and manage it as a number of buttons
 * Each text button is inserted by pressing the "return" key. If there is no space in the current row,
 * a new button is added to the next row. When a text button is pressed, it will become focused.
 * Backspace removes the focus.
 * When the Multibuttonentry loses focus items longer than 1 lines are shrunk to one line.
 *
 * Smart callbacks one can register:
 * - @c "item,selected" - when item is selected. May be called on backspace key.
 * - @c "item,added" - when a new multibuttonentry item is added.
 * - @c "item,deleted" - when a multibuttonentry item is deleted.
 * - @c "item,clicked" - selected item of multibuttonentry is clicked.
 * - @c "clicked" - when multibuttonentry is clicked.
 * - @c "focused" - when multibuttonentry is focused.
 * - @c "unfocused" - when multibuttonentry is unfocused.
 * - @c "expanded" - when multibuttonentry is expanded.
 * - @c "shrank" - when multibuttonentry is shrank.
 * - @c "shrank,state,changed" - when shrink mode state of multibuttonentry is changed.
 */

/**
 * @addtogroup Multibuttonentry
 * @{
 */

typedef struct _Multibuttonentry_Item Elm_Multibuttonentry_Item;
typedef Eina_Bool                   (*Elm_Multibuttonentry_Item_Filter_callback)(Evas_Object *obj, const char *item_label, void *item_data, void *data);

/**
 * @brief Add a new multibuttonentry to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 */
EAPI Evas_Object *
                                elm_multibuttonentry_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * Get the label
 *
 * @param obj The multibuttonentry object
 * @return The label, or NULL if none
 *
 */
EAPI const char                *elm_multibuttonentry_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the label
 *
 * @param obj The multibuttonentry object
 * @param label The text label string
 *
 */
EAPI void                       elm_multibuttonentry_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

/**
 * Get the entry of the multibuttonentry object
 *
 * @param obj The multibuttonentry object
 * @return The entry object, or NULL if none
 *
 */
EAPI Evas_Object               *elm_multibuttonentry_entry_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the guide text
 *
 * @param obj The multibuttonentry object
 * @return The guide text, or NULL if none
 *
 */
EAPI const char                *elm_multibuttonentry_guide_text_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the guide text
 *
 * @param obj The multibuttonentry object
 * @param guidetext The guide text string
 *
 */
EAPI void                       elm_multibuttonentry_guide_text_set(Evas_Object *obj, const char *guidetext) EINA_ARG_NONNULL(1);

/**
 * Get the value of shrink_mode state.
 *
 * @param obj The multibuttonentry object
 * @return the value of shrink mode state.
 *
 */
EAPI int                        elm_multibuttonentry_shrink_mode_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set/Unset the multibuttonentry to shrink mode state of single line
 *
 * @param obj The multibuttonentry object
 * @param shrink the value of shrink_mode state. set this to 1 to set the multibuttonentry to shrink state of single line. set this to 0 to unset the contracted state.
 *
 */
EAPI void                       elm_multibuttonentry_shrink_mode_set(Evas_Object *obj, int shrink) EINA_ARG_NONNULL(1);

/**
 * Prepend a new item to the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @param label The label of new item
 * @param data The ponter to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_item_prepend(Evas_Object *obj, const char *label, void *data) EINA_ARG_NONNULL(1);

/**
 * Append a new item to the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @param label The label of new item
 * @param data The ponter to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_item_append(Evas_Object *obj, const char *label, void *data) EINA_ARG_NONNULL(1);

/**
 * Add a new item to the multibuttonentry before the indicated object
 *
 * reference.
 * @param obj The multibuttonentry object
 * @param before The item before which to add it
 * @param label The label of new item
 * @param data The ponter to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_item_insert_before(Evas_Object *obj, Elm_Multibuttonentry_Item *before, const char *label, void *data) EINA_ARG_NONNULL(1);

/**
 * Add a new item to the multibuttonentry after the indicated object
 *
 * @param obj The multibuttonentry object
 * @param after The item after which to add it
 * @param label The label of new item
 * @param data The ponter to the data to be attached
 * @return A handle to the item added or NULL if not possible
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_item_insert_after(Evas_Object *obj, Elm_Multibuttonentry_Item *after, const char *label, void *data) EINA_ARG_NONNULL(1);

/**
 * Get a list of items in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The list of items, or NULL if none
 *
 */
EAPI const Eina_List           *elm_multibuttonentry_items_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the first item in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The first item, or NULL if none
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_first_item_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the last item in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The last item, or NULL if none
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_last_item_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the selected item in the multibuttonentry
 *
 * @param obj The multibuttonentry object
 * @return The selected item, or NULL if none
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_selected_item_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the selected state of an item
 *
 * @param item The item
 * @param selected if it's EINA_TRUE, select the item otherwise, unselect the item
 *
 */
EAPI void                       elm_multibuttonentry_item_select(Elm_Multibuttonentry_Item *item, Eina_Bool selected) EINA_ARG_NONNULL(1);

/**
 * unselect all items.
 *
 * @param obj The multibuttonentry object
 *
 */
EAPI void                       elm_multibuttonentry_item_unselect_all(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Delete a given item
 *
 * @param item The item
 *
 */
EAPI void                       elm_multibuttonentry_item_del(Elm_Multibuttonentry_Item *item) EINA_ARG_NONNULL(1);

/**
 * Remove all items in the multibuttonentry.
 *
 * @param obj The multibuttonentry object
 *
 */
EAPI void                       elm_multibuttonentry_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Get the label of a given item
 *
 * @param item The item
 * @return The label of a given item, or NULL if none
 *
 */
EAPI const char                *elm_multibuttonentry_item_label_get(const Elm_Multibuttonentry_Item *item) EINA_ARG_NONNULL(1);

/**
 * Set the label of a given item
 *
 * @param item The item
 * @param str The text label string
 *
 */
EAPI void                       elm_multibuttonentry_item_label_set(Elm_Multibuttonentry_Item *item, const char *str) EINA_ARG_NONNULL(1);

/**
 * Get the previous item in the multibuttonentry
 *
 * @param item The item
 * @return The item before the item @p item
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_item_prev_get(const Elm_Multibuttonentry_Item *item) EINA_ARG_NONNULL(1);

/**
 * Get the next item in the multibuttonentry
 *
 * @param item The item
 * @return The item after the item @p item
 *
 */
EAPI Elm_Multibuttonentry_Item *elm_multibuttonentry_item_next_get(const Elm_Multibuttonentry_Item *item) EINA_ARG_NONNULL(1);

/**
 * Append a item filter function for text inserted in the Multibuttonentry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the Multibuttonentry, with the text to be inserted
 * as a parameter. The callback function is free to alter the text in any way
 * it wants, but it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set it text
 * parameter to NULL. This will also prevent any following filters from being
 * called.
 *
 * @param obj The multibuttonentryentry object
 * @param func The function to use as item filter
 * @param data User data to pass to @p func
 *
 */
EAPI void                       elm_multibuttonentry_item_filter_append(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_callback func, void *data) EINA_ARG_NONNULL(1);

/**
 * Prepend a filter function for text inserted in the Multibuttentry
 *
 * Prepend the given callback to the list. See elm_multibuttonentry_item_filter_append()
 * for more information
 *
 * @param obj The multibuttonentry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 */
EAPI void                       elm_multibuttonentry_item_filter_prepend(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_callback func, void *data) EINA_ARG_NONNULL(1);

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
 */
EAPI void                       elm_multibuttonentry_item_filter_remove(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_callback func, void *data) EINA_ARG_NONNULL(1);

/**
 * @}
 */
