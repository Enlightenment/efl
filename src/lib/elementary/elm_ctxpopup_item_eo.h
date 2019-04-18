#ifndef _ELM_CTXPOPUP_ITEM_EO_H_
#define _ELM_CTXPOPUP_ITEM_EO_H_

#ifndef _ELM_CTXPOPUP_ITEM_EO_CLASS_TYPE
#define _ELM_CTXPOPUP_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Ctxpopup_Item;

#endif

#ifndef _ELM_CTXPOPUP_ITEM_EO_TYPES
#define _ELM_CTXPOPUP_ITEM_EO_TYPES


#endif
/** Elementary context popup item class
 *
 * @ingroup Elm_Ctxpopup_Item
 */
#define ELM_CTXPOPUP_ITEM_CLASS elm_ctxpopup_item_class_get()

EWAPI const Efl_Class *elm_ctxpopup_item_class_get(void);

/**
 * @brief Get the item before this one in the widget's list of items.
 *
 * See also @ref elm_obj_ctxpopup_item_next_get.
 *
 * @param[in] obj The object.
 *
 * @return The item before the object in its parent's list. If there is no
 * previous item or in case of error, @c null is returned.
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EOAPI Elm_Widget_Item *elm_obj_ctxpopup_item_prev_get(const Eo *obj);

/**
 * @brief Get the item after this one in the widget's list of items.
 *
 * See also @ref elm_obj_ctxpopup_item_prev_get.
 *
 * @param[in] obj The object.
 *
 * @return The item after the object in its parent's list. If there is no next
 * item or in case of error, @c null is returned.
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EOAPI Elm_Widget_Item *elm_obj_ctxpopup_item_next_get(const Eo *obj);

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
 * @ref elm_obj_ctxpopup_selected_item_get.
 *
 * Selected items will be highlighted.
 *
 * @param[in] obj The object.
 * @param[in] selected The selection state.
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EOAPI void elm_obj_ctxpopup_item_selected_set(Eo *obj, Eina_Bool selected);

/**
 * @brief Indicates whether this item is currently selected.
 *
 * Get the selected state of this item.
 *
 * @param[in] obj The object.
 *
 * @return The selection state.
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EOAPI Eina_Bool elm_obj_ctxpopup_item_selected_get(const Eo *obj);

/**
 * @brief Init context popup item
 *
 * @param[in] obj The object.
 * @param[in] func Smart callback function
 * @param[in] data Data pointer
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EOAPI void elm_obj_ctxpopup_item_init(Eo *obj, Evas_Smart_Cb func, const void *data);

#endif
