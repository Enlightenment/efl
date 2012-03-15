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
 * @brief Gets the label of the toggle
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
 * @brief Gets the labels associated with the on and off states of the
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
 * @brief Gets the state of the toggle to @p state.
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

/*
 * Add a new pager to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_pager_add(Evas_Object *parent);

/**
 * @brief Push an object to the top of the pager stack (and show it).
 *
 * @param obj The pager object
 * @param content The object to push
 *
 * The object pushed becomes a child of the pager, it will be controlled and
 * deleted when the pager is deleted.
 *
 * @note If the content is already in the stack use
 * elm_pager_content_promote().
 * @warning Using this function on @p content already in the stack results in
 * undefined behavior.
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI void         elm_pager_content_push(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Pop the object that is on top of the stack
 *
 * @param obj The pager object
 *
 * This pops the object that is on the top(visible) of the pager, makes it
 * disappear, then deletes the object. The object that was underneath it on
 * the stack will become visible.
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI void         elm_pager_content_pop(Evas_Object *obj);

/**
 * @brief Moves an object already in the pager stack to the top of the stack.
 *
 * @param obj The pager object
 * @param content The object to promote
 *
 * This will take the @p content and move it to the top of the stack as
 * if it had been pushed there.
 *
 * @note If the content isn't already in the stack use
 * elm_pager_content_push().
 * @warning Using this function on @p content not already in the stack
 * results in undefined behavior.
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI void         elm_pager_content_promote(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Return the object at the bottom of the pager stack
 *
 * @param obj The pager object
 * @return The bottom object or NULL if none
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_pager_content_bottom_get(const Evas_Object *obj);

/**
 * @brief  Return the object at the top of the pager stack
 *
 * @param obj The pager object
 * @return The top object or NULL if none
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_pager_content_top_get(const Evas_Object *obj);

/**
 * @brief Set the default item style.
 *
 * Default item style will be used with items who's style is NULL
 *
 * @param obj The pager object
 * @param style The style
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI void             elm_pager_item_style_default_set(Evas_Object *obj, const char *style);

/**
 * @brief Get the default item style
 *
 * @param obj The pager object
 * @return the default item style
 *
 * @see elm_pager_item_style_default_set()
 * @deprecated Use naviframe instead
 */
EINA_DEPRECATED EAPI const char      *elm_pager_item_style_default_get(const Evas_Object *obj);

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
 * Gets whether the no select mode is enabled.
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
