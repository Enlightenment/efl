#ifndef _ELM_MULTIBUTTONENTRY_ITEM_EO_H_
#define _ELM_MULTIBUTTONENTRY_ITEM_EO_H_

#ifndef _ELM_MULTIBUTTONENTRY_ITEM_EO_CLASS_TYPE
#define _ELM_MULTIBUTTONENTRY_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Multibuttonentry_Item;

#endif

#ifndef _ELM_MULTIBUTTONENTRY_ITEM_EO_TYPES
#define _ELM_MULTIBUTTONENTRY_ITEM_EO_TYPES


#endif
/** Elementary multibuttonentry class
 *
 * @ingroup Elm_Multibuttonentry_Item
 */
#define ELM_MULTIBUTTONENTRY_ITEM_CLASS elm_multibuttonentry_item_class_get()

EWAPI const Efl_Class *elm_multibuttonentry_item_class_get(void);

/**
 * @brief Control the selected state of an item
 *
 * @param[in] obj The object.
 * @param[in] selected @c true if selected @c false otherwise
 *
 * @ingroup Elm_Multibuttonentry_Item
 */
EOAPI void elm_obj_multibuttonentry_item_selected_set(Eo *obj, Eina_Bool selected);

/**
 * @brief Control the selected state of an item
 *
 * @param[in] obj The object.
 *
 * @return @c true if selected @c false otherwise
 *
 * @ingroup Elm_Multibuttonentry_Item
 */
EOAPI Eina_Bool elm_obj_multibuttonentry_item_selected_get(const Eo *obj);

/**
 * @brief Get the previous item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The item before the item
 *
 * @ingroup Elm_Multibuttonentry_Item
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_item_prev_get(const Eo *obj);

/**
 * @brief Get the next item in the multibuttonentry
 *
 * @param[in] obj The object.
 *
 * @return The item after the item
 *
 * @ingroup Elm_Multibuttonentry_Item
 */
EOAPI Elm_Widget_Item *elm_obj_multibuttonentry_item_next_get(const Eo *obj);

#endif
