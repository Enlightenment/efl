/**
 * @brief Callback to be invoked when an item is added to the multibuttonentry.
 *
 * @param obj The parent object
 * @param item_label The label corresponding to the added item.
 * @param item_data data specific to this item.
 * @param data data specific to the multibuttonentry.
 *
 * @return @c EINA_TRUE
 *         @c EINA_FALSE otherwise.
 *
 * @ingroup Multibuttonentry
 */
typedef Eina_Bool                   (*Elm_Multibuttonentry_Item_Filter_Cb)(Evas_Object *obj, const char *item_label, void *item_data, void *data);

/**
 * @typedef Elm_Multibuttonentry_Format_Cb
 *
 * This callback type is used to format the string that will be used
 * to display the hidden items counter, when not in expanded mode.
 *
 * @param count Number of hidden items
 * @param data The (context) data passed in to
 * elm_multibuttonentry_format_function_set()
 * @return String representing the counter that will be set to
 * multibuttonentry's counter item's text.
 *
 * @see elm_multibuttonentry_format_function_set()
 * @see elm_multibuttonentry_expanded_set()
 *
 * @ingroup Multibuttonentry
 */
typedef char * (*Elm_Multibuttonentry_Format_Cb)(int count, void *data);
