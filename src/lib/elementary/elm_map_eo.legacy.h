#ifndef _ELM_MAP_EO_LEGACY_H_
#define _ELM_MAP_EO_LEGACY_H_

#ifndef _ELM_MAP_EO_CLASS_TYPE
#define _ELM_MAP_EO_CLASS_TYPE

typedef Eo Elm_Map;

#endif

#ifndef _ELM_MAP_EO_TYPES
#define _ELM_MAP_EO_TYPES

/**
 * @brief Widget is broken due to on-line service API breaks
 *
 * Widget is broken due to on-line service API breaks
 *
 * @ingroup Elm_Map_Group
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
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
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
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
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
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_zoom_min_set(Elm_Map *obj, int zoom);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI int elm_map_zoom_min_get(const Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_rotate_set(Elm_Map *obj, double degree, int cx, int cy);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_rotate_get(const Elm_Map *obj, double *degree, int *cx, int *cy);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_user_agent_set(Elm_Map *obj, const char *user_agent);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI const char *elm_map_user_agent_get(const Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_zoom_max_set(Elm_Map *obj, int zoom);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI int elm_map_zoom_max_get(const Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_get(const Elm_Map *obj, double *lon, double *lat);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Eina_List *elm_map_overlays_get(const Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_tile_load_status_get(const Elm_Map *obj, int *try_num, int *finish_num);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_source_set(Elm_Map *obj, Elm_Map_Source_Type type, const char *source_name);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI const char *elm_map_source_get(const Elm_Map *obj, Elm_Map_Source_Type type);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Route *elm_map_route_add(Elm_Map *obj, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Efl_Canvas_Object *elm_map_track_add(Elm_Map *obj, void *emap);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_to_canvas_convert(const Elm_Map *obj, double lon, double lat, int *x, int *y);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_circle_add(Elm_Map *obj, double lon, double lat, double radius);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_class_add(Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_bubble_add(Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI const char **elm_map_sources_get(const Elm_Map *obj, Elm_Map_Source_Type type);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_polygon_add(Elm_Map *obj);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_line_add(Elm_Map *obj, double flon, double flat, double tlon, double tlat);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_show(Elm_Map *obj, double lon, double lat);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Name *elm_map_name_add(const Elm_Map *obj, const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_name_search(const Elm_Map *obj, const char *address, Elm_Map_Name_List_Cb name_cb, void *data);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_bring_in(Elm_Map *obj, double lon, double lat);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_region_zoom_bring_in(Elm_Map *obj, int zoom, double lon, double lat);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_track_remove(Elm_Map *obj, Efl_Canvas_Object *route);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_route_add(Elm_Map *obj, const Elm_Map_Route *route);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_scale_add(Elm_Map *obj, int x, int y);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI Elm_Map_Overlay *elm_map_overlay_add(Elm_Map *obj, double lon, double lat);

/**
 * @brief Widget is broken due to on-line service API breaks
 * 
 * @ingroup Elm_Map_Group
 */
EAPI void elm_map_canvas_to_region_convert(const Elm_Map *obj, int x, int y, double *lon, double *lat);

#endif
