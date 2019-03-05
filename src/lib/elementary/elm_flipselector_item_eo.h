#ifndef _ELM_FLIPSELECTOR_ITEM_EO_H_
#define _ELM_FLIPSELECTOR_ITEM_EO_H_

#ifndef _ELM_FLIPSELECTOR_ITEM_EO_CLASS_TYPE
#define _ELM_FLIPSELECTOR_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Flipselector_Item;

#endif

#ifndef _ELM_FLIPSELECTOR_ITEM_EO_TYPES
#define _ELM_FLIPSELECTOR_ITEM_EO_TYPES


#endif
/** Elementary flipselector item class
 *
 * @ingroup Elm_Flipselector_Item
 */
#define ELM_FLIPSELECTOR_ITEM_CLASS elm_flipselector_item_class_get()

EWAPI const Efl_Class *elm_flipselector_item_class_get(void);

/**
 * @brief Set whether a given flip selector widget's item should be the
 * currently selected one.
 *
 * This sets whether @c item is or not the selected (thus, under display) one.
 * If @c item is different than the one under display, the latter will be
 * unselected. If the @c item is set to be unselected, on the other hand, the
 * first item in the widget's internal members list will be the new selected
 * one.
 *
 * See also @ref elm_obj_flipselector_item_selected_get.
 *
 * @param[in] obj The object.
 * @param[in] selected @c true if selected, @c false otherwise.
 *
 * @ingroup Elm_Flipselector_Item
 */
EOAPI void elm_obj_flipselector_item_selected_set(Eo *obj, Eina_Bool selected);

/**
 * @brief Get whether a given flip selector widget's item is the currently
 * selected one.
 *
 * See also @ref elm_obj_flipselector_item_selected_set.
 *
 * @param[in] obj The object.
 *
 * @return @c true if selected, @c false otherwise.
 *
 * @ingroup Elm_Flipselector_Item
 */
EOAPI Eina_Bool elm_obj_flipselector_item_selected_get(const Eo *obj);

/**
 * @brief Get the item before @c item in a flip selector widget's internal list
 * of items.
 *
 * See also @ref elm_obj_flipselector_item_next_get.
 *
 * @param[in] obj The object.
 *
 * @return The item before the @c item, in its parent's list. If there is no
 * previous item for @c item or there's an error, @c null is returned.
 *
 * @ingroup Elm_Flipselector_Item
 */
EOAPI Elm_Widget_Item *elm_obj_flipselector_item_prev_get(const Eo *obj);

/**
 * @brief Get the item after @c item in a flip selector widget's internal list
 * of items.
 *
 * See also @ref elm_obj_flipselector_item_prev_get.
 *
 * @param[in] obj The object.
 *
 * @return The item after the @c item, in its parent's list. If there is no
 * next item for @c item or there's an error, @c null is returned.
 *
 * @ingroup Elm_Flipselector_Item
 */
EOAPI Elm_Widget_Item *elm_obj_flipselector_item_next_get(const Eo *obj);

#endif
