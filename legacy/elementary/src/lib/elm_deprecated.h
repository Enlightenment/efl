
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_add(Evas_Object *parent);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_single_line_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_password_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry);
EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_entry_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_append(Evas_Object *obj, const char *entry);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_is_empty(const Evas_Object *obj);
EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_selection_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_editable_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_select_none(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_select_all(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_next(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_prev(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_up(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_down(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_begin_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_end_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_selection_end(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_cursor_content_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_pos_set(Evas_Object *obj, int pos);
EINA_DEPRECATED EAPI int          elm_scrolled_entry_cursor_pos_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_cut(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_copy(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_paste(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_clear(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_icon_set(Evas_Object *obj, Evas_Object *icon);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_icon_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_icon_unset(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_end_set(Evas_Object *obj, Evas_Object *end);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_end_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_end_unset(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_append(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * entry, const char *item), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_prepend(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * entry, const char *item), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_remove(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * entry, const char *item), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_append(Evas_Object *obj, void (*func)(void *data, Evas_Object *entry, char **text), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_prepend(Evas_Object *obj, void (*func)(void *data, Evas_Object *entry, char **text), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_remove(Evas_Object *obj, void (*func)(void *data, Evas_Object *entry, char **text), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_save(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_autosave_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cnp_textonly_set(Evas_Object *obj, Eina_Bool textonly);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cnp_textonly_get(Evas_Object *obj);

/**
 * Convert a pixel coordinate (x,y) into a geographic coordinate
 * (longitude, latitude).
 *
 * @param obj The map object.
 * @param x the coordinate.
 * @param y the coordinate.
 * @param size the size in pixels of the map.
 * The map is a square and generally his size is : pow(2.0, zoom)*256.
 * @param lon Pointer to store the longitude that correspond to x.
 * @param lat Pointer to store the latitude that correspond to y.
 *
 * @note Origin pixel point is the top left corner of the viewport.
 * Map zoom and size are taken on account.
 *
 * @see elm_map_utils_convert_geo_into_coord() if you need the inverse.
 *
 * @deprecated Use elm_map_canvas_to_geo_convert() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_utils_convert_coord_into_geo(const Evas_Object *obj, int x, int y, int size, double *lon, double *lat);

/**
 * Convert a geographic coordinate (longitude, latitude) into a pixel
 * coordinate (x, y).
 *
 * @param obj The map object.
 * @param lon the longitude.
 * @param lat the latitude.
 * @param size the size in pixels of the map. The map is a square
 * and generally his size is : pow(2.0, zoom)*256.
 * @param x Pointer to store the horizontal pixel coordinate that
 * correspond to the longitude.
 * @param y Pointer to store the vertical pixel coordinate that
 * correspond to the latitude.
 *
 * @note Origin pixel point is the top left corner of the viewport.
 * Map zoom and size are taken on account.
 *
 * @see elm_map_utils_convert_coord_into_geo() if you need the inverse.
 *
 * @deprecatedUse Use elm_map_canvas_to_geo_convert() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_utils_convert_geo_into_coord(const Evas_Object *obj, double lon, double lat, int size, int *x, int *y);

/**
 * Get the information of downloading status.
 *
 * @param obj The map object.
 * @param try_num Pointer to store number of tiles being downloaded.
 * @param finish_num Pointer to store number of tiles successfully
 * downloaded.
 *
 * This gets the current downloading status for the map object, the number
 * of tiles being downloaded and the number of tiles already downloaded.
 *
 * @deprecatedUse Use elm_map_tile_load_status_get() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_utils_downloading_status_get(const Evas_Object *obj, int *try_num, int *finish_num);

/**
 * Convert a geographic coordinate (longitude, latitude) into a name
 * (address).
 *
 * @param obj The map object.
 * @param lon the longitude.
 * @param lat the latitude.
 * @return name A #Elm_Map_Name handle for this coordinate.
 *
 * To get the string for this address, elm_map_name_address_get()
 * should be used.
 *
 * @see elm_map_utils_convert_name_into_coord() if you need the inverse.
 * @deprecatedUse Use elm_map_name_add() instead
 *
 */
EINA_DEPRECATED EAPI Elm_Map_Name         *elm_map_utils_convert_coord_into_name(const Evas_Object *obj, double lon, double lat);

/**
 * Convert a name (address) into a geographic coordinate
 * (longitude, latitude).
 *
 * @param obj The map object.
 * @param address The address.
 * @return name A #Elm_Map_Name handle for this address.
 *
 * To get the longitude and latitude, elm_map_name_region_get()
 * should be used.
 *
 * @see elm_map_utils_convert_coord_into_name() if you need the inverse.
 * @deprecatedUse Use elm_map_name_geo_request() instead
 *
 */
EINA_DEPRECATED EAPI Elm_Map_Name         *elm_map_utils_convert_name_into_coord(const Evas_Object *obj, char *address);

/**
 * Add a new marker to the map object.
 *
 * @param obj The map object.
 * @param lon The longitude of the marker.
 * @param lat The latitude of the marker.
 * @param clas The class, to use when marker @b isn't grouped to others.
 * @param clas_group The class group, to use when marker is grouped to others
 * @param data The data passed to the callbacks.
 *
 * @return The created marker or @c NULL upon failure.
 *
 * A marker will be created and shown in a specific point of the map, defined
 * by @p lon and @p lat.
 *
 * It will be displayed using style defined by @p class when this marker
 * is displayed alone (not grouped). A new class can be created with
 * elm_map_marker_class_new().
 *
 * If the marker is grouped to other markers, it will be displayed with
 * style defined by @p class_group. Markers with the same group are grouped
 * if they are close. A new group class can be created with
 * elm_map_marker_group_class_new().
 *
 * Markers created with this method can be deleted with
 * elm_map_marker_remove().
 *
 * A marker can have associated content to be displayed by a bubble,
 * when a user click over it, as well as an icon. These objects will
 * be fetch using class' callback functions.
 *
 * @see elm_map_marker_class_new()
 * @see elm_map_marker_group_class_new()
 * @see elm_map_marker_remove()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI Elm_Map_Marker       *elm_map_marker_add(Evas_Object *obj, double lon, double lat, Elm_Map_Marker_Class *clas, Elm_Map_Group_Class *clas_group, void *data);

/**
 * Remove a marker from the map.
 *
 * @param marker The marker to remove.
 *
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_remove(Elm_Map_Marker *marker);

/**
 * Get the current coordinates of the marker.
 *
 * @param marker marker.
 * @param lat Pointer to store the marker's latitude.
 * @param lon Pointer to store the marker's longitude.
 *
 * These values are set when adding markers, with function
 * elm_map_marker_add().
 *
 * @see elm_map_marker_add()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_region_get(const Elm_Map_Marker *marker, double *lon, double *lat);

/**
 * Animatedly bring in given marker to the center of the map.
 *
 * @param marker The marker to center at.
 *
 * This causes map to jump to the given @p marker's coordinates
 * and show it (by scrolling) in the center of the viewport, if it is not
 * already centered. This will use animation to do so and take a period
 * of time to complete.
 *
 * @see elm_map_marker_show() for a function to avoid animation.
 * @see elm_map_marker_region_get()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_bring_in(Elm_Map_Marker *marker);

/**
 * Show the given marker at the center of the map, @b immediately.
 *
 * @param marker The marker to center at.
 *
 * This causes map to @b redraw its viewport's contents to the
 * region containing the given @p marker's coordinates, that will be
 * moved to the center of the map.
 *
 * @see elm_map_marker_bring_in() for a function to move with animation.
 * @see elm_map_markers_list_show() if more than one marker need to be
 * displayed.
 * @see elm_map_marker_region_get()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_show(Elm_Map_Marker *marker);

/**
 * Move and zoom the map to display a list of markers.
 *
 * @param markers A list of #Elm_Map_Marker handles.
 *
 * The map will be centered on the center point of the markers in the list.
 * Then the map will be zoomed in order to fit the markers using the maximum
 * zoom which allows display of all the markers.
 *
 * @warning All the markers should belong to the same map object.
 *
 * @see elm_map_marker_show() to show a single marker.
 * @see elm_map_marker_bring_in()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_markers_list_show(Eina_List *markers);

/**
 * Get the Evas object returned by the Elm_Map_Marker_Get_Func callback
 *
 * @param marker The marker which content should be returned.
 * @return Return the evas object if it exists, else @c NULL.
 *
 * To set callback function #Elm_Map_Marker_Get_Func for the marker class,
 * elm_map_marker_class_get_cb_set() should be used.
 *
 * This content is what will be inside the bubble that will be displayed
 * when an user clicks over the marker.
 *
 * This returns the actual Evas object used to be placed inside
 * the bubble. This may be @c NULL, as it may
 * not have been created or may have been deleted, at any time, by
 * the map. <b>Do not modify this object</b> (move, resize,
 * show, hide, etc.), as the map is controlling it. This
 * function is for querying, emitting custom signals or hooking
 * lower level callbacks for events on that object. Do not delete
 * this object under any circumstances.
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI Evas_Object          *elm_map_marker_object_get(const Elm_Map_Marker *marker);

/**
 * Update the marker
 *
 * @param marker The marker to be updated.
 *
 * If a content is set to this marker, it will call function to delete it,
 * #Elm_Map_Marker_Del_Func, and then will fetch the content again with
 * #Elm_Map_Marker_Get_Func.
 *
 * These functions are set for the marker class with
 * elm_map_marker_class_get_cb_set() and elm_map_marker_class_del_cb_set().
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI void                  elm_map_marker_update(Elm_Map_Marker *marker);

/**
 * Create a new group class.
 *
 * @param obj The map object.
 * @return Returns the new group class.
 *
 * Each marker must be associated to a group class. Markers in the same
 * group are grouped if they are close.
 *
 * The group class defines the style of the marker when a marker is grouped
 * to others markers. When it is alone, another class will be used.
 *
 * A group class will need to be provided when creating a marker with
 * elm_map_marker_add().
 *
 * Some properties and functions can be set by class, as:
 * - style, with elm_map_group_class_style_set()
 * - data - to be associated to the group class. It can be set using
 *   elm_map_group_class_data_set().
 * - min zoom to display markers, set with
 *   elm_map_group_class_zoom_displayed_set().
 * - max zoom to group markers, set using
 *   elm_map_group_class_zoom_grouped_set().
 * - visibility - set if markers will be visible or not, set with
 *   elm_map_group_class_hide_set().
 * - #Elm_Map_Group_Icon_Get_Func - used to fetch icon for markers group classes.
 *   It can be set using elm_map_group_class_icon_cb_set().
 *
 * @see elm_map_marker_add()
 * @see elm_map_group_class_style_set()
 * @see elm_map_group_class_data_set()
 * @see elm_map_group_class_zoom_displayed_set()
 * @see elm_map_group_class_zoom_grouped_set()
 * @see elm_map_group_class_hide_set()
 * @see elm_map_group_class_icon_cb_set()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI Elm_Map_Group_Class  *elm_map_group_class_new(Evas_Object *obj);

/**
 * Create a new marker class.
 *
 * @param obj The map object.
 * @return Returns the new group class.
 *
 * Each marker must be associated to a class.
 *
 * The marker class defines the style of the marker when a marker is
 * displayed alone, i.e., not grouped to to others markers. When grouped
 * it will use group class style.
 *
 * A marker class will need to be provided when creating a marker with
 * elm_map_marker_add().
 *
 * Some properties and functions can be set by class, as:
 * - style, with elm_map_marker_class_style_set()
 * - #Elm_Map_Marker_Icon_Get_Func - used to fetch icon for markers classes.
 *   It can be set using elm_map_marker_class_icon_cb_set().
 * - #Elm_Map_Marker_Get_Func - used to fetch bubble content for marker classes.
 *   Set using elm_map_marker_class_get_cb_set().
 * - #Elm_Map_Marker_Del_Func - used to delete bubble content for marker classes.
 *   Set using elm_map_marker_class_del_cb_set().
 *
 * @see elm_map_marker_add()
 * @see elm_map_marker_class_style_set()
 * @see elm_map_marker_class_icon_cb_set()
 * @see elm_map_marker_class_get_cb_set()
 * @see elm_map_marker_class_del_cb_set()
 *
 * @deprecated Use Elm_Map_Overlay instead
 */
EINA_DEPRECATED EAPI Elm_Map_Marker_Class *elm_map_marker_class_new(Evas_Object *obj);

/**
 * Remove a route from the map.
 *
 * @param route The route to remove.
 *
 * @see elm_map_route_add()
 * @deprecated Use elm_map_route_del() instead
 *
 */
EINA_DEPRECATED EAPI void                  elm_map_route_remove(Elm_Map_Route *route);

/**
 * @deprecated Use elm_object_item_data_get instead.
 */
EINA_DEPRECATED EAPI void * elm_multibuttonentry_item_data_get(const Elm_Object_Item *it);

/**
 * @deprecated Use elm_object_item_data_set instead.
 */
EINA_DEPRECATED EAPI void elm_multibuttonentry_item_data_set(Elm_Object_Item *it, void *data);

/**
 * @deprecated Use elm_calendar_select_mode_set instead.
 */
EINA_DEPRECATED EAPI void                 elm_calendar_day_selection_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * @deprecated Use elm_calendar_select_mode_get instead.
 */
EINA_DEPRECATED EAPI Eina_Bool            elm_calendar_day_selection_disabled_get(const Evas_Object *obj);

/**
 * @deprecated Possible orient values for notify.
 *
 * This values should be used in conjunction to elm_notify_orient_set() to
 * set the position in which the notify should appear(relative to its parent)
 * and in conjunction with elm_notify_orient_get() to know where the notify
 * is appearing.
 */
typedef enum
{
   ELM_NOTIFY_ORIENT_TOP, /**< Notify should appear in the top of parent, default */
   ELM_NOTIFY_ORIENT_CENTER, /**< Notify should appear in the center of parent */
   ELM_NOTIFY_ORIENT_BOTTOM, /**< Notify should appear in the bottom of parent */
   ELM_NOTIFY_ORIENT_LEFT, /**< Notify should appear in the left of parent */
   ELM_NOTIFY_ORIENT_RIGHT, /**< Notify should appear in the right of parent */
   ELM_NOTIFY_ORIENT_TOP_LEFT, /**< Notify should appear in the top left of parent */
   ELM_NOTIFY_ORIENT_TOP_RIGHT, /**< Notify should appear in the top right of parent */
   ELM_NOTIFY_ORIENT_BOTTOM_LEFT, /**< Notify should appear in the bottom left of parent */
   ELM_NOTIFY_ORIENT_BOTTOM_RIGHT, /**< Notify should appear in the bottom right of parent */
   ELM_NOTIFY_ORIENT_LAST /**< Sentinel value, @b don't use */
} Elm_Notify_Orient;


/**
 * @deprecated Use elm_notify_align_set instead.
 */
EINA_DEPRECATED EAPI void                         elm_notify_orient_set(Evas_Object *obj, Elm_Notify_Orient orient);

/**
 * @deprecated Use elm_notify_align_get instead.
 */
EINA_DEPRECATED EAPI Elm_Notify_Orient            elm_notify_orient_get(const Evas_Object *obj);

/**
 * @brief Set slide effect of label widget.
 *
 * @param obj The label object
 * @param slide If true, slide effect will be shown
 *
 * If set to true, the text of the label will slide/scroll through the length of
 * label.
 *
 * @warning This only works with the themes "slide_short", "slide_long" and
 * "slide_bounce".
 * @warning This doesn't work if the line wrap(elm_label_line_wrap_set()) or
 * ellipsis(elm_label_ellipsis_set()) is set.
 *
 * @deprecated see elm_label_slide_mode_set() instead.
 *
 * @ingroup Label
 */
EINA_DEPRECATED EAPI void                        elm_label_slide_set(Evas_Object *obj, Eina_Bool slide);

/**
 * @brief Get whether slide effect is shown or not.
 *
 * @param obj The label object
 * @return If true, slide effect is shown.
 *
 * @see elm_label_slide_set()
 *
 * @deprecated see elm_label_slide_mode_get() instead.
 *
 * @ingroup Label
 */
EINA_DEPRECATED EAPI Eina_Bool                   elm_label_slide_get(const Evas_Object *obj);

/**
 * Set the text for an object's part, marking it as translatable.
 *
 * The string to set as @p text must be the original one. Do not pass the
 * return of @c gettext() here. Elementary will translate the string
 * internally and set it on the object using elm_object_part_text_set(),
 * also storing the original string so that it can be automatically
 * translated when the language is changed with elm_language_set().
 *
 * The @p domain will be stored along to find the translation in the
 * correct catalog. It can be NULL, in which case it will use whatever
 * domain was set by the application with @c textdomain(). This is useful
 * in case you are building a library on top of Elementary that will have
 * its own translatable strings, that should not be mixed with those of
 * programs using the library.
 *
 * @param obj The object containing the text part
 * @param part The name of the part to set
 * @param domain The translation domain to use
 * @param text The original, non-translated text to set
 *
 * @deprecated Use elm_object_domain_translatable_part_text_set() instead.
 *
 * @ingroup General
 */
EINA_DEPRECATED EAPI void      elm_object_domain_translatable_text_part_set(Evas_Object *obj, const char *part, const char *domain, const char *text);

/**
 * Get the original string set as translatable for an object
 *
 * When setting translated strings, the function elm_object_part_text_get()
 * will return the translation returned by @c gettext(). To get the
 * original string use this function.
 *
 * @param obj The object
 * @param part The name of the part that was set
 *
 * @return The original, untranslated string
 *
 * @deprecated Use elm_object_translatable_part_text_get() instead.
 *
 * @ingroup General
 */
EINA_DEPRECATED EAPI const char *elm_object_translatable_text_part_get(const Evas_Object *obj, const char *part);

/**
 * @brief Show/Hide the title area
 *
 * @param it The naviframe item
 * @param visible If @c EINA_TRUE, title area will be visible, hidden
 *        otherwise
 *
 * When the title area is invisible, then the controls would be hidden so as
 * to expand the content area to full-size.
 *
 * @deprecated Use elm_naviframe_item_title_enabled_set() instead.
 *
 * @see also elm_naviframe_item_title_visible_get()
 * @see also elm_naviframe_item_title_enabled_get()
 *
 * @ingroup Naviframe
 */
EINA_DEPRECATED EAPI void      elm_naviframe_item_title_visible_set(Elm_Object_Item *it, Eina_Bool visible);

/**
 * @brief Get a value whether title area is visible or not.
 *
 * @param it The naviframe item
 * @return If @c EINA_TRUE, title area is visible
 *
 * @deprecated Use elm_naviframe_item_title_enabled_get() instead.
 *
 * @see also elm_naviframe_item_title_visible_set()
 *
 * @ingroup Naviframe
 */
EINA_DEPRECATED EAPI Eina_Bool elm_naviframe_item_title_visible_get(const Elm_Object_Item *it);

/**
 * Enable/disable horizontal and vertical bouncing effect.
 *
 * @param obj The genlist object
 * @param h_bounce Allow bounce horizontally (@c EINA_TRUE = on, @c
 * EINA_FALSE = off). Default is @c EINA_FALSE.
 * @param v_bounce Allow bounce vertically (@c EINA_TRUE = on, @c
 * EINA_FALSE = off). Default is @c EINA_TRUE.
 *
 * This will enable or disable the scroller bouncing effect for the
 * genlist. See elm_scroller_bounce_set() for details.
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @see elm_scroller_bounce_set()
 * @see elm_genlist_bounce_get()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get whether the horizontal and vertical bouncing effect is enabled.
 *
 * @param obj The genlist object
 * @param h_bounce Pointer to a bool to receive if the bounce horizontally
 * option is set.
 * @param v_bounce Pointer to a bool to receive if the bounce vertically
 * option is set.
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @see elm_scroller_bounce_get()
 * @see elm_genlist_bounce_set()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Set the scrollbar policy
 *
 * @param obj The genlist object
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given genlist
 * scroller. #ELM_SCROLLER_POLICY_AUTO means the scrollbar is
 * made visible if it is needed, and otherwise kept hidden. #ELM_SCROLLER_POLICY_ON
 * turns it on all the time, and #ELM_SCROLLER_POLICY_OFF always keeps it off.
 * This applies respectively for the horizontal and vertical scrollbars.
 * Default is #ELM_SCROLLER_POLICY_AUTO
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @see elm_scroller_policy_set()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Get the scrollbar policy
 *
 * @param obj The genlist object
 * @param policy_h Pointer to store the horizontal scrollbar policy.
 * @param policy_v Pointer to store the vertical scrollbar policy.
 *
 * @deprecated Use elm_scroller_policy_get() instead.
 *
 * @see elm_scroller_policy_get()
 *
 * @ingroup Genlist
 */
EINA_DEPRECATED EAPI void          elm_genlist_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * This sets the entry's scrollbar policy (i.e. enabling/disabling
 * them).
 *
 * Setting an entry to single-line mode with elm_entry_single_line_set()
 * will automatically disable the display of scrollbars when the entry
 * moves inside its scroller.
 *
 * @param obj The entry object
 * @param h The horizontal scrollbar policy to apply
 * @param v The vertical scrollbar policy to apply
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @ingroup Entry
 */
EINA_DEPRECATED EAPI void elm_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v);

/**
 * This enables/disables bouncing within the entry.
 *
 * This function sets whether the entry will bounce when scrolling reaches
 * the end of the contained entry.
 *
 * @param obj The entry object
 * @param h_bounce The horizontal bounce state
 * @param v_bounce The vertical bounce state
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @ingroup Entry
 */
EINA_DEPRECATED EAPI void elm_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get the bounce mode
 *
 * @param obj The Entry object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @ingroup Entry
 */
EINA_DEPRECATED EAPI void elm_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * @brief Set the photocam scrolling bouncing.
 *
 * @param obj The photocam object
 * @param h_bounce set this to @c EINA_TRUE for horizontal bouncing
 * @param v_bounce set this to @c EINA_TRUE for vertical bouncing
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @ingroup Photocam
 */
EINA_DEPRECATED EAPI void   elm_photocam_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * @brief Get the photocam scrolling bouncing.
 *
 * @param obj The photocam object
 * @param h_bounce horizontal bouncing
 * @param v_bounce vertical bouncing
 *
 * @see elm_photocam_bounce_set()
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @ingroup Photocam
 */
EINA_DEPRECATED EAPI void   elm_photocam_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Set bouncing behaviour when the scrolled content reaches an edge.
 *
 * Tell the internal scroller object whether it should bounce or not
 * when it reaches the respective edges for each axis.
 *
 * @param obj The list object
 * @param h_bounce Whether to bounce or not in the horizontal axis.
 * @param v_bounce Whether to bounce or not in the vertical axis.
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @see elm_scroller_bounce_set()
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void         elm_list_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get the bouncing behaviour of the internal scroller.
 *
 * Get whether the internal scroller should bounce when the edge of each
 * axis is reached scrolling.
 *
 * @param obj The list object.
 * @param h_bounce Pointer to store the bounce state of the horizontal
 * axis.
 * @param v_bounce Pointer to store the bounce state of the vertical
 * axis.
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @see elm_scroller_bounce_get()
 * @see elm_list_bounce_set()
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void         elm_list_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Set the scrollbar policy.
 *
 * @param obj The list object
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given
 * scroller. #ELM_SCROLLER_POLICY_AUTO means the scrollbar is made
 * visible if it is needed, and otherwise kept
 * hidden. #ELM_SCROLLER_POLICY_ON turns it on all the time, and
 * #ELM_SCROLLER_POLICY_OFF always keeps it off. This applies
 * respectively for the horizontal and vertical scrollbars.
 *
 * The both are disabled by default, i.e., are set to
 * #ELM_SCROLLER_POLICY_OFF.
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void         elm_list_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Get the scrollbar policy.
 *
 * @see elm_list_scroller_policy_get() for details.
 *
 * @param obj The list object.
 * @param policy_h Pointer to store horizontal scrollbar policy.
 * @param policy_v Pointer to store vertical scrollbar policy.
 *
 * @deprecated Use elm_scroller_policy_get() instead.
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void         elm_list_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * @brief Set custom theme elements for the scroller
 *
 * @param obj The scroller object
 * @param widget The widget name to use (default is "scroller")
 * @param base The base name to use (default is "base")
 *
 * @deprecated Use elm_layout_theme_set() instead.
 *
 * @ingroup Scroller
 */
EINA_DEPRECATED EAPI void         elm_scroller_custom_widget_base_theme_set(Evas_Object *obj, const char *widget, const char *base);

/**
 * Set bouncing behaviour when the scrolled content reaches an edge.
 *
 * Tell the internal scroller object whether it should bounce or not
 * when it reaches the respective edges for each axis.
 *
 * @param obj The diskselector object.
 * @param h_bounce Whether to bounce or not in the horizontal axis.
 * @param v_bounce Whether to bounce or not in the vertical axis.
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @see elm_scroller_bounce_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void elm_diskselector_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get the bouncing behaviour of the internal scroller.
 *
 * Get whether the internal scroller should bounce when the edge of each
 * axis is reached scrolling.
 *
 * @param obj The diskselector object.
 * @param h_bounce Pointer to store the bounce state of the horizontal
 * axis.
 * @param v_bounce Pointer to store the bounce state of the vertical
 * axis.
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @see elm_scroller_bounce_get()
 * @see elm_diskselector_bounce_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void elm_diskselector_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Get the scrollbar policy.
 *
 * @see elm_diskselector_scroller_policy_get() for details.
 *
 * @param obj The diskselector object.
 * @param policy_h Pointer to store horizontal scrollbar policy.
 * @param policy_v Pointer to store vertical scrollbar policy.
 *
 * @deprecated Use elm_scroller_policy_get() instead.
 *
 * @see elm_scroller_policy_get()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void elm_diskselector_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * Set the scrollbar policy.
 *
 * @param obj The diskselector object.
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given
 * scroller. #ELM_SCROLLER_POLICY_AUTO means the scrollbar is made visible if
 * it is needed, and otherwise kept hidden. #ELM_SCROLLER_POLICY_ON turns
 * it on all the time, and #ELM_SCROLLER_POLICY_OFF always keeps it off.
 * This applies respectively for the horizontal and vertical scrollbars.
 *
 * The both are disabled by default, i.e., are set to #ELM_SCROLLER_POLICY_OFF.
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @see elm_scroller_policy_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void elm_diskselector_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Set the file that will be used as icon.
 *
 * @param obj The icon object
 * @param file The path to file that will be used as icon image
 * @param group The group that the icon belongs to an edje file
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @note The icon image set by this function can be changed by
 * elm_icon_standard_set().
 *
 * @see elm_icon_file_get()
 *
 * @deprecated Use elm_image_file_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Set a location in memory to be used as an icon
 *
 * @param obj The icon object
 * @param img The binary data that will be used as an image
 * @param size The size of binary data @p img
 * @param format Optional format of @p img to pass to the image loader
 * @param key Optional key of @p img to pass to the image loader (eg. if @p img is an edje file)
 *
 * The @p format string should be something like "png", "jpg", "tga",
 * "tiff", "bmp" etc. if it is provided (NULL if not). This improves
 * the loader performance as it tries the "correct" loader first before
 * trying a range of other possible loaders until one succeeds.
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @note The icon image set by this function can be changed by
 * elm_icon_standard_set().
 *
 * @deprecated Use elm_image_memfile_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key);

/**
 * Get the file that will be used as icon.
 *
 * @param obj The icon object
 * @param file The path to file that will be used as the icon image
 * @param group The group that the icon belongs to, in edje file
 *
 * @see elm_image_file_set()
 *
 * @deprecated Use elm_image_file_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_file_get(const Evas_Object *obj, const char **file, const char **group);

/**
 * Set the smooth scaling for an icon object.
 *
 * @param obj The icon object
 * @param smooth @c EINA_TRUE if smooth scaling should be used, @c EINA_FALSE
 * otherwise. Default is @c EINA_TRUE.
 *
 * Set the scaling algorithm to be used when scaling the icon image. Smooth
 * scaling provides a better resulting image, but is slower.
 *
 * The smooth scaling should be disabled when making animations that change
 * the icon size, since they will be faster. Animations that don't require
 * resizing of the icon can keep the smooth scaling enabled (even if the icon
 * is already scaled, since the scaled icon image will be cached).
 *
 * @see elm_icon_smooth_get()
 *
 * @deprecated Use elm_image_smooth_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_smooth_set(Evas_Object *obj, Eina_Bool smooth);

/**
 * Get whether smooth scaling is enabled for an icon object.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if smooth scaling is enabled, @c EINA_FALSE otherwise.
 *
 * @see elm_icon_smooth_set()
 *
 * @deprecated Use elm_image_smooth_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_smooth_get(const Evas_Object *obj);

/**
 * Disable scaling of this object.
 *
 * @param obj The icon object.
 * @param no_scale @c EINA_TRUE if the object is not scalable, @c EINA_FALSE
 * otherwise. Default is @c EINA_FALSE.
 *
 * This function disables scaling of the icon object through the function
 * elm_object_scale_set(). However, this does not affect the object
 * size/resize in any way. For that effect, take a look at
 * elm_icon_resizable_set().
 *
 * @see elm_icon_no_scale_get()
 * @see elm_icon_resizable_set()
 * @see elm_object_scale_set()
 *
 * @deprecated Use elm_image_no_scale_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);

/**
 * Get whether scaling is disabled on the object.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if scaling is disabled, @c EINA_FALSE otherwise
 *
 * @see elm_icon_no_scale_set()
 *
 * @deprecated Use elm_image_no_scale_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_no_scale_get(const Evas_Object *obj);

/**
 * Set if the object is (up/down) resizable.
 *
 * @param obj The icon object
 * @param size_up A bool to set if the object is resizable up. Default is
 * @c EINA_TRUE.
 * @param size_down A bool to set if the object is resizable down. Default
 * is @c EINA_TRUE.
 *
 * This function limits the icon object resize ability. If @p size_up is set to
 * @c EINA_FALSE, the object can't have its height or width resized to a value
 * higher than the original icon size. Same is valid for @p size_down.
 *
 * @see elm_icon_resizable_get()
 *
 * @deprecated Use elm_image_resizable_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_resizable_set(Evas_Object *obj, Eina_Bool size_up, Eina_Bool size_down);

/**
 * Get if the object is (up/down) resizable.
 *
 * @param obj The icon object
 * @param size_up A bool to set if the object is resizable up
 * @param size_down A bool to set if the object is resizable down
 *
 * @see elm_icon_resizable_set()
 *
 * @deprecated Use elm_image_resizable_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_resizable_get(const Evas_Object *obj, Eina_Bool *size_up, Eina_Bool *size_down);

/**
 * Get the object's image size
 *
 * @param obj The icon object
 * @param w A pointer to store the width in
 * @param h A pointer to store the height in
 *
 * @deprecated Use elm_image_object_size_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * Set if the icon fill the entire object area.
 *
 * @param obj The icon object
 * @param fill_outside @c EINA_TRUE if the object is filled outside,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * When the icon object is resized to a different aspect ratio from the
 * original icon image, the icon image will still keep its aspect. This flag
 * tells how the image should fill the object's area. They are: keep the
 * entire icon inside the limits of height and width of the object (@p
 * fill_outside is @c EINA_FALSE) or let the extra width or height go outside
 * of the object, and the icon will fill the entire object (@p fill_outside
 * is @c EINA_TRUE).
 *
 * @note Unlike @ref Image, there's no option in icon to set the aspect ratio
 * retain property to false. Thus, the icon image will always keep its
 * original aspect ratio.
 *
 * @see elm_icon_fill_outside_get()
 *
 * @deprecated Use elm_image_fill_outside_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);

/**
 * Get if the object is filled outside.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if the object is filled outside, @c EINA_FALSE
 *         otherwise.
 *
 * @see elm_icon_fill_outside_set()
 *
 * @deprecated Use elm_image_fill_outside_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_fill_outside_get(const Evas_Object *obj);

/**
 * Set the prescale size for the icon.
 *
 * @param obj The icon object
 * @param size The prescale size. This value is used for both width and
 * height.
 *
 * This function sets a new size for pixmap representation of the given
 * icon. It allows the icon to be loaded already in the specified size,
 * reducing the memory usage and load time when loading a big icon with load
 * size set to a smaller size.
 *
 * It's equivalent to the elm_bg_load_size_set() function for bg.
 *
 * @note this is just a hint, the real size of the pixmap may differ
 * depending on the type of icon being loaded, being bigger than requested.
 *
 * @see elm_icon_prescale_get()
 * @see elm_bg_load_size_set()
 *
 * @deprecated Use elm_image_prescale_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_prescale_set(Evas_Object *obj, int size);

/**
 * Get the prescale size for the icon.
 *
 * @param obj The icon object
 * @return The prescale size
 *
 * @see elm_icon_prescale_set()
 *
 * @deprecated Use elm_image_prescale_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI int                   elm_icon_prescale_get(const Evas_Object *obj);

/**
 * Get the image object of the icon. DO NOT MODIFY THIS.
 *
 * @param obj The icon object
 * @return The internal icon object
 *
 * @deprecated Use elm_image_object_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Evas_Object          *elm_icon_object_get(Evas_Object *obj);

/**
 * Enable or disable preloading of the icon
 *
 * @param obj The icon object
 * @param disabled If EINA_TRUE, preloading will be disabled
 * @ingroup Icon
 *
 * @deprecated Use elm_image_preload_disabled_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_preload_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get if the icon supports animation or not.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if the icon supports animation,
 *         @c EINA_FALSE otherwise.
 *
 * Return if this elm icon's image can be animated. Currently Evas only
 * supports gif animation. If the return value is EINA_FALSE, other
 * elm_icon_animated_xxx APIs won't work.
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_available_get() instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_animated_available_get(const Evas_Object *obj);

/**
 * Set animation mode of the icon.
 *
 * @param obj The icon object
 * @param animated @c EINA_TRUE if the object do animation job,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * Since the default animation mode is set to EINA_FALSE,
 * the icon is shown without animation. Files like animated GIF files
 * can animate, and this is supported if you enable animated support
 * on the icon.
 * Set it to EINA_TRUE when the icon needs to be animated.
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_animated_set(Evas_Object *obj, Eina_Bool animated);

/**
 * Get animation mode of the icon.
 *
 * @param obj The icon object
 * @return The animation mode of the icon object
 * @see elm_icon_animated_set
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_get() instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_animated_get(const Evas_Object *obj);

/**
 * Set animation play mode of the icon.
 *
 * @param obj The icon object
 * @param play @c EINA_TRUE the object play animation images,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * To play elm icon's animation, set play to EINA_TRUE.
 * For example, you make gif player using this set/get API and click event.
 * This literally lets you control current play or paused state. To have
 * this work with animated GIF files for example, you first, before
 * setting the file have to use elm_icon_animated_set() to enable animation
 * at all on the icon.
 *
 * 1. Click event occurs
 * 2. Check play flag using elm_icon_animated_play_get
 * 3. If elm icon was playing, set play to EINA_FALSE.
 *    Then animation will be stopped and vice versa
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_play_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_animated_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * Get animation play mode of the icon.
 *
 * @param obj The icon object
 * @return The play mode of the icon object
 *
 * @see elm_icon_animated_play_get
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_play_get() instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_animated_play_get(const Evas_Object *obj);

/**
 * Set whether the original aspect ratio of the icon should be kept on resize.
 *
 * @param obj The icon object.
 * @param fixed @c EINA_TRUE if the icon should retain the aspect,
 * @c EINA_FALSE otherwise.
 *
 * The original aspect ratio (width / height) of the icon is usually
 * distorted to match the object's size. Enabling this option will retain
 * this original aspect, and the way that the icon is fit into the object's
 * area depends on the option set by elm_icon_fill_outside_set().
 *
 * @see elm_icon_aspect_fixed_get()
 * @see elm_icon_fill_outside_set()
 *
 * @ingroup Icon
 *
 * @deprecated Use elm_image_aspect_fixed_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Get if the object retains the original aspect ratio.
 *
 * @param obj The icon object.
 * @return @c EINA_TRUE if the object keeps the original aspect, @c EINA_FALSE
 * otherwise.
 *
 * @deprecated Use elm_image_aspect_fixed_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_aspect_fixed_get(const Evas_Object *obj);

/**
 * Set the initial file system path for a given file selector
 * button widget
 *
 * @param obj The file selector button widget
 * @param path The path string
 *
 * It must be a <b>directory</b> path, which will have the contents
 * displayed initially in the file selector's view, when invoked
 * from @p obj. The default initial path is the @c "HOME"
 * environment variable's value.
 *
 * @see elm_fileselector_path_get()
 *
 * @deprecated Use elm_fileselector_path_set() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI void        elm_fileselector_button_path_set(Evas_Object *obj, const char *path);

/**
 * Get the initial file system path set for a given file selector
 * button widget
 *
 * @param obj The file selector button widget
 * @return path The path string
 *
 * @see elm_fileselector_path_set() for more details
 *
 * @deprecated Use elm_fileselector_path_get() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_button_path_get(const Evas_Object *obj);

/**
 * Enable/disable a tree view in the given file selector button
 * widget's internal file selector
 *
 * @param obj The file selector button widget
 * @param value @c EINA_TRUE to enable tree view, @c EINA_FALSE to
 * disable
 *
 * This has the same effect as elm_fileselector_expandable_set(),
 * but now applied to a file selector button's internal file
 * selector.
 *
 * @note There's no way to put a file selector button's internal
 * file selector in "grid mode", as one may do with "pure" file
 * selectors.
 *
 * @see elm_fileselector_expandable_get()
 *
 * @deprecated Use elm_fileselector_expandable_set() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI void        elm_fileselector_button_expandable_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether tree view is enabled for the given file selector
 * button widget's internal file selector
 *
 * @param obj The file selector button widget
 * @return @c EINA_TRUE if @p obj widget's internal file selector
 * is in tree view, @c EINA_FALSE otherwise (and or errors)
 *
 * @see elm_fileselector_expandable_set() for more details
 *
 * @deprecated Use elm_fileselector_expandable_get() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_fileselector_button_expandable_get(const Evas_Object *obj);

/**
 * Set whether a given file selector button widget's internal file
 * selector is to display folders only or the directory contents,
 * as well.
 *
 * @param obj The file selector button widget
 * @param value @c EINA_TRUE to make @p obj widget's internal file
 * selector only display directories, @c EINA_FALSE to make files
 * to be displayed in it too
 *
 * This has the same effect as elm_fileselector_folder_only_set(),
 * but now applied to a file selector button's internal file
 * selector.
 *
 * @see elm_fileselector_folder_only_get()
 *
 * @deprecated Use elm_fileselector_folder_only_set() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI void        elm_fileselector_button_folder_only_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether a given file selector button widget's internal file
 * selector is displaying folders only or the directory contents,
 * as well.
 *
 * @param obj The file selector button widget
 * @return @c EINA_TRUE if @p obj widget's internal file
 * selector is only displaying directories, @c EINA_FALSE if files
 * are being displayed in it too (and on errors)
 *
 * @see elm_fileselector_folder_only_set() for more details
 *
 * @deprecated Use elm_fileselector_folder_only_get() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_fileselector_button_folder_only_get(const Evas_Object *obj);

/**
 * Enable/disable the file name entry box where the user can type
 * in a name for a file, in a given file selector button widget's
 * internal file selector.
 *
 * @param obj The file selector button widget
 * @param value @c EINA_TRUE to make @p obj widget's internal
 * file selector a "saving dialog", @c EINA_FALSE otherwise
 *
 * This has the same effect as elm_fileselector_is_save_set(),
 * but now applied to a file selector button's internal file
 * selector.
 *
 * @see elm_fileselector_is_save_get()
 *
 * @deprecated Use elm_fileselector_is_save_set() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI void        elm_fileselector_button_is_save_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether the given file selector button widget's internal
 * file selector is in "saving dialog" mode
 *
 * @param obj The file selector button widget
 * @return @c EINA_TRUE, if @p obj widget's internal file selector
 * is in "saving dialog" mode, @c EINA_FALSE otherwise (and on
 * errors)
 *
 * @see elm_fileselector_is_save_set() for more details
 *
 * @deprecated Use elm_fileselector_is_save_get() instead.
 *
 * @ingroup File_Selector_Button
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_fileselector_button_is_save_get(const Evas_Object *obj);

/**
 * Set the initial file system path and the entry's path string for
 * a given file selector entry widget
 *
 * @param obj The file selector entry widget
 * @param path The path string
 *
 * It must be a <b>directory</b> path, which will have the contents
 * displayed initially in the file selector's view, when invoked
 * from @p obj. The default initial path is the @c "HOME"
 * environment variable's value.
 *
 * @see elm_fileselector_path_get()
 *
 * @deprecated Use elm_fileselector_path_set() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_path_set(Evas_Object *obj, const char *path);

/**
 * Get the entry's path string for a given file selector entry
 * widget
 *
 * @param obj The file selector entry widget
 * @return path The path string
 *
 * @see elm_fileselector_path_set() for more details
 *
 * @deprecated Use elm_fileselector_path_get() instead.
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_entry_path_get(const Evas_Object *obj);

/**
 * Enable/disable a tree view in the given file selector entry
 * widget's internal file selector
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to enable tree view, @c EINA_FALSE to disable
 *
 * This has the same effect as elm_fileselector_expandable_set(),
 * but now applied to a file selector entry's internal file
 * selector.
 *
 * @note There's no way to put a file selector entry's internal
 * file selector in "grid mode", as one may do with "pure" file
 * selectors.
 *
 * @see elm_fileselector_expandable_get()
 *
 * @deprecated Use elm_fileselector_expandable_set() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_expandable_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether tree view is enabled for the given file selector
 * entry widget's internal file selector
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE if @p obj widget's internal file selector
 * is in tree view, @c EINA_FALSE otherwise (and or errors)
 *
 * @see elm_fileselector_expandable_set() for more details
 *
 * @deprecated Use elm_fileselector_expandable_get() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_fileselector_entry_expandable_get(const Evas_Object *obj);

/**
 * Set whether a given file selector entry widget's internal file
 * selector is to display folders only or the directory contents,
 * as well.
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to make @p obj widget's internal file
 * selector only display directories, @c EINA_FALSE to make files
 * to be displayed in it too
 *
 * This has the same effect as elm_fileselector_folder_only_set(),
 * but now applied to a file selector entry's internal file
 * selector.
 *
 * @see elm_fileselector_folder_only_get()
 *
 * @deprecated Use elm_fileselector_folder_only_set() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_folder_only_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether a given file selector entry widget's internal file
 * selector is displaying folders only or the directory contents,
 * as well.
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE if @p obj widget's internal file
 * selector is only displaying directories, @c EINA_FALSE if files
 * are being displayed in it too (and on errors)
 *
 * @see elm_fileselector_folder_only_set() for more details
 *
 * @deprecated Use elm_fileselector_folder_only_get() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_fileselector_entry_folder_only_get(const Evas_Object *obj);

/**
 * Enable/disable the file name entry box where the user can type
 * in a name for a file, in a given file selector entry widget's
 * internal file selector.
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to make @p obj widget's internal
 * file selector a "saving dialog", @c EINA_FALSE otherwise
 *
 * This has the same effect as elm_fileselector_is_save_set(),
 * but now applied to a file selector entry's internal file
 * selector.
 *
 * @see elm_fileselector_is_save_get()
 *
 * @deprecated Use elm_fileselector_is_save_set() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_is_save_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether the given file selector entry widget's internal
 * file selector is in "saving dialog" mode
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE, if @p obj widget's internal file selector
 * is in "saving dialog" mode, @c EINA_FALSE otherwise (and on
 * errors)
 *
 * @see elm_fileselector_is_save_set() for more details
 *
 * @deprecated Use elm_fileselector_is_save_get() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI Eina_Bool   elm_fileselector_entry_is_save_get(const Evas_Object *obj);

/**
 * Set the initial file system path for a given file selector entry
 * widget
 *
 * @param obj The file selector entry widget
 * @param path The path string
 *
 * It must be a <b>directory</b> path, which will have the contents
 * displayed initially in the file selector's view, when invoked
 * from @p obj. The default initial path is the @c "HOME"
 * environment variable's value.
 *
 * @see elm_fileselector_path_get()
 *
 * @deprecated Use elm_fileselector_selected_set() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_selected_set(Evas_Object *obj, const char *path);

/**
 * Get the parent directory's path to the latest file selection on
 * a given filer selector entry widget
 *
 * @param obj The file selector object
 * @return The (full) path of the directory of the last selection
 * on @p obj widget, a @b stringshared string
 *
 * @see elm_fileselector_path_set()
 *
 * @deprecated Use elm_fileselector_selected_get() instead.
 *
 * @ingroup File_Selector_Entry
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_entry_selected_get(const Evas_Object *obj);

//TODO: remvoe below - use elm_access_text_set(); or elm_access_cb_set();
EINA_DEPRECATED EAPI void elm_access_external_info_set(Evas_Object *obj, const char *text);
EINA_DEPRECATED EAPI char *elm_access_external_info_get(const Evas_Object *obj);
