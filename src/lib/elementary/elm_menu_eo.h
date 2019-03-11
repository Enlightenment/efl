#ifndef _ELM_MENU_EO_H_
#define _ELM_MENU_EO_H_

#ifndef _ELM_MENU_EO_CLASS_TYPE
#define _ELM_MENU_EO_CLASS_TYPE

typedef Eo Elm_Menu;

#endif

#ifndef _ELM_MENU_EO_TYPES
#define _ELM_MENU_EO_TYPES


#endif
/** Elementary menu class
 *
 * @ingroup Elm_Menu
 */
#define ELM_MENU_CLASS elm_menu_class_get()

EWAPI const Efl_Class *elm_menu_class_get(void);

/**
 * @brief Get the selected item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The selected item or @c null.
 *
 * @ingroup Elm_Menu
 */
EOAPI Elm_Widget_Item *elm_obj_menu_selected_item_get(const Eo *obj);

/**
 * @brief Get the first item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The first item or @c null.
 *
 * @ingroup Elm_Menu
 */
EOAPI Elm_Widget_Item *elm_obj_menu_first_item_get(const Eo *obj);

/**
 * @brief Get the last item in the widget.
 *
 * @param[in] obj The object.
 *
 * @return The last item or @c null.
 *
 * @ingroup Elm_Menu
 */
EOAPI Elm_Widget_Item *elm_obj_menu_last_item_get(const Eo *obj);

/**
 * @brief Returns a list of the widget item.
 *
 * @param[in] obj The object.
 *
 * @return const list to widget items
 *
 * @ingroup Elm_Menu
 */
EOAPI const Eina_List *elm_obj_menu_items_get(const Eo *obj);

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
 * @ingroup Elm_Menu
 */
EOAPI void elm_obj_menu_relative_move(Eo *obj, int x, int y);

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
 * @ingroup Elm_Menu
 */
EOAPI Elm_Widget_Item *elm_obj_menu_item_add(Eo *obj, Elm_Widget_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * @brief Open a closed menu
 *
 * Show the menu with no child sub-menus expanded..
 * @param[in] obj The object.
 *
 * @ingroup Elm_Menu
 */
EOAPI void elm_obj_menu_open(Eo *obj);

/**
 * @brief Close a opened menu
 *
 * Hides the menu and all it's sub-menus.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Menu
 */
EOAPI void elm_obj_menu_close(Eo *obj);

/**
 * @brief Add a separator item to menu @c obj under @c parent.
 *
 * @param[in] obj The object.
 * @param[in] parent The item to add the separator under.
 *
 * @return The created item or @c null.
 *
 * @ingroup Elm_Menu
 */
EOAPI Elm_Widget_Item *elm_obj_menu_item_separator_add(Eo *obj, Elm_Widget_Item *parent);

EWAPI extern const Efl_Event_Description _ELM_MENU_EVENT_DISMISSED;

/** Called when menu widget was dismissed
 *
 * @ingroup Elm_Menu
 */
#define ELM_MENU_EVENT_DISMISSED (&(_ELM_MENU_EVENT_DISMISSED))

EWAPI extern const Efl_Event_Description _ELM_MENU_EVENT_ELM_ACTION_BLOCK_MENU;

/** Called when menu blocking have been enabled
 *
 * @ingroup Elm_Menu
 */
#define ELM_MENU_EVENT_ELM_ACTION_BLOCK_MENU (&(_ELM_MENU_EVENT_ELM_ACTION_BLOCK_MENU))

EWAPI extern const Efl_Event_Description _ELM_MENU_EVENT_ELM_ACTION_UNBLOCK_MENU;

/** Called when menu blocking has been disabled
 *
 * @ingroup Elm_Menu
 */
#define ELM_MENU_EVENT_ELM_ACTION_UNBLOCK_MENU (&(_ELM_MENU_EVENT_ELM_ACTION_UNBLOCK_MENU))

#endif
