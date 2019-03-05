#ifndef _ELM_MENU_EO_LEGACY_H_
#define _ELM_MENU_EO_LEGACY_H_

#ifndef _ELM_MENU_EO_CLASS_TYPE
#define _ELM_MENU_EO_CLASS_TYPE

typedef Eo Elm_Menu;

#endif

#ifndef _ELM_MENU_EO_TYPES
#define _ELM_MENU_EO_TYPES


#endif

/**
 * @brief Get the selected item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The selected item or @c null.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI Elm_Widget_Item *elm_menu_selected_item_get(const Elm_Menu *obj);

/**
 * @brief Get the first item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The first item or @c null.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI Elm_Widget_Item *elm_menu_first_item_get(const Elm_Menu *obj);

/**
 * @brief Get the last item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The last item or @c null.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI Elm_Widget_Item *elm_menu_last_item_get(const Elm_Menu *obj);

/**
 * @brief Returns a list of the widget item.
 *
 * @param[in] obj The object.
 *
 * @return const list to widget items
 *
 * @ingroup Elm_Menu_Group
 */
EAPI const Eina_List *elm_menu_items_get(const Elm_Menu *obj);

/**
 * @brief Move the menu to a new position
 *
 * Sets the top-left position of the menu to ($x, @c y).
 *
 * @note @c x and @c y coordinates are relative to parent.
 *
 * @param[in] obj The object.
 * @param[in] x The new X coordinate
 * @param[in] y The new Y coordinate
 *
 * @ingroup Elm_Menu_Group
 */
EAPI void elm_menu_move(Elm_Menu *obj, int x, int y);

/**
 * @brief Add an item at the end of the given menu widget.
 *
 * @note This function does not accept relative icon path.
 *
 * @param[in] obj The object.
 * @param[in] parent The parent menu item (optional).
 * @param[in] icon An icon display on the item. The icon will be destroyed by
 * the menu.
 * @param[in] label The label of the item.
 * @param[in] func Function called when the user select the item.
 * @param[in] data Data sent by the callback.
 *
 * @return The new menu item.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI Elm_Widget_Item *elm_menu_item_add(Elm_Menu *obj, Elm_Widget_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * @brief Open a closed menu
 *
 * Show the menu with no child sub-menus expanded..
 * @param[in] obj The object.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI void elm_menu_open(Elm_Menu *obj);

/**
 * @brief Close a opened menu
 *
 * Hides the menu and all it's sub-menus.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI void elm_menu_close(Elm_Menu *obj);

/**
 * @brief Add a separator item to menu @c obj under @c parent.
 *
 * @param[in] obj The object.
 * @param[in] parent The item to add the separator under.
 *
 * @return The created item or @c null.
 *
 * @ingroup Elm_Menu_Group
 */
EAPI Elm_Widget_Item *elm_menu_item_separator_add(Elm_Menu *obj, Elm_Widget_Item *parent);

#endif
