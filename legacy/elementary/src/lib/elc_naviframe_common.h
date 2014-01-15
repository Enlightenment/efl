/**
 * @typedef Elm_Naviframe_Item_Pop_Cb
 *
 * Pop callback called when @c it is going to be popped. @c data is user
 * specific data. If it returns the @c EINA_FALSE in the callback, item popping
 * will be cancelled.
 *
 * @see elm_naviframe_item_pop_cb_set()
 *
 * @since 1.8
 */
typedef Eina_Bool (*Elm_Naviframe_Item_Pop_Cb)(void *data, Elm_Object_Item *it);

/**
 * @brief Add a new Naviframe object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Naviframe
 */
EAPI Evas_Object     *elm_naviframe_add(Evas_Object *parent);

/**
 * @brief Pop the items between the top and the above one on the given item.
 *
 * The items between the top and the given item will be deleted first,
 * and then the top item will be popped at last.
 *
 * @param it The naviframe item
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_item_pop_to(Elm_Object_Item *it);

/**
 * Promote an item already in the naviframe stack to the top of the stack
 *
 * @param it The naviframe item
 *
 * This will take the indicated item and promote it to the top of the stack
 * as if it had been pushed there. The item must already be inside the
 * naviframe stack to work.
 *
 */
EAPI void             elm_naviframe_item_promote(Elm_Object_Item *it);

/**
 * @brief Set an item style
 *
 * @param it The naviframe item
 * @param item_style The current item style name. @c NULL would be default
 *
 * The following styles are available for this item:
 * @li @c "default"
 *
 * @see also elm_naviframe_item_style_get()
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_item_style_set(Elm_Object_Item *it, const char *item_style);

/**
 * @brief Get an item style
 *
 * @param it The naviframe item
 * @return The current item style name
 *
 * @see also elm_naviframe_item_style_set()
 *
 * @ingroup Naviframe
 */
EAPI const char      *elm_naviframe_item_style_get(const Elm_Object_Item *it);

/**
 * @brief Enable/Disable the title area with transition effect
 *
 * @param it The naviframe item
 * @param enabled If @c EINA_TRUE, title area will be enabled, disabled
 *        otherwise
 * @param transition If @c EINA_TRUE, transition effect of the title will be
 *        visible, invisible otherwise
 *
 * When the title area is disabled, then the controls would be hidden so as
 * to expand the content area to full-size.
 *
 * @see also elm_naviframe_item_title_enabled_get()
 * @see also elm_naviframe_item_title_visible_set()
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_item_title_enabled_set(Elm_Object_Item *it, Eina_Bool enabled, Eina_Bool transition);

/**
 * @brief Get a value whether title area is enabled or not.
 *
 * @param it The naviframe item
 * @return If @c EINA_TRUE, title area is enabled
 *
 * @see also elm_naviframe_item_title_enabled_set()
 *
 * @ingroup Naviframe
 */
EAPI Eina_Bool        elm_naviframe_item_title_enabled_get(const Elm_Object_Item *it);

/**
 * @brief Set a function to be called when @c it of the naviframe is going to be
 * popped.
 *
 * @param it The item to set the callback on
 * @param func the callback function.
 *
 * @warning Don't set "clicked" callback to the prev button additionally if the
 * function does an exact same logic with this @c func. When hardware back key
 * is pressed then both callbacks will be called.
 *
 * @since 1.8
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_item_pop_cb_set(Elm_Object_Item *it, Elm_Naviframe_Item_Pop_Cb func, void *data);

Elm_Object_Item *elm_naviframe_item_push(Evas_Object *obj, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style);
/**
 * @brief Simple version of item_push.
 *
 * @see elm_naviframe_item_push
 */
static inline Elm_Object_Item *
elm_naviframe_item_simple_push(Evas_Object *obj, Evas_Object *content)
{
   Elm_Object_Item *it;
   it = elm_naviframe_item_push(obj, NULL, NULL, NULL, content, NULL);
   elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
   return it;
}
