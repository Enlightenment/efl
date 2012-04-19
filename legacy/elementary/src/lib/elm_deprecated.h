
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

