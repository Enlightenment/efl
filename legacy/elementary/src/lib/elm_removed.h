/*
 * DO NOT INCLUDE THIS HEADER INSIDE ANY SOURCE.
 * THIS IS ONLY FOR ELEMENTARY API DEPRECATION HISTORY AND WILL BE REMOVED LATER.
 * THIS WILL BE MAINLY MANAGED BY JIYOUN PARK (jypark).
 *
 * (deprecation process)
 *   1. mark EINA_DEPRECATED and move declarations to elm_deprecated.h.
 *   2. remove it from elm_deprecated.h and apply it to all trunk.
 *   3. move it to elm_removed.h.
 */

#error "do not include this"

/**
 * @brief Get Elementary's rendering engine in use.
 *
 * @return The rendering engine's name
 * @note there's no need to free the returned string, here.
 *
 * This gets the global rendering engine that is applied to all Elementary
 * applications.
 *
 * @see elm_engine_set()
 * @deprecated Use elm_engine_get() instead.
 */
EINA_DEPRECATED EAPI const char *elm_engine_current_get(void);

/**
 * Set the configured finger size for all applications on the display
 *
 * This sets the globally configured finger size in pixels for all
 * applications on the display
 *
 * @param size The finger size
 * @ingroup Fingers
 * @deprecated Use elm_finger_size_set() and elm_config_all_flush()
 */
EINA_DEPRECATED EAPI void       elm_finger_size_all_set(Evas_Coord size);

/**
 * Set the global scaling factor for all applications on the display
 *
 * This sets the globally configured scaling factor that is applied to all
 * objects for all applications.
 * @param scale The scaling factor to set
 * @ingroup Scaling
 * @deprecated use elm_scale_set() and elm_config_all_flush()
 */
EINA_DEPRECATED EAPI void   elm_scale_all_set(double scale);

/**
 * Apply the changes made with elm_font_overlay_set() and
 * elm_font_overlay_unset() on all Elementary application windows.
 *
 * @ingroup Fonts
 *
 * This applies all font overlays set to all objects in the UI.
 * @deprecated Use elm_font_overlay_apply and elm_config_all_flush()
 */
EINA_DEPRECATED EAPI void             elm_font_overlay_all_apply(void);

/**
 * Set the configured cache flush interval time for all applications on the
 * display
 *
 * This sets the globally configured cache flush interval time -- in ticks
 * -- for all applications on the display.
 *
 * @param size The cache flush interval time
 * @deprecated Use elm_cache_flush_interval_set() and elm_config_all_flush()
 * @ingroup Caches
 */
EINA_DEPRECATED EAPI void      elm_cache_flush_interval_all_set(int size);

/**
 * Set the configured cache flush enabled state for all applications on the
 * display
 *
 * This sets the globally configured cache flush enabled state for all
 * applications on the display.
 *
 * @param enabled The cache flush enabled state
 * @deprecated Use elm_cache_flush_enabled_set adnd elm_config_all_flush()
 * @ingroup Caches
 */
EINA_DEPRECATED EAPI void      elm_cache_flush_enabled_all_set(Eina_Bool enabled);

/**
 * Set the configured font cache size for all applications on the
 * display
 *
 * This sets the globally configured font cache size -- in bytes
 * -- for all applications on the display.
 *
 * @param size The font cache size
 * @deprecated Use elm_font_cache_set() and elm_config_all_flush()
 * @ingroup Caches
 */
EINA_DEPRECATED EAPI void      elm_font_cache_all_set(int size);

/**
 * Set the configured image cache size for all applications on the
 * display
 *
 * This sets the globally configured image cache size -- in bytes
 * -- for all applications on the display.
 *
 * @param size The image cache size
 * @deprecated Use elm_image_cache_set() and elm_config_all_flush()
 * @ingroup Caches
 */
EINA_DEPRECATED EAPI void      elm_image_cache_all_set(int size);

/**
 * Set the configured edje file cache size for all applications on the
 * display
 *
 * This sets the globally configured edje file cache size -- in number
 * of files -- for all applications on the display.
 *
 * @param size The edje file cache size
 * @deprecated Use elm_edje_file_cache_set() and elm_config_all_flush()
 * @ingroup Caches
 */
EINA_DEPRECATED EAPI void      elm_edje_file_cache_all_set(int size);

/**
 * Set the configured edje collections (groups) cache size for all
 * applications on the display
 *
 * This sets the globally configured edje collections cache size -- in
 * number of collections -- for all applications on the display.
 *
 * @param size The edje collections cache size
 * @deprecated Use elm_edje_collection_cache_set() and elm_config_all_flush()
 * @ingroup Caches
 */
EINA_DEPRECATED EAPI void      elm_edje_collection_cache_all_set(int size);

/**
 * Set Elementary's profile.
 *
 * This sets the global profile that is applied to all Elementary
 * applications. All running Elementary windows will be affected.
 *
 * @param profile The profile's name
 * @deprecated Use elm_profile_set() and elm_config_all_flush()
 * @ingroup Profile
 *
 */
EINA_DEPRECATED EAPI void        elm_profile_all_set(const char *profile);

/**
 * Set whether scrollers should bounce when they reach their
 * viewport's edge during a scroll, for all Elementary application
 * windows.
 *
 * @param enabled the thumb scroll bouncing state
 *
 * @see elm_thumbscroll_bounce_enabled_get()
 * @deprecated Use elm_scroll_bounce_enabled_set() and elm_config_all_flush()
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_bounce_enabled_all_set(Eina_Bool enabled);

/**
 * Set the amount of inertia a scroller will impose at bounce
 * animations, for all Elementary application windows.
 *
 * @param friction the thumb scroll bounce friction
 *
 * @see elm_thumbscroll_bounce_friction_get()
 * @deprecated Use elm_scroll_bounce_friction_set() and elm_config_all_flush()
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_bounce_friction_all_set(double friction);

/**
 * Set the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations, for all Elementary application windows.
 *
 * @param friction the page scroll friction
 *
 * @see elm_thumbscroll_page_scroll_friction_get()
 * @deprecated Use elm_scroll_page_scroll_friction_set() and 
 * elm_config_all_flush()
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_page_scroll_friction_all_set(double friction);

/**
 * Set the amount of inertia a scroller will impose at region bring
 * animations, for all Elementary application windows.
 *
 * @param friction the bring in scroll friction
 *
 * @see elm_thumbscroll_bring_in_scroll_friction_get()
 * @deprecated Use elm_scroll_bring_in_scroll_friction_set() and
 * elm_config_all_flush()
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_bring_in_scroll_friction_all_set(double friction);

/**
 * Set the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API, for all Elementary
 * application windows.
 *
 * @param friction the zoom friction
 *
 * @see elm_thumbscroll_zoom_friction_get()
 * @deprecated Use elm_scroll_zoom_friction_set() and elm_config_all_flush()
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_zoom_friction_all_set(double friction);

/**
 * Set whether scrollers should be draggable from any point in their
 * views, for all Elementary application windows.
 *
 * @param enabled the thumb scroll state
 *
 * @see elm_thumbscroll_enabled_get()
 * @deprecated Use elm_scroll_thumbscroll_enabled_set()
 * and elm_config_all_flush()
 *
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_thumbscroll_enabled_all_set(Eina_Bool enabled);

/**

 * Set the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling, for all Elementary
 * application windows.
 *
 * @param threshold the thumb scroll threshold
 *
 * @see elm_thumbscroll_threshold_get()
 * @deprecated Use elm_scroll_thumbscroll_threshold_set()
 * and elm_config_all_flush()
 *
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_thumbscroll_threshold_all_set(unsigned int threshold);

/**
 * Set the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second), for all Elementary application windows.
 *
 * @param threshold the thumb scroll momentum threshold
 *
 * @see elm_thumbscroll_momentum_threshold_get()
 * @deprecated Use elm_scroll_thumbscroll_momentum_threshold_set()
 * and elm_config_all_flush()
 *
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_thumbscroll_momentum_threshold_all_set(double threshold);

/**
 * Set the amount of inertia a scroller will impose at self scrolling
 * animations, for all Elementary application windows.
 *
 * @param friction the thumb scroll friction
 *
 * @see elm_thumbscroll_friction_get()
 * @deprecated Use elm_scroll_thumbscroll_friction_set()
 * and elm_config_all_flush()
 *
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_thumbscroll_friction_all_set(double friction);

/**
 * Set the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually, for all Elementary application windows.
 *
 * @param friction the thumb scroll border friction. @c 0.0 for
 *        perfect synchrony between two movements, @c 1.0 for maximum
 *        lag.
 *
 * @see elm_thumbscroll_border_friction_get()
 * @note parameter value will get bound to 0.0 - 1.0 interval, always
 * @deprecated Use elm_scroll_thumbscroll_border_friction_set()
 * and elm_config_all_flush()
 *
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_thumbscroll_border_friction_all_set(double friction);

/**
 * Set the sensitivity amount which is be multiplied by the length of
 * mouse dragging, for all Elementary application windows.
 *
 * @param friction the thumb scroll sensitivity friction. @c 0.1 for
 *        minimum sensitivity, @c 1.0 for maximum sensitivity. 0.25
 *        is proper.
 *
 * @see elm_thumbscroll_sensitivity_friction_get()
 * @note parameter value will get bound to 0.1 - 1.0 interval, always
 * @deprecated Use elm_scroll_thumbscroll_sensitivity_friction_set()
 * and elm_config_all_flush()
 *
 * @ingroup Scrolling
 */
EINA_DEPRECATED EAPI void         elm_scroll_thumbscroll_sensitivity_friction_all_set(double friction);

EINA_DEPRECATED EAPI void          elm_gen_clear(Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_gen_item_selected_set(Elm_Gen_Item *it, Eina_Bool selected);
EINA_DEPRECATED EAPI Eina_Bool     elm_gen_item_selected_get(const Elm_Gen_Item *it);
EINA_DEPRECATED EAPI void          elm_gen_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);
EINA_DEPRECATED EAPI Eina_Bool     elm_gen_always_select_mode_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_gen_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);
EINA_DEPRECATED EAPI Eina_Bool     elm_gen_no_select_mode_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_gen_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
EINA_DEPRECATED EAPI void          elm_gen_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
EINA_DEPRECATED EAPI void          elm_gen_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel);
EINA_DEPRECATED EAPI void          elm_gen_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel);

EINA_DEPRECATED EAPI void          elm_gen_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize);
EINA_DEPRECATED EAPI void          elm_gen_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);
EINA_DEPRECATED EAPI void          elm_gen_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);
EINA_DEPRECATED EAPI void          elm_gen_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);
EINA_DEPRECATED EAPI void          elm_gen_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_first_item_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_last_item_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_item_next_get(const Elm_Gen_Item *it);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_item_prev_get(const Elm_Gen_Item *it);
EINA_DEPRECATED EAPI Evas_Object  *elm_gen_item_widget_get(const Elm_Gen_Item *it);

/**
 * Get the widget object's handle which contains a given item
 *
 * @param it The Elementary object item
 * @return The widget object
 *
 * @note This returns the widget object itself that an item belongs to.
 * @note Every elm_object_item supports this API
 * @deprecated Use elm_object_item_widget_get() instead
 * @ingroup General
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_object_item_object_get(const Elm_Object_Item *it);

/**
 * Set the text to show in the anchorblock
 *
 * Sets the text of the anchorblock to @p text. This text can include markup
 * format tags, including <c>\<a href=anchorname\></a></c> to begin a segment
 * of text that will be specially styled and react to click events, ended
 * with either of \</a\> or \</\>. When clicked, the anchor will emit an
 * "anchor,clicked" signal that you can attach a callback to with
 * evas_object_smart_callback_add(). The name of the anchor given in the
 * event info struct will be the one set in the href attribute, in this
 * case, anchorname.
 *
 * Other markup can be used to style the text in different ways, but it's
 * up to the style defined in the theme which tags do what.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_anchorblock_text_set(Evas_Object *obj, const char *text);

/**
 * Get the markup text set for the anchorblock
 *
 * Retrieves the text set on the anchorblock, with markup tags included.
 *
 * @param obj The anchorblock object
 * @return The markup text set or @c NULL if nothing was set or an error
 * occurred
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_anchorblock_text_get(const Evas_Object *obj);

/**
 * Set the text to show in the anchorview
 *
 * Sets the text of the anchorview to @p text. This text can include markup
 * format tags, including <c>\<a href=anchorname\></c> to begin a segment of
 * text that will be specially styled and react to click events, ended with
 * either of \</a\> or \</\>. When clicked, the anchor will emit an
 * "anchor,clicked" signal that you can attach a callback to with
 * evas_object_smart_callback_add(). The name of the anchor given in the
 * event info struct will be the one set in the href attribute, in this
 * case, anchorname.
 *
 * Other markup can be used to style the text in different ways, but it's
 * up to the style defined in the theme which tags do what.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_anchorview_text_set(Evas_Object *obj, const char *text);

/**
 * Get the markup text set for the anchorview
 *
 * Retrieves the text set on the anchorview, with markup tags included.
 *
 * @param obj The anchorview object
 * @return The markup text set or @c NULL if nothing was set or an error
 * occurred
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_anchorview_text_get(const Evas_Object *obj);

/**
 * @brief Get the ctxpopup item's disabled/enabled state.
 *
 * @param it Ctxpopup item to be enabled/disabled
 * @return disabled @c EINA_TRUE, if disabled, @c EINA_FALSE otherwise
 *
 * @see elm_ctxpopup_item_disabled_set()
 * @deprecated use elm_object_item_disabled_get() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_ctxpopup_item_disabled_get(const Elm_Object_Item *it);

/**
 * @brief Set the ctxpopup item's state as disabled or enabled.
 *
 * @param it Ctxpopup item to be enabled/disabled
 * @param disabled @c EINA_TRUE to disable it, @c EINA_FALSE to enable it
 *
 * When disabled the item is greyed out to indicate it's state.
 * @deprecated use elm_object_item_disabled_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * @brief Get the icon object for the given ctxpopup item.
 *
 * @param it Ctxpopup item
 * @return icon object or @c NULL, if the item does not have icon or an error
 * occurred
 *
 * @see elm_ctxpopup_item_append()
 * @see elm_ctxpopup_item_icon_set()
 *
 * @deprecated use elm_object_item_part_content_get() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI Evas_Object *elm_ctxpopup_item_icon_get(const Elm_Object_Item *it);

/**
 * @brief Sets the side icon associated with the ctxpopup item
 *
 * @param it Ctxpopup item
 * @param icon Icon object to be set
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * disappear from the first item.
 *
 * @see elm_ctxpopup_item_append()
 *
 * @deprecated use elm_object_item_part_content_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * @brief Get the label for the given ctxpopup item.
 *
 * @param it Ctxpopup item
 * @return label string or @c NULL, if the item does not have label or an
 * error occurred
 *
 * @see elm_ctxpopup_item_append()
 * @see elm_ctxpopup_item_label_set()
 *
 * @deprecated use elm_object_item_text_get() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI const char  *elm_ctxpopup_item_label_get(const Elm_Object_Item *it);

/**
 * @brief (Re)set the label on the given ctxpopup item.
 *
 * @param it Ctxpopup item
 * @param label String to set as label
 *
 * @deprecated use elm_object_item_text_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * @brief Set an elm widget as the content of the ctxpopup.
 *
 * @param obj Ctxpopup object
 * @param content Content to be swallowed
 *
 * If the content object is already set, a previous one will be deleted. If
 * you want to keep that old content object, use the
 * elm_ctxpopup_content_unset() function.
 *
 * @warning Ctxpopup can't hold both a item list and a content at the same
 * time. When a content is set, any previous items will be removed.
 *
 * @deprecated use elm_object_content_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Unset the ctxpopup content
 *
 * @param obj Ctxpopup object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @deprecated use elm_object_content_unset()
 *
 * @see elm_ctxpopup_content_set()
 *
 * @deprecated use elm_object_content_unset() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI Evas_Object *elm_ctxpopup_content_unset(Evas_Object *obj);

/**
 * @brief Delete the given item in a ctxpopup object.
 *
 * @param it Ctxpopup item to be deleted
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_ctxpopup_item_append()
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void                         elm_ctxpopup_item_del(Elm_Object_Item *it);

/**
 * Set the label for a given file selector button widget
 *
 * @param obj The file selector button widget
 * @param label The text label to be displayed on @p obj
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_fileselector_button_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label set for a given file selector button widget
 *
 * @param obj The file selector button widget
 * @return The button label
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_button_label_get(const Evas_Object *obj);

/**
 * Set the icon on a given file selector button widget
 *
 * @param obj The file selector button widget
 * @param icon The icon object for the button
 *
 * Once the icon object is set, a previously set one will be
 * deleted. If you want to keep the latter, use the
 * elm_fileselector_button_icon_unset() function.
 *
 * @deprecated Use elm_object_part_content_set() instead
 * @see elm_fileselector_button_icon_get()
 */
EINA_DEPRECATED EAPI void                        elm_fileselector_button_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon set for a given file selector button widget
 *
 * @param obj The file selector button widget
 * @return The icon object currently set on @p obj or @c NULL, if
 * none is
 *
 * @deprecated Use elm_object_part_content_get() instead
 * @see elm_fileselector_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_button_icon_get(const Evas_Object *obj);

/**
 * Unset the icon used in a given file selector button widget
 *
 * @param obj The file selector button widget
 * @return The icon object that was being used on @p obj or @c
 * NULL, on errors
 *
 * Unparent and return the icon object which was set for this
 * widget.
 *
 * @deprecated Use elm_object_part_content_unset() instead
 * @see elm_fileselector_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_button_icon_unset(Evas_Object *obj);

/**
 * Set the label for a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @param label The text label to be displayed on @p obj widget's
 * button
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_button_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label set for a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @return The widget button's label
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_entry_button_label_get(const Evas_Object *obj);

/**
 * Set the icon on a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @param icon The icon object for the entry's button
 *
 * Once the icon object is set, a previously set one will be
 * deleted. If you want to keep the latter, use the
 * elm_fileselector_entry_button_icon_unset() function.
 *
 * @deprecated Use elm_object_part_content_set() instead
 * @see elm_fileselector_entry_button_icon_get()
 */
EINA_DEPRECATED EAPI void                        elm_fileselector_entry_button_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon set for a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @return The icon object currently set on @p obj widget's button
 * or @c NULL, if none is
 *
 * @deprecated Use elm_object_part_content_get() instead
 * @see elm_fileselector_entry_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_entry_button_icon_get(const Evas_Object *obj);

/**
 * Unset the icon used in a given file selector entry widget's
 * button
 *
 * @param obj The file selector entry widget
 * @return The icon object that was being used on @p obj widget's
 * button or @c NULL, on errors
 *
 * Unparent and return the icon object which was set for this
 * widget's button.
 *
 * @deprecated Use elm_object_part_content_unset() instead
 * @see elm_fileselector_entry_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_entry_button_icon_unset(Evas_Object *obj);

/**
 * @brief Sets the content of the hover object and the direction in which it
 * will pop out.
 *
 * @param obj The hover object
 * @param swallow The direction that the object will be displayed
 * at. Accepted values are "left", "top-left", "top", "top-right",
 * "right", "bottom-right", "bottom", "bottom-left", "middle" and
 * "smart".
 * @param content The content to place at @p swallow
 *
 * Once the content object is set for a given direction, a previously
 * set one (on the same direction) will be deleted. If you want to
 * keep that old content object, use the elm_object_part_content_unset()
 * function.
 *
 * All directions may have contents at the same time, except for
 * "smart". This is a special placement hint and its use case
 * depends of the calculations coming from
 * elm_hover_best_content_location_get(). Its use is for cases when
 * one desires only one hover content, but with a dynamic special
 * placement within the hover area. The content's geometry, whenever
 * it changes, will be used to decide on a best location, not
 * extrapolating the hover's parent object view to show it in (still
 * being the hover's target determinant of its medium part -- move and
 * resize it to simulate finger sizes, for example). If one of the
 * directions other than "smart" are used, a previously content set
 * using it will be deleted, and vice-versa.
 *
 * @deprecated Use elm_object_part_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);

/**
 * @brief Get the content of the hover object, in a given direction.
 *
 * Return the content object which was set for this widget in the
 * @p swallow direction.
 *
 * @param obj The hover object
 * @param swallow The direction that the object was display at.
 * @return The content that was being used
 *
 * @deprecated Use elm_object_part_content_get() instead
 * @see elm_object_part_content_set()
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hover_content_get(const Evas_Object *obj, const char *swallow);

/**
 * @brief Unset the content of the hover object, in a given direction.
 *
 * Unparent and return the content object set at @p swallow direction.
 *
 * @param obj The hover object
 * @param swallow The direction that the object was display at.
 * @return The content that was being used.
 *
 * @deprecated Use elm_object_part_content_unset() instead
 * @see elm_object_part_content_set()
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hover_content_unset(Evas_Object *obj, const char *swallow);

/**
 * @brief Set the hoversel button label
 *
 * @param obj The hoversel object
 * @param label The label text.
 *
 * This sets the label of the button that is always visible (before it is
 * clicked and expanded).
 *
 * @deprecated elm_object_text_set()
 */
EINA_DEPRECATED EAPI void         elm_hoversel_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the hoversel button label
 *
 * @param obj The hoversel object
 * @return The label text.
 *
 * @deprecated elm_object_text_get()
 */
EINA_DEPRECATED EAPI const char  *elm_hoversel_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @param icon The icon object
 *
 * Sets the icon of the button that is always visible (before it is clicked
 * and expanded).  Once the icon object is set, a previously set one will be
 * deleted, if you want to keep that old content object, use the
 * elm_hoversel_icon_unset() function.
 *
 * @see elm_object_content_set() for the button widget
 * @deprecated Use elm_object_item_part_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @return The icon object
 *
 * Get the icon of the button that is always visible (before it is clicked
 * and expanded). Also see elm_object_content_get() for the button widget.
 *
 * @see elm_hoversel_icon_set()
 * @deprecated Use elm_object_item_part_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hoversel_icon_get(const Evas_Object *obj);

/**
 * @brief Get and unparent the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @return The icon object that was being used
 *
 * Unparent and return the icon of the button that is always visible
 * (before it is clicked and expanded).
 *
 * @see elm_hoversel_icon_set()
 * @see elm_object_content_unset() for the button widget
 * @deprecated Use elm_object_item_part_content_unset() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hoversel_icon_unset(Evas_Object *obj);

/**
 * @brief This returns the data pointer supplied with elm_hoversel_item_add()
 * that will be passed to associated function callbacks.
 *
 * @param it The item to get the data from
 * @return The data pointer set with elm_hoversel_item_add()
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void        *elm_hoversel_item_data_get(const Elm_Object_Item *it);

/**
 * @brief This returns the label text of the given hoversel item.
 *
 * @param it The item to get the label
 * @return The label text of the hoversel item
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_text_get() instead
 */
EINA_DEPRECATED EAPI const char  *elm_hoversel_item_label_get(const Elm_Object_Item *it);

/**
 * @brief Set the function to be called when an item from the hoversel is
 * freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * That function will receive these parameters:
 * @li void * item data
 * @li Evas_Object * hoversel object
 * @li Elm_Object_Item * hoversel item
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_del_cb_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_hoversel_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * @brief Delete an item from the hoversel
 *
 * @param it The item to delete
 *
 * This deletes the item from the hoversel (should not be called while the
 * hoversel is active; use elm_hoversel_expanded_get() to check first).
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_hoversel_item_add()
 */
EINA_DEPRECATED EAPI void                         elm_hoversel_item_del(Elm_Object_Item *it);

/**
 * Set actionslider labels.
 *
 * @param obj The actionslider object
 * @param left_label The label to be set on the left.
 * @param center_label The label to be set on the center.
 * @param right_label The label to be set on the right.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_actionslider_labels_set(Evas_Object *obj, const char *left_label, const char *center_label, const char *right_label);

/**
 * Get actionslider labels.
 *
 * @param obj The actionslider object
 * @param left_label A char** to place the left_label of @p obj into.
 * @param center_label A char** to place the center_label of @p obj into.
 * @param right_label A char** to place the right_label of @p obj into.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_actionslider_labels_get(const Evas_Object *obj, const char **left_label, const char **center_label, const char **right_label);

/**
 * Set the label used on the indicator.
 *
 * @param obj The actionslider object
 * @param label The label to be set on the indicator.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_actionslider_indicator_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label used on the indicator object.
 *
 * @param obj The actionslider object
 * @return The indicator label
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char *elm_actionslider_indicator_label_get(Evas_Object *obj);

/**
 * Set the overlay object used for the background object.
 *
 * @param obj The bg object
 * @param overlay The overlay object
 *
 * This provides a way for elm_bg to have an 'overlay' that will be on top
 * of the bg. Once the over object is set, a previously set one will be
 * deleted, even if you set the new one to NULL. If you want to keep that
 * old content object, use the elm_bg_overlay_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 * @ingroup Bg
 */

EINA_DEPRECATED EAPI void         elm_bg_overlay_set(Evas_Object *obj, Evas_Object *overlay);

/**
 * Get the overlay object used for the background object.
 *
 * @param obj The bg object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 * @ingroup Bg
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bg_overlay_get(const Evas_Object *obj);

/**
 * Get the overlay object used for the background object.
 *
 * @param obj The bg object
 * @return The content that was being used
 *
 * Unparent and return the overlay object which was set for this widget
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 * @ingroup Bg
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bg_overlay_unset(Evas_Object *obj);


/**
 * Set the label of the bubble
 *
 * @param obj The bubble object
 * @param label The string to set in the label
 *
 * This function sets the title of the bubble. Where this appears depends on
 * the selected corner.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_bubble_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of the bubble
 *
 * @param obj The bubble object
 * @return The string of set in the label
 *
 * This function gets the title of the bubble.
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_bubble_label_get(const Evas_Object *obj);

/**
 * Set the info of the bubble
 *
 * @param obj The bubble object
 * @param info The given info about the bubble
 *
 * This function sets the info of the bubble. Where this appears depends on
 * the selected corner.
 * @deprecated use elm_object_part_text_set() instead. (with "info" as the parameter).
 */
EINA_DEPRECATED EAPI void         elm_bubble_info_set(Evas_Object *obj, const char *info);

/**
 * Get the info of the bubble
 *
 * @param obj The bubble object
 *
 * @return The "info" string of the bubble
 *
 * This function gets the info text.
 * @deprecated use elm_object_part_text_get() instead. (with "info" as the parameter).
 */
EINA_DEPRECATED EAPI const char  *elm_bubble_info_get(const Evas_Object *obj);

/**
 * Set the content to be shown in the bubble
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_bubble_content_unset() function.
 *
 * @param obj The bubble object
 * @param content The given content of the bubble
 *
 * This function sets the content shown on the middle of the bubble.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_bubble_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content shown in the bubble
 *
 * Return the content object which is set for this widget.
 *
 * @param obj The bubble object
 * @return The content that is being used
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_content_get(const Evas_Object *obj);

/**
 * Unset the content shown in the bubble
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The content that was being used
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_content_unset(Evas_Object *obj);

/**
 * Set the icon of the bubble
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_icon_content_unset() function.
 *
 * @param obj The bubble object
 * @param icon The given icon for the bubble
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon of the bubble
 *
 * @param obj The bubble object
 * @return The icon for the bubble
 *
 * This function gets the icon shown on the top left of bubble.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_icon_get(const Evas_Object *obj);

/**
 * Unset the icon of the bubble
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The icon that was being used
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_icon_unset(Evas_Object *obj);


/**
 * Set the label used in the button
 *
 * The passed @p label can be NULL to clean any existing text in it and
 * leave the button as an icon only object.
 *
 * @param obj The button object
 * @param label The text will be written on the button
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_button_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label set for the button
 *
 * The string returned is an internal pointer and should not be freed or
 * altered. It will also become invalid when the button is destroyed.
 * The string returned, if not NULL, is a stringshare, so if you need to
 * keep it around even after the button is destroyed, you can use
 * eina_stringshare_ref().
 *
 * @param obj The button object
 * @return The text set to the label, or NULL if nothing is set
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_button_label_get(const Evas_Object *obj);

/**
 * Set the icon used for the button
 *
 * Setting a new icon will delete any other that was previously set, making
 * any reference to them invalid. If you need to maintain the previous
 * object alive, unset it first with elm_button_icon_unset().
 *
 * @param obj The button object
 * @param icon The icon object for the button
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_button_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon used for the button
 *
 * Return the icon object which is set for this widget. If the button is
 * destroyed or another icon is set, the returned object will be deleted
 * and any reference to it will be invalid.
 *
 * @param obj The button object
 * @return The icon object that is being used
 *
 * @deprecated use elm_object_part_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_button_icon_get(const Evas_Object *obj);

/**
 * Remove the icon set without deleting it and return the object
 *
 * This function drops the reference the button holds of the icon object
 * and returns this last object. It is used in case you want to remove any
 * icon, or set another one, without deleting the actual object. The button
 * will be left without an icon set.
 *
 * @param obj The button object
 * @return The icon object that was being used
 * @deprecated use elm_object_part_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_button_icon_unset(Evas_Object *obj);

/**
 * Set a day text color to the same that represents Saturdays.
 *
 * @param obj The calendar object.
 * @param pos The text position. Position is the cell counter, from left
 * to right, up to down. It starts on 0 and ends on 41.
 *
 * @deprecated use elm_calendar_mark_add() instead like:
 *
 * @code
 * struct tm t = { 0, 0, 12, 6, 0, 0, 6, 6, -1 };
 * elm_calendar_mark_add(obj, "sat", &t, ELM_CALENDAR_WEEKLY);
 * @endcode
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
EINA_DEPRECATED EAPI void elm_calendar_text_saturday_color_set(Evas_Object *obj, int pos);

/**
 * Set a day text color to the same that represents Sundays.
 *
 * @param obj The calendar object.
 * @param pos The text position. Position is the cell counter, from left
 * to right, up to down. It starts on 0 and ends on 41.

 * @deprecated use elm_calendar_mark_add() instead like:
 *
 * @code
 * struct tm t = { 0, 0, 12, 7, 0, 0, 0, 0, -1 };
 * elm_calendar_mark_add(obj, "sat", &t, ELM_CALENDAR_WEEKLY);
 * @endcode
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
EINA_DEPRECATED EAPI void elm_calendar_text_sunday_color_set(Evas_Object *obj, int pos);

/**
 * Set a day text color to the same that represents Weekdays.
 *
 * @param obj The calendar object
 * @param pos The text position. Position is the cell counter, from left
 * to right, up to down. It starts on 0 and ends on 41.
 *
 * @deprecated use elm_calendar_mark_add() instead like:
 *
 * @code
 * struct tm t = { 0, 0, 12, 1, 0, 0, 0, 0, -1 };
 *
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // monday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // tuesday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // wednesday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // thursday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // friday
 * @endcode
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
EINA_DEPRECATED EAPI void elm_calendar_text_weekday_color_set(Evas_Object *obj, int pos);


/**
 * @brief Set the text label of the check object
 *
 * @param obj The check object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_check_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the text label of the check object
 *
 * @param obj The check object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_check_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon object of the check object
 *
 * @param obj The check object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_check_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object of the check object
 *
 * @param obj The check object
 * @return The icon object
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the check object
 *
 * @param obj The check object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_unset(Evas_Object *obj);

/**
 * @brief Set the text label of the check object
 *
 * @param obj The check object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_check_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the text label of the check object
 *
 * @param obj The check object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_check_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon object of the check object
 *
 * @param obj The check object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_check_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object of the check object
 *
 * @param obj The check object
 * @return The icon object
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the check object
 *
 * @param obj The check object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_unset(Evas_Object *obj);

EINA_DEPRECATED EAPI void         elm_check_states_labels_set(Evas_Object *obj, const char *ontext, const char *offtext);
EINA_DEPRECATED EAPI void         elm_check_states_labels_get(const Evas_Object *obj, const char **ontext, const char **offtext);


/**
 * Set the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @param content The content to be displayed by the conformant.
 *
 * Content will be sized and positioned considering the space required
 * to display a virtual keyboard. So it won't fill all the conformant
 * size. This way is possible to be sure that content won't resize
 * or be re-positioned after the keyboard is displayed.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @see elm_object_content_unset()
 * @see elm_object_content_get()
 *
 * @deprecated use elm_object_content_set() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI void         elm_conformant_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @return The content that is being used.
 *
 * Return the content object which is set for this widget.
 * It won't be unparent from conformant. For that, use
 * elm_object_content_unset().
 *
 * @see elm_object_content_set().
 * @see elm_object_content_unset()
 *
 * @deprecated use elm_object_content_get() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI Evas_Object *elm_conformant_content_get(const Evas_Object *obj);

/**
 * Unset the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @return The content that was being used.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_object_content_set().
 *
 * @deprecated use elm_object_content_unset() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI Evas_Object *elm_conformant_content_unset(Evas_Object *obj);

/**
 * Get the side labels max length.
 *
 * @deprecated use elm_diskselector_side_label_length_get() instead:
 *
 * @param obj The diskselector object.
 * @return The max length defined for side labels, or 0 if not a valid
 * diskselector.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI int    elm_diskselector_side_label_lenght_get(const Evas_Object *obj);

/**
 * Set the side labels max length.
 *
 * @deprecated use elm_diskselector_side_label_length_set() instead:
 *
 * @param obj The diskselector object.
 * @param len The max length defined for side labels.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void   elm_diskselector_side_label_lenght_set(Evas_Object *obj, int len);

/**
 * Get the data associated to the item.
 *
 * @param it The diskselector item
 * @return The data associated to @p it
 *
 * The return value is a pointer to data associated to @p item when it was
 * created, with function elm_diskselector_item_append(). If no data
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_diskselector_item_append()
 * @deprecated Use elm_object_item_data_get()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                  *elm_diskselector_item_data_get(const Elm_Object_Item *it);

/**
 * Set the icon associated to the item.
 *
 * @param it The diskselector item
 * @param icon The icon object to associate with @p it
 *
 * The icon object to use at left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * disappear from the first item.
 *
 * If an icon was passed as argument on item creation, with function
 * elm_diskselector_item_append(), it will be already
 * associated to the item.
 *
 * @see elm_diskselector_item_append()
 * @see elm_diskselector_item_icon_get()
 *
 * @deprecated Use elm_object_item_part_content_set() instead
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * Get the icon associated to the item.
 *
 * @param it The diskselector item
 * @return The icon associated to @p it
 *
 * The return value is a pointer to the icon associated to @p item when it was
 * created, with function elm_diskselector_item_append(), or later
 * with function elm_diskselector_item_icon_set. If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_diskselector_item_append()
 * @see elm_diskselector_item_icon_set()
 *
 * @deprecated Use elm_object_item_part_content_set() instead
 * @ingroup Diskselector
 */
EAPI Evas_Object           *elm_diskselector_item_icon_get(const Elm_Object_Item *it);

/**
 * Set the label of item.
 *
 * @param it The item of diskselector.
 * @param label The label of item.
 *
 * The label to be displayed by the item.
 *
 * If no icon is set, label will be centered on item position, otherwise
 * the icon will be placed at left of the label, that will be shifted
 * to the right.
 *
 * An item with label "January" would be displayed on side position as
 * "Jan" if max length is set to 3 with function
 * elm_diskselector_side_label_length_set(), or "Janu", if this property
 * is set to 4.
 *
 * When this @p item is selected, the entire label will be displayed,
 * except for width restrictions.
 * In this case label will be cropped and "..." will be concatenated,
 * but only for display purposes. It will keep the entire string, so
 * if diskselector is resized the remaining characters will be displayed.
 *
 * If a label was passed as argument on item creation, with function
 * elm_diskselector_item_append(), it will be already
 * displayed by the item.
 *
 * @see elm_diskselector_side_label_length_set()
 * @see elm_diskselector_item_label_get()
 * @see elm_diskselector_item_append()
 *
 * @deprecated Use elm_object_item_text_set() instead
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * Get the label of item.
 *
 * @param it The item of diskselector.
 * @return The label of item.
 *
 * The return value is a pointer to the label associated to @p item when it was
 * created, with function elm_diskselector_item_append(), or later
 * with function elm_diskselector_item_label_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_diskselector_item_label_set() for more details.
 * @see elm_diskselector_item_append()
 * @deprecated Use elm_object_item_text_get() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI const char            *elm_diskselector_item_label_get(const Elm_Object_Item *it);

/**
 * Set the function called when a diskselector item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * If there is a @p func, then it will be called prior item's memory release.
 * That will be called with the following arguments:
 * @li item's data;
 * @li item's Evas object;
 * @li item itself;
 *
 * This way, a data associated to a diskselector item could be properly
 * freed.
 * @deprecated Use elm_object_item_del_cb_set() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Delete them item from the diskselector.
 *
 * @param it The item of diskselector to be deleted.
 *
 * If deleting all diskselector items is required, elm_diskselector_clear()
 * should be used instead of getting items list and deleting each one.
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_diskselector_clear()
 * @see elm_diskselector_item_append()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_del(Elm_Object_Item *it);

/**
 * Set the text to be shown in the diskselector item.
 *
 * @param it Target item
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data is removed.
 *
 * @deprecated Use elm_object_item_tooltip_text_set() instead
 * @see elm_object_tooltip_text_set() for more details.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the diskselector item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @p func, the tooltip is unset with
 *        elm_diskselector_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @deprecated Use elm_object_item_tooltip_content_cb_set() instead
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it diskselector item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_diskselector_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @deprecated Use elm_object_item_tooltip_unset() instead
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_diskselector_item_tooltip_content_cb_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_diskselector_item_tooltip_content_cb_set() or
 *       elm_diskselector_item_tooltip_text_set()
 *
 * @param it diskselector item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @deprecated Use elm_object_item_tooltip_style_set() instead
 * @see elm_object_tooltip_style_set() for more details.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param it diskselector item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @deprecated Use elm_object_item_tooltip_style_get() instead
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_diskselector_item_tooltip_style_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI const char            *elm_diskselector_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * Set the cursor to be shown when mouse is over the diskselector item
 *
 * @param it Target item
 * @param cursor the cursor name to be used.
 *
 * @see elm_object_cursor_set() for more details.
 * @deprecated Use elm_object_item_cursor_set() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/**
 * Get the cursor to be shown when mouse is over the diskselector item
 *
 * @param it diskselector item with cursor already set.
 * @return the cursor name.
 *
 * @see elm_object_cursor_get() for more details.
 * @see elm_diskselector_item_cursor_set()
 * @deprecated Use elm_object_item_cursor_get() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI const char            *elm_diskselector_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset the cursor to be shown when mouse is over the diskselector item
 *
 * @param it Target item
 *
 * @see elm_object_cursor_unset() for more details.
 * @see elm_diskselector_item_cursor_set()
 * @deprecated use elm_object_item_cursor_unset() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_cursor_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item cursor.
 *
 * @note before you set a style you should define a cursor with
 *       elm_diskselector_item_cursor_set()
 *
 * @param it diskselector item with cursor already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @see elm_object_cursor_style_set() for more details.
 * @deprecated Use elm_object_item_cursor_style_set() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item cursor.
 *
 * @param it diskselector item with cursor already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then @c NULL is returned.
 *
 * @see elm_object_cursor_style_get() for more details.
 * @see elm_diskselector_item_cursor_style_set()
 * @deprecated Use elm_object_item_cursor_style_get() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI const char            *elm_diskselector_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Set if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a cursor
 * with elm_diskselector_item_cursor_set().
 * By default it will only look for cursors provided by the engine.
 *
 * @param it widget item with cursor already set.
 * @param engine_only boolean to define if cursors set with
 * elm_diskselector_item_cursor_set() should be searched only
 * between cursors provided by the engine or searched on widget's
 * theme as well.
 *
 * @deprecated Use elm_object_item_cursor_engine_only_set() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only);

/**
 * Get the cursor engine only usage for this item cursor.
 *
 * @param it widget item with cursor already set.
 * @return engine_only boolean to define it cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well.
 * If the item does not have a cursor set, then @c EINA_FALSE is returned.
 *
 * @see elm_diskselector_item_cursor_engine_only_set()
 * @deprecated Use elm_object_item_cursor_engine_only_get() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI Eina_Bool              elm_diskselector_item_cursor_engine_only_get(const Elm_Object_Item *it);

EINA_DEPRECATED EAPI void         elm_factory_content_set(Evas_Object *obj, Evas_Object *content);
EINA_DEPRECATED EAPI Evas_Object *elm_factory_content_get(const Evas_Object *obj);

/**
 * Get the label of a given flip selector widget's item.
 *
 * @param it The item to get label from
 * @return The text label of @p item or @c NULL, on errors
 *
 * @see elm_object_item_text_set()
 *
 * @deprecated see elm_object_item_text_get() instead
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI const char *elm_flipselector_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of a given flip selector widget's item.
 *
 * @param it The item to set label on
 * @param label The text label string, in UTF-8 encoding
 *
 * @see elm_object_item_text_get()
 *
 * @deprecated see elm_object_item_text_set() instead
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI void        elm_flipselector_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * Delete a given item from a flip selector widget.
 *
 * @param it The item to delete
 *
 * @deprecated Use elm_object_item_del() instead
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI void       elm_flipselector_item_del(Elm_Object_Item *it);

/**
 * Set the interval on time updates for a user mouse button hold
 * on a flip selector widget.
 *
 * @param obj The flip selector object
 * @param interval The (first) interval value in seconds
 *
 * This interval value is @b decreased while the user holds the
 * mouse pointer either flipping up or flipping down a given flip
 * selector.
 *
 * This helps the user to get to a given item distant from the
 * current one easier/faster, as it will start to flip quicker and
 * quicker on mouse button holds.
 *
 * The calculation for the next flip interval value, starting from
 * the one set with this call, is the previous interval divided by
 * 1.05, so it decreases a little bit.
 *
 * The default starting interval value for automatic flips is
 * @b 0.85 seconds.
 *
 * @see elm_flipselector_interval_get()
 * @deprecated Use elm_flipselector_first_interval_set()
 *
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI void                        elm_flipselector_interval_set(Evas_Object *obj, double interval);

/**
 * Get the interval on time updates for an user mouse button hold
 * on a flip selector widget.
 *
 * @param obj The flip selector object
 * @return The (first) interval value, in seconds, set on it
 *
 * @see elm_flipselector_interval_set() for more details
 * @deprecated Use elm_flipselector_first_interval_get()
 *
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI double                      elm_flipselector_interval_get(const Evas_Object *obj);

/**
 * Make a given Elementary object the focused one.
 *
 * @param obj The Elementary object to make focused.
 *
 * @note This object, if it can handle focus, will take the focus
 * away from the one who had it previously and will, for now on, be
 * the one receiving input events.
 *
 * @see elm_object_focus_get()
 * @deprecated use elm_object_focus_set() instead.
 *
 * @ingroup Focus
 */
EINA_DEPRECATED EAPI void elm_object_focus(Evas_Object *obj);

/**
 * Remove the focus from an Elementary object
 *
 * @param obj The Elementary to take focus from
 *
 * This removes the focus from @p obj, passing it back to the
 * previous element in the focus chain list.
 *
 * @see elm_object_focus() and elm_object_focus_custom_chain_get()
 * @deprecated use elm_object_focus_set() instead.
 *
 * @ingroup Focus
 */
EINA_DEPRECATED EAPI void elm_object_unfocus(Evas_Object *obj);

/**
 * Give focus to near object in one direction.
 *
 * Give focus to near object in direction of one object.
 * If none focusable object in given direction, the focus will not change.
 *
 * @param obj The reference object
 * @param x Horizontal component of direction to focus
 * @param y Vertical component of direction to focus
 *
 * @deprecated Support it later.
 * @ingroup Focus
 */
EINA_DEPRECATED EAPI void                 elm_object_focus_direction_go(Evas_Object *obj, int x, int y);

/**
 * @brief Set the frame label
 *
 * @param obj The frame object
 * @param label The label of this frame object
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_frame_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the frame label
 *
 * @param obj The frame object
 *
 * @return The label of this frame object or NULL if unable to get frame
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_frame_label_get(const Evas_Object *obj);

/**
 * @brief Set the content of the frame widget
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_frame_content_unset() function.
 *
 * @param obj The frame object
 * @param content The content will be filled in this frame object
 *
 * @deprecated use elm_object_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_frame_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Get the content of the frame widget
 *
 * Return the content object which is set for this widget
 *
 * @param obj The frame object
 * @return The content that is being used
 *
 * @deprecated use elm_object_content_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_frame_content_get(const Evas_Object *obj);

/**
 * @brief Unset the content of the frame widget
 *
 * Unparent and return the content object which was set for this widget
 *
 * @param obj The frame object
 * @return The content that was being used
 *
 * @deprecated use elm_object_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_frame_content_unset(Evas_Object *obj);

EINA_DEPRECATED EAPI void          elm_genlist_horizontal_mode_set(Evas_Object *obj, Elm_List_Mode mode);
EINA_DEPRECATED EAPI Elm_List_Mode elm_genlist_horizontal_mode_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_genlist_item_icons_orphan(Elm_Object_Item *it);
/**
 * This sets the horizontal stretching mode.
 *
 * @param obj The genlist object
 * @param mode The mode to use (one of #ELM_LIST_SCROLL or #ELM_LIST_LIMIT).
 *
 * This sets the mode used for sizing items horizontally. Valid modes
 * are #ELM_LIST_LIMIT and #ELM_LIST_SCROLL. The default is
 * ELM_LIST_SCROLL. This mode means that if items are too wide to fit,
 * the scroller will scroll horizontally. Otherwise items are expanded
 * to fill the width of the viewport of the scroller. If it is
 * ELM_LIST_LIMIT, items will be expanded to the viewport width and
 * limited to that size.
 *
 * @see elm_genlist_horizontal_get()
 *
 * @deprecated use elm_genlist_mode_set()
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void                          elm_genlist_horizontal_set(Evas_Object *obj, Elm_List_Mode mode);

/**
 * Gets the horizontal stretching mode.
 *
 * @param obj The genlist object
 * @return The mode to use
 * (#ELM_LIST_LIMIT, #ELM_LIST_SCROLL)
 *
 * @see elm_genlist_horizontal_set()
 *
 * @deprecated use elm_genlist_mode_get()
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI Elm_List_Mode                 elm_genlist_horizontal_get(const Evas_Object *obj);

/**
 * Return the data associated to a given genlist item
 *
 * @param it The genlist item.
 * @return the data associated to this item.
 *
 * This returns the @c data value passed on the
 * elm_genlist_item_append() and related item addition calls.
 *
 * @see elm_genlist_item_append()
 * @see elm_genlist_item_data_set()
 *
 * @deprecated Use elm_object_item_data_get() instead
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void                         *elm_genlist_item_data_get(const Elm_Object_Item *it);

/**
 * Set the data associated to a given genlist item
 *
 * @param it The genlist item
 * @param data The new data pointer to set on it
 *
 * This @b overrides the @c data value passed on the
 * elm_genlist_item_append() and related item addition calls. This
 * function @b won't call elm_genlist_item_update() automatically,
 * so you'd issue it afterwards if you want to hove the item
 * updated to reflect the that new data.
 *
 * @see elm_genlist_item_data_get()
 *
 * @deprecated Use elm_object_item_data_set() instead
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * Set whether a given genlist item is disabled or not.
 *
 * @param it The item
 * @param disabled Use @c EINA_TRUE, true disable it, @c EINA_FALSE
 * to enable it back.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change its appearance, to signal the user it's disabled.
 *
 * @see elm_genlist_item_disabled_get()
 * @deprecated Use elm_object_item_disabled_set() instead
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Get whether a given genlist item is disabled or not.
 *
 * @param it The item
 * @return @c EINA_TRUE, if it's disabled, @c EINA_FALSE otherwise
 * (and on errors).
 *
 * @see elm_genlist_item_disabled_set() for more details
 * @deprecated Use elm_object_item_disabled_get() instead
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_genlist_item_disabled_get(const Elm_Object_Item *it);

/**
 * Remove a genlist item from the its parent, deleting it.
 *
 * @param it The item to be removed.
 * @return @c EINA_TRUE on success or @c EINA_FALSE, otherwise.
 *
 * @see elm_genlist_clear(), to remove all items in a genlist at
 * once.
 *
 * @deprecated Use elm_object_item_del() instead
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_del(Elm_Object_Item *it);

/**
 * Get the genlist object's handle which contains a given genlist
 * item
 *
 * @param it The item to fetch the container from
 * @return The genlist (parent) object
 *
 * This returns the genlist object itself that an item belongs to.
 * @deprecated Use elm_object_item_widget_get() instead
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI Evas_Object                  *elm_genlist_item_genlist_get(const Elm_Object_Item *it);

/**
 * Get the mode item style of items in the genlist
 * @param obj The genlist object
 * @return The mode item style string, or NULL if none is specified
 *
 * This is a constant string and simply defines the name of the
 * style that will be used for mode animations. It can be
 * @c NULL if you don't plan to use Genlist mode. See
 * elm_genlist_item_mode_set() for more info.
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI const char                   *elm_genlist_mode_item_style_get(const Evas_Object *obj);

/**
 * Set the mode item style of items in the genlist
 * @param obj The genlist object
 * @param style The mode item style string, or NULL if none is desired
 *
 * This is a constant string and simply defines the name of the
 * style that will be used for mode animations. It can be
 * @c NULL if you don't plan to use Genlist mode. See
 * elm_genlist_item_mode_set() for more info.
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void                          elm_genlist_mode_item_style_set(Evas_Object *obj, const char *style);

/**
 * Get the real Evas object created to implement the view of a
 * given genlist item
 *
 * @param it The genlist item.
 * @return the Evas object implementing this item's view.
 *
 * This returns the actual Evas object used to implement the
 * specified genlist item's view. This may be @c NULL, as it may
 * not have been created or may have been deleted, at any time, by
 * the genlist. <b>Do not modify this object</b> (move, resize,
 * show, hide, etc.), as the genlist is controlling it. This
 * function is for querying, emitting custom signals or hooking
 * lower level callbacks for events on that object. Do not delete
 * this object under any circumstances.
 *
 * @see elm_object_item_data_get()
 * @deprecated No more support
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI const Evas_Object            *elm_genlist_item_object_get(const Elm_Object_Item *it);

/**
 * Get the Item's Flags
 *
 * @param it The genlist item
 * @return The item type.
 *
 * This function returns the item's type. Normally the item's type.
 * If it failed, return value is ELM_GENLIST_ITEM_MAX
 *
 * @deprecated Use elm_genlist_item_type_get() instead
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI Elm_Genlist_Item_Type        elm_genlist_item_flags_get(const Elm_Object_Item *it);

/**
 * Tells genlist to "orphan" contents fetched by the item class
 *
 * @param it The item
 *
 * This instructs genlist to release references to contents in the item,
 * meaning that they will no longer be managed by genlist and are
 * floating "orphans" that can be re-used elsewhere if the user wants
 * to.
 *
 * @deprecated Use elm_genlist_item_all_contents_unset() instead
 * @ingroup Genlist
 */
