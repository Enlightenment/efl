/**
 * @defgroup Map Map
 * @ingroup Elementary
 *
 * @image html map_inheritance_tree.png
 * @image latex map_inheritance_tree.eps
 *
 * @image html img/widget/map/preview-00.png
 * @image latex img/widget/map/preview-00.eps
 *
 * This is a widget specifically for displaying a map. It uses basically
 * OpenStreetMap provider http://www.openstreetmap.org/,
 * but custom providers can be added.
 *
 * It supports some basic but yet nice features:
 * @li zooming and scrolling,
 * @li markers with content to be displayed when user clicks over them,
 * @li group of markers and
 * @li routes.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base @ref
 * Scroller widget also work for map objects.
 *
 * Smart callbacks one can listen to:
 * - @c "clicked" - This is called when a user has clicked the map without
 *                  dragging around.
 * - @c "clicked,double" - This is called when a user has double-clicked
 *                         the map.
 * - @c "press" - This is called when a user has pressed down on the map.
 * - @c "longpressed" - This is called when a user has pressed down on the map
 *   @c for a long time without dragging around.
 * - @c "scroll" - the content has been scrolled (moved).
 * - @c "scroll,drag,start" - dragging the contents around has started.
 * - @c "scroll,drag,stop" - dragging the contents around has stopped.
 * - @c "scroll,anim,start" - scrolling animation has started.
 * - @c "scroll,anim,stop" - scrolling animation has stopped.
 * - @c "zoom,start" - Zoom animation started.
 * - @c "zoom,stop" - Zoom animation stopped.
 * - @c "zoom,change" - Zoom changed when using an auto zoom mode.
 * - @c "tile,load" - A map tile image load begins.
 * - @c "tile,loaded" -  A map tile image load ends.
 * - @c "tile,loaded,fail" -  A map tile image load fails.
 * - @c "route,load" - Route request begins.
 * - @c "route,loaded" - Route request ends.
 * - @c "route,loaded,fail" - Route request fails.
 * - @c "name,load" - Name request begins.
 * - @c "name,loaded" - Name request ends.
 * - @c "name,loaded,fail" - Name request fails.
 * - @c "overlay,clicked" - A overlay is clicked.
 * - @c "loaded" - when a map is finally loaded. (since 1.7)
 * - @c "language,changed" - the program's language changed
 * - @c "focused" - When the map has received focus. (since 1.8)
 * - @c "unfocused" - When the map has lost focus. (since 1.8)
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

#define ELM_OBJ_MAP_PAN_CLASS elm_obj_map_pan_class_get()

const Eo_Class *elm_obj_map_pan_class_get(void) EINA_CONST;

#define ELM_OBJ_MAP_CLASS elm_obj_map_class_get()

const Eo_Class *elm_obj_map_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_MAP_BASE_ID;

enum
{
   ELM_OBJ_MAP_SUB_ID_ZOOM_SET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_GET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_MODE_SET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_MODE_GET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_MAX_SET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_MAX_GET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_MIN_SET,
   ELM_OBJ_MAP_SUB_ID_ZOOM_MIN_GET,
   ELM_OBJ_MAP_SUB_ID_REGION_BRING_IN,
   ELM_OBJ_MAP_SUB_ID_REGION_SHOW,
   ELM_OBJ_MAP_SUB_ID_REGION_GET,
   ELM_OBJ_MAP_SUB_ID_PAUSED_SET,
   ELM_OBJ_MAP_SUB_ID_PAUSED_GET,
   ELM_OBJ_MAP_SUB_ID_ROTATE_SET,
   ELM_OBJ_MAP_SUB_ID_ROTATE_GET,
   ELM_OBJ_MAP_SUB_ID_WHEEL_DISABLED_SET,
   ELM_OBJ_MAP_SUB_ID_WHEEL_DISABLED_GET,
   ELM_OBJ_MAP_SUB_ID_TILE_LOAD_STATUS_GET,
   ELM_OBJ_MAP_SUB_ID_CANVAS_TO_REGION_CONVERT,
   ELM_OBJ_MAP_SUB_ID_REGION_TO_CANVAS_CONVERT,
   ELM_OBJ_MAP_SUB_ID_USER_AGENT_SET,
   ELM_OBJ_MAP_SUB_ID_USER_AGENT_GET,
   ELM_OBJ_MAP_SUB_ID_SOURCE_SET,
   ELM_OBJ_MAP_SUB_ID_SOURCE_GET,
   ELM_OBJ_MAP_SUB_ID_SOURCES_GET,
   ELM_OBJ_MAP_SUB_ID_ROUTE_ADD,
   ELM_OBJ_MAP_SUB_ID_NAME_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAYS_GET,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_CLASS_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_BUBBLE_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_ROUTE_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_LINE_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_POLYGON_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_CIRCLE_ADD,
   ELM_OBJ_MAP_SUB_ID_OVERLAY_SCALE_ADD,
   ELM_OBJ_MAP_SUB_ID_TRACK_ADD,
   ELM_OBJ_MAP_SUB_ID_TRACK_REMOVE,
   ELM_OBJ_MAP_SUB_ID_LAST
};

#define ELM_OBJ_MAP_ID(sub_id) (ELM_OBJ_MAP_BASE_ID + sub_id)


/**
 * @def elm_obj_map_zoom_set
 * @since 1.8
 *
 * Set the zoom level of the map.
 *
 * @param[in] zoom
 *
 * @see elm_map_zoom_set
 */
#define elm_obj_map_zoom_set(zoom) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_SET), EO_TYPECHECK(int, zoom)

/**
 * @def elm_obj_map_zoom_get
 * @since 1.8
 *
 * Get the zoom level of the map.
 *
 * @param[out] ret
 *
 * @see elm_map_zoom_get
 */
#define elm_obj_map_zoom_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_map_zoom_mode_set
 * @since 1.8
 *
 * Set the zoom mode used by the map object.
 *
 * @param[in] mode
 *
 * @see elm_map_zoom_mode_set
 */
#define elm_obj_map_zoom_mode_set(mode) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_MODE_SET), EO_TYPECHECK(Elm_Map_Zoom_Mode, mode)

/**
 * @def elm_obj_map_zoom_mode_get
 * @since 1.8
 *
 * Get the zoom mode used by the map object.
 *
 * @param[out] ret
 *
 * @see elm_map_zoom_mode_get
 */
#define elm_obj_map_zoom_mode_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_MODE_GET), EO_TYPECHECK(Elm_Map_Zoom_Mode *, ret)

/**
 * @def elm_obj_map_zoom_max_set
 * @since 1.8
 *
 * Set the maximum zoom of the source.
 *
 * @param[in] zoom
 *
 * @see elm_map_zoom_max_set
 */
#define elm_obj_map_zoom_max_set(zoom) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_MAX_SET), EO_TYPECHECK(int, zoom)

/**
 * @def elm_obj_map_zoom_max_get
 * @since 1.8
 *
 * Get the maximum zoom of the source.
 *
 * @param[out] ret
 *
 * @see elm_map_zoom_max_get
 */
#define elm_obj_map_zoom_max_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_MAX_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_map_zoom_min_set
 * @since 1.8
 *
 * Set the minimum zoom of the source.
 *
 * @param[in] zoom
 *
 * @see elm_map_zoom_min_set
 */
#define elm_obj_map_zoom_min_set(zoom) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_MIN_SET), EO_TYPECHECK(int, zoom)

/**
 * @def elm_obj_map_zoom_min_get
 * @since 1.8
 *
 * Get the minimum zoom of the source.
 *
 * @param[out] ret
 *
 * @see elm_map_zoom_min_get
 */
#define elm_obj_map_zoom_min_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ZOOM_MIN_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_map_region_bring_in
 * @since 1.8
 *
 * Animatedly bring in given coordinates to the center of the map.
 *
 * @param[in] lon
 * @param[in] lat
 *
 * @see elm_map_region_bring_in
 */
#define elm_obj_map_region_bring_in(lon, lat) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_REGION_BRING_IN), EO_TYPECHECK(double, lon), EO_TYPECHECK(double, lat)

/**
 * @def elm_obj_map_region_show
 * @since 1.8
 *
 * Show the given coordinates at the center of the map, immediately.
 *
 * @param[in] lon
 * @param[in] lat
 *
 * @see elm_map_region_show
 */
#define elm_obj_map_region_show(lon, lat) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_REGION_SHOW), EO_TYPECHECK(double, lon), EO_TYPECHECK(double, lat)

/**
 * @def elm_obj_map_region_get
 * @since 1.8
 *
 * Get the current geographic coordinates of the map.
 *
 * @param[out] lon
 * @param[out] lat
 *
 * @see elm_map_region_get
 */
#define elm_obj_map_region_get(lon, lat) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_REGION_GET), EO_TYPECHECK(double *, lon), EO_TYPECHECK(double *, lat)

/**
 * @def elm_obj_map_paused_set
 * @since 1.8
 *
 * Pause or unpause the map.
 *
 * @param[in] paused
 *
 * @see elm_map_paused_set
 */
#define elm_obj_map_paused_set(paused) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_PAUSED_SET), EO_TYPECHECK(Eina_Bool, paused)

/**
 * @def elm_obj_map_paused_get
 * @since 1.8
 *
 * Get a value whether map is paused or not.
 *
 * @param[out] ret
 *
 * @see elm_map_paused_get
 */
#define elm_obj_map_paused_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_PAUSED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_map_rotate_set
 * @since 1.8
 *
 * Rotate the map.
 *
 * @param[in] degree
 * @param[in] cx
 * @param[in] cy
 *
 * @see elm_map_rotate_set
 */
#define elm_obj_map_rotate_set(degree, cx, cy) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ROTATE_SET), EO_TYPECHECK(double, degree), EO_TYPECHECK(Evas_Coord, cx), EO_TYPECHECK(Evas_Coord, cy)

/**
 * @def elm_obj_map_rotate_get
 * @since 1.8
 *
 * Get the rotate degree of the map
 *
 * @param[out] degree
 * @param[out] cx
 * @param[out] cy
 *
 * @see elm_map_rotate_get
 */
#define elm_obj_map_rotate_get(degree, cx, cy) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ROTATE_GET), EO_TYPECHECK(double *, degree), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy)

/**
 * @def elm_obj_map_wheel_disabled_set
 * @since 1.8
 *
 * Enable or disable mouse wheel to be used to zoom in / out the map.
 *
 * @param[in] disabled
 *
 * @see elm_map_wheel_disabled_set
 */
#define elm_obj_map_wheel_disabled_set(disabled) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_WHEEL_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_map_wheel_disabled_get
 * @since 1.8
 *
 * Get a value whether mouse wheel is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_map_wheel_disabled_get
 */
#define elm_obj_map_wheel_disabled_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_WHEEL_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_map_tile_load_status_get
 * @since 1.8
 *
 * Get the information of tile load status.
 *
 * @param[out] try_num
 * @param[out] finish_num
 *
 * @see elm_map_tile_load_status_get
 */
#define elm_obj_map_tile_load_status_get(try_num, finish_num) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_TILE_LOAD_STATUS_GET), EO_TYPECHECK(int *, try_num), EO_TYPECHECK(int *, finish_num)

/**
 * @def elm_obj_map_canvas_to_region_convert
 * @since 1.8
 *
 * Convert canvas coordinates into geographic coordinates
 *
 * @param[in] x
 * @param[in] y
 * @param[out] lon
 * @param[out] lat
 *
 * @see elm_map_canvas_to_region_convert
 */
#define elm_obj_map_canvas_to_region_convert(x, y, lon, lat) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_CANVAS_TO_REGION_CONVERT), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(double *, lon), EO_TYPECHECK(double *, lat)

/**
 * @def elm_obj_map_region_to_canvas_convert
 * @since 1.8
 *
 * Convert geographic coordinates (longitude, latitude)
 *
 * @param[in] lon
 * @param[in] lat
 * @param[out] x
 * @param[out] y
 *
 * @see elm_map_region_to_canvas_convert
 */
#define elm_obj_map_region_to_canvas_convert(lon, lat, x, y) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_REGION_TO_CANVAS_CONVERT), EO_TYPECHECK(double, lon), EO_TYPECHECK(double, lat), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y)

/**
 * @def elm_obj_map_user_agent_set
 * @since 1.8
 *
 * Set the user agent used by the map object to access routing services.
 *
 * @param[in] user_agent
 *
 * @see elm_map_user_agent_set
 */
#define elm_obj_map_user_agent_set(user_agent) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_USER_AGENT_SET), EO_TYPECHECK(const char *, user_agent)

/**
 * @def elm_obj_map_user_agent_get
 * @since 1.8
 *
 * Get the user agent used by the map object.
 *
 * @param[out] ret
 *
 * @see elm_map_user_agent_get
 */
#define elm_obj_map_user_agent_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_USER_AGENT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_map_source_set
 * @since 1.8
 *
 * Set the current source of the map for a specific type.
 *
 * @param[in] type
 * @param[in] source_name
 *
 * @see elm_map_source_set
 */
#define elm_obj_map_source_set(type, source_name) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_SOURCE_SET), EO_TYPECHECK(Elm_Map_Source_Type, type), EO_TYPECHECK(const char *, source_name)

/**
 * @def elm_obj_map_source_get
 * @since 1.8
 *
 * Get the name of currently used source for a specific type.
 *
 * @param[in] type
 * @param[out] ret
 *
 * @see elm_map_source_get
 */
#define elm_obj_map_source_get(type, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_SOURCE_GET), EO_TYPECHECK(Elm_Map_Source_Type, type), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_map_sources_get
 * @since 1.8
 *
 * Get the names of available sources for a specific type.
 *
 * @param[in] type
 * @param[out] ret
 *
 * @see elm_map_sources_get
 */
#define elm_obj_map_sources_get(type, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_SOURCES_GET), EO_TYPECHECK(Elm_Map_Source_Type, type), EO_TYPECHECK(const char ***, ret)

/**
 * @def elm_obj_map_route_add
 * @since 1.8
 *
 * Add a new route to the map object.
 *
 * @param[in] type
 * @param[in] method
 * @param[in] flon
 * @param[in] flat
 * @param[in] tlon
 * @param[in] tlat
 * @param[in] route_cb
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_map_route_add
 */
#define elm_obj_map_route_add(type, method, flon, flat, tlon, tlat, route_cb, data, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_ROUTE_ADD), EO_TYPECHECK(Elm_Map_Route_Type, type), EO_TYPECHECK(Elm_Map_Route_Method, method), EO_TYPECHECK(double, flon), EO_TYPECHECK(double, flat), EO_TYPECHECK(double, tlon), EO_TYPECHECK(double, tlat), EO_TYPECHECK(Elm_Map_Route_Cb, route_cb), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Map_Route **, ret)

/**
 * @def elm_obj_map_name_add
 * @since 1.8
 *
 * Request a address or geographic coordinates(longitude, latitude)
 *
 * @param[in] address
 * @param[in] lon
 * @param[in] lat
 * @param[in] name_cb
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_map_name_add
 */
#define elm_obj_map_name_add(address, lon, lat, name_cb, data, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_NAME_ADD), EO_TYPECHECK(const char *, address), EO_TYPECHECK(double, lon), EO_TYPECHECK(double, lat), EO_TYPECHECK(Elm_Map_Name_Cb, name_cb), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Map_Name **, ret)

/**
 * @def elm_obj_map_overlay_add
 * @since 1.8
 *
 * Add a new overlay to the map object. This overlay has a default type.
 *
 * @param[in] lon
 * @param[in] lat
 * @param[out] ret
 *
 * @see elm_map_overlay_add
 */
#define elm_obj_map_overlay_add(lon, lat, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_ADD), EO_TYPECHECK(double, lon), EO_TYPECHECK(double, lat), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlays_get
 * @since 1.8
 *
 * Return all overlays in the map object.
 *
 * @param[out] ret
 *
 * @see elm_map_overlays_get
 */
#define elm_obj_map_overlays_get(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAYS_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def elm_obj_map_overlay_class_add
 * @since 1.8
 *
 * Add a new class overlay to the map object.
 *
 * @param[out] ret
 *
 * @see elm_map_overlay_class_add
 */
#define elm_obj_map_overlay_class_add(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_CLASS_ADD), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlay_bubble_add
 * @since 1.8
 *
 * Add a new bubble overlay to the map object.
 *
 * @param[out] ret
 *
 * @see elm_map_overlay_bubble_add
 */
#define elm_obj_map_overlay_bubble_add(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_BUBBLE_ADD), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlay_route_add
 * @since 1.8
 *
 * Add a new route overlay to the map object.
 *
 * @param[in] route
 * @param[out] ret
 *
 * @see elm_map_overlay_route_add
 */
#define elm_obj_map_overlay_route_add(route, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_ROUTE_ADD), EO_TYPECHECK(const Elm_Map_Route *, route), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlay_line_add
 * @since 1.8
 *
 * Add a new line overlay to the map object.
 *
 * @param[in] flon
 * @param[in] flat
 * @param[in] tlon
 * @param[in] tlat
 * @param[out] ret
 *
 * @see elm_map_overlay_line_add
 */
#define elm_obj_map_overlay_line_add(flon, flat, tlon, tlat, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_LINE_ADD), EO_TYPECHECK(double, flon), EO_TYPECHECK(double, flat), EO_TYPECHECK(double, tlon), EO_TYPECHECK(double, tlat), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlay_polygon_add
 * @since 1.8
 *
 * Add a new polygon overlay to the map object.
 *
 * @param[out] ret
 *
 * @see elm_map_overlay_polygon_add
 */
#define elm_obj_map_overlay_polygon_add(ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_POLYGON_ADD), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlay_circle_add
 * @since 1.8
 *
 * Add a new circle overlay to the map object.
 *
 * @param[in] lon
 * @param[in] lat
 * @param[in] radius
 * @param[out] ret
 *
 * @see elm_map_overlay_circle_add
 */
#define elm_obj_map_overlay_circle_add(lon, lat, radius, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_CIRCLE_ADD), EO_TYPECHECK(double, lon), EO_TYPECHECK(double, lat), EO_TYPECHECK(double, radius), EO_TYPECHECK(Elm_Map_Overlay **, ret)

/**
 * @def elm_obj_map_overlay_scale_add
 * @since 1.8
 *
 * Add a new scale overlay to the map object.
 *
 * @param[in] x
 * @param[in] y
 * @param[out] ret
 *
 * @see elm_map_overlay_scale_add
 */
#define elm_obj_map_overlay_scale_add(x, y, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_SCALE_ADD), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Elm_Map_Overlay **, ret)
#ifdef ELM_EMAP

/**
 * @def elm_obj_map_track_add
 * @since 1.8
 *
 * Add a track on the map
 *
 * @param[in] emap
 * @param[out] ret
 *
 * @see elm_map_track_add
 */
#define elm_obj_map_track_add(emap, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_TRACK_ADD), EO_TYPECHECK(void *, emap), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_map_track_remove
 * @since 1.8
 *
 * Remove a track from the map
 *
 * @param[in] route
 *
 * @see elm_map_track_remove
 */
#define elm_obj_map_track_remove(route) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_TRACK_REMOVE), EO_TYPECHECK(Evas_Object *, route)
#endif

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
   ELM_MAP_ZOOM_MODE_MANUAL,      /**< Zoom controlled manually by elm_map_zoom_set(). It's set by default. */
   ELM_MAP_ZOOM_MODE_AUTO_FIT,    /**< Zoom until map fits inside the scroll frame with no pixels outside this area. */
   ELM_MAP_ZOOM_MODE_AUTO_FILL,   /**< Zoom until map fills scroll, ensuring no pixels are left unfilled. */
   ELM_MAP_ZOOM_MODE_LAST
} Elm_Map_Zoom_Mode;

/**
 * Set type of a external source (provider).
 *
 * @see elm_map_sources_get()
 * @see elm_map_source_get()
 * @see elm_map_source_set()
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_SOURCE_TYPE_TILE,   /**< Map tile provider. */
   ELM_MAP_SOURCE_TYPE_ROUTE,  /**< Route service provider. */
   ELM_MAP_SOURCE_TYPE_NAME,   /**< Name service provider. */
   ELM_MAP_SOURCE_TYPE_LAST
} Elm_Map_Source_Type;

/**
 * Set type of transport used on route.
 *
 * @see elm_map_route_add()
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_ROUTE_TYPE_MOTOCAR,   /**< Route should consider an automobile will be used. */
   ELM_MAP_ROUTE_TYPE_BICYCLE,   /**< Route should consider a bicycle will be used by the user. */
   ELM_MAP_ROUTE_TYPE_FOOT,      /**< Route should consider user will be walking. */
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
   ELM_MAP_ROUTE_METHOD_FASTEST,  /**< Route should prioritize time. */
   ELM_MAP_ROUTE_METHOD_SHORTEST, /**< Route should prioritize distance. */
   ELM_MAP_ROUTE_METHOD_LAST
} Elm_Map_Route_Method;

/**
 * Set the name search method.
 *
 * This is for name module interface.
 *
 * @ingroup Map
 */
typedef enum
{
   ELM_MAP_NAME_METHOD_SEARCH,
   ELM_MAP_NAME_METHOD_REVERSE,
   ELM_MAP_NAME_METHOD_LAST
} Elm_Map_Name_Method;

/**
 * Set overlay type to be used. This type is resolved
 * when the overlay is created.
 * You can get this value by elm_map_overlay_type_get().
 *
 * @see elm_map_overlay_type_get()
 * @see elm_map_overlay_add()
 * @see elm_map_overlay_class_add()
 * @see elm_map_overlay_bubble_add()
 *
 * @ingroup Map
 */
typedef enum _Elm_Map_Overlay_Type
{
   ELM_MAP_OVERLAY_TYPE_NONE = 0,
   ELM_MAP_OVERLAY_TYPE_DEFAULT,
   ELM_MAP_OVERLAY_TYPE_CLASS,
   ELM_MAP_OVERLAY_TYPE_GROUP,
   ELM_MAP_OVERLAY_TYPE_BUBBLE,
   ELM_MAP_OVERLAY_TYPE_ROUTE,
   ELM_MAP_OVERLAY_TYPE_LINE,
   ELM_MAP_OVERLAY_TYPE_POLYGON,
   ELM_MAP_OVERLAY_TYPE_CIRCLE,
   ELM_MAP_OVERLAY_TYPE_SCALE

} Elm_Map_Overlay_Type;

typedef struct _Elm_Map_Marker       Elm_Map_Marker;       /**< A marker to be shown in a specific point of the map. Can be created with elm_map_marker_add() and deleted with elm_map_marker_remove(). */
typedef struct _Elm_Map_Marker_Class Elm_Map_Marker_Class; /**< Each marker must be associated to a class. It's required to add a mark. The class defines the style of the marker when a marker is displayed alone (not grouped). A new class can be created with elm_map_marker_class_new(). */
typedef struct _Elm_Map_Group_Class  Elm_Map_Group_Class;  /**< Each marker must be associated to a group class. It's required to add a mark. The group class defines the style of the marker when a marker is grouped to other markers. Markers with the same group are grouped if they are close. A new group class can be created with elm_map_marker_group_class_new(). */
typedef struct _Elm_Map_Route        Elm_Map_Route;        /**< A route to be shown in the map. Can be created with elm_map_route_add() and deleted with elm_map_route_del(). */
typedef struct _Elm_Map_Name         Elm_Map_Name;         /**< A handle for specific coordinates. */
typedef struct _Elm_Map_Overlay      Elm_Map_Overlay;      /**< A overlay to be shown in a specific point of the map. This can be created by elm_map_overlay_add() and similar functions and deleted by elm_map_overlay_del(). */

typedef Evas_Object               *(*Elm_Map_Marker_Get_Func)(Evas_Object *obj, Elm_Map_Marker *marker, void *data); /**< Bubble content fetching class function for marker classes. When the user click on a marker, a bubble is displayed with a content. */
typedef void                       (*Elm_Map_Marker_Del_Func)(Evas_Object *obj, Elm_Map_Marker *marker, void *data, Evas_Object *o); /**< Function to delete bubble content for marker classes. */
typedef Evas_Object               *(*Elm_Map_Marker_Icon_Get_Func)(Evas_Object *obj, Elm_Map_Marker *marker, void *data); /**< Icon fetching class function for marker classes. */
typedef Evas_Object               *(*Elm_Map_Group_Icon_Get_Func)(Evas_Object *obj, void *data); /**< Icon fetching class function for markers group classes. */

typedef void                       (*Elm_Map_Overlay_Get_Cb)(void *data, Evas_Object *map, Elm_Map_Overlay *overlay);   /**< Get callback function for the overlay. */
typedef void                       (*Elm_Map_Overlay_Del_Cb)(void *data, Evas_Object *map, Elm_Map_Overlay *overlay);   /**< Det callback function for the overlay. @since 1.7 */
typedef void                       (*Elm_Map_Name_Cb)(void *data, Evas_Object *map, Elm_Map_Name *name);                /**< Async-callback function for the name request. */
typedef void                       (*Elm_Map_Name_List_Cb)(void *data, Evas_Object *map, Eina_List *name_list);                /**< Async-callback function for the name list request. */
typedef void                       (*Elm_Map_Route_Cb)(void *data, Evas_Object *map, Elm_Map_Route *route);             /**< Async-callback function for the route request. */

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
 * It will respect limits defined by elm_map_zoom_min_set() and
 * elm_map_zoom_max_set().
 *
 * By default these values are 0 (world map) and 18 (maximum zoom).
 *
 * This function should be used when zoom mode is set to #ELM_MAP_ZOOM_MODE_MANUAL.
 * This is the default mode, and can be set with elm_map_zoom_mode_set().
 *
 * @see elm_map_zoom_mode_set()
 * @see elm_map_zoom_get()
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
 * @param mode The zoom mode of the map, being it one of #ELM_MAP_ZOOM_MODE_MANUAL
 * (default), #ELM_MAP_ZOOM_MODE_AUTO_FIT, or #ELM_MAP_ZOOM_MODE_AUTO_FILL.
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
 * @return The zoom mode of the map, being it one of #ELM_MAP_ZOOM_MODE_MANUAL
 * (default), #ELM_MAP_ZOOM_MODE_AUTO_FIT, or #ELM_MAP_ZOOM_MODE_AUTO_FILL.
 *
 * This function returns the current zoom mode used by the map object.
 *
 * @see elm_map_zoom_mode_set() for more details.
 *
 * @ingroup Map
 */
EAPI Elm_Map_Zoom_Mode     elm_map_zoom_mode_get(const Evas_Object *obj);

/**
 * Set the minimum zoom of the source.
 *
 * @param obj The map object.
 * @param zoom New minimum zoom value to be used.
 *
 * @see elm_map_zoom_min_get() for details.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_zoom_min_set(Evas_Object *obj, int zoom);

/**
 * Get the minimum zoom of the source.
 *
 * @param obj The map object.
 * @return Returns the minimum zoom of the source.
 *
 * @see elm_map_zoom_min_set() for details.
 *
 * @ingroup Map
 */
EAPI int                   elm_map_zoom_min_get(const Evas_Object *obj);

/**
 * Set the maximum zoom of the source.
 *
 * @param obj The map object.
 * @param zoom New maximum zoom value to be used.
 *
 * @see elm_map_zoom_max_get() for details.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_zoom_max_set(Evas_Object *obj, int zoom);

/**
 * Get the maximum zoom of the source.
 *
 * @param obj The map object.
 * @return Returns the maximum zoom of the source.
 *
 * @see elm_map_zoom_max_set() for details.
 *
 * @ingroup Map
 */
EAPI int                   elm_map_zoom_max_get(const Evas_Object *obj);

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
 * @ingroup Map
 */
EAPI void                  elm_map_region_get(const Evas_Object *obj, double *lon, double *lat);

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
 * @ingroup Map
 */
EAPI void                  elm_map_region_bring_in(Evas_Object *obj, double lon, double lat);

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
 * @ingroup Map
 */
EAPI void                  elm_map_region_show(Evas_Object *obj, double lon, double lat);

/**
 * Convert canvas coordinates into geographic coordinates
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
 * see elm_map_region_to_canvas_convert()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_canvas_to_region_convert(const Evas_Object *obj, const Evas_Coord x, const Evas_Coord y, double *lon, double *lat);

/**
 * Convert geographic coordinates (longitude, latitude)
 * into canvas coordinates.
 *
 * @param obj The map object.
 * @param lon The longitude to convert.
 * @param lat The latitude to convert.
 * @param x   A pointer to horizontal coordinate.
 * @param y   A pointer to vertical coordinate.
 *
 * This gets canvas x, y coordinates from longitude and latitude. The canvas
 * coordinates mean x, y coordinate from current viewport.
 *
 * see elm_map_canvas_to_region_convert()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_region_to_canvas_convert(const Evas_Object *obj, double lon, double lat, Evas_Coord *x, Evas_Coord *y);

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
 * it is not.
 *
 * This gets the current paused state for the map object.
 *
 * @see elm_map_paused_set() for details.
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_paused_get(const Evas_Object *obj);

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
 * @param degree Pointer to store degrees from 0.0 to 360.0
 * to rotate around Z axis.
 * @param cx Pointer to store rotation's center horizontal position.
 * @param cy Pointer to store rotation's center vertical position.
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
 * it is enabled.
 *
 * Mouse wheel can be used for the user to zoom in or zoom out the map.
 *
 * @see elm_map_wheel_disabled_set() for details.
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_wheel_disabled_get(const Evas_Object *obj);

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
 * Add a new overlay to the map object. This overlay has a default type.
 *
 * @param obj The map object to add a new overlay.
 * @param lon The longitude of the overlay.
 * @param lat The latitude of the overlay.
 * @return The created overlay or @c NULL upon failure.
 *
 * A overlay will be created and shown in a specific point of the map, defined
 * by @p lon and @p lat.
 *
 * The created overlay has a default style layout before content or
 * icon is set.
 * If content or icon is set, those are displayed instead of default style
 * layout.
 * You can set by using elm_map_overlay_content_set() or
 * elm_map_overlay_icon_set(). If NULL is set, default style
 * is shown again.
 *
 * Overlay created with this method can be deleted by elm_map_overlay_del().
 *
 * @see elm_map_overlay_del()
 * @see elm_map_overlay_class_add()
 * @see elm_map_overlay_bubble_add()
 * @see elm_map_overlay_content_set()
 * @see elm_map_overlay_icon_set()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_add(Evas_Object *obj, double lon, double lat);

/**
 * Return all overlays in the map object.
 *
 * @param obj The map object to return overlays.
 * @return The list of all overlays or @c NULL upon failure.
 *
 * This list includes group overlays also.
 * So this can be changed dynamically while zooming and panning.
 *
 * @since 1.7
 *
 * @ingroup Map
 */
EAPI EAPI Eina_List *      elm_map_overlays_get(Evas_Object *obj);

/**
 * Delete a overlay from the map. This function can delete all types
 * of overlays.
 *
 * @param overlay The overlay to be deleted.
 *
 * @see elm_map_overlay_add()
 * @see elm_map_overlay_class_add()
 * @see elm_map_overlay_bubble_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_del(Elm_Map_Overlay *overlay);

/**
 * Get the overlay type.
 *
 * @param overlay The overlay to return type.
 * @return Return the overlay type.
 *
 * This type is resolved when the overlay is created.
 *
 * @see elm_map_overlay_add()
 * @see elm_map_overlay_class_add()
 * @see elm_map_overlay_bubble_add()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay_Type  elm_map_overlay_type_get(const Elm_Map_Overlay *overlay);

 /**
 * Set a pointer of user data for a overlay.
 *
 * @param overlay The overlay to own the user data.
 * @param data A pointer of user data
 *
 * @see elm_map_overlay_data_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_data_set(Elm_Map_Overlay *overlay, void *data);

/**
 * Get the user data stored on a overlay.
 *
 * @param overlay The overlay to return the user data.
 * @return A pointer to data stored using elm_map_overlay_data_set(),
 *         or @c NULL, if none has been set.
 *
 * @see elm_map_overlay_data_set()
 *
 * @ingroup Map
 */
EAPI void *                elm_map_overlay_data_get(const Elm_Map_Overlay *overlay);

/**
 * Set if the overlay is hidden or not.
 *
 * @param overlay The overlay to be hidden.
 * @param hide Use @c EINA_TRUE to hide the overlay or @c EINA_FALSE to show.
 *
 * @see elm_map_overlay_hide_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_hide_set(Elm_Map_Overlay *overlay, Eina_Bool hide);

/**
 * Get a value whether the overlay is hidden or not.
 *
 * @param overlay The overlay to return the hidden state.
 * @return @c EINA_TRUE means the overlay is hidden. @c EINA_FALSE indicates
 * it is not.
 *
 * This gets the current hidden state for the overlay.
 *
 * @see elm_map_overlay_hide_set()
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_overlay_hide_get(const Elm_Map_Overlay *overlay);

/**
 * Set the minimum zoom from where the overlay is displayed.
 *
 * @param overlay The overlay to be set the minimum zoom.
 * @param zoom The minimum zoom.
 *
 * The overlay only will be displayed when the map is displayed at @p zoom
 * or bigger.
 *
 * @see elm_map_overlay_displayed_zoom_min_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_displayed_zoom_min_set(Elm_Map_Overlay *overlay, int zoom);

/**
 * Get the minimum zoom from where the overlay is displayed.
 *
 * @param overlay The overlay to return the minimum zoom.
 * @return zoom The minimum zoom.
 *
 * @see elm_map_overlay_displayed_zoom_min_set()
 *
 * @ingroup Map
 */
EAPI int                   elm_map_overlay_displayed_zoom_min_get(const Elm_Map_Overlay *overlay);

/**
 * Pause or unpause the overlay.
 *
 * @param overlay The overlay to be paused.
 * @param paused Use @c EINA_TRUE to pause the @p overlay or @c EINA_FALSE
 * to unpause it.
 *
 * This sets the paused state to on (@c EINA_TRUE) or off (@c EINA_FALSE)
 * for the overlay.
 *
 * The default is off.
 *
 * This will stop moving the overlay coordinates instantly.
 * even if map being scrolled or zoomed.
 *
 * @see elm_map_overlay_paused_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_paused_set(Elm_Map_Overlay *overlay, Eina_Bool paused);

/**
 * Get a value whether the overlay is paused or not.
 *
 * @param overlay The overlay to return paused state.
 * @return @c EINA_TRUE means overlay is paused. @c EINA_FALSE indicates
 * it is not.
 *
 * This gets the current paused state for the overlay.
 *
 * @see elm_map_overlay_paused_set()
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_overlay_paused_get(const Elm_Map_Overlay *overlay);

/**
 * Get a value whether the overlay is visible or not.
 *
 * @param overlay The overlay to return visible state.
 * @return @c EINA_TRUE means overlay is visible. @c EINA_FALSE indicates
 * it is not.
 *
 * The visible of the overlay can not be set.
 * This value can be changed dynamically while zooming and panning
 *
 * @since 1.7
 *
 * @ingroup Map
 */
EAPI Eina_Bool             elm_map_overlay_visible_get(const Elm_Map_Overlay *overlay);

/**
 * Set the content object of the overlay.
 *
 * @param overlay The overlay to be set the content.
 * @param obj The evas object will be used to display the overlay.
 *
 * Only default and class type overlay support this function.
 *
 * The content should be resized or set size hints before set to the overlay.
 * <b> Do not modify this object</b> (move, show, hide, del, etc.),
 * after set.
 * You can only resize this.
 *
 * This content is what will be inside the overlay that will be displayed.
 * If a content is set, icon and default style layout are no more used before
 * the content is deleted.
 *
 * If @p obj is @c NULL, content inside the overlay is deleted.
 *
 * @see elm_map_overlay_content_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_content_set(Elm_Map_Overlay *overlay, Evas_Object *obj);

/**
 * Get the content object.
 *
 * @param overlay The overlay to return the content.
 * @return Return the evas object if it exists, else @c NULL.
 *
 * Only default and class type overlay support this function.
 *
 * Returned content is what being inside the overlay that being displayed.
 *
 * <b> Do not modify this object</b> (move, show, hide, del, etc.).
 * You can only resize this.
 *
 * The content can be set by elm_map_overlay_content_set().
 *
 * @see elm_map_overlay_content_set()
 *
 * @ingroup Map
 */
EAPI const Evas_Object *   elm_map_overlay_content_get(const Elm_Map_Overlay *overlay);

/**
 * Set a icon of the overlay.
 *
 * @param overlay The overlay to be set the icon.
 * @param icon The icon will be used to display the overlay.
 *
 * Only default and class type overlay support this function.
 *
 * <b> Do not modify this object</b> (move, show, hide, resize, del, etc.),
 * after set.
 *
 * If icon is set, default style layout will not be used.
 *
 * If @p icon is @c NULL, icon inside the overlay will be deleted.
 *
 * @see elm_map_overlay_icon_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_icon_set(Elm_Map_Overlay *overlay, Evas_Object *icon);

/**
 * Get the icon object.
 *
 * @param overlay The overlay to return the icon.
 * @return Return the icon object if it exists, else @c NULL.
 *
 * Only default and class type overlay support this function.
 *
 * Returned icon is what being inside the overlay that being displayed.
 *
 * <b> Do not modify this icon </b> (move, show, hide, resize, del, etc.).
 *
 * The icon can be set by elm_map_overlay_icon_set().
 *
 * @see elm_map_overlay_icon_set()
 *
 * @ingroup Map
 */
EAPI const Evas_Object *   elm_map_overlay_icon_get(const Elm_Map_Overlay *overlay);

/**
 * Set the geographic coordinates of the overlay.
 *
 * @param overlay The overlay to be set geographic coordinates.
 * @param lon Longitude to be set.
 * @param lat Latitude to be set.
 *
 * Only default and bubble type overlay support this function.
 *
 * This sets the center coordinates of the overlay. It can be
 * get by elm_map_overlay_region_get().
 *
 * @see elm_map_overlay_region_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_region_set(Elm_Map_Overlay *overlay, double lon, double lat);

/**
 * Get the geographic coordinates of the overlay.
 *
 * @param overlay The overlay to return geographic coordinates.
 * @param lon Pointer to store longitude.
 * @param lat Pointer to store latitude.
 *
 * Only default and bubble type overlay support this function.
 *
 * This returns the center coordinates of the overlay. It can be
 * set by elm_map_overlay_region_set().
 *
 * @see elm_map_overlay_region_set()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_region_get(const Elm_Map_Overlay *overlay, double *lon, double *lat);


/**
 * Set the object color of the overlay.
 *
 * @param overlay The overlay to be set color.
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
 * This sets the color used for the overlay. By default, it is set to
 * solid red (r = 255, g = 0, b = 0, a = 255).
 *
 * For alpha channel, 0 represents completely transparent, and 255, opaque.
 *
 * Function supports only ELM_MAP_OVERLAY_TYPE_CLASS, ELM_MAP_OVERLAY_TYPE_DEFAULT
 * and ELM_MAP_OVERLAY_TYPE_ROUTE Elm_Map_Overlay_Type types.
 *
 * @see elm_map_overlay_color_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_color_set(Elm_Map_Overlay *overlay, int r, int g, int b, int a);

/**
 * Get the object color of the overlay.
 *
 * @param overlay The overlay to return color.
 * @param r Pointer to store the red channel value.
 * @param g Pointer to store the green channel value.
 * @param b Pointer to store the blue channel value.
 * @param a Pointer to store the alpha channel value.
 *
 * @see elm_map_overlay_color_set()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_color_get(const Elm_Map_Overlay *overlay, int *r, int *g, int *b, int *a);

/**
 * Show the given overlay at the center of the map, immediately.
 *
 * @param overlay The overlay to be center at.
 *
 * This causes map to @b redraw its viewport's contents to the
 * region containing the given @p overlay's coordinates, that will be
 * moved to the center of the map.
 *
 * @see elm_map_overlays_show() if more than one overlay need to be displayed.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_show(Elm_Map_Overlay *overlay);

/**
 * Move and zoom the map to display a list of overlays.
 *
 * @param overlays A list of #Elm_Map_Overlay handles.
 *
 * The map will be centered on the center point of the overlays in the list.
 * Then the map will be zoomed in order to fit the overlays using the maximum
 * zoom which allows display of all the overlays.
 *
 * @warning All the overlays should belong to the same map object.
 *
 * @see elm_map_overlay_show() to show a single overlay.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlays_show(Eina_List *overlays);

/**
 * Set the get callback function of the overlay.
 *
 * @param overlay The overlay to own the get callback function.
 * @param get_cb The callback function.
 * @param data The user callback data.
 *
 * If the overlay is clicked, the callback wll be called.
 * The clicked overlay is returned by callback.
 *
 * You can add callback to the class overlay. If one of the group overlays in this class
 * is clicked, callback will be called and return a virtual group overlays.
 *
 * You can delete this callback function by setting @c NULL.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_get_cb_set(Elm_Map_Overlay *overlay, Elm_Map_Overlay_Get_Cb get_cb, void *data);

/**
 * Set the get callback function to call when the overlay is deleted.
 *
 * @param overlay The overlay to own the del callback function.
 * @param get_cb The callback function.
 * @param data The user callback data.
 *
 * If the overlay is deleted, the callback wll be called.
 * The deleted overlay is returned by callback.
 *
 * You can delete this callback function by setting @c NULL.
 *
 * @since 1.7
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_del_cb_set(Elm_Map_Overlay *overlay, Elm_Map_Overlay_Del_Cb del_cb, void *data);

/**
 * Add a new class overlay to the map object.
 * This overlay has a class type.
 *
 * @param obj The map object to add a new overlay.
 * @return The created overlay or @c NULL upon failure.
 *
 * This overlay is not shown before overlay members are appended.
 * if overlay members in the same class are close, group overlays
 * are created. If they are far away, group overlays are hidden.
 * When group overlays are shown, they have default style layouts at first.
 *
 * You can change the state (hidden, paused, etc.) or set the content
 * or icon of the group overlays by chaning the state of the class overlay.
 * Do not modify the group overlay itself.
 *
 * Also these changes have a influence on the overlays in the same class
 * even if each overlay is alone and is not grouped.
 *
 * @see elm_map_overlay_del()
 * @see elm_map_overlay_add()
 * @see elm_map_overlay_bubble_add()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_class_add(Evas_Object *obj);

/**
 * Add a new overlay member to the class overlay.
 *
 * @param clas The class overlay to add a new overlay.
 * @param overlay The overlay to be added to the class overlay.
 *
 * @see elm_map_overlay_class_remove()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_class_append(Elm_Map_Overlay *clas, Elm_Map_Overlay *overlay);

/**
 * Remove a overlay from the class.
 *
 * @param clas The class overlay to delete the overlay.
 * @param overlay The overlay to be deleted from the class overlay.
 *
 * @see elm_map_overlay_class_append()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_class_remove(Elm_Map_Overlay *clas, Elm_Map_Overlay *overlay);

/**
 * Set the maximum zoom from where the overlay members in the class can be
 * grouped.
 *
 * @param clas The overlay class has overlay members.
 * @param zoom The maximum zoom.
 *
 * Overlay members in the class only will be grouped when the map
 * is displayed at less than @p zoom.
 *
 * @see elm_map_overlay_class_zoom_max_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_class_zoom_max_set(Elm_Map_Overlay *clas, int zoom);

/**
 * Get the maximum zoom from where the overlay members in the class can be
 * grouped.
 *
 * @param clas The overlay class has overlay members.
 *
 * @return The maximum zoom.
 *
 * @see elm_map_overlay_class_zoom_max_set()
 *
 * @ingroup Map
 */
EAPI int                   elm_map_overlay_class_zoom_max_get(const Elm_Map_Overlay *clas);

/**
 * Get the overlay members of the group overlay.
 *
 * @param grp The group overlay has overlay members.
 *
 * @return The list of group overlay members.
 *
 * The group overlays are virtualy overlays. Those are shown and hidden dynamically.
 * You can add callback to the class overlay. If one of the group overlays in this class
 * is clicked, callback will be called and return a virtual group overlays.
 *
 * You can change the state (hidden, paused, etc.) or set the content
 * or icon of the group overlays by chaning the state of the class overlay.
 * Do not modifty the group overlay itself.
 *
 * @see elm_map_overlay_class_add()
 *
 * @ingroup Map
 */
EAPI Eina_List *      elm_map_overlay_group_members_get(const Elm_Map_Overlay *grp);

/**
 * Add a new bubble overlay to the map object.
 * This overlay has a bubble type.
 *
 * @param obj The map object to add a new overlay.
 * @return The created overlay or @c NULL upon failure.
 *
 * A bubble will not be displayed before geographic coordinates are set or
 * any other overlays are followed.
 *
 * This overlay has a bubble style layout and icon or content can not
 * be set.
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_del()
 * @see elm_map_overlay_add()
 * @see elm_map_overlay_class_add()
 * @see elm_map_overlay_region_set()
 * @see elm_map_overlay_bubble_follow()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_bubble_add(Evas_Object *obj);

/**
 * Follow a other overlay.
 *
 * @param bubble The bubble overlay to follow a parent overlay.
 * @param parent The parent overlay to be followed by the bubble overlay.
 *
 * Bubble overlay will follow the parent overlay's movement (hide, show, move).
 *
 * @see elm_map_overlay_bubble_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_bubble_follow(Elm_Map_Overlay *bubble, const Elm_Map_Overlay *parent);

/**
 * Add a content object to the bubble overlay.
 *
 * @param bubble The bubble overlay to add a content.
 * @param content The content to be added to the bubble overlay.
 *
 * Added contents will be displayed inside the bubble overlay.
 *
 * @see elm_map_overlay_bubble_content_clear()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_bubble_content_append(Elm_Map_Overlay *bubble, Evas_Object *content);

/**
 * Clear all contents inside the bubble overlay.
 *
 * @param bubble The bubble overlay to clear the contents.
 *
 * This will delete all contents inside the bubble overlay.
 *
 * @see elm_map_overlay_bubble_content_append()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_overlay_bubble_content_clear(Elm_Map_Overlay *bubble);

/**
 * Add a new route overlay to the map object.
 * This overlay has a route type.
 *
 * @param obj The map object to add a new overlay.
 * @param route The route object to make a overlay.
 * @return The created overlay or @c NULL upon failure.
 *
 * This overlay has a route style layout and icon or content can not
 * be set.
 *
 * The color scheme can be changed by elm_map_overlay_content_set().
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_del()
 * @see elm_map_overlay_class_add()
 * @see elm_map_overlay_content_set()
 * @see elm_map_overlay_content_get()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_route_add(Evas_Object *obj, const Elm_Map_Route *route);

/**
 * Add a new line overlay to the map object.
 * This overlay has a line type.
 *
 * @param obj The map object to add a new overlay.
 * @param flon The start longitude.
 * @param flat The start latitude.
 * @param tlon The destination longitude.
 * @param tlat The destination latitude.
 * @return The created overlay or @c NULL upon failure.
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_del()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_line_add(Evas_Object *obj, double flon, double flat, double tlon, double tlat);

/**
 * Add a new polygon overlay to the map object.
 * This overlay has a polygon type.
 *
 * @param obj The map object to add a new overlay.
 * @return The created overlay or @c NULL upon failure.
 *
 * At least 3 regions should be added to show the polygon overlay.
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_polygon_region_add()
 * @see elm_map_overlay_del()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_polygon_add(Evas_Object *obj);

/**
 * Add a  geographic coordinates  to the polygon overlay.
 *
 * @param overlay The polygon overlay to get a region.
 * @param lon The longitude.
 * @param lat The latitude.
 *
 * At least 3 regions should be added to show the polygon overlay.
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_polygon_add()
 * @see elm_map_overlay_del()
 *
 * @ingroup Map
 */
EAPI void                             elm_map_overlay_polygon_region_add(Elm_Map_Overlay *overlay, double lon, double lat);

/**
 * Add a new circle overlay to the map object.
 * This overlay has a circle type.
 *
 * @param obj The map object to add a new overlay.
 * @param lon The center longitude.
 * @param lat The center latitude.
 * @param radius The pixel length of radius.
 * @return The created overlay or @c NULL upon failure.
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_del()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_circle_add(Evas_Object *obj, double lon, double lat, double radius);

/**
 * Add a new scale overlay to the map object.
 * This overlay has a scale type.
 *
 * @param obj The map object to add a new overlay.
 * @param x  horizontal pixel coordinate.
 * @param y  vertical pixel coordinate
 * @return The created overlay or @c NULL upon failure.
 *
 * The scale overlay shows the ratio of a distance on the map to the corresponding distance.
 *
 * Overlay created with this method can be deleted with elm_map_overlay_del().
 *
 * @see elm_map_overlay_del()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Overlay *     elm_map_overlay_scale_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/**
 * Get the information of tile load status.
 *
 * @param obj The map object.
 * @param try_num Pointer to store number of tiles download requested.
 * @param finish_num Pointer to store number of tiles successfully downloaded.
 *
 * This gets the current tile loaded status for the map object.
 *
 * @ingroup Map
 */
EAPI void                  elm_map_tile_load_status_get(const Evas_Object *obj, int *try_num, int *finish_num);

/**
 * Get the names of available sources for a specific type.
 *
 * @param obj The map object.
 * @param type source type.
 * @return The char pointer array of source names.
 *
 * It will provide a list with all available sources.
 * Current source can be set by elm_map_source_set(), or get with
 * elm_map_source_get().
 *
 * At least available sources of tile type:
 * @li "Mapnik"
 * @li "Osmarender"
 * @li "CycleMap"
 * @li "Maplint"
 *
 * At least available sources of route type:
 * @li "Yours"
 *
 * At least available sources of name type:
 * @li "Nominatim"
 *
 * @see elm_map_source_set()
 * @see elm_map_source_get()
 *
 * @ingroup Map
 */
EAPI const char          **elm_map_sources_get(const Evas_Object *obj, Elm_Map_Source_Type type);

/**
 * Set the current source of the map for a specific type.
 *
 * @param obj The map object.
 * @param type source type.
 * @param source_name The source to be used.
 *
 * Map widget retrieves tile images that composes the map from a web service.
 * This web service can be set with this method
 * for ELM_MAP_SOURCE_TYPE_TILE type.
 * A different service can return a different maps with different
 * information and it can use different zoom values.
 *
 * Map widget provides route data based on a external web service.
 * This web service can be set with this method
 * for ELM_MAP_SOURCE_TYPE_ROUTE type.
 *
 * Map widget also provide geoname data based on a external web service.
 * This web service can be set with this method
 * for ELM_MAP_SOURCE_TYPE_NAME type.
 *
 * The @p source_name need to match one of the names provided by
 * elm_map_sources_get().
 *
 * The current source can be get using elm_map_source_get().
 *
 * @see elm_map_sources_get()
 * @see elm_map_source_get()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_source_set(Evas_Object *obj, Elm_Map_Source_Type type, const char *source_name);

/**
 * Get the name of currently used source for a specific type.
 *
 * @param obj The map object.
 * @param type source type.
 * @return Returns the name of the source in use.
 *
 * @see elm_map_sources_get()
 * @see elm_map_source_set()
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_source_get(const Evas_Object *obj, Elm_Map_Source_Type type);

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
 * @param route_cb The route to be traced.
 * @param data A pointer of user data.
 *
 * @return The created route or @c NULL upon failure.
 *
 * A route will be traced by point on coordinates (@p flat, @p flon)
 * to point on coordinates (@p tlat, @p tlon), using the route service
 * set with elm_map_source_set().
 *
 * It will take @p type on consideration to define the route,
 * depending if the user will be walking or driving, the route may vary.
 * One of #ELM_MAP_ROUTE_TYPE_MOTOCAR, #ELM_MAP_ROUTE_TYPE_BICYCLE,
 * or #ELM_MAP_ROUTE_TYPE_FOOT need to be used.
 *
 * Another parameter is what the route should prioritize, the minor distance
 * or the less time to be spend on the route. So @p method should be one
 * of #ELM_MAP_ROUTE_METHOD_SHORTEST or #ELM_MAP_ROUTE_METHOD_FASTEST.
 *
 * Routes created with this method can be deleted with
 * elm_map_route_del(),
 * and distance can be get with elm_map_route_distance_get().
 *
 * @see elm_map_route_del()
 * @see elm_map_route_distance_get()
 * @see elm_map_source_set()
 *
 * @ingroup Map
 */
EAPI Elm_Map_Route        *elm_map_route_add(Evas_Object *obj, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data);

/**
 * Remove a route from the map.
 *
 * @param route The route to remove.
 *
 * @see elm_map_route_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_route_del(Elm_Map_Route *route);

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
 * Request a address or geographic coordinates(longitude, latitude)
 * from a given address or geographic coordinate(longitude, latitude).
 *
 * @param obj The map object.
 * @param address The address.
 * @param lon The longitude.
 * @param lat The latitude.
 * @param name_cb The callback function.
 * @param data The user callback data.
 * @return name A #Elm_Map_Name handle for this coordinate.
 *
 * If you want to get address from geographic coordinates, set input @p address
 * as @c NULL and set @p lon, @p lat as you want to convert.
 * If address is set except NULL, @p lon and @p lat are checked.
 *
 * To get the string for this address, elm_map_name_address_get()
 * should be used after callback or "name,loaded" signal is called.
 *
 * To get the longitude and latitude, elm_map_name_region_get()
 * should be used.
 *
 * @ingroup Map
 */
EAPI Elm_Map_Name         *elm_map_name_add(const Evas_Object *obj, const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data);

/**
 * Request a list of addresses corresponding to a given name
 *
 * @param obj The map object.
 * @param address The address.
 * @param name_cb The callback function.
 * @param data The user callback data.
 *
 * If you want to search address from a name.
 *
 * @since 1.8
 *
 * @ingroup Map
 */
EAPI void                  elm_map_name_search(const Evas_Object *obj, const char *address, Elm_Map_Name_List_Cb name_cb, void *data);

/**
 * Get the address of the name.
 *
 * @param name The name handle.
 * @return Returns the address string of @p name.
 *
 * This gets the coordinates of the @p name, created with one of the
 * conversion functions.
 *
 * @see elm_map_name_add()
 *
 * @ingroup Map
 */
EAPI const char           *elm_map_name_address_get(const Elm_Map_Name *name);

/**
 * Get the current coordinates of the name.
 *
 * @param name The name handle.
 * @param lat Pointer to store the latitude.
 * @param lon Pointer to store The longitude.
 *
 * This gets the coordinates of the @p name, created with one of the
 * conversion functions.
 *
 * @see elm_map_name_add()
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
 * @see elm_map_name_add()
 *
 * @ingroup Map
 */
EAPI void                  elm_map_name_del(Elm_Map_Name *name);

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
EAPI Evas_Object          *elm_map_track_add(Evas_Object *obj, void *emap);

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
