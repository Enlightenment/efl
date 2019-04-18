#ifndef _ELM_MULTIBUTTONENTRY_ITEM_EO_LEGACY_H_
#define _ELM_MULTIBUTTONENTRY_ITEM_EO_LEGACY_H_

#ifndef _ELM_MULTIBUTTONENTRY_ITEM_EO_CLASS_TYPE
#define _ELM_MULTIBUTTONENTRY_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Multibuttonentry_Item;

#endif

#ifndef _ELM_MULTIBUTTONENTRY_ITEM_EO_TYPES
#define _ELM_MULTIBUTTONENTRY_ITEM_EO_TYPES


#endif

/**
 * @brief Control the selected state of an item
 *
 * @param[in] obj The object.
 * @param[in] selected @c true if selected @c false otherwise
 *
 * @ingroup Elm_Multibuttonentry_Item_Group
 */
EAPI void elm_multibuttonentry_item_selected_set(Elm_Multibuttonentry_Item *obj, Eina_Bool selected);

/**
 * @brief Control the selected state of an item
 *
 * @param[in] obj The object.
 *
 * @return @c true if selected @c false otherwise
 *
 * @ingroup Elm_Multibuttonentry_Item_Group
 */
EAPI Eina_Bool elm_multibuttonentry_item_selected_get(const Elm_Multibuttonentry_Item *obj);

/**
 * @brief Get the previous item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The item before the item
 *
 * @ingroup Elm_Multibuttonentry_Item_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_item_prev_get(const Elm_Multibuttonentry_Item *obj);

/**
 * @brief Get the next item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The item after the item
 *
 * @ingroup Elm_Multibuttonentry_Item_Group
 */
EAPI Elm_Widget_Item *elm_multibuttonentry_item_next_get(const Elm_Multibuttonentry_Item *obj);

#endif
