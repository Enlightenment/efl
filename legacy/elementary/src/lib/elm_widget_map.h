#ifndef ELM_WIDGET_MAP_H
#define ELM_WIDGET_MAP_H

#include "elm_interface_scrollable.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-map-class The Elementary Map Class
 *
 * Elementary, besides having the @ref Map widget, exposes its
 * foundation -- the Elementary Map Class -- in order to create
 * other widgets which are a map with some more logic on top.
 */

/**
 * @def ELM_MAP_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Map_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_MAP_CLASS(x) ((Elm_Map_Smart_Class *) x)

/**
 * @def ELM_MAP_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Map_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_MAP_DATA(x) ((Elm_Map_Smart_Data *) x)

/**
 * @def ELM_MAP_SMART_CLASS_VERSION
 *
 * Current version for Elementary map @b base smart class, a value
 * which goes to _Elm_Map_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_MAP_SMART_CLASS_VERSION 1

/**
 * @def ELM_MAP_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Map_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_MAP_SMART_CLASS_INIT_NULL
 * @see ELM_MAP_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_MAP_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_MAP_SMART_CLASS_VERSION}

/**
 * @def ELM_MAP_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Map_Smart_Class structure.
 *
 * @see ELM_MAP_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_MAP_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_MAP_SMART_CLASS_INIT_NULL \
  ELM_MAP_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_MAP_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Map_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_MAP_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Map_Smart_Class (base field)
 * to the latest #ELM_MAP_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_MAP_SMART_CLASS_INIT_NULL
 * @see ELM_MAP_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_MAP_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_MAP_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary map base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a map.
 *
 * All of the functions listed on @ref Map namespace will work for
 * objects deriving from #Elm_Map_Smart_Class.
 */
typedef struct _Elm_Map_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Map_Smart_Class;

/**
 * Base widget smart data extended with map instance data.
 */
typedef struct _Elm_Map_Smart_Data     Elm_Map_Smart_Data;

typedef char *(*Elm_Map_Module_Source_Name_Func)(void);
typedef int   (*Elm_Map_Module_Tile_Zoom_Min_Func)(void);
typedef int   (*Elm_Map_Module_Tile_Zoom_Max_Func)(void);
typedef char *(*Elm_Map_Module_Tile_Url_Func)(const Evas_Object *,
                                              int,
                                              int,
                                              int);
typedef Eina_Bool (*Elm_Map_Module_Tile_Geo_to_Coord_Func)(const Evas_Object *,
                                                           int,
                                                           double,
                                                           double,
                                                           int,
                                                           int *,
                                                           int *);
typedef Eina_Bool (*Elm_Map_Module_Tile_Coord_to_Geo_Func)(const Evas_Object *,
                                                           int,
                                                           int,
                                                           int,
                                                           int,
                                                           double *,
                                                           double *);
typedef double (*Elm_Map_Module_Tile_Scale_Func)(const Evas_Object *,
                                                 double,
                                                 double,
                                                 int);
typedef char *(*Elm_Map_Module_Route_Url_Func)(const Evas_Object *,
                                               const char *,
                                               int,
                                               double,
                                               double,
                                               double,
                                               double);
typedef char *(*Elm_Map_Module_Name_Url_Func)(const Evas_Object *,
                                              int,
                                              const char *,
                                              double,
                                              double);

typedef struct _Source_Tile            Source_Tile;
// FIXME: Currently tile size must be 256*256
// and the map size is pow(2.0, z) * (tile size)
struct _Source_Tile
{
   const char                           *name;
   int                                   zoom_min;
   int                                   zoom_max;
   Elm_Map_Module_Tile_Url_Func          url_cb;
   Elm_Map_Module_Tile_Geo_to_Coord_Func geo_to_coord;
   Elm_Map_Module_Tile_Coord_to_Geo_Func coord_to_geo;
   Elm_Map_Module_Tile_Scale_Func        scale_cb;
};

typedef struct _Source_Route           Source_Route;
struct _Source_Route
{
   const char                   *name;
   Elm_Map_Module_Route_Url_Func url_cb;
};

typedef struct _Source_Name            Source_Name;
struct _Source_Name
{
   const char                  *name;
   Elm_Map_Module_Name_Url_Func url_cb;
};

typedef struct _Path                   Path;
typedef struct _Color                  Color;
typedef struct _Region                 Region;
typedef struct _Grid                   Grid;
typedef struct _Grid_Item              Grid_Item;
typedef struct _Overlay_Default        Overlay_Default;
typedef struct _Overlay_Class          Overlay_Class;
typedef struct _Overlay_Group          Overlay_Group;
typedef struct _Overlay_Bubble         Overlay_Bubble;
typedef struct _Overlay_Route          Overlay_Route;
typedef struct _Overlay_Line           Overlay_Line;
typedef struct _Overlay_Polygon        Overlay_Polygon;
typedef struct _Overlay_Circle         Overlay_Circle;
typedef struct _Overlay_Scale          Overlay_Scale;
typedef struct _Path_Node              Path_Node;
typedef struct _Path_Waypoint          Path_Waypoint;
typedef struct _Route_Dump             Route_Dump;
typedef struct _Name_Dump              Name_Dump;
typedef struct _Delayed_Data           Delayed_Data;

enum _Route_Xml_Attribute
{
   ROUTE_XML_NONE,
   ROUTE_XML_DISTANCE,
   ROUTE_XML_DESCRIPTION,
   ROUTE_XML_COORDINATES,
   ROUTE_XML_LAST
} Route_Xml_Attibute;

enum _Name_Xml_Attribute
{
   NAME_XML_NONE,
   NAME_XML_NAME,
   NAME_XML_LON,
   NAME_XML_LAT,
   NAME_XML_LAST
} Name_Xml_Attibute;

enum _Track_Xml_Attribute
{
   TRACK_XML_NONE,
   TRACK_XML_COORDINATES,
   TRACK_XML_LAST
} Track_Xml_Attibute;

struct _Path
{
   Evas_Coord x, y;
};

struct _Region
{
   double lon, lat;
};

struct _Delayed_Data
{
   void                (*func)(void *data);
   Elm_Map_Smart_Data *wsd;
   Elm_Map_Zoom_Mode   mode;
   int                 zoom;
   double              lon, lat;
   Eina_List          *overlays;
};

struct _Color
{
   int r, g, b, a;
};

struct _Overlay_Group
{
   Elm_Map_Smart_Data *wsd;
   double              lon, lat;
   Elm_Map_Overlay    *overlay; // virtual group type overlay
   Elm_Map_Overlay    *klass; // class overlay for this virtual group
   Overlay_Default    *ovl;  // rendered overlay
   Eina_List          *members;
   Eina_Bool           in : 1;
   Eina_Bool           boss : 1;
};

struct _Overlay_Default
{
   Elm_Map_Smart_Data *wsd;
   Evas_Coord          w, h;

   double              lon, lat;
   Evas_Coord          x, y;

   // Display priority is content > icon > clas_obj > clas_icon > layout
   Evas_Object        *content;
   Evas_Object        *icon;

   Color               c;
   // if clas_content or icon exists, do not inherit from class
   Evas_Object        *clas_content; // Duplicated from class content
   Evas_Object        *clas_icon; // Duplicated from class icon
   Evas_Object        *layout;
};

struct _Overlay_Class
{
   Elm_Map_Smart_Data *wsd;
   Eina_List          *members;
   int                 zoom_max;
   Evas_Object        *content;
   Evas_Object        *icon;
};

struct _Overlay_Bubble
{
   Elm_Map_Smart_Data *wsd;
   Evas_Object        *pobj;
   Evas_Object        *obj, *sc, *bx;
   double              lon, lat;
   Evas_Coord          x, y, w, h;
};

struct _Overlay_Route
{
   Elm_Map_Smart_Data *wsd;

   Evas_Object        *obj;
   Eina_List          *paths;
   Eina_List          *nodes;
};

struct _Overlay_Line
{
   Elm_Map_Smart_Data *wsd;
   double              flon, flat, tlon, tlat;
   Evas_Object        *obj;
};

struct _Overlay_Polygon
{
   Elm_Map_Smart_Data *wsd;
   Eina_List          *regions; // list of Regions
   Evas_Object        *obj;
};

struct _Overlay_Circle
{
   Elm_Map_Smart_Data *wsd;
   double              lon, lat;
   double              radius; // Intial pixel in intial view
   double              ratio; // initial-radius/map-size
   Evas_Object        *obj;
};

struct _Overlay_Scale
{
   Elm_Map_Smart_Data *wsd;
   Evas_Coord          x, y;
   Evas_Coord          w, h;
   Evas_Object        *obj;
};

struct _Elm_Map_Overlay
{
   Elm_Map_Smart_Data    *wsd;

   Evas_Coord             zoom_min;
   Color                  c;
   void                  *data; // user set data

   Elm_Map_Overlay_Type   type;
   void                  *ovl; // Overlay Data for each type

   Elm_Map_Overlay_Get_Cb cb;
   void                  *cb_data;

   Elm_Map_Overlay_Del_Cb del_cb;
   void                  *del_cb_data;

   // These are not used if overlay type is class or group
   Overlay_Group         *grp;

   Eina_Bool              visible : 1;
   Eina_Bool              paused : 1;
   Eina_Bool              hide : 1;
};

struct _Elm_Map_Route
{
   Elm_Map_Smart_Data      *wsd;

   char                    *fname;
   Elm_Map_Route_Type       type;
   Elm_Map_Route_Method     method;
   double                   flon, flat, tlon, tlat;
   Elm_Map_Route_Cb         cb;
   void                    *data;
   Ecore_File_Download_Job *job;

   Eina_List               *nodes;
   Eina_List               *waypoint;
   struct
   {
      int         node_count;
      int         waypoint_count;
      const char *nodes;
      const char *waypoints;
      double      distance; /* unit : km */
   } info;

   Path_Node               *n;
   Path_Waypoint           *w;
};

struct _Path_Node
{
   Elm_Map_Smart_Data *wsd;

   int                 idx;
   struct
   {
      double lon, lat;
      char  *address;
   } pos;
};

struct _Path_Waypoint
{
   Elm_Map_Smart_Data *wsd;

   const char         *point;
};

struct _Elm_Map_Name
{
   Elm_Map_Smart_Data      *wsd;

   int                      method;
   char                    *address;
   double                   lon, lat;

   char                    *fname;
   Ecore_File_Download_Job *job;
   Elm_Map_Name_Cb          cb;
   void                    *data;
};

struct _Route_Dump
{
   int    id;
   char  *fname;
   double distance;
   char  *description;
   char  *coordinates;
};

struct _Name_Dump
{
   int    id;
   char  *address;
   double lon;
   double lat;
};

struct _Grid_Item
{
   Grid                    *g;

   Elm_Map_Smart_Data      *wsd;
   Evas_Object             *img;
   const char              *file;
   const char              *url;
   int                      x, y; // Tile coordinate

   Ecore_File_Download_Job *job;

   Eina_Bool                file_have : 1;
};

struct _Grid
{
   Elm_Map_Smart_Data *wsd;
   int                 zoom;  /* zoom level tiles want for optimal
                               * display (1, 2, 4, 8) */
   int                 tw, th; // size of grid in tiles
   Eina_Matrixsparse  *grid;
};

struct _Elm_Map_Smart_Data
{
   Elm_Widget_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   Evas_Object                          *hit_rect;
   const Elm_Scrollable_Smart_Interface *s_iface;
   Evas_Object                          *pan_obj;
   Evas_Object                          *g_layer;

   /* Tiles are below this and overlays are on top */
   Evas_Object                          *sep_maps_overlays;
   Evas_Map                             *map;

   Eina_Array                           *src_tile_mods;
   Source_Tile                          *src_tile;
   Eina_List                            *src_tiles;
   const char                          **src_tile_names;

   Eina_Array                           *src_route_mods;
   Source_Route                         *src_route;
   Eina_List                            *src_routes;
   const char                          **src_route_names;

   Eina_Array                           *src_name_mods;
   Source_Name                          *src_name;
   Eina_List                            *src_names;
   const char                          **src_name_names;

   Eina_List                            *grids;

   int                                   zoom_min, zoom_max;
   int                                   tsize;
   int                                   id;
   int                                   zoom;

   double                                zoom_detail;
   struct
   {
      int w, h;    // Current pixel width, heigth of a grid
      int tile;    // Current pixel size of a grid item
   } size;

   Elm_Map_Zoom_Mode                     mode;

   struct
   {
      double zoom;
      double diff;
      int    cnt;
   } ani;

   Ecore_Timer                          *zoom_timer;
   Ecore_Animator                       *zoom_animator;
   Ecore_Timer                          *loaded_timer;

   int                                   try_num;
   int                                   finish_num;
   int                                   download_num;

   Eina_List                            *download_list;
   Ecore_Idler                          *download_idler;
   Eina_Hash                            *ua;
   const char                           *user_agent;

   Evas_Coord                            pan_x, pan_y;
   Eina_List                            *delayed_jobs;

   Ecore_Timer                          *scr_timer;
   Ecore_Timer                          *long_timer;
   Evas_Event_Mouse_Down                 ev;

   double                                pinch_zoom;
   struct
   {
      Evas_Coord cx, cy;
      double     a, d;
   } rotate;

   Eina_List                            *routes;
   Eina_List                            *track;
   Eina_List                            *names;

   Eina_List                            *overlays;
   Eina_List                            *group_overlays;
   Eina_List                            *all_overlays;

   Eina_Bool                             wheel_disabled : 1;
   Eina_Bool                             on_hold : 1;
   Eina_Bool                             paused : 1;
};

typedef struct _Elm_Map_Pan_Smart_Class
{
   Elm_Pan_Smart_Class base;
} Elm_Map_Pan_Smart_Class;

typedef struct _Elm_Map_Pan_Smart_Data Elm_Map_Pan_Smart_Data;
struct _Elm_Map_Pan_Smart_Data
{
   Elm_Pan_Smart_Data  base;
   Elm_Map_Smart_Data *wsd;
};

/**
 * @}
 */

EAPI extern const char ELM_MAP_SMART_NAME[];
EAPI extern const char ELM_MAP_PAN_SMART_NAME[];
EAPI const Elm_Map_Smart_Class *elm_map_smart_class_get(void);

#define ELM_MAP_DATA_GET(o, sd) \
  Elm_Map_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_MAP_PAN_DATA_GET(o, sd) \
  Elm_Map_Pan_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_MAP_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_MAP_DATA_GET(o, ptr);                     \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_MAP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_MAP_DATA_GET(o, ptr);                         \
  if (!ptr)                                              \
    {                                                    \
       CRITICAL("No widget data for object %p (%s)",     \
                o, evas_object_type_get(o));             \
       return val;                                       \
    }

#define ELM_MAP_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                \
        ((obj), ELM_MAP_SMART_NAME, __func__)) \
    return

#endif
