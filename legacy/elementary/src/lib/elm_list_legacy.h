/**
 * Add a new list widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new list widget handle or @c NULL, on errors.
 *
 * This function inserts a new list widget on the canvas.
 *
 * @ingroup List
 */
EAPI Evas_Object                 *elm_list_add(Evas_Object *parent);

/**
 * Starts the list.
 *
 * @param obj The list object
 *
 * @note Call before running show() on the list object.
 * @warning If not called, it won't display the list properly.
 *
 * @code
 * li = elm_list_add(win);
 * elm_list_item_append(li, "First", NULL, NULL, NULL, NULL);
 * elm_list_item_append(li, "Second", NULL, NULL, NULL, NULL);
 * elm_list_go(li);
 * evas_object_show(li);
 * @endcode
 *
 * @ingroup List
 */
EAPI void                         elm_list_go(Evas_Object *obj);

/**
 * Enable or disable multiple items selection on the list object.
 *
 * @param obj The list object
 * @param multi @c EINA_TRUE to enable multi selection or @c EINA_FALSE to
 * disable it.
 *
 * Disabled by default. If disabled, the user can select a single item of
 * the list each time. Selected items are highlighted on list.
 * If enabled, many items can be selected.
 *
 * If a selected item is selected again, it will be unselected.
 *
 * @see elm_list_multi_select_get()
 *
 * @ingroup List
 */
EAPI void                         elm_list_multi_select_set(Evas_Object *obj, Eina_Bool multi);

/**
 * Get a value whether multiple items selection is enabled or not.
 *
 * @see elm_list_multi_select_set() for details.
 *
 * @param obj The list object.
 * @return @c EINA_TRUE means multiple items selection is enabled.
 * @c EINA_FALSE indicates it's disabled. If @p obj is @c NULL,
 * @c EINA_FALSE is returned.
 *
 * @ingroup List
 */
EAPI Eina_Bool                    elm_list_multi_select_get(const Evas_Object *obj);

/**
 * Set which mode to use for the list object.
 *
 * @param obj The list object
 * @param mode One of #Elm_List_Mode: #ELM_LIST_COMPRESS,
 * #ELM_LIST_SCROLL, #ELM_LIST_LIMIT or #ELM_LIST_EXPAND.
 *
 * Set list's resize behavior, transverse axis scroll and
 * items cropping. See each mode's description for more details.
 *
 * @note Default value is #ELM_LIST_SCROLL.
 *
 * Only one mode at a time can be set. If a previous one was set, it
 * will be changed by the new mode after this call. Bitmasks won't
 * work here as well.
 *
 * @warning This function's behavior will clash with those of
 * elm_scroller_content_min_limit(), so use either one of them, but
 * not both.
 *
 * @see elm_list_mode_get()
 *
 * @ingroup List
 */
EAPI void                         elm_list_mode_set(Evas_Object *obj, Elm_List_Mode mode);

/**
 * Get the mode the list is at.
 *
 * @param obj The list object
 * @return One of #Elm_List_Mode: #ELM_LIST_COMPRESS, #ELM_LIST_SCROLL, #ELM_LIST_LIMIT, #ELM_LIST_EXPAND or #ELM_LIST_LAST on errors.
 *
 * @note see elm_list_mode_set() for more information.
 *
 * @ingroup List
 */
EAPI Elm_List_Mode                elm_list_mode_get(const Evas_Object *obj);

/**
 * Enable or disable horizontal mode on the list object.
 *
 * @param obj The list object.
 * @param horizontal @c EINA_TRUE to enable horizontal or @c EINA_FALSE to
 * disable it, i.e., to enable vertical mode.
 *
 * @note Vertical mode is set by default.
 *
 * On horizontal mode items are displayed on list from left to right,
 * instead of from top to bottom. Also, the list will scroll horizontally.
 * Each item will presents left icon on top and right icon, or end, at
 * the bottom.
 *
 * @see elm_list_horizontal_get()
 *
 * @ingroup List
 */
EAPI void                         elm_list_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Get a value whether horizontal mode is enabled or not.
 *
 * @param obj The list object.
 * @return @c EINA_TRUE means horizontal mode selection is enabled.
 * @c EINA_FALSE indicates it's disabled. If @p obj is @c NULL,
 * @c EINA_FALSE is returned.
 *
 * @see elm_list_horizontal_set() for details.
 *
 * @ingroup List
 */
EAPI Eina_Bool                    elm_list_horizontal_get(const Evas_Object *obj);

/**
 * Set the list select mode.
 *
 * @param obj The list object
 * @param mode The select mode
 *
 * elm_list_select_mode_set() changes item select mode in the list widget.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : Items will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS :  This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select items
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 *
 * @see elm_list_select_mode_get()
 *
 * @ingroup List
 */
EAPI void                         elm_list_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode);

/**
 * Get the list select mode.
 *
 * @param obj The list object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_list_select_mode_set()
 *
 * @ingroup List
 */
EAPI Elm_Object_Select_Mode       elm_list_select_mode_get(const Evas_Object *obj);

/**
 * Append a new item to the list object.
 *
 * @param obj The list object.
 * @param label The label of the list item.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param end The icon object to use for the right side of the item. An
 * icon can be any Evas object.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 *
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and appended to the list, i.e., will
 * be set as @b last item.
 *
 * Items created with this method can be deleted with elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @note Remember that the label parameter is copied inside the list. So if an
 * application dynamically allocated the label, the application should free the
 * label after this call.
 *
 * Simple example (with no function callback or data associated):
 * @code
 * li = elm_list_add(win);
 * ic = elm_icon_add(win);
 * elm_image_file_set(ic, "path/to/image", NULL);
 * elm_icon_resizable_set(ic, EINA_TRUE, EINA_TRUE);
 * elm_list_item_append(li, "label", ic, NULL, NULL, NULL);
 * elm_list_go(li);
 * evas_object_show(li);
 * @endcode
 *
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_object_item_del_cb_set()
 * @see elm_list_clear()
 * @see elm_icon_add()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);

/**
 * Prepend a new item to the list object.
 *
 * @param obj The list object.
 * @param label The label of the list item.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param end The icon object to use for the right side of the item. An
 * icon can be any Evas object.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 *
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and prepended to the list, i.e., will
 * be set as @b first item.
 *
 * Items created with this method can be deleted with elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @note Remember that the label parameter is copied inside the list. So if an
 * application dynamically allocated the label, the application should free the
 * label after this call.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_object_item_del_cb_set()
 * @see elm_list_clear()
 * @see elm_icon_add()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_prepend(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the list object before item @p before.
 *
 * @param obj The list object.
 * @param before The list item to insert before.
 * @param label The label of the list item.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param end The icon object to use for the right side of the item. An
 * icon can be any Evas object.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 *
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and added to the list. Its position in
 * this list will be just before item @p before.
 *
 * Items created with this method can be deleted with elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @note Remember that the label parameter is copied inside the list. So if an
 * application dynamically allocated the label, the application should free the
 * label after this call.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_object_item_del_cb_set()
 * @see elm_list_clear()
 * @see elm_icon_add()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the list object after item @p after.
 *
 * @param obj The list object.
 * @param after The list item to insert after.
 * @param label The label of the list item.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param end The icon object to use for the right side of the item. An
 * icon can be any Evas object.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 *
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and added to the list. Its position in
 * this list will be just after item @p after.
 *
 * Items created with this method can be deleted with elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @note Remember that the label parameter is copied inside the list. So if an
 * application dynamically allocated the label, the application should free the
 * label after this call.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_object_item_del_cb_set()
 * @see elm_list_clear()
 * @see elm_icon_add()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the sorted list object.
 *
 * @param obj The list object.
 * @param label The label of the list item.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param end The icon object to use for the right side of the item. An
 * icon can be any Evas object.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @param cmp_func The comparing function to be used to sort list
 * items <b>by #Elm_Object_Item item handles</b>. This function will
 * receive two items and compare them, returning a non-negative integer
 * if the second item should be place after the first, or negative value
 * if should be placed before.
 *
 * @return The created item or @c NULL upon failure.
 *
 * @note This function inserts values into a list object assuming it was
 * sorted and the result will be sorted.
 *
 * A new item will be created and added to the list. Its position in
 * this list will be found comparing the new item with previously inserted
 * items using function @p cmp_func.
 *
 * Items created with this method can be deleted with elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @note Remember that the label parameter is copied inside the list. So if an
 * application dynamically allocated the label, the application should free the
 * label after this call.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_object_item_del_cb_set()
 * @see elm_list_clear()
 * @see elm_icon_add()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_item_sorted_insert(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func);

/**
 * Remove all list's items.
 *
 * @param obj The list object
 *
 * @see elm_object_item_del()
 * @see elm_list_item_append()
 *
 * @ingroup List
 */
EAPI void                         elm_list_clear(Evas_Object *obj);

/**
 * Get a list of all the list items.
 *
 * @param obj The list object
 * @return An @c Eina_List of list items, #Elm_Object_Item,
 * or @c NULL on failure.
 *
 * @see elm_list_item_append()
 * @see elm_object_item_del()
 * @see elm_list_clear()
 *
 * @ingroup List
 */
EAPI const Eina_List             *elm_list_items_get(const Evas_Object *obj);

/**
 * Get the selected item.
 *
 * @param obj The list object.
 * @return The selected list item.
 *
 * The selected item can be unselected with function
 * elm_list_item_selected_set().
 *
 * The selected item always will be highlighted on list.
 *
 * @see elm_list_selected_items_get()
 *
 * @ingroup List
 */
EAPI Elm_Object_Item               *elm_list_selected_item_get(const Evas_Object *obj);

/**
 * Return a list of the currently selected list items.
 *
 * @param obj The list object.
 * @return An @c Eina_List of list items, #Elm_Object_Item,
 * or @c NULL on failure.
 *
 * Multiple items can be selected if multi select is enabled. It can be
 * done with elm_list_multi_select_set().
 *
 * @see elm_list_selected_item_get()
 * @see elm_list_multi_select_set()
 *
 * @ingroup List
 */
EAPI const Eina_List             *elm_list_selected_items_get(const Evas_Object *obj);

/**
 * Get the first item in the list
 *
 * This returns the first item in the list.
 *
 * @param obj The list object
 * @return The first item, or NULL if none
 *
 * @ingroup List
 */
EAPI Elm_Object_Item             *elm_list_first_item_get(const Evas_Object *obj);

/**
 * Get the last item in the list
 *
 * This returns the last item in the list.
 *
 * @return The last item, or NULL if none
 *
 * @ingroup List
 */
EAPI Elm_Object_Item             *elm_list_last_item_get(const Evas_Object *obj);
