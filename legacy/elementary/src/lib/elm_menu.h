/**
 * @defgroup Menu Menu
 * @ingroup Elementary
 *
 * @image html menu_inheritance_tree.png
 * @image latex menu_inheritance_tree.eps
 *
 * @image html img/widget/menu/preview-00.png
 * @image latex img/widget/menu/preview-00.eps
 *
 * A menu is a list of items displayed above its parent. When the menu is
 * showing its parent is darkened. Each item can have a sub-menu. The menu
 * object can be used to display a menu on a right click event, in a toolbar,
 * anywhere.
 *
 * Signals that you can add callbacks for are:
 * @li "clicked" - the user clicked the empty space in the menu to dismiss.
 *
 * Default content parts of the menu items that you can use for are:
 * @li "default" - A main content of the menu item
 *
 * Default text parts of the menu items that you can use for are:
 * @li "default" - label in the menu item
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 *
 * @see @ref tutorial_menu
 * @{
 */

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
 * @brief Returns a list of @p item's items.
 *
 * @param obj The menu object
 * @return An Eina_List* of @p item's items
 *
 * @ingroup Menu
 */
EAPI const Eina_List             *elm_menu_items_get(const Evas_Object *obj);

/**
 * @brief Get the Evas_Object of an Elm_Object_Item
 *
 * @param it The menu item object.
 * @return The edje object containing the swallowed content
 *
 * @warning Don't manipulate this object!
 *
 * @ingroup Menu
 */
EAPI Evas_Object                 *elm_menu_item_object_get(const Elm_Object_Item *it);

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
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_item_add(Evas_Object *obj, Elm_Object_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * @brief Set the icon of a menu item to the standard icon with name @p icon
 *
 * @param it The menu item object.
 * @param icon The name of icon object to set for the content of @p item
 *
 * Once this icon is set, any previously set icon will be deleted.
 *
 * @ingroup Menu
 */
EAPI void                         elm_menu_item_icon_name_set(Elm_Object_Item *it, const char *icon);

/**
 * @brief Get the string representation from the icon of a menu item
 *
 * @param it The menu item object.
 * @return The string representation of @p item's icon or NULL
 *
 * @see elm_menu_item_icon_name_set()
 *
 * @ingroup Menu
 */
EAPI const char                  *elm_menu_item_icon_name_get(const Elm_Object_Item *it);

/**
 * @brief Set the selected state of @p item.
 *
 * @param it The menu item object.
 * @param selected The selected/unselected state of the item
 *
 * @ingroup Menu
 */
EAPI void                         elm_menu_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * @brief Get the selected state of @p item.
 *
 * @param it The menu item object.
 * @return The selected/unselected state of the item
 *
 * @see elm_menu_item_selected_set()
 *
 * @ingroup Menu
 */
EAPI Eina_Bool                    elm_menu_item_selected_get(const Elm_Object_Item *it);

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
 * @brief Returns whether @p item is a separator.
 *
 * @param it The item to check
 * @return If true, @p item is a separator
 *
 * @see elm_menu_item_separator_add()
 *
 * @ingroup Menu
 */
EAPI Eina_Bool                    elm_menu_item_is_separator(Elm_Object_Item *it);

/**
 * @brief Returns a list of @p item's subitems.
 *
 * @param it The item
 * @return An Eina_List* of @p item's subitems
 *
 * @see elm_menu_add()
 *
 * @ingroup Menu
 */
EAPI const Eina_List             *elm_menu_item_subitems_get(const Elm_Object_Item *it);

/**
 * @brief Get the position of a menu item
 *
 * @param it The menu item
 * @return The item's index
 *
 * This function returns the index position of a menu item in a menu.
 * For a sub-menu, this number is relative to the first item in the sub-menu.
 *
 * @note Index values begin with 0
 *
 * @ingroup Menu
 */
EAPI unsigned int                 elm_menu_item_index_get(const Elm_Object_Item *it);

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
 * @brief Get the first item in the menu
 *
 * @param obj The menu object
 * @return The first item, or NULL if none
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_first_item_get(const Evas_Object *obj);

/**
 * @brief Get the next item in the menu.
 *
 * @param it The menu item object.
 * @return The item after it, or NULL if none
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_item_next_get(const Elm_Object_Item *it);

/**
 * @brief Get the previous item in the menu.
 *
 * @param it The menu item object.
 * @return The item before it, or NULL if none
 *
 * @ingroup Menu
 */
EAPI Elm_Object_Item             *elm_menu_item_prev_get(const Elm_Object_Item *it);

/**
 * @}
 */
