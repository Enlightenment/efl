/**
 * Get the real Evas(Edje) object created to implement the view of a given
 * menu @p item.
 *
 * @param it The menu item.
 * @return The base Edje object containing the swallowed content associated with
 * @p it.
 *
 * @warning Don't manipulate this object!
 *
 * @ingroup Menu
 */
EAPI Evas_Object                 *elm_menu_item_object_get(const Elm_Object_Item *it);

/**
 * @brief Set the icon of a menu item to the standard icon with name @p icon
 *
 * @param it The menu item object.
 * @param icon The name of icon object to set for the content of @p item
 *
 * Once this icon is set, any previously set icon will be deleted.
 * To hide icon set it to "".
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
 * Remove all sub-items (children) of the given item
 * @since 1.8
 *
 * @param it The item
 *
 * This removes all items that are children (and their descendants) of the
 * given item @p it.
 *
 * @see elm_object_item_del()
 *
 * @ingroup Menu
 */
EAPI void                         elm_menu_item_subitems_clear(Elm_Object_Item *it);

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

