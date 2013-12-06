/**
 * Add a new toolbar widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new toolbar widget handle or @c NULL, on errors.
 *
 * This function inserts a new toolbar widget on the canvas.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_add(Evas_Object *parent);

/**
 * Set the icon size, in pixels, to be used by toolbar items.
 *
 * @param obj The toolbar object
 * @param icon_size The icon size in pixels
 *
 * @note Default value is @c 32. It reads value from elm config.
 *
 * @see elm_toolbar_icon_size_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_icon_size_set(Evas_Object *obj, int icon_size);

/**
 * Get the icon size, in pixels, to be used by toolbar items.
 *
 * @param obj The toolbar object.
 * @return The icon size in pixels.
 *
 * @see elm_toolbar_icon_size_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI int                          elm_toolbar_icon_size_get(const Evas_Object *obj);

/**
 * Sets icon lookup order, for toolbar items' icons.
 *
 * @param obj The toolbar object.
 * @param order The icon lookup order.
 *
 * Icons added before calling this function will not be affected.
 * The default lookup order is #ELM_ICON_LOOKUP_THEME_FDO.
 *
 * @see elm_toolbar_icon_order_lookup_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_icon_order_lookup_set(Evas_Object *obj, Elm_Icon_Lookup_Order order);

/**
 * Get the icon lookup order.
 *
 * @param obj The toolbar object.
 * @return The icon lookup order.
 *
 * @see elm_toolbar_icon_order_lookup_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Icon_Lookup_Order        elm_toolbar_icon_order_lookup_get(const Evas_Object *obj);

/**
 * Append item to the toolbar.
 *
 * @param obj The toolbar object.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and appended to the toolbar, i.e., will
 * be set as @b last item.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_append(Evas_Object *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Prepend item to the toolbar.
 *
 * @param obj The toolbar object.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and prepended to the toolbar, i.e., will
 * be set as @b first item.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_prepend(Evas_Object *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the toolbar object before item @p before.
 *
 * @param obj The toolbar object.
 * @param before The toolbar item to insert before.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and added to the toolbar. Its position in
 * this toolbar will be just before item @p before.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the toolbar object after item @p after.
 *
 * @param obj The toolbar object.
 * @param after The toolbar item to insert after.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and added to the toolbar. Its position in
 * this toolbar will be just after item @p after.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Get the first item in the given toolbar widget's list of
 * items.
 *
 * @param obj The toolbar object
 * @return The first item or @c NULL, if it has no items (and on
 * errors)
 *
 * @see elm_toolbar_item_append()
 * @see elm_toolbar_last_item_get()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_first_item_get(const Evas_Object *obj);

/**
 * Get the last item in the given toolbar widget's list of
 * items.
 *
 * @param obj The toolbar object
 * @return The last item or @c NULL, if it has no items (and on
 * errors)
 *
 * @see elm_toolbar_item_prepend()
 * @see elm_toolbar_first_item_get()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_last_item_get(const Evas_Object *obj);

/**
 * Returns a pointer to a toolbar item by its label.
 *
 * @param obj The toolbar object.
 * @param label The label of the item to find.
 *
 * @return The pointer to the toolbar item matching @p label or @c NULL
 * on failure.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_find_by_label(const Evas_Object *obj, const char *label);

/**
 * Get the selected item.
 *
 * @param obj The toolbar object.
 * @return The selected toolbar item.
 *
 * The selected item can be unselected with function
 * elm_toolbar_item_selected_set().
 *
 * The selected item always will be highlighted on toolbar.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_selected_item_get(const Evas_Object *obj);

/**
 * Get the more item which is auto-generated by toolbar.
 *
 * @param obj The toolbar object.
 * @return The toolbar more item.
 *
 * Toolbar generates 'more' item when there is no more space to fit items in
 * and toolbar is in ELM_TOOLBAR_SHRINK_MENU or ELM_TOOLBAR_SHRINK_EXPAND mode.
 * The more item can be manipulated by elm_object_item_text_set() and
 * elm_object_item_content_set.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_more_item_get(const Evas_Object *obj);

/**
 * Set the item displaying mode of a given toolbar widget @p obj.
 *
 * @param obj The toolbar object handle
 * @param shrink_mode Toolbar's items display behavior
 *
 * The toolbar won't scroll under #ELM_TOOLBAR_SHRINK_NONE mode, but
 * it will enforce a minimum size, so that all the items will fit
 * inside it. It won't scroll and won't show the items that don't fit
 * under #ELM_TOOLBAR_SHRINK_HIDE mode. Finally, it'll scroll under
 * #ELM_TOOLBAR_SHRINK_SCROLL mode, and it will create a button to
 * aggregate items which didn't fit with the #ELM_TOOLBAR_SHRINK_MENU
 * mode.
 *
 * @warning This function's behavior will clash with those of
 * elm_scroller_policy_set(), so use either one of them, but not both.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_shrink_mode_set(Evas_Object *obj, Elm_Toolbar_Shrink_Mode shrink_mode);

/**
 * Get the shrink mode of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return Toolbar's items display behavior.
 *
 * @see elm_toolbar_shrink_mode_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Shrink_Mode      elm_toolbar_shrink_mode_get(const Evas_Object *obj);

/**
 * Set the item's transverse expansion of a given toolbar widget @p obj.
 *
 * @param obj The toolbar object.
 * @param transverse_expanded The transverse expansion of the item.
 * (EINA_TRUE = on, EINA_FALSE = off, default = EINA_FALSE)
 *
 * This will expand the transverse length of the item according the transverse length of the toolbar.
 * The default is what the transverse length of the item is set according its min value.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_transverse_expanded_set(Evas_Object *obj, Eina_Bool transverse_expanded);

/**
 * Get the transverse expansion of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return The transverse expansion of the item.
 * (EINA_TRUE = on, EINA_FALSE = off, default = EINA_FALSE)
 *
 * @see elm_toolbar_transverse_expand_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_transverse_expanded_get(const Evas_Object *obj);

/**
 * Enable/disable homogeneous mode.
 *
 * @param obj The toolbar object
 * @param homogeneous Assume the items within the toolbar are of the
 * same size (EINA_TRUE = on, EINA_FALSE = off). Default is @c EINA_FALSE.
 *
 * This will enable the homogeneous mode where items are of the same size.
 * @see elm_toolbar_homogeneous_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous);

/**
 * Get whether the homogeneous mode is enabled.
 *
 * @param obj The toolbar object.
 * @return Assume the items within the toolbar are of the same height
 * and width (EINA_TRUE = on, EINA_FALSE = off).
 *
 * @see elm_toolbar_homogeneous_set()
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_homogeneous_get(const Evas_Object *obj);

/**
 * Set the parent object of the toolbar items' menus.
 *
 * @param obj The toolbar object.
 * @param parent The parent of the menu objects.
 *
 * Each item can be set as item menu, with elm_toolbar_item_menu_set().
 *
 * For more details about setting the parent for toolbar menus, see
 * elm_menu_parent_set().
 *
 * @see elm_menu_parent_set() for details.
 * @see elm_toolbar_item_menu_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_menu_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * Get the parent object of the toolbar items' menus.
 *
 * @param obj The toolbar object.
 * @return The parent of the menu objects.
 *
 * @see elm_toolbar_menu_parent_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_menu_parent_get(const Evas_Object *obj);

/**
 * Set the alignment of the items.
 *
 * @param obj The toolbar object.
 * @param align The new alignment, a float between <tt> 0.0 </tt>
 * and <tt> 1.0 </tt>.
 *
 * Alignment of toolbar items, from <tt> 0.0 </tt> to indicates to align
 * left, to <tt> 1.0 </tt>, to align to right. <tt> 0.5 </tt> centralize
 * items.
 *
 * Centered items by default.
 *
 * @see elm_toolbar_align_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_align_set(Evas_Object *obj, double align);

/**
 * Get the alignment of the items.
 *
 * @param obj The toolbar object.
 * @return toolbar items alignment, a float between <tt> 0.0 </tt> and
 * <tt> 1.0 </tt>.
 *
 * @see elm_toolbar_align_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI double                       elm_toolbar_align_get(const Evas_Object *obj);

/**
 * Change a toolbar's orientation
 * @param obj The toolbar object
 * @param horizontal If @c EINA_TRUE, the toolbar is horizontal
 * By default, a toolbar will be horizontal. Use this function to create a vertical toolbar.
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Get a toolbar's orientation
 * @param obj The toolbar object
 * @return If @c EINA_TRUE, the toolbar is horizontal
 * By default, a toolbar will be horizontal. Use this function to determine whether a toolbar is vertical.
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_horizontal_get(const Evas_Object *obj);

/**
 * Get the number of items in a toolbar
 * @param obj The toolbar object
 * @return The number of items in @p obj toolbar
 * @ingroup Toolbar
 */
EAPI unsigned int                 elm_toolbar_items_count(const Evas_Object *obj);

/**
 * Set the standard priority of visible items in a toolbar
 * @param obj The toolbar object
 * @param priority The standard_priority of visible items
 *
 * If the priority of the item is up to standard priority, it is shown in basic panel.
 * The other items are located in more menu or panel. The more menu or panel can be shown when the more item is clicked.
 *
 * @see elm_toolbar_standard_priority_get()
 *
 * @since 1.7
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_standard_priority_set(Evas_Object *obj, int priority);

/**
 * Get the standard_priority of visible items in a toolbar
 * @param obj The toolbar object
 * @return The standard priority of items in @p obj toolbar
 *
 * @see elm_toolbar_standard_priority_set()
 *
 * @since 1.7
 * @ingroup Toolbar
 */
EAPI int                          elm_toolbar_standard_priority_get(const Evas_Object *obj);

/**
 * Set the toolbar select mode.
 *
 * @param obj The toolbar object
 * @param mode The select mode
 *
 * elm_toolbar_select_mode_set() changes item select mode in the toolbar widget.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : Items will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS :  This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select items
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 *
 * @see elm_toolbar_select_mode_get()
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode);

/**
 * Get the toolbar select mode.
 *
 * @param obj The toolbar object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_toolbar_select_mode_set()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Select_Mode
elm_toolbar_select_mode_get(const Evas_Object *obj);

/**
 * Set reorder mode
 *
 * @param obj The toolbar object
 * @param reorder_mode The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Toolbar
 */
EAPI void                          elm_toolbar_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode);

/**
 * Get the reorder mode
 *
 * @param obj The toolbar object
 * @return The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                     elm_toolbar_reorder_mode_get(const Evas_Object *obj);

