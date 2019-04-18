#ifndef _ELM_DISKSELECTOR_EO_LEGACY_H_
#define _ELM_DISKSELECTOR_EO_LEGACY_H_

#ifndef _ELM_DISKSELECTOR_EO_CLASS_TYPE
#define _ELM_DISKSELECTOR_EO_CLASS_TYPE

typedef Eo Elm_Diskselector;

#endif

#ifndef _ELM_DISKSELECTOR_EO_TYPES
#define _ELM_DISKSELECTOR_EO_TYPES


#endif

/**
 * @brief Set the side labels max length.
 *
 * Length is the number of characters of items' label that will be visible when
 * it's set on side positions. It will just crop the string after defined size.
 * E.g.:
 *
 * An item with label "January" would be displayed on side position as "Jan" if
 * max length is set to 3, or "Janu", if this property is set to 4.
 *
 * When it's selected, the entire label will be displayed, except for width
 * restrictions. In this case label will be cropped and "..." will be
 * concatenated.
 *
 * Default side label max length is 3.
 *
 * This property will be applied over all items, included before or later this
 * function call.
 *
 * @param[in] obj The object.
 * @param[in] len The max length defined for side labels.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI void elm_diskselector_side_text_max_length_set(Elm_Diskselector *obj, int len);

/**
 * @brief Get the side labels max length.
 *
 * See also @ref elm_diskselector_side_text_max_length_set for details.
 *
 * @param[in] obj The object.
 *
 * @return The max length defined for side labels.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI int elm_diskselector_side_text_max_length_get(const Elm_Diskselector *obj);

/**
 * @brief Enable or disable round mode.
 *
 * Disabled by default. If round mode is enabled the items list will work like
 * a circular list, so when the user reaches the last item, the first one will
 * popup.
 *
 * See also @ref elm_diskselector_round_enabled_get.
 *
 * @param[in] obj The object.
 * @param[in] enabled @c true to enable round mode or @c false to disable it.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI void elm_diskselector_round_enabled_set(Elm_Diskselector *obj, Eina_Bool enabled);

/**
 * @brief Get a value whether round mode is enabled or not.
 *
 * See also @ref elm_diskselector_round_enabled_set for details.
 *
 * @param[in] obj The object.
 *
 * @return @c true to enable round mode or @c false to disable it.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI Eina_Bool elm_diskselector_round_enabled_get(const Elm_Diskselector *obj);

/**
 * @brief Set the number of items to be displayed.
 *
 * Default value is 3, and also it's the minimum. If @c num is less than 3, it
 * will be set to 3.
 *
 * Also, it can be set on theme, using data item @c display_item_num on group
 * "elm/diskselector/item/X", where X is style set.
 *
 * @param[in] obj The object.
 * @param[in] num The number of items the diskselector will display.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI void elm_diskselector_display_item_num_set(Elm_Diskselector *obj, int num);

/**
 * @brief Get the number of items in the diskselector object.
 *
 * @param[in] obj The object.
 *
 * @return The number of items the diskselector will display.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI int elm_diskselector_display_item_num_get(const Elm_Diskselector *obj);

/**
 * @brief Get the first item of the diskselector.
 *
 * The list of items follows append order. So it will return the first item
 * appended to the widget that wasn't deleted.
 *
 * See also @ref elm_diskselector_item_append, @ref elm_diskselector_items_get.
 *
 * @param[in] obj The object.
 *
 * @return The first item, or @c null if none.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI Elm_Widget_Item *elm_diskselector_first_item_get(const Elm_Diskselector *obj);

/**
 * @brief Get a list of all the diskselector items.
 *
 * See also @ref elm_diskselector_item_append, @ref elm_object_item_del,
 * @ref elm_diskselector_clear.
 *
 * @param[in] obj The object.
 *
 * @return A @c list of diskselector items, or @c ull on failure.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI const Eina_List *elm_diskselector_items_get(const Elm_Diskselector *obj);

/**
 * @brief Get the last item of the diskselector.
 *
 * The list of items follows append order. So it will return last first item
 * appended to the widget that wasn't deleted.
 *
 * See also @ref elm_diskselector_item_append, @ref elm_diskselector_items_get.
 *
 * @param[in] obj The object.
 *
 * @return The last item, or @c null if none.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI Elm_Widget_Item *elm_diskselector_last_item_get(const Elm_Diskselector *obj);

/**
 * @brief Get the selected item.
 *
 * The selected item can be unselected with
 * @ref elm_diskselector_item_selected_set, and the first item of diskselector
 * will be selected.
 *
 * The selected item always will be centered on diskselector, with full label
 * displayed, i.e., max length set to side labels won't apply on the selected
 * item. More details on @ref elm_diskselector_side_text_max_length_set.
 *
 * @param[in] obj The object.
 *
 * @return The selected diskselector item.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI Elm_Widget_Item *elm_diskselector_selected_item_get(const Elm_Diskselector *obj);

/**
 * @brief Appends a new item to the diskselector object.
 *
 * A new item will be created and appended to the diskselector, i.e., will be
 * set as last item. Also, if there is no selected item, it will be selected.
 * This will always happens for the first appended item.
 *
 * If no icon is set, label will be centered on item position, otherwise the
 * icon will be placed at left of the label, that will be shifted to the right.
 *
 * Items created with this method can be deleted with @ref elm_object_item_del.
 *
 * Associated @c data can be properly freed when item is deleted if a callback
 * function is set with @ref elm_object_item_del_cb_set.
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user stops the diskselector with this item on center
 * position. If such function isn't needed, just passing @c null as @c func is
 * enough. The same should be done for @c data.
 *
 * See also @ref elm_object_item_del, @ref elm_diskselector_clear, @ref
 * elm_icon_add.
 *
 * @param[in] obj The object.
 * @param[in] label The label of the diskselector item.
 * @param[in] icon The icon object to use at left side of the item. An icon can
 * be any Evas object, but usually it is an icon created with elm_icon_add().
 * @param[in] func The function to call when the item is selected.
 * @param[in] data The data to associate with the item for related callbacks.
 *
 * @return New item
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI Elm_Widget_Item *elm_diskselector_item_append(Elm_Diskselector *obj, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * @brief Remove all diskselector's items.
 *
 * See also @ref elm_object_item_del, @ref elm_diskselector_item_append.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Diskselector_Group
 */
EAPI void elm_diskselector_clear(Elm_Diskselector *obj);

#endif
