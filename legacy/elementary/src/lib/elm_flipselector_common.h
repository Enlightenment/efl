/**
 * Set whether a given flip selector widget's item should be the
 * currently selected one.
 *
 * @param it The flip selector item
 * @param selected @c EINA_TRUE to select it, @c EINA_FALSE to unselect.
 *
 * This sets whether @p item is or not the selected (thus, under
 * display) one. If @p item is different than the one under display,
 * the latter will be unselected. If the @p item is set to be
 * unselected, on the other hand, the @b first item in the widget's
 * internal members list will be the new selected one.
 *
 * @see elm_flipselector_item_selected_get()
 *
 * @ingroup Flipselector
 */
EAPI void                        elm_flipselector_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * Get whether a given flip selector widget's item is the currently
 * selected one.
 *
 * @param it The flip selector item
 * @return @c EINA_TRUE, if it's selected, @c EINA_FALSE otherwise
 * (or on errors).
 *
 * @see elm_flipselector_item_selected_set()
 *
 * @ingroup Flipselector
 */
EAPI Eina_Bool  elm_flipselector_item_selected_get(const Elm_Object_Item *it);

/**
 * Get the item before @p item in a flip selector widget's internal list of
 * items.
 *
 * @param it The item to fetch previous from
 * @return The item before the @p item, in its parent's list. If there is no
 *         previous item for @p item or there's an error, @c NULL is returned.
 *
 * @see elm_flipselector_item_next_get()
 *
 * @ingroup Flipselector
 */
EAPI Elm_Object_Item *elm_flipselector_item_prev_get(const Elm_Object_Item *it);

/**
 * Get the item after @p item in a flip selector widget's
 * internal list of items.
 *
 * @param it The item to fetch next from
 * @return The item after the @p item, in its parent's list. If there is no next
 *         item for @p item or there's an error, @c NULL is returned.
 *
 * @see elm_flipselector_item_prev_get()
 *
 * @ingroup Flipselector
 */
EAPI Elm_Object_Item            *elm_flipselector_item_next_get(const Elm_Object_Item *it);

