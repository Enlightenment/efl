#define ELM_GENLIST_ITEM_CLASS_VERSION ELM_GEN_ITEM_CLASS_VERSION
#define ELM_GENLIST_ITEM_CLASS_HEADER ELM_GEN_ITEM_CLASS_HEADER
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
 * @see Elm_Gen_Item_Filter_Get_Cb
 */
typedef Elm_Gen_Item_Filter_Get_Cb Elm_Genlist_Item_Filter_Get_Cb;

/**
 * @see Elm_Gen_Item_Reusable_Content_Get_Cb
 */
typedef Elm_Gen_Item_Reusable_Content_Get_Cb Elm_Genlist_Reusable_Content_Get_Cb;

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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 *        given @p data, being @p event_info the item handle.
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
 */
EAPI const char                   *elm_genlist_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If @c EINA_TRUE, size restrictions are disabled
 * @return @c EINA_FALSE on failure, @c EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_genlist_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Get size restriction state of an object's tooltip
 * @param it The tooltip's anchor object
 * @return If @c EINA_TRUE, size restrictions are disabled
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
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
 * @ingroup Elm_Genlist_Group
 */
EAPI Eina_Bool                     elm_genlist_item_cursor_engine_only_get(const Elm_Object_Item *it);

