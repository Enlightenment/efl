#define ELM_GENLIST_ITEM_CLASS_VERSION ELM_GEN_ITEM_CLASS_VERSION
#define ELM_GENLIST_ITEM_CLASS_HEADER ELM_GEN_ITEM_CLASS_HEADER

/**
 * Defines if the item is of any special type (has subitems or it's the
 * index of a group), or is just a simple item.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENLIST_ITEM_NONE = 0, /**< simple item */
   ELM_GENLIST_ITEM_TREE = (1 << 0), /**< this may be expanded and have child items. */
   ELM_GENLIST_ITEM_GROUP = (1 << 1), /**< an index item of a group of items. this item can have child items. */

   ELM_GENLIST_ITEM_MAX = (1 << 2)
} Elm_Genlist_Item_Type;

/**
 * Defines the type of the item part
 * Used while updating item's parts
 * It can be used at updating multi fields.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENLIST_ITEM_FIELD_ALL = 0,
   ELM_GENLIST_ITEM_FIELD_TEXT = (1 << 0),
   ELM_GENLIST_ITEM_FIELD_CONTENT = (1 << 1),
   ELM_GENLIST_ITEM_FIELD_STATE = (1 << 2)
} Elm_Genlist_Item_Field_Type;

/**
 * Defines where to position the item in the genlist.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENLIST_ITEM_SCROLLTO_NONE = 0,   /**< no scrollto */
   ELM_GENLIST_ITEM_SCROLLTO_IN = (1 << 0),   /**< to the nearest viewport */
   ELM_GENLIST_ITEM_SCROLLTO_TOP = (1 << 1),   /**< to the top of viewport */
   ELM_GENLIST_ITEM_SCROLLTO_MIDDLE = (1 << 2)   /**< to the middle of viewport */
} Elm_Genlist_Item_Scrollto_Type;

/**
 * @see Elm_Gen_Item_Class
 */
typedef Elm_Gen_Item_Class Elm_Genlist_Item_Class;

/**
 * @see Elm_Gen_Item_Text_Get_Cb
 */
typedef Elm_Gen_Item_Text_Get_Cb Elm_Genlist_Item_Text_Get_Cb;

/**
 * @see Elm_Gen_Item_Content_Get_Cb
 */
typedef Elm_Gen_Item_Content_Get_Cb Elm_Genlist_Item_Content_Get_Cb;

/**
 * @see Elm_Gen_Item_State_Get_Cb
 */
typedef Elm_Gen_Item_State_Get_Cb Elm_Genlist_Item_State_Get_Cb;

/**
 * @see Elm_Gen_Item_Del_Cb
 */
typedef Elm_Gen_Item_Del_Cb Elm_Genlist_Item_Del_Cb;

/**
 * Get the @b next item in a genlist widget's internal list of items,
 * given a handle to one of those items.
 *
 * @param it The genlist item to fetch next from
 * @return The item after @p item, or @c NULL if there's none (and
 * on errors)
 *
 * This returns the item placed after the @p item, on the container
 * genlist.
 *
 * @see elm_genlist_item_prev_get()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_next_get(const Elm_Object_Item *it);

/**
 * Get the @b previous item in a genlist widget's internal list of items,
 * given a handle to one of those items.
 *
 * @param it The genlist item to fetch previous from
 * @return The item before @p item, or @c NULL if there's none (and
 * on errors)
 *
 * This returns the item placed before the @p item, on the container
 * genlist.
 *
 * @see elm_genlist_item_next_get()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_prev_get(const Elm_Object_Item *it);

/**
 * Set whether a given genlist item is selected or not
 *
 * @param it The item
 * @param selected Use @c EINA_TRUE, to make it selected, @c
 * EINA_FALSE to make it unselected
 *
 * This sets the selected state of an item. If multi selection is
 * not enabled on the containing genlist and @p selected is @c
 * EINA_TRUE, any other previously selected items will get
 * unselected in favor of this new one.
 *
 * @see elm_genlist_item_selected_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * Get whether a given genlist item is selected or not
 *
 * @param it The item
 * @return @c EINA_TRUE, if it's selected, @c EINA_FALSE otherwise
 *
 * @see elm_genlist_item_selected_set() for more details
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_item_selected_get(const Elm_Object_Item *it);

/**
 * Show the portion of a genlist's internal list containing a given
 * item, immediately.
 *
 * @param it The item to display
 * @param type The position to bring in, the given item to.
 *             @ref Elm_Genlist_Item_Scrollto_Type
 *
 * This causes genlist to jump to the given item @p it and show it (by
 * jumping to that position), if it is not fully visible.
 *
 * @see elm_genlist_item_bring_in()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_show(Elm_Object_Item *it, Elm_Genlist_Item_Scrollto_Type type);

/**
 * Animatedly bring in, to the visible area of a genlist, a given
 * item on it.
 *
 * @param it The item to display
 * @param type The position to bring in, the given item to.
 *             @ref Elm_Genlist_Item_Scrollto_Type
 *
 * This causes genlist to jump to the given item @p it and show it (by
 * animatedly scrolling), if it is not fully visible.
 * This may use animation and take a some time to do so.
 *
 * @see elm_genlist_item_show()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_bring_in(Elm_Object_Item *it, Elm_Genlist_Item_Scrollto_Type type);

/**
 * Update all the contents of an item
 *
 * @param it The item
 *
 * This updates an item by calling all the item class functions again
 * to get the contents, texts and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 *
 * Use elm_genlist_realized_items_update() to update all already realized
 * items.
 *
 * @note This also updates internal genlist item object(edje_object as of now).
 * So when this is called between mouse down and mouse up, mouse up event will
 * be ignored because edje_object is deleted and created again by this API. If
 * you want to avoid this, please use @ref elm_genlist_item_fields_update.
 *
 * @see elm_genlist_realized_items_update()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_update(Elm_Object_Item *it);

/**
 * Update the item class of an item
 *
 * @param it The item
 * @param itc The item class for the item
 *
 * This sets another class of the item, changing the way that it is
 * displayed. After changing the item class, elm_genlist_item_update() is
 * called on the item @p it.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_item_class_update(Elm_Object_Item *it, const Elm_Genlist_Item_Class *itc);

/**
 * Get the Genlist Item class for the given Genlist Item.
 *
 * @param it The genlist item
 *
 * This returns the Genlist_Item_Class for the given item. It can be used to
 * examine the function pointers and item_style.
 *
 * @ingroup Genlist
 */
EAPI const Elm_Genlist_Item_Class *elm_genlist_item_item_class_get(const Elm_Object_Item *it);

/**
 * Get the index of the item. It is only valid once displayed.
 *
 * @param it a genlist item
 * @return the position inside the list of item.
 *
 * @ingroup Genlist
 */
EAPI int                           elm_genlist_item_index_get(const Elm_Object_Item *it);

/**
 * Create a new genlist item class in a given genlist widget.
 *
 * @return New allocated genlist item class.
 *
 * This adds genlist item class for the genlist widget. When adding an item,
 * genlist_item_{append, prepend, insert} function needs item class of the item.
 * Given callback parameters are used at retrieving {text, content} of
 * added item. Set as NULL if it's not used.
 * If there's no available memory, return can be NULL.
 *
 * @see elm_genlist_item_class_free()
 * @see elm_genlist_item_append()
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item_Class *elm_genlist_item_class_new(void);

/**
 * Remove an item class in a given genlist widget.
 *
 * @param itc The itc to be removed.
 *
 * This removes item class from the genlist widget.
 * Whenever it has no more references to it, item class is going to be freed.
 * Otherwise it just decreases its reference count.
 *
 * @see elm_genlist_item_class_new()
 * @see elm_genlist_item_class_ref()
 * @see elm_genlist_item_class_unref()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_class_free(Elm_Genlist_Item_Class *itc);

/**
 * Increments object reference count for the item class.
 *
 * @param itc The given item class object to reference
 *
 * This API just increases its reference count for item class management.
 *
 * @see elm_genlist_item_class_unref()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_class_ref(Elm_Genlist_Item_Class *itc);

/**
 * Decrements object reference count for the item class.
 *
 * @param itc The given item class object to reference
 *
 * This API just decreases its reference count for item class management.
 * Reference count can't be less than 0.
 *
 * @see elm_genlist_item_class_ref()
 * @see elm_genlist_item_class_free()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_class_unref(Elm_Genlist_Item_Class *itc);

/**
 * Set the text to be shown in a given genlist item's tooltips.
 *
 * @param it The genlist item
 * @param text The text to set in the content
 *
 * This call will setup the text to be used as tooltip to that item
 * (analogous to elm_object_tooltip_text_set(), but being item
 * tooltips with higher precedence than object tooltips). It can
 * have only one tooltip at a time, so any previous tooltip data
 * will get removed.
 *
 * In order to set a content or something else as a tooltip, look at
 * elm_genlist_item_tooltip_content_cb_set().
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in a given genlist item's tooltips
 *
 * @param it The genlist item.
 * @param func The function returning the tooltip contents.
 * @param data What to provide to @a func as callback data/context.
 * @param del_cb Called when data is not needed anymore, either when
 *        another callback replaces @p func, the tooltip is unset with
 *        elm_genlist_item_tooltip_unset() or the owner @p item
 *        dies. This callback receives as its first parameter the
 *        given @p data, being @c event_info the item handle.
 *
 * This call will setup the tooltip's contents to @p item
 * (analogous to elm_object_tooltip_content_cb_set(), but being
 * item tooltips with higher precedence than object tooltips). It
 * can have only one tooltip at a time, so any previous tooltip
 * content will get removed. @p func (with @p data) will be called
 * every time Elementary needs to show the tooltip and it should
 * return a valid Evas object, which will be fully managed by the
 * tooltip system, getting deleted when the tooltip is gone.
 *
 * In order to set just a text as a tooltip, look at
 * elm_genlist_item_tooltip_text_set().
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset a tooltip from a given genlist item
 *
 * @param it genlist item to remove a previously set tooltip from.
 *
 * This call removes any tooltip set on @p item. The callback
 * provided as @c del_cb to
 * elm_genlist_item_tooltip_content_cb_set() will be called to
 * notify it is not used anymore (and have resources cleaned, if
 * need be).
 *
 * @see elm_genlist_item_tooltip_content_cb_set()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given genlist item's tooltip.
 *
 * @param it genlist item with tooltip set
 * @param style the <b>theme style</b> to use on tooltips (e.g. @c
 * "default", @c "transparent", etc)
 *
 * Tooltips can have <b>alternate styles</b> to be displayed on,
 * which are defined by the theme set on Elementary. This function
 * works analogously as elm_object_tooltip_style_set(), but here
 * applied only to genlist item objects. The default style for
 * tooltips is @c "default".
 *
 * @note before you set a style you should define a tooltip with
 *       elm_genlist_item_tooltip_content_cb_set() or
 *       elm_genlist_item_tooltip_text_set()
 *
 * @see elm_genlist_item_tooltip_style_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style set a given genlist item's tooltip.
 *
 * @param it genlist item with tooltip already set on.
 * @return style the theme style in use, which defaults to
 *         "default". If the object does not have a tooltip set,
 *         then @c NULL is returned.
 *
 * @see elm_genlist_item_tooltip_style_set() for more details
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_genlist_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param it The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_genlist_item_tooltip_window_mode_get(const Elm_Object_Item *it);

/**
 * Set the type of mouse pointer/cursor decoration to be shown,
 * when the mouse pointer is over the given genlist widget item
 *
 * @param it genlist item to customize cursor on
 * @param cursor the cursor type's name
 *
 * This function works analogously as elm_object_cursor_set(), but
 * here the cursor's changing area is restricted to the item's
 * area, and not the whole widget's. Note that that item cursors
 * have precedence over widget cursors, so that a mouse over @p
 * item will always show cursor @p type.
 *
 * If this function is called twice for an object, a previously set
 * cursor will be unset on the second call.
 *
 * @see elm_object_cursor_set()
 * @see elm_genlist_item_cursor_get()
 * @see elm_genlist_item_cursor_unset()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/**
 * Get the type of mouse pointer/cursor decoration set to be shown,
 * when the mouse pointer is over the given genlist widget item
 *
 * @param it genlist item with custom cursor set
 * @return the cursor type's name or @c NULL, if no custom cursors
 * were set to @p item (and on errors)
 *
 * @see elm_object_cursor_get()
 * @see elm_genlist_item_cursor_set() for more details
 * @see elm_genlist_item_cursor_unset()
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset any custom mouse pointer/cursor decoration set to be
 * shown, when the mouse pointer is over the given genlist widget
 * item, thus making it show the @b default cursor again.
 *
 * @param it a genlist item
 *
 * Use this call to undo any custom settings on this item's cursor
 * decoration, bringing it back to defaults (no custom style set).
 *
 * @see elm_object_cursor_unset()
 * @see elm_genlist_item_cursor_set() for more details
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given custom cursor set for a
 * genlist item.
 *
 * @param it genlist item with custom cursor set
 * @param style the <b>theme style</b> to use (e.g. @c "default",
 * @c "transparent", etc)
 *
 * This function only makes sense when one is using custom mouse
 * cursor decorations <b>defined in a theme file</b> , which can
 * have, given a cursor name/type, <b>alternate styles</b> on
 * it. It works analogously as elm_object_cursor_style_set(), but
 * here applied only to genlist item objects.
 *
 * @warning Before you set a cursor style you should have defined a
 *       custom cursor previously on the item, with
 *       elm_genlist_item_cursor_set()
 *
 * @see elm_genlist_item_cursor_engine_only_set()
 * @see elm_genlist_item_cursor_style_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the current @b style set for a given genlist item's custom
 * cursor
 *
 * @param it genlist item with custom cursor set.
 * @return style the cursor style in use. If the object does not
 *         have a cursor set, then @c NULL is returned.
 *
 * @see elm_genlist_item_cursor_style_set() for more details
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Set if the (custom) cursor for a given genlist item should be
 * searched in its theme, also, or should only rely on the
 * rendering engine.
 *
 * @param it item with custom (custom) cursor already set on
 * @param engine_only Use @c EINA_TRUE to have cursors looked for
 * only on those provided by the rendering engine, @c EINA_FALSE to
 * have them searched on the widget's theme, as well.
 *
 * @note This call is of use only if you've set a custom cursor
 * for genlist items, with elm_genlist_item_cursor_set().
 *
 * @note By default, cursors will only be looked for between those
 * provided by the rendering engine.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only);

/**
 * Get if the (custom) cursor for a given genlist item is being
 * searched in its theme, also, or is only relying on the rendering
 * engine.
 *
 * @param it a genlist item
 * @return @c EINA_TRUE, if cursors are being looked for only on
 * those provided by the rendering engine, @c EINA_FALSE if they
 * are being searched on the widget's theme, as well.
 *
 * @see elm_genlist_item_cursor_engine_only_set(), for more details
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_item_cursor_engine_only_get(const Elm_Object_Item *it);

/**
 * Get the parent item of the given item
 *
 * @param it The item
 * @return The parent of the item or @c NULL if it has no parent.
 *
 * This returns the item that was specified as parent of the item @p it on
 * elm_genlist_item_append() and insertion related functions.
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Item             *elm_genlist_item_parent_get(const Elm_Object_Item *it);

/**
 * Get the number of subitems of a given item
 *
 * @param it The item
 * @return The number of subitems, 0 on error
 *
 * This returns the number of subitems that an item possesses.
 *
 * @since 1.9
 * @ingroup Genlist
 */
EAPI unsigned int                 elm_genlist_item_subitems_count(const Elm_Object_Item *it);

/**
 * Get the list of subitems of a given item
 *
 * @param it The item
 * @return The list of subitems, @c NULL on error
 *
 * This returns the list of subitems that an item possesses. It cannot be changed.
 *
 * @since 1.9
 * @ingroup Genlist
 */
EAPI const Eina_List             *elm_genlist_item_subitems_get(const Elm_Object_Item *it);
/**
 * Remove all sub-items (children) of the given item
 *
 * @param it The item
 *
 * This removes all items that are children (and their descendants) of the
 * given item @p it.
 *
 * @see elm_genlist_clear()
 * @see elm_object_item_del()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_subitems_clear(Elm_Object_Item *it);

/**
 * Sets the expanded state of an item.
 *
 * @param it The item
 * @param expanded The expanded state (@c EINA_TRUE expanded, @c EINA_FALSE not expanded).
 *
 * This function flags the item of type #ELM_GENLIST_ITEM_TREE as
 * expanded or not.
 *
 * The theme will respond to this change visually, and a signal "expanded" or
 * "contracted" will be sent from the genlist with a pointer to the item that
 * has been expanded/contracted.
 *
 * Calling this function won't show or hide any child of this item (if it is
 * a parent). You must manually delete and create them on the callbacks of
 * the "expanded" or "contracted" signals.
 *
 * @see elm_genlist_item_expanded_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_expanded_set(Elm_Object_Item *it, Eina_Bool expanded);

/**
 * Get the expanded state of an item
 *
 * @param it The item
 * @return The expanded state
 *
 * This gets the expanded state of an item.
 *
 * @see elm_genlist_item_expanded_set()
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool                     elm_genlist_item_expanded_get(const Elm_Object_Item *it);

/**
 * Get the depth of expanded item
 *
 * @param it The genlist item object
 * @return The depth of expanded item
 *
 * @ingroup Genlist
 */
EAPI int                           elm_genlist_item_expanded_depth_get(const Elm_Object_Item *it);

/**
 * Unset all contents fetched by the item class
 *
 * @param it The item
 * @param l The contents list to return
 *
 * This instructs genlist to release references to contents in the item,
 * meaning that they will no longer be managed by genlist and are
 * floating "orphans" that can be re-used elsewhere if the user wants
 * to.
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_all_contents_unset(Elm_Object_Item *it, Eina_List **l);

/**
 * Promote an item to the top of the list
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_promote(Elm_Object_Item *it);

/**
 * Demote an item to the end of the list
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_demote(Elm_Object_Item *it);

/**
 * Update the part of an item
 *
 * @param it The item
 * @param parts The name of item's part
 * @param itf The type of item's part type
 *
 * This updates an item's part by calling item's fetching functions again
 * to get the contents, texts and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 * Second parts argument is used for globbing to match '*', '?', and '.'
 * It can be used at updating multi fields.
 *
 * Use elm_genlist_realized_items_update() to update an item's all
 * property.
 *
 * @see elm_genlist_item_update()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_fields_update(Elm_Object_Item *it, const char *parts, Elm_Genlist_Item_Field_Type itf);

/**
 * Activate a genlist mode on an item
 *
 * @param it The genlist item
 * @param decorate_it_type Mode name
 * @param decorate_it_set Boolean to define set or unset mode.

 *
 * A genlist mode is a different way of selecting an item. Once a mode is
 * activated on an item, any other selected item is immediately unselected.
 * This feature provides an easy way of implementing a new kind of animation
 * for selecting an item, without having to entirely rewrite the item style
 * theme. However, the elm_genlist_selected_* API can't be used to get what
 * item is activate for a mode.
 *
 * The current item style will still be used, but applying a genlist mode to
 * an item will select it using a different kind of animation.
 *
 * The current active item for a mode can be found by
 * elm_genlist_decorated_item_get().
 *
 * The characteristics of genlist mode are:
 * - Only one mode can be active at any time, and for only one item.
 * - Genlist handles deactivating other items when one item is activated.
 * - A mode is defined in the genlist theme (edc), and more modes can easily
 *   be added.
 * - A mode style and the genlist item style are different things. They
 *   can be combined to provide a default style to the item, with some kind
 *   of animation for that item when the mode is activated.
 *
 * When a mode is activated on an item, a new view for that item is created.
 * The theme of this mode defines the animation that will be used to transit
 * the item from the old view to the new view. This second (new) view will be
 * active for that item while the mode is active on the item, and will be
 * destroyed after the mode is totally deactivated from that item.
 *
 * @see elm_genlist_mode_get()
 * @see elm_genlist_decorated_item_get()
 *
 * @ingroup Genlist
 */
EAPI void                          elm_genlist_item_decorate_mode_set(Elm_Object_Item *it, const char *decorate_it_type, Eina_Bool decorate_it_set);

/**
 * Get the item's decorate mode.
 *
 * @param it The genlist item
 *
 * This function just returns the name of the item's decorate mode.
 *
 * @see elm_genlist_item_decorate_mode_set()
 * @see elm_genlist_decorated_item_get()
 *
 * @ingroup Genlist
 */
EAPI const char                   *elm_genlist_item_decorate_mode_get(const Elm_Object_Item *it);

/**
 * Get the Item's Type
 *
 * @param it The genlist item
 * @return The item type.
 *
 * This function returns the item's type. Normally the item's type.
 * If it failed, return value is ELM_GENLIST_ITEM_MAX
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item_Type        elm_genlist_item_type_get(const Elm_Object_Item *it);

/**
 * Set the flip state of a given genlist item.
 *
 * @param it The genlist item object
 * @param flip The flip mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * This function sets the flip state of a given genlist item.
 * Flip mode overrides current item object.
 * It can be used for on-the-fly item replace.
 * Flip mode can be used with/without decorate mode.
 *
 * @see elm_genlist_item_flip_get()
 *
 * @ingroup Genlist
 */
EAPI void elm_genlist_item_flip_set(Elm_Object_Item *it, Eina_Bool flip);

/**
 * Get the flip state of a given genlist item.
 *
 * @param it The genlist item object
 *
 * This function returns the flip state of a given genlist item.
 * If the parameter is invalid, it returns EINA_FALSE.
 *
 * @see elm_genlist_item_flip_set()
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool elm_genlist_item_flip_get(const Elm_Object_Item *it);

/**
 * Set the genlist item's select mode.
 *
 * @param it The genlist item object
 * @param mode The select mode
 *
 * elm_genlist_select_mode_set() changes item's select mode.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : The item will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS : This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select the item
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 * - ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY : This will apply no-finger-size rule
 *      with ELM_OBJECT_SELECT_MODE_NONE. No-finger-size rule makes an item can be
 *      smaller than lower limit. Clickable objects should be bigger than
 *      human touch point device (your finger) for some touch or
 *      small screen devices. So it is enabled, the item can be shrink than
 *      predefined finger-size value. And the item will be updated.
 *
 * @see elm_genlist_item_select_mode_get()
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_select_mode_set(Elm_Object_Item *it,
                                 Elm_Object_Select_Mode mode);

/**
 * Get the genlist item's select mode.
 *
 * @param it The genlist item object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_genlist_item_select_mode_set()
 *
 * @ingroup Genlist
 */
EAPI Elm_Object_Select_Mode
elm_genlist_item_select_mode_get(const Elm_Object_Item *it);


