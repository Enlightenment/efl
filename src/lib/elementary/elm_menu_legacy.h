/**
 * @brief Add a new menu to the parent
 *
 * @param parent The parent object.
 * @return The new object or NULL if it cannot be created.
 *
 * @ingroup Elm_Menu
 */
EAPI Evas_Object                 *elm_menu_add(Evas_Object *parent);

/**
 * @brief Set the parent for the given menu widget
 *
 * @param obj The menu object.
 * @param parent The new parent.
 *
 * @ingroup Elm_Menu
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
 * @ingroup Elm_Menu
 */
EAPI Evas_Object                 *elm_menu_parent_get(const Evas_Object *obj);

/**
 * @brief Get the selected item in the menu.
 *
 * @return The selected item or @c null.
 *
 * @ingroup Elm_Menu
 */
EAPI Elm_Widget_Item *elm_menu_selected_item_get(const Evas_Object *obj);

/** Returns a list of the item's items.
 *
 * @ingroup Elm_Menu
 */
EAPI const Eina_List *elm_menu_items_get(const Evas_Object *obj);

/** Get the first item in the menu.
 *
 * @ingroup Elm_Menu
 */
EAPI Elm_Widget_Item *elm_menu_first_item_get(const Evas_Object *obj);

/** Get the last item in the menu.
 *
 * @ingroup Elm_Menu
 */
EAPI Elm_Widget_Item *elm_menu_last_item_get(const Evas_Object *obj);

/**
 * @brief Set the selected state of @c item.
 *
 * @param[in] selected The selection state.
 *
 * @ingroup Elm_Menu_Item
 */
EAPI void elm_menu_item_selected_set(Evas_Object *obj, Eina_Bool selected);

/**
 * @brief Get the selected state of @c item.
 *
 * @return The selection state.
 *
 * @ingroup Elm_Menu_Item
 */
EAPI Eina_Bool elm_menu_item_selected_get(const Evas_Object *obj);

/** Get the previous item in the menu.
 *
 * @ingroup Elm_Menu_Item
 */
EAPI Elm_Widget_Item *elm_menu_item_prev_get(const Evas_Object *obj);

/** Get the next item in the menu.
 *
 * @ingroup Elm_Menu_Item
 */
EAPI Elm_Widget_Item *elm_menu_item_next_get(const Evas_Object *obj);

#include "elm_menu_item.eo.legacy.h"
#include "elm_menu.eo.legacy.h"
