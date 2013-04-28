/**
 * @brief Add a new menu to the parent
 *
 * @param parent The parent object.
 * @return The new object or NULL if it cannot be created.
 *
 * @ingroup Menu
 */
EAPI Evas_Object                 *elm_menu_add(Evas_Object *parent);

/**
 * @brief Set the parent for the given menu widget
 *
 * @param obj The menu object.
 * @param parent The new parent.
 *
 * @ingroup Menu
 */
EAPI void                         elm_menu_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the parent for the given menu widget
 *
 * @param obj The menu object.
 * @return The parent.
 *
 * @see elm_menu_parent_set()
 *
 * @ingroup Menu
 */
EAPI Evas_Object                 *elm_menu_parent_get(const Evas_Object *obj);

/**
 * @brief Move the menu to a new position
 *
 * @param obj The menu object.
 * @param x The new position.
 * @param y The new position.
 *
 * Sets the top-left position of the menu to (@p x,@p y).
 *
 * @note @p x and @p y coordinates are relative to parent.
 *
 * @ingroup Menu
 */
EAPI void                         elm_menu_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/**
 * @brief Close a opened menu
 *
 * @param obj the menu object
 * @return void
 *
 * Hides the menu and all it's sub-menus.
 *
 * @ingroup Menu
 */
EAPI void                         elm_menu_close(Evas_Object *obj);

/**
 * @brief Add an item at the end of the given menu widget
 *
 * @param obj The menu object.
 * @param parent The parent menu item (optional)
 * @param icon An icon display on the item. The icon will be destroyed by the menu.
 * @param label The label of the item.
 * @param func Function called when the user select the item.
 * @param data Data sent by the callback.
 * @return Returns the new item.
 *
 * @note This function does not accept relative icon path.
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_item_add(Evas_Object *obj, Elm_Object_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * @brief Add a separator item to menu @p obj under @p parent.
 *
 * @param obj The menu object
 * @param parent The item to add the separator under
 * @return The created item or NULL on failure
 *
 * This is item is a @ref Separator.
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_item_separator_add(Evas_Object *obj, Elm_Object_Item *parent);

/**
 * @brief Returns a list of @p item's items.
 *
 * @param obj The menu object
 * @return An Eina_List* of @p item's items
 *
 * @ingroup Menu
 */
EAPI const Eina_List             *elm_menu_items_get(const Evas_Object *obj);

/**
 * @brief Get the first item in the menu
 *
 * @param obj The menu object
 * @return The first item, or NULL if none
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_first_item_get(const Evas_Object *obj);

/**
 * @brief Get the last item in the menu
 *
 * @param obj The menu object
 * @return The last item, or NULL if none
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_last_item_get(const Evas_Object *obj);

/**
 * @brief Get the selected item in the menu
 *
 * @param obj The menu object
 * @return The selected item, or NULL if none
 *
 * @see elm_menu_item_selected_get()
 * @see elm_menu_item_selected_set()
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_selected_item_get(const Evas_Object *obj);
