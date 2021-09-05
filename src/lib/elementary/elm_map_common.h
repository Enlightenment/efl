/**
 * @addtogroup Elm_Map_Group
 *
 * @{
 */


/**
 * Widget is broken due to on-line service API breaks
 */
typedef enum
{
   ELM_MAP_NAME_METHOD_SEARCH,
   ELM_MAP_NAME_METHOD_REVERSE,
   ELM_MAP_NAME_METHOD_LAST
} Elm_Map_Name_Method;

/**
 * Widget is broken due to on-line service API breaks
 */
typedef enum
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
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_del(Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI Elm_Map_Overlay_Type  elm_map_overlay_type_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_data_set(Elm_Map_Overlay *overlay, void *data);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void *                elm_map_overlay_data_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_hide_set(Elm_Map_Overlay *overlay, Eina_Bool hide);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI Eina_Bool             elm_map_overlay_hide_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_displayed_zoom_min_set(Elm_Map_Overlay *overlay, int zoom);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI int                   elm_map_overlay_displayed_zoom_min_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_paused_set(Elm_Map_Overlay *overlay, Eina_Bool paused);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI Eina_Bool             elm_map_overlay_paused_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI Eina_Bool             elm_map_overlay_visible_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_content_set(Elm_Map_Overlay *overlay, Evas_Object *obj);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI const Evas_Object *   elm_map_overlay_content_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_icon_set(Elm_Map_Overlay *overlay, Evas_Object *icon);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI const Evas_Object *   elm_map_overlay_icon_get(const Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_region_set(Elm_Map_Overlay *overlay, double lon, double lat);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_region_get(const Elm_Map_Overlay *overlay, double *lon, double *lat);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_color_set(Elm_Map_Overlay *overlay, int r, int g, int b, int a);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_color_get(const Elm_Map_Overlay *overlay, int *r, int *g, int *b, int *a);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_show(Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlays_show(Eina_List *overlays);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_get_cb_set(Elm_Map_Overlay *overlay, Elm_Map_Overlay_Get_Cb get_cb, void *data);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_del_cb_set(Elm_Map_Overlay *overlay, Elm_Map_Overlay_Del_Cb del_cb, void *data);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_class_append(Elm_Map_Overlay *clas, Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_class_remove(Elm_Map_Overlay *clas, Elm_Map_Overlay *overlay);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_class_zoom_max_set(Elm_Map_Overlay *clas, int zoom);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI int                   elm_map_overlay_class_zoom_max_get(const Elm_Map_Overlay *clas);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI Eina_List *           elm_map_overlay_group_members_get(const Elm_Map_Overlay *grp);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_bubble_follow(Elm_Map_Overlay *bubble, const Elm_Map_Overlay *parent);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_bubble_content_append(Elm_Map_Overlay *bubble, Evas_Object *content);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_bubble_content_clear(Elm_Map_Overlay *bubble);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_overlay_polygon_region_add(Elm_Map_Overlay *overlay, double lon, double lat);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_route_del(Elm_Map_Route *route);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI double                elm_map_route_distance_get(const Elm_Map_Route *route);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI const char           *elm_map_route_node_get(const Elm_Map_Route *route);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI const char           *elm_map_route_waypoint_get(const Elm_Map_Route *route);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI const char           *elm_map_name_address_get(const Elm_Map_Name *name);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_name_region_get(const Elm_Map_Name *name, double *lon, double *lat);

/**
 * Widget is broken due to on-line service API breaks
 */
EAPI void                  elm_map_name_del(Elm_Map_Name *name);

/**
 * @}
 */
