#ifndef _ELM_NAVIFRAME_ITEM_EO_H_
#define _ELM_NAVIFRAME_ITEM_EO_H_

#ifndef _ELM_NAVIFRAME_ITEM_EO_CLASS_TYPE
#define _ELM_NAVIFRAME_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Naviframe_Item;

#endif

#ifndef _ELM_NAVIFRAME_ITEM_EO_TYPES
#define _ELM_NAVIFRAME_ITEM_EO_TYPES


#endif
/** Elementary naviframe item class
 *
 * @ingroup Elm_Naviframe_Item
 */
#define ELM_NAVIFRAME_ITEM_CLASS elm_naviframe_item_class_get()

EWAPI const Efl_Class *elm_naviframe_item_class_get(void);

/**
 * @brief Pop the top item and delete the items between the top and the above
 * one on the given item.
 *
 * The items between the top and the given item will be deleted first, and then
 * the top item will be popped at last.
 *
 * @warning The pop callback function set by elm_naviframe_item_pop_cb_set is
 * not called for items between the top item and the destination item since
 * those items are deleted without being popped.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Naviframe_Item
 */
EOAPI void elm_obj_naviframe_item_pop_to(Eo *obj);

/**
 * @brief Get a value whether title area is enabled or not.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, title area will be enabled, disabled otherwise
 *
 * @ingroup Elm_Naviframe_Item
 */
EOAPI Eina_Bool elm_obj_naviframe_item_title_enabled_get(const Eo *obj);

/**
 * @brief Enable/Disable the title area with transition effect
 *
 * When the title area is disabled, then the controls would be hidden so as to
 * expand the content area to full-size.
 *
 * @param[in] obj The object.
 * @param[in] enable If @c true, title area will be enabled, disabled otherwise
 * @param[in] transition If @c true, transition effect of the title will be
 * visible, invisible otherwise
 *
 * @ingroup Elm_Naviframe_Item
 */
EOAPI void elm_obj_naviframe_item_title_enabled_set(Eo *obj, Eina_Bool enable, Eina_Bool transition);

/**
 * @brief Promote an item already in the naviframe stack to the top of the
 * stack.
 *
 * This will take the indicated item and promote it to the top of the stack as
 * if it had been pushed there. The item must already be inside the naviframe
 * stack to work.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Naviframe_Item
 */
EOAPI void elm_obj_naviframe_item_promote(Eo *obj);

/**
 * @brief Set a function to be called when an item of the naviframe is going to
 * be popped.
 *
 * @warning Don't set "clicked" callback to the prev button additionally if the
 * function does an exact same logic with this @c func. When hardware back key
 * is pressed then both callbacks will be called. Warning: The pop callback
 * function is called only if the item is popped. Therefore, the pop callback
 * function is not called if the item is deleted without being popped.
 *
 * @param[in] obj The object.
 * @param[in] func The callback function.
 * @param[in] data Data to be passed to func call.
 *
 * @since 1.8
 *
 * @ingroup Elm_Naviframe_Item
 */
EOAPI void elm_obj_naviframe_item_pop_cb_set(Eo *obj, Elm_Naviframe_Item_Pop_Cb func, void *data);

#endif
