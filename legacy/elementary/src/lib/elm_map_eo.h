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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
 */
#define elm_obj_map_overlay_scale_add(x, y, ret) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_OVERLAY_SCALE_ADD), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Elm_Map_Overlay **, ret)

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
 *
 * @ingroup Map
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
 *
 * @ingroup Map
 */
#define elm_obj_map_track_remove(route) ELM_OBJ_MAP_ID(ELM_OBJ_MAP_SUB_ID_TRACK_REMOVE), EO_TYPECHECK(Evas_Object *, route)
