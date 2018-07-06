#ifndef ELM_WIDGET_MAP_H
#define ELM_WIDGET_MAP_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
 * Base widget smart data extended with map instance data.
 */
typedef struct _Elm_Map_Data     Elm_Map_Data;

typedef struct _Elm_Map_Name_List      Elm_Map_Name_List;
typedef Eina_Stringshare *(*Elm_Map_Module_Source_Name_Func)(void);
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
typedef Evas_Object *(*Elm_Map_Module_Tile_Copyright_Func)(Evas_Object *);
typedef char *(*Elm_Map_Module_Route_Url_Func)(const Evas_Object *,
                                               const char *,
                                               int,
                                               double,
                                               double,
                                               double,
                                               double);
typedef void (*Elm_Map_Module_Route_Parse_Func)(Elm_Map_Route *);
typedef char *(*Elm_Map_Module_Name_Url_Func)(const Evas_Object *,
                                              int,
                                              const char *,
                                              double,
                                              double);
typedef void (*Elm_Map_Module_Name_Parse_Func)(Elm_Map_Name *n);
typedef void (*Elm_Map_Module_Name_List_Parse_Func)(Elm_Map_Name_List *nl);

typedef struct _Source_Tile            Source_Tile;
// FIXME: Currently tile size must be 256*256
// and the map size is pow(2.0, z) * (tile size)
struct _Source_Tile
{
   Eina_Stringshare                     *name;
   int                                   zoom_min;
   int                                   zoom_max;
   Elm_Map_Module_Tile_Url_Func          url_cb;
   Elm_Map_Module_Tile_Geo_to_Coord_Func geo_to_coord;
   Elm_Map_Module_Tile_Coord_to_Geo_Func coord_to_geo;
   Elm_Map_Module_Tile_Scale_Func        scale_cb;
   Elm_Map_Module_Tile_Copyright_Func    copyright_cb;
};

typedef struct _Source_Route           Source_Route;
struct _Source_Route
{
   Eina_Stringshare             *name;
   Elm_Map_Module_Route_Url_Func url_cb;
   Elm_Map_Module_Route_Parse_Func route_parse_cb;
};

typedef struct _Source_Name            Source_Name;
struct _Source_Name
{
   Eina_Stringshare            *name;
   Elm_Map_Module_Name_Url_Func url_cb;
   Elm_Map_Module_Name_Parse_Func name_parse_cb;
   Elm_Map_Module_Name_List_Parse_Func name_list_parse_cb;
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
typedef struct _Calc_Job               Calc_Job;

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

struct _Color
{
   int r, g, b, a;
};

struct _Overlay_Group
{
   Elm_Map_Data *wsd;
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
   Elm_Map_Data *wsd;
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
   Elm_Map_Data *wsd;
   Eina_List          *members;
   int                 zoom_max;
   Evas_Object        *content;
   Evas_Object        *icon;
};

struct _Overlay_Bubble
{
   Elm_Map_Data *wsd;
   Evas_Object        *pobj;
   Evas_Object        *obj, *sc, *bx;
   double              lon, lat;
   Evas_Coord          x, y, w, h;
};

struct _Overlay_Route
{
   Elm_Map_Data *wsd;

   Evas_Object        *obj;
   Eina_List          *paths;
   Eina_List          *nodes;
};

struct _Overlay_Line
{
   Elm_Map_Data *wsd;
   double              flon, flat, tlon, tlat;
   Evas_Object        *obj;
};

struct _Overlay_Polygon
{
   Elm_Map_Data *wsd;
   Eina_List          *regions; // list of Regions
   Evas_Object        *obj;
};

struct _Overlay_Circle
{
   Elm_Map_Data *wsd;
   double              lon, lat;
   double              radius; // Initial pixel in initial view
   double              ratio; // initial-radius/map-size
   Evas_Object        *obj;
};

struct _Overlay_Scale
{
   Elm_Map_Data *wsd;
   Evas_Coord          x, y;
   Evas_Coord          w, h;
   Evas_Object        *obj;
};

struct _Elm_Map_Overlay
{
   Elm_Map_Data    *wsd;

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
   Elm_Map_Data      *wsd;

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
   Elm_Map_Data *wsd;

   int                 idx;
   struct
   {
      double lon, lat;
      char  *address;
   } pos;
};

struct _Path_Waypoint
{
   Elm_Map_Data *wsd;

   const char         *point;
};

struct _Elm_Map_Name
{
   Elm_Map_Data      *wsd;

   int                      method;
   char                    *address;
   double                   lon, lat;

   char                    *fname;
   Ecore_File_Download_Job *job;
   Elm_Map_Name_Cb          cb;
   void                    *data;
};

struct _Elm_Map_Name_List
{
   Elm_Map_Data      *wsd;

   Eina_List               *names;
   double                   lon, lat;

   char                    *fname;
   Ecore_File_Download_Job *job;
   Elm_Map_Name_List_Cb     cb;
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

   Elm_Map_Data      *wsd;
   Evas_Object             *img;
   const char              *file;
   const char              *url;
   int                      x, y; // Tile coordinate

   Ecore_File_Download_Job *job;

   Eina_Bool                file_have : 1;
};

struct _Grid
{
   Elm_Map_Data *wsd;
   int                 zoom;  /* zoom level tiles want for optimal
                               * display (1, 2, 4, 8) */
   int                 tw, th; // size of grid in tiles
   Eina_Matrixsparse  *grid;
};

struct _Calc_Job
{
   double zoom;
   void (*zoom_mode_set)(Elm_Map_Data *sd, double zoom);

   Eina_Bool bring_in : 1;
   double lon, lat;
   void (*region_show_bring_in)(Elm_Map_Data *sd, double lon,
                                double lat, Eina_Bool bring_in);

   Eina_List *overlays;
   void (*overlays_show)(Elm_Map_Data *sd, Eina_List *overlays);
};

struct _Elm_Map_Data
{
   Evas_Object                          *hit_rect;
   Evas_Object                          *pan_obj;
   Evas_Object                          *g_layer;
   Evas_Object                          *obj; // The object itself

   /* Tiles are below this and overlays are on top */
   Evas_Object                          *sep_maps_overlays;
   Evas_Map                             *map;

   Eina_Array                           *src_mods;

   Source_Tile                          *src_tile;
   Eina_List                            *src_tiles;
   const char                          **src_tile_names;

   Source_Route                         *src_route;
   Eina_List                            *src_routes;
   const char                          **src_route_names;

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

   Efl_Ui_Zoom_Mode                     mode;

   struct
   {
      double zoom;
      double zoom_diff;
      double lon, lat;
      double lon_diff, lat_diff;
      int    zoom_cnt;
      int    region_cnt;
   } ani;

   Ecore_Timer                          *zoom_timer;
   Ecore_Timer                          *loaded_timer;

   int                                   try_num;
   int                                   finish_num;
   int                                   download_num;

   Eina_List                            *download_list;
   Ecore_Idler                          *download_idler;
   Eina_Hash                            *ua;
   const char                           *user_agent;

   Evas_Coord                            pan_x, pan_y;

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
   Eina_Bool                             zoom_animator : 1;

   Calc_Job                               calc_job;
   Evas_Object                          *copyright;
};

typedef struct _Elm_Map_Pan_Data Elm_Map_Pan_Data;
struct _Elm_Map_Pan_Data
{
   Evas_Object            *wobj;
   Elm_Map_Data *wsd;
};

/**
 * @}
 */

#define ELM_MAP_DATA_GET(o, sd) \
  Elm_Map_Data * sd = efl_data_scope_get(o, ELM_MAP_CLASS)

#define ELM_MAP_PAN_DATA_GET(o, sd) \
  Elm_Map_Pan_Data * sd = efl_data_scope_get(o, ELM_MAP_PAN_CLASS)

#define ELM_MAP_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_MAP_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                      \
    {                                           \
       ERR("No widget data for object %p (%s)", \
           o, evas_object_type_get(o));         \
       return;                                  \
    }

#define ELM_MAP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_MAP_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                          \
    {                                               \
       ERR("No widget data for object %p (%s)",     \
           o, evas_object_type_get(o));             \
       return val;                                  \
    }

#define ELM_MAP_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_MAP_CLASS))) \
    return

#endif
