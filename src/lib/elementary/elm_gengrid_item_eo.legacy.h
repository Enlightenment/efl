#ifndef _ELM_GENGRID_ITEM_EO_LEGACY_H_
#define _ELM_GENGRID_ITEM_EO_LEGACY_H_

#ifndef _ELM_GENGRID_ITEM_EO_CLASS_TYPE
#define _ELM_GENGRID_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Gengrid_Item;

#endif

#ifndef _ELM_GENGRID_ITEM_EO_TYPES
#define _ELM_GENGRID_ITEM_EO_TYPES


#endif

/**
 * @brief Get the previous item in a gengrid widget's internal list of items,
 * given a handle to one of those items.
 *
 * This returns the item placed before the @c item, on the container gengrid.
 *
 * @param[in] obj The object.
 *
 * @return The item before @c item, or @c NULL if there's none (and on errors)
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI Elm_Widget_Item *elm_gengrid_item_prev_get(const Elm_Gengrid_Item *obj);

/**
 * @brief Get the next item in a gengrid widget's internal list of items, given
 * a handle to one of those items.
 *
 * This returns the item placed after the @c item, on the container gengrid.
 *
 * @param[in] obj The object.
 *
 * @return The item after @c item, or @c NULL if there's none (and on errors)
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI Elm_Widget_Item *elm_gengrid_item_next_get(const Elm_Gengrid_Item *obj);

/**
 * @brief Control whether a given gengrid item is selected or not
 *
 * This API returns true for all the items selected in multi-select mode as
 * well.
 *
 * This sets the selected state of an item. If multi-selection is not enabled
 * on the containing gengrid and @c selected is true, any other previously
 * selected items will get unselected in favor of this new one.
 *
 * @param[in] obj The object.
 * @param[in] selected The selected state ($true selected, @c false not
 * selected)
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_selected_set(Elm_Gengrid_Item *obj, Eina_Bool selected);

/**
 * @brief Control whether a given gengrid item is selected or not
 *
 * This API returns true for all the items selected in multi-select mode as
 * well.
 *
 * This sets the selected state of an item. If multi-selection is not enabled
 * on the containing gengrid and @c selected is true, any other previously
 * selected items will get unselected in favor of this new one.
 *
 * @param[in] obj The object.
 *
 * @return The selected state ($true selected, @c false not selected)
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI Eina_Bool elm_gengrid_item_selected_get(const Elm_Gengrid_Item *obj);

/**
 * @brief Get the Gengrid Item class for the given Gengrid Item.
 *
 * This returns the Gengrid_Item_Class for the given item. It can be used to
 * examine the function pointers and item_style.
 *
 * @param[in] obj The object.
 *
 * @return Gengrid Item class for the given item
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI const Elm_Gengrid_Item_Class *elm_gengrid_item_item_class_get(const Elm_Gengrid_Item *obj);

/**
 * @brief Get the index of the item. It is only valid once displayed.
 *
 * @param[in] obj The object.
 *
 * @return The position inside the list of item.
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI int elm_gengrid_item_index_get(const Elm_Gengrid_Item *obj);

/**
 * @brief Get a given gengrid item's position, relative to the whole gengrid's
 * grid area.
 *
 * This returns the "logical" position of the item within the gengrid. For
 * example, $(0, 1) would stand for first row, second column.
 *
 * @param[in] obj The object.
 * @param[out] x Pointer to variable to store the item's <b>row number</b>.
 * @param[out] y Pointer to variable to store the item's <b>column number</b>.
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_pos_get(const Elm_Gengrid_Item *obj, unsigned int *x, unsigned int *y);

/**
 * @brief Control the gengrid item's select mode.
 *
 * (If getting mode fails, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * elm_gengrid_select_mode_set() changes item's select mode. -
 * ELM_OBJECT_SELECT_MODE_DEFAULT : The item will only call their selection
 * func and callback when first becoming selected. Any further clicks will do
 * nothing, unless you set always select mode. - ELM_OBJECT_SELECT_MODE_ALWAYS
 * : This means that, even if selected, every click will make the selected
 * callbacks be called. - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the
 * ability to select the item entirely and they will neither appear selected
 * nor call selected callback functions. - ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY
 * : This will apply no-finger-size rule with ELM_OBJECT_SELECT_MODE_NONE.
 * No-finger-size rule makes an item can be smaller than lower limit. Clickable
 * objects should be bigger than human touch point device (your finger) for
 * some touch or small screen devices. So it is enabled, the item can be shrink
 * than predefined finger-size value. And the item will be updated.
 *
 * @param[in] obj The object.
 * @param[in] mode The selected mode
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_select_mode_set(Elm_Gengrid_Item *obj, Elm_Object_Select_Mode mode);

/**
 * @brief Control the gengrid item's select mode.
 *
 * (If getting mode fails, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * elm_gengrid_select_mode_set() changes item's select mode. -
 * ELM_OBJECT_SELECT_MODE_DEFAULT : The item will only call their selection
 * func and callback when first becoming selected. Any further clicks will do
 * nothing, unless you set always select mode. - ELM_OBJECT_SELECT_MODE_ALWAYS
 * : This means that, even if selected, every click will make the selected
 * callbacks be called. - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the
 * ability to select the item entirely and they will neither appear selected
 * nor call selected callback functions. - ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY
 * : This will apply no-finger-size rule with ELM_OBJECT_SELECT_MODE_NONE.
 * No-finger-size rule makes an item can be smaller than lower limit. Clickable
 * objects should be bigger than human touch point device (your finger) for
 * some touch or small screen devices. So it is enabled, the item can be shrink
 * than predefined finger-size value. And the item will be updated.
 *
 * @param[in] obj The object.
 *
 * @return The selected mode
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI Elm_Object_Select_Mode elm_gengrid_item_select_mode_get(const Elm_Gengrid_Item *obj);

/**
 * @brief Custom size mode for non-homogeneous gengrid.
 *
 * Resize dimensions of a gengrid item.
 *
 * In case of a horizontal grid, only the widths only be resized and in case of
 * vertical only the heights can be resized. Item size should be set by
 * elm_gengrid_item_size_set() beforehand.
 *
 * The values set by elm_gengrid_item_size_set() will be used for the dimension
 * that remains fixed.
 *
 * @param[in] obj The object.
 * @param[in] w The item's width.
 * @param[in] h The item's height.
 *
 * @since 1.19
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_custom_size_set(Elm_Gengrid_Item *obj, int w, int h);

/**
 * @brief Custom size mode for non-homogeneous gengrid.
 *
 * Get the dimensions of a gengrid item.
 *
 * Gives the dimensions set with elm_gengrid_item_custom_size_set(). If the
 * item has not been modified values set with elm_gengrid_item_size_set() are
 * obtained.
 *
 * @param[in] obj The object.
 * @param[out] w The item's width.
 * @param[out] h The item's height.
 *
 * @since 1.19
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_custom_size_get(const Elm_Gengrid_Item *obj, int *w, int *h);

/**
 * @brief Show the portion of a gengrid's internal grid containing a given
 * item, immediately.
 *
 * This causes gengrid to redraw its viewport's contents to the region
 * containing the given @c item item, if it is not fully visible.
 *
 * @param[in] obj The object.
 * @param[in] type Where to position the item in the viewport.
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_show(Elm_Gengrid_Item *obj, Elm_Gengrid_Item_Scrollto_Type type);

/**
 * @brief Animatedly bring in, to the visible area of a gengrid, a given item
 * on it.
 *
 * This causes gengrid to jump to the given @c item and show it (by scrolling),
 * if it is not fully visible. This will use animation to do so and take a
 * period of time to complete.
 *
 * @param[in] obj The object.
 * @param[in] type Where to position the item in the viewport.
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_bring_in(Elm_Gengrid_Item *obj, Elm_Gengrid_Item_Scrollto_Type type);

/**
 * @brief Update the contents of a given gengrid item
 *
 * This updates an item by calling all the item class functions again to get
 * the contents, texts and states. Use this when the original item data has
 * changed and you want the changes to be reflected.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_update(Elm_Gengrid_Item *obj);

/**
 * @brief Update the part of an item
 *
 * This updates an item's part by calling item's fetching functions again to
 * get the contents, texts and states. Use this when the original item data has
 * changed and the changes are desired to be reflected. Second parts argument
 * is used for globbing to match '*', '?', and '.' It can be used at updating
 * multi fields.
 *
 * Use elm_gengrid_realized_items_update() to update an item's all property.
 *
 * @param[in] obj The object.
 * @param[in] parts The name of item's part
 * @param[in] itf The type of item's part type
 *
 * @since 1.15
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_fields_update(Elm_Gengrid_Item *obj, const char *parts, Elm_Gengrid_Item_Field_Type itf);

/**
 * @brief Update the item class of a gengrid item.
 *
 * This sets another class of the item, changing the way that it is displayed.
 * After changing the item class, elm_gengrid_item_update() is called on the
 * item @c it.
 *
 * @param[in] obj The object.
 * @param[in] itc The gengrid item class describing the function pointers and
 * the item style.
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_item_class_update(Elm_Gengrid_Item *obj, const Elm_Gengrid_Item_Class *itc);

/**
 * @brief Unset all contents fetched by the item class
 *
 * This instructs gengrid to release references to contents in the item,
 * meaning that they will no longer be managed by gengrid and are floating
 * "orphans" that can be re-used elsewhere if the user wants to.
 *
 * @param[in] obj The object.
 * @param[out] l The contents list to return.
 *
 * @since 1.18
 *
 * @ingroup Elm_Gengrid_Item_Group
 */
EAPI void elm_gengrid_item_all_contents_unset(Elm_Gengrid_Item *obj, Eina_List **l);

#endif
