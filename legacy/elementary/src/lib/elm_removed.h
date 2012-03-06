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

