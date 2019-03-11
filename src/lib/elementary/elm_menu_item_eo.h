#ifndef _ELM_MENU_ITEM_EO_H_
#define _ELM_MENU_ITEM_EO_H_

#ifndef _ELM_MENU_ITEM_EO_CLASS_TYPE
#define _ELM_MENU_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Menu_Item;

#endif

#ifndef _ELM_MENU_ITEM_EO_TYPES
#define _ELM_MENU_ITEM_EO_TYPES


#endif
/** Elementary menu item class
 *
 * @ingroup Elm_Menu_Item
 */
#define ELM_MENU_ITEM_CLASS elm_menu_item_class_get()

EWAPI const Efl_Class *elm_menu_item_class_get(void);

/**
 * @brief Set the icon of a menu item to the standard icon with the given name.
 *
 * Once this icon is set, any previously set icon will be deleted. To hide icon
 * set it to "".
 *
 * @param[in] obj The object.
 * @param[in] icon The name of icon object.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI void elm_obj_menu_item_icon_name_set(Eo *obj, const char *icon);

/**
 * @brief Get the string representation from the icon of a menu item
 *
 * Once this icon is set, any previously set icon will be deleted. To hide icon
 * set it to "".
 *
 * @param[in] obj The object.
 *
 * @return The name of icon object.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI const char *elm_obj_menu_item_icon_name_get(const Eo *obj);

/**
 * @brief Get the item before this one in the widget's list of items.
 *
 * See also @ref elm_obj_menu_item_next_get.
 *
 * @param[in] obj The object.
 *
 * @return The item before the object in its parent's list. If there is no
 * previous item or in case of error, @c null is returned.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI Elm_Widget_Item *elm_obj_menu_item_prev_get(const Eo *obj);

/**
 * @brief Get the item after this one in the widget's list of items.
 *
 * See also @ref elm_obj_menu_item_prev_get.
 *
 * @param[in] obj The object.
 *
 * @return The item after the object in its parent's list. If there is no next
 * item or in case of error, @c null is returned.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI Elm_Widget_Item *elm_obj_menu_item_next_get(const Eo *obj);

/**
 * @brief Indicates whether this item is currently selected.
 *
 * Set the selected state of @c item.
 *
 * This sets the selected state of the given item @c it. @c true for selected,
 * @c false for not selected.
 *
 * If a new item is selected the previously selected will be unselected.
 * Previously selected item can be retrieved with
 * @ref elm_obj_menu_selected_item_get.
 *
 * Selected items will be highlighted.
 *
 * @param[in] obj The object.
 * @param[in] selected The selection state.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI void elm_obj_menu_item_selected_set(Eo *obj, Eina_Bool selected);

/**
 * @brief Indicates whether this item is currently selected.
 *
 * Get the selected state of this item.
 *
 * @param[in] obj The object.
 *
 * @return The selection state.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI Eina_Bool elm_obj_menu_item_selected_get(const Eo *obj);

/**
 * @brief Get the item index
 *
 * @param[in] obj The object.
 *
 * @return Item index
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI unsigned int elm_obj_menu_item_index_get(const Eo *obj);

/**
 * @brief Remove all sub-items (children) of the given item
 *
 * This removes all items that are children (and their descendants) of the
 * given item @c it.
 *
 * @param[in] obj The object.
 *
 * @since 1.8
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI void elm_obj_menu_item_subitems_clear(Eo *obj);

/**
 * @brief Returns a list of @c item's subitems.
 *
 * @param[in] obj The object.
 *
 * @return A list of @c item's subitems
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI const Eina_List *elm_obj_menu_item_subitems_get(const Eo *obj);

/**
 * @brief Returns whether @c item is a separator.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the item is a separator.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI Eina_Bool elm_obj_menu_item_is_separator(const Eo *obj);

/**
 * @brief Get the real Evas(Edje) object created to implement the view of a
 * given menu item.
 *
 * @warning Don't manipulate this object!
 *
 * @param[in] obj The object.
 *
 * @return The base Edje object containing the swallowed content associated
 * with the item.
 *
 * @ingroup Elm_Menu_Item
 */
EOAPI Efl_Canvas_Object *elm_obj_menu_item_object_get(const Eo *obj);

#endif
