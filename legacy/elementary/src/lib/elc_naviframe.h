/**
 * @defgroup Naviframe Naviframe
 * @ingroup Elementary
 *
 * @brief Naviframe is a kind of view manager for the applications.
 *
 * Naviframe provides functions to switch different pages with stack
 * mechanism. It means if one page(item) needs to be changed to the new one,
 * then naviframe would push the new page to its internal stack. Of course,
 * it can be back to the previous page by popping the top page. Naviframe
 * provides some transition effect while the pages are switching (same as
 * pager).
 *
 * Since each item could keep the different styles, users could keep the
 * same look & feel for the pages or different styles for the items in it's
 * application.
 * 
 * Default content parts of the naviframe that you can use content hooks for
 * are:
 * @li "default" - The main content of the current page
 * @li "icon" - An icon in the title area of the current page
 * @li "prev_btn" - A button of the current page to go to the previous page
 * @li "next_btn" - A button of the current page to go to the next page
 * 
 * Default text parts of the naviframe that you can use for are:
 * @li "default" - Title label in the title area of the current page
 * @li "subtitle" - Sub-title label in the title area of the current page
 *
 * Signals that you can add callbacks for are:
 * @li "transition,finished" - When the transition is finished in changing the
 * item
 * @li "title,clicked" - User clicked title area
 *
 * Item Signals that you can add callbacks for are:
 * @li "show,begin" - When the item is started to be top item.
 * @li "hide,finished" - When a new top item is finished to push onto the
 * item.
 *
 * Default content parts of the naviframe items that you can use content hooks
 * for are:
 * @li "default" - The main content of the page
 * @li "icon" - An icon in the title area
 * @li "prev_btn" - A button to go to the previous page
 * @li "next_btn" - A button to go to the next page
 *
 * Default text parts of the naviframe items that you can use for are:
 * @li "default" - Title label in the title area
 * @li "subtitle" - Sub-title label in the title area
 *
 * Supported elm_object common APIs.
 * @li elm_object_signal_emit
 * @li elm_object_part_text_set
 * @li elm_object_part_text_get
 * @li elm_object_part_content_set
 * @li elm_object_part_content_get
 * @li elm_object_part_content_unset
 *
 * Supported elm_object_item common APIs.
 * @li elm_object_item_part_text_set
 * @li elm_object_item_part_text_get
 * @li elm_object_item_part_content_set
 * @li elm_object_item_part_content_get
 * @li elm_object_item_part_content_unset
 * @li elm_object_item_signal_emit
 */

/**
 * @addtogroup Naviframe
 * @{
 */

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
 * @brief Push a new item to the top of the naviframe stack (and show it).
 *
 * @param obj The naviframe object
 * @param title_label The label in the title area. The name of the title
 *        label part is "elm.text.title"
 * @param prev_btn The button to go to the previous item. If it is NULL,
 *        then naviframe will create a back button automatically. The name of
 *        the prev_btn part is "elm.swallow.prev_btn"
 * @param next_btn The button to go to the next item. Or It could be just an
 *        extra function button. The name of the next_btn part is
 *        "elm.swallow.next_btn"
 * @param content The main content object. The name of content part is
 *        "elm.swallow.content"
 * @param item_style The current item style name. @c NULL would be default.
 * @return The created item or @c NULL upon failure.
 *
 * The item pushed becomes one page of the naviframe, this item will be
 * deleted when it is popped.
 *
 * @see also elm_naviframe_item_style_set()
 * @see also elm_naviframe_item_insert_before()
 * @see also elm_naviframe_item_insert_after()
 *
 * The following styles are available for this item:
 * @li @c "default"
 *
 * @ingroup Naviframe
 */
EAPI Elm_Object_Item *elm_naviframe_item_push(Evas_Object *obj, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style);

/**
 * @brief Insert a new item into the naviframe before item @p before.
 *
 * @param before The naviframe item to insert before.
 * @param title_label The label in the title area. The name of the title
 *        label part is "elm.text.title"
 * @param prev_btn The button to go to the previous item. If it is NULL,
 *        then naviframe will create a back button automatically. The name of
 *        the prev_btn part is "elm.swallow.prev_btn"
 * @param next_btn The button to go to the next item. Or It could be just an
 *        extra function button. The name of the next_btn part is
 *        "elm.swallow.next_btn"
 * @param content The main content object. The name of content part is
 *        "elm.swallow.content"
 * @param item_style The current item style name. @c NULL would be default.
 * @return The created item or @c NULL upon failure.
 *
 * The item is inserted into the naviframe straight away without any
 * transition operations. This item will be deleted when it is popped.
 *
 * @see also elm_naviframe_item_style_set()
 * @see also elm_naviframe_item_push()
 * @see also elm_naviframe_item_insert_after()
 *
 * The following styles are available for this item:
 * @li @c "default"
 *
 * @ingroup Naviframe
 */
EAPI Elm_Object_Item *elm_naviframe_item_insert_before(Elm_Object_Item *before, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style);

/**
 * @brief Insert a new item into the naviframe after item @p after.
 *
 * @param after The naviframe item to insert after.
 * @param title_label The label in the title area. The name of the title
 *        label part is "elm.text.title"
 * @param prev_btn The button to go to the previous item. If it is NULL,
 *        then naviframe will create a back button automatically. The name of
 *        the prev_btn part is "elm.swallow.prev_btn"
 * @param next_btn The button to go to the next item. Or It could be just an
 *        extra function button. The name of the next_btn part is
 *        "elm.swallow.next_btn"
 * @param content The main content object. The name of content part is
 *        "elm.swallow.content"
 * @param item_style The current item style name. @c NULL would be default.
 * @return The created item or @c NULL upon failure.
 *
 * The item is inserted into the naviframe straight away without any
 * transition operations. This item will be deleted when it is popped.
 *
 * @see also elm_naviframe_item_style_set()
 * @see also elm_naviframe_item_push()
 * @see also elm_naviframe_item_insert_before()
 *
 * The following styles are available for this item:
 * @li @c "default"
 *
 * @ingroup Naviframe
 */
EAPI Elm_Object_Item *elm_naviframe_item_insert_after(Elm_Object_Item *after, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style);

/**
 * @brief Pop an item that is on top of the stack
 *
 * @param obj The naviframe object
 * @return @c NULL or the content object(if the
 *         elm_naviframe_content_preserve_on_pop_get is true).
 *
 * This pops an item that is on the top(visible) of the naviframe, makes it
 * disappear, then deletes the item. The item that was underneath it on the
 * stack will become visible.
 *
 * @see also elm_naviframe_content_preserve_on_pop_get()
 *
 * @ingroup Naviframe
 */
EAPI Evas_Object     *elm_naviframe_item_pop(Evas_Object *obj);

/**
 * @brief Pop the items between the top and the above one on the given item.
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
 * @brief preserve the content objects when items are popped.
 *
 * @param obj The naviframe object
 * @param preserve Enable the preserve mode if EINA_TRUE, disable otherwise
 *
 * @see also elm_naviframe_content_preserve_on_pop_get()
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_content_preserve_on_pop_set(Evas_Object *obj, Eina_Bool preserve);

/**
 * @brief Get a value whether preserve mode is enabled or not.
 *
 * @param obj The naviframe object
 * @return If @c EINA_TRUE, preserve mode is enabled
 *
 * @see also elm_naviframe_content_preserve_on_pop_set()
 *
 * @ingroup Naviframe
 */
EAPI Eina_Bool        elm_naviframe_content_preserve_on_pop_get(const Evas_Object *obj);

/**
 * @brief Get a top item on the naviframe stack
 *
 * @param obj The naviframe object
 * @return The top item on the naviframe stack or @c NULL, if the stack is
 *         empty
 *
 * @ingroup Naviframe
 */
EAPI Elm_Object_Item *elm_naviframe_top_item_get(const Evas_Object *obj);

/**
 * @brief Get a bottom item on the naviframe stack
 *
 * @param obj The naviframe object
 * @return The bottom item on the naviframe stack or @c NULL, if the stack is
 *         empty
 *
 * @ingroup Naviframe
 */
EAPI Elm_Object_Item *elm_naviframe_bottom_item_get(const Evas_Object *obj);

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
 * @brief Show/Hide the title area
 *
 * @param it The naviframe item
 * @param visible If @c EINA_TRUE, title area will be visible, hidden
 *        otherwise
 *
 * When the title area is invisible, then the controls would be hidden so as     * to expand the content area to full-size.
 *
 * @see also elm_naviframe_item_title_visible_get()
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_item_title_visible_set(Elm_Object_Item *it, Eina_Bool visible);

/**
 * @brief Get a value whether title area is visible or not.
 *
 * @param it The naviframe item
 * @return If @c EINA_TRUE, title area is visible
 *
 * @see also elm_naviframe_item_title_visible_set()
 *
 * @ingroup Naviframe
 */
EAPI Eina_Bool        elm_naviframe_item_title_visible_get(const Elm_Object_Item *it);

/**
 * @brief Set creating prev button automatically or not
 *
 * @param obj The naviframe object
 * @param auto_pushed If @c EINA_TRUE, the previous button(back button) will
 *        be created internally when you pass the @c NULL to the prev_btn
 *        parameter in elm_naviframe_item_push
 *
 * @see also elm_naviframe_item_push()
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_prev_btn_auto_pushed_set(Evas_Object *obj, Eina_Bool auto_pushed);

/**
 * @brief Get a value whether prev button(back button) will be auto pushed or
 *        not.
 *
 * @param obj The naviframe object
 * @return If @c EINA_TRUE, prev button will be auto pushed.
 *
 * @see also elm_naviframe_item_push()
 *           elm_naviframe_prev_btn_auto_pushed_set()
 *
 * @ingroup Naviframe
 */
EAPI Eina_Bool        elm_naviframe_prev_btn_auto_pushed_get(const Evas_Object *obj);

/**
 * @brief Get a list of all the naviframe items.
 *
 * @param obj The naviframe object
 * @return An Eina_List of naviframe items, #Elm_Object_Item,
 * or @c NULL on failure.
 * @note The returned list MUST be freed.
 *
 * @ingroup Naviframe
 */
EAPI Eina_List *elm_naviframe_items_get(const Evas_Object *obj) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set the event enabled when pushing/popping items
 *
 * If @c enabled is EINA_TRUE, the contents of the naviframe item will
 * receives events from mouse and keyboard during view changing such as
 * item push/pop.
 *
 * @param obj The naviframe object
 * @param enabled Events are received when enabled is @c EINA_TRUE, and
 * ignored otherwise.
 *
 * @warning Events will be blocked by calling evas_object_freeze_events_set()
 * internally. So don't call the API whiling pushing/popping items.
 *
 * @see elm_naviframe_event_enabled_get()
 * @see evas_object_freeze_events_set()
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_event_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * @brief Get the value of event enabled status.
 *
 * @param obj The naviframe object
 * @return EINA_TRUE, when event is enabled
 *
 * @see elm_naviframe_event_enabled_set()
 *
 * @ingroup Naviframe
 */
EAPI Eina_Bool        elm_naviframe_event_enabled_get(const Evas_Object *obj);

/**
 * @brief Set the default item style.
 *
 * Default item style will be used with items who's style is NULL
 *
 * @param obj The naviframe object
 * @param style The style
 *
 * @ingroup Naviframe
 */
EAPI void             elm_naviframe_item_style_default_set(Evas_Object *obj, const char *style);

/**
 * @brief Get the default item style
 *
 * @param obj The naviframe object
 * @return the default item style
 *
 * @see elm_naviframe_item_style_default_set()
 *
 * @ingroup Naviframe
 */
EAPI const char      *elm_naviframe_item_style_default_get(const Evas_Object *obj);

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
   elm_naviframe_item_title_visible_set(it, EINA_FALSE);
   return it;
}

/**
 * @brief Simple version of item_promote.
 *
 * @see elm_naviframe_item_promote
 */
EAPI void             elm_naviframe_item_simple_promote(Evas_Object *obj, Evas_Object *content);

/**
 * @}
 */
