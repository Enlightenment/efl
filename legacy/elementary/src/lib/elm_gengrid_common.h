#define ELM_GENGRID_ITEM_CLASS_VERSION ELM_GEN_ITEM_CLASS_VERSION
#define ELM_GENGRID_ITEM_CLASS_HEADER ELM_GEN_ITEM_CLASS_HEADER

/**
 * Defines where to position the item in the genlist.
 *
 * @ingroup Genlist
 */
typedef enum
{
   ELM_GENGRID_ITEM_SCROLLTO_NONE = 0,   /**< no scrollto */
   ELM_GENGRID_ITEM_SCROLLTO_IN = (1 << 0),   /**< to the nearest viewport */
   ELM_GENGRID_ITEM_SCROLLTO_TOP = (1 << 1),   /**< to the top of viewport */
   ELM_GENGRID_ITEM_SCROLLTO_MIDDLE = (1 << 2)   /**< to the middle of viewport */
} Elm_Gengrid_Item_Scrollto_Type;


/**
 * @see Elm_Gen_Item_Class
 */
typedef Elm_Gen_Item_Class Elm_Gengrid_Item_Class;

/**
 * @see Elm_Gen_Item_Text_Get_Cb
 */
typedef Elm_Gen_Item_Text_Get_Cb Elm_Gengrid_Item_Text_Get_Cb;

/**
 * @see Elm_Gen_Item_Content_Get_Cb
 */
typedef Elm_Gen_Item_Content_Get_Cb Elm_Gengrid_Item_Content_Get_Cb;

/**
 * @see Elm_Gen_Item_State_Get_Cb
 */
typedef Elm_Gen_Item_State_Get_Cb Elm_Gengrid_Item_State_Get_Cb;

/**
 * @see Elm_Gen_Item_Del_Cb
 */
typedef Elm_Gen_Item_Del_Cb Elm_Gengrid_Item_Del_Cb;

/**
 * Get the @b next item in a gengrid widget's internal list of items,
 * given a handle to one of those items.
 *
 * @param it The gengrid item to fetch next from
 * @return The item after @p item, or @c NULL if there's none (and
 * on errors)
 *
 * This returns the item placed after the @p item, on the container
 * gengrid.
 *
 * @see elm_gengrid_item_prev_get()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_next_get(const Elm_Object_Item *it);

/**
 * Get the @b previous item in a gengrid widget's internal list of items,
 * given a handle to one of those items.
 *
 * @param it The gengrid item to fetch previous from
 * @return The item before @p item, or @c NULL if there's none (and
 * on errors)
 *
 * This returns the item placed before the @p item, on the container
 * gengrid.
 *
 * @see elm_gengrid_item_next_get()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_prev_get(const Elm_Object_Item *it);

/**
 * Set whether a given gengrid item is selected or not
 *
 * @param it The gengrid item
 * @param selected Use @c EINA_TRUE, to make it selected, @c
 * EINA_FALSE to make it unselected
 *
 * This sets the selected state of an item. If multi-selection is
 * not enabled on the containing gengrid and @p selected is @c
 * EINA_TRUE, any other previously selected items will get
 * unselected in favor of this new one.
 *
 * @see elm_gengrid_item_selected_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * Get whether a given gengrid item is selected or not
 *
 * @param it The gengrid item
 * @return @c EINA_TRUE, if it's selected, @c EINA_FALSE otherwise
 *
 * This API returns EINA_TRUE for all the items selected in multi-select mode as well.
 *
 * @see elm_gengrid_item_selected_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool                     elm_gengrid_item_selected_get(const Elm_Object_Item *it);

/**
 * Show the portion of a gengrid's internal grid containing a given
 * item, @b immediately.
 *
 * @param it The item to display
 * @param type Where to position the item in the viewport.
 *
 * This causes gengrid to @b redraw its viewport's contents to the
 * region containing the given @p item item, if it is not fully
 * visible.
 *
 * @see elm_gengrid_item_bring_in()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_show(Elm_Object_Item *it, Elm_Gengrid_Item_Scrollto_Type type);

/**
 * Animatedly bring in, to the visible area of a gengrid, a given
 * item on it.
 *
 * @param it The gengrid item to display
 * @param type Where to position the item in the viewport.
 *
 * This causes gengrid to jump to the given @p item and show
 * it (by scrolling), if it is not fully visible. This will use
 * animation to do so and take a period of time to complete.
 *
 * @see elm_gengrid_item_show()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_bring_in(Elm_Object_Item *it, Elm_Gengrid_Item_Scrollto_Type type);

/**
 * Update the contents of a given gengrid item
 *
 * @param it The gengrid item
 *
 * This updates an item by calling all the item class functions
 * again to get the contents, texts and states. Use this when the
 * original item data has changed and you want the changes to be
 * reflected.
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_update(Elm_Object_Item *it);

/**
 * Update the item class of a gengrid item.
 *
 * This sets another class of the item, changing the way that it is
 * displayed. After changing the item class, elm_gengrid_item_update() is
 * called on the item @p it.
 *
 * @param it The gengrid item
 * @param gic The gengrid item class describing the function pointers and the item style.
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_item_class_update(Elm_Object_Item *it, const Elm_Gengrid_Item_Class *gic);

/**
 * Get the Gengrid Item class for the given Gengrid Item.
 *
 * @param it The gengrid item
 *
 * This returns the Gengrid_Item_Class for the given item. It can be used to examine
 * the function pointers and item_style.
 *
 * @ingroup Gengrid
 */
EAPI const Elm_Gengrid_Item_Class *elm_gengrid_item_item_class_get(const Elm_Object_Item *it);

/**
 * Get the index of the item. It is only valid once displayed.
 *
 * @param it a gengrid item
 * @return the position inside the list of item.
 *
 * @ingroup Gengrid
 */
EAPI int                           elm_gengrid_item_index_get(const Elm_Object_Item *it);

/**
 * Add a new gengrid item class in a given gengrid widget.
 *
 * @return New allocated a gengrid item class.
 *
 * This adds gengrid item class for the gengrid widget. When adding an item,
 * gengrid_item_{append, prepend, insert} function needs item class of the item.
 * Given callback parameters are used at retrieving {text, content} of
 * added item. Set as NULL if it's not used.
 * If there's no available memory, return can be NULL.
 *
 * @see elm_gengrid_item_class_free()
 * @see elm_gengrid_item_append()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item_Class *elm_gengrid_item_class_new(void);

/**
 * Remove an item class in a given gengrid widget.
 *
 * @param itc The itc to be removed.
 *
 * This removes item class from the gengrid widget.
 * Whenever it has no more references to it, item class is going to be freed.
 * Otherwise it just decreases its reference count.
 *
 * @see elm_gengrid_item_class_new()
 * @see elm_gengrid_item_class_ref()
 * @see elm_gengrid_item_class_unref()
 *
 * @ingroup Gengrid
 */
EAPI void elm_gengrid_item_class_free(Elm_Gengrid_Item_Class *itc);

/**
 * Increments object reference count for the item class.
 *
 * @param itc The given item class object to reference
 *
 * This API just increases its reference count for item class management.
 *
 * @see elm_gengrid_item_class_unref()
 *
 * @ingroup Gengrid
 */
EAPI void elm_gengrid_item_class_ref(Elm_Gengrid_Item_Class *itc);

/**
 * Decrements object reference count for the item class.
 *
 * @param itc The given item class object to reference
 *
 * This API just decreases its reference count for item class management.
 * Reference count can't be less than 0.
 *
 * @see elm_gengrid_item_class_ref()
 * @see elm_gengrid_item_class_free()
 *
 * @ingroup Gengrid
 */
EAPI void elm_gengrid_item_class_unref(Elm_Gengrid_Item_Class *itc);

/**
 * Set the text to be shown in a given gengrid item's tooltips.
 *
 * @param it The gengrid item
 * @param text The text to set in the content
 *
 * This call will setup the text to be used as tooltip to that item
 * (analogous to elm_object_tooltip_text_set(), but being item
 * tooltips with higher precedence than object tooltips). It can
 * have only one tooltip at a time, so any previous tooltip data
 * will get removed.
 *
 * In order to set a content or something else as a tooltip, look at
 * elm_gengrid_item_tooltip_content_cb_set().
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in a given gengrid item's tooltip
 *
 * @param it The gengrid item.
 * @param func The function returning the tooltip contents.
 * @param data What to provide to @a func as callback data/context.
 * @param del_cb Called when data is not needed anymore, either when
 *        another callback replaces @p func, the tooltip is unset with
 *        elm_gengrid_item_tooltip_unset() or the owner @p item
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
 * elm_gengrid_item_tooltip_text_set().
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset a tooltip from a given gengrid item
 *
 * @param it gengrid item to remove a previously set tooltip from.
 *
 * This call removes any tooltip set on @p item. The callback
 * provided as @c del_cb to
 * elm_gengrid_item_tooltip_content_cb_set() will be called to
 * notify it is not used anymore (and have resources cleaned, if
 * need be).
 *
 * @see elm_gengrid_item_tooltip_content_cb_set()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given gengrid item's tooltip.
 *
 * @param it gengrid item with tooltip set
 * @param style the <b>theme style</b> to use on tooltips (e.g. @c
 * "default", @c "transparent", etc)
 *
 * Tooltips can have <b>alternate styles</b> to be displayed on,
 * which are defined by the theme set on Elementary. This function
 * works analogously as elm_object_tooltip_style_set(), but here
 * applied only to gengrid item objects. The default style for
 * tooltips is @c "default".
 *
 * @note before you set a style you should define a tooltip with
 *       elm_gengrid_item_tooltip_content_cb_set() or
 *       elm_gengrid_item_tooltip_text_set()
 *
 * @see elm_gengrid_item_tooltip_style_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style set a given gengrid item's tooltip.
 *
 * @param it gengrid item with tooltip already set on.
 * @return style the theme style in use, which defaults to
 *         "default". If the object does not have a tooltip set,
 *         then @c NULL is returned.
 *
 * @see elm_gengrid_item_tooltip_style_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI const char                   *elm_gengrid_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_gengrid_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param it The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                     elm_gengrid_item_tooltip_window_mode_get(const Elm_Object_Item *it);

/**
 * Set the type of mouse pointer/cursor decoration to be shown,
 * when the mouse pointer is over the given gengrid widget item
 *
 * @param it gengrid item to customize cursor on
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
 * @see elm_gengrid_item_cursor_get()
 * @see elm_gengrid_item_cursor_unset()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/**
 * Get the type of mouse pointer/cursor decoration set to be shown,
 * when the mouse pointer is over the given gengrid widget item
 *
 * @param it gengrid item with custom cursor set
 * @return the cursor type's name or @c NULL, if no custom cursors
 * were set to @p item (and on errors)
 *
 * @see elm_object_cursor_get()
 * @see elm_gengrid_item_cursor_set() for more details
 * @see elm_gengrid_item_cursor_unset()
 *
 * @ingroup Gengrid
 */
EAPI const char                   *elm_gengrid_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset any custom mouse pointer/cursor decoration set to be
 * shown, when the mouse pointer is over the given gengrid widget
 * item, thus making it show the @b default cursor again.
 *
 * @param it a gengrid item
 *
 * Use this call to undo any custom settings on this item's cursor
 * decoration, bringing it back to defaults (no custom style set).
 *
 * @see elm_object_cursor_unset()
 * @see elm_gengrid_item_cursor_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_cursor_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given custom cursor set for a
 * gengrid item.
 *
 * @param it gengrid item with custom cursor set
 * @param style the <b>theme style</b> to use (e.g. @c "default",
 * @c "transparent", etc)
 *
 * This function only makes sense when one is using custom mouse
 * cursor decorations <b>defined in a theme file</b> , which can
 * have, given a cursor name/type, <b>alternate styles</b> on
 * it. It works analogously as elm_object_cursor_style_set(), but
 * here applied only to gengrid item objects.
 *
 * @warning Before you set a cursor style you should have defined a
 *       custom cursor previously on the item, with
 *       elm_gengrid_item_cursor_set()
 *
 * @see elm_gengrid_item_cursor_engine_only_set()
 * @see elm_gengrid_item_cursor_style_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the current @b style set for a given gengrid item's custom
 * cursor
 *
 * @param it gengrid item with custom cursor set.
 * @return style the cursor style in use. If the object does not
 *         have a cursor set, then @c NULL is returned.
 *
 * @see elm_gengrid_item_cursor_style_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI const char                   *elm_gengrid_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Set if the (custom) cursor for a given gengrid item should be
 * searched in its theme, also, or should only rely on the
 * rendering engine.
 *
 * @param it item with custom (custom) cursor already set on
 * @param engine_only Use @c EINA_TRUE to have cursors looked for
 * only on those provided by the rendering engine, @c EINA_FALSE to
 * have them searched on the widget's theme, as well.
 *
 * @note This call is of use only if you've set a custom cursor
 * for gengrid items, with elm_gengrid_item_cursor_set().
 *
 * @note By default, cursors will only be looked for between those
 * provided by the rendering engine.
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only);

/**
 * Get if the (custom) cursor for a given gengrid item is being
 * searched in its theme, also, or is only relying on the rendering
 * engine.
 *
 * @param it a gengrid item
 * @return @c EINA_TRUE, if cursors are being looked for only on
 * those provided by the rendering engine, @c EINA_FALSE if they
 * are being searched on the widget's theme, as well.
 *
 * @see elm_gengrid_item_cursor_engine_only_set(), for more details
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool                     elm_gengrid_item_cursor_engine_only_get(const Elm_Object_Item *it);

/**
 * Get a given gengrid item's position, relative to the whole
 * gengrid's grid area.
 *
 * @param it The Gengrid item.
 * @param x Pointer to variable to store the item's <b>row number</b>.
 * @param y Pointer to variable to store the item's <b>column number</b>.
 *
 * This returns the "logical" position of the item within the
 * gengrid. For example, @c (0, 1) would stand for first row,
 * second column.
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_pos_get(const Elm_Object_Item *it, unsigned int *x, unsigned int *y);

/**
 * Set the gengrid item's select mode.
 *
 * @param it The gengrid item object
 * @param mode The select mode
 *
 * elm_gengrid_select_mode_set() changes item's select mode.
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
 * @see elm_gengrid_item_select_mode_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_select_mode_set(Elm_Object_Item *it, Elm_Object_Select_Mode mode);

/**
 * Get the gengrid item's select mode.
 *
 * @param it The gengrid item object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_gengrid_item_select_mode_set()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Select_Mode        elm_gengrid_item_select_mode_get(const Elm_Object_Item *it);

