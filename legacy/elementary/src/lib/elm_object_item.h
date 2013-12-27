/**
 * @typedef Elm_Object_Item
 * An Elementary Object item handle.
 * @ingroup General
 */
typedef struct _Elm_Object_Item Elm_Object_Item;

/**
 * @typedef Elm_Object_Item_Signal_Cb
 *
 * Elm_Object_Item Signal Callback functions' prototype definition. @c data
 * will have the auxiliary data pointer at the time the callback registration.
 * @c it will be a pointer the Elm_Object_Item that have the edje object where
 * the signal comes from. @c emission will identify the exact signal's emission
 * string and @c source the exact signal's source one.
 *
 * @see elm_object_item_signal_callback_add()
 * @since 1.8
 *
 * @ingroup General
 */
typedef void                  (*Elm_Object_Item_Signal_Cb)(void *data, Elm_Object_Item *it, const char *emission, const char *source);

/**
 * Get the widget object's handle which contains a given item
 *
 * @param it The Elementary object item
 * @return The widget object
 *
 * @note This returns the widget object itself that an item belongs to.
 * @note Every elm_object_item supports this API
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_item_widget_get(const Elm_Object_Item *it);

/**
 * Set a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to set (NULL for the default content)
 * @param content The new content of the object item
 *
 * This sets a new object to an item as a content object. If any object was
 * already set as a content object in the same part, previous object will be
 * deleted automatically.
 *
 * @note Elementary object items may have many contents
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_part_content_set(Elm_Object_Item *it, const char *part, Evas_Object *content);

#define elm_object_item_content_set(it, content) elm_object_item_part_content_set((it), NULL, (content))

/**
 * Get a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to unset (NULL for the default content)
 * @return content of the object item or NULL for any error
 *
 * @note Elementary object items may have many contents
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_item_part_content_get(const Elm_Object_Item *it, const char *part);

#define elm_object_item_content_get(it) elm_object_item_part_content_get((it), NULL)

/**
 * Unset a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to unset (NULL for the default content)
 *
 * @note Elementary object items may have many contents
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_item_part_content_unset(Elm_Object_Item *it, const char *part);

#define elm_object_item_content_unset(it) elm_object_item_part_content_unset((it), NULL)

/**
 * Set a label of an object item
 *
 * @param it The Elementary object item
 * @param part The text part name to set (NULL for the default label)
 * @param label The new text of the label
 *
 * @note Elementary object items may have many labels
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_part_text_set(Elm_Object_Item *it, const char *part, const char *label);

/**
 * Macro to set a label of an object item.
 *
 * @param it The Elementary object item.
 * @param label The new text of the label.
 *
 * @note Elementary object items may have many labels.
 *
 * @ingroup General
 */
#define elm_object_item_text_set(it, label) elm_object_item_part_text_set((it), NULL, (label))

/**
 * Get a label of an object item
 *
 * @param it The Elementary object item
 * @param part The text part name to get (NULL for the default label)
 * @return text of the label or NULL for any error
 *
 * @note Elementary object items may have many labels
 *
 * @ingroup General
 */
EAPI const char                  *elm_object_item_part_text_get(const Elm_Object_Item *it, const char *part);

#define elm_object_item_text_get(it) elm_object_item_part_text_get((it), NULL)

/**
 * Set the text for an object item's part, marking it as translatable.
 *
 * The string to set as @p text must be the original one. Do not pass the
 * return of @c gettext() here. Elementary will translate the string
 * internally and set it on the object item using
 * elm_object_item_part_text_set(), also storing the original string so that it
 * can be automatically translated when the language is changed with
 * elm_language_set(). The @p domain will be stored along to find the
 * translation in the correct catalog. It can be NULL, in which case it will use
 * whatever domain was set by the application with @c textdomain(). This is
 * useful in case you are building a library on top of Elementary that will have
 * its own translatable strings, that should not be mixed with those of programs
 * using the library.
 *
 * @param it The object item containing the text part
 * @param part The name of the part to set
 * @param domain The translation domain to use
 * @param text The original, non-translated text to set
 *
 * @ingroup General
 * @since 1.8
 */
EAPI void      elm_object_item_domain_translatable_part_text_set(Elm_Object_Item *it, const char *part, const char *domain, const char *text);

#define elm_object_item_domain_translatable_text_set(it, domain, text) elm_object_item_domain_translatable_part_text_set((it), NULL, (domain), (text))

#define elm_object_item_translatable_text_set(it, text) elm_object_item_domain_translatable_part_text_set((it), NULL, NULL, (text))

#define elm_object_item_translatable_part_text_set(it, part, text) elm_object_item_domain_translatable_part_text_set((it), (part), NULL, (text))

/**
 * Get the original string set as translatable for an object item.
 *
 * When setting translated strings, the function elm_object_item_part_text_get()
 * will return the translation returned by @c gettext(). To get the original
 * string use this function.
 *
 * @param it The object item.
 * @param part The name of the part that was set
 *
 * @return The original, untranslated string
 *
 * @ingroup General
 * @since 1.8
 */
EAPI const char *elm_object_item_translatable_part_text_get(const Elm_Object_Item *it, const char *part);

#define elm_object_item_translatable_text_get(it) elm_object_item_translatable_part_text_get((it), NULL)

/**
 * Mark the part text to be translatable or not.
 *
 * Once you mark the part text to be translatable, the text will be translated
 * internally regardless of elm_object_item_part_text_set() and
 * elm_object_item_domain_translatable_part_text_set(). In other case, if you
 * set the Elementary policy that all text will be translatable in default, you
 * can set the part text to not be translated by calling this API.
 *
 * @param it The object item containing the text part
 * @param part The part name of the translatable text
 * @param domain The translation domain to use
 * @param translatable @c EINA_TRUE, the part text will be translated
 *        internally. @c EINA_FALSE, otherwise.
 *
 * @see elm_object_item_domain_translatable_part_text_set()
 * @see elm_object_item_part_text_set()
 * @see elm_policy()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void elm_object_item_domain_part_text_translatable_set(Elm_Object_Item *it, const char *part, const char *domain, Eina_Bool translatable);

#define elm_object_item_part_text_translatable_set(it, part, translatable) elm_object_item_domain_part_text_translatable_set((it), (part), NULL, (translatable))

#define elm_object_item_domain_text_translatable_set(it, domain, translatable) elm_object_item_domain_part_text_translatable_set((it), NULL, (domain), (translatable))

/**
 * Set the text to read out when in accessibility mode
 *
 * @param it The object item which is to be described
 * @param txt The text that describes the widget to people with poor or no vision
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_access_info_set(Elm_Object_Item *it, const char *txt);

/**
 * @brief Register object item as an accessible object.
 * @since 1.8
 *
 * @param item The elementary object item
 * @return Accessible object of the object item or NULL for any error
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_item_access_register(Elm_Object_Item *item);

/**
 * @brief Unregister accessible object of the object item.
 * @since 1.8
 *
 * @param item The elementary object item
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_access_unregister(Elm_Object_Item *item);

/**
 * @brief Get an accessible object of the object item.
 * @since 1.8
 *
 * @param item The elementary object item
 * @return Accessible object of the object item or NULL for any error
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_item_access_object_get(const Elm_Object_Item *item);

/**
 * @brief Set highlight order
 * @since 1.8
 *
 * @param item The container object item
 * @param objs Order of objects to pass highlight
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_access_order_set(Elm_Object_Item *item, Eina_List *objs);

/**
 * @brief Get highlight order
 * @since 1.8
 *
 * @param item The container object item
 * @return Order of objects to pass highlight
 *
 * @ingroup General
 */
EAPI const Eina_List              *elm_object_item_access_order_get(const Elm_Object_Item *item);

/**
 * @brief Unset highlight order
 * @since 1.8
 *
 * @param item The container object item
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_access_order_unset(Elm_Object_Item *item);

/**
 * Get the data associated with an object item
 * @param it The Elementary object item
 * @return The data associated with @p it
 *
 * @note Every elm_object_item supports this API
 * @ingroup General
 */
EAPI void                        *elm_object_item_data_get(const Elm_Object_Item *it);

/**
 * Set the data associated with an object item
 * @param it The Elementary object item
 * @param data The data to be associated with @p it
 *
 * @note Every elm_object_item supports this API
 * @ingroup General
 */
EAPI void                         elm_object_item_data_set(Elm_Object_Item *it, void *data);

/**
 * Send a signal to the edje object of the widget item.
 *
 * This function sends a signal to the edje object of the obj item. An
 * edje program can respond to a signal by specifying matching
 * 'signal' and 'source' fields.
 *
 * @param it The Elementary object item
 * @param emission The signal's name.
 * @param source The signal's source.
 * @ingroup General
 */
EAPI void                         elm_object_item_signal_emit(Elm_Object_Item *it, const char *emission, const char *source);

/**
 * Add a callback for a signal emitted by object item edje.
 *
 * This function connects a callback function to a signal emitted by the
 * edje object of the object item.
 * Globs can occur in either the emission or source name.
 *
 * @param it The elementary object item
 * @param emission The signal's name.
 * @param source The signal's source.
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass to the callback function.
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_signal_callback_add(Elm_Object_Item *it, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func, void *data);

/**
 * Remove a signal-triggered callback from a object item edje object.
 *
 * @param it The elementary object item
 * @param emission The signal's name.
 * @param source The signal's source.
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @return The data pointer of the signal callback or @c NULL, on
 * errors.
 *
 * This function removes the @b last callback, previously attached to
 * a signal emitted by an underlying Edje object of @a it, whose
 * parameters @a emission, @a source and @c func match exactly with
 * those passed to a previous call to
 * elm_object_item_signal_callback_add(). The data pointer that was passed
 * to this call will be returned.
 *
 * @see elm_object_item_signal_callback_add()
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void                        *elm_object_item_signal_callback_del(Elm_Object_Item *it, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func);

/**
 * Set the disabled state of an widget item.
 *
 * @param it The Elementary object item
 * @param disabled The state to put in in: @c EINA_TRUE for
 *        disabled, @c EINA_FALSE for enabled
 *
 * Elementary object item can be @b disabled, in which state they won't
 * receive input and, in general, will be themed differently from
 * their normal state, usually greyed out. Useful for contexts
 * where you don't want your users to interact with some of the
 * parts of you interface.
 *
 * This sets the state for the widget item, either disabling it or
 * enabling it back.
 *
 * @ingroup Styles
 */
EAPI void                         elm_object_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Get the disabled state of an widget item.
 *
 * @param it The Elementary object item
 * @return @c EINA_TRUE, if the widget item is disabled, @c EINA_FALSE
 *            if it's enabled (or on errors)
 *
 * This gets the state of the widget, which might be enabled or disabled.
 *
 * @ingroup Styles
 */
EAPI Eina_Bool                    elm_object_item_disabled_get(const Elm_Object_Item *it);

/**
 * @brief Set the function to be called when an item from the widget is
 * freed.
 *
 * @param it The item to set the callback on
 * @param del_cb The function called
 *
 * That function will receive these parameters:
 * @li void * item data
 * @li Evas_Object * widget object
 * @li Elm_Object_Item * widget item
 *
 * @note Every elm_object_item supports this API
 *
 * @see elm_object_item_del()
 * @ingroup General
 */
EAPI void                         elm_object_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb del_cb);

/**
 * Delete the given item.
 *
 * @param it The item to be deleted.
 *
 * @ingroup General
 */
EAPI void                       elm_object_item_del(Elm_Object_Item *it);

/**
 * Set the text to be shown in a given object item's tooltips.
 *
 * @param it Target item.
 * @param text The text to set in the content.
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data - set with this function or
 * elm_object_item_tooltip_content_cb_set() - is removed.
 *
 * @see elm_object_tooltip_text_set() for more details.
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                    elm_object_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param it The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parent window's canvas.
 * It will instead be limited only by the size of the display.
 */
EAPI Eina_Bool                    elm_object_item_tooltip_window_mode_get(const Elm_Object_Item *it);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the object item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @a func, the tooltip is unset with
 *        elm_object_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it object item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_object_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_object_item_tooltip_content_cb_set()
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_object_item_tooltip_content_cb_set() or
 *       elm_object_item_tooltip_text_set()
 *
 * @param it object item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @see elm_object_tooltip_style_set() for more details.
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param it object item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_object_item_tooltip_style_set()
 *
 * @ingroup General
 */
EAPI const char                  *elm_object_item_tooltip_style_get(const Elm_Object_Item *it);

/**
  * Set the type of mouse pointer/cursor decoration to be shown,
  * when the mouse pointer is over the given item
  *
  * @param it item to customize cursor on
  * @param cursor the cursor type's name
  *
  * This function works analogously as elm_object_cursor_set(), but
  * here the cursor's changing area is restricted to the item's
  * area, and not the whole widget's. Note that that item cursors
  * have precedence over widget cursors, so that a mouse over an
  * item with custom cursor set will always show @b that cursor.
  *
  * If this function is called twice for an object, a previously set
  * cursor will be unset on the second call.
  *
  * @see elm_object_cursor_set()
  * @see elm_object_item_cursor_get()
  * @see elm_object_item_cursor_unset()
  *
  * @ingroup General
  */
EAPI void                         elm_object_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/*
 * Get the type of mouse pointer/cursor decoration set to be shown,
 * when the mouse pointer is over the given item
 *
 * @param it item with custom cursor set
 * @return the cursor type's name or @c NULL, if no custom cursors
 * were set to @p item (and on errors)
 *
 * @see elm_object_cursor_get()
 * @see elm_object_item_cursor_set()
 * @see elm_object_item_cursor_unset()
 *
 * @ingroup General
 */
EAPI const char                  *elm_object_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset any custom mouse pointer/cursor decoration set to be
 * shown, when the mouse pointer is over the given
 * item, thus making it show the @b default cursor again.
 *
 * @param it the item
 *
 * Use this call to undo any custom settings on this item's cursor
 * decoration, bringing it back to defaults (no custom style set).
 *
 * @see elm_object_cursor_unset()
 * @see elm_object_item_cursor_set()
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_cursor_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given custom cursor set for an
 * item.
 *
 * @param it item with custom cursor set
 * @param style the <b>theme style</b> to use (e.g. @c "default",
 * @c "transparent", etc)
 *
 * This function only makes sense when one is using custom mouse
 * cursor decorations <b>defined in a theme file</b>, which can have,
 * given a cursor name/type, <b>alternate styles</b> on it. It
 * works analogously as elm_object_cursor_style_set(), but here
 * applies only to item objects.
 *
 * @warning Before you set a cursor style you should have defined a
 *       custom cursor previously on the item, with
 *       elm_object_item_cursor_set()
 *
 * @see elm_object_item_cursor_engine_only_set()
 * @see elm_object_item_cursor_style_get()
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the current @b style set for a given item's custom
 * cursor
 *
 * @param it item with custom cursor set.
 * @return style the cursor style in use. If the object does not
 *         have a cursor set, then @c NULL is returned.
 *
 * @see elm_object_item_cursor_style_set() for more details
 *
 * @ingroup General
 */
EAPI const char                  *elm_object_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Set if the (custom)cursor for a given item should be
 * searched in its theme, also, or should only rely on the
 * rendering engine.
 *
 * @param it item with custom (custom) cursor already set on
 * @param engine_only Use @c EINA_TRUE to have cursors looked for
 * only on those provided by the rendering engine, @c EINA_FALSE to
 * have them searched on the widget's theme, as well.
 *
 * @note This call is of use only if you've set a custom cursor
 * for items, with elm_object_item_cursor_set().
 *
 * @note By default, cursors will only be looked for between those
 * provided by the rendering engine.
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only);

/**
 * Get if the (custom) cursor for a given item is being
 * searched in its theme, also, or is only relying on the rendering
 * engine.
 *
 * @param it an object item
 * @return @c EINA_TRUE, if cursors are being looked for only on
 * those provided by the rendering engine, @c EINA_FALSE if they
 * are being searched on the widget's theme, as well.
 *
 * @see elm_object_item_cursor_engine_only_set(), for more details
 *
 * @ingroup General
 */
EAPI Eina_Bool                    elm_object_item_cursor_engine_only_get(const Elm_Object_Item *it);

/**
 * This returns track object of the item.
 *
 * @param it The Elementary Object Item to be tracked.
 * @return The track object.
 *
 * @note This gets a rectangle object that represents the object item's internal
 *       object. If you wanna check the geometry, visibility of the item, you
 *       can call the evas apis such as evas_object_geometry_get(),
 *       evas_object_visible_get() to the track object. Note that all of the
 *       widget items may/may not have the internal object so this api may
 *       return @c NULL if the widget item doesn't have it. Additionally, the
 *       widget item is managed/controlled by the widget, the widget item could
 *       be changed(moved, resized even deleted) anytime by it's own widget's
 *       decision. So please dont' change the track object as well as don't
 *       keep the track object in your side as possible but get the track object
 *       at the moment you need to refer. Otherwise, you need to add some
 *       callbacks to the track object to track it's attributes changes.
 *
 * @warning After use the track object, please call the
 *          elm_object_item_untrack() paired to elm_object_item_track()
 *          definitely to free the track object properly. Don't delete the
 *          track object.
 *
 * @see elm_object_item_untrack()
 * @see elm_object_item_track_get()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI Evas_Object                 *elm_object_item_track(Elm_Object_Item *it);

/**
 * This retrieve the track object of the item.
 *
 * @param it The Elementary Object Item that returned track object.
 *
 * @note This retrieves the track object that was returned from
 *       elm_object_item_track().
 *
 * @see elm_object_item_track()
 * @see elm_object_item_track_get()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_untrack(Elm_Object_Item *it);

/**
 * Get the track object reference count.
 *
 * @param it The Elementary Object Item that returned track object.
 *
 * @note This gets the reference count for the track object. Whenever you call
 *       the elm_object_item_track(), the reference count will be increased by
 *       one. Likely the referece count will be decreased again when you call
 *       the elm_object_item_untrack(). Unless the reference count reaches to
 *       zero, the track object won't be deleted. So please be sure to call
 *       elm_object_item_untrack() paired to the elm_object_item_track() call
 *        count.
 *
 * @see elm_object_item_track()
 * @see elm_object_item_track_get()
 *
 * @since 1.8
 *
 * @ingroup General
 */
EAPI int                          elm_object_item_track_get(const Elm_Object_Item *it);

/**
 * Set a style of an object item
 *
 * @param it The Elementary object item
 * @param part The style
 *
 * @ingroup General
 */
EAPI void                         elm_object_item_style_set(Elm_Object_Item *it, const char *part);

/**
 * Get the style of an object item
 *
 * @param it The Elementary object item
 * @return The style of object item
 *
 * @ingroup General
 */
EAPI const char                  *elm_object_item_style_get(Elm_Object_Item *it);
