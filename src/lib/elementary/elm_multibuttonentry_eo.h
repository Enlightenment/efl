#ifndef _ELM_MULTIBUTTONENTRY_EO_H_
#define _ELM_MULTIBUTTONENTRY_EO_H_

#ifndef _ELM_MULTIBUTTONENTRY_EO_CLASS_TYPE
#define _ELM_MULTIBUTTONENTRY_EO_CLASS_TYPE

typedef Eo Elm_Multibuttonentry;

#endif

#ifndef _ELM_MULTIBUTTONENTRY_EO_TYPES
#define _ELM_MULTIBUTTONENTRY_EO_TYPES


#endif
/** Elementary multibuttonentry class
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_CLASS elm_multibuttonentry_class_get()

EWAPI const Efl_Class *elm_multibuttonentry_class_get(void);

/**
 * @brief Control if the multibuttonentry is to be editable or not.
 *
 * @param[in] obj The object.
 * @param[in] editable If @c true, user can add/delete item in
 * multibuttonentry, if not, the multibuttonentry is non-editable.
 *
 * @since 1.7
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_editable_set(Eo *obj, Eina_Bool editable);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Eina_Bool elm_obj_multibuttonentry_editable_get(const Eo *obj);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_expanded_set(Eo *obj, Eina_Bool expanded);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Eina_Bool elm_obj_multibuttonentry_expanded_get(const Eo *obj);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_format_function_set(Eo *obj, Elm_Multibuttonentry_Format_Cb format_function, const void *data);

/**
 * @brief Get a list of items in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The list of items, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI const Eina_List *elm_obj_multibuttonentry_items_get(const Eo *obj);

/**
 * @brief Get the first item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The first item, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_first_item_get(const Eo *obj);

/**
 * @brief Get the last item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The last item, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_last_item_get(const Eo *obj);

/**
 * @brief Get the entry of the multibuttonentry object
 *
 * @param[in] obj The object.
 *
 * @return The entry object, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Efl_Canvas_Object *elm_obj_multibuttonentry_entry_get(const Eo *obj);

/**
 * @brief Get the selected item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The selected item, or NULL if none
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_selected_item_get(const Eo *obj);

/**
 * @brief Prepend a new item to the multibuttonentry
 *
 * See @ref efl_invalidate to delete the item.
 *
 * @param[in] obj The object.
 * @param[in] label The label of new item
 * @param[in] func The callback function to be invoked when this item is
 * pressed.
 * @param[in] data The pointer to the data to be attached
 *
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_item_prepend(Eo *obj, const char *label, Evas_Smart_Cb func, void *data);

/** Remove all items in the multibuttonentry.
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_clear(Eo *obj);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_item_filter_remove(Eo *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * @brief Add a new item to the multibuttonentry before the indicated object
 * reference.
 *
 * See @ref efl_invalidate to delete the item.
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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_item_insert_before(Eo *obj, Elm_Widget_Item *before, const char *label, Evas_Smart_Cb func, void *data);

/**
 * @brief Append a new item to the multibuttonentry
 *
 * See @ref efl_invalidate to delete the item.
 *
 * @param[in] obj The object.
 * @param[in] label The label of new item
 * @param[in] func The callback function to be invoked when this item is
 * pressed.
 * @param[in] data The pointer to the data to be attached
 *
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_item_append(Eo *obj, const char *label, Evas_Smart_Cb func, void *data);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_item_filter_prepend(Eo *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI void elm_obj_multibuttonentry_item_filter_append(Eo *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data);

/**
 * @brief Add a new item to the multibuttonentry after the indicated object
 *
 * See @ref efl_invalidate to delete the item.
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
 * @ingroup Elm_Multibuttonentry
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_item_insert_after(Eo *obj, Elm_Widget_Item *after, const char *label, Evas_Smart_Cb func, void *data);

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_SELECTED;

/** Called when item was selected
 * @return Efl_Object *
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_ITEM_SELECTED (&(_ELM_MULTIBUTTONENTRY_EVENT_ITEM_SELECTED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_ADDED;

/** Called when item was added
 * @return Efl_Object *
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_ITEM_ADDED (&(_ELM_MULTIBUTTONENTRY_EVENT_ITEM_ADDED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_DELETED;

/** Called when item was deleted
 * @return Efl_Object *
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_ITEM_DELETED (&(_ELM_MULTIBUTTONENTRY_EVENT_ITEM_DELETED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_CLICKED;

/** Called when item was clicked
 * @return Efl_Object *
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_ITEM_CLICKED (&(_ELM_MULTIBUTTONENTRY_EVENT_ITEM_CLICKED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_ITEM_LONGPRESSED;

/** Called when item got a longpress
 * @return Efl_Object *
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_ITEM_LONGPRESSED (&(_ELM_MULTIBUTTONENTRY_EVENT_ITEM_LONGPRESSED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_EXPANDED;

/** Called when expanded
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_EXPANDED (&(_ELM_MULTIBUTTONENTRY_EVENT_EXPANDED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_CONTRACTED;

/** Called when contracted
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_CONTRACTED (&(_ELM_MULTIBUTTONENTRY_EVENT_CONTRACTED))

EWAPI extern const Efl_Event_Description _ELM_MULTIBUTTONENTRY_EVENT_EXPAND_STATE_CHANGED;

/** Called when expanded state changed
 * @return int
 *
 * @ingroup Elm_Multibuttonentry
 */
#define ELM_MULTIBUTTONENTRY_EVENT_EXPAND_STATE_CHANGED (&(_ELM_MULTIBUTTONENTRY_EVENT_EXPAND_STATE_CHANGED))

#endif
