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
 * @image html img/widget/toggle/preview-00.png
 * @image latex img/widget/toggle/preview-00.eps
 *
 * @brief A toggle is a slider which can be used to toggle between
 * two values.  It has two states: on and off.
 *
 * This widget is deprecated. Please use elm_check_add() instead using the
 * toggle style like:
 *
 * @code
 * obj = elm_check_add(parent);
 * elm_object_style_set(obj, "toggle");
 * elm_object_part_text_set(obj, "on", "ON");
 * elm_object_part_text_set(obj, "off", "OFF");
 * @endcode
 *
 * Signals that you can add callbacks for are:
 * @li "changed" - Whenever the toggle value has been changed.  Is not called
 *                 until the toggle is released by the cursor (assuming it
 *                 has been triggered by the cursor in the first place).
 *
 * Default content parts of the toggle widget that you can use for are:
 * @li "icon" - An icon of the toggle
 *
 * Default text parts of the toggle widget that you can use for are:
 * @li "elm.text" - Label of the toggle
 *
 * @ref tutorial_toggle show how to use a toggle.
 * @{
 */

/**
 * @brief Add a toggle to @p parent.
 *
 * @param parent The parent object
 *
 * @return The toggle object
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toggle_add(Evas_Object *parent);

/**
 * @brief Sets the label to be displayed with the toggle.
 *
 * @param obj The toggle object
 * @param label The label to be displayed
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the label of the toggle
 *
 * @param obj  toggle object
 * @return The label of the toggle
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_toggle_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon used for the toggle
 *
 * @param obj The toggle object
 * @param icon The icon object for the button
 *
 * Once the icon object is set, a previously set one will be deleted
 * If you want to keep that old content object, use the
 * elm_toggle_icon_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon used for the toggle
 *
 * @param obj The toggle object
 * @return The icon object that is being used
 *
 * Return the icon object which is set for this widget.
 *
 * @deprecated use elm_object_part_content_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toggle_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the toggle
 *
 * @param obj The toggle object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @deprecated use elm_object_part_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toggle_icon_unset(Evas_Object *obj);

/**
 * @brief Sets the labels to be associated with the on and off states of the toggle.
 *
 * @param obj The toggle object
 * @param onlabel The label displayed when the toggle is in the "on" state
 * @param offlabel The label displayed when the toggle is in the "off" state
 *
 * @deprecated use elm_object_part_text_set() for "on" and "off" parts
 * instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel);

/**
 * @brief Get the labels associated with the on and off states of the
 * toggle.
 *
 * @param obj The toggle object
 * @param onlabel A char** to place the onlabel of @p obj into
 * @param offlabel A char** to place the offlabel of @p obj into
 *
 * @deprecated use elm_object_part_text_get() for "on" and "off" parts
 * instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_states_labels_get(const Evas_Object *obj, const char **onlabel, const char **offlabel);

/**
 * @brief Sets the state of the toggle to @p state.
 *
 * @param obj The toggle object
 * @param state The state of @p obj
 *
 * @deprecated use elm_check_state_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_state_set(Evas_Object *obj, Eina_Bool state);

/**
 * @brief Get the state of the toggle to @p state.
 *
 * @param obj The toggle object
 * @return The state of @p obj
 *
 * @deprecated use elm_check_state_get() instead.
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_toggle_state_get(const Evas_Object *obj);

/**
 * @brief Sets the state pointer of the toggle to @p statep.
 *
 * @param obj The toggle object
 * @param statep The state pointer of @p obj
 *
 * @deprecated use elm_check_state_pointer_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);

/**
 * @}
 */

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
 * @deprecated Use elm_config_finger_size_set() and elm_config_all_flush()
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
 *        given @a data, and @p event_info is the item.
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
 * Give focus to next object in object tree.
 *
 * Give focus to next object in focus chain of one object sub-tree.
 * If the last object of chain already have focus, the focus will go to the
 * first object of chain.
 *
 * @param obj The object root of sub-tree
 * @param dir Direction to cycle the focus
 *
 * @deprecated Use elm_object_focus_next() instead
 *
 * @ingroup Focus
 */
EINA_DEPRECATED EAPI void                 elm_object_focus_cycle(Evas_Object *obj, Elm_Focus_Direction dir);

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
 * Get the horizontal stretching mode.
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
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_contents_orphan(Elm_Object_Item *it);

#define ELM_IMAGE_ROTATE_90_CW 1
#define ELM_IMAGE_ROTATE_180_CW 2
#define ELM_IMAGE_ROTATE_90_CCW 3

/**
 * Return the data associated with a given index widget item
 *
 * @param item The index widget item handle
 * @return The data associated with @p it
 * @deprecated Use elm_object_item_data_get() instead
 *
 * @see elm_index_item_data_set()
 *
 */
EINA_DEPRECATED EAPI void *elm_index_item_data_get(const Elm_Object_Item *item);

/**
 * Set the data associated with a given index widget item
 *
 * @param it The index widget item handle
 * @param data The new data pointer to set to @p it
 *
 * This sets new item data on @p it.
 *
 * @warning The old data pointer won't be touched by this function, so
 * the user had better to free that old data himself/herself.
 *
 * @deprecated Use elm_object_item_data_set() instead
 */
EINA_DEPRECATED EAPI void  elm_index_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * Set the function to be called when a given index widget item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function to call on the item's deletion
 *
 * When called, @p func will have both @c data and @p event_info
 * arguments with the @p it item's data value and, naturally, the
 * @c obj argument with a handle to the parent index widget.
 *
 * @deprecated Use elm_object_item_del_cb_set() instead
 */
EINA_DEPRECATED EAPI void                  elm_index_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Remove an item from a given index widget, <b>to be referenced by
 * it's data value</b>.
 *
 * @param obj The index object
 * @param item The item to be removed from @p obj
 *
 * If a deletion callback is set, via elm_object_item_del_cb_set(),
 * that callback function will be called by this one.
 *
 * @deprecated Use elm_object_item_del() instead
 */
EINA_DEPRECATED EAPI void                  elm_index_item_del(Evas_Object *obj, Elm_Object_Item *item);

/**
* @brief Set the label on the label object
 *
 * @param obj The label object
 * @param label The label will be used on the label object
 * @deprecated See elm_object_text_set()
 */
EINA_DEPRECATED EAPI void        elm_label_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the label used on the label object
 *
 * @param obj The label object
 * @return The string inside the label
 * @deprecated See elm_object_text_get()
 */
EINA_DEPRECATED EAPI const char *elm_label_label_get(const Evas_Object *obj);

/**
 * @def elm_layout_label_set
 * Convenience macro to set the label in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @deprecated use elm_object_text_set() instead.
 */
#define elm_layout_label_set(_ly, _txt) \
  elm_layout_text_set((_ly), "elm.text", (_txt))

/**
 * @def elm_layout_label_get
 * Convenience macro to get the label in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @deprecated use elm_object_text_set() instead.
 */
#define elm_layout_label_get(_ly) \
  elm_layout_text_get((_ly), "elm.text")

EINA_DEPRECATED EAPI Evas_Object *elm_list_item_base_get(const Elm_Object_Item *it);


/**
 * Set the content of the mapbuf.
 *
 * @param obj The mapbuf object.
 * @param content The content that will be filled in this mapbuf object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_mapbuf_content_unset() function.
 *
 * To enable map, elm_mapbuf_enabled_set() should be used.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_mapbuf_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content of the mapbuf.
 *
 * @param obj The mapbuf object.
 * @return The content that is being used.
 *
 * Return the content object which is set for this widget.
 *
 * @see elm_mapbuf_content_set() for details.
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_mapbuf_content_get(const Evas_Object *obj);

/**
 * Unset the content of the mapbuf.
 *
 * @param obj The mapbuf object.
 * @return The content that was being used.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_mapbuf_content_set() for details.
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_mapbuf_content_unset(Evas_Object *obj);

/**
 * @brief Set the label of a menu item
 *
 * @param it The menu item object.
 * @param label The label to set for @p item
 *
 * @warning Don't use this function on items created with
 * elm_menu_item_add_object() or elm_menu_item_separator_add().
 *
 * @deprecated Use elm_object_item_text_set() instead
 */
EINA_DEPRECATED EAPI void         elm_menu_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * @brief Get the label of a menu item
 *
 * @param it The menu item object.
 * @return The label of @p item
 * @deprecated Use elm_object_item_text_get() instead
 */
EINA_DEPRECATED EAPI const char  *elm_menu_item_label_get(const Elm_Object_Item *it);

/**
 * @brief Set the content object of a menu item
 *
 * @param it The menu item object
 * @param The content object or NULL
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * Use this function to change the object swallowed by a menu item, deleting
 * any previously swallowed object.
 *
 * @deprecated Use elm_object_item_content_set() instead
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_menu_item_object_content_set(Elm_Object_Item *it, Evas_Object *obj);

/**
 * @brief Get the content object of a menu item
 *
 * @param it The menu item object
 * @return The content object or NULL
 * @note If @p item was added with elm_menu_item_add_object, this
 * function will return the object passed, else it will return the
 * icon object.
 *
 * @see elm_menu_item_object_content_set()
 *
 * @deprecated Use elm_object_item_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_menu_item_object_content_get(const Elm_Object_Item *it);

/**
 * @brief Set the disabled state of @p item.
 *
 * @param it The menu item object.
 * @param disabled The enabled/disabled state of the item
 * @deprecated Use elm_object_item_disabled_set() instead
 */
EINA_DEPRECATED EAPI void         elm_menu_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * @brief Get the disabled state of @p item.
 *
 * @param it The menu item object.
 * @return The enabled/disabled state of the item
 *
 * @see elm_menu_item_disabled_set()
 * @deprecated Use elm_object_item_disabled_get() instead
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_menu_item_disabled_get(const Elm_Object_Item *it);

/**
 * @brief Returns the data associated with menu item @p item.
 *
 * @param it The item
 * @return The data associated with @p item or NULL if none was set.
 *
 * This is the data set with elm_menu_add() or elm_menu_item_data_set().
 *
 * @deprecated Use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void        *elm_menu_item_data_get(const Elm_Object_Item *it);

/**
 * @brief Set the function called when a menu item is deleted.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * @see elm_menu_item_add()
 * @see elm_menu_item_del()
 * @deprecated Use elm_object_item_del_cb_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_menu_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * @brief Sets the data to be associated with menu item @p item.
 *
 * @param it The item
 * @param data The data to be associated with @p item
 *
 * @deprecated Use elm_object_item_data_set() instead
 */
EINA_DEPRECATED EAPI void         elm_menu_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * @brief Deletes an item from the menu.
 *
 * @param it The item to delete.
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_menu_item_add()
 */
EINA_DEPRECATED EAPI void                         elm_menu_item_del(Elm_Object_Item *it);

/**
 * @brief @brief Return a menu item's owner menu
 *
 * @param it The menu item
 * @return The menu object owning @p item, or NULL on failure
 *
 * Use this function to get the menu object owning an item.
 * @deprecated Use elm_object_item_widget_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_menu_item_menu_get(const Elm_Object_Item *it);

/**
 * @brief Set the icon of a menu item to the standard icon with name @p icon
 *
 * @param it The menu item object.
 * @param icon The icon object to set for the content of @p item
 *
 * @deprecated elm_menu_item_icon_name_set() instead
 * Once this icon is set, any previously set icon will be deleted.
 */
EINA_DEPRECATED EAPI void                         elm_menu_item_object_icon_name_set(Elm_Object_Item *it, const char *icon);

/**
 * @brief Get the string representation from the icon of a menu item
 *
 * @param it The menu item object.
 * @return The string representation of @p item's icon or NULL
 *
 * @deprecated elm_menu_item_icon_name_get() instead
 * @see elm_menu_item_icon_name_set()
 */
EINA_DEPRECATED EAPI const char                  *elm_menu_item_object_icon_name_get(const Elm_Object_Item *it);

/**
 * @brief Set the content of the notify widget
 *
 * @param obj The notify object
 * @param content The content will be filled in this notify object
 *
 * Once the content object is set, a previously set one will be deleted. If
 * you want to keep that old content object, use the
 * elm_notify_content_unset() function.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_notify_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Unset the content of the notify widget
 *
 * @param obj The notify object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget
 *
 * @see elm_notify_content_set()
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_notify_content_unset(Evas_Object *obj);

/**
 * @brief Return the content of the notify widget
 *
 * @param obj The notify object
 * @return The content that is being used
 *
 * @see elm_notify_content_set()
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_notify_content_get(const Evas_Object *obj);

/**
 * Set a label of an object
 *
 * @param obj The Elementary object
 * @param part The text part name to set (NULL for the default label)
 * @param label The new text of the label
 *
 * @note Elementary objects may have many labels (e.g. Action Slider)
 * @deprecated Use elm_object_part_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_object_text_part_set(Evas_Object *obj, const char *part, const char *label);

/**
 * Get a label of an object
 *
 * @param obj The Elementary object
 * @param part The text part name to get (NULL for the default label)
 * @return text of the label or NULL for any error
 *
 * @note Elementary objects may have many labels (e.g. Action Slider)
 * @deprecated Use elm_object_part_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_object_text_part_get(const Evas_Object *obj, const char *part);

/**
 * Set a content of an object
 *
 * @param obj The Elementary object
 * @param part The content part name to set (NULL for the default content)
 * @param content The new content of the object
 *
 * @note Elementary objects may have many contents
 * @deprecated Use elm_object_part_content_set instead.
 */
EINA_DEPRECATED EAPI void         elm_object_content_part_set(Evas_Object *obj, const char *part, Evas_Object *content);

/**
 * Get a content of an object
 *
 * @param obj The Elementary object
 * @param part The content part name to get (NULL for the default content)
 * @return content of the object or NULL for any error
 *
 * @note Elementary objects may have many contents
 * @deprecated Use elm_object_part_content_get instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_object_content_part_get(const Evas_Object *obj, const char *part);

/**
 * Unset a content of an object
 *
 * @param obj The Elementary object
 * @param part The content part name to unset (NULL for the default content)
 *
 * @note Elementary objects may have many contents
 * @deprecated Use elm_object_part_content_unset instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_object_content_part_unset(Evas_Object *obj, const char *part);

/**
 * Set a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to set (NULL for the default content)
 * @param content The new content of the object item
 *
 * @note Elementary object items may have many contents
 * @deprecated Use elm_object_item_part_content_set instead.
 */
EINA_DEPRECATED EAPI void         elm_object_item_content_part_set(Elm_Object_Item *it, const char *part, Evas_Object *content);

/**
 * Get a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to unset (NULL for the default content)
 * @return content of the object item or NULL for any error
 *
 * @note Elementary object items may have many contents
 * @deprecated Use elm_object_item_part_content_get instead.
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_object_item_content_part_get(const Elm_Object_Item *it, const char *part);

/**
 * Unset a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to unset (NULL for the default content)
 *
 * @note Elementary object items may have many contents
 * @deprecated Use elm_object_item_part_content_unset instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_object_item_content_part_unset(Elm_Object_Item *it, const char *part);

/**
 * Get a label of an object item
 *
 * @param it The Elementary object item
 * @param part The text part name to get (NULL for the default label)
 * @return text of the label or NULL for any error
 *
 * @note Elementary object items may have many labels
 * @deprecated Use elm_object_item_part_text_get instead.
 */
EINA_DEPRECATED EAPI const char  *elm_object_item_text_part_get(const Elm_Object_Item *it, const char *part);

/**
 * Set a label of an object item
 *
 * @param it The Elementary object item
 * @param part The text part name to set (NULL for the default label)
 * @param label The new text of the label
 *
 * @note Elementary object items may have many labels
 * @deprecated Use elm_object_item_part_text_set instead.
 */
EINA_DEPRECATED EAPI void         elm_object_item_text_part_set(Elm_Object_Item *it, const char *part, const char *label);

/**
 * @brief Set the content of the panel.
 *
 * @param obj The panel object
 * @param content The panel content
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panel_content_unset() function.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_panel_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Get the content of the panel.
 *
 * @param obj The panel object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget.
 *
 * @see elm_panel_content_set()
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panel_content_get(const Evas_Object *obj);

/**
 * @brief Unset the content of the panel.
 *
 * @param obj The panel object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_panel_content_set()
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panel_content_unset(Evas_Object *obj);

/**
 * Set the left content of the panes widget.
 *
 * @param obj The panes object.
 * @param content The new left content object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panes_content_left_unset() function.
 *
 * If panes is displayed vertically, left content will be displayed at
 * top.
 *
 * @see elm_panes_content_left_get()
 * @see elm_panes_content_right_set() to set content on the other side.
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_panes_content_left_set(Evas_Object *obj, Evas_Object *content);

/**
 * Set the right content of the panes widget.
 *
 * @param obj The panes object.
 * @param content The new right content object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panes_content_right_unset() function.
 *
 * If panes is displayed vertically, left content will be displayed at
 * bottom.
 *
 * @see elm_panes_content_right_get()
 * @see elm_panes_content_left_set() to set content on the other side.
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_panes_content_right_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the left content of the panes.
 *
 * @param obj The panes object.
 * @return The left content object that is being used.
 *
 * Return the left content object which is set for this widget.
 *
 * @see elm_panes_content_left_set() for details.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_left_get(const Evas_Object *obj);

/**
 * Get the right content of the panes.
 *
 * @param obj The panes object
 * @return The right content object that is being used
 *
 * Return the right content object which is set for this widget.
 *
 * @see elm_panes_content_right_set() for details.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_right_get(const Evas_Object *obj);

/**
 * Unset the left content used for the panes.
 *
 * @param obj The panes object.
 * @return The left content object that was being used.
 *
 * Unparent and return the left content object which was set for this widget.
 *
 * @see elm_panes_content_left_set() for details.
 * @see elm_panes_content_left_get().
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_left_unset(Evas_Object *obj);

/**
 * Unset the right content used for the panes.
 *
 * @param obj The panes object.
 * @return The right content object that was being used.
 *
 * Unparent and return the right content object which was set for this
 * widget.
 *
 * @see elm_panes_content_right_set() for details.
 * @see elm_panes_content_right_get().
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_right_unset(Evas_Object *obj);
/**
 * Set the label of a given progress bar widget
 *
 * @param obj The progress bar object
 * @param label The text label string, in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_progressbar_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of a given progress bar widget
 *
 * @param obj The progressbar object
 * @return The text label string, in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_progressbar_label_get(const Evas_Object *obj);

/**
 * Set the icon object of a given progress bar widget
 *
 * @param obj The progress bar object
 * @param icon The icon object
 *
 * Use this call to decorate @p obj with an icon next to it.
 *
 * @note Once the icon object is set, a previously set one will be
 * deleted. If you want to keep that old content object, use the
 * elm_progressbar_icon_unset() function.
 *
 * @see elm_progressbar_icon_get()
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_progressbar_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Retrieve the icon object set for a given progress bar widget
 *
 * @param obj The progress bar object
 * @return The icon object's handle, if @p obj had one set, or @c NULL,
 * otherwise (and on errors)
 *
 * @see elm_progressbar_icon_set() for more details
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_progressbar_icon_get(const Evas_Object *obj);

/**
 * Unset an icon set on a given progress bar widget
 *
 * @param obj The progress bar object
 * @return The icon object that was being used, if any was set, or
 * @c NULL, otherwise (and on errors)
 *
 * This call will unparent and return the icon object which was set
 * for this widget, previously, on success.
 *
 * @see elm_progressbar_icon_set() for more details
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_progressbar_icon_unset(Evas_Object *obj);

/**
 * @brief Set the text label of the radio object
 *
 * @param obj The radio object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_radio_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the text label of the radio object
 *
 * @param obj The radio object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_radio_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon object of the radio object
 *
 * @param obj The radio object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted. If you
 * want to keep that old content object, use the elm_radio_icon_unset()
 * function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_radio_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object of the radio object
 *
 * @param obj The radio object
 * @return The icon object
 *
 * @see elm_radio_icon_set()
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_radio_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the radio object
 *
 * @param obj The radio object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @see elm_radio_icon_set()
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_radio_icon_unset(Evas_Object *obj);

/**
 * Set the label of a given slider widget
 *
 * @param obj The progress bar object
 * @param label The text label string, in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_slider_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of a given slider widget
 *
 * @param obj The progressbar object
 * @return The text label string, in UTF-8
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_slider_label_get(const Evas_Object *obj);

/**
 * Set the icon object of the slider object.
 *
 * @param obj The slider object.
 * @param icon The icon object.
 *
 * On horizontal mode, icon is placed at left, and on vertical mode,
 * placed at top.
 *
 * @note Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_slider_icon_unset() function.
 *
 * @warning If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_slider_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Unset an icon set on a given slider widget.
 *
 * @param obj The slider object.
 * @return The icon object that was being used, if any was set, or
 * @c NULL, otherwise (and on errors).
 *
 * On horizontal mode, icon is placed at left, and on vertical mode,
 * placed at top.
 *
 * This call will unparent and return the icon object which was set
 * for this widget, previously, on success.
 *
 * @see elm_slider_icon_set() for more details
 * @see elm_slider_icon_get()
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_icon_unset(Evas_Object *obj);

/**
 * Retrieve the icon object set for a given slider widget.
 *
 * @param obj The slider object.
 * @return The icon object's handle, if @p obj had one set, or @c NULL,
 * otherwise (and on errors).
 *
 * On horizontal mode, icon is placed at left, and on vertical mode,
 * placed at top.
 *
 * @see elm_slider_icon_set() for more details
 * @see elm_slider_icon_unset()
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_icon_get(const Evas_Object *obj);

/**
 * Set the end object of the slider object.
 *
 * @param obj The slider object.
 * @param end The end object.
 *
 * On horizontal mode, end is placed at left, and on vertical mode,
 * placed at bottom.
 *
 * @note Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_slider_end_unset() function.
 *
 * @warning If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_slider_end_set(Evas_Object *obj, Evas_Object *end);

/**
 * Unset an end object set on a given slider widget.
 *
 * @param obj The slider object.
 * @return The end object that was being used, if any was set, or
 * @c NULL, otherwise (and on errors).
 *
 * On horizontal mode, end is placed at left, and on vertical mode,
 * placed at bottom.
 *
 * This call will unparent and return the icon object which was set
 * for this widget, previously, on success.
 *
 * @see elm_slider_end_set() for more details.
 * @see elm_slider_end_get()
 *
 * @deprecated use elm_object_part_content_unset() instead
 * instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_end_unset(Evas_Object *obj);

/**
 * Retrieve the end object set for a given slider widget.
 *
 * @param obj The slider object.
 * @return The end object's handle, if @p obj had one set, or @c NULL,
 * otherwise (and on errors).
 *
 * On horizontal mode, icon is placed at right, and on vertical mode,
 * placed at bottom.
 *
 * @see elm_slider_end_set() for more details.
 * @see elm_slider_end_unset()
 *
 *
 * @deprecated use elm_object_part_content_get() instead
 * instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_end_get(const Evas_Object *obj);

/**
 * Return the data associated with a given slideshow item
 *
 * @param it The slideshow item
 * @return Returns the data associated to this item
 *
 * @deprecated use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void *elm_slideshow_item_data_get(const Elm_Object_Item *it);

/**
 * Delete a given item from a slideshow widget.
 *
 * @param it The slideshow item
 *
 * @deprecated Use elm_object_item_de() instead
 */
EINA_DEPRECATED EAPI void                  elm_slideshow_item_del(Elm_Object_Item *it);

/**
 * Display a given slideshow widget's item, programmatically.
 *
 * @param it The item to display on @p obj's viewport
 *
 * The change between the current item and @p item will use the
 * transition @p obj is set to use (@see
 * elm_slideshow_transition_set()).
 *
 * @deprecated use elm_slideshow_item_show() instead
 */
EINA_DEPRECATED EAPI void                  elm_slideshow_show(Elm_Object_Item *it);

/**
 * Get the toolbar object from an item.
 *
 * @param it The item.
 * @return The toolbar object.
 *
 * This returns the toolbar object itself that an item belongs to.
 *
 * @deprecated use elm_object_item_widget_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toolbar_item_toolbar_get(const Elm_Object_Item *it);

/**
 * Get the label of item.
 *
 * @param it The item of toolbar.
 * @return The label of item.
 *
 * The return value is a pointer to the label associated to @p item when
 * it was created, with function elm_toolbar_item_append() or similar,
 * or later,
 * with function elm_toolbar_item_label_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_toolbar_item_label_set() for more details.
 * @see elm_toolbar_item_append()
 *
 * @deprecated use elm_object_item_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_toolbar_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of item.
 *
 * @param it The item of toolbar.
 * @param label The label of item.
 *
 * The label to be displayed by the item.
 * Label will be placed at icons bottom (if set).
 *
 * If a label was passed as argument on item creation, with function
 * elm_toolbar_item_append() or similar, it will be already
 * displayed by the item.
 *
 * @see elm_toolbar_item_label_get()
 * @see elm_toolbar_item_append()
 *
 * @deprecated use elm_object_item_text_set() instead
 */
EINA_DEPRECATED EAPI void         elm_toolbar_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * Return the data associated with a given toolbar widget item.
 *
 * @param it The toolbar widget item handle.
 * @return The data associated with @p item.
 *
 * @see elm_toolbar_item_data_set()
 *
 * @deprecated use elm_object_item_data_get() instead.
 */
EINA_DEPRECATED EAPI void        *elm_toolbar_item_data_get(const Elm_Object_Item *it);

/**
 * Set the data associated with a given toolbar widget item.
 *
 * @param it The toolbar widget item handle
 * @param data The new data pointer to set to @p item.
 *
 * This sets new item data on @p item.
 *
 * @warning The old data pointer won't be touched by this function, so
 * the user had better to free that old data himself/herself.
 *
 * @deprecated use elm_object_item_data_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toolbar_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * Get a value whether toolbar item is disabled or not.
 *
 * @param it The item.
 * @return The disabled state.
 *
 * @see elm_toolbar_item_disabled_set() for more details.
 *
 * @deprecated use elm_object_item_disabled_get() instead.
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_toolbar_item_disabled_get(const Elm_Object_Item *it);

/**
 * Sets the disabled/enabled state of a toolbar item.
 *
 * @param it The item.
 * @param disabled The disabled state.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change its appearance (generally greyed out). This sets the
 * disabled state (@c EINA_TRUE for disabled, @c EINA_FALSE for
 * enabled).
 *
 * @deprecated use elm_object_item_disabled_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toolbar_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Change a toolbar's orientation
 * @param obj The toolbar object
 * @param vertical If @c EINA_TRUE, the toolbar is vertical
 * By default, a toolbar will be horizontal. Use this function to create a vertical toolbar.
 * @deprecated use elm_toolbar_horizontal_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toolbar_orientation_set(Evas_Object *obj, Eina_Bool vertical);

/**
 * Get a toolbar's orientation
 * @param obj The toolbar object
 * @return If @c EINA_TRUE, the toolbar is vertical
 * By default, a toolbar will be horizontal. Use this function to determine whether a toolbar is vertical.
 * @deprecated use elm_toolbar_horizontal_get() instead.
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_toolbar_orientation_get(const Evas_Object *obj);

/**
 * Set the function called when a toolbar item is freed.
 *
 * @param it The item to set the callback on.
 * @param func The function called.
 *
 * If there is a @p func, then it will be called prior item's memory release.
 * That will be called with the following arguments:
 * @li item's data;
 * @li item's Evas object;
 * @li item itself;
 *
 * This way, a data associated to a toolbar item could be properly freed.
 *
 * @deprecated Use elm_object_item_del_cb_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Delete them item from the toolbar.
 *
 * @param it The item of toolbar to be deleted.
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_toolbar_item_append()
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_del(Elm_Object_Item *it);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the toolbar item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @a func, the tooltip is unset with
 *        elm_toolbar_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @p event_info is the item.
 *
 * @deprecated Use elm_object_item_tooltip_content_cb_set() instead
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it toolbar item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_toolbar_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @deprecated Use elm_object_item_tooltip_unset() instead
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_toolbar_item_tooltip_content_cb_set()
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_toolbar_item_tooltip_content_cb_set() or
 *       elm_toolbar_item_tooltip_text_set()
 *
 * @param it toolbar item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @deprecated Use elm_object_item_tooltip_style_set() instead
 * @see elm_object_tooltip_style_set() for more details.
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param it toolbar item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @deprecated Use elm_object_item_style_get() instead
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_toolbar_item_tooltip_style_set()
 *
 */
EINA_DEPRECATED EAPI const char                  *elm_toolbar_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * Set the type of mouse pointer/cursor decoration to be shown,
 * when the mouse pointer is over the given toolbar widget item
 *
 * @param it toolbar item to customize cursor on
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
 * @see elm_toolbar_item_cursor_get()
 * @see elm_toolbar_item_cursor_unset()
 *
 * @deprecated use elm_object_item_cursor_set() instead
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/*
 * Get the type of mouse pointer/cursor decoration set to be shown,
 * when the mouse pointer is over the given toolbar widget item
 *
 * @param it toolbar item with custom cursor set
 * @return the cursor type's name or @c NULL, if no custom cursors
 * were set to @p item (and on errors)
 *
 * @see elm_object_cursor_get()
 * @see elm_toolbar_item_cursor_set()
 * @see elm_toolbar_item_cursor_unset()
 *
 * @deprecated Use elm_object_item_cursor_get() instead
 *
 */
EINA_DEPRECATED EAPI const char                  *elm_toolbar_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset any custom mouse pointer/cursor decoration set to be
 * shown, when the mouse pointer is over the given toolbar widget
 * item, thus making it show the @b default cursor again.
 *
 * @param it a toolbar item
 *
 * Use this call to undo any custom settings on this item's cursor
 * decoration, bringing it back to defaults (no custom style set).
 *
 * @see elm_object_cursor_unset()
 * @see elm_toolbar_item_cursor_set()
 *
 * @deprecated Use elm_object_item_cursor_unset() instead
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_cursor_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given custom cursor set for a
 * toolbar item.
 *
 * @param it toolbar item with custom cursor set
 * @param style the <b>theme style</b> to use (e.g. @c "default",
 * @c "transparent", etc)
 *
 * This function only makes sense when one is using custom mouse
 * cursor decorations <b>defined in a theme file</b>, which can have,
 * given a cursor name/type, <b>alternate styles</b> on it. It
 * works analogously as elm_object_cursor_style_set(), but here
 * applies only to toolbar item objects.
 *
 * @warning Before you set a cursor style you should have defined a
 *       custom cursor previously on the item, with
 *       elm_toolbar_item_cursor_set()
 *
 * @see elm_toolbar_item_cursor_engine_only_set()
 * @see elm_toolbar_item_cursor_style_get()
 *
 * @deprecated Use elm_object_item_cursor_style_set() instead
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the current @b style set for a given toolbar item's custom
 * cursor
 *
 * @param it toolbar item with custom cursor set.
 * @return style the cursor style in use. If the object does not
 *         have a cursor set, then @c NULL is returned.
 *
 * @see elm_toolbar_item_cursor_style_set() for more details
 *
 * @deprecated Use elm_object_item_cursor_style_get() instead
 *
 */
EINA_DEPRECATED EAPI const char                  *elm_toolbar_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Set if the (custom)cursor for a given toolbar item should be
 * searched in its theme, also, or should only rely on the
 * rendering engine.
 *
 * @param it item with custom (custom) cursor already set on
 * @param engine_only Use @c EINA_TRUE to have cursors looked for
 * only on those provided by the rendering engine, @c EINA_FALSE to
 * have them searched on the widget's theme, as well.
 *
 * @note This call is of use only if you've set a custom cursor
 * for toolbar items, with elm_toolbar_item_cursor_set().
 *
 * @note By default, cursors will only be looked for between those
 * provided by the rendering engine.
 *
 * @deprecated Use elm_object_item_cursor_engine_only_set() instead
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only);

/**
 * Get if the (custom) cursor for a given toolbar item is being
 * searched in its theme, also, or is only relying on the rendering
 * engine.
 *
 * @param it a toolbar item
 * @return @c EINA_TRUE, if cursors are being looked for only on
 * those provided by the rendering engine, @c EINA_FALSE if they
 * are being searched on the widget's theme, as well.
 *
 * @see elm_toolbar_item_cursor_engine_only_set(), for more details
 *
 * @deprecated Use elm_object_item_cursor_engine_only_get() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_toolbar_item_cursor_engine_only_get(const Elm_Object_Item *it);

/**
 * @brief Link a Elm_Payer with an Elm_Video object.
 *
 * @param player the Elm_Player object.
 * @param video The Elm_Video object.
 *
 * This mean that action on the player widget will affect the
 * video object and the state of the video will be reflected in
 * the player itself.
 *
 * @see elm_player_add()
 * @see elm_video_add()
 * @deprecated use elm_object_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void elm_player_video_set(Evas_Object *player, Evas_Object *video);

/**
 * Set the label of item.
 *
 * @param it The item of segment control.
 * @param label The label of item.
 *
 * The label to be displayed by the item.
 * Label will be at right of the icon (if set).
 *
 * If a label was passed as argument on item creation, with function
 * elm_control_segment_item_add(), it will be already
 * displayed by the item.
 *
 * @see elm_segment_control_item_label_get()
 * @see elm_segment_control_item_add()
 * @deprecated Use elm_object_item_text_set() instead
 *
 */
EINA_DEPRECATED EAPI void              elm_segment_control_item_label_set(Elm_Object_Item *it, const char *label);


/**
 * Set the icon associated to the item.
 *
 * @param it The segment control item.
 * @param icon The icon object to associate with @p it.
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
 * elm_segment_control_item_add(), it will be already
 * associated to the item.
 *
 * @see elm_segment_control_item_add()
 * @see elm_segment_control_item_icon_get()
 * @deprecated Use elm_object_item_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void              elm_segment_control_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * Remove a segment control item from its parent, deleting it.
 *
 * @param it The item to be removed.
 *
 * Items can be added with elm_segment_control_item_add() or
 * elm_segment_control_item_insert_at().
 *
 * @deprecated Use elm_object_item_del() instead
 */
EINA_DEPRECATED EAPI void              elm_segment_control_item_del(Elm_Object_Item *it);

/**
 * Get the label
 *
 * @param obj The multibuttonentry object
 * @return The label, or NULL if none
 *
 * @deprecated Use elm_object_text_get() instead
 *
 */
EINA_DEPRECATED EAPI const char                *elm_multibuttonentry_label_get(const Evas_Object *obj);

/**
 * Set the label
 *
 * @param obj The multibuttonentry object
 * @param label The text label string
 *
 * @deprecated Use elm_object_text_set() instead
 *
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of a given item
 *
 * @param it The item
 * @return The label of a given item, or NULL if none
 *
 * @deprecated Use elm_object_item_text_get() instead
 *
 */
EINA_DEPRECATED EAPI const char                *elm_multibuttonentry_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of a given item
 *
 * @param it The item
 * @param str The text label string
 *
 * @deprecated Use elm_object_item_text_set() instead
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_item_label_set(Elm_Object_Item *it, const char *str);

/**
 * Delete a given item
 *
 * @param it The item
 *
 * @deprecated Use elm_object_item_del() instead
 *
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_item_del(Elm_Object_Item *it);

/**
 * @brief Delete the given item instantly.
 *
 * @param it The naviframe item
 *
 * This just deletes the given item from the naviframe item list instantly.
 * So this would not emit any signals for view transitions but just change
 * the current view if the given item is a top one.
 *
 * @deprecated Use elm_object_item_del() instead
 */
EINA_DEPRECATED EAPI void             elm_naviframe_item_del(Elm_Object_Item *it);



/**
 * Sets the disabled/enabled state of a list item.
 *
 * @param it The item.
 * @param disabled The disabled state.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change its appearance (generally greyed out). This sets the
 * disabled state (@c EINA_TRUE for disabled, @c EINA_FALSE for
 * enabled).
 *
 * @deprecated Use elm_object_item_disabled_set() instead
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Get a value whether list item is disabled or not.
 *
 * @param it The item.
 * @return The disabled state.
 *
 * @see elm_list_item_disabled_set() for more details.
 *
 * @deprecated Use elm_object_item_disabled_get() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_disabled_get(const Elm_Object_Item *it);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 *
 * @deprecated Use elm_object_item_tooltip_window_mode_set() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param obj The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parent window's canvas.
 * It will instead be limited only by the size of the display.
 *
 * @deprecated Use elm_object_item_tooltip_window_mode_get() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_tooltip_window_mode_get(const Elm_Object_Item *it);

/**
 * Set the function called when a list item is freed.
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
 * This way, a data associated to a list item could be properly freed.
 *
 * @deprecated Please use elm_object_item_del_cb_set() instead.
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Get the data associated to the item.
 *
 * @param it The list item
 * @return The data associated to @p item
 *
 * The return value is a pointer to data associated to @p item when it was
 * created, with function elm_list_item_append() or similar. If no data
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_append()
 *
 * @deprecated Please use elm_object_item_data_get() instead.
 *
 */
EINA_DEPRECATED EAPI void                        *elm_list_item_data_get(const Elm_Object_Item *it);

/**
 * Get the left side icon associated to the item.
 *
 * @param it The list item
 * @return The left side icon associated to @p item
 *
 * The return value is a pointer to the icon associated to @p item when
 * it was
 * created, with function elm_list_item_append() or similar, or later
 * with function elm_list_item_icon_set(). If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_icon_set()
 *
 * @deprecated Please use elm_object_item_part_content_get(item, NULL);
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_list_item_icon_get(const Elm_Object_Item *it);

/**
 * Set the left side icon associated to the item.
 *
 * @param it The list item
 * @param icon The left side icon object to associate with @p item
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
 * elm_list_item_append() or similar, it will be already
 * associated to the item.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_icon_get()
 *
 * @deprecated Please use elm_object_item_part_content_set(item, NULL, icon);
 */
EINA_DEPRECATED EAPI void                         elm_list_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * Get the right side icon associated to the item.
 *
 * @param it The list item
 * @return The right side icon associated to @p item
 *
 * The return value is a pointer to the icon associated to @p item when
 * it was
 * created, with function elm_list_item_append() or similar, or later
 * with function elm_list_item_icon_set(). If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_icon_set()
 *
 * @deprecated Please use elm_object_item_part_content_get(item, "end");
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_list_item_end_get(const Elm_Object_Item *it);

/**
 * Set the right side icon associated to the item.
 *
 * @param it The list item
 * @param end The right side icon object to associate with @p item
 *
 * The icon object to use at right side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * disappear from the first item.
 *
 * If an icon was passed as argument on item creation, with function
 * elm_list_item_append() or similar, it will be already
 * associated to the item.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_end_get()
 *
 * @deprecated Please use elm_object_item_part_content_set(item, "end", end);
 */
EINA_DEPRECATED EAPI void                         elm_list_item_end_set(Elm_Object_Item *it, Evas_Object *end);

/**
 * Get the label of item.
 *
 * @param it The item of list.
 * @return The label of item.
 *
 * The return value is a pointer to the label associated to @p item when
 * it was created, with function elm_list_item_append(), or later
 * with function elm_list_item_label_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_label_set() for more details.
 * @see elm_list_item_append()
 *
 * @deprecated Please use elm_object_item_text_get(item);
 */
EINA_DEPRECATED EAPI const char                  *elm_list_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of item.
 *
 * @param it The item of list.
 * @param text The label of item.
 *
 * The label to be displayed by the item.
 * Label will be placed between left and right side icons (if set).
 *
 * If a label was passed as argument on item creation, with function
 * elm_list_item_append() or similar, it will be already
 * displayed by the item.
 *
 * @see elm_list_item_label_get()
 * @see elm_list_item_append()
 *
 * @deprecated Please use elm_object_item_text_set(item, text);
 */
EINA_DEPRECATED EAPI void                         elm_list_item_label_set(Elm_Object_Item *it, const char *text);

/**
 * Set the text to be shown in a given list item's tooltips.
 *
 * @param it Target item.
 * @param text The text to set in the content.
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data - set with this function or
 * elm_list_item_tooltip_content_cb_set() - is removed.
 *
 * @deprecated Use elm_object_item_tooltip_text_set() instead
 * @see elm_object_tooltip_text_set() for more details.
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the list item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @a func, the tooltip is unset with
 *        elm_list_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @p event_info is the item.
 *
 * @deprecated Use elm_object_item_tooltip_content_cb_set() instead
 *
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it list item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_list_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @deprecated Use elm_object_item_tooltip_unset() instead
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_list_item_tooltip_content_cb_set()
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_list_item_tooltip_content_cb_set() or
 *       elm_list_item_tooltip_text_set()
 *
 * @param it list item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 *
 * @deprecated Use elm_object_item_tooltip_style_set() instead
 * @see elm_object_tooltip_style_set() for more details.
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param it list item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @deprecated Use elm_object_item_tooltip_style_get() instead
 *
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_list_item_tooltip_style_set()
 *
 */
EINA_DEPRECATED EAPI const char                  *elm_list_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * Set the type of mouse pointer/cursor decoration to be shown,
 * when the mouse pointer is over the given list widget item
 *
 * @param it list item to customize cursor on
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
 * @see elm_list_item_cursor_get()
 * @see elm_list_item_cursor_unset()
 *
 * @deprecated Please use elm_object_item_cursor_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_list_item_cursor_set(Elm_Object_Item *it, const char *cursor);

/*
 * Get the type of mouse pointer/cursor decoration set to be shown,
 * when the mouse pointer is over the given list widget item
 *
 * @param it list item with custom cursor set
 * @return the cursor type's name or @c NULL, if no custom cursors
 * were set to @p item (and on errors)
 *
 * @see elm_object_cursor_get()
 * @see elm_list_item_cursor_set()
 * @see elm_list_item_cursor_unset()
 *
 * @deprecated Please use elm_object_item_cursor_get() instead
 */
EINA_DEPRECATED EAPI const char                  *elm_list_item_cursor_get(const Elm_Object_Item *it);

/**
 * Unset any custom mouse pointer/cursor decoration set to be
 * shown, when the mouse pointer is over the given list widget
 * item, thus making it show the @b default cursor again.
 *
 * @param it a list item
 *
 * Use this call to undo any custom settings on this item's cursor
 * decoration, bringing it back to defaults (no custom style set).
 *
 * @see elm_object_cursor_unset()
 * @see elm_list_item_cursor_set()
 *
 * @deprecated Please use elm_list_item_cursor_unset() instead
 */
EINA_DEPRECATED EAPI void                         elm_list_item_cursor_unset(Elm_Object_Item *it);

/**
 * Set a different @b style for a given custom cursor set for a
 * list item.
 *
 * @param it list item with custom cursor set
 * @param style the <b>theme style</b> to use (e.g. @c "default",
 * @c "transparent", etc)
 *
 * This function only makes sense when one is using custom mouse
 * cursor decorations <b>defined in a theme file</b>, which can have,
 * given a cursor name/type, <b>alternate styles</b> on it. It
 * works analogously as elm_object_cursor_style_set(), but here
 * applies only to list item objects.
 *
 * @warning Before you set a cursor style you should have defined a
 *       custom cursor previously on the item, with
 *       elm_list_item_cursor_set()
 *
 * @see elm_list_item_cursor_engine_only_set()
 * @see elm_list_item_cursor_style_get()
 *
 * @deprecated Please use elm_list_item_cursor_style_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_list_item_cursor_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the current @b style set for a given list item's custom
 * cursor
 *
 * @param it list item with custom cursor set.
 * @return style the cursor style in use. If the object does not
 *         have a cursor set, then @c NULL is returned.
 *
 * @see elm_list_item_cursor_style_set() for more details
 *
 * @deprecated Please use elm_list_item_cursor_style_get() instead
 */
EINA_DEPRECATED EAPI const char                  *elm_list_item_cursor_style_get(const Elm_Object_Item *it);

/**
 * Get if the (custom) cursor for a given list item is being
 * searched in its theme, also, or is only relying on the rendering
 * engine.
 *
 * @param it a list item
 * @return @c EINA_TRUE, if cursors are being looked for only on
 * those provided by the rendering engine, @c EINA_FALSE if they
 * are being searched on the widget's theme, as well.
 *
 * @see elm_list_item_cursor_engine_only_set(), for more details
 *
 * @deprecated Please use elm_list_item_cursor_engine_only_get() instead
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_cursor_engine_only_get(const Elm_Object_Item *it);

/**
 * Delete the item from the list.
 *
 * @param it The item of list to be deleted.
 *
 * If deleting all list items is required, elm_list_clear()
 * should be used instead of getting items list and deleting each one.
 *
 * @see elm_list_clear()
 * @see elm_list_item_append()
 * @see elm_widget_item_del_cb_set()
 * @deprecated Use elm_object_item_del() instead
 *
 */
EINA_DEPRECATED EAPI void                         elm_list_item_del(Elm_Object_Item *it);


/**
 * This sets a widget to be displayed to the left of a scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param icon The widget to display on the left side of the scrolled
 * entry.
 *
 * @note A previously set widget will be destroyed.
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated Use elm_object_part_content_set(entry, "icon", content) instead
 * @see elm_entry_end_set()
 */
EINA_DEPRECATED EAPI void               elm_entry_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the leftmost widget of the scrolled entry. This object is
 * owned by the scrolled entry and should not be modified.
 *
 * @param obj The scrolled entry object
 * @return the left widget inside the scroller
 *
 * @deprecated Use elm_object_part_content_get(entry, "icon") instead
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_icon_get(const Evas_Object *obj);

/**
 * Unset the leftmost widget of the scrolled entry, unparenting and
 * returning it.
 *
 * @param obj The scrolled entry object
 * @return the previously set icon sub-object of this entry, on
 * success.
 *
 * @deprecated Use elm_object_part_content_unset(entry, "icon") instead
 * @see elm_entry_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_icon_unset(Evas_Object *obj);

/**
 * This sets a widget to be displayed to the end of a scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param end The widget to display on the right side of the scrolled
 * entry.
 *
 * @note A previously set widget will be destroyed.
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated Use elm_object_part_content_set(entry, "end", content) instead
 * @see elm_entry_icon_set
 */
EINA_DEPRECATED EAPI void               elm_entry_end_set(Evas_Object *obj, Evas_Object *end);

/**
 * Get the endmost widget of the scrolled entry. This object is owned
 * by the scrolled entry and should not be modified.
 *
 * @param obj The scrolled entry object
 * @return the right widget inside the scroller
 *
 * @deprecated Use elm_object_part_content_get(entry, "end") instead
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_end_get(const Evas_Object *obj);

/**
 * Unset the endmost widget of the scrolled entry, unparenting and
 * returning it.
 *
 * @param obj The scrolled entry object
 * @return the previously set icon sub-object of this entry, on
 * success.
 *
 * @deprecated Use elm_object_part_content_unset(entry, "end") instead
 * @see elm_entry_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_end_unset(Evas_Object *obj);

/**
 * Set route service to be used. By default used source is
 * #ELM_MAP_ROUTE_SOURCE_YOURS.
 *
 * @see elm_map_route_source_set()
 * @see elm_map_route_source_get()
 *
 */
typedef enum
{
   ELM_MAP_ROUTE_SOURCE_YOURS, /**< Routing service http://www.yournavigation.org/ . Set by default.*/
   ELM_MAP_ROUTE_SOURCE_MONAV, /**< MoNav offers exact routing without heuristic assumptions. Its routing core is based on Contraction Hierarchies. It's not working with Map yet. */
   ELM_MAP_ROUTE_SOURCE_ORS, /**< Open Route Service: http://www.openrouteservice.org/ . It's not working with Map yet. */
   ELM_MAP_ROUTE_SOURCE_LAST
} Elm_Map_Route_Sources;

/**
 * Convert a pixel coordinate into a rotated pixel coordinate.
 *
 * @param obj The map object.
 * @param x horizontal coordinate of the point to rotate.
 * @param y vertical coordinate of the point to rotate.
 * @param cx rotation's center horizontal position.
 * @param cy rotation's center vertical position.
 * @param degree amount of degrees from 0.0 to 360.0 to rotate around Z axis.
 * @param xx Pointer where to store rotated x.
 * @param yy Pointer where to store rotated y.
 *
 * @deprecated Use elm_map_canvas_to_geo_convert() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_utils_rotate_coord(const Evas_Object *obj, const Evas_Coord x, const Evas_Coord y, const Evas_Coord cx, const Evas_Coord cy, const double degree, Evas_Coord *xx, Evas_Coord *yy);

/**
 * Convert canvas coordinates into a geographic coordinate
 * (longitude, latitude).
 *
 * @param obj The map object.
 * @param x   horizontal coordinate of the point to convert.
 * @param y   vertical coordinate of the point to convert.
 * @param lon A pointer to the longitude.
 * @param lat A pointer to the latitude.
 *
 * This gets longitude and latitude from canvas x, y coordinates. The canvas
 * coordinates mean x, y coordinate from current viewport.
 *
 * see elm_map_rotate_get()
 * @deprecatedUse Use elm_map_canvas_to_region_convert() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_canvas_to_geo_convert(const Evas_Object *obj, const Evas_Coord x, const Evas_Coord y, double *lon, double *lat);

/**
 * Get the current geographic coordinates of the map.
 *
 * @param obj The map object.
 * @param lon Pointer to store longitude.
 * @param lat Pointer to store latitude.
 *
 * This gets the current center coordinates of the map object. It can be
 * set by elm_map_region_bring_in() and elm_map_region_show().
 *
 * @see elm_map_region_bring_in()
 * @see elm_map_region_show()
 *
 * @deprecated Use elm_map_region_get() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_geo_region_get(const Evas_Object *obj, double *lon, double *lat);

/**
 * Animatedly bring in given coordinates to the center of the map.
 *
 * @param obj The map object.
 * @param lon Longitude to center at.
 * @param lat Latitude to center at.
 *
 * This causes map to jump to the given @p lat and @p lon coordinates
 * and show it (by scrolling) in the center of the viewport, if it is not
 * already centered. This will use animation to do so and take a period
 * of time to complete.
 *
 * @see elm_map_region_show() for a function to avoid animation.
 * @see elm_map_region_get()
 *
 * @deprecated Use elm_map_region_bring_in() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_geo_region_bring_in(Evas_Object *obj, double lon, double lat);

/**
 * Show the given coordinates at the center of the map, @b immediately.
 *
 * @param obj The map object.
 * @param lon Longitude to center at.
 * @param lat Latitude to center at.
 *
 * This causes map to @b redraw its viewport's contents to the
 * region containing the given @p lat and @p lon, that will be moved to the
 * center of the map.
 *
 * @see elm_map_region_bring_in() for a function to move with animation.
 * @see elm_map_region_get()
 *
 * @deprecated Use elm_map_region_show() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_geo_region_show(Evas_Object *obj, double lon, double lat);

/**
 * Set the minimum zoom of the source.
 *
 * @param obj The map object.
 * @param zoom New minimum zoom value to be used.
 *
 * By default, it's 0.
 *
 * @deprecated Use elm_map_zoom_min_set() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_source_zoom_min_set(Evas_Object *obj, int zoom);

/**
 * Get the minimum zoom of the source.
 *
 * @param obj The map object.
 * @return Returns the minimum zoom of the source.
 *
 * @see elm_map_zoom_min_set() for details.
 *
 * @deprecated Use elm_map_zoom_min_get() instead
 */
EINA_DEPRECATED EAPI int                   elm_map_source_zoom_min_get(const Evas_Object *obj);

/**
 * Set the maximum zoom of the source.
 *
 * @param obj The map object.
 * @param zoom New maximum zoom value to be used.
 *
 * By default, it's 18.
 *
 * @deprecated Use elm_map_zoom_max_set() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_source_zoom_max_set(Evas_Object *obj, int zoom);

/**
 * Get the maximum zoom of the source.
 *
 * @param obj The map object.
 * @return Returns the maximum zoom of the source.
 *
 * @see elm_map_zoom_min_set() for details.
 *
 * @deprecated Use elm_map_zoom_max_get() instead
 */
EINA_DEPRECATED EAPI int                   elm_map_source_zoom_max_get(const Evas_Object *obj);


/**
 * Get the list of available sources.
 *
 * @param obj The map object.
 * @return The source names list.
 *
 * It will provide a list with all available sources, that can be set as
 * current source with elm_map_source_name_set(), or get with
 * elm_map_source_name_get().
 *
 * Available sources:
 * @li "Mapnik"
 * @li "Osmarender"
 * @li "CycleMap"
 * @li "Maplint"
 *
 * @see elm_map_source_name_set() for more details.
 * @see elm_map_source_name_get()
 * @deprecated Use elm_map_sources_get() instead
 *
 */
EINA_DEPRECATED EAPI const char          **elm_map_source_names_get(const Evas_Object *obj);

/**
 * Set the source of the map.
 *
 * @param obj The map object.
 * @param source_name The source to be used.
 *
 * Map widget retrieves images that composes the map from a web service.
 * This web service can be set with this method.
 *
 * A different service can return a different maps with different
 * information and it can use different zoom values.
 *
 * The @p source_name need to match one of the names provided by
 * elm_map_source_names_get().
 *
 * The current source can be get using elm_map_source_name_get().
 *
 * @see elm_map_source_names_get()
 * @see elm_map_source_name_get()
 * @deprecated Use elm_map_source_set() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_source_name_set(Evas_Object *obj, const char *source_name);

/**
 * Get the name of currently used source.
 *
 * @param obj The map object.
 * @return Returns the name of the source in use.
 *
 * @see elm_map_source_name_set() for more details.
 * @deprecated Use elm_map_source_get() instead
 *
 */
EINA_DEPRECATED EAPI const char           *elm_map_source_name_get(const Evas_Object *obj);

/**
 * Set the source of the route service to be used by the map.
 *
 * @param obj The map object.
 * @param source The route service to be used, being it one of
 * #ELM_MAP_ROUTE_SOURCE_YOURS (default), #ELM_MAP_ROUTE_SOURCE_MONAV,
 * and #ELM_MAP_ROUTE_SOURCE_ORS.
 *
 * Each one has its own algorithm, so the route retrieved may
 * differ depending on the source route. Now, only the default is working.
 *
 * #ELM_MAP_ROUTE_SOURCE_YOURS is the routing service provided at
 * http://www.yournavigation.org/.
 *
 * #ELM_MAP_ROUTE_SOURCE_MONAV, offers exact routing without heuristic
 * assumptions. Its routing core is based on Contraction Hierarchies.
 *
 * #ELM_MAP_ROUTE_SOURCE_ORS, is provided at http://www.openrouteservice.org/
 *
 * @see elm_map_route_source_get().
 * @deprecated Use elm_map_source_set() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_route_source_set(Evas_Object *obj, Elm_Map_Route_Sources source);

/**
 * Get the current route source.
 *
 * @param obj The map object.
 * @return The source of the route service used by the map.
 *
 * @see elm_map_route_source_set() for details.
 * @deprecated Use elm_map_source_get() instead
 *
 */
EINA_DEPRECATED EAPI Elm_Map_Route_Sources elm_map_route_source_get(const Evas_Object *obj);

/**
 * Set the maximum numbers of markers' content to be displayed in a group.
 *
 * @param obj The map object.
 * @param max The maximum numbers of items displayed in a bubble.
 *
 * A bubble will be displayed when the user clicks over the group,
 * and will place the content of markers that belong to this group
 * inside it.
 *
 * A group can have a long list of markers, consequently the creation
 * of the content of the bubble can be very slow.
 *
 * In order to avoid this, a maximum number of items is displayed
 * in a bubble.
 *
 * By default this number is 30.
 *
 * Marker with the same group class are grouped if they are close.
 *
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_max_marker_per_group_set(Evas_Object *obj, int max);

/**
 * Set to show markers during zoom level changes or not.
 *
 * @param obj The map object.
 * @param paused Use @c EINA_TRUE to @b not show markers or @c EINA_FALSE
 * to show them.
 *
 * This sets the paused state to on (@c EINA_TRUE) or off (@c EINA_FALSE)
 * for map.
 *
 * The default is off.
 *
 * This will stop zooming using animation, changing zoom levels will
 * change instantly. This will stop any existing animations that are running.
 *
 * This sets the paused state to on (@c EINA_TRUE) or off (@c EINA_FALSE)
 * for the markers.
 *
 * The default  is off.
 *
 * Enabling it will force the map to stop displaying the markers during
 * zoom level changes. Set to on if you have a large number of markers.
 *
 * @see elm_map_paused_markers_get()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_paused_markers_set(Evas_Object *obj, Eina_Bool paused);

/**
 * Get a value whether markers will be displayed on zoom level changes or not
 *
 * @param obj The map object.
 * @return @c EINA_TRUE means map @b won't display markers or @c EINA_FALSE
 * indicates it will.
 *
 * This gets the current markers paused state for the map object.
 *
 * @see elm_map_paused_markers_set() for details.
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_map_paused_markers_get(const Evas_Object *obj);

/**
 * Close all the bubbles opened by the user.
 *
 * @param obj The map object.
 *
 * A bubble is displayed with a content fetched with #Elm_Map_Marker_Get_Func
 * when the user clicks on a marker.
 *
 * This functions is set for the marker class with
 * elm_map_marker_class_get_cb_set().
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_bubbles_close(Evas_Object *obj);

/**
 * Set the marker's style of a group class.
 *
 * @param clas The group class.
 * @param style The style to be used by markers.
 *
 * Each marker must be associated to a group class, and will use the style
 * defined by such class when grouped to other markers.
 *
 * The following styles are provided by default theme:
 * @li @c radio - blue circle
 * @li @c radio2 - green circle
 * @li @c empty
 *
 * @see elm_map_group_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_group_class_style_set(Elm_Map_Group_Class *clas, const char *style);

/**
 * Set the icon callback function of a group class.
 *
 * @param clas The group class.
 * @param icon_get The callback function that will return the icon.
 *
 * Each marker must be associated to a group class, and it can display a
 * custom icon. The function @p icon_get must return this icon.
 *
 * @see elm_map_group_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_group_class_icon_cb_set(Elm_Map_Group_Class *clas, Elm_Map_Group_Icon_Get_Func icon_get);

/**
 * Set the data associated to the group class.
 *
 * @param clas The group class.
 * @param data The new user data.
 *
 * This data will be passed for callback functions, like icon get callback,
 * that can be set with elm_map_group_class_icon_cb_set().
 *
 * If a data was previously set, the object will lose the pointer for it,
 * so if needs to be freed, you must do it yourself.
 *
 * @see elm_map_group_class_new() for more details.
 * @see elm_map_group_class_icon_cb_set()
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_group_class_data_set(Elm_Map_Group_Class *clas, void *data);

/**
 * Set the minimum zoom from where the markers are displayed.
 *
 * @param clas The group class.
 * @param zoom The minimum zoom.
 *
 * Markers only will be displayed when the map is displayed at @p zoom
 * or bigger.
 *
 * @see elm_map_group_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_group_class_zoom_displayed_set(Elm_Map_Group_Class *clas, int zoom);

/**
 * Set the zoom from where the markers are no more grouped.
 *
 * @param clas The group class.
 * @param zoom The maximum zoom.
 *
 * Markers only will be grouped when the map is displayed at
 * less than @p zoom.
 *
 * @see elm_map_group_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_group_class_zoom_grouped_set(Elm_Map_Group_Class *clas, int zoom);

/**
 * Set if the markers associated to the group class @p clas are hidden or not.
 *
 * @param clas The group class.
 * @param hide Use @c EINA_TRUE to hide markers or @c EINA_FALSE
 * to show them.
 *
 * @param obj The map object.
 * If @p hide is @c EINA_TRUE the markers will be hidden, but default
 * is to show them.
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_group_class_hide_set(Evas_Object *obj, Elm_Map_Group_Class *clas, Eina_Bool hide);

/**
 * Set the marker's style of a marker class.
 *
 * @param clas The marker class.
 * @param style The style to be used by markers.
 *
 * Each marker must be associated to a marker class, and will use the style
 * defined by such class when alone, i.e., @b not grouped to other markers.
 *
 * The following styles are provided by default theme:
 * @li @c radio
 * @li @c radio2
 * @li @c empty
 *
 * @see elm_map_marker_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_class_style_set(Elm_Map_Marker_Class *clas, const char *style);

/**
 * Set the icon callback function of a marker class.
 *
 * @param clas The marker class.
 * @param icon_get The callback function that will return the icon.
 *
 * Each marker must be associated to a marker class, and it can display a
 * custom icon. The function @p icon_get must return this icon.
 *
 * @see elm_map_marker_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_class_icon_cb_set(Elm_Map_Marker_Class *clas, Elm_Map_Marker_Icon_Get_Func icon_get);

/**
 * Set the bubble content callback function of a marker class.
 *
 * @param clas The marker class.
 * @param get The callback function that will return the content.
 *
 * Each marker must be associated to a marker class, and it can display a
 * a content on a bubble that opens when the user click over the marker.
 * The function @p get must return this content object.
 *
 * If this content will need to be deleted, elm_map_marker_class_del_cb_set()
 * can be used.
 *
 * @see elm_map_marker_class_new() for more details.
 * @see elm_map_marker_class_del_cb_set()
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_class_get_cb_set(Elm_Map_Marker_Class *clas, Elm_Map_Marker_Get_Func get);

/**
 * Set the callback function used to delete bubble content of a marker class.
 *
 * @param clas The marker class.
 * @param del The callback function that will delete the content.
 *
 * Each marker must be associated to a marker class, and it can display a
 * a content on a bubble that opens when the user click over the marker.
 * The function to return such content can be set with
 * elm_map_marker_class_get_cb_set().
 *
 * If this content must be freed, a callback function need to be
 * set for that task with this function.
 *
 * If this callback is defined it will have to delete (or not) the
 * object inside, but if the callback is not defined the object will be
 * destroyed with evas_object_del().
 *
 * @see elm_map_marker_class_new() for more details.
 * @see elm_map_marker_class_get_cb_set()
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_class_del_cb_set(Elm_Map_Marker_Class *clas, Elm_Map_Marker_Del_Func del);

/**
 * Set the route color.
 *
 * @param route The route object.
 * @param r Red channel value, from 0 to 255.
 * @param g Green channel value, from 0 to 255.
 * @param b Blue channel value, from 0 to 255.
 * @param a Alpha channel value, from 0 to 255.
 *
 * It uses an additive color model, so each color channel represents
 * how much of each primary colors must to be used. 0 represents
 * absence of this color, so if all of the three are set to 0,
 * the color will be black.
 *
 * These component values should be integers in the range 0 to 255,
 * (single 8-bit byte).
 *
 * This sets the color used for the route. By default, it is set to
 * solid red (r = 255, g = 0, b = 0, a = 255).
 *
 * For alpha channel, 0 represents completely transparent, and 255, opaque.
 *
 * @see elm_map_route_color_get()
 * @deprecated Use elm_map_overlay_color_set() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_route_color_set(Elm_Map_Route *route, int r, int g, int b, int a);

/**
 * Get the route color.
 *
 * @param route The route object.
 * @param r Pointer to store the red channel value.
 * @param g Pointer to store the green channel value.
 * @param b Pointer to store the blue channel value.
 * @param a Pointer to store the alpha channel value.
 *
 * @see elm_map_route_color_set() for details.
 * @deprecated Use elm_map_overlay_color_get() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_route_color_get(const Elm_Map_Route *route, int *r, int *g, int *b, int *a);

/**
 * Remove a name from the map.
 *
 * @param name The name to remove.
 *
 * Basically the struct handled by @p name will be freed, so conversions
 * between address and coordinates will be lost.
 *
 * @see elm_map_utils_convert_name_into_coord()
 * @see elm_map_utils_convert_coord_into_name()
 * @deprecated Use elm_map_name_del() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_name_remove(Elm_Map_Name *name);

/**
 * Get the gengrid object's handle which contains a given gengrid item
 *
 * @param it The item to fetch the container from
 * @return The gengrid (parent) object
 *
 * This returns the gengrid object itself that an item belongs to.
 *
 * @deprecated Use elm_object_item_widget_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object                  *elm_gengrid_item_gengrid_get(const Elm_Object_Item *it);

/**
 * Return the data associated to a given gengrid item
 *
 * @param it The gengrid item.
 * @return the data associated with this item.
 *
 * This returns the @c data value passed on the
 * elm_gengrid_item_append() and related item addition calls.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_data_set()
 * @deprecated Use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void                         *elm_gengrid_item_data_get(const Elm_Object_Item *it);

/**
 * Set the data associated with a given gengrid item
 *
 * @param it The gengrid item
 * @param data The data pointer to set on it
 *
 * This @b overrides the @c data value passed on the
 * elm_gengrid_item_append() and related item addition calls. This
 * function @b won't call elm_gengrid_item_update() automatically,
 * so you'd issue it afterwards if you want to have the item
 * updated to reflect the new data.
 *
 * @see elm_gengrid_item_data_get()
 * @see elm_gengrid_item_update()
 * @deprecated Use elm_object_item_data_set() instead
 *
 */
EINA_DEPRECATED EAPI void                          elm_gengrid_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * Set whether a given gengrid item is disabled or not.
 *
 * @param it The gengrid item
 * @param disabled Use @c EINA_TRUE, true disable it, @c EINA_FALSE
 * to enable it back.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change its appearance, to signal the user it's disabled.
 *
 * @see elm_gengrid_item_disabled_get()
 * @deprecated Use elm_object_item_disabled_set() instead
 *
 */
EINA_DEPRECATED EAPI void                          elm_gengrid_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Get whether a given gengrid item is disabled or not.
 *
 * @param it The gengrid item
 * @return @c EINA_TRUE, if it's disabled, @c EINA_FALSE otherwise
 * (and on errors).
 *
 * @see elm_gengrid_item_disabled_set() for more details
 * @deprecated Use elm_object_item_disabled_get() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_gengrid_item_disabled_get(const Elm_Object_Item *it);

/**
 * Remove a gengrid item from its parent, deleting it.
 *
 * @param it The item to be removed.
 * @return @c EINA_TRUE on success or @c EINA_FALSE, otherwise.
 *
 * @see elm_gengrid_clear(), to remove all items in a gengrid at
 * once.
 * @deprecated use elm_object_item_de() instead
 *
 */
EINA_DEPRECATED EAPI void                          elm_gengrid_item_del(Elm_Object_Item *it);

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
 * @deprecated Use elm_gengrid_item_item_class_update instead
 */
EINA_DEPRECATED EAPI void                          elm_gengrid_item_item_class_set(Elm_Object_Item *it, const Elm_Gengrid_Item_Class *gic);

/**
 * Insert an item in a gengrid widget using a user-defined sort function.
 *
 * @param obj The gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param comp User defined comparison function that defines the sort order
 *             based on Elm_Gen_Item.
 * @param func Convenience function called when the item is selected.
 * @param func_data Data to be passed to @p func.
 * @return A handle to the item added or @c NULL, on errors.
 *
 * This inserts an item in the gengrid based on user defined comparison function.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_after()
 * @see elm_object_item_del()
 * @see elm_gengrid_item_sorted_insert()
 *
 * @deprecated Use elm_gengrid_item_sorted_insert() instead
 */
EINA_DEPRECATED EAPI Elm_Object_Item             *elm_gengrid_item_direct_sorted_insert(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic, const void *data, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

/**
 * Append a filter function for text inserted in the entry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the entry, with the text to be inserted
 * as a parameter. The callback function is free to alter the text in any way
 * it wants, but it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set its
 * text parameter to NULL. This will also prevent any following filters from
 * being called.
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 * @deprecated use elm_entry_markup_filter_append() instead
 */
EINA_DEPRECATED EAPI void               elm_entry_text_filter_append(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Prepend a filter function for text inserted in the entry
 *
 * Prepend the given callback to the list. See elm_entry_text_filter_append()
 * for more information
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 * @deprecated use elm_entry_markup_filter_prepend() instead
 */
EINA_DEPRECATED EAPI void               elm_entry_text_filter_prepend(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Remove a filter from the list
 *
 * Removes the given callback from the filter list. See
 * elm_entry_text_filter_append() for more information.
 *
 * @param obj The entry object
 * @param func The filter function to remove
 * @param data The user data passed when adding the function
 * @deprecated use elm_entry_markup_filter_remove() instead
 */
EINA_DEPRECATED EAPI void               elm_entry_text_filter_remove(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * @brief Set the front content of the flip widget.
 *
 * @param obj The flip object
 * @param content The new front content object
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_flip_content_front_unset() function.
 *
 * @deprecated Use elm_object_part_content_set(flip, "front") instead
 */
EINA_DEPRECATED EAPI void                 elm_flip_content_front_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Set the back content of the flip widget.
 *
 * @param obj The flip object
 * @param content The new back content object
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_flip_content_back_unset() function.
 *
 * @deprecated Use elm_object_part_content_set(flip, "back") instead
 */
EINA_DEPRECATED EAPI void                 elm_flip_content_back_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Get the front content used for the flip
 *
 * @param obj The flip object
 * @return The front content object that is being used
 *
 * Return the front content object which is set for this widget.
 *
 * @deprecated Use elm_object_part_content_get(flip, "front") instead
 */
EINA_DEPRECATED EAPI Evas_Object         *elm_flip_content_front_get(const Evas_Object *obj);

/**
 * @brief Get the back content used for the flip
 *
 * @param obj The flip object
 * @return The back content object that is being used
 *
 * Return the back content object which is set for this widget.
 *
 * @deprecated Use elm_object_part_content_get(flip, "back") instead
 */
EINA_DEPRECATED EAPI Evas_Object         *elm_flip_content_back_get(const Evas_Object *obj);

/**
 * @brief Unset the front content used for the flip
 *
 * @param obj The flip object
 * @return The front content object that was being used
 *
 * Unparent and return the front content object which was set for this widget.
 *
 * @deprecated Use elm_object_part_content_unset(flip, "front") instead
 */
EINA_DEPRECATED EAPI Evas_Object         *elm_flip_content_front_unset(Evas_Object *obj);

/**
 * @brief Unset the back content used for the flip
 *
 * @param obj The flip object
 * @return The back content object that was being used
 *
 * Unparent and return the back content object which was set for this widget.
 *
 * @deprecated Use elm_object_part_content_unset(flip, "back") instead
 */
EINA_DEPRECATED EAPI Evas_Object         *elm_flip_content_back_unset(Evas_Object *obj);

/**
 * @brief Get flip front visibility state
 *
 * @param obj The flip object
 * @return EINA_TRUE if front front is showing, EINA_FALSE if the back is
 * showing.
 *
 * @deprecated Use elm_flip_front_visible_get() instead
 */
EINA_DEPRECATED EAPI Eina_Bool            elm_flip_front_get(const Evas_Object *obj);

/**
 * @brief Set the font size on the label object.
 *
 * @param obj The label object
 * @param size font size
 *
 * @warning NEVER use this. It is for hyper-special cases only. use styles
 * instead. e.g. "default", "marker", "slide_long" etc.
 * @deprecated Use <font_size> tag instead. eg) <font_size=40>abc</font_size>
 */
EINA_DEPRECATED EAPI void                        elm_label_fontsize_set(Evas_Object *obj, int fontsize);

/**
 * @brief Set the text color on the label object
 *
 * @param obj The label object
 * @param r Red property background color of The label object
 * @param g Green property background color of The label object
 * @param b Blue property background color of The label object
 * @param a Alpha property background color of The label object
 *
 * @warning NEVER use this. It is for hyper-special cases only. use styles
 * instead. e.g. "default", "marker", "slide_long" etc.
 * @deprecated Use <color> tag instead. about <color> tag - Text color in one of the following formats: "#RRGGBB", "#RRGGBBAA", "#RGB", and "#RGBA"
 */
EINA_DEPRECATED EAPI void                        elm_label_text_color_set(Evas_Object *obj, unsigned int r, unsigned int g, unsigned int b, unsigned int a);

/**
 * @brief Set the text align on the label object
 *
 * @param obj The label object
 * @param align align mode ("left", "center", "right")
 *
 * @warning NEVER use this. It is for hyper-special cases only. use styles
 * instead. e.g. "default", "marker", "slide_long" etc.
 * @deprecated Use <align> tag instead. about <align> tag - Either "auto" (meaning according to text direction), "left", "right", "center", "middle", a value between 0.0 and 1.0, or a value between 0% to 100%.
 */
EINA_DEPRECATED EAPI void                        elm_label_text_align_set(Evas_Object *obj, const char *alignmode);

/**
 * @brief Set background color of the label
 *
 * @param obj The label object
 * @param r Red property background color of The label object
 * @param g Green property background color of The label object
 * @param b Blue property background color of The label object
 * @param a Alpha property background alpha of The label object
 *
 * @warning NEVER use this. It is for hyper-special cases only. use styles
 * instead. e.g. "default", "marker", "slide_long" etc.
 * @deprecated Just make colored background by yourself.
 */
EINA_DEPRECATED EAPI void                        elm_label_background_color_set(Evas_Object *obj, unsigned int r, unsigned int g, unsigned int b, unsigned int a);

/**
 * @brief Set wrap height of the label
 *
 * @param obj The label object
 * @param h The wrap height in pixels at a minimum where words need to wrap
 *
 * This function sets the maximum height size hint of the label.
 *
 * @warning This is only relevant if the label is inside a container.
 * @deprecated This function should not be used because of wrong concept.
 */
EINA_DEPRECATED EAPI void                        elm_label_wrap_height_set(Evas_Object *obj, Evas_Coord h);

/**
 * @brief get wrap width of the label
 *
 * @param obj The label object
 * @return The wrap height in pixels at a minimum where words need to wrap
 * @deprecated This function should not be used because of wrong concept.
 */
EINA_DEPRECATED EAPI Evas_Coord                  elm_label_wrap_height_get(const Evas_Object *obj);

/**
 * @brief Flush all caches.
 *
 * Frees all data that was in cache and is not currently being used to reduce
 * memory usage. This frees Edje's, Evas' and Eet's cache. This is equivalent
 * to calling all of the following functions:
 * @li edje_file_cache_flush()
 * @li edje_collection_cache_flush()
 * @li eet_clearcache()
 * @li evas_image_cache_flush()
 * @li evas_font_cache_flush()
 * @li evas_render_dump()
 * @note Evas caches are flushed for every canvas associated with a window.
 * @deprecated Use elm_cache_all_flush() instead.
 */
EINA_DEPRECATED EAPI void      elm_all_flush(void);


/**
 * @brief Define the uri that will be the video source.
 *
 * @param video The video object to define the file for.
 * @param uri The uri to target.
 *
 * This function will define an uri as a source for the video of the
 * Elm_Video object. URI could be remote source of video, like http:// or local
 * like for example WebCam who are most of the time v4l2:// (but that depend an
 * you should use Emotion API to request and list the available Webcam on your
 *
 * @deprecated Use elm_video_file_set() instead.
 *
 */
EINA_DEPRECATED EAPI void                 elm_video_uri_set(Evas_Object *video, const char *uri);

/**
 * @brief Get the region of the image that is currently shown
 *
 * @param obj
 * @param x A pointer to the X-coordinate of region
 * @param y A pointer to the Y-coordinate of region
 * @param w A pointer to the width
 * @param h A pointer to the height
 *
 * @deprecated Use elm_photocam_image_region_get() instead.
 */
EINA_DEPRECATED EAPI void                   elm_photocam_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

/**
 * @brief Set the gesture state for photocam.
 *
 * @param obj The photocam object
 * @param gesture The gesture state to set
 *
 * This sets the gesture state to on(EINA_TRUE) or off (EINA_FALSE) for
 * photocam. The default is off. This will start multi touch zooming.
 *
 * @deprecated Use elm_photocam_gesture_enabled_set() instead.
 */
EINA_DEPRECATED EAPI void		    elm_photocam_gesture_set(Evas_Object *obj, Eina_Bool gesture);

/**
 * @brief Get the gesture state for photocam.
 *
 * @param obj The photocam object
 * @return The current gesture state
 *
 * This gets the current gesture state for the photocam object.
 *
 * @deprecated Use elm_photocam_gesture_enabled_get() instead.
 */
EINA_DEPRECATED EAPI Eina_Bool		    elm_photocam_gesture_get(const Evas_Object *obj);
/* No documentation for these API before.
 *
 * @deprecated Use elm_route_latitude_min_max_get()
 *             elm_route_longitude_min_max_get()
 *             instead.
 */
EINA_DEPRECATED EAPI double       elm_route_lon_min_get(Evas_Object *obj);
EINA_DEPRECATED EAPI double       elm_route_lat_min_get(Evas_Object *obj);
EINA_DEPRECATED EAPI double       elm_route_lon_max_get(Evas_Object *obj);
EINA_DEPRECATED EAPI double       elm_route_lat_max_get(Evas_Object *obj);


/**
 * Get the duration after which tooltip will be shown.
 *
 * @return Duration after which tooltip will be shown.
 * @deprecated Use elm_config_tooltip_delay_get(void);
 */
EINA_DEPRECATED EAPI double      elm_tooltip_delay_get(void);

/**
 * Set the duration after which tooltip will be shown.
 *
 * @return EINA_TRUE if value is set.
 * @deprecated Use elm_config_tooltip_delay_set(double delay);
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_tooltip_delay_set(double delay);

/**
 * Set the corner of the bubble
 *
 * @param obj The bubble object.
 * @param corner The given corner for the bubble.
 *
 * @deprecated Use elm_bubble_pos_set()
 *
 */
EINA_DEPRECATED EAPI void elm_bubble_corner_set(Evas_Object *obj, const char *corner);

/**
 * Get the corner of the bubble
 *
 * @param obj The bubble object.
 * @return The given corner for the bubble.
 *
 * @deprecated Use elm_bubble_pos_get()
 */
EINA_DEPRECATED EAPI const char *elm_bubble_corner_get(const Evas_Object *obj);

/**
 * Enable or disable day selection
 *
 * @param obj The calendar object.
 * @param enabled @c EINA_TRUE to enable selection or @c EINA_FALSE to
 * disable it.
 *
 * @deprecated Use elm_calendar_day_selection_disabled_set()
 */
EINA_DEPRECATED EAPI void                 elm_calendar_day_selection_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Get a value whether day selection is enabled or not.
 *
 * @param obj The calendar object.
 * @return EINA_TRUE means day selection is enabled. EINA_FALSE indicates
 * it's disabled. If @p obj is NULL, EINA_FALSE is returned.
 *
 * @deprecated elm_calendar_day_selection_disabled_get()
 */
EINA_DEPRECATED EAPI Eina_Bool            elm_calendar_day_selection_enabled_get(const Evas_Object *obj);

/**
 * @deprecated Use Elm_Calendar_Mark_Repeat_Type instead.
 */
typedef _Elm_Calendar_Mark_Repeat_Type Elm_Calendar_Mark_Repeat EINA_DEPRECATED;

/**
 * Get the configured font cache size
 *
 * This gets the globally configured font cache size, in bytes.
 *
 * @return The font cache size
 * @deprecated elm_cache_font_cache_size_get(void);
 */
EINA_DEPRECATED EAPI int       elm_font_cache_get(void);

/**
 * Set the configured font cache size
 *
 * This sets the globally configured font cache size, in bytes
 *
 * @param size The font cache size
 * @deprecated elm_cache_font_cache_size_set(int size);
 */
EINA_DEPRECATED EAPI void      elm_font_cache_set(int size);

/**
 * Get the configured image cache size
 *
 * This gets the globally configured image cache size, in bytes
 *
 * @return The image cache size
 */
EINA_DEPRECATED EAPI int       elm_image_cache_get(void);
EINA_DEPRECATED EAPI int       elm_cache_image_cache_size_get(void);

/**
 * Set the configured image cache size
 *
 * This sets the globally configured image cache size, in bytes
 *
 * @param size The image cache size
 * @deprecated Use elm_cache_image_cache_size_set(int size);
 */
EINA_DEPRECATED EAPI void      elm_image_cache_set(int size);


/**
 * Get the configured edje file cache size.
 *
 * This gets the globally configured edje file cache size, in number
 * of files.
 *
 * @return The edje file cache size
 * @deprecated Use elm_cache_edje_file_cache_size_get(void);
 */
EINA_DEPRECATED EAPI int       elm_edje_file_cache_get(void);

/**
 * Set the configured edje file cache size
 *
 * This sets the globally configured edje file cache size, in number
 * of files.
 *
 * @param size The edje file cache size
 * @deprecated Use elm_cache_edje_file_cache_size_get(int size);
 */
EINA_DEPRECATED EAPI void      elm_edje_file_cache_set(int size);

/**
 * Get the configured edje collections (groups) cache size.
 *
 * This gets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @return The edje collections cache size
 * @deprecated Use elm_cache_edje_collection_cache_size_get(void);
 */
EINA_DEPRECATED EAPI int       elm_edje_collection_cache_get(void);

/**
 * Set the configured edje collections (groups) cache size
 *
 * This sets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @param size The edje collections cache size
 * @deprecated elm_cache_edje_collection_cache_size_set(int size);
 */
EINA_DEPRECATED EAPI void      elm_edje_collection_cache_set(int size);

/**
 * Get whether browsing history is enabled for the given object
 *
 * @param obj The web object
 *
 * @return EINA_TRUE if history is enabled, EINA_FALSE otherwise
 *
 * @deprecated Use elm_web_history_enabled_get()
 */
EINA_DEPRECATED EAPI Eina_Bool         elm_web_history_enable_get(const Evas_Object *obj);

/**
 * Enables or disables the browsing history
 *
 * @param obj The web object
 * @param enable Whether to enable or disable the browsing history
 *
 * @deprecated Use elm_web_history_enabled_set()
 *
 */
EINA_DEPRECATED EAPI void              elm_web_history_enable_set(Evas_Object *obj, Eina_Bool enable);

/**
 * @brief Add an object swallowed in an item at the end of the given menu
 * widget
 *
 * @param obj The menu object.
 * @param parent The parent menu item (optional)
 * @param subobj The object to swallow
 * @param func Function called when the user select the item.
 * @param data Data sent by the callback.
 * @return Returns the new item.
 *
 * Add an evas object as an item to the menu.
 * @deprecated please use "elm_menu_item_add" + "elm_object_item_content_set" instead.
 */
EINA_DEPRECATED EAPI Elm_Object_Item             *elm_menu_item_add_object(Evas_Object *obj, Elm_Object_Item *parent, Evas_Object *subobj, Evas_Smart_Cb func, const void *data);

/**
 * @brief Sets whether events should be passed to by a click outside
 * its area.
 *
 * @param obj The notify object
 * @param repeat EINA_TRUE Events are repeats, else no
 *
 * When true if the user clicks outside the window the events will be caught
 * by the others widgets, else the events are blocked.
 *
 * @note The default value is EINA_TRUE.
 * @deprecated Please use elm_notify_allow_events_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_notify_repeat_events_set(Evas_Object *obj, Eina_Bool repeat);

/**
 * @brief Return true if events are repeat below the notify object
 * @param obj the notify object
 *
 * @see elm_notify_repeat_events_set()
 * @deprecated Please use elm_notify_allow_events_get() instead
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_notify_repeat_events_get(const Evas_Object *obj);

/**
 * Set if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @param obj an object with cursor already set.
 * @param engine_only boolean to define if cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well.
 *
 * @deprecated Use elm_object_cursor_theme_search_enabled_set()
 */
EINA_DEPRECATED EAPI void        elm_object_cursor_engine_only_set(Evas_Object *obj, Eina_Bool engine_only);

/**
 * Get the cursor engine only usage for this object cursor.
 *
 * @param obj an object with cursor already set.
 * @return engine_only boolean to define it cursors should be
 * looked only between the provided by the engine or searched on
 * widget's theme as well. If the object does not have a cursor
 * set, then EINA_FALSE is returned.
 *
 * @deprecated Use elm_object_cursor_theme_search_enabled_get();
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_object_cursor_engine_only_get(const Evas_Object *obj);

/**
 * Go to a given items level on a index widget
 *
 * @param obj The index object
 * @param level The index level (one of @c 0 or @c 1)
 *
 * @deprecated please use "elm_index_level_go" instead.
 */
EINA_DEPRECATED EAPI void                  elm_index_item_go(Evas_Object *obj, int level);

/**
 * Enable or disable auto hiding feature for a given index widget.
 *
 * @param obj The index object
 * @param active @c EINA_TRUE to enable auto hiding, @c EINA_FALSE to disable
 *
 * @see elm_index_active_get()
 *
 * @deprecated please use "elm_index_autohide_disabled_set" instead.
 */
EINA_DEPRECATED EAPI void                  elm_index_active_set(Evas_Object *obj, Eina_Bool active);

/**
 * Get whether auto hiding feature is enabled or not for a given index widget.
 *
 * @param obj The index object
 * @return @c EINA_TRUE, if auto hiding is enabled, @c EINA_FALSE otherwise
 *
 * @see elm_index_active_set() for more details
 *
 * @deprecated please use "elm_index_autohide_disabled_get" instead.
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_index_active_get(const Evas_Object *obj);

/**
 * Append a new item, on a given index widget, <b>after the item
 * having @p relative as data</b>.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
 * @param relative The index item to be the predecessor of this new one
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @note If @p relative is @c NULL this function will behave as
 * elm_index_item_append().
 *
 * @deprecated please use "elm_index_item_insert_after" instead.
 */
EINA_DEPRECATED EAPI Elm_Object_Item      *elm_index_item_append_relative(Evas_Object *obj, const char *letter, const void *item, const Elm_Object_Item *relative);

/**
 * Prepend a new item, on a given index widget, <b>after the item
 * having @p relative as data</b>.
 *
 * @param obj The index object.
 * @param letter Letter under which the item should be indexed
 * @param item The item data to set for the index's item
 * @param relative The index item to be the successor of this new one
 * @return A handle to the item added or @c NULL, on errors
 *
 * Despite the most common usage of the @p letter argument is for
 * single char strings, one could use arbitrary strings as index
 * entries.
 *
 * @c item will be the pointer returned back on @c "changed", @c
 * "delay,changed" and @c "selected" smart events.
 *
 * @note If @p relative is @c NULL this function will behave as
 * elm_index_item_prepend().
 *
 * @deprecated please use "elm_index_item_insert_before" instead.
 */
EINA_DEPRECATED EAPI Elm_Object_Item      *elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const Elm_Object_Item *relative);

/**
 * Set the transparency state of a window.
 *
 * Use elm_win_alpha_set() instead.
 *
 * @param obj The window object
 * @param transparent If true, the window is transparent
 *
 * @see elm_win_alpha_set()
 * @deprecated Please use elm_win_alpha_set()
 */
EINA_DEPRECATED EAPI void                  elm_win_transparent_set(Evas_Object *obj, Eina_Bool transparent);

/**
 * Get the transparency state of a window.
 *
 * @param obj The window object
 * @return If true, the window is transparent
 *
 * @see elm_win_transparent_set()
 * @deprecated Please use elm_win_alpha_get()
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_win_transparent_get(const Evas_Object *obj);

/**
 * Set the theme for all elementary using applications on the current display
 *
 * @param theme The name of the theme to use. Format same as the ELM_THEME
 * environment variable.
 * @deprecated Use elm_theme_set(NULL, theme); elm_config_all_flush(); instead.
 */
EINA_DEPRECATED EAPI void             elm_theme_all_set(const char *theme);

/**
 * Returns the Evas_Object that represents the content area.
 *
 * @param obj The conformant object.
 * @return The content area of the widget.
 *
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_conformant_content_area_get(const Evas_Object *obj);

/**
 * Set if the object is (up/down) resizable.
 *
 * @param obj The image object
 * @param scale_up A bool to set if the object is resizable up. Default is
 * @c EINA_TRUE.
 * @param scale_down A bool to set if the object is resizable down. Default
 * is @c EINA_TRUE.
 *
 * This function limits the image resize ability. If @p scale_up is set to
 * @c EINA_FALSE, the object can't have its height or width resized to a value
 * higher than the original image size. Same is valid for @p scale_down.
 *
 * @see elm_image_scale_get()
 * @deprecated Please use elm_image_resizable_set()
 *
 */
EINA_DEPRECATED EAPI void             elm_image_scale_set(Evas_Object *obj, Eina_Bool scale_up, Eina_Bool scale_down);

/**
 * Get if the object is (up/down) resizable.
 *
 * @param obj The image object
 * @param scale_up A bool to set if the object is resizable up
 * @param scale_down A bool to set if the object is resizable down
 *
 * @see elm_image_scale_set()
 * @deprecated Please use elm_image_resizable_get()
 *
 */
EINA_DEPRECATED EAPI void             elm_image_scale_get(const Evas_Object *obj, Eina_Bool *scale_up, Eina_Bool *scale_down);

/**
 * Set if the object is (up/down) resizable.
 *
 * @param obj The icon object
 * @param scale_up A bool to set if the object is resizable up. Default is
 * @c EINA_TRUE.
 * @param scale_down A bool to set if the object is resizable down. Default
 * is @c EINA_TRUE.
 *
 * This function limits the icon object resize ability. If @p scale_up is set to
 * @c EINA_FALSE, the object can't have its height or width resized to a value
 * higher than the original icon size. Same is valid for @p scale_down.
 *
 * @see elm_icon_scale_get()
 * @deprecated Please use elm_icon_resizable_set()
 *
 */
EINA_DEPRECATED EAPI void                  elm_icon_scale_set(Evas_Object *obj, Eina_Bool scale_up, Eina_Bool scale_down);

/**
 * Get if the object is (up/down) resizable.
 *
 * @param obj The icon object
 * @param scale_up A bool to set if the object is resizable up
 * @param scale_down A bool to set if the object is resizable down
 *
 * @see elm_icon_scale_set()
 * @deprecated Please use elm_icon_resizable_get()
 *
 */
EINA_DEPRECATED EAPI void                  elm_icon_scale_get(const Evas_Object *obj, Eina_Bool *scale_up, Eina_Bool *scale_down);

/**
 * Enable or disable preloading of the icon
 *
 * @param obj The icon object
 * @param disable If EINA_TRUE, preloading will be disabled
 * @deprecated Use elm_icon_preload_disabled_set() instead
 */
EINA_DEPRECATED EAPI void                  elm_icon_preload_set(Evas_Object *obj, Eina_Bool disable);

/**
 * Returns the last selected item, for a given index widget.
 *
 * @param obj The index object.
 * @return The last item @b selected on @p obj (or @c NULL, on errors).
 * @deprecated Please use elm_index_selected_item_get() instead.
 *
 *
 */
EINA_DEPRECATED EAPI Elm_Object_Item      *elm_index_item_selected_get(const Evas_Object *obj, int level);

/**
 * Get the value of shrink_mode state.
 *
 * @deprecated elm_multibuttonentry_expanded_get()
 */
EINA_DEPRECATED EAPI int                        elm_multibuttonentry_shrink_mode_get(const Evas_Object *obj);

/**
 * Set/Unset the multibuttonentry to shrink mode state of single line
 *
 * @deprecated elm_multibuttonentry_expanded_set()
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_shrink_mode_set(Evas_Object *obj, int shrink_mode);

/**
 * Get the guide text
 *
 * @param obj The multibuttonentry object
 * @return The guide text, or NULL if none
 *
 * @deprecated Please use elm_object_part_text_get(obj, "guide");
 */
EINA_DEPRECATED EAPI const char                *elm_multibuttonentry_guide_text_get(const Evas_Object *obj);

/**
 * Set the guide text
 *
 * @param obj The multibuttonentry object
 * @param guidetext The guide text string
 *
 * @deprecated Please use elm_object_part_text_set(obj, "guide", guidetext);
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_guide_text_set(Evas_Object *obj, const char *guidetext);

/**
 * Make the elementary object and its children to be unfocusable
 * (or focusable).
 *
 * @param obj The Elementary object to operate on
 * @param tree_unfocusable @c EINA_TRUE for unfocusable,
 *        @c EINA_FALSE for focusable.
 *
 * This sets whether the object @p obj and its children objects
 * are able to take focus or not. If the tree is set as unfocusable,
 * newest focused object which is not in this tree will get focus.
 * This API can be helpful for an object to be deleted.
 * When an object will be deleted soon, it and its children may not
 * want to get focus (by focus reverting or by other focus controls).
 * Then, just use this API before deleting.
 *
 * @see elm_object_tree_unfocusable_get()
 *
 * @deprecated Please use elm_object_tree_unfocusable_set()
 */
EINA_DEPRECATED EAPI void                 elm_object_tree_unfocusable_set(Evas_Object *obj, Eina_Bool tree_unfocusable);

/**
 * Get whether an Elementary object and its children are unfocusable or not.
 *
 * @param obj The Elementary object to get the information from
 * @return @c EINA_TRUE, if the tree is unfocussable,
 *         @c EINA_FALSE if not (and on errors).
 *
 * @see elm_object_tree_unfocusable_set()
 *
 * @deprecated Please use elm_object_tree_unfocusable_get()
 */
EINA_DEPRECATED EAPI Eina_Bool            elm_object_tree_unfocusable_get(const Evas_Object *obj);

/**
 * Animatedly bring in, to the visible are of a genlist, a given
 * item on it.
 *
 * @deprecated elm_genlist_item_bring_in()
 */
EINA_DEPRECATED EAPI void elm_genlist_item_top_bring_in(Elm_Object_Item *it);

/**
 * Animatedly bring in, to the visible are of a genlist, a given
 * item on it.
 *
 * @deprecated elm_genlist_item_bring_in()
 */
EINA_DEPRECATED EAPI void elm_genlist_item_middle_bring_in(Elm_Object_Item *it);

/**
 * Show the portion of a genlist's internal list containing a given
 * item, immediately.
 *
 * @param it The item to display
 *
 * @deprecated elm_genlist_item_show()
 *
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_top_show(Elm_Object_Item *it);

/**
 * Show the portion of a genlist's internal list containing a given
 * item, immediately.
 *
 * @param it The item to display
 *
 * @deprecated elm_genlist_item_show()
 *
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_middle_show(Elm_Object_Item *it);

/**
 * Enable or disable round mode.
 *
 * @param obj The diskselector object.
 * @param round @c EINA_TRUE to enable round mode or @c EINA_FALSE to
 * disable it.
 *
 * Disabled by default. If round mode is enabled the items list will
 * work like a circle list, so when the user reaches the last item,
 * the first one will popup.
 *
 * @see elm_diskselector_round_enabled_get()
 *
 * @deprecated elm_diskselector_round_enabled_set()
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_round_set(Evas_Object *obj, Eina_Bool round);

/**
 * Set whether items on a given gengrid widget are to get their
 * selection callbacks issued for @b every subsequent selection
 * click on them or just for the first click.
 *
 * @param obj The gengrid object
 * @param always_select @c EINA_TRUE to make items "always
 * selected", @c EINA_FALSE, otherwise
 *
 * By default, grid items will only call their selection callback
 * function when firstly getting selected, any subsequent further
 * clicks will do nothing. With this call, you make those
 * subsequent clicks also to issue the selection callbacks.
 *
 * @note <b>Double clicks</b> will @b always be reported on items.
 *
 * @see elm_gengrid_always_select_mode_get()
 *
 */
EINA_DEPRECATED EAPI void                          elm_gengrid_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);

/**
 * Get whether items on a given gengrid widget have their selection
 * callbacks issued for @b every subsequent selection click on them
 * or just for the first click.
 *
 * @param obj The gengrid object.
 * @return @c EINA_TRUE if the gengrid items are "always selected",
 * @c EINA_FALSE, otherwise
 *
 * @see elm_gengrid_always_select_mode_set() for more details
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_gengrid_always_select_mode_get(const Evas_Object *obj);

/**
 * Set whether items on a given gengrid widget can be selected or not.
 *
 * @param obj The gengrid object
 * @param no_select @c EINA_TRUE to make items selectable,
 * @c EINA_FALSE otherwise
 *
 * This will make items in @p obj selectable or not. In the latter
 * case, any user interaction on the gengrid items will neither make
 * them appear selected nor them call their selection callback
 * functions.
 *
 * @see elm_gengrid_no_select_mode_get()
 *
 */
EINA_DEPRECATED EAPI void                          elm_gengrid_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);

/**
 * Get whether items on a given gengrid widget can be selected or
 * not.
 *
 * @param obj The gengrid object
 * @return @c EINA_TRUE, if items are selectable, @c EINA_FALSE
 * otherwise
 *
 * @see elm_gengrid_no_select_mode_set() for more details
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_gengrid_no_select_mode_get(const Evas_Object *obj);

/**
 * Get the side labels max length.
 *
 * @see elm_diskselector_side_text_max_length_set() for details.
 *
 * @param obj The diskselector object.
 * @return The max length defined for side labels, or 0 if not a valid
 * diskselector.
 *
 * @deprecated elm_diskselector_side_text_max_length_get()
 */
EINA_DEPRECATED EAPI int                    elm_diskselector_side_label_length_get(const Evas_Object *obj);

/**
 * Set the side labels max length.
 *
 * @param obj The diskselector object.
 * @param len The max length defined for side labels.
 *
 * Length is the number of characters of items' label that will be
 * visible when it's set on side positions. It will just crop
 * the string after defined size. E.g.:
 *
 * An item with label "January" would be displayed on side position as
 * "Jan" if max length is set to 3, or "Janu", if this property
 * is set to 4.
 *
 * When it's selected, the entire label will be displayed, except for
 * width restrictions. In this case label will be cropped and "..."
 * will be concatenated.
 *
 * Default side label max length is 3.
 *
 * This property will be applied over all items, included before or
 * later this function call.
 *
 * @deprecated elm_diskselector_side_text_max_length_set()
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_side_label_length_set(Evas_Object *obj, int len);

/**
 * Set whether the toolbar should always have an item selected.
 *
 * @param obj The toolbar object.
 * @param always_select @c EINA_TRUE to enable always-select mode or @c EINA_FALSE to
 * disable it.
 *
 * This will cause the toolbar to always have an item selected, and clicking
 * the selected item will not cause a selected event to be emitted. Enabling this mode
 * will immediately select the first toolbar item.
 *
 * Always-selected is disabled by default.
 *
 * @see elm_toolbar_always_select_mode_get().
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);

/**
 * Get whether the toolbar should always have an item selected.
 *
 * @param obj The toolbar object.
 * @return @c EINA_TRUE means an item will always be selected, @c EINA_FALSE indicates
 * that it is possible to have no items selected. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_toolbar_always_select_mode_set() for details.
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_toolbar_always_select_mode_get(const Evas_Object *obj);

/**
 * Set whether the toolbar items' should be selected by the user or not.
 *
 * @param obj The toolbar object.
 * @param no_select @c EINA_TRUE to disable selection or @c EINA_FALSE to
 * enable it.
 *
 * This will turn off the ability to select items entirely and they will
 * neither appear selected nor emit selected signals. The clicked
 * callback function will still be called.
 *
 * Selection is enabled by default.
 *
 * @see elm_toolbar_no_select_mode_get().
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);

/**
 * Set whether the toolbar items' should be selected by the user or not.
 *
 * @param obj The toolbar object.
 * @return @c EINA_TRUE means items can be selected. @c EINA_FALSE indicates
 * they can't. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_toolbar_no_select_mode_set() for details.
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_toolbar_no_select_mode_get(const Evas_Object *obj);

/*
 * Set the interval on time updates for a user mouse button hold
 * on clock widgets' time edition.
 *
 * @param obj The clock object
 * @param interval The (first) interval value in seconds
 *
 * This interval value is @b decreased while the user holds the
 * mouse pointer either incrementing or decrementing a given the
 * clock digit's value.
 *
 * This helps the user to get to a given time distant from the
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
 * @deprecated elm_clock_first_interval_set()
 *
 */
EINA_DEPRECATED EAPI void              elm_clock_interval_set(Evas_Object *obj, double interval);

/**
 * Get the interval on time updates for a user mouse button hold
 * on clock widgets' time edition.
 *
 * @param obj The clock object
 * @return The (first) interval value, in seconds, set on it
 *
 * @elm_clock_first_interval_get()
 */
EINA_DEPRECATED EAPI double            elm_clock_interval_get(const Evas_Object *obj);

/**
 * Set what digits of the given clock widget should be editable
 * when in edition mode.
 *
 * @param obj The clock object
 * @param digedit Bit mask indicating the digits to be editable
 * (values in #Elm_Clock_Edit_Mode).
 *
 * If the @p digedit param is #ELM_CLOCK_NONE, editing will be
 * disabled on @p obj (same effect as elm_clock_edit_set(), with @c
 * EINA_FALSE).
 *
 * @deprecated elm_clock_edit_mode_set()
 */
EINA_DEPRECATED EAPI void              elm_clock_digit_edit_set(Evas_Object *obj, Elm_Clock_Edit_Mode digedit);

/**
 * Retrieve what digits of the given clock widget should be
 * editable when in edition mode.
 *
 * @param obj The clock object
 * @return Bit mask indicating the digits to be editable
 * (values in #Elm_Clock_Edit_Mode).
 *
 * @deprecated elm_clock_edit_mode_get()
 */
EINA_DEPRECATED EAPI Elm_Clock_Edit_Mode elm_clock_digit_edit_get(const Evas_Object *obj);

/*
 * Queries whether it's possible to go back in history
 *
 * @param obj The web object
 *
 * @return EINA_TRUE if it's possible to back in history, EINA_FALSE
 * otherwise
 * @deprecated elm_web_back_possible_get();
 */
EINA_DEPRECATED EAPI Eina_Bool         elm_web_backward_possible(Evas_Object *obj);

/**
 * Queries whether it's possible to go forward in history
 *
 * @param obj The web object
 *
 * @return EINA_TRUE if it's possible to forward in history, EINA_FALSE
 * otherwise
 *
 * @deprecated elm_web_forward_possible_get();
 */
EINA_DEPRECATED EAPI Eina_Bool         elm_web_forward_possible(Evas_Object *obj);

/**
 * Queries whether it's possible to jump the given number of steps
 *
 * @deprecated elm_web_navigate_possible_get();
 *
 */
EINA_DEPRECATED EAPI Eina_Bool         elm_web_navigate_possible(Evas_Object *obj, int steps);

/**
 * @brief Set the content of the scroller widget (the object to be scrolled around).
 *
 * @param obj The scroller object
 * @param content The new content object
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_scroller_content_unset() function.
 * @deprecated use elm_object_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_scroller_content_set(Evas_Object *obj, Evas_Object *child);

/**
 * @brief Get the content of the scroller widget
 *
 * @param obj The slider object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget
 *
 * @see elm_scroller_content_set()
 * @deprecated use elm_object_content_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_scroller_content_get(const Evas_Object *obj);

/**
 * @brief Unset the content of the scroller widget
 *
 * @param obj The slider object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget
 *
 * @see elm_scroller_content_set()
 * @deprecated use elm_object_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_scroller_content_unset(Evas_Object *obj);

/**
 * Set the shrink state of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @param shrink_mode Toolbar's items display behavior.
 *
 * The toolbar won't scroll if #ELM_TOOLBAR_SHRINK_NONE,
 * but will enforce a minimum size so all the items will fit, won't scroll
 * and won't show the items that don't fit if #ELM_TOOLBAR_SHRINK_HIDE,
 * will scroll if #ELM_TOOLBAR_SHRINK_SCROLL, and will create a button to
 * pop up excess elements with #ELM_TOOLBAR_SHRINK_MENU.
 *
 * @deprecated Please use elm_toolbar_shrink_mode_set(obj, shrink_mode);
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_mode_shrink_set(Evas_Object *obj, Elm_Toolbar_Shrink_Mode shrink_mode);

/**
 * Get the shrink mode of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return Toolbar's items display behavior.
 *
 * @see elm_toolbar_shrink_mode_set() for details.
 *
 * @deprecated Please use elm_toolbar_shrink_mode_get(obj);
 */
EINA_DEPRECATED EAPI Elm_Toolbar_Shrink_Mode      elm_toolbar_mode_shrink_get(const Evas_Object *obj);

/**
 * Set the text to be shown in a given toolbar item's tooltips.
 *
 * @param it toolbar item.
 * @param text The text to set in the content.
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data - set with this function or
 * elm_toolbar_item_tooltip_content_cb_set() - is removed.
 *
 * @deprecated Use elm_object_item_tooltip_text_set() instead
 * @see elm_object_tooltip_text_set() for more details.
 *
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Enable or disable always select mode on the list object.
 *
 * @param obj The list object
 * @param always_select @c EINA_TRUE to enable always select mode or
 * @c EINA_FALSE to disable it.
 *
 * @note Always select mode is disabled by default.
 *
 * Default behavior of list items is to only call its callback function
 * the first time it's pressed, i.e., when it is selected. If a selected
 * item is pressed again, and multi-select is disabled, it won't call
 * this function (if multi-select is enabled it will unselect the item).
 *
 * If always select is enabled, it will call the callback function
 * every time a item is pressed, so it will call when the item is selected,
 * and again when a selected item is pressed.
 *
 * @deprecated elm_list_always_select_mode_set()
 */
EINA_DEPRECATED EAPI void                         elm_list_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);

/**
 * Get a value whether always select mode is enabled or not, meaning that
 * an item will always call its callback function, even if already selected.
 *
 * @param obj The list object
 * @return @c EINA_TRUE means horizontal mode selection is enabled.
 * @c EINA_FALSE indicates it's disabled. If @p obj is @c NULL,
 * @c EINA_FALSE is returned.
 *
 * @see elm_list_always_select_mode_set() for details.
 *
 * @deprecated elm_list_always_select_mode_get()
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_always_select_mode_get(const Evas_Object *obj);

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
 * @deprecated No more support. If you need to emit signal to item's edje object, use elm_object_item_signal_emit().
 *
 */
EINA_DEPRECATED EAPI const Evas_Object            *elm_genlist_item_object_get(const Elm_Object_Item *it);

/**
 * Set the always select mode.
 *
 * @param obj The genlist object
 * @param always_select The always select mode (@c EINA_TRUE = on, @c
 * EINA_FALSE = off). Default is @c EINA_FALSE.
 *
 * Items will only call their selection func and callback when first
 * becoming selected. Any further clicks will do nothing, unless you
 * enable always select with elm_genlist_always_select_mode_set().
 * This means that, even if selected, every click will make the selected
 * callbacks be called.
 *
 * @deprecated use elm_genlist_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS); instead. use elm_genlist_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_DEFAULT); for normal mode.
 *
 */
EINA_DEPRECATED EAPI void                          elm_genlist_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);


/**
 * Get the always select mode.
 *
 * @param obj The genlist object
 * @return The always select mode
 * (@c EINA_TRUE = on, @c EINA_FALSE = off)
 *
 * @deprecated use elm_genlist_select_mode_get instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_genlist_always_select_mode_get(const Evas_Object *obj);

/**
 * Enable/disable the no select mode.
 *
 * @param obj The genlist object
 * @param no_select The no select mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * This will turn off the ability to select items entirely and they
 * will neither appear selected nor call selected callback functions.
 *
 * @deprecated use elm_genlist_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE); instead. use elm_genlist_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_DEFAULT); for normal mode.
 *
 */
EINA_DEPRECATED EAPI void                          elm_genlist_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);

/**
 * Get whether the no select mode is enabled.
 *
 * @param obj The genlist object
 * @return The no select mode
 * (@c EINA_TRUE = on, @c EINA_FALSE = off)
 *
 * @deprecated use elm_genlist_select_mode_get instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_genlist_no_select_mode_get(const Evas_Object *obj);

/*
 * Enable/disable compress mode.
 *
 * @param obj The genlist object
 * @param compress The compress mode
 * (@c EINA_TRUE = on, @c EINA_FALSE = off). Default is @c EINA_FALSE.
 *
 * This will enable the compress mode where items are "compressed"
 * horizontally to fit the genlist scrollable viewport width. This is
 * special for genlist.  Do not rely on
 * elm_genlist_mode_set() being set to @c ELM_LIST_COMPRESS to
 * work as genlist needs to handle it specially.
 *
 * @deprecated elm_genlist_mode_set(obj, ELM_LIST_COMPRESS)
 */
EINA_DEPRECATED EAPI void                          elm_genlist_compress_mode_set(Evas_Object *obj, Eina_Bool compress);

/**
 * Sets the display only state of an item.
 *
 * @param it The item
 * @param display_only @c EINA_TRUE if the item is display only, @c
 * EINA_FALSE otherwise.
 *
 * A display only item cannot be selected or unselected. It is for
 * display only and not selecting or otherwise clicking, dragging
 * etc. by the user, thus finger size rules will not be applied to
 * this item.
 *
 * It's good to set group index items to display only state.
 *
 * @see elm_genlist_item_display_only_get()
 *
 * @deprecated elm_genlist_item_display_only_set()
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_display_only_set(Elm_Object_Item *it, Eina_Bool display_only);

/**
 * Get the display only state of an item
 *
 * @param it The item
 * @return @c EINA_TRUE if the item is display only, @c
 * EINA_FALSE otherwise.
 *
 * @see elm_genlist_item_display_only_set()
 *
 * @deprecated elm_genlist_item_display_only_get()
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_genlist_item_display_only_get(const Elm_Object_Item *it);

/**
 * Get whether the compress mode is enabled.
 *
 * @param obj The genlist object
 * @return The compress mode
 * (@c EINA_TRUE = on, @c EINA_FALSE = off)
 *
 * @deprecated elm_genlsit_mode_get()
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_genlist_compress_mode_get(const Evas_Object *obj);

/**
 * Enable/disable height-for-width mode.
 *
 * @param obj The genlist object
 * @param height_for_width The height-for-width mode (@c EINA_TRUE = on,
 * @c EINA_FALSE = off). Default is @c EINA_FALSE.
 *
 * With height-for-width mode the item width will be fixed (restricted
 * to a minimum of) to the list width when calculating its size in
 * order to allow the height to be calculated based on it. This allows,
 * for instance, text block to wrap lines if the Edje part is
 * configured with "text.min: 0 1".
 *
 * @note This mode will make list resize slower as it will have to
 *       recalculate every item height again whenever the list width
 *       changes!
 *
 * @note When height-for-width mode is enabled, it also enables
 *       compress mode (see elm_genlist_compress_mode_set()) and
 *       disables homogeneous (see elm_genlist_homogeneous_set()).
 *
 * @deprecated elm_genlist_mode_set(obj, ELM_LIST_COMPRESS)
 */
EINA_DEPRECATED EAPI void                          elm_genlist_height_for_width_mode_set(Evas_Object *obj, Eina_Bool height_for_width);

/**
 * Get whether the height-for-width mode is enabled.
 *
 * @param obj The genlist object
 * @return The height-for-width mode (@c EINA_TRUE = on, @c EINA_FALSE =
 * off)
 *
 * @deprecated elm_genlist_mode_set(obj, ELM_LIST_COMPRESS)
 */
EINA_DEPRECATED EAPI Eina_Bool                     elm_genlist_height_for_width_mode_get(const Evas_Object *obj);

/**
 * Activate a genlist mode on an item
 *
 * @param it The genlist item
 * @param mode_type Mode name
 * @param mode_set Boolean to define set or unset mode.
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
 * @deprecated elm_genlist_item_decorate_mode_set()
 */
EINA_DEPRECATED EAPI void                          elm_genlist_item_mode_set(Elm_Object_Item *it, const char *mode_type, Eina_Bool mode_set);

/**
 * Get the last (or current) genlist mode used.
 *
 * @param obj The genlist object
 *
 * This function just returns the name of the last used genlist mode. It will
 * be the current mode if it's still active.
 *
 * @deprecated elm_genlist_item_decorate_mode_get()
 */
EINA_DEPRECATED EAPI const char                   *elm_genlist_mode_type_get(const Evas_Object *obj);

/**
 * Get active genlist mode item
 *
 * @param obj The genlist object
 * @return The active item for that current mode. Or @c NULL if no item is
 * activated with any mode.
 *
 * This function returns the item that was activated with a mode, by the
 * function elm_genlist_item_decorate_mode_set().
 *
 * @deprecated elm_genlist_decorated_item_get()
 */
EINA_DEPRECATED EAPI const Elm_Object_Item       *elm_genlist_mode_item_get(const Evas_Object *obj);

/**
 * Set Genlist edit mode
 *
 * This sets Genlist edit mode.
 *
 * @param obj The Genlist object
 * @param The edit mode status
 * (EINA_TRUE = edit mode, EINA_FALSE = normal mode
 *
 * @deprecated elm_genlist_decorate_mode_set
 */
EINA_DEPRECATED EAPI void               elm_genlist_edit_mode_set(Evas_Object *obj, Eina_Bool edit_mode);

/**
 * Get Genlist edit mode
 *
 * @param obj The genlist object
 * @return The edit mode status
 * (EINA_TRUE = edit mode, EINA_FALSE = normal mode
 *
 * @deprecated elm_genlist_decorate_mode_get()
 */
EINA_DEPRECATED EAPI Eina_Bool          elm_genlist_edit_mode_get(const Evas_Object *obj);

/**
 * @}
 */
