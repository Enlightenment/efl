#ifndef _ELM_MAP_EO_LEGACY_H_
#define _ELM_MAP_EO_LEGACY_H_

#ifndef _ELM_MAP_EO_CLASS_TYPE
#define _ELM_MAP_EO_CLASS_TYPE

typedef Eo Elm_Map;

#endif

#ifndef _ELM_MAP_EO_TYPES
#define _ELM_MAP_EO_TYPES

/**
 * @brief Set type of a external source (provider).
 *
 * See @ref elm_map_sources_get() See @ref elm_map_source_get() See
 * @ref elm_map_source_set()
 *
 * @ingroup Elm_Map
 */
typedef enum
{
  ELM_MAP_SOURCE_TYPE_TILE = 0, /**< Map tile provider. */
  ELM_MAP_SOURCE_TYPE_ROUTE, /**< Route service provider. */
  ELM_MAP_SOURCE_TYPE_NAME, /**< Name service provider. */
  ELM_MAP_SOURCE_TYPE_LAST /**< Sentinel value to indicate last enum field
                            * during iteration */
} Elm_Map_Source_Type;

/**
 * @brief Set type of transport used on route.
 *
 * See @ref elm_map_route_add()
 *
 * @ingroup Elm_Map
 */
typedef enum
{
  ELM_MAP_ROUTE_TYPE_MOTOCAR = 0, /**< Route should consider an automobile will
                                   * be used. */
  ELM_MAP_ROUTE_TYPE_BICYCLE, /**< Route should consider a bicycle will be used
                               * by the user. */
  ELM_MAP_ROUTE_TYPE_FOOT, /**< Route should consider user will be walking. */
  ELM_MAP_ROUTE_TYPE_LAST /**< Sentinel value to indicate last enum field during
                           * iteration */
} Elm_Map_Route_Type;

/**
 * @brief Set the routing method, what should be prioritized, time or distance.
 *
 * See @ref elm_map_route_add()
 *
 * @ingroup Elm_Map
 */
typedef enum
{
  ELM_MAP_ROUTE_METHOD_FASTEST = 0, /**< Route should prioritize time. */
  ELM_MAP_ROUTE_METHOD_SHORTEST, /**< Route should prioritize distance. */
  ELM_MAP_ROUTE_METHOD_LAST /**< Sentinel value to indicate last enum field
                             * during iteration */
} Elm_Map_Route_Method;


#endif

/**
 * @brief Set the minimum zoom of the source.
 *
 * @param[in] obj The object.
 * @param[in] zoom Minimum zoom value to be used.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_zoom_min_set(Elm_Map *obj, int zoom);

/**
 * @brief Get the minimum zoom of the source.
 *
 * @param[in] obj The object.
 *
 * @return Minimum zoom value to be used.
 *
 * @ingroup Elm_Map_Group
 */
EAPI int elm_map_zoom_min_get(const Elm_Map *obj);

/**
 * @brief Rotate the map.
 *
 * @param[in] obj The object.
 * @param[in] degree Angle from 0.0 to 360.0 to rotate around Z axis.
 * @param[in] cx Rotation's center horizontal position.
 * @param[in] cy Rotation's center vertical position.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_rotate_set(Elm_Map *obj, double degree, int cx, int cy);

/**
 * @brief Get the rotate degree of the map.
 *
 * @param[in] obj The object.
 * @param[out] degree Angle from 0.0 to 360.0 to rotate around Z axis.
 * @param[out] cx Rotation's center horizontal position.
 * @param[out] cy Rotation's center vertical position.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_rotate_get(const Elm_Map *obj, double *degree, int *cx, int *cy);

/**
 * @brief Set the user agent used by the map object to access routing services.
 *
 * User agent is a client application implementing a network protocol used in
 * communications within a clientserver distributed computing system
 *
 * The @c user_agent identification string will transmitted in a header field
 * @c User-Agent.
 *
 * @param[in] obj The object.
 * @param[in] user_agent The user agent to be used by the map.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_user_agent_set(Elm_Map *obj, const char *user_agent);

/**
 * @brief Get the user agent used by the map object.
 *
 * @param[in] obj The object.
 *
 * @return The user agent to be used by the map.
 *
 * @ingroup Elm_Map_Group
 */
EAPI const char *elm_map_user_agent_get(const Elm_Map *obj);

/**
 * @brief Set the maximum zoom of the source.
 *
 * @param[in] obj The object.
 * @param[in] zoom Maximum zoom value to be used.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_zoom_max_set(Elm_Map *obj, int zoom);

/**
 * @brief Get the maximum zoom of the source.
 *
 * @param[in] obj The object.
 *
 * @return Maximum zoom value to be used.
 *
 * @ingroup Elm_Map_Group
 */
EAPI int elm_map_zoom_max_get(const Elm_Map *obj);

/**
 * @brief Get the current geographic coordinates of the map.
 *
 * This gets the current center coordinates of the map object. It can be set by
 * @ref elm_map_region_bring_in and @ref elm_map_region_show.
 *
 * @param[in] obj The object.
 * @param[out] lon Pointer to store longitude.
 * @param[out] lat Pointer to store latitude.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_get(const Elm_Map *obj, double *lon, double *lat);

/**
 * @brief Return all overlays in the map object.
 *
 * This list includes group overlays also. So this can be changed dynamically
 * while zooming and panning.
 *
 * @param[in] obj The object.
 *
 * @return The list of all overlays or @c null upon failure.
 *
 * @since 1.7
 *
 * @ingroup Elm_Map_Group
 */
EAPI Eina_List *elm_map_overlays_get(const Elm_Map *obj);

/**
 * @brief Get the information of tile load status.
 *
 * This gets the current tile loaded status for the map object.
 *
 * @param[in] obj The object.
 * @param[out] try_num Pointer to store number of tiles download requested.
 * @param[out] finish_num Pointer to store number of tiles successfully
 * downloaded.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_tile_load_status_get(const Elm_Map *obj, int *try_num, int *finish_num);

/**
 * @brief Set the current source of the map for a specific type.
 *
 * Map widget retrieves tile images that composes the map from a web service.
 * This web service can be set with this method for #ELM_MAP_SOURCE_TYPE_TILE
 * type. A different service can return a different maps with different
 * information and it can use different zoom values.
 *
 * Map widget provides route data based on a external web service. This web
 * service can be set with this method for #ELM_MAP_SOURCE_TYPE_ROUTE type.
 *
 * Map widget also provide geoname data based on a external web service. This
 * web service can be set with this method for #ELM_MAP_SOURCE_TYPE_NAME type.
 *
 * The @c source_name need to match one of the names provided by
 * @ref elm_map_sources_get.
 *
 * The current source can be get using @ref elm_map_source_get.
 *
 * @param[in] obj The object.
 * @param[in] type Source type.
 * @param[in] source_name The source to be used.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_source_set(Elm_Map *obj, Elm_Map_Source_Type type, const char *source_name);

/**
 * @brief Get the name of currently used source for a specific type.
 *
 * @param[in] obj The object.
 * @param[in] type Source type.
 *
 * @return The name of the source in use.
 *
 * @ingroup Elm_Map_Group
 */
EAPI const char *elm_map_source_get(const Elm_Map *obj, Elm_Map_Source_Type type);

/**
 * @brief Add a new route to the map object.
 *
 * A route will be traced by point on coordinates ($flat, @c flon) to point on
 * coordinates ($tlat, @c tlon), using the route service set with
 * @ref elm_map_source_set.
 *
 * It will take @c type on consideration to define the route, depending if the
 * user will be walking or driving, the route may vary. One of
 * #ELM_MAP_ROUTE_TYPE_MOTOCAR, #ELM_MAP_ROUTE_TYPE_BICYCLE, or
 * #ELM_MAP_ROUTE_TYPE_FOOT need to be used.
 *
 * Another parameter is what the route should prioritize, the minor distance or
 * the less time to be spend on the route. So @c method should be one of
 * #ELM_MAP_ROUTE_METHOD_SHORTEST or #ELM_MAP_ROUTE_METHOD_FASTEST.
 *
 * Routes created with this method can be deleted with @ref elm_map_route_del
 * and distance can be get with @ref elm_map_route_distance_get.
 *
 * @param[in] obj The object.
 * @param[in] type The type of transport to be considered when tracing a route.
 * @param[in] method The routing method, what should be prioritized.
 * @param[in] flon The start longitude.
 * @param[in] flat The start latitude.
 * @param[in] tlon The destination longitude.
 * @param[in] tlat The destination latitude.
 * @param[in] route_cb The route to be traced.
 * @param[in] data A pointer of user data.
 *
 * @return The created route or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Route *elm_map_route_add(Elm_Map *obj, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data);

/**
 * @brief Add a track on the map.
 *
 * @param[in] obj The object.
 * @param[in] emap The emap route object.
 *
 * @return The route object. This is an elm object of type Route.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Efl_Canvas_Object *elm_map_track_add(Elm_Map *obj, void *emap);

/**
 * @brief Convert geographic coordinates (longitude, latitude) into canvas
 * coordinates.
 *
 * This gets canvas x, y coordinates from longitude and latitude. The canvas
 * coordinates mean x, y coordinate from current viewport.
 *
 * @param[in] obj The object.
 * @param[in] lon The longitude to convert.
 * @param[in] lat The latitude to convert.
 * @param[out] x A pointer to horizontal coordinate.
 * @param[out] y A pointer to vertical coordinate.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_to_canvas_convert(const Elm_Map *obj, double lon, double lat, int *x, int *y);

/**
 * @brief Add a new circle overlay to the map object. This overlay has a circle
 * type.
 *
 * Overlay created with this method can be deleted with @ref
 * elm_map_overlay_del.
 *
 * @param[in] obj The object.
 * @param[in] lon The center longitude.
 * @param[in] lat The center latitude.
 * @param[in] radius The pixel length of radius.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_circle_add(Elm_Map *obj, double lon, double lat, double radius);

/**
 * @brief Add a new class overlay to the map object. This overlay has a class
 * type.
 *
 * This overlay is not shown before overlay members are appended. if overlay
 * members in the same class are close, group overlays are created. If they are
 * far away, group overlays are hidden. When group overlays are shown, they
 * have default style layouts at first.
 *
 * You can change the state (hidden, paused, etc.) or set the content or icon
 * of the group overlays by chaning the state of the class overlay. Do not
 * modify the group overlay itself.
 *
 * Also these changes have a influence on the overlays in the same class even
 * if each overlay is alone and is not grouped.
 *
 * @param[in] obj The object.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_class_add(Elm_Map *obj);

/**
 * @brief Add a new bubble overlay to the map object. This overlay has a bubble
 * type.
 *
 * A bubble will not be displayed before geographic coordinates are set or any
 * other overlays are followed.
 *
 * This overlay has a bubble style layout and icon or content can not be set.
 *
 * Overlay created with this method can be deleted with @ref
 * elm_map_overlay_del.
 *
 * @param[in] obj The object.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_bubble_add(Elm_Map *obj);

/**
 * @brief Get the names of available sources for a specific type.
 *
 * It will provide a list with all available sources. Current source can be set
 * by @ref elm_map_source_set, or get with @ref elm_map_source_get.
 *
 * At least available sources of tile type are "Mapnik", "Osmarender",
 * "CycleMap" and "Maplint".
 *
 * At least available sources of route type are "Yours".
 *
 * At least available sources of name type are "Nominatim".
 *
 * @param[in] obj The object.
 * @param[in] type Source type.
 *
 * @return The char pointer array of source names.
 *
 * @ingroup Elm_Map_Group
 */
EAPI const char **elm_map_sources_get(const Elm_Map *obj, Elm_Map_Source_Type type);

/**
 * @brief Add a new polygon overlay to the map object. This overlay has a
 * polygon type.
 *
 * At least 3 regions should be added to show the polygon overlay.
 *
 * Overlay created with this method can be deleted with @ref
 * elm_map_overlay_del.
 *
 * @param[in] obj The object.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_polygon_add(Elm_Map *obj);

/**
 * @brief Add a new line overlay to the map object. This overlay has a line
 * type.
 *
 * Overlay created with this method can be deleted with @ref
 * elm_map_overlay_del.
 *
 * @param[in] obj The object.
 * @param[in] flon The start longitude.
 * @param[in] flat The start latitude.
 * @param[in] tlon The destination longitude.
 * @param[in] tlat The destination latitude.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_line_add(Elm_Map *obj, double flon, double flat, double tlon, double tlat);

/**
 * @brief Show the given coordinates at the center of the map, immediately.
 *
 * This causes map to redraw its viewport's contents to the region containing
 * the given @c lat and @c lon, that will be moved to the center of the map.
 *
 * See @ref elm_map_region_bring_in for a function to move with animation.
 *
 * @param[in] obj The object.
 * @param[in] lon Longitude to center at.
 * @param[in] lat Latitude to center at.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_show(Elm_Map *obj, double lon, double lat);

/**
 * @brief Request a address or geographic coordinates(longitude, latitude) from
 * a given address or geographic coordinate(longitude, latitude).
 *
 * If you want to get address from geographic coordinates, set input @c address
 * as @c null and set @c lon, @c lat as you want to convert. If address is set
 * except NULL, @c lon and @c lat are checked.
 *
 * To get the string for this address, @ref elm_map_name_address_get should be
 * used after callback or "name,loaded" signal is called.
 *
 * To get the longitude and latitude, @ref elm_map_region_get should be used.
 *
 * @param[in] obj The object.
 * @param[in] address The address.
 * @param[in] lon The longitude.
 * @param[in] lat The latitude.
 * @param[in] name_cb The callback function.
 * @param[in] data The user callback data.
 *
 * @return A #Elm_Map_Name handle for this coordinate.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Name *elm_map_name_add(const Elm_Map *obj, const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data);

/**
 * @brief Requests a list of addresses corresponding to a given name.
 *
 * @internal
 *
 * @param[in] obj The object.
 * @param[in] address The address.
 * @param[in] name_cb The callback function.
 * @param[in] data The user callback data.
 *
 * @since 1.8
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_name_search(const Elm_Map *obj, const char *address, Elm_Map_Name_List_Cb name_cb, void *data);

/**
 * @brief Animatedly bring in given coordinates to the center of the map.
 *
 * This causes map to jump to the given @c lat and @c lon coordinates and show
 * it (by scrolling) in the center of the viewport, if it is not already
 * centered. This will use animation to do so and take a period of time to
 * complete.
 *
 * See @ref elm_map_region_show for a function to avoid animation.
 *
 * @param[in] obj The object.
 * @param[in] lon Longitude to center at.
 * @param[in] lat Latitude to center at.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_bring_in(Elm_Map *obj, double lon, double lat);

/**
 * @brief Animatedly set the zoom level of the map and bring in given
 * coordinates to the center of the map.
 *
 * This causes map to zoom into specific zoom level and also move to the given
 * @c lat and @c lon coordinates and show it (by scrolling) in the center of
 * the viewport concurrently.
 *
 * See also @ref elm_map_region_bring_in.
 *
 * @param[in] obj The object.
 * @param[in] zoom The zoom level to set.
 * @param[in] lon Longitude to center at.
 * @param[in] lat Latitude to center at.
 *
 * @since 1.11
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_zoom_bring_in(Elm_Map *obj, int zoom, double lon, double lat);

/**
 * @brief Remove a track from the map.
 *
 * @param[in] obj The object.
 * @param[in] route The track to remove.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_track_remove(Elm_Map *obj, Efl_Canvas_Object *route);

/**
 * @brief Add a new route overlay to the map object. This overlay has a route
 * type.
 *
 * This overlay has a route style layout and icon or content can not be set.
 *
 * The color scheme can be changed by @ref elm_map_overlay_content_set.
 *
 * Overlay created with this method can be deleted with @ref
 * elm_map_overlay_del.
 *
 * @param[in] obj The object.
 * @param[in] route The route object to make a overlay.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_route_add(Elm_Map *obj, const Elm_Map_Route *route);

/**
 * @brief Add a new scale overlay to the map object. This overlay has a scale
 * type.
 *
 * The scale overlay shows the ratio of a distance on the map to the
 * corresponding distance.
 *
 * Overlay created with this method can be deleted with @ref
 * elm_map_overlay_del.
 *
 * @param[in] obj The object.
 * @param[in] x horizontal pixel coordinate.
 * @param[in] y vertical pixel coordinate.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_scale_add(Elm_Map *obj, int x, int y);

/**
 * @brief Add a new overlay to the map object. This overlay has a default type.
 *
 * A overlay will be created and shown in a specific point of the map, defined
 * by @c lon and @c lat.
 *
 * The created overlay has a default style layout before content or icon is
 * set. If content or icon is set, those are displayed instead of default style
 * layout.
 *
 * You can set by using @ref elm_map_overlay_content_set or @ref
 * elm_map_overlay_icon_set. If @c null is set, default style is shown again.
 *
 * Overlay created with this method can be deleted by @ref elm_map_overlay_del.
 *
 * @param[in] obj The object.
 * @param[in] lon The longitude of the overlay.
 * @param[in] lat The latitude of the overlay.
 *
 * @return The created overlay or @c null upon failure.
 *
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_add(Elm_Map *obj, double lon, double lat);

/**
 * @brief Convert canvas coordinates into geographic coordinates (longitude,
 * latitude).
 *
 * This gets longitude and latitude from canvas x, y coordinates. The canvas
 * coordinates mean x, y coordinate from current viewport.
 *
 * @param[in] obj The object.
 * @param[in] x Horizontal coordinate of the point to convert.
 * @param[in] y Vertical coordinate of the point to convert.
 * @param[out] lon A pointer to the longitude.
 * @param[out] lat A pointer to the latitude.
 *
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_canvas_to_region_convert(const Elm_Map *obj, int x, int y, double *lon, double *lat);

#endif
