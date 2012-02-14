/**
 * @defgroup Map Map
 * @ingroup Elementary
 *
 * @image html img/widget/map/preview-00.png
 * @image latex img/widget/map/preview-00.eps
 *
 * This is a widget specifically for displaying a map. It uses basically
 * OpenStreetMap provider http://www.openstreetmap.org/,
 * but custom providers can be added.
 *
 * It supports some basic but yet nice features:
 * @li zoom and scroll
 * @li markers with content to be displayed when user clicks over it
 * @li group of markers
 * @li routes
 *
 * Smart callbacks one can listen to:
 *
 * - "clicked" - This is called when a user has clicked the map without
 *   dragging around.
 * - "press" - This is called when a user has pressed down on the map.
 * - "longpressed" - This is called when a user has pressed down on the map
 *   for a long time without dragging around.
 * - "clicked,double" - This is called when a user has double-clicked
 *   the map.
 * - "load,detail" - Map detailed data load begins.
 * - "loaded,detail" - This is called when all currently visible parts of
 *   the map are loaded.
 * - "zoom,start" - Zoom animation started.
 * - "zoom,stop" - Zoom animation stopped.
 * - "zoom,change" - Zoom changed when using an auto zoom mode.
 * - "scroll" - the content has been scrolled (moved).
 * - "scroll,anim,start" - scrolling animation has started.
 * - "scroll,anim,stop" - scrolling animation has stopped.
 * - "scroll,drag,start" - dragging the contents around has started.
 * - "scroll,drag,stop" - dragging the contents around has stopped.
 * - "downloaded" - This is called when all currently required map images
 *   are downloaded.
 * - "route,load" - This is called when route request begins.
 * - "route,loaded" - This is called when route request ends.
 * - "name,load" - This is called when name request begins.
 * - "name,loaded- This is called when name request ends.
 *
 * Available style for map widget:
 * - @c "default"
 *
 * Available style for markers:
 * - @c "radio"
 * - @c "radio2"
 * - @c "empty"
 *
 * Available style for marker bubble:
 * - @c "default"
 *
 * List of examples:
 * @li @ref map_example_01
 * @li @ref map_example_02
 * @li @ref map_example_03
 */

/**
 * @addtogroup Map
 * @{
 */

/**
 * Set map's zoom behavior. It can be set to manual or automatic.
 *
 * Default value is #ELM_MAP_ZOOM_MODE_MANUAL.
 *
 * Values <b> don't </b> work as bitmask, only one can be chosen.
 *
 * @note Valid sizes are 2^zoom, consequently the map may be smaller
 * than the scroller view.
 *
 * @see elm_map_zoom_mode_set()
 * @see elm_map_zoom_mode_get()
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_ZOOM_MODE_MANUAL, /**< Zoom controlled manually by elm_map_zoom_set(). It's set by default. */
   ELM_MAP_ZOOM_MODE_AUTO_FIT, /**< Zoom until map fits inside the scroll frame with no pixels outside this area. */
   ELM_MAP_ZOOM_MODE_AUTO_FILL, /**< Zoom until map fills scroll, ensuring no pixels are left unfilled. */
   ELM_MAP_ZOOM_MODE_LAST
} Elm_Map_Zoom_Mode;

/**
 * Set route service to be used. By default used source is
 * #ELM_MAP_ROUTE_SOURCE_YOURS.
 *
 * @see elm_map_route_source_set()
 * @see elm_map_route_source_get()
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_ROUTE_SOURCE_YOURS, /**< Routing service http://www.yournavigation.org/ . Set by default.*/
   ELM_MAP_ROUTE_SOURCE_MONAV, /**< MoNav offers exact routing without heuristic assumptions. Its routing core is based on Contraction Hierarchies. It's not working with Map yet. */
   ELM_MAP_ROUTE_SOURCE_ORS, /**< Open Route Service: http://www.openrouteservice.org/ . It's not working with Map yet. */
   ELM_MAP_ROUTE_SOURCE_LAST
} Elm_Map_Route_Sources;

typedef enum
{
   ELM_MAP_NAME_SOURCE_NOMINATIM,
   ELM_MAP_NAME_SOURCE_LAST
} Elm_Map_Name_Sources;

/**
 * Set type of transport used on route.
 *
 * @see elm_map_route_add()
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_ROUTE_TYPE_MOTOCAR, /**< Route should consider an automobile will be used. */
   ELM_MAP_ROUTE_TYPE_BICYCLE, /**< Route should consider a bicycle will be used by the user. */
   ELM_MAP_ROUTE_TYPE_FOOT, /**< Route should consider user will be walking. */
   ELM_MAP_ROUTE_TYPE_LAST
} Elm_Map_Route_Type;

/**
 * Set the routing method, what should be prioritized, time or distance.
 *
 * @see elm_map_route_add()
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_ROUTE_METHOD_FASTEST, /**< Route should prioritize time. */
   ELM_MAP_ROUTE_METHOD_SHORTEST, /**< Route should prioritized distance. */
   ELM_MAP_ROUTE_METHOD_LAST
} Elm_Map_Route_Method;

typedef enum
{
   ELM_MAP_NAME_METHOD_SEARCH,
   ELM_MAP_NAME_METHOD_REVERSE,
   ELM_MAP_NAME_METHOD_LAST
} Elm_Map_Name_Method;

typedef struct _Elm_Map_Marker       Elm_Map_Marker; /**< A marker to be shown in a specific point of the map. Can be created with elm_map_marker_add() and deleted with elm_map_marker_remove(). */
typedef struct _Elm_Map_Marker_Class Elm_Map_Marker_Class; /**< Each marker must be associated to a class. It's required to add a mark. The class defines the style of the marker when a marker is displayed alone (not grouped). A new class can be created with elm_map_marker_class_new(). */
typedef struct _Elm_Map_Group_Class  Elm_Map_Group_Class; /**< Each marker must be associated to a group class. It's required to add a mark. The group class defines the style of the marker when a marker is grouped to other markers. Markers with the same group are grouped if they are close. A new group class can be created with elm_map_marker_group_class_new(). */
typedef struct _Elm_Map_Route        Elm_Map_Route; /**< A route to be shown in the map. Can be created with elm_map_route_add() and deleted with elm_map_route_remove(). */
typedef struct _Elm_Map_Name         Elm_Map_Name; /**< A handle for specific coordinates. */
typedef struct _Elm_Map_Track        Elm_Map_Track;

typedef Evas_Object               *(*ElmMapMarkerGetFunc)(Evas_Object *obj, Elm_Map_Marker *marker, void *data); /**< Bubble content fetching class function for marker classes. When the user click on a marker, a bubble is displayed with a content. */
typedef void                       (*ElmMapMarkerDelFunc)(Evas_Object *obj, Elm_Map_Marker *marker, void *data, Evas_Object *o); /**< Function to delete bubble content for marker classes. */
typedef Evas_Object               *(*ElmMapMarkerIconGetFunc)(Evas_Object *obj, Elm_Map_Marker *marker, void *data); /**< Icon fetching class function for marker classes. */
typedef Evas_Object               *(*ElmMapGroupIconGetFunc)(Evas_Object *obj, void *data); /**< Icon fetching class function for markers group classes. */

typedef char                      *(*ElmMapModuleSourceFunc)(void);
typedef int                        (*ElmMapModuleZoomMinFunc)(void);
typedef int                        (*ElmMapModuleZoomMaxFunc)(void);
typedef char                      *(*ElmMapModuleUrlFunc)(Evas_Object *obj, int x, int y, int zoom);
typedef int                        (*ElmMapModuleRouteSourceFunc)(void);
typedef char                      *(*ElmMapModuleRouteUrlFunc)(Evas_Object *obj, char *type_name, int method, double flon, double flat, double tlon, double tlat);
typedef char                      *(*ElmMapModuleNameUrlFunc)(Evas_Object *obj, int method, char *name, double lon, double lat);
typedef Eina_Bool                  (*ElmMapModuleGeoIntoCoordFunc)(const Evas_Object *obj, int zoom, double lon, double lat, int size, int *x, int *y);
typedef Eina_Bool                  (*ElmMapModuleCoordIntoGeoFunc)(const Evas_Object *obj, int zoom, int x, int y, int size, double *lon, double *lat);

/**
 * Add a new map widget to the given parent Elementary (container) object.
 *
 * @param parent The parent object.
 * @return a new map widget handle or @c NULL, on errors.
 *
 * This function inserts a new map widget on the canvas.
 *
 * @ingroup Map
 */
EAPI Evas_Object          *elm_map_add(Evas_Object *parent);

/**
 * Set the zoom level of the map.
 *
 * @param obj The map object.
 * @param zoom The zoom level to set.
 *
 * This sets the zoom level.
 *
 * It will respect limits defined by elm_map_source_zoom_min_set() and
 * elm_map_source_zoom_max_set().
 *
 * By default these values are 0 (world map) and 18 (maximum zoom).
 *
 * This function should be used when zoom mode is set to
 * #ELM_MAP_ZOOM_MODE_MANUAL. This is the default mode, and can be set
 * with elm_map_zoom_mode_set().
 *
 * @see elm_map_zoom_mode_set().
 * @see elm_map_zoom_get().
 *
 * @ingroup Map
 */
EAPI void                  elm_map_zoom_set(Evas_Object *obj, int zoom);

/**
 * Get the zoom level of the map.
 *
 * @param obj The map object.
 * @return The current zoom level.
 *
 * This returns the current zoom level of the map object.
 *
 * Note that if you set the fill mode to other than #ELM_MAP_ZOOM_MODE_MANUAL
 * (which is the default), the zoom level may be changed at any time by the
 * map object itself to account for map size and map viewport size.
 *
 * @see elm_map_zoom_set() for details.
 *
 * @ingroup Map
 */
EAPI int                   elm_map_zoom_get(const Evas_Object *obj);

/**
 * Set the zoom mode used by the map object.
 *
 * @param obj The map object.
 * @param mode The zoom mode of the map, being it one of
 * #ELM_MAP_ZOOM_MODE_MANUAL (default), #ELM_MAP_ZOOM_MODE_AUTO_FIT,
 * or #ELM_MAP_ZOOM_MODE_AUTO_FILL.
 *
 * This sets the zoom mode to manual or one of the automatic levels.
 * Manual (#ELM_MAP_ZOOM_MODE_MANUAL) means that zoom is set manually by
 * elm_map_zoom_set() and will stay at that level until changed by code
 * or until zoom mode is changed. This is the default mode.
 *
 * The Automatic modes will allow the map object to automatically
 * adjust zoom mode based on properties. #ELM_MAP_ZOOM_MODE_AUTO_FIT will
 * adjust zoom so the map fits inside the scroll frame with no pixels
 * outside this area. #ELM_MAP_ZOOM_MODE_AUTO_FILL will be similar but
 * ensure no pixels within the frame are left unfilled. Do not forget that
 * the valid sizes are 2^zoom, consequently the map may be smaller than
 * the scroller view.
 *
 * @see elm_map_zoom_set()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_zoom_mode_set(Evas_Object *obj, Elm_Map_Zoom_Mode mode);

/**
 * Get the zoom mode used by the map object.
 *
 * @param obj The map object.
 * @return The zoom mode of the map, being it one of
 * #ELM_MAP_ZOOM_MODE_MANUAL (default), #ELM_MAP_ZOOM_MODE_AUTO_FIT,
 * or #ELM_MAP_ZOOM_MODE_AUTO_FILL.
 *
 * This function returns the current zoom mode used by the map object.
 *
 * @see elm_map_zoom_mode_set() for more details.
 *
 * @ingroup Map
 */
EAPI Elm_Map_Zoom_Mode     elm_map_zoom_mode_get(const Evas_Object *obj);

/**
 * Get the current geographic coordinates of the map.
 *
 * @param obj The map object.
 * @param lon Pointer where to store longitude.
 * @param lat Pointer where to store latitude.
 *
 * This gets the current center coordinates of the map object. It can be
 * set by elm_map_geo_region_bring_in() and elm_map_geo_region_show().
 *
 * @see elm_map_geo_region_bring_in()
 * @see elm_map_geo_region_show()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_geo_region_get(const Evas_Object *obj, double *lon, double *lat);

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
 * @see elm_map_geo_region_show() for a function to avoid animation.
 * @see elm_map_geo_region_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_geo_region_bring_in(Evas_Object *obj, double lon, double lat);

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
 * @see elm_map_geo_region_bring_in() for a function to move with animation.
 * @see elm_map_geo_region_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_geo_region_show(Evas_Object *obj, double lon, double lat);

/**
 * Pause or unpause the map.
 *
 * @param obj The map object.
 * @param paused Use @c EINA_TRUE to pause the map @p obj or @c EINA_FALSE
 * to unpause it.
 *
 * This sets the paused state to on (@c EINA_TRUE) or off (@c EINA_FALSE)
 * for map.
 *
 * The default is off.
 *
 * This will stop zooming using animation, changing zoom levels will
 * change instantly. This will stop any existing animations that are running.
 *
 * @see elm_map_paused_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_paused_set(Evas_Object *obj, Eina_Bool paused);

/**
 * Get a value whether map is paused or not.
 *
 * @param obj The map object.
 * @return @c EINA_TRUE means map is pause. @c EINA_FALSE indicates
 * it is not. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * This gets the current paused state for the map object.
 *
 * @see elm_map_paused_set() for details.
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_paused_get(const Evas_Object *obj);

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
 * @ingroup Map
 */
EAPI void                  elm_map_paused_markers_set(Evas_Object *obj, Eina_Bool paused);

/**
 * Get a value whether markers will be displayed on zoom level changes or not
 *
 * @param obj The map object.
 * @return @c EINA_TRUE means map @b won't display markers or @c EINA_FALSE
 * indicates it will. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * This gets the current markers paused state for the map object.
 *
 * @see elm_map_paused_markers_set() for details.
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_paused_markers_get(const Evas_Object *obj);

/**
 * Get the information of downloading status.
 *
 * @param obj The map object.
 * @param try_num Pointer where to store number of tiles being downloaded.
 * @param finish_num Pointer where to store number of tiles successfully
 * downloaded.
 *
 * This gets the current downloading status for the map object, the number
 * of tiles being downloaded and the number of tiles already downloaded.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_utils_downloading_status_get(const Evas_Object *obj, int *try_num, int *finish_num);

/**
 * Convert a pixel coordinate (x,y) into a geographic coordinate
 * (longitude, latitude).
 *
 * @param obj The map object.
 * @param x the coordinate.
 * @param y the coordinate.
 * @param size the size in pixels of the map.
 * The map is a square and generally his size is : pow(2.0, zoom)*256.
 * @param lon Pointer where to store the longitude that correspond to x.
 * @param lat Pointer where to store the latitude that correspond to y.
 *
 * @note Origin pixel point is the top left corner of the viewport.
 * Map zoom and size are taken on account.
 *
 * @see elm_map_utils_convert_geo_into_coord() if you need the inverse.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_utils_convert_coord_into_geo(const Evas_Object *obj, int x, int y, int size, double *lon, double *lat);

/**
 * Convert a geographic coordinate (longitude, latitude) into a pixel
 * coordinate (x, y).
 *
 * @param obj The map object.
 * @param lon the longitude.
 * @param lat the latitude.
 * @param size the size in pixels of the map. The map is a square
 * and generally his size is : pow(2.0, zoom)*256.
 * @param x Pointer where to store the horizontal pixel coordinate that
 * correspond to the longitude.
 * @param y Pointer where to store the vertical pixel coordinate that
 * correspond to the latitude.
 *
 * @note Origin pixel point is the top left corner of the viewport.
 * Map zoom and size are taken on account.
 *
 * @see elm_map_utils_convert_coord_into_geo() if you need the inverse.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_utils_convert_geo_into_coord(const Evas_Object *obj, double lon, double lat, int size, int *x, int *y);

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
 *
 * @ingroup Map
 */
EAPI Elm_Map_Name         *elm_map_utils_convert_coord_into_name(const Evas_Object *obj, double lon, double lat);

/**
 * Convert a name (address) into a geographic coordinate
 * (longitude, latitude).
 *
 * @param obj The map object.
 * @param name The address.
 * @return name A #Elm_Map_Name handle for this address.
 *
 * To get the longitude and latitude, elm_map_name_region_get()
 * should be used.
 *
 * @see elm_map_utils_convert_coord_into_name() if you need the inverse.
 *
 * @ingroup Map
 */
EAPI Elm_Map_Name         *elm_map_utils_convert_name_into_coord(const Evas_Object *obj, char *address);

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
 * elm_map_utils_convert_coord_into_geo() internally to get the geographic
 * location.
 *
 * see elm_map_rotate_get()
 * see elm_map_utils_convert_coord_into_geo()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_canvas_to_geo_convert(const Evas_Object *obj, const Evas_Coord x, const Evas_Coord y, double *lon, double *lat);

/**
 * Add a new marker to the map object.
 *
 * @param obj The map object.
 * @param lon The longitude of the marker.
 * @param lat The latitude of the marker.
 * @param class The class, to use when marker @b isn't grouped to others.
 * @param class_group The class group, to use when marker is grouped to others
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
 * @ingroup Map
 */
EAPI Elm_Map_Marker       *elm_map_marker_add(Evas_Object *obj, double lon, double lat, Elm_Map_Marker_Class *clas, Elm_Map_Group_Class *clas_group, void *data);

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
 * @ingroup Map
 */
EAPI void                  elm_map_max_marker_per_group_set(Evas_Object *obj, int max);

/**
 * Remove a marker from the map.
 *
 * @param marker The marker to remove.
 *
 * @see elm_map_marker_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_marker_remove(Elm_Map_Marker *marker);

/**
 * Get the current coordinates of the marker.
 *
 * @param marker marker.
 * @param lat Pointer where to store the marker's latitude.
 * @param lon Pointer where to store the marker's longitude.
 *
 * These values are set when adding markers, with function
 * elm_map_marker_add().
 *
 * @see elm_map_marker_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_marker_region_get(const Elm_Map_Marker *marker, double *lon, double *lat);

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
 * @ingroup Map
 */
EAPI void                  elm_map_marker_bring_in(Elm_Map_Marker *marker);

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
 * @ingroup Map
 */
EAPI void                  elm_map_marker_show(Elm_Map_Marker *marker);

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
 * @ingroup Map
 */
EAPI void                  elm_map_markers_list_show(Eina_List *markers);

/**
 * Get the Evas object returned by the ElmMapMarkerGetFunc callback
 *
 * @param marker The marker which content should be returned.
 * @return Return the evas object if it exists, else @c NULL.
 *
 * To set callback function #ElmMapMarkerGetFunc for the marker class,
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
 * @ingroup Map
 */
EAPI Evas_Object          *elm_map_marker_object_get(const Elm_Map_Marker *marker);

/**
 * Update the marker
 *
 * @param marker The marker to be updated.
 *
 * If a content is set to this marker, it will call function to delete it,
 * #ElmMapMarkerDelFunc, and then will fetch the content again with
 * #ElmMapMarkerGetFunc.
 *
 * These functions are set for the marker class with
 * elm_map_marker_class_get_cb_set() and elm_map_marker_class_del_cb_set().
 *
 * @ingroup Map
 */
EAPI void                  elm_map_marker_update(Elm_Map_Marker *marker);

/**
 * Close all the bubbles opened by the user.
 *
 * @param obj The map object.
 *
 * A bubble is displayed with a content fetched with #ElmMapMarkerGetFunc
 * when the user clicks on a marker.
 *
 * This functions is set for the marker class with
 * elm_map_marker_class_get_cb_set().
 *
 * @ingroup Map
 */
EAPI void                  elm_map_bubbles_close(Evas_Object *obj);

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
 * - #ElmMapGroupIconGetFunc - used to fetch icon for markers group classes.
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
 * @ingroup Map
 */
EAPI Elm_Map_Group_Class  *elm_map_group_class_new(Evas_Object *obj);

/**
 * Set the marker's style of a group class.
 *
 * @param class The group class.
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
 * @ingroup Map
 */
EAPI void                  elm_map_group_class_style_set(Elm_Map_Group_Class *clas, const char *style);

/**
 * Set the icon callback function of a group class.
 *
 * @param class The group class.
 * @param icon_get The callback function that will return the icon.
 *
 * Each marker must be associated to a group class, and it can display a
 * custom icon. The function @p icon_get must return this icon.
 *
 * @see elm_map_group_class_new() for more details.
 * @see elm_map_marker_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_group_class_icon_cb_set(Elm_Map_Group_Class *clas, ElmMapGroupIconGetFunc icon_get);

/**
 * Set the data associated to the group class.
 *
 * @param class The group class.
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
 * @ingroup Map
 */
EAPI void                  elm_map_group_class_data_set(Elm_Map_Group_Class *clas, void *data);

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
 * @ingroup Map
 */
EAPI void                  elm_map_group_class_zoom_displayed_set(Elm_Map_Group_Class *clas, int zoom);

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
 * @ingroup Map
 */
EAPI void                  elm_map_group_class_zoom_grouped_set(Elm_Map_Group_Class *clas, int zoom);

/**
 * Set if the markers associated to the group class @p clas are hidden or not.
 *
 * @param clas The group class.
 * @param hide Use @c EINA_TRUE to hide markers or @c EINA_FALSE
 * to show them.
 *
 * If @p hide is @c EINA_TRUE the markers will be hidden, but default
 * is to show them.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_group_class_hide_set(Evas_Object *obj, Elm_Map_Group_Class *clas, Eina_Bool hide);

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
 * - #ElmMapMarkerIconGetFunc - used to fetch icon for markers classes.
 *   It can be set using elm_map_marker_class_icon_cb_set().
 * - #ElmMapMarkerGetFunc - used to fetch bubble content for marker classes.
 *   Set using elm_map_marker_class_get_cb_set().
 * - #ElmMapMarkerDelFunc - used to delete bubble content for marker classes.
 *   Set using elm_map_marker_class_del_cb_set().
 *
 * @see elm_map_marker_add()
 * @see elm_map_marker_class_style_set()
 * @see elm_map_marker_class_icon_cb_set()
 * @see elm_map_marker_class_get_cb_set()
 * @see elm_map_marker_class_del_cb_set()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Marker_Class *elm_map_marker_class_new(Evas_Object *obj);

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
 * @ingroup Map
 */
EAPI void                  elm_map_marker_class_style_set(Elm_Map_Marker_Class *clas, const char *style);

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
 * @ingroup Map
 */
EAPI void                  elm_map_marker_class_icon_cb_set(Elm_Map_Marker_Class *clas, ElmMapMarkerIconGetFunc icon_get);

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
 * @ingroup Map
 */
EAPI void                  elm_map_marker_class_get_cb_set(Elm_Map_Marker_Class *clas, ElmMapMarkerGetFunc get);

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
 * @ingroup Map
 */
EAPI void                  elm_map_marker_class_del_cb_set(Elm_Map_Marker_Class *clas, ElmMapMarkerDelFunc del);

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
 *
 * @ingroup Map
 */
EAPI const char          **elm_map_source_names_get(const Evas_Object *obj);

/**
 * Set the source of the map.
 *
 * @param obj The map object.
 * @param source The source to be used.
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
 *
 *
 * @ingroup Map
 */
EAPI void                  elm_map_source_name_set(Evas_Object *obj, const char *source_name);

/**
 * Get the name of currently used source.
 *
 * @param obj The map object.
 * @return Returns the name of the source in use.
 *
 * @see elm_map_source_name_set() for more details.
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_source_name_get(const Evas_Object *obj);

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
 *
 * @ingroup Map
 */
EAPI void                  elm_map_route_source_set(Evas_Object *obj, Elm_Map_Route_Sources source);

/**
 * Get the current route source.
 *
 * @param obj The map object.
 * @return The source of the route service used by the map.
 *
 * @see elm_map_route_source_set() for details.
 *
 * @ingroup Map
 */
EAPI Elm_Map_Route_Sources elm_map_route_source_get(const Evas_Object *obj);

/**
 * Set the minimum zoom of the source.
 *
 * @param obj The map object.
 * @param zoom New minimum zoom value to be used.
 *
 * By default, it's 0.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_source_zoom_min_set(Evas_Object *obj, int zoom);

/**
 * Get the minimum zoom of the source.
 *
 * @param obj The map object.
 * @return Returns the minimum zoom of the source.
 *
 * @see elm_map_source_zoom_min_set() for details.
 *
 * @ingroup Map
 */
EAPI int                   elm_map_source_zoom_min_get(const Evas_Object *obj);

/**
 * Set the maximum zoom of the source.
 *
 * @param obj The map object.
 * @param zoom New maximum zoom value to be used.
 *
 * By default, it's 18.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_source_zoom_max_set(Evas_Object *obj, int zoom);

/**
 * Get the maximum zoom of the source.
 *
 * @param obj The map object.
 * @return Returns the maximum zoom of the source.
 *
 * @see elm_map_source_zoom_min_set() for details.
 *
 * @ingroup Map
 */
EAPI int                   elm_map_source_zoom_max_get(const Evas_Object *obj);

/**
 * Set the user agent used by the map object to access routing services.
 *
 * @param obj The map object.
 * @param user_agent The user agent to be used by the map.
 *
 * User agent is a client application implementing a network protocol used
 * in communications within a client–server distributed computing system
 *
 * The @p user_agent identification string will transmitted in a header
 * field @c User-Agent.
 *
 * @see elm_map_user_agent_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_user_agent_set(Evas_Object *obj, const char *user_agent);

/**
 * Get the user agent used by the map object.
 *
 * @param obj The map object.
 * @return The user agent identification string used by the map.
 *
 * @see elm_map_user_agent_set() for details.
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_user_agent_get(const Evas_Object *obj);

/**
 * Add a new route to the map object.
 *
 * @param obj The map object.
 * @param type The type of transport to be considered when tracing a route.
 * @param method The routing method, what should be prioritized.
 * @param flon The start longitude.
 * @param flat The start latitude.
 * @param tlon The destination longitude.
 * @param tlat The destination latitude.
 *
 * @return The created route or @c NULL upon failure.
 *
 * A route will be traced by point on coordinates (@p flat, @p flon)
 * to point on coordinates (@p tlat, @p tlon), using the route service
 * set with elm_map_route_source_set().
 *
 * It will take @p type on consideration to define the route,
 * depending if the user will be walking or driving, the route may vary.
 * One of #ELM_MAP_ROUTE_TYPE_MOTOCAR, #ELM_MAP_ROUTE_TYPE_BICYCLE, or
 * #ELM_MAP_ROUTE_TYPE_FOOT need to be used.
 *
 * Another parameter is what the route should prioritize, the minor distance
 * or the less time to be spend on the route. So @p method should be one
 * of #ELM_MAP_ROUTE_METHOD_SHORTEST or #ELM_MAP_ROUTE_METHOD_FASTEST.
 *
 * Routes created with this method can be deleted with
 * elm_map_route_remove(), colored with elm_map_route_color_set(),
 * and distance can be get with elm_map_route_distance_get().
 *
 * @see elm_map_route_remove()
 * @see elm_map_route_color_set()
 * @see elm_map_route_distance_get()
 * @see elm_map_route_source_set()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Route        *elm_map_route_add(Evas_Object *obj, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat);

/**
 * Remove a route from the map.
 *
 * @param route The route to remove.
 *
 * @see elm_map_route_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_route_remove(Elm_Map_Route *route);

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
 *
 * @ingroup Map
 */
EAPI void                  elm_map_route_color_set(Elm_Map_Route *route, int r, int g, int b, int a);

/**
 * Get the route color.
 *
 * @param route The route object.
 * @param r Pointer where to store the red channel value.
 * @param g Pointer where to store the green channel value.
 * @param b Pointer where to store the blue channel value.
 * @param a Pointer where to store the alpha channel value.
 *
 * @see elm_map_route_color_set() for details.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_route_color_get(const Elm_Map_Route *route, int *r, int *g, int *b, int *a);

/**
 * Get the route distance in kilometers.
 *
 * @param route The route object.
 * @return The distance of route (unit : km).
 *
 * @ingroup Map
 */
EAPI double                elm_map_route_distance_get(const Elm_Map_Route *route);

/**
 * Get the information of route nodes.
 *
 * @param route The route object.
 * @return Returns a string with the nodes of route.
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_route_node_get(const Elm_Map_Route *route);

/**
 * Get the information of route waypoint.
 *
 * @param route the route object.
 * @return Returns a string with information about waypoint of route.
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_route_waypoint_get(const Elm_Map_Route *route);

/**
 * Get the address of the name.
 *
 * @param name The name handle.
 * @return Returns the address string of @p name.
 *
 * This gets the coordinates of the @p name, created with one of the
 * conversion functions.
 *
 * @see elm_map_utils_convert_name_into_coord()
 * @see elm_map_utils_convert_coord_into_name()
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_name_address_get(const Elm_Map_Name *name);

/**
 * Get the current coordinates of the name.
 *
 * @param name The name handle.
 * @param lat Pointer where to store the latitude.
 * @param lon Pointer where to store The longitude.
 *
 * This gets the coordinates of the @p name, created with one of the
 * conversion functions.
 *
 * @see elm_map_utils_convert_name_into_coord()
 * @see elm_map_utils_convert_coord_into_name()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_name_region_get(const Elm_Map_Name *name, double *lon, double *lat);

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
 *
 * @ingroup Map
 */
EAPI void                  elm_map_name_remove(Elm_Map_Name *name);

/**
 * Rotate the map.
 *
 * @param obj The map object.
 * @param degree Angle from 0.0 to 360.0 to rotate around Z axis.
 * @param cx Rotation's center horizontal position.
 * @param cy Rotation's center vertical position.
 *
 * @see elm_map_rotate_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_rotate_set(Evas_Object *obj, double degree, Evas_Coord cx, Evas_Coord cy);

/**
 * Get the rotate degree of the map
 *
 * @param obj The map object
 * @param degree Pointer where to store degrees from 0.0 to 360.0
 * to rotate around Z axis.
 * @param cx Pointer where to store rotation's center horizontal position.
 * @param cy Pointer where to store rotation's center vertical position.
 *
 * @see elm_map_rotate_set() to set map rotation.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_rotate_get(const Evas_Object *obj, double *degree, Evas_Coord *cx, Evas_Coord *cy);

/**
 * Enable or disable mouse wheel to be used to zoom in / out the map.
 *
 * @param obj The map object.
 * @param disabled Use @c EINA_TRUE to disable mouse wheel or @c EINA_FALSE
 * to enable it.
 *
 * Mouse wheel can be used for the user to zoom in or zoom out the map.
 *
 * It's disabled by default.
 *
 * @see elm_map_wheel_disabled_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_wheel_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get a value whether mouse wheel is enabled or not.
 *
 * @param obj The map object.
 * @return @c EINA_TRUE means map is disabled. @c EINA_FALSE indicates
 * it is enabled. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * Mouse wheel can be used for the user to zoom in or zoom out the map.
 *
 * @see elm_map_wheel_disabled_set() for details.
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_wheel_disabled_get(const Evas_Object *obj);

#ifdef ELM_EMAP
/**
 * Add a track on the map
 *
 * @param obj The map object.
 * @param emap The emap route object.
 * @return The route object. This is an elm object of type Route.
 *
 * @see elm_route_add() for details.
 *
 * @ingroup Map
 */
EAPI Evas_Object          *elm_map_track_add(Evas_Object *obj, EMap_Route *emap);
#endif

/**
 * Remove a track from the map
 *
 * @param obj The map object.
 * @param route The track to remove.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_track_remove(Evas_Object *obj, Evas_Object *route);

/**
 * @}
 */
