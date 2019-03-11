#ifndef _ELM_INDEX_ITEM_EO_LEGACY_H_
#define _ELM_INDEX_ITEM_EO_LEGACY_H_

#ifndef _ELM_INDEX_ITEM_EO_CLASS_TYPE
#define _ELM_INDEX_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Index_Item;

#endif

#ifndef _ELM_INDEX_ITEM_EO_TYPES
#define _ELM_INDEX_ITEM_EO_TYPES


#endif

/**
 * @brief Set the selected state of an item.
 *
 * This sets the selected state of the given item @c it. @c true for selected,
 * @c false for not selected.
 *
 * If a new item is selected the previously selected will be unselected.
 * Previously selected item can be get with function
 * elm_index_selected_item_get().
 *
 * Selected items will be highlighted.
 *
 * @param[in] obj The object.
 * @param[in] selected @c true if selected, @c false otherwise
 *
 * @ingroup Elm_Index_Item_Group
 */
EAPI void elm_index_item_selected_set(Elm_Index_Item *obj, Eina_Bool selected);

/**
 * @brief Sets the priority of an item.
 *
 * The priority is -1 by default, which means that the item doesn't belong to a
 * group. The value of the priority starts from 0.
 *
 * In elm_index_level_go, the items are sorted in ascending order according to
 * priority. Items of the same priority make a group and the primary group is
 * shown by default.
 *
 * @param[in] obj The object.
 * @param[in] priority The priority
 *
 * @ingroup Elm_Index_Item_Group
 */
EAPI void elm_index_item_priority_set(Elm_Index_Item *obj, int priority);

/**
 * @brief Get the letter (string) set on a given index widget item.
 *
 * @param[in] obj The object.
 *
 * @return The letter string set on @c item
 *
 * @ingroup Elm_Index_Item_Group
 */
EAPI const char *elm_index_item_letter_get(const Elm_Index_Item *obj);

#endif
