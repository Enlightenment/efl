/**
 * @defgroup List List
 * @ingroup Elementary
 *
 * @image html img/widget/list/preview-00.png
 * @image latex img/widget/list/preview-00.eps width=\textwidth
 *
 * @image html img/list.png
 * @image latex img/list.eps width=\textwidth
 *
 * A list widget is a container whose children are displayed vertically or
 * horizontally, in order, and can be selected.
 * The list can accept only one or multiple items selection. Also has many
 * modes of items displaying.
 *
 * A list is a very simple type of list widget.  For more robust
 * lists, @ref Genlist should probably be used.
 *
 * Smart callbacks one can listen to:
 * - @c "activated" - The user has double-clicked or pressed
 *   (enter|return|spacebar) on an item. The @c event_info parameter
 *   is the item that was activated.
 * - @c "clicked,double" - The user has double-clicked an item.
 *   The @c event_info parameter is the item that was double-clicked.
 * - "selected" - when the user selected an item
 * - "unselected" - when the user unselected an item
 * - "longpressed" - an item in the list is long-pressed
 * - "edge,top" - the list is scrolled until the top edge
 * - "edge,bottom" - the list is scrolled until the bottom edge
 * - "edge,left" - the list is scrolled until the left edge
 * - "edge,right" - the list is scrolled until the right edge
 * - "language,changed" - the program's language changed
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the list items that you can use for are:
 * @li "start" - A start position object in the list item
 * @li "end" - A end position object in the list item
 *
 * Default text parts of the list items that you can use for are:
 * @li "default" - label in the list item
 *
 * Supported elm_object_item common APIs.
 * @li elm_object_item_disabled_set
 * @li elm_object_item_disabled_get
 * @li elm_object_item_part_text_set
 * @li elm_object_item_part_text_get
 * @li elm_object_item_part_content_set
 * @li elm_object_item_part_content_get
 * @li elm_object_item_part_content_unset
 *
 * List of examples:
 * @li @ref list_example_01
 * @li @ref list_example_02
 * @li @ref list_example_03
 */

/**
 * @addtogroup List
 * @{
 */

/**
 * @enum _Elm_List_Mode
 * @typedef Elm_List_Mode
 *
 * Set list's resize behavior, transverse axis scroll and
 * items cropping. See each mode's description for more details.
 *
 * @note Default value is #ELM_LIST_SCROLL.
 *
 * Values <b> don't </b> work as bitmask, only one can be chosen.
 *
 * @see elm_list_mode_set()
 * @see elm_list_mode_get()
 *
 * @ingroup List
 */
typedef enum
{
   ELM_LIST_COMPRESS = 0, /**< Won't set any of its size hints to inform how a possible container should resize it. Then, if it's not created as a "resize object", it might end with zero dimensions. The list will respect the container's geometry and, if any of its items won't fit into its transverse axis, one won't be able to scroll it in that direction. */
   ELM_LIST_SCROLL, /**< Default value. Won't set any of its size hints to inform how a possible container should resize it. Then, if it's not created as a "resize object", it might end with zero dimensions. The list will respect the container's geometry and, if any of its items won't fit into its transverse axis, one will be able to scroll it in that direction (large items will get cropped). */
   ELM_LIST_LIMIT, /**< Set a minimum size hint on the list object, so that containers may respect it (and resize itself to fit the child properly). More specifically, a minimum size hint will be set for its transverse axis, so that the @b largest item in that direction fits well. Can have effects bounded by setting the list object's maximum size hints. */
   ELM_LIST_EXPAND, /**< Besides setting a minimum size on the transverse axis, just like the previous mode, will set a minimum size on the longitudinal axis too, trying to reserve space to all its children to be visible at a time. Can have effects bounded by setting the list object's maximum size hints. */
   ELM_LIST_LAST /**< Indicates error if returned by elm_list_mode_get() */
} Elm_List_Mode;

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
 * @param mode One of #Elm_List_Mode: #ELM_LIST_COMPRESS, #ELM_LIST_SCROLL,
 * #ELM_LIST_LIMIT or #ELM_LIST_EXPAND.
 *
 * Set list's resize behavior, transverse axis scroll and
 * items cropping. See each mode's description for more details.
 *
 * @note Default value is #ELM_LIST_SCROLL.
 *
 * Only one can be set, if a previous one was set, it will be changed
 * by the new mode set. Bitmask won't work as well.
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
 * @return One of #Elm_List_Mode: #ELM_LIST_COMPRESS, #ELM_LIST_SCROLL,
 * #ELM_LIST_LIMIT, #ELM_LIST_EXPAND or #ELM_LIST_LAST on errors.
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
EAPI void
elm_list_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode);

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
EAPI Elm_Object_Select_Mode
elm_list_select_mode_get(const Evas_Object *obj);

/**
 * Set bouncing behaviour when the scrolled content reaches an edge.
 *
 * Tell the internal scroller object whether it should bounce or not
 * when it reaches the respective edges for each axis.
 *
 * @param obj The list object
 * @param h_bounce Whether to bounce or not in the horizontal axis.
 * @param v_bounce Whether to bounce or not in the vertical axis.
 *
 * @see elm_scroller_bounce_set()
 *
 * @ingroup List
 */
EAPI void                         elm_list_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get the bouncing behaviour of the internal scroller.
 *
 * Get whether the internal scroller should bounce when the edge of each
 * axis is reached scrolling.
 *
 * @param obj The list object.
 * @param h_bounce Pointer to store the bounce state of the horizontal
 * axis.
 * @param v_bounce Pointer to store the bounce state of the vertical
 * axis.
 *
 * @see elm_scroller_bounce_get()
 * @see elm_list_bounce_set()
 *
 * @ingroup List
 */
EAPI void                         elm_list_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Set the scrollbar policy.
 *
 * @param obj The list object
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given scroller.
 * #ELM_SCROLLER_POLICY_AUTO means the scrollbar is made visible if it
 * is needed, and otherwise kept hidden. #ELM_SCROLLER_POLICY_ON turns
 * it on all the time, and #ELM_SCROLLER_POLICY_OFF always keeps it off.
 * This applies respectively for the horizontal and vertical scrollbars.
 *
 * The both are disabled by default, i.e., are set to
 * #ELM_SCROLLER_POLICY_OFF.
 *
 * @ingroup List
 */
EAPI void                         elm_list_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Get the scrollbar policy.
 *
 * @see elm_list_scroller_policy_get() for details.
 *
 * @param obj The list object.
 * @param policy_h Pointer to store horizontal scrollbar policy.
 * @param policy_v Pointer to store vertical scrollbar policy.
 *
 * @ingroup List
 */
EAPI void                         elm_list_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

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
 * callback function is set with elm_widget_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Simple example (with no function callback or data associated):
 * @code
 * li = elm_list_add(win);
 * ic = elm_icon_add(win);
 * elm_icon_file_set(ic, "path/to/image", NULL);
 * elm_icon_resizable_set(ic, EINA_TRUE, EINA_TRUE);
 * elm_list_item_append(li, "label", ic, NULL, NULL, NULL);
 * elm_list_go(li);
 * evas_object_show(li);
 * @endcode
 *
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_widget_item_del_cb_set()
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
 * callback function is set with elm_widget_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_widget_item_del_cb_set()
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
 * callback function is set with elm_widget_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_widget_item_del_cb_set()
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
 * callback function is set with elm_widget_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_widget_item_del_cb_set()
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
 * callback function is set with elm_widget_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If always select is enabled it will call this function every time
 * user clicks over an item (already selected or not).
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * @see elm_list_item_append() for a simple code example.
 * @see elm_list_select_mode_set()
 * @see elm_object_item_del()
 * @see elm_widget_item_del_cb_set()
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
 * Gets the base object of the item.
 *
 * @param it The list item
 * @return The base object associated with @p item
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

/**
 * @}
 */
