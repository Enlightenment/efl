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

