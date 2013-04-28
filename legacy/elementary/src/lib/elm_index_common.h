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

