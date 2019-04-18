#ifndef _ELM_GENLIST_ITEM_EO_LEGACY_H_
#define _ELM_GENLIST_ITEM_EO_LEGACY_H_

#ifndef _ELM_GENLIST_ITEM_EO_CLASS_TYPE
#define _ELM_GENLIST_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Genlist_Item;

#endif

#ifndef _ELM_GENLIST_ITEM_EO_TYPES
#define _ELM_GENLIST_ITEM_EO_TYPES


#endif

/**
 * @brief Get the previous item in a genlist widget's internal list of items,
 * given a handle to one of those items.
 *
 * This returns the item placed before the @c item, on the container genlist.
 *
 * If filter is set on genlist, this returns the filtered item placed before
 * @c item in the list.
 *
 * Note that parent / child relationship is not taken into account, the
 * previous visual item is always returned, could it be a parent, a child or a
 * group item.
 *
 * NULL is returned if called on the first item.
 *
 * @param[in] obj The object.
 *
 * @return The item before @c item, or @c null if there's none (and on errors).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Elm_Widget_Item *elm_genlist_item_prev_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the next item in a genlist widget's internal lis of items, given
 * a handle to one of those items.
 *
 * This returns the item placed after the @c item, on the container genlist.
 *
 * If filter is set on genlist, this returns the filtered item placed after
 * @c item in the list.
 *
 * Note that parent / child relationship is not taken into account, the next
 * visual item is always returned, could it be a parent, a child or a group
 * item.
 *
 * NULL is returned if called on the last item.
 *
 * @param[in] obj The object.
 *
 * @return The item after @c item, or @c null if there's none (and on errors).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Elm_Widget_Item *elm_genlist_item_next_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the parent item of the given item
 *
 * This returns the item that was specified as parent of the item @c it on @ref
 * elm_genlist_item_append and insertion related functions.
 *
 * @param[in] obj The object.
 *
 * @return The parent of the item or @c null if it has no parent.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Elm_Widget_Item *elm_genlist_item_parent_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the list of subitems of a given item
 *
 * This returns the list of subitems that an item possesses. It cannot be
 * changed.
 *
 * @param[in] obj The object.
 *
 * @return The list of subitems, @c null on error.
 *
 * @since 1.9
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI const Eina_List *elm_genlist_item_subitems_get(const Elm_Genlist_Item *obj);

/**
 * @brief Set whether a given genlist item is selected or not
 *
 * This sets the selected state of an item. If multi selection is not enabled
 * on the containing genlist and @c selected is @c true, any other previously
 * selected items will get unselected in favor of this new one.
 *
 * @param[in] obj The object.
 * @param[in] selected The selected state ($true selected, @c false not
 * selected).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_selected_set(Elm_Genlist_Item *obj, Eina_Bool selected);

/**
 * @brief Get whether a given genlist item is selected or not.
 *
 * @param[in] obj The object.
 *
 * @return The selected state ($true selected, @c false not selected).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Eina_Bool elm_genlist_item_selected_get(const Elm_Genlist_Item *obj);

/**
 * @brief Sets the expanded state of an item.
 *
 * This function flags the item of type #ELM_GENLIST_ITEM_TREE as expanded or
 * not.
 *
 * The theme will respond to this change visually, and a signal "expanded" or
 * "contracted" will be sent from the genlist with a pointer to the item that
 * has been expanded/contracted.
 *
 * Calling this function won't show or hide any child of this item (if it is a
 * parent). You must manually delete and create them on the callbacks of the
 * "expanded" or "contracted" signals.
 *
 * @param[in] obj The object.
 * @param[in] expanded The expanded state ($true expanded, @c false not
 * expanded).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_expanded_set(Elm_Genlist_Item *obj, Eina_Bool expanded);

/**
 * @brief Get the expanded state of an item
 *
 * This gets the expanded state of an item.
 *
 * @param[in] obj The object.
 *
 * @return The expanded state ($true expanded, @c false not expanded).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Eina_Bool elm_genlist_item_expanded_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the depth of expanded item.
 *
 * @param[in] obj The object.
 *
 * @return The depth of expanded item.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI int elm_genlist_item_expanded_depth_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the Genlist Item class for the given Genlist Item.
 *
 * This returns the Genlist_Item_Class for the given item. It can be used to
 * examine the function pointers and item_style.
 *
 * @param[in] obj The object.
 *
 * @return Genlist Item class for the given item.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI const Elm_Genlist_Item_Class *elm_genlist_item_item_class_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the index of the item. It is only valid once displayed.
 *
 * The index start from 1.
 *
 * @param[in] obj The object.
 *
 * @return The position inside the list of item.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI int elm_genlist_item_index_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the item's decorate mode.
 *
 * This function just returns the name of the item's decorate mode.
 *
 * @param[in] obj The object.
 *
 * @return Name of the item's decorate mode.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI const char *elm_genlist_item_decorate_mode_get(const Elm_Genlist_Item *obj);

/**
 * @brief Set the flip state of a given genlist item.
 *
 * This function sets the flip state of a given genlist item. Flip mode
 * overrides current item object. It can be used for on-the-fly item replace.
 * Flip mode can be used with/without decorate mode.
 *
 * @param[in] obj The object.
 * @param[in] flip The flip mode.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_flip_set(Elm_Genlist_Item *obj, Eina_Bool flip);

/**
 * @brief Get the flip state of a given genlist item.
 *
 * This function returns the flip state of a given genlist item. If the
 * parameter is invalid, it returns @c false.
 *
 * @param[in] obj The object.
 *
 * @return The flip mode.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Eina_Bool elm_genlist_item_flip_get(const Elm_Genlist_Item *obj);

/**
 * @brief Set the genlist item's select mode.
 *
 * ELM_OBJECT_SELECT_MODE_DEFAULT means that the item will only call their
 * selection func and callback when first becoming selected. Any further clicks
 * will do nothing, unless you set always select mode.
 *
 * ELM_OBJECT_SELECT_MODE_ALWAYS means that even if selected, every click will
 * make the selected callbacks be called.
 *
 * ELM_OBJECT_SELECT_MODE_NONE will turn off the ability to select the item
 * entirely and they will neither appear selected nor call selected callback
 * functions.
 *
 * ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY will apply no-finger-size rule with
 * ELM_OBJECT_SELECT_MODE_NONE. No-finger-size rule makes an item can be
 * smaller than lower limit. Clickable objects should be bigger than human
 * touch point device (your finger) for some touch or small screen devices. So
 * it is enabled, the item can be shrink than predefined finger-size value. And
 * the item will be updated.
 *
 * @param[in] obj The object.
 * @param[in] mode The selected mode.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_select_mode_set(Elm_Genlist_Item *obj, Elm_Object_Select_Mode mode);

/**
 * @brief Get the genlist item's select mode.
 *
 * It's ELM_OBJECT_SELECT_MODE_MAX on failure.
 *
 * @param[in] obj The object.
 *
 * @return The selected mode.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Elm_Object_Select_Mode elm_genlist_item_select_mode_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the Item's type.
 *
 * This function returns the item's type. Normally the item's type. If it
 * failed, return value is ELM_GENLIST_ITEM_MAX.
 *
 * @param[in] obj The object.
 *
 * @return Item type.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Elm_Genlist_Item_Type elm_genlist_item_type_get(const Elm_Genlist_Item *obj);

/**
 * @brief Set whether a given genlist item is pinned or not
 *
 * This sets a genlist item as pinned so that it will be always available in
 * the viewport available for user interaction. Group items cannot be pinned.
 * Also when a new item is pinned, the current pinned item will get unpinned.
 * Item pinning cannot be done in reorder mode too.
 *
 * @param[in] obj The object.
 * @param[in] pin The item pin state state ($true pin item, @c false unpin
 * item).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_pin_set(Elm_Genlist_Item *obj, Eina_Bool pin);

/**
 * @brief Get whether a given genlist item is pinned or not.
 *
 * @param[in] obj The object.
 *
 * @return The item pin state state ($true pin item, @c false unpin item).
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI Eina_Bool elm_genlist_item_pin_get(const Elm_Genlist_Item *obj);

/**
 * @brief Get the number of subitems of a given item.
 *
 * This returns the number of subitems that an item possesses.
 *
 * @param[in] obj The object.
 *
 * @return The number of subitems, 0 on error.
 *
 * @since 1.9
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI unsigned int elm_genlist_item_subitems_count(Elm_Genlist_Item *obj);

/**
 * @brief Remove all sub-items (children) of the given item.
 *
 * This removes all items that are children (and their descendants) of the
 * given item @c it.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_subitems_clear(Elm_Genlist_Item *obj);

/** Promote an item to the top of the list.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_promote(Elm_Genlist_Item *obj);

/** Demote an item to the end of the list.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_demote(Elm_Genlist_Item *obj);

/**
 * @brief Show the portion of a genlist's internal list containing a given
 * item, immediately.
 *
 * This causes genlist to jump to the given item @c it and show it (by jumping
 * to that position), if it is not fully visible.
 *
 * @param[in] obj The object.
 * @param[in] type The position to bring in, the given item to. @ref
 * Elm_Genlist_Item_Scrollto_Type.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_show(Elm_Genlist_Item *obj, Elm_Genlist_Item_Scrollto_Type type);

/**
 * @brief Animatedly bring in, to the visible area of a genlist, a given item
 * on it.
 *
 * This causes genlist to jump to the given item @c it and show it (by
 * animatedly scrolling), if it is not fully visible. This may use animation
 * and take a some time to do so.
 *
 * @param[in] obj The object.
 * @param[in] type The position to bring in, the given item to. @ref
 * Elm_Genlist_Item_Scrollto_Type.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_bring_in(Elm_Genlist_Item *obj, Elm_Genlist_Item_Scrollto_Type type);

/**
 * @brief Unset all contents fetched by the item class.
 *
 * This instructs genlist to release references to contents in the item,
 * meaning that they will no longer be managed by genlist and are floating
 * "orphans" that can be re-used elsewhere if the user wants to.
 *
 * @param[in] obj The object.
 * @param[out] l The contents list to return.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_all_contents_unset(Elm_Genlist_Item *obj, Eina_List **l);

/**
 * @brief Update all the contents of an item.
 *
 * This updates an item by calling all the item class functions again to get
 * the contents, texts and states. Use this when the original item data has
 * changed and the changes are desired to be reflected.
 *
 * Use elm_genlist_realized_items_update() to update all already realized
 * items.
 *
 * @note This also updates internal genlist item object (edje_object as of
 * now). So when this is called between mouse down and mouse up, mouse up event
 * will be ignored because edje_object is deleted and created again by this
 * API. If you want to avoid this, please use @ref
 * elm_genlist_item_fields_update.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_update(Elm_Genlist_Item *obj);

/**
 * @brief Update the part of an item.
 *
 * This updates an item's part by calling item's fetching functions again to
 * get the contents, texts and states. Use this when the original item data has
 * changed and the changes are desired to be reflected. Second part argument is
 * used for globbing to match '*', '?', and '.' It can be used at updating
 * multi fields.
 *
 * Use @ref elm_genlist_realized_items_update to update an item's all property.
 *
 * @param[in] obj The object.
 * @param[in] parts The name of item's part.
 * @param[in] itf The type of item's part type.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_fields_update(Elm_Genlist_Item *obj, const char *parts, Elm_Genlist_Item_Field_Type itf);

/**
 * @brief Update the item class of an item.
 *
 * This sets another class of the item, changing the way that it is displayed.
 * After changing the item class @ref elm_genlist_item_update is called on the
 * item @c it.
 *
 * @param[in] obj The object.
 * @param[in] itc The item class for the item.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_item_class_update(Elm_Genlist_Item *obj, const Elm_Genlist_Item_Class *itc);

/**
 * @brief Activate a genlist mode on an item.
 *
 * A genlist mode is a different way of selecting an item. Once a mode is
 * activated on an item, any other selected item is immediately unselected.
 * This feature provides an easy way of implementing a new kind of animation
 * for selecting an item, without having to entirely rewrite the item style
 * theme. However, the elm_genlist_selected_* API can't be used to get what
 * item is activate for a mode.
 *
 * The current item style will still be used, but applying a genlist mode to an
 * item will select it using a different kind of animation.
 *
 * The current active item for a mode can be found by @ref
 * elm_genlist_decorated_item_get.
 *
 * Only one mode can be active at any time, and for only one item. Genlist
 * handles deactivating other items when one item is activated. A mode is
 * defined in the genlist theme (edc), and more modes can easily be added. A
 * mode style and the genlist item style are different things. They can be
 * combined to provide a default style to the item, with some kind of animation
 * for that item when the mode is activated.
 *
 * When a mode is activated on an item, a new view for that item is created.
 * The theme of this mode defines the animation that will be used to transit
 * the item from the old view to the new view. This second (new) view will be
 * active for that item while the mode is active on the item, and will be
 * destroyed after the mode is totally deactivated from that item.
 *
 * @param[in] obj The object.
 * @param[in] decorate_it_type Mode name.
 * @param[in] decorate_it_set Boolean to define set or unset mode.
 *
 * @ingroup Elm_Genlist_Item_Group
 */
EAPI void elm_genlist_item_decorate_mode_set(Elm_Genlist_Item *obj, const char *decorate_it_type, Eina_Bool decorate_it_set);

#endif
