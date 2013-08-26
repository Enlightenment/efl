/**
 * Set list's resizing behavior, transverse axis scrolling and items
 * cropping. See each mode's description for more details.
 *
 * @note Default value is #ELM_LIST_SCROLL.
 *
 * Values here @b don't work as bitmasks -- only one can be chosen at
 * a time.
 *
 * @see elm_list_mode_set()
 * @see elm_list_mode_get()
 *
 * @ingroup List
 */
typedef enum
{
   ELM_LIST_COMPRESS = 0, /**< The list @b won't set any of its size hints to inform how a possible container should resize it. Then, if it's not created as a "resize object", it might end with zeroed dimensions. The list will respect the container's geometry and, if any of its items won't fit into its @b transverse axis, one won't be able to scroll it in that direction. */
   ELM_LIST_SCROLL, /**< Default value. This is the same as #ELM_LIST_COMPRESS, with the exception that if any of its items won't fit into its transverse axis, one @b will be able to scroll it in that direction. */
   ELM_LIST_LIMIT, /**< Sets a minimum size hint on the list object, so that containers may respect it (and resize itself to fit the child properly). More specifically, a @b minimum size hint will be set for its @b transverse axis, so that the @b largest item in that direction fits well. This is naturally bound by the list object's maximum size hints, set externally. */
   ELM_LIST_EXPAND, /**< Besides setting a minimum size on the transverse axis, just like on #ELM_LIST_LIMIT, the list will set a minimum size on the @b longitudinal axis, trying to reserve space to all its children to be visible at a time. . This is naturally bound by the list object's maximum size hints, set externally. */
   ELM_LIST_LAST /**< Indicates error if returned by elm_list_mode_get() */
} Elm_List_Mode;

/**
 * Set the selected state of an item.
 *
 * @param it The list item
 * @param selected The selected state
 *
 * This sets the selected state of the given item @p it.
 * @c EINA_TRUE for selected, @c EINA_FALSE for not selected.
 *
 * If a new item is selected the previously selected will be unselected,
 * unless multiple selection is enabled with elm_list_multi_select_set().
 * Previously selected item can be get with function
 * elm_list_selected_item_get().
 *
 * Selected items will be highlighted.
 *
 * @see elm_list_item_selected_get()
 * @see elm_list_selected_item_get()
 * @see elm_list_multi_select_set()
 *
 * @ingroup List
 */
EAPI void                         elm_list_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/*
 * Get whether the @p item is selected or not.
 *
 * @param it The list item.
 * @return @c EINA_TRUE means item is selected. @c EINA_FALSE indicates
 * it's not. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_list_selected_item_set() for details.
 * @see elm_list_item_selected_get()
 *
 * @ingroup List
 */
EAPI Eina_Bool                    elm_list_item_selected_get(const Elm_Object_Item *it);

/**
 * Set or unset item as a separator.
 *
 * @param it The list item.
 * @param setting @c EINA_TRUE to set item @p it as separator or
 * @c EINA_FALSE to unset, i.e., item will be used as a regular item.
 *
 * Items aren't set as separator by default.
 *
 * If set as separator it will display separator theme, so won't display
 * icons or label.
 *
 * @see elm_list_item_separator_get()
 *
 * @ingroup List
 */
EAPI void                         elm_list_item_separator_set(Elm_Object_Item *it, Eina_Bool setting);

/**
 * Get a value whether item is a separator or not.
 *
 * @see elm_list_item_separator_set() for details.
 *
 * @param it The list item.
 * @return @c EINA_TRUE means item @p it is a separator. @c EINA_FALSE
 * indicates it's not. If @p it is @c NULL, @c EINA_FALSE is returned.
 *
 * @ingroup List
 */
EAPI Eina_Bool                    elm_list_item_separator_get(const Elm_Object_Item *it);

/**
 * Show @p item in the list view.
 *
 * @param it The list item to be shown.
 *
 * It won't animate list until item is visible. If such behavior is wanted,
 * use elm_list_bring_in() instead.
 *
 * @ingroup List
 */
EAPI void                         elm_list_item_show(Elm_Object_Item *it);

/**
 * Bring in the given item to list view.
 *
 * @param it The item.
 *
 * This causes list to jump to the given item @p item and show it
 * (by scrolling), if it is not fully visible.
 *
 * This may use animation to do so and take a period of time.
 *
 * If animation isn't wanted, elm_list_item_show() can be used.
 *
 * @ingroup List
 */
EAPI void                         elm_list_item_bring_in(Elm_Object_Item *it);

/**
 * Get the real Evas(Edje) object created to implement the view of a given
 * list @p item.
 *
 * @param it The list item
 * @return The base Edje object associated with @p it.
 *
 * Base object is the @c Evas_Object that represents that item.
 *
 * @ingroup List
 */
EAPI Evas_Object                 *elm_list_item_object_get(const Elm_Object_Item *it);

/**
 * Get the item before @p it in list.
 *
 * @param it The list item.
 * @return The item before @p it, or @c NULL if none or on failure.
 *
 * @note If it is the first item, @c NULL will be returned.
 *
 * @see elm_list_item_append()
 * @see elm_list_items_get()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_prev(const Elm_Object_Item *it);

/**
 * Get the item after @p it in list.
 *
 * @param it The list item.
 * @return The item after @p it, or @c NULL if none or on failure.
 *
 * @note If it is the last item, @c NULL will be returned.
 *
 * @see elm_list_item_append()
 * @see elm_list_items_get()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_next(const Elm_Object_Item *it);

