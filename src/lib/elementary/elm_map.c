#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_map.h"
#include "elm_interface_scrollable.h"
#include "elm_pan_eo.h"
#include "elm_map_pan_eo.h"
#include "elm_map_eo.h"

#define MY_PAN_CLASS ELM_MAP_PAN_CLASS

#define MY_PAN_CLASS_NAME "Elm_Map_Pan"
#define MY_PAN_CLASS_NAME_LEGACY "elm_map_pan"

#define MY_CLASS ELM_MAP_CLASS

#define MY_CLASS_NAME "Elm_Map"
#define MY_CLASS_NAME_LEGACY "elm_map"

#define OVERLAY_CLASS_ZOOM_MAX  255
#define MAX_CONCURRENT_DOWNLOAD 10

#define ROUND(z) (((z) < 0) ? (int)ceil((z) - 0.005) : (int)floor((z) + 0.005))
#define EVAS_MAP_POINT         4
#define DEFAULT_TILE_SIZE      256
#define MARER_MAX_NUMBER       30
#define OVERLAY_GROUPING_SCALE 2
#define ZOOM_ANIM_CNT          75
#define ZOOM_BRING_CNT         80

#define CACHE_ROOT             "/elm_map"
#define CACHE_TILE_ROOT        CACHE_ROOT "/%d/%d/%d"
#define CACHE_TILE_PATH        "%s/%d.png"
#define CACHE_ROUTE_ROOT       CACHE_ROOT "/route"
#define CACHE_NAME_ROOT        CACHE_ROOT "/name"

#define ROUTE_YOURS_URL        "http://www.yournavigation.org/api/dev/route.php"
#define ROUTE_TYPE_MOTORCAR    "motocar"
#define ROUTE_TYPE_BICYCLE     "bicycle"
#define ROUTE_TYPE_FOOT        "foot"
#define YOURS_DISTANCE         "distance"
#define YOURS_DESCRIPTION      "description"
#define YOURS_COORDINATES      "coordinates"

#define NAME_NOMINATIM_URL     "http://nominatim.openstreetmap.org"
#define NOMINATIM_RESULT       "result"
#define NOMINATIM_PLACE        "place"
#define NOMINATIM_ATTR_LON     "lon"
#define NOMINATIM_ATTR_LAT     "lat"
#define NOMINATIM_ATTR_ADDRESS "display_name"

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_zoom(Evas_Object *obj, const char *params);
static Eina_Bool _zoom_animator_set(Elm_Map_Data *sd, void *callback);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"zoom", _key_action_zoom},
   {NULL, NULL}
};

static char *
_mapnik_url_cb(const Evas_Object *obj EINA_UNUSED,
               int x,
               int y,
               int zoom)
{
   char buf[PATH_MAX];

   // ((x+y+zoom)%3)+'a' is requesting map images from distributed
   // tile servers (eg., a, b, c)
   snprintf(buf, sizeof(buf), "http://%c.tile.openstreetmap.org/%d/%d/%d.png",
            ((x + y + zoom) % 3) + 'a', zoom, x, y);
   return strdup(buf);
}

static char *
_osmarender_url_cb(const Evas_Object *obj EINA_UNUSED,
                   int x,
                   int y,
                   int zoom)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf),
            "http://%c.tah.openstreetmap.org/Tiles/tile/%d/%d/%d.png",
            ((x + y + zoom) % 3) + 'a', zoom, x, y);

   return strdup(buf);
}

static char *
_cyclemap_url_cb(const Evas_Object *obj EINA_UNUSED,
                 int x,
                 int y,
                 int zoom)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf),
            "http://%c.tile.opencyclemap.org/cycle/%d/%d/%d.png",
            ((x + y + zoom) % 3) + 'a', zoom, x, y);

   return strdup(buf);
}

static char *
_mapquest_url_cb(const Evas_Object *obj EINA_UNUSED,
                 int x,
                 int y,
                 int zoom)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf),
            "http://otile%d.mqcdn.com/tiles/1.0.0/osm/%d/%d/%d.png",
            ((x + y + zoom) % 4) + 1, zoom, x, y);

   return strdup(buf);
}

static char *
_mapquest_aerial_url_cb(const Evas_Object *obj EINA_UNUSED,
                        int x,
                        int y,
                        int zoom)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "http://oatile%d.mqcdn.com/naip/%d/%d/%d.png",
            ((x + y + zoom) % 4) + 1, zoom, x, y);

   return strdup(buf);
}

static char *
_yours_url_cb(const Evas_Object *obj EINA_UNUSED,
              const char *type_name,
              int method,
              double flon,
              double flat,
              double tlon,
              double tlat)
{
   char buf[PATH_MAX];

   snprintf
     (buf, sizeof(buf),
     "%s?flat=%lf&flon=%lf&tlat=%lf&tlon=%lf&v=%s&fast=%d&instructions=1",
     ROUTE_YOURS_URL, flat, flon, tlat, tlon, type_name, method);

   return strdup(buf);
}

// TODO: fix monav api
/*
   static char *
   _monav_url_cb(const Evas_Object *obj EINA_UNUSED,
              char *type_name,
              int method,
              double flon,
              double flat,
              double tlon,
              double tlat)
   {
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf),
            "%s?flat=%f&flon=%f&tlat=%f&tlon=%f&v=%s&fast=%d&instructions=1",
            ROUTE_MONAV_URL, flat, flon, tlat, tlon, type_name, method);

   return strdup(buf);
   }

   //TODO: fix ors api

   static char *
   _ors_url_cb(const Evas_Object *obj EINA_UNUSED,
            char *type_name,
            int method,
            double flon,
            double flat,
            double tlon,
            double tlat)
   {
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf),
            "%s?flat=%f&flon=%f&tlat=%f&tlon=%f&v=%s&fast=%d&instructions=1",
            ROUTE_ORS_URL, flat, flon, tlat, tlon, type_name, method);

   return strdup(buf);
   }
 */

static char *
_nominatim_url_cb(const Evas_Object *obj,
                  int method,
                  const char *name,
                  double lon,
                  double lat)
{
   char **str;
   char buf[PATH_MAX + 256];
   unsigned int ele, idx;
   char search_url[PATH_MAX];

   ELM_MAP_DATA_GET(obj, sd);

   if (method == ELM_MAP_NAME_METHOD_SEARCH)
     {
        search_url[0] = '\0';
        str = eina_str_split_full(name, " ", 0, &ele);
        for (idx = 0; idx < ele; idx++)
          {
             eina_strlcat(search_url, str[idx], sizeof(search_url));
             if (!(idx == (ele - 1)))
               eina_strlcat(search_url, "+", sizeof(search_url));
          }
        snprintf(buf, sizeof(buf),
                 "%s/search?q=%s&format=xml&polygon=0&addressdetails=0",
                 NAME_NOMINATIM_URL, search_url);

        if (str)
          {
             free(str[0]);
             free(str);
          }
     }
   else if (method == ELM_MAP_NAME_METHOD_REVERSE)
     snprintf(buf, sizeof(buf),
              "%s/reverse?format=xml&lat=%lf&lon=%lf&zoom=%d&addressdetails=0",
              NAME_NOMINATIM_URL, lat, lon, (int)sd->zoom);
   else strcpy(buf, "");

   return strdup(buf);
}

// Refer : http://wiki.openstreetmap.org/wiki/FAQ
// meters per pixel when latitude is 0 (equator)
// meters per pixel  = _osm_scale_meter[zoom] * cos (latitude)
static const double _osm_scale_meter[] =
{
   78206, 39135.758482, 19567.879241, 9783.939621, 4891.969810,
   2445.984905, 1222.992453, 611.496226, 305.748113, 152.874057, 76.437028,
   38.218514, 19.109257, 9.554629, 4.777314, 2.388657, 1.194329, 0.597164,
   0.29858
};

static double
_scale_cb(const Evas_Object *obj EINA_UNUSED,
          double lon EINA_UNUSED,
          double lat,
          int zoom)
{
   if ((zoom < 0) ||
       (zoom >= (int)(sizeof(_osm_scale_meter) / sizeof(_osm_scale_meter[0])))
      )
     return 0;
   return _osm_scale_meter[zoom] * cos(lat * ELM_PI / 180.0);
}

static Evas_Object *
_osm_copyright_cb(Evas_Object *obj)
{
   Evas_Object *label;

   label = elm_label_add(obj);
   elm_object_text_set(label, "<color=#000000FF><backing=on><backing_color=#FCFCFBFF><shadow_color=#00000000>"
                       "openstreetmap.org opendatacommons.org creativecommons.org"
                       "</shadow_color></backing_color></backing></color>");

   return label;
}

static const Source_Tile src_tiles[] =
{
   {"Mapnik", 0, 18, _mapnik_url_cb, NULL, NULL, _scale_cb, _osm_copyright_cb},
   {"Osmarender", 0, 17, _osmarender_url_cb, NULL, NULL, _scale_cb, _osm_copyright_cb},
   {"CycleMap", 0, 16, _cyclemap_url_cb, NULL, NULL, _scale_cb, _osm_copyright_cb},
   {"MapQuest", 0, 18, _mapquest_url_cb, NULL, NULL, _scale_cb, _osm_copyright_cb},
   {"MapQuest Open Aerial", 0, 11, _mapquest_aerial_url_cb, NULL, NULL,
    _scale_cb, _osm_copyright_cb}
};

static void _kml_parse(Elm_Map_Route *r);
// FIXME: Fix more open sources
static const Source_Route src_routes[] =
{
   {"Yours", _yours_url_cb, _kml_parse}    // http://www.yournavigation.org/
   //{"Monav", _monav_url_cb, _kml_parse},
   //{"ORS", _ors_url_cb, _kml_parse)},     // http://www.openrouteservice.org
};

// Scale in meters
static const double _scale_tb[] =
{
   10000000, 5000000, 2000000, 1000000, 500000, 200000, 100000, 50000,
   20000, 10000, 5000, 2000, 1000, 500, 500, 200, 100, 50, 20, 10, 5, 2, 1
};

static void _name_parse(Elm_Map_Name *n);
static void _name_list_parse(Elm_Map_Name_List *nl);
// FIXME: Add more open sources
static const Source_Name src_names[] =
{
   {"Nominatim", _nominatim_url_cb, _name_parse, _name_list_parse}
};

static int id_num = 1;

static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_PRESS[] = "press";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_LOADED[] = "loaded";
static const char SIG_TILE_LOAD[] = "tile,load";
static const char SIG_TILE_LOADED[] = "tile,loaded";
static const char SIG_TILE_LOADED_FAIL[] = "tile,loaded,fail";
static const char SIG_ROUTE_LOAD[] = "route,load";
static const char SIG_ROUTE_LOADED[] = "route,loaded";
static const char SIG_ROUTE_LOADED_FAIL[] = "route,loaded,fail";
static const char SIG_NAME_LOAD[] = "name,load";
static const char SIG_NAME_LOADED[] = "name,loaded";
static const char SIG_NAME_LOADED_FAIL[] = "name,loaded,fail";
static const char SIG_OVERLAY_CLICKED[] = "overlay,clicked";
static const char SIG_OVERLAY_DEL[] = "overlay,del";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_PRESS, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_SCROLL, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_LOADED, ""},
   {SIG_TILE_LOAD, ""},
   {SIG_TILE_LOADED, ""},
   {SIG_TILE_LOADED_FAIL, ""},
   {SIG_ROUTE_LOAD, ""},
   {SIG_ROUTE_LOADED, ""},
   {SIG_ROUTE_LOADED_FAIL, ""},
   {SIG_NAME_LOAD, ""},
   {SIG_NAME_LOADED, ""},
   {SIG_NAME_LOADED_FAIL, ""},
   {SIG_OVERLAY_CLICKED, ""},
   {SIG_OVERLAY_DEL, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void
_edj_overlay_size_get(Elm_Map_Data *sd,
                      Evas_Coord *w,
                      Evas_Coord *h)
{
   Evas_Object *edj;
   const char *s;

   EINA_SAFETY_ON_NULL_RETURN(w);
   EINA_SAFETY_ON_NULL_RETURN(h);

   edj = edje_object_add(evas_object_evas_get(sd->obj));
   elm_widget_theme_object_set
     (sd->obj, edj, "map/marker", "radio",
     elm_widget_style_get(sd->obj));

   s = edje_object_data_get(edj, "size_w");
   if (s) *w = atoi(s);
   else *w = 0;

   s = edje_object_data_get(edj, "size_h");
   if (s) *h = atoi(s);
   else *h = 0;

   evas_object_del(edj);
}

static void
_rotate_do(Evas_Coord x,
           Evas_Coord y,
           Evas_Coord cx,
           Evas_Coord cy,
           double degree,
           Evas_Coord *xx,
           Evas_Coord *yy)
{
   double r = (degree * M_PI) / 180.0;

   if (xx) *xx = ((x - cx) * cos(r)) + ((y - cy) * cos(r + M_PI_2)) + cx;
   if (yy) *yy = ((x - cx) * sin(r)) + ((y - cy) * sin(r + M_PI_2)) + cy;
}

static void
_obj_rotate(Elm_Map_Data *sd,
            Evas_Object *obj)
{
   Evas_Coord w, h, ow, oh;

   evas_map_util_points_populate_from_object(sd->map, obj);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   evas_object_image_size_get(obj, &w, &h);
   if ((w > ow) || (h > oh))
     {
        evas_map_point_image_uv_set(sd->map, 0, 0, 0);
        evas_map_point_image_uv_set(sd->map, 1, w, 0);
        evas_map_point_image_uv_set(sd->map, 2, w, h);
        evas_map_point_image_uv_set(sd->map, 3, 0, h);
     }
   evas_map_util_rotate(sd->map, sd->rotate.d, sd->rotate.cx, sd->rotate.cy);

   evas_object_map_set(obj, sd->map);
   evas_object_map_enable_set(obj, EINA_TRUE);
}

static void
_obj_place(Evas_Object *obj,
           Evas_Coord x,
           Evas_Coord y,
           Evas_Coord w,
           Evas_Coord h)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   evas_object_geometry_set(obj, x, y, w, h);
   evas_object_show(obj);
}

static void
_coord_to_region_convert(Elm_Map_Data *sd,
                         Evas_Coord x,
                         Evas_Coord y,
                         Evas_Coord size,
                         double *lon,
                         double *lat)
{
   int zoom;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   zoom = floor(log((double)size / sd->size.tile) / log(2));
   if ((sd->src_tile) && (sd->src_tile->coord_to_geo))
     {
        if (sd->src_tile->coord_to_geo
              (sd->obj, zoom, x, y, size, lon, lat))
          return;
     }

   if (lon) *lon = (x / (double)size * 360.0) - 180;
   if (lat)
     {
        double n = ELM_PI - (2.0 * ELM_PI * y / size);
        *lat = 180.0 / ELM_PI *atan(0.5 * (exp(n) - exp(-n)));
     }
}

static void
_region_to_coord_convert(Elm_Map_Data *sd,
                         double lon,
                         double lat,
                         Evas_Coord size,
                         Evas_Coord *x,
                         Evas_Coord *y)
{
   int zoom;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   zoom = floor(log((double)size / 256) / log(2));
   if ((sd->src_tile) && (sd->src_tile->geo_to_coord))
     {
        if (sd->src_tile->geo_to_coord
              (sd->obj, zoom, lon, lat, size, x, y)) return;
     }

   if (x) *x = floor((lon + 180.0) / 360.0 * size);
   if (y)
     *y = floor((1.0 - log(tan(lat * ELM_PI / 180.0) +
                           (1.0 / cos(lat * ELM_PI / 180.0)))
                 / ELM_PI) / 2.0 * size);
}

static void
_viewport_coord_get(Elm_Map_Data *sd,
                    Evas_Coord *vx,
                    Evas_Coord *vy,
                    Evas_Coord *vw,
                    Evas_Coord *vh)
{
   Evas_Coord x = 0, y = 0, w = 0, h = 0;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   elm_interface_scrollable_content_pos_get(sd->obj, &x, &y);
   elm_interface_scrollable_content_viewport_geometry_get
         (sd->obj, NULL, NULL, &w, &h);

   if (w > sd->size.w) x -= ((w - sd->size.w) / 2);
   if (h > sd->size.h) y -= ((h - sd->size.h) / 2);
   if (vx) *vx = x;
   if (vy) *vy = y;
   if (vw) *vw = w;
   if (vh) *vh = h;
}

// Map coordinates to canvas geometry without rotate
static void
_coord_to_canvas_no_rotation(Elm_Map_Data *sd,
                             Evas_Coord x,
                             Evas_Coord y,
                             Evas_Coord *xx,
                             Evas_Coord *yy)
{
   Evas_Coord vx = 0, vy = 0, sx = 0, sy = 0;

   _viewport_coord_get(sd, &vx, &vy, NULL, NULL);
   evas_object_geometry_get(sd->pan_obj, &sx, &sy, NULL, NULL);
   if (xx) *xx = x - vx + sx;
   if (yy) *yy = y - vy + sy;
}

// Map coordinates to canvas geometry
static void
_coord_to_canvas(Elm_Map_Data *sd,
                 Evas_Coord x,
                 Evas_Coord y,
                 Evas_Coord *xx,
                 Evas_Coord *yy)
{
   _coord_to_canvas_no_rotation(sd, x, y, &x, &y);
   _rotate_do(x, y, sd->rotate.cx, sd->rotate.cy, sd->rotate.d, &x, &y);
   if (xx) *xx = x;
   if (yy) *yy = y;
}

// Canvas geometry to map coordinates
static void
_canvas_to_coord(Elm_Map_Data *sd,
                 Evas_Coord x,
                 Evas_Coord y,
                 Evas_Coord *xx,
                 Evas_Coord *yy)
{
   Evas_Coord vx = 0, vy = 0, sx = 0, sy = 0;

   _viewport_coord_get(sd, &vx, &vy, NULL, NULL);
   evas_object_geometry_get(sd->pan_obj, &sx, &sy, NULL, NULL);
   _rotate_do(x - sx + vx, y - sy + vy, sd->rotate.cx - sx + vx,
              sd->rotate.cy - sy + vy, -sd->rotate.d, &x, &y);
   if (xx) *xx = x;
   if (yy) *yy = y;
}

static void
_grid_item_coord_get(Grid_Item *gi,
                     int *x,
                     int *y,
                     int *w,
                     int *h)
{
   EINA_SAFETY_ON_NULL_RETURN(gi);

   if (x) *x = gi->x * gi->wsd->size.tile;
   if (y) *y = gi->y * gi->wsd->size.tile;
   if (w) *w = gi->wsd->size.tile;
   if (h) *h = gi->wsd->size.tile;
}

static Eina_Bool
_grid_item_in_viewport(Grid_Item *gi)
{
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0;
   Evas_Coord x = 0, y = 0, w = 0, h = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gi, EINA_FALSE);

   _viewport_coord_get(gi->wsd, &vx, &vy, &vw, &vh);
   _grid_item_coord_get(gi, &x, &y, &w, &h);

   return ELM_RECTS_INTERSECT(x, y, w, h, vx, vy, vw, vh);
}

static Eina_Bool
_loaded_timeout_cb(void *data)
{
   ELM_MAP_DATA_GET(data, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);

   sd->loaded_timer = NULL;
   if (!(sd->download_num) && !(sd->download_idler))
      efl_event_callback_legacy_call
        (sd->obj, ELM_MAP_EVENT_LOADED, NULL);
   return ECORE_CALLBACK_CANCEL;
}

static void
_grid_item_update(Grid_Item *gi)
{
   Evas_Load_Error err;

   EINA_SAFETY_ON_NULL_RETURN(gi);

   evas_object_image_file_set(gi->img, gi->file, NULL);
   if (!gi->wsd->zoom_timer && !gi->wsd->scr_timer)
     evas_object_image_smooth_scale_set(gi->img, EINA_TRUE);
   else evas_object_image_smooth_scale_set(gi->img, EINA_FALSE);

   err = evas_object_image_load_error_get(gi->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Image loading error (%s): %s", gi->file, evas_load_error_str(err));
        ecore_file_remove(gi->file);
        gi->file_have = EINA_FALSE;
     }
   else
     {
        Evas_Coord x, y, w, h;

        _grid_item_coord_get(gi, &x, &y, &w, &h);
        _coord_to_canvas_no_rotation(gi->wsd, x, y, &x, &y);
        _obj_place(gi->img, x, y, w, h);
        _obj_rotate(gi->wsd, gi->img);
        gi->file_have = EINA_TRUE;
     }

   ecore_timer_del(gi->wsd->loaded_timer);
   gi->wsd->loaded_timer = ecore_timer_add(0.25, _loaded_timeout_cb, gi->wsd->obj);
}

static void
_grid_item_load(Grid_Item *gi)
{
   EINA_SAFETY_ON_NULL_RETURN(gi);

   if (gi->file_have) _grid_item_update(gi);
   else if (!gi->job)
     {
        gi->wsd->download_list = eina_list_remove(gi->wsd->download_list, gi);
        gi->wsd->download_list = eina_list_append(gi->wsd->download_list, gi);
     }
}

static void
_grid_item_unload(Grid_Item *gi)
{
   EINA_SAFETY_ON_NULL_RETURN(gi);

   if (gi->file_have)
     {
        evas_object_hide(gi->img);
        evas_object_image_file_set(gi->img, NULL, NULL);
     }
   else if (gi->job)
     {
        ecore_file_download_abort(gi->job);
        ecore_file_remove(gi->file);
        gi->job = NULL;
        gi->wsd->try_num--;
     }
   else gi->wsd->download_list = eina_list_remove(gi->wsd->download_list, gi);
}

static Grid_Item *
_grid_item_create(Grid *g,
                  Evas_Coord x,
                  Evas_Coord y)
{
   char buf[PATH_MAX];
   char buf2[PATH_MAX + 128];
   Grid_Item *gi;
   char *url;

   EINA_SAFETY_ON_NULL_RETURN_VAL(g, NULL);

   gi = ELM_NEW(Grid_Item);
   gi->wsd = g->wsd;
   gi->g = g;
   gi->x = x;
   gi->y = y;

   gi->file_have = EINA_FALSE;
   gi->job = NULL;

   gi->img = evas_object_image_add
       (evas_object_evas_get((g->wsd)->obj));
   evas_object_image_smooth_scale_set(gi->img, EINA_FALSE);
   evas_object_image_scale_hint_set(gi->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
   evas_object_image_filled_set(gi->img, EINA_TRUE);
   evas_object_smart_member_add(gi->img, g->wsd->pan_obj);
   evas_object_pass_events_set(gi->img, EINA_TRUE);
   evas_object_stack_below(gi->img, g->wsd->sep_maps_overlays);

   snprintf(buf, sizeof(buf), "%s" CACHE_TILE_ROOT, efreet_cache_home_get(),
            g->wsd->id, g->zoom, x);

   snprintf(buf2, sizeof(buf2), CACHE_TILE_PATH, buf, y);
   if (!ecore_file_exists(buf)) ecore_file_mkpath(buf);

   eina_stringshare_replace(&gi->file, buf2);
   url = g->wsd->src_tile->url_cb((g->wsd)->obj, x, y, g->zoom);
   if ((!url) || (!strlen(url)))
     {
        eina_stringshare_replace(&gi->url, NULL);
        ERR("Getting source url failed: %s", gi->file);
     }
   else eina_stringshare_replace(&gi->url, url);

   free(url);
   eina_matrixsparse_data_idx_set(g->grid, y, x, gi);

   return gi;
}

static void
_grid_item_free(Grid_Item *gi)
{
   EINA_SAFETY_ON_NULL_RETURN(gi);

   _grid_item_unload(gi);
   if (gi->g && gi->g->grid)
     eina_matrixsparse_data_idx_set(gi->g->grid, gi->y, gi->x, NULL);
   eina_stringshare_del(gi->url);
   if (gi->file_have) ecore_file_remove(gi->file);
   eina_stringshare_del(gi->file);
   evas_object_del(gi->img);

   free(gi);
}

static void
_downloaded_cb(void *data,
               const char *file EINA_UNUSED,
               int status)
{
   Grid_Item *gi = data;

   if (status == 200)
     {
        DBG("Download success from %s to %s", gi->url, gi->file);

        _grid_item_update(gi);
        gi->wsd->finish_num++;
        efl_event_callback_legacy_call
          ((gi->wsd)->obj, ELM_MAP_EVENT_TILE_LOADED, NULL);
     }
   else
     {
        WRN("Download failed from %s to %s (%d) ", gi->url, gi->file, status);

        ecore_file_remove(gi->file);
        gi->file_have = EINA_FALSE;
        efl_event_callback_legacy_call
          ((gi->wsd)->obj, ELM_MAP_EVENT_TILE_LOADED_FAIL, NULL);
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(gi->wsd->obj, wd);
   gi->job = NULL;
   gi->wsd->download_num--;
   if (!gi->wsd->download_num)
     edje_object_signal_emit(wd->resize_obj,
                             "elm,state,busy,stop", "elm");
}

static Eina_Bool
_download_job(void *data)
{
   Evas_Object *obj = data;
   ELM_MAP_DATA_GET(obj, sd);
   Eina_List *l, *ll;
   Grid_Item *gi;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ECORE_CALLBACK_CANCEL);

   if (!eina_list_count(sd->download_list))
     {
        sd->download_idler = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   EINA_LIST_REVERSE_FOREACH_SAFE(sd->download_list, l, ll, gi)
   {
      Eina_Bool ret, file_exists;

      if ((gi->g->zoom != sd->zoom) || !(_grid_item_in_viewport(gi)))
        {
           sd->download_list = eina_list_remove(sd->download_list, gi);
           continue;
        }
      if (sd->download_num >= MAX_CONCURRENT_DOWNLOAD)
        return ECORE_CALLBACK_RENEW;

      file_exists = ecore_file_exists(gi->file);
      if (!file_exists)
        {
           /* Check here if we can download into this directory even if this one
              disappear due to some user black magic */
           char *dir_path;
           dir_path = ecore_file_dir_get(gi->file);
           if (!ecore_file_exists(dir_path)) ecore_file_mkpath(dir_path);
           free(dir_path);
           ret = ecore_file_download_full
              (gi->url, gi->file, _downloaded_cb, NULL, gi, &(gi->job), sd->ua);

           if ((!ret) || (!gi->job))
             {
                ERR("Can't start to download from %s to %s", gi->url, gi->file);
                continue;
             }
           sd->try_num++;
           sd->download_num++;
        }
      sd->download_list = eina_list_remove(sd->download_list, gi);
      efl_event_callback_legacy_call
         (obj, ELM_MAP_EVENT_TILE_LOAD, NULL);
      if (sd->download_num == 1)
        edje_object_signal_emit(wd->resize_obj,
                                "elm,state,busy,start", "elm");
      if (file_exists)
        {
           /* It seem the file already exists, try to load it and let
              _grid_item_update do his job. If this file isn't a image the func
              will invalidate it and try to redownload it. */
           _grid_item_update(gi);
           gi->wsd->finish_num++;
           efl_event_callback_legacy_call
              ((gi->wsd)->obj, ELM_MAP_EVENT_TILE_LOADED, NULL);
        }
   }

   return ECORE_CALLBACK_RENEW;
}

static void
_grid_viewport_get(Grid *g,
                   int *x,
                   int *y,
                   int *w,
                   int *h)
{
   int xx, yy, ww, hh;
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0;

   EINA_SAFETY_ON_NULL_RETURN(g);

   _viewport_coord_get(g->wsd, &vx, &vy, &vw, &vh);
   if (vx < 0) vx = 0;
   if (vy < 0) vy = 0;

   xx = (vx / g->wsd->size.tile) - 1;
   if (xx < 0) xx = 0;

   yy = (vy / g->wsd->size.tile) - 1;
   if (yy < 0) yy = 0;

   ww = (vw / g->wsd->size.tile) + 3;
   if (xx + ww >= g->tw) ww = g->tw - xx;

   hh = (vh / g->wsd->size.tile) + 3;
   if (yy + hh >= g->th) hh = g->th - yy;

   if (x) *x = xx;
   if (y) *y = yy;
   if (w) *w = ww;
   if (h) *h = hh;
}

static void
_grid_unload(Grid *g)
{
   Eina_Matrixsparse_Cell *cell;
   Eina_Iterator *it;
   Grid_Item *gi;

   EINA_SAFETY_ON_NULL_RETURN(g);

   it = eina_matrixsparse_iterator_new(g->grid);
   EINA_ITERATOR_FOREACH(it, cell)
     {
        gi = eina_matrixsparse_cell_data_get(cell);
        _grid_item_unload(gi);
     }
   eina_iterator_free(it);
}

static void
_grid_load(Grid *g)
{
   Eina_Matrixsparse_Cell *cell;
   int x, y, xx, yy, ww, hh;
   Eina_Iterator *it;
   Grid_Item *gi;

   EINA_SAFETY_ON_NULL_RETURN(g);

   it = eina_matrixsparse_iterator_new(g->grid);
   EINA_ITERATOR_FOREACH(it, cell)
     {
        gi = eina_matrixsparse_cell_data_get(cell);
        if (!_grid_item_in_viewport(gi)) _grid_item_unload(gi);
     }
   eina_iterator_free(it);

   _grid_viewport_get(g, &xx, &yy, &ww, &hh);
   for (y = yy; y < yy + hh; y++)
     {
        for (x = xx; x < xx + ww; x++)
          {
             gi = eina_matrixsparse_data_idx_get(g->grid, y, x);
             if (!gi) gi = _grid_item_create(g, x, y);
             _grid_item_load(gi);
          }
     }
}

static void
_grid_place(Elm_Map_Data *sd)
{
   Eina_List *l;
   Grid *g;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   EINA_LIST_FOREACH(sd->grids, l, g)
     {
        if (sd->zoom == g->zoom) _grid_load(g);
        else _grid_unload(g);
     }
   if (!sd->download_idler)
     sd->download_idler = ecore_idler_add(_download_job, sd->obj);
}

static void
_grid_all_create(Elm_Map_Data *sd)
{
   int zoom;

   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   for (zoom = sd->src_tile->zoom_min; zoom <= sd->src_tile->zoom_max; zoom++)
     {
        Grid *g;
        int tnum;

        g = ELM_NEW(Grid);
        g->wsd = sd;
        g->zoom = zoom;
        tnum = pow(2.0, g->zoom);
        g->tw = tnum;
        g->th = tnum;
        g->grid = eina_matrixsparse_new(g->th, g->tw, NULL, NULL);
        sd->grids = eina_list_append(sd->grids, g);
     }
}

static void
_grid_all_clear(Elm_Map_Data *sd)
{
   Grid *g;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   EINA_LIST_FREE(sd->grids, g)
     {
        Eina_Matrixsparse_Cell *cell;
        Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);

        EINA_ITERATOR_FOREACH(it, cell)
          {
             Grid_Item *gi;

             gi = eina_matrixsparse_cell_data_get(cell);
             if (gi) _grid_item_free(gi);
          }
        eina_iterator_free(it);

        eina_matrixsparse_free(g->grid);
        free(g);
     }
}

static void
_track_place(Elm_Map_Data *sd)
{
#ifdef ELM_EMAP
   Eina_List *l;
   Evas_Coord size;
   Evas_Object *route;
   Evas_Coord px, py, ow, oh;
   int xmin, xmax, ymin, ymax;

   px = sd->pan_x;
   py = sd->pan_y;
   _viewport_coord_get(sd, NULL, NULL, &ow, &oh);

   size = sd->size.w;

   EINA_LIST_FOREACH(sd->track, l, route)
     {
        double lon_min, lon_max;
        double lat_min, lat_max;

        elm_route_longitude_min_max_get(route, &lon_min, &lon_max);
        elm_route_latitude_min_max_get(route, &lat_min, &lat_max);
        _region_to_coord_convert(sd, lon_min, lat_max, size, &xmin, &ymin);
        _region_to_coord_convert(sd, lon_max, lat_min, size, &xmax, &ymax);

        if ( !(xmin < px && xmax < px) && !(xmin > px + ow && xmax > px + ow))
          {
             if ((ymin < py && ymax < py) ||
                 (ymin > py + oh && ymax > py + oh))
               {
                  //display the route
                  evas_object_geometry_set(route, xmin - px, ymin - py,
                                           xmax - xmin, ymax - ymin);
                  evas_object_raise(route);
                  _obj_rotate(sd, route);
                  evas_object_show(route);

                  continue;
               }
          }
        //the route is not display
        evas_object_hide(route);
     }
#else
   (void)sd;
#endif
}

static void
_calc_job(Elm_Map_Data *sd)
{
   if (sd->calc_job.zoom_mode_set)
     {
        sd->calc_job.zoom_mode_set(sd, sd->calc_job.zoom);
        sd->calc_job.zoom_mode_set = NULL;
     }
   if (!sd->zoom_animator)
     {
        if (sd->calc_job.region_show_bring_in)
          {
             sd->calc_job.region_show_bring_in
                (sd, sd->calc_job.lon, sd->calc_job.lat, sd->calc_job.bring_in);
             sd->calc_job.region_show_bring_in = NULL;
          }
        if (sd->calc_job.overlays_show)
          {
             sd->calc_job.overlays_show(sd, sd->calc_job.overlays);
             sd->calc_job.overlays_show = NULL;
          }
     }
}

static void
_smooth_update(Elm_Map_Data *sd)
{
   Eina_List *l;
   Grid *g;

   EINA_LIST_FOREACH(sd->grids, l, g)
     {
        Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);
        Eina_Matrixsparse_Cell *cell;

        EINA_ITERATOR_FOREACH(it, cell)
          {
             Grid_Item *gi = eina_matrixsparse_cell_data_get(cell);
             if (_grid_item_in_viewport(gi))
               evas_object_image_smooth_scale_set(gi->img, EINA_TRUE);
          }
        eina_iterator_free(it);
     }
}

static Eina_Bool
_zoom_timeout_cb(void *data)
{
   ELM_MAP_DATA_GET(data, sd);

   _smooth_update(sd);
   sd->zoom_timer = NULL;
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_ZOOM_STOP, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_zoom_do(Elm_Map_Data *sd,
         double zoom)
{
   Evas_Coord vx, vy, vw, vh;
   Evas_Coord ow, oh;

   if (zoom > sd->src_tile->zoom_max) zoom = sd->src_tile->zoom_max;
   else if (zoom < sd->src_tile->zoom_min)
     zoom = sd->src_tile->zoom_min;
   if (zoom > sd->zoom_max) zoom = sd->zoom_max;
   else if (zoom < sd->zoom_min)
     zoom = sd->zoom_min;

   sd->zoom = ROUND(zoom);
   sd->zoom_detail = zoom;
   ow = sd->size.w;
   oh = sd->size.h;
   sd->size.tile = pow(2.0, (zoom - sd->zoom)) * sd->tsize;
   sd->size.w = pow(2.0, sd->zoom) * sd->size.tile;
   sd->size.h = sd->size.w;

   // Fix to zooming with (viewport center vx, vy) as the center to prevent
   // from zooming with (0,0) as the center. (scroller default behavior)
   _viewport_coord_get(sd, &vx, &vy, &vw, &vh);
   if ((vw > 0) && (vh > 0) && (ow > 0) && (oh > 0))
     {
        Evas_Coord x, y;
        double sx, sy;

        if (vw > ow) sx = 0.5;
        else sx = (vx + ((double)vw / 2)) / ow;
        if (vh > oh) sy = 0.5;
        else sy = (vy + ((double)vh / 2)) / oh;

        if (sx > 1.0) sx = 1.0;
        if (sy > 1.0) sy = 1.0;

        x = ceil((sx * sd->size.w) - ((double)vw / 2));
        y = ceil((sy * sd->size.h) - ((double)vh / 2));
        if (x < 0) x = 0;
        else if (x > (sd->size.w - vw))
          x = sd->size.w - vw;
        if (y < 0) y = 0;
        else if (y > (sd->size.h - vh))
          y = sd->size.h - vh;
        elm_interface_scrollable_content_region_show
              (sd->obj, x, y, vw, vh);
     }

   if (sd->zoom_timer)
     {
        ecore_timer_del(sd->zoom_timer);
        sd->zoom_timer = NULL;
     }
   else
      efl_event_callback_legacy_call
        (sd->obj, EFL_UI_EVENT_ZOOM_START, NULL);

   if (sd->obj)
     sd->zoom_timer = ecore_timer_add(0.25, _zoom_timeout_cb, sd->obj);
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_ZOOM_CHANGE, NULL);

   efl_event_callback_legacy_call
     (sd->pan_obj, ELM_PAN_EVENT_CHANGED, NULL);
   evas_object_smart_changed(sd->pan_obj);
}

static void
_zoom_anim_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_MAP_DATA_GET(data, sd);

   if (sd->ani.zoom_cnt <= 0)
     {
        _zoom_animator_set(sd, NULL);
        evas_object_smart_changed(sd->pan_obj);
        _calc_job(sd);
     }
   else
     {
        sd->ani.zoom += sd->ani.zoom_diff;
        sd->ani.zoom_cnt--;
        _zoom_do(sd, sd->ani.zoom);
     }
}

static void
_zoom_bring_anim_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_MAP_DATA_GET(data, sd);

   if ((sd->ani.zoom_cnt <= 0) && (sd->ani.region_cnt <= 0))
     {
        _zoom_animator_set(sd, NULL);
        evas_object_smart_changed(sd->pan_obj);
        _calc_job(sd);
     }
   else
     {
        Evas_Coord x, y, w, h;
        if (sd->ani.zoom_cnt > 0)
          {
             sd->ani.zoom += sd->ani.zoom_diff;
             _zoom_do(sd, sd->ani.zoom);
             sd->ani.zoom_cnt--;
          }
        if (sd->ani.region_cnt > 0)
          {
             sd->ani.lon += sd->ani.lon_diff;
             sd->ani.lat += sd->ani.lat_diff;

             _region_to_coord_convert
                (sd, sd->ani.lon, sd->ani.lat, sd->size.w, &x, &y);
             _viewport_coord_get(sd, NULL, NULL, &w, &h);
             x = x - (w / 2);
             y = y - (h / 2);
             elm_interface_scrollable_content_region_show(sd->obj, x, y, w, h);
             sd->ani.region_cnt--;
          }
     }
}

static Eina_Bool
_zoom_animator_set(Elm_Map_Data *sd,
                   void *callback)
{
   Eina_Bool r = EINA_FALSE;

   sd->zoom_animator = !!callback;
   r = efl_event_callback_del(sd->obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_anim_cb, sd->obj);
   r |= efl_event_callback_del(sd->obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_bring_anim_cb, sd->obj);
   if (callback) efl_event_callback_add(sd->obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, callback, sd->obj);

   return r;
}

static void
_zoom_with_animation(Elm_Map_Data *sd,
                     double zoom,
                     int cnt)
{
   if (cnt == 0) return;

   sd->ani.zoom_cnt = cnt;
   sd->ani.zoom = sd->zoom;
   sd->ani.zoom_diff = (double)(zoom - sd->zoom) / cnt;
   _zoom_animator_set(sd, _zoom_anim_cb);
}

static void
_zoom_bring_with_animation(Elm_Map_Data *sd,
                           double zoom,
                           double lon,
                           double lat,
                           int zoom_cnt,
                           int region_cnt)
{
   double tlon, tlat;
   Evas_Coord vx, vy, vw, vh;
   if ((zoom_cnt == 0) || (region_cnt == 0)) return;

   sd->ani.zoom_cnt = zoom_cnt;
   sd->ani.zoom = sd->zoom;
   sd->ani.zoom_diff = (double)(zoom - sd->zoom) / zoom_cnt;

   sd->ani.region_cnt = region_cnt;
   _viewport_coord_get(sd, &vx, &vy, &vw, &vh);
   _coord_to_region_convert
     (sd, vx + vw / 2, vy + vh / 2, sd->size.w, &tlon, &tlat);
   sd->ani.lon = tlon;
   sd->ani.lat = tlat;
   sd->ani.lon_diff = (lon - tlon) / region_cnt;
   sd->ani.lat_diff = (lat - tlat) / region_cnt;

   _zoom_animator_set(sd, _zoom_bring_anim_cb);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord maxw = -1, maxh = -1;

   evas_object_size_hint_max_get(obj, &maxw, &maxh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

static Eina_Bool
_scr_timeout_cb(void *data)
{
   ELM_MAP_DATA_GET(data, sd);

   _smooth_update(sd);
   sd->scr_timer = NULL;
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_scroll_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   ELM_MAP_DATA_GET(obj, sd);

   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   else
      efl_event_callback_legacy_call
        (sd->obj, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);
   sd->scr_timer = ecore_timer_add(0.25, _scr_timeout_cb, obj);
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_SCROLL, NULL);
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data EINA_UNUSED)
{
   ELM_MAP_DATA_GET(obj, sd);

   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data EINA_UNUSED)
{
   ELM_MAP_DATA_GET(obj, sd);

   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_SCROLL_ANIM_STOP, NULL);
}

static Eina_Bool
_long_press_cb(void *data)
{
   ELM_MAP_DATA_GET(data, sd);

   sd->long_timer = NULL;
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_LONGPRESSED, &sd->ev);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   ELM_MAP_DATA_GET(data, sd);
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     efl_event_callback_legacy_call
       (sd->obj, EFL_UI_EVENT_CLICKED_DOUBLE, ev);
   else
     efl_event_callback_legacy_call
       (sd->obj, ELM_MAP_EVENT_PRESS, ev);

   ecore_timer_del(sd->long_timer);
   sd->ev = *ev;
   sd->long_timer =
     ecore_timer_add(_elm_config->longpress_timeout, _long_press_cb, data);
}

static void
_mouse_up_cb(void *data,
             Evas *evas EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   ELM_MAP_DATA_GET(data, sd);
   Evas_Event_Mouse_Up *ev = event_info;

   EINA_SAFETY_ON_NULL_RETURN(ev);

   if (ev->button != 1) return;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);

   if (!sd->on_hold)
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_EVENT_CLICKED, ev);
   sd->on_hold = EINA_FALSE;
}

static void
_mouse_wheel_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   ELM_MAP_DATA_GET(data, sd);

   if (!sd->paused)
     {
        Evas_Event_Mouse_Wheel *ev = event_info;

        _zoom_do(sd, sd->zoom_detail - ((double)ev->z / 10));
     }
}

static void
_region_max_min_get(Eina_List *overlays,
                    double *max_longitude,
                    double *min_longitude,
                    double *max_latitude,
                    double *min_latitude)
{
   double max_lon = -180, min_lon = 180;
   double max_lat = -90, min_lat = 90;
   Elm_Map_Overlay *overlay;
   Eina_List *l;

   EINA_LIST_FOREACH(overlays, l, overlay)
     {
        double lon, lat;

        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          {
             Overlay_Default *ovl = overlay->ovl;

             lon = ovl->lon;
             lat = ovl->lat;
          }
        else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
          {
             // FIXME: class center coord is alright??
             Overlay_Class *ovl = overlay->ovl;
             double max_lo, min_lo, max_la, min_la;

             _region_max_min_get
               (ovl->members, &max_lo, &min_lo, &max_la, &min_la);
             lon = (max_lo + min_lo) / 2;
             lat = (max_la + min_la) / 2;
          }
        else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
          {
             Overlay_Bubble *ovl = overlay->ovl;

             lon = ovl->lon;
             lat = ovl->lat;
          }
        else
          {
             WRN("Not supported overlay type: %d", overlay->type);
             continue;
          }
        if (lon > max_lon) max_lon = lon;
        if (lon < min_lon) min_lon = lon;
        if (lat > max_lat) max_lat = lat;
        if (lat < min_lat) min_lat = lat;
     }

   if (max_longitude) *max_longitude = max_lon;
   if (min_longitude) *min_longitude = min_lon;
   if (max_latitude) *max_latitude = max_lat;
   if (min_latitude) *min_latitude = min_lat;
}

static Evas_Object *
_icon_dup(Evas_Object *icon,
          Evas_Object *parent)
{
   Evas_Object *dupp;
   Evas_Coord w, h;

   if (!icon || !parent) return NULL;
   dupp = evas_object_image_filled_add(evas_object_evas_get(parent));
   evas_object_image_source_set(dupp, icon);
   // Set size as origin' size for proxy
   evas_object_geometry_get(icon, NULL, NULL, &w, &h);
   if (w <= 0 || h <= 0)
     {
        evas_object_size_hint_combined_min_get(icon, &w, &h);
        evas_object_size_hint_min_set(dupp, w, h);
     }
   else evas_object_resize(dupp, w, h);
   // Original should have size for proxy
   evas_object_resize(icon, w, h);

   return dupp;
}

static void
_overlay_clicked_cb(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void *ev EINA_UNUSED)
{
   Elm_Map_Overlay *overlay = data;

   EINA_SAFETY_ON_NULL_RETURN(data);

   efl_event_callback_legacy_call
     ((overlay->wsd)->obj, ELM_MAP_EVENT_OVERLAY_CLICKED, overlay);
   if (overlay->cb)
     overlay->cb(overlay->cb_data, (overlay->wsd)->obj, overlay);
}

static void
_overlay_default_hide(Overlay_Default *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->content) evas_object_hide(ovl->content);
   if (ovl->icon) evas_object_hide(ovl->icon);
   if (ovl->clas_content) evas_object_hide(ovl->clas_content);
   if (ovl->clas_icon) evas_object_hide(ovl->clas_icon);
   if (ovl->layout) evas_object_hide(ovl->layout);
}

static void
_overlay_default_show(Overlay_Default *ovl)
{
   Evas_Object *disp;
   Evas_Coord x, y, w, h;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_hide(ovl->layout);
   if (ovl->content)
     {
        disp = ovl->content;
        evas_object_geometry_get(disp, NULL, NULL, &w, &h);
        if (w <= 0 || h <= 0) evas_object_size_hint_combined_min_get(disp, &w, &h);
        ovl->w = w;
        ovl->h = h;
     }
   else if (!(ovl->icon) && ovl->clas_content)
     {
        disp = ovl->clas_content;

        evas_object_geometry_get(disp, NULL, NULL, &w, &h);
        if (w <= 0 || h <= 0) evas_object_size_hint_combined_min_get(disp, &w, &h);
     }
   else
     {
        if (ovl->icon) evas_object_show(ovl->icon);
        else if (ovl->clas_icon)
          evas_object_show(ovl->clas_icon);
        disp = ovl->layout;
        w = ovl->w;
        h = ovl->h;
     }
   _coord_to_canvas(ovl->wsd, ovl->x, ovl->y, &x, &y);
   _obj_place(disp, x - (w / 2), y - (h / 2), w, h);
}

static void
_overlay_default_coord_get(Overlay_Default *ovl,
                           Evas_Coord *x,
                           Evas_Coord *y,
                           Evas_Coord *w,
                           Evas_Coord *h)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (x) *x = ovl->x;
   if (y) *y = ovl->y;
   if (w) *w = ovl->w;
   if (h) *h = ovl->h;
}

static void
_overlay_default_coord_set(Overlay_Default *ovl,
                           Evas_Coord x,
                           Evas_Coord y)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   ovl->x = x;
   ovl->y = y;
}

static void
_overlay_default_coord_update(Overlay_Default *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   _region_to_coord_convert
     (ovl->wsd, ovl->lon, ovl->lat, ovl->wsd->size.w, &ovl->x, &ovl->y);
}

static void
_overlay_default_layout_text_update(Overlay_Default *ovl,
                                    const char *text)
{
   if (!ovl->content && !ovl->icon && !ovl->clas_content && !ovl->clas_icon)
     elm_layout_text_set(ovl->layout, "elm.text", text);
}

static void
_overlay_default_content_update(Overlay_Default *ovl,
                                Evas_Object *content,
                                Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->content == content) return;
   evas_object_del(ovl->content);
   ovl->content = content;
   evas_object_smart_member_add(ovl->content, ovl->wsd->pan_obj);
   evas_object_stack_above(ovl->content, ovl->wsd->sep_maps_overlays);

   if (ovl->content)
     evas_object_event_callback_add(ovl->content, EVAS_CALLBACK_MOUSE_DOWN,
                                    _overlay_clicked_cb, overlay);
}

static void
_overlay_default_layout_update(Overlay_Default *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->icon)
     {
        evas_object_color_set(ovl->layout, 255, 255, 255, 255);
        if (!elm_layout_theme_set
            (ovl->layout, "map/marker", "empty",
             elm_widget_style_get((ovl->wsd)->obj)))
          CRI("Failed to set layout!");

        elm_object_part_content_set(ovl->layout, "elm.icon", ovl->icon);
     }
   else if (!ovl->icon && ovl->clas_icon)
     {
        evas_object_color_set(ovl->layout, 255, 255, 255, 255);
        if (!elm_layout_theme_set
            (ovl->layout, "map/marker", "empty",
             elm_widget_style_get((ovl->wsd)->obj)))
          CRI("Failed to set layout!");

        elm_object_part_content_set(ovl->layout, "elm.icon", ovl->clas_icon);
     }
   else
     {
        evas_object_color_set
          (ovl->layout, ovl->c.r, ovl->c.g, ovl->c.b, ovl->c.a);
        if (!elm_layout_theme_set
            (ovl->layout, "map/marker", "radio",
             elm_widget_style_get((ovl->wsd)->obj)))
          CRI("Failed to set layout!");
     }
}

static void
_overlay_default_class_content_update(Overlay_Default *ovl,
                                      Evas_Object *content)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->clas_content);
   ovl->clas_content = _icon_dup(content, ovl->layout);
   _overlay_default_layout_update(ovl);
}

static void
_overlay_default_icon_update(Overlay_Default *ovl,
                             Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->icon == icon) return;

   evas_object_del(ovl->icon);
   ovl->icon = icon;
   _overlay_default_layout_update(ovl);
}

static void
_overlay_default_class_icon_update(Overlay_Default *ovl,
                                   Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->clas_icon);
   ovl->clas_icon = _icon_dup(icon, ovl->layout);
   _overlay_default_layout_update(ovl);
}

static void
_overlay_default_color_update(Overlay_Default *ovl,
                              Color c)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   ovl->c = c;
   _overlay_default_layout_update(ovl);
}

static void
_overlay_default_free(Overlay_Default *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->content);
   evas_object_del(ovl->icon);
   evas_object_del(ovl->clas_content);
   evas_object_del(ovl->clas_icon);
   evas_object_del(ovl->layout);

   free(ovl);
}

static Overlay_Default *
_overlay_default_new(Elm_Map_Overlay *overlay,
                     double lon,
                     double lat,
                     Color c,
                     double scale)
{
   Overlay_Default *ovl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);

   ovl = ELM_NEW(Overlay_Default);
   ovl->wsd = overlay->wsd;
   _edj_overlay_size_get(ovl->wsd, &(ovl->w), &(ovl->h));
   ovl->w *= scale;
   ovl->h *= scale;
   ovl->layout = elm_layout_add((ovl->wsd)->obj);
   evas_object_smart_member_add(ovl->layout, ovl->wsd->pan_obj);
   evas_object_stack_above(ovl->layout, ovl->wsd->sep_maps_overlays);
   if (!elm_layout_theme_set(ovl->layout, "map/marker", "radio",
                             elm_widget_style_get((ovl->wsd)->obj)))
     CRI("Failed to set layout!");

   evas_object_event_callback_add(ovl->layout, EVAS_CALLBACK_MOUSE_DOWN,
                                  _overlay_clicked_cb, overlay);
   ovl->lon = lon;
   ovl->lat = lat;
   _overlay_default_color_update(ovl, c);

   return ovl;
}

static void
_overlay_group_hide(Overlay_Group *grp)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   if (grp->ovl) _overlay_default_hide(grp->ovl);
}

static void
_overlay_group_show(Overlay_Group *grp)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   if (grp->ovl) _overlay_default_show(grp->ovl);
}

static void
_overlay_group_coord_member_update(Overlay_Group *grp,
                                   Evas_Coord x,
                                   Evas_Coord y,
                                   Eina_List *members)
{
   char text[32];

   EINA_SAFETY_ON_NULL_RETURN(grp);

   if (!grp->ovl) return;

   _overlay_default_coord_set(grp->ovl, x, y);
   _coord_to_region_convert
     (grp->wsd, x, y, grp->wsd->size.w, &grp->lon, &grp->lat);

   eina_list_free(grp->members);
   grp->members = members;
   snprintf(text, sizeof(text), "%d", eina_list_count(members));

   _overlay_default_layout_text_update(grp->ovl, text);
}

static void
_overlay_group_icon_update(Overlay_Group *grp,
                           Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   if (grp->ovl)
     _overlay_default_icon_update
       (grp->ovl, _icon_dup(icon, (grp->wsd)->obj));
}

static void
_overlay_group_content_update(Overlay_Group *grp,
                              Evas_Object *content,
                              Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   _overlay_default_content_update
      (grp->ovl, _icon_dup(content, grp->wsd->obj), overlay);

   return;
}

static void
_overlay_group_color_update(Overlay_Group *grp,
                            Color c)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   _overlay_default_color_update(grp->ovl, c);
}

static void
_overlay_group_cb_set(Overlay_Group *grp,
                      Elm_Map_Overlay_Get_Cb cb,
                      void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   grp->overlay->cb = cb;
   grp->overlay->data = data;
}

static void
_overlay_group_free(Overlay_Group *grp)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   free(grp->overlay);
   if (grp->ovl) _overlay_default_free(grp->ovl);
   eina_list_free(grp->members);

   free(grp);
}

static Overlay_Group *
_overlay_group_new(Elm_Map_Data *sd)
{
   Overlay_Group *grp;
   Color c = {0x90, 0xee, 0x90, 0xff};

   grp = ELM_NEW(Overlay_Group);
   grp->wsd = sd;
   grp->overlay = ELM_NEW(Elm_Map_Overlay);  // this is a virtual overlay
   grp->overlay->wsd = sd;
   grp->overlay->type = ELM_MAP_OVERLAY_TYPE_GROUP;
   grp->overlay->ovl = grp;
   grp->ovl = _overlay_default_new(grp->overlay, -1, -1, c, 2);

   return grp;
}

static void
_overlay_class_cb_set(Overlay_Class *ovl,
                      Elm_Map_Overlay_Get_Cb cb,
                      void *data)
{
   Eina_List *l;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   // Update class members' callbacks
   EINA_LIST_FOREACH(ovl->members, l, overlay)
     _overlay_group_cb_set(overlay->grp, cb, data);
}

static void
_overlay_class_icon_update(Overlay_Class *ovl,
                           Evas_Object *icon)
{
   Eina_List *l;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->icon == icon) return;
   evas_object_del(ovl->icon);
   ovl->icon = icon;
   // For using proxy, it should have size and be shown but moved away to hide.
   evas_object_geometry_set(icon, -9999, -9999, 32, 32);
   evas_object_show(icon);

   // Update class members' class icons
   EINA_LIST_FOREACH(ovl->members, l, overlay)
     {
        _overlay_group_icon_update(overlay->grp, icon);

        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          _overlay_default_class_icon_update(overlay->ovl, icon);
     }
}

static void
_overlay_class_content_update(Overlay_Class *ovl,
                              Evas_Object *content)
{
   Eina_List *l;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->content == content) return;
   evas_object_del(ovl->content);
   ovl->content = content;
   // For using proxy, it should have size and be shown but moved away to hide.
   // content should have it's own size
   evas_object_move(content, -9999, -9999);

   // Update class members' class contents
   EINA_LIST_FOREACH(ovl->members, l, overlay)
     {
        _overlay_group_content_update(overlay->grp, content, overlay);

        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          _overlay_default_class_content_update(overlay->ovl, content);
     }
}

static void
_overlay_class_color_update(Overlay_Class *ovl,
                            Color c)
{
   Eina_List *l;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   // Update class members' class contents
   EINA_LIST_FOREACH(ovl->members, l, overlay)
     _overlay_group_color_update(overlay->grp, c);
}

static void
_overlay_class_free(Overlay_Class *clas)
{
   Eina_List *l;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(clas);

   // Update class members' class contents
   EINA_LIST_FOREACH(clas->members, l, overlay)
     {
        overlay->grp->klass = NULL;
        _overlay_group_content_update(overlay->grp, NULL, NULL);
        _overlay_group_icon_update(overlay->grp, NULL);

        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          {
             _overlay_default_class_content_update(overlay->ovl, NULL);
             _overlay_default_class_icon_update(overlay->ovl, NULL);
          }
     }
   evas_object_del(clas->icon);
   eina_list_free(clas->members);

   free(clas);
}

static Overlay_Class *
_overlay_class_new(Elm_Map_Data *sd)
{
   Overlay_Class *ovl;

   ovl = ELM_NEW(Overlay_Class);
   ovl->wsd = sd;
   ovl->icon = NULL;
   ovl->zoom_max = OVERLAY_CLASS_ZOOM_MAX;

   return ovl;
}

static void
_overlay_bubble_coord_update(Overlay_Bubble *bubble)
{
   EINA_SAFETY_ON_NULL_RETURN(bubble);

   if (bubble->pobj)
     {
        Evas_Coord x, y, w, h;

        evas_object_geometry_get(bubble->pobj, &x, &y, &w, &h);
        bubble->x = x + (w / 2);
        bubble->y = y - (bubble->h / 2);
        _canvas_to_coord
          (bubble->wsd, bubble->x, bubble->y, &(bubble->x), &(bubble->y));
        _coord_to_region_convert
          (bubble->wsd, bubble->x, bubble->y, bubble->wsd->size.w,
          &(bubble->lon), &(bubble->lat));
     }
   else
     {
        _region_to_coord_convert(bubble->wsd, bubble->lon, bubble->lat,
                                 bubble->wsd->size.w, &bubble->x, &bubble->y);
     }
}

static void
_overlay_bubble_coord_get(Overlay_Bubble *bubble,
                          Evas_Coord *x,
                          Evas_Coord *y,
                          Evas_Coord *w,
                          Evas_Coord *h)
{
   EINA_SAFETY_ON_NULL_RETURN(bubble);

   if (!(bubble->pobj))
     {
        if (x) *x = bubble->x;
        if (y) *y = bubble->y;
        if (w) *w = bubble->w;
        if (h) *h = bubble->h;
     }
   else
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
     }
}

static Eina_Bool
_overlay_bubble_show_hide(Overlay_Bubble *bubble,
                          Eina_Bool visible)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(bubble, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bubble->obj, EINA_FALSE);

   if (!visible) evas_object_hide(bubble->obj);
   else if (bubble->pobj && !evas_object_visible_get(bubble->pobj))
     {
        evas_object_hide(bubble->obj);
        visible = EINA_FALSE;
     }
   else
     {
        _coord_to_canvas
          (bubble->wsd, bubble->x, bubble->y, &(bubble->x), &(bubble->y));
        _obj_place(bubble->obj, bubble->x - (bubble->w / 2),
                   bubble->y - (bubble->h / 2), bubble->w, bubble->h);
        evas_object_raise(bubble->obj);
     }
   return visible;
}

static void
_overlay_bubble_free(Overlay_Bubble *bubble)
{
   EINA_SAFETY_ON_NULL_RETURN(bubble);

   evas_object_del(bubble->bx);
   evas_object_del(bubble->sc);
   evas_object_del(bubble->obj);

   free(bubble);
}

static Overlay_Bubble *
_overlay_bubble_new(Elm_Map_Overlay *overlay)
{
   Evas_Coord h;
   const char *s;
   Overlay_Bubble *bubble;

   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);

   bubble = ELM_NEW(Overlay_Bubble);
   bubble->wsd = overlay->wsd;

   bubble->obj =
     edje_object_add(evas_object_evas_get((overlay->wsd)->obj));
   elm_widget_theme_object_set
     ((overlay->wsd)->obj, bubble->obj, "map", "marker_bubble",
     elm_widget_style_get((overlay->wsd)->obj));

   evas_object_event_callback_add(bubble->obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _overlay_clicked_cb, overlay);

   bubble->sc = elm_scroller_add(bubble->obj);
   elm_widget_style_set(bubble->sc, "map_bubble");
   elm_scroller_content_min_limit(bubble->sc, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set
     (bubble->sc, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
   elm_scroller_bounce_set
     (bubble->sc, _elm_config->thumbscroll_bounce_enable, EINA_FALSE);
   edje_object_part_swallow(bubble->obj, "elm.swallow.content", bubble->sc);

   bubble->bx = elm_box_add(bubble->sc);
   evas_object_size_hint_align_set(bubble->bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set
     (bubble->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bubble->bx, EINA_TRUE);
   elm_object_content_set(bubble->sc, bubble->bx);

   s = edje_object_data_get(bubble->obj, "size_w");
   if (s) bubble->w = atoi(s);
   else bubble->w = 0;

   edje_object_size_min_calc(bubble->obj, NULL, &(bubble->h));
   s = edje_object_data_get(bubble->obj, "size_h");
   if (s) h = atoi(s);
   else h = 0;

   if (bubble->h < h) bubble->h = h;

   bubble->lon = -1;
   bubble->lat = -1;
   bubble->x = -1;
   bubble->y = -1;

   return bubble;
}

static void
_overlay_route_color_update(Overlay_Route *ovl,
                            Color c)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_color_set(ovl->obj, c.r, c.g, c.b, c.a);
}

static void
_overlay_route_hide(Overlay_Route *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_hide(ovl->obj);
}

static void
_overlay_route_show(Overlay_Route *r)
{
   Path *p;
   Eina_List *l;
   Path_Node *n;

   EINA_SAFETY_ON_NULL_RETURN(r);
   EINA_SAFETY_ON_NULL_RETURN(r->wsd);

   evas_object_polygon_points_clear(r->obj);
   EINA_LIST_FOREACH(r->nodes, l, n)
     {
        p = eina_list_nth(r->paths, n->idx);
        if (!p) continue;

        _region_to_coord_convert
          (r->wsd, n->pos.lon, n->pos.lat, r->wsd->size.w, &p->x, &p->y);
        _coord_to_canvas(r->wsd, p->x, p->y, &p->x, &p->y);
     }
   EINA_LIST_FOREACH(r->paths, l, p)
     evas_object_polygon_point_add(r->obj, p->x - 3, p->y - 3);

   EINA_LIST_REVERSE_FOREACH(r->paths, l, p)
     evas_object_polygon_point_add(r->obj, p->x + 3, p->y + 3);

   evas_object_show(r->obj);
}

static void
_overlay_route_free(Overlay_Route *route)
{
   Path *p;
   Path_Node *n;

   EINA_SAFETY_ON_NULL_RETURN(route);

   evas_object_del(route->obj);

   EINA_LIST_FREE(route->paths, p)
     free(p);

   EINA_LIST_FREE(route->nodes, n)
     {
        eina_stringshare_del(n->pos.address);
        free(n);
     }

   free(route);
}

static Overlay_Route *
_overlay_route_new(Elm_Map_Data *sd,
                   const Elm_Map_Route *route,
                   Color c)
{
   Eina_List *l;
   Path_Node *n;
   Overlay_Route *ovl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);

   ovl = ELM_NEW(Overlay_Route);
   ovl->wsd = sd;
   ovl->obj =
     evas_object_polygon_add(evas_object_evas_get(sd->obj));
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   _overlay_route_color_update(ovl, c);

   EINA_LIST_FOREACH(route->nodes, l, n)
     {
        Path *path;
        Path_Node *node;

        node = ELM_NEW(Path_Node);
        node->idx = n->idx;
        node->pos.lon = n->pos.lon;
        node->pos.lat = n->pos.lat;
        if (n->pos.address) node->pos.address = strdup(n->pos.address);
        ovl->nodes = eina_list_append(ovl->nodes, node);

        path = ELM_NEW(Path);
        ovl->paths = eina_list_append(ovl->paths, path);
     }

   return ovl;
}

static void
_overlay_line_color_update(Overlay_Line *ovl,
                           Color c)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_color_set(ovl->obj, c.r, c.g, c.b, c.a);
}

static void
_overlay_line_hide(Overlay_Line *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->obj) evas_object_hide(ovl->obj);
}

static void
_overlay_line_show(Overlay_Line *ovl)
{
   Evas_Coord fx, fy, tx, ty;
   Elm_Map_Data *sd = ovl->wsd;

   _region_to_coord_convert(sd, ovl->flon, ovl->flat, sd->size.w, &fx, &fy);
   _region_to_coord_convert(sd, ovl->tlon, ovl->tlat, sd->size.w, &tx, &ty);
   _coord_to_canvas(sd, fx, fy, &fx, &fy);
   _coord_to_canvas(sd, tx, ty, &tx, &ty);
   evas_object_line_xy_set(ovl->obj, fx, fy, tx, ty);
   evas_object_show(ovl->obj);
}

static void
_overlay_line_free(Overlay_Line *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->obj);
   free(ovl);
}

static Overlay_Line *
_overlay_line_new(Elm_Map_Data *sd,
                  double flon,
                  double flat,
                  double tlon,
                  double tlat,
                  Color c)
{
   Overlay_Line *ovl;

   ovl = ELM_NEW(Overlay_Line);
   ovl->wsd = sd;
   ovl->flon = flon;
   ovl->flat = flat;
   ovl->tlon = tlon;
   ovl->tlat = tlat;
   ovl->obj =
     evas_object_line_add(evas_object_evas_get(sd->obj));
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   _overlay_line_color_update(ovl, c);

   return ovl;
}

static void
_overlay_polygon_color_update(Overlay_Polygon *ovl,
                              Color c)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_color_set(ovl->obj, c.r, c.g, c.b, c.a);
}

static void
_overlay_polygon_hide(Overlay_Polygon *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->obj) evas_object_hide(ovl->obj);
}

static void
_overlay_polygon_show(Overlay_Polygon *ovl)
{
   Eina_List *l;
   Region *r;
   Elm_Map_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   sd = ovl->wsd;

   evas_object_polygon_points_clear(ovl->obj);
   EINA_LIST_FOREACH(ovl->regions, l, r)
     {
        Evas_Coord x, y;

        _region_to_coord_convert(sd, r->lon, r->lat, sd->size.w, &x, &y);
        _coord_to_canvas(sd, x, y, &x, &y);
        evas_object_polygon_point_add(ovl->obj, x, y);
     }
   evas_object_show(ovl->obj);
}

static void
_overlay_polygon_free(Overlay_Polygon *ovl)
{
   Region *r;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->obj);

   EINA_LIST_FREE(ovl->regions, r)
     free(r);

   free(ovl);
}

static Overlay_Polygon *
_overlay_polygon_new(Elm_Map_Data *sd, Color c)
{
   Overlay_Polygon *ovl;

   ovl = ELM_NEW(Overlay_Polygon);
   ovl->wsd = sd;
   ovl->obj =
     evas_object_polygon_add(evas_object_evas_get(sd->obj));
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   _overlay_polygon_color_update(ovl, c);

   return ovl;
}

static void
_overlay_circle_color_update(Overlay_Circle *ovl,
                             Color c)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   obj = elm_layout_edje_get(ovl->obj);
   evas_object_color_set(obj, c.r, c.g, c.b, c.a);
}

static void
_overlay_circle_hide(Overlay_Circle *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->obj) evas_object_hide(ovl->obj);
}

static void
_overlay_circle_show(Overlay_Circle *ovl)
{
   double r;
   Evas_Coord x, y;
   Elm_Map_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   sd = ovl->wsd;

   r = (ovl->ratio) * sd->size.w;
   _region_to_coord_convert(sd, ovl->lon, ovl->lat, sd->size.w, &x, &y);
   _coord_to_canvas(sd, x, y, &x, &y);
   _obj_place(ovl->obj, x - r, y - r, r * 2, r * 2);
}

static void
_overlay_circle_free(Overlay_Circle *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->obj);
   free(ovl);
}

static Overlay_Circle *
_overlay_circle_new(Elm_Map_Data *sd,
                    double lon,
                    double lat,
                    double radius,
                    Color c)
{
   Overlay_Circle *ovl;

   ovl = ELM_NEW(Overlay_Circle);
   ovl->wsd = sd;
   ovl->lon = lon;
   ovl->lat = lat;
   ovl->radius = radius;
   ovl->ratio = radius / sd->size.w;

   ovl->obj = elm_layout_add(sd->obj);
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   evas_object_stack_above(ovl->obj, sd->sep_maps_overlays);
   if (!elm_layout_theme_set(ovl->obj, "map/circle", "base",
                             elm_widget_style_get(sd->obj)))
     CRI("Failed to set layout!");

   _overlay_circle_color_update(ovl, c);

   return ovl;
}

static void
_overlay_scale_color_update(Overlay_Scale *ovl,
                            Color c)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_color_set(ovl->obj, c.r, c.g, c.b, c.a);
}

static void
_overlay_scale_hide(Overlay_Scale *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->obj) evas_object_hide(ovl->obj);
}

static void
_overlay_scale_show(Overlay_Scale *ovl)
{
   double text;
   Evas_Coord w;
   char buf[32];
   double meter;
   double lon, lat;
   Elm_Map_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   sd = ovl->wsd;

   if ((int)(sizeof(_scale_tb)/sizeof(double)) <= sd->zoom)
     {
        ERR("Zoom level is too high");
        return;
     }

   elm_map_region_get(sd->obj, &lon, &lat);
   meter =
     sd->src_tile->scale_cb(sd->obj, lon, lat, sd->zoom);

   w = (_scale_tb[sd->zoom] / meter) * (sd->zoom_detail - sd->zoom + 1);

   text = _scale_tb[sd->zoom] / 1000;
   if (text < 1) snprintf(buf, sizeof(buf), "%d m", (int)(text * 1000));
   else snprintf(buf, sizeof(buf), "%d km", (int)text);

   elm_layout_text_set(ovl->obj, "elm.text", buf);
   _obj_place(ovl->obj, ovl->x, ovl->y, w, ovl->h);
}

static void
_overlay_scale_free(Overlay_Scale *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   evas_object_del(ovl->obj);
   free(ovl);
}

static Overlay_Scale *
_overlay_scale_new(Elm_Map_Data *sd,
                   Evas_Coord x,
                   Evas_Coord y,
                   Color c)
{
   const char *s;
   Overlay_Scale *ovl;

   ovl = ELM_NEW(Overlay_Scale);
   ovl->wsd = sd;
   ovl->x = x;
   ovl->y = y;

   ovl->obj = elm_layout_add(sd->obj);
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   evas_object_stack_above(ovl->obj, sd->sep_maps_overlays);
   if (!elm_layout_theme_set(ovl->obj, "map/scale", "base",
                             elm_widget_style_get(sd->obj)))
     CRI("Failed to set layout!");

   s = edje_object_data_get(elm_layout_edje_get(ovl->obj), "size_w");

   if (s) ovl->w = atoi(s);
   else ovl->w = 100;

   s = edje_object_data_get(elm_layout_edje_get(ovl->obj), "size_h");
   if (s) ovl->h = atoi(s);
   else ovl->h = 60;

   _overlay_scale_color_update(ovl, c);

   return ovl;
}

static void
_overlay_grouping(Eina_List *clas_membs,
                  Elm_Map_Overlay *boss)
{
   Eina_List *l;
   Elm_Map_Overlay *memb;
   Eina_List *grp_membs = NULL;
   int sum_x = 0, sum_y = 0, cnt = 0;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;

   EINA_SAFETY_ON_NULL_RETURN(clas_membs);
   EINA_SAFETY_ON_NULL_RETURN(boss);

   if (boss->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_coord_get(boss->ovl, &bx, &by, &bw, &bh);
   else if (boss->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
     _overlay_bubble_coord_get(boss->ovl, &bx, &by, &bw, &bh);

   EINA_LIST_FOREACH(clas_membs, l, memb)
     {
        Evas_Coord x = 0, y = 0, w = 0, h = 0;

        if (boss == memb || memb->grp->in) continue;
        if ((memb->hide) || (memb->zoom_min > memb->wsd->zoom)) continue;

        if (memb->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          _overlay_default_coord_get(memb->ovl, &x, &y, &w, &h);
        else if (memb->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
          _overlay_bubble_coord_get(memb->ovl, &x, &y, &w, &h);

        if (bw <= 0 || bh <= 0 || w <= 0 || h <= 0) continue;
        if (ELM_RECTS_INTERSECT(x, y, w, h, bx, by,
                                bw * OVERLAY_GROUPING_SCALE,
                                bh * OVERLAY_GROUPING_SCALE))
          {
             // Join group.
             memb->grp->boss = EINA_FALSE;
             memb->grp->in = EINA_TRUE;
             sum_x += x;
             sum_y += y;
             cnt++;
             grp_membs = eina_list_append(grp_membs, memb);
          }
     }

   if (cnt >= 1)
     {
        // Mark as boss
        boss->grp->boss = EINA_TRUE;
        boss->grp->in = EINA_TRUE;
        sum_x = (sum_x + bx) / (cnt + 1);
        sum_y = (sum_y + by) / (cnt + 1);
        grp_membs = eina_list_append(grp_membs, boss);
        _overlay_group_coord_member_update(boss->grp, sum_x, sum_y, grp_membs);

        // Append group to all overlay list
        boss->wsd->group_overlays =
          eina_list_append(boss->wsd->group_overlays, boss->grp->overlay);
     }
}

static void
_overlay_show(Elm_Map_Overlay *overlay)
{
   Elm_Map_Data *sd = overlay->wsd;

   if (overlay->paused) return;
   if ((overlay->grp) && (overlay->grp->klass) && (overlay->grp->klass->paused))
     return;

   overlay->visible = EINA_TRUE;
   if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     {
        overlay->visible = EINA_FALSE;
        return;
     }
   if (overlay->grp)
     {
        if ((overlay->grp->in) ||
            (overlay->hide) || (overlay->zoom_min > sd->zoom))
          overlay->visible = EINA_FALSE;

        if ((overlay->grp->klass) &&
            ((overlay->grp->klass->hide) ||
             (overlay->grp->klass->zoom_min > sd->zoom)))
          overlay->visible = EINA_FALSE;
     }

   switch (overlay->type)
     {
      case ELM_MAP_OVERLAY_TYPE_DEFAULT:
        if (overlay->visible) _overlay_default_show(overlay->ovl);
        else _overlay_default_hide(overlay->ovl);
        break;

      case ELM_MAP_OVERLAY_TYPE_GROUP:
        if (overlay->visible) _overlay_group_show(overlay->ovl);
        else _overlay_group_hide(overlay->ovl);
        break;

      case ELM_MAP_OVERLAY_TYPE_BUBBLE:
        overlay->visible =
          _overlay_bubble_show_hide(overlay->ovl, overlay->visible);
        break;

      case ELM_MAP_OVERLAY_TYPE_ROUTE:
        if (overlay->visible) _overlay_route_show(overlay->ovl);
        else _overlay_route_hide(overlay->ovl);
        break;

      case ELM_MAP_OVERLAY_TYPE_LINE:
        if (overlay->visible) _overlay_line_show(overlay->ovl);
        else _overlay_line_hide(overlay->ovl);
        break;

      case ELM_MAP_OVERLAY_TYPE_POLYGON:
        if (overlay->visible) _overlay_polygon_show(overlay->ovl);
        else _overlay_polygon_hide(overlay->ovl);
        break;

      case ELM_MAP_OVERLAY_TYPE_CIRCLE:
        if (overlay->visible) _overlay_circle_show(overlay->ovl);
        else _overlay_circle_hide(overlay->ovl);
        break;

      case ELM_MAP_OVERLAY_TYPE_SCALE:
        if (overlay->visible) _overlay_scale_show(overlay->ovl);
        else _overlay_scale_hide(overlay->ovl);
        break;
      default:
        ERR("Invalid overlay type to show: %d", overlay->type);
     }
}

static void
_copyright_place(Elm_Map_Data *sd)
{
   Evas_Coord w, h;
   Evas_Coord vw, vx, vy, vh;

   _viewport_coord_get(sd, &vx, &vy, &vw, &vh);
   _coord_to_canvas_no_rotation(sd, vx, vy, &vx, &vy);
   evas_object_size_hint_min_get(sd->copyright, &w, &h);
   _obj_place(sd->copyright, vx + vw - w, vy + vh - h, w, h);
}

static void
_overlay_place(Elm_Map_Data *sd)
{
   Eina_List *l, *ll;
   Elm_Map_Overlay *overlay;

   sd->group_overlays = eina_list_free(sd->group_overlays);

   EINA_LIST_FOREACH(sd->overlays, l, overlay)
     {
        // Reset groups
        if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS) continue;
        overlay->grp->in = EINA_FALSE;
        overlay->grp->boss = EINA_FALSE;
        _overlay_group_hide(overlay->grp);

        // Update overlays' coord
        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          _overlay_default_coord_update(overlay->ovl);
        else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
          _overlay_bubble_coord_update(overlay->ovl);
     }

   // Classify into group boss or follower
   EINA_LIST_FOREACH(sd->overlays, l, overlay)
     {
        Elm_Map_Overlay *boss;
        Overlay_Class *clas;

        if (overlay->type != ELM_MAP_OVERLAY_TYPE_CLASS) continue;
        if (overlay->hide || (overlay->zoom_min > sd->zoom)) continue;

        clas = overlay->ovl;
        if (clas->zoom_max < sd->zoom) continue;
        EINA_LIST_FOREACH(clas->members, ll, boss)
          {
             if (boss->type == ELM_MAP_OVERLAY_TYPE_CLASS) continue;
             if (boss->hide || (boss->zoom_min > sd->zoom)) continue;
             if (boss->grp->in) continue;
             _overlay_grouping(clas->members, boss);
          }
     }

   // Place group overlays and overlays
   EINA_LIST_FOREACH(sd->group_overlays, l, overlay)
     _overlay_show(overlay);
   EINA_LIST_FOREACH(sd->overlays, l, overlay)
     _overlay_show(overlay);
}

static Evas_Object *
_overlay_obj_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_GROUP)
     {
        Overlay_Group *ovl = overlay->ovl;
        Overlay_Default *df = ovl->ovl;

        return df->layout;
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        Overlay_Default *ovl = overlay->ovl;

        return ovl->layout;
     }
   else
     {
        ERR("Not supported overlay type: %d", overlay->type);
        return NULL;
     }
}

static Eina_Bool
_xml_name_attrs_dump_cb(void *data,
                        const char *key,
                        const char *value)
{
   Name_Dump *dump = (Name_Dump *)data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dump, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   if (!strncmp(key, NOMINATIM_ATTR_LON, strlen(NOMINATIM_ATTR_LON)))
     dump->lon = _elm_atof(value);
   else if (!strncmp(key, NOMINATIM_ATTR_LAT, strlen(NOMINATIM_ATTR_LAT)))
     dump->lat = _elm_atof(value);
   else if (!strncmp(key, NOMINATIM_ATTR_ADDRESS, strlen(NOMINATIM_ATTR_ADDRESS)))
     {
        if (!dump->address)
          dump->address = strdup(value);
     }

   return EINA_TRUE;
}

static Eina_Bool
_xml_route_dump_cb(void *data,
                   Eina_Simple_XML_Type type,
                   const char *value,
                   unsigned offset EINA_UNUSED,
                   unsigned length)
{
   Route_Dump *dump = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dump, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
      case EINA_SIMPLE_XML_OPEN_EMPTY:
      {
         const char *attrs;

         attrs = eina_simple_xml_tag_attributes_find(value, length);
         if (!attrs)
           {
              if (!strncmp(value, YOURS_DISTANCE, length))
                dump->id = ROUTE_XML_DISTANCE;
              else if (!strncmp(value, YOURS_DESCRIPTION, length))
                dump->id = ROUTE_XML_DESCRIPTION;
              else if (!strncmp(value, YOURS_COORDINATES, length))
                dump->id = ROUTE_XML_COORDINATES;
              else dump->id = ROUTE_XML_NONE;
           }
      }
      break;

      case EINA_SIMPLE_XML_DATA:
      {
         char *buf = malloc(length);

         if (!buf) return EINA_FALSE;

         snprintf(buf, length, "%s", value);
         if (dump->id == ROUTE_XML_DISTANCE)
           {
              dump->distance = _elm_atof(buf);
              free(buf);
           }
         else if (!(dump->description) && (dump->id == ROUTE_XML_DESCRIPTION))
           dump->description = buf;
         else if (dump->id == ROUTE_XML_COORDINATES)
           dump->coordinates = buf;
         else free(buf);
      }
      break;

      default:
        break;
     }

   return EINA_TRUE;
}

static Eina_Bool
_xml_name_dump_cb(void *data,
                  Eina_Simple_XML_Type type,
                  const char *value,
                  unsigned offset EINA_UNUSED,
                  unsigned length)
{
   Name_Dump *dump = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dump, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
      case EINA_SIMPLE_XML_OPEN_EMPTY:
      {
         const char *attrs;
         attrs = eina_simple_xml_tag_attributes_find(value, length);
         if (attrs)
           {
              if (!strncmp(value, NOMINATIM_RESULT, strlen(NOMINATIM_RESULT)))
                dump->id = NAME_XML_NAME;
              else dump->id = NAME_XML_NONE;

              eina_simple_xml_attributes_parse
                (attrs, length - (attrs - value), _xml_name_attrs_dump_cb,
                dump);
           }
      }
      break;

      case EINA_SIMPLE_XML_DATA:
      {
         char *buf = malloc(length + 1);

         if (!buf) return EINA_FALSE;
         snprintf(buf, length + 1, "%s", value);
         if (dump->id == NAME_XML_NAME) dump->address = buf;
         else free(buf);
      }
      break;

      default:
        break;
     }

   return EINA_TRUE;
}

static Eina_Bool
_xml_name_dump_list_cb(void *data,
                       Eina_Simple_XML_Type type,
                       const char *value,
                       unsigned offset,
                       unsigned length)
{
   Elm_Map_Name_List *name_list = data;
   Elm_Map_Name *name;
   Name_Dump dump = {0, NULL, 0.0, 0.0};
   _xml_name_dump_cb(&dump, type, value, offset, length);
   if (dump.address)
     {
        name = calloc(1, sizeof(Elm_Map_Name));
        if (!name) return EINA_FALSE;
        name->address = dump.address;
        name->lon = dump.lon;
        name->lat = dump.lat;
        name->wsd = name_list->wsd;
        name_list->names = eina_list_append(name_list->names, name);
        name->wsd->names = eina_list_append(name->wsd->names, name);
     }
   return EINA_TRUE;
}

static void
_kml_parse(Elm_Map_Route *r)
{
   FILE *f;
   char **str;
   double lon, lat;
   unsigned int ele, idx;

   EINA_SAFETY_ON_NULL_RETURN(r);
   EINA_SAFETY_ON_NULL_RETURN(r->fname);

   Route_Dump dump = {0, r->fname, 0.0, NULL, NULL};

   f = fopen(r->fname, "rb");
   if (f)
     {
        long sz;

        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        if (sz > 0)
          {
             char *buf = malloc(sz + 1);
             if (buf)
               {
                  memset(buf, 0, sz + 1);
                  rewind(f);
                  if (fread(buf, 1, sz, f) == (size_t)sz)
                    {
                       eina_simple_xml_parse
                         (buf, sz, EINA_TRUE, _xml_route_dump_cb, &dump);
                    }
                  free(buf);
               }
          }
        fclose(f);

        if (!EINA_DBL_EQ(dump.distance, 0)) r->info.distance = dump.distance;
        if (dump.description)
          {
             eina_stringshare_replace(&r->info.waypoints, dump.description);
             str = eina_str_split_full(dump.description, "\n", 0, &ele);
             r->info.waypoint_count = ele;
             for (idx = 0; idx < ele; idx++)
               {
                  Path_Waypoint *wp = ELM_NEW(Path_Waypoint);

                  if (wp)
                    {
                       wp->wsd = r->wsd;
                       wp->point = eina_stringshare_add(str[idx]);
                       DBG("%s", str[idx]);
                       r->waypoint = eina_list_append(r->waypoint, wp);
                    }
               }
             if (str && str[0])
               {
                  free(str[0]);
                  free(str);
               }
             free(dump.description);
          }
        else WRN("description is not found !");

        if (dump.coordinates)
          {
             eina_stringshare_replace(&r->info.nodes, dump.coordinates);
             str = eina_str_split_full(dump.coordinates, "\n", 0, &ele);
             r->info.node_count = ele;
             for (idx = 0; idx < ele; idx++)
               {
                  Path_Node *n = ELM_NEW(Path_Node);

                  sscanf(str[idx], "%lf,%lf", &lon, &lat);
                  if (n)
                    {
                       n->wsd = r->wsd;
                       n->pos.lon = lon;
                       n->pos.lat = lat;
                       n->idx = idx;
                       DBG("%lf:%lf", lon, lat);
                       n->pos.address = NULL;
                       r->nodes = eina_list_append(r->nodes, n);
                    }
               }
             if (str && str[0])
               {
                  free(str[0]);
                  free(str);
               }
             free(dump.coordinates);
          }
     }
}

static void
_name_parse(Elm_Map_Name *n)
{
   FILE *f;

   EINA_SAFETY_ON_NULL_RETURN(n);
   EINA_SAFETY_ON_NULL_RETURN(n->fname);

   Name_Dump dump = {0, NULL, 0.0, 0.0};

   f = fopen(n->fname, "rb");
   if (f)
     {
        long sz;

        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        if (sz > 0)
          {
             char *buf = malloc(sz + 1);
             if (buf)
               {
                  memset(buf, 0, sz + 1);
                  rewind(f);
                  if (fread(buf, 1, sz, f) == (size_t)sz)
                    {
                       eina_simple_xml_parse
                         (buf, sz, EINA_TRUE, _xml_name_dump_cb, &dump);
                    }
                  free(buf);
               }
          }
        fclose(f);

        if (dump.address)
          {
             INF("[%lf : %lf] ADDRESS : %s", n->lon, n->lat, dump.address);
             n->address = dump.address;
          }
        n->lon = dump.lon;
        n->lat = dump.lat;
     }
}

static void
_name_list_parse(Elm_Map_Name_List *nl)
{
   FILE *f;
   EINA_SAFETY_ON_NULL_RETURN(nl);
   EINA_SAFETY_ON_NULL_RETURN(nl->fname);

   f = fopen(nl->fname, "rb");
   if (f)
     {
        long sz;

        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        if (sz > 0)
          {
             char *buf = malloc(sz + 1);
             if (buf)
               {
                  memset(buf, 0, sz + 1);
                  rewind(f);
                  if (fread(buf, 1, sz, f) == (size_t)sz)
                    {
                       eina_simple_xml_parse
                         (buf, sz, EINA_TRUE, _xml_name_dump_list_cb, nl);
                    }
                  free(buf);
               }
          }
        fclose(f);
     }
}

static void
_route_cb(void *data,
          const char *file,
          int status)
{
   Elm_Map_Route *route;
   Elm_Map_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(file);

   route = data;
   sd = route->wsd;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   route->job = NULL;
   if (status == 200)
     {
        sd->src_route->route_parse_cb(route);
        INF("Route request success from (%lf, %lf) to (%lf, %lf)",
            route->flon, route->flat, route->tlon, route->tlat);
        if (route->cb) route->cb(route->data, sd->obj, route);
        efl_event_callback_legacy_call
          (sd->obj, ELM_MAP_EVENT_ROUTE_LOADED, NULL);
     }
   else
     {
        ERR("Route request failed: %d", status);
        if (route->cb) route->cb(route->data, sd->obj, NULL);
        efl_event_callback_legacy_call
          (sd->obj, ELM_MAP_EVENT_ROUTE_LOADED_FAIL, NULL);
     }

   edje_object_signal_emit(wd->resize_obj,
                           "elm,state,busy,stop", "elm");
}

static void
_name_cb(void *data,
         const char *file,
         int status)
{
   Elm_Map_Name *name;
   Elm_Map_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(file);

   name = data;
   sd = name->wsd;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   name->job = NULL;
   if (status == 200)
     {
        sd->src_name->name_parse_cb(name);
        INF("Name request success address:%s, lon:%lf, lat:%lf",
            name->address, name->lon, name->lat);
        if (name->cb) name->cb(name->data, sd->obj, name);
        efl_event_callback_legacy_call
          (sd->obj, ELM_MAP_EVENT_NAME_LOADED, NULL);
     }
   else
     {
        ERR("Name request failed: %d", status);
        if (name->cb) name->cb(name->data, sd->obj, NULL);
        efl_event_callback_legacy_call
          (sd->obj, ELM_MAP_EVENT_NAME_LOADED_FAIL, NULL);
     }
   edje_object_signal_emit(wd->resize_obj,
                           "elm,state,busy,stop", "elm");
}

static void
_name_list_cb(void *data,
              const char *file,
              int status)
{
   Elm_Map_Name_List *name_list;
   Elm_Map_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(file);

   name_list = data;
   sd = name_list->wsd;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   name_list->job = NULL;
   if (status == 200)
     {
        sd->src_name->name_list_parse_cb(name_list);
        INF("Name List request success address");
        if (name_list->cb)
          name_list->cb(name_list->data, wd->obj,
                        name_list->names);
        efl_event_callback_legacy_call
          (wd->obj, ELM_MAP_EVENT_NAME_LOADED, NULL);
     }
   else
     {
        ERR("Name List request failed: %d", status);
        if (name_list->cb)
          name_list->cb(name_list->data, wd->obj, NULL);
        efl_event_callback_legacy_call
          (wd->obj, ELM_MAP_EVENT_NAME_LOADED_FAIL, NULL);
     }

   edje_object_signal_emit(wd->resize_obj,
                           "elm,state,busy,stop", "elm");
   free(name_list->fname);
   free(name_list);
}

static char *
_prepare_download()
{
   char fname[PATH_MAX];

   snprintf(fname, sizeof(fname), "%s" CACHE_NAME_ROOT,
            efreet_cache_home_get());
   if (!ecore_file_exists(fname)) ecore_file_mkpath(fname);
   return strdup(fname);
}

static Elm_Map_Name *
_name_request(const Evas_Object *obj,
              int method,
              const char *address,
              double lon,
              double lat,
              Elm_Map_Name_Cb name_cb,
              void *data)
{
   char *url;
   Elm_Map_Name *name;
   char *fname, fname2[PATH_MAX + 128];

   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_name, NULL);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   fname = _prepare_download();
   url = sd->src_name->url_cb
   (sd->obj, method, address, lon, lat);
   if (!url)
     {
        ERR("Name URL is NULL");
        free(fname);
        return NULL;
     }

   name = ELM_NEW(Elm_Map_Name);
   name->wsd = sd;
   snprintf(fname2, sizeof(fname2), "%s/%d", fname, rand());
   name->fname = strdup(fname2);
   name->method = method;
   if (method == ELM_MAP_NAME_METHOD_SEARCH) name->address = strdup(address);
   else if (method == ELM_MAP_NAME_METHOD_REVERSE)
     {
        name->lon = lon;
        name->lat = lat;
     }
   name->cb = name_cb;
   name->data = data;

   if (!ecore_file_download_full(url, name->fname, _name_cb, NULL, name,
                                 &(name->job), sd->ua) || !(name->job))
     {
        ERR("Can't request Name from %s to %s", url, name->fname);
        free(name->address);
        free(name->fname);
        free(name);
        free(fname);
        free(url);
        return NULL;
     }
   INF("Name requested from %s to %s", url, name->fname);
   free(url);
   free(fname);

   sd->names = eina_list_append(sd->names, name);
   efl_event_callback_legacy_call
     (sd->obj, ELM_MAP_EVENT_NAME_LOAD, name);
   edje_object_signal_emit(wd->resize_obj,
                           "elm,state,busy,start", "elm");
   return name;
}

static Eina_List *
_name_list_request(const Evas_Object *obj,
              int method,
              const char *address,
              double lon,
              double lat,
              Elm_Map_Name_List_Cb name_cb,
              void *data)
{
   char *url;
   Elm_Map_Name_List *name_list;
   char *fname, fname2[PATH_MAX + 128];

   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_name, NULL);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   fname = _prepare_download();
   url = sd->src_name->url_cb
   (wd->obj, method, address, lon, lat);
   if (!url)
     {
        ERR("Name URL is NULL");
        free(fname);
        return NULL;
     }
   name_list = ELM_NEW(Elm_Map_Name_List);
   name_list->wsd = sd;
   snprintf(fname2, sizeof(fname2), "%s/%d", fname, rand());
   name_list->fname = strdup(fname2);
   name_list->cb = name_cb;
   name_list->data = data;
   if (!ecore_file_download_full(url, name_list->fname, _name_list_cb,
                                 NULL, name_list,
                                 &(name_list->job), sd->ua) || !(name_list->job))
     {
        ERR("Can't request Name from %s to %s", url, name_list->fname);
        free(name_list->fname);
        free(name_list);
        free(url);
        free(fname);
        return NULL;
     }
   INF("Name requested from %s to %s", url, name_list->fname);
   free(url);
   free(fname);

   efl_event_callback_legacy_call
     (wd->obj, ELM_MAP_EVENT_NAME_LOAD, name_list->names);
   edje_object_signal_emit(wd->resize_obj,
                           "elm,state,busy,start", "elm");
   return name_list->names;
}

static Evas_Event_Flags
_pinch_zoom_start_cb(void *data,
                     void *event_info EINA_UNUSED)
{
   ELM_MAP_DATA_GET(data, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EVAS_EVENT_FLAG_NONE);

   sd->pinch_zoom = sd->zoom_detail;
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_pinch_zoom_cb(void *data,
               void *event_info)
{
   ELM_MAP_DATA_GET(data, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EVAS_EVENT_FLAG_NONE);

   if (!sd->paused)
     {
        Elm_Gesture_Zoom_Info *ei = event_info;

        _zoom_do(sd, sd->pinch_zoom + ei->zoom - 1);
     }

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_pinch_rotate_cb(void *data,
                 void *event_info)
{
   ELM_MAP_DATA_GET(data, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EVAS_EVENT_FLAG_NONE);

   if (!sd->paused)
     {
        int x, y, w, h;
        Elm_Gesture_Rotate_Info *ei = event_info;

        evas_object_geometry_get(sd->obj, &x, &y, &w, &h);

        sd->rotate.d = sd->rotate.a + ei->angle - ei->base_angle;
        sd->rotate.cx = x + ((double)w * 0.5);
        sd->rotate.cy = y + ((double)h * 0.5);

        evas_object_smart_changed(sd->pan_obj);
     }
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_pinch_rotate_end_cb(void *data,
                     void *event_info EINA_UNUSED)
{
   ELM_MAP_DATA_GET(data, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EVAS_EVENT_FLAG_NONE);

   sd->rotate.a = sd->rotate.d;

   return EVAS_EVENT_FLAG_NONE;
}

static void
_source_tile_set(Elm_Map_Data *sd,
                 const char *source_name)
{
   Source_Tile *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (sd->src_tile && !strcmp(sd->src_tile->name, source_name))
     return;

   EINA_LIST_FOREACH(sd->src_tiles, l, s)
     {
        if (!strcmp(s->name, source_name))
          {
             sd->src_tile = s;
             break;
          }
     }
   if (!l)
     {
        ERR("source name (%s) is not found", source_name);
        return;
     }

   if (sd->src_tile->zoom_max < sd->zoom)
     sd->zoom = sd->src_tile->zoom_max;
   else if (sd->src_tile->zoom_min > sd->zoom)
     sd->zoom = sd->src_tile->zoom_min;

   if (sd->src_tile->zoom_max < sd->zoom_max)
     sd->zoom_max = sd->src_tile->zoom_max;
   if (sd->src_tile->zoom_min > sd->zoom_min)
     sd->zoom_min = sd->src_tile->zoom_min;

   ELM_SAFE_FREE(sd->copyright, evas_object_del);
   if (sd->src_tile->copyright_cb)
     {
        sd->copyright = sd->src_tile->copyright_cb(sd->obj);
        if (sd->copyright)
          {
             evas_object_smart_member_add(sd->copyright, sd->pan_obj);
             evas_object_stack_above(sd->copyright, sd->sep_maps_overlays);
          }
     }

   _grid_all_clear(sd);
   _grid_all_create(sd);
   _zoom_do(sd, sd->zoom);
}

static void
_source_route_set(Elm_Map_Data *sd,
                  const char *source_name)
{
   Source_Route *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (sd->src_route && !strcmp(sd->src_route->name, source_name))
     return;

   EINA_LIST_FOREACH(sd->src_routes, l, s)
     {
        if (!strcmp(s->name, source_name))
          {
             sd->src_route = s;
             break;
          }
     }
   if (!l)
     {
        ERR("source name (%s) is not found", source_name);
        return;
     }
}

static void
_source_name_set(Elm_Map_Data *sd,
                 const char *source_name)
{
   Source_Name *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (sd->src_name && !strcmp(sd->src_name->name, source_name))
     return;

   EINA_LIST_FOREACH(sd->src_names, l, s)
     {
        if (!strcmp(s->name, source_name))
          {
             sd->src_name = s;
             break;
          }
     }
   if (!l)
     {
        ERR("source name (%s) is not found", source_name);
        return;
     }
}

static Eina_Bool
_source_mod_cb(Eina_Module *m,
                    void *data)
{
   const char *file;
   Elm_Map_Data *sd = data;
   char *dir;
   const char *subfile;

   Elm_Map_Module_Source_Name_Func name_cb;
   Elm_Map_Module_Tile_Url_Func tile_url_cb;
   Elm_Map_Module_Tile_Scale_Func scale_cb;
   Elm_Map_Module_Tile_Copyright_Func copyright_cb;
   Elm_Map_Module_Tile_Zoom_Min_Func zoom_min;
   Elm_Map_Module_Tile_Zoom_Max_Func zoom_max;
   Elm_Map_Module_Tile_Geo_to_Coord_Func geo_to_coord;
   Elm_Map_Module_Tile_Coord_to_Geo_Func coord_to_geo;
   Elm_Map_Module_Route_Url_Func route_url_cb;
   Elm_Map_Module_Name_Url_Func name_url_cb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);

   file = eina_module_file_get(m);
   if (!file) return EINA_FALSE;

   dir = ecore_file_dir_get(file);
   if (!dir) return EINA_FALSE;
   subfile = ecore_file_file_get(dir);
   if (!subfile)
     {
        free(dir);
        return EINA_FALSE;
     }
   if (strcmp(subfile, MODULE_ARCH))
     {
        free(dir);
        return EINA_FALSE;
     }
   free(dir);
   if (!eina_module_load(m)) return EINA_FALSE;

   name_cb = eina_module_symbol_get(m, "map_module_source_name_get");
   if ((!name_cb))
     {
        eina_module_unload(m);
        return EINA_FALSE;
     }

   // Find TILE module
   tile_url_cb = eina_module_symbol_get(m, "map_module_tile_url_get");
   zoom_min = eina_module_symbol_get(m, "map_module_tile_zoom_min_get");
   zoom_max = eina_module_symbol_get(m, "map_module_tile_zoom_max_get");
   geo_to_coord = eina_module_symbol_get(m, "map_module_tile_geo_to_coord");
   coord_to_geo = eina_module_symbol_get(m, "map_module_tile_coord_to_geo");
   scale_cb = eina_module_symbol_get(m, "map_module_tile_scale_get");
   copyright_cb = eina_module_symbol_get(m, "map_module_tile_copyright_get");
   if (tile_url_cb && zoom_min && zoom_max && geo_to_coord && coord_to_geo &&
       scale_cb)
     {
        INF("Map TILE module is loaded \"%s\"",  file);
        Source_Tile *s;
        s = ELM_NEW(Source_Tile);
        s->name = name_cb();
        s->url_cb = tile_url_cb;
        s->zoom_min = zoom_min();
        s->zoom_max = zoom_max();
        s->geo_to_coord = geo_to_coord;
        s->coord_to_geo = coord_to_geo;
        s->scale_cb = scale_cb;
        s->copyright_cb = copyright_cb;
        sd->src_tiles = eina_list_append(sd->src_tiles, s);
     }

   // Find ROUTE module
   route_url_cb = eina_module_symbol_get(m, "map_module_route_url_get");
   if (route_url_cb)
     {
        INF("Map ROUTE module is loaded \"%s\"",  file);
        Source_Route *s;
        s = ELM_NEW(Source_Route);
        s->name = name_cb();
        s->url_cb = route_url_cb;
        s->route_parse_cb = eina_module_symbol_get(m, "map_module_route_source_parse");
        if (!s->route_parse_cb)
          s->route_parse_cb = _kml_parse;
        sd->src_routes = eina_list_append(sd->src_routes, s);
     }

   // Find NAME module
   name_url_cb = eina_module_symbol_get(m, "map_module_name_url_get");
   if (name_url_cb)
     {
        INF("Map NAME module is loaded \"%s\"",  file);
        Source_Name *s;
        s = ELM_NEW(Source_Name);
        s->name = name_cb();
        s->url_cb = name_url_cb;
        s->name_parse_cb = eina_module_symbol_get(m, "map_module_name_source_parse");
        if (!s->name_parse_cb)
          s->name_parse_cb = _name_parse;
        s->name_list_parse_cb = eina_module_symbol_get(m, "map_module_name_list_source_parse");
        if (!s->name_list_parse_cb)
          s->name_list_parse_cb = _name_list_parse;
        sd->src_names = eina_list_append(sd->src_names, s);
     }
   return EINA_TRUE;
}

static void
_source_all_unload(Elm_Map_Data *sd)
{
   int idx = 0;
   Source_Tile *s;

   for (idx = 0; sd->src_tile_names[idx]; idx++)
     eina_stringshare_del(sd->src_tile_names[idx]);
   free(sd->src_tile_names);

   for (idx = 0; sd->src_route_names[idx]; idx++)
     eina_stringshare_del(sd->src_route_names[idx]);
   free(sd->src_route_names);

   for (idx = 0; sd->src_name_names[idx]; idx++)
     eina_stringshare_del(sd->src_name_names[idx]);
   free(sd->src_name_names);

   EINA_LIST_FREE(sd->src_tiles, s)
     {
        eina_stringshare_del(s->name);
        free(s);
     }
   EINA_LIST_FREE(sd->src_routes, s)
     {
        eina_stringshare_del(s->name);
        free(s);
     }
   EINA_LIST_FREE(sd->src_names, s)
     {
        eina_stringshare_del(s->name);
        free(s);
     }

   eina_module_list_free(sd->src_mods);
   eina_array_free(sd->src_mods);
}

static void
_source_all_load(Elm_Map_Data *sd)
{
   Source_Tile *src_tile;
   Source_Route *src_route;
   Source_Name *src_name;
   unsigned int idx;
   Eina_List *l;

   // Load hard coded TILE source
   for (idx = 0; idx < (sizeof(src_tiles) / sizeof(Source_Tile)); idx++)
     {
        src_tile = ELM_NEW(Source_Tile);
        src_tile->name = eina_stringshare_add(src_tiles[idx].name);
        src_tile->zoom_min = src_tiles[idx].zoom_min;
        src_tile->zoom_max = src_tiles[idx].zoom_max;
        src_tile->url_cb = src_tiles[idx].url_cb;
        src_tile->geo_to_coord = src_tiles[idx].geo_to_coord;
        src_tile->coord_to_geo = src_tiles[idx].coord_to_geo;
        src_tile->scale_cb = src_tiles[idx].scale_cb;
        src_tile->copyright_cb = src_tiles[idx].copyright_cb;
        sd->src_tiles = eina_list_append(sd->src_tiles, src_tile);
     }
   // Load hard coded ROUTE source
   for (idx = 0; idx < (sizeof(src_routes) / sizeof(Source_Route)); idx++)
     {
        src_route = ELM_NEW(Source_Route);
        src_route->name = eina_stringshare_add(src_routes[idx].name);
        src_route->url_cb = src_routes[idx].url_cb;
        src_route->route_parse_cb = src_routes[idx].route_parse_cb;
        sd->src_routes = eina_list_append(sd->src_routes, src_route);
     }
   // Load from hard coded NAME source
   for (idx = 0; idx < (sizeof(src_names) / sizeof(Source_Name)); idx++)
     {
        src_name = ELM_NEW(Source_Name);
        src_name->name = eina_stringshare_add(src_names[idx].name);
        src_name->url_cb = src_names[idx].url_cb;
        src_name->name_parse_cb = src_names[idx].name_parse_cb;
        src_name->name_list_parse_cb = src_names[idx].name_list_parse_cb;
        sd->src_names = eina_list_append(sd->src_names, src_name);
     }

   // Load from modules
   sd->src_mods = eina_module_list_get(sd->src_mods, MODULES_PATH, 1,
                                            &_source_mod_cb, sd);

   // Set default source
   sd->src_tile = eina_list_nth(sd->src_tiles, 0);
   sd->src_route = eina_list_nth(sd->src_routes, 0);
   sd->src_name = eina_list_nth(sd->src_names, 0);

   // Make name string of sources
   idx = 0;
   sd->src_tile_names = calloc((eina_list_count(sd->src_tiles) + 1),
                               sizeof(const char *));
   EINA_LIST_FOREACH(sd->src_tiles, l, src_tile)
     {
        eina_stringshare_replace(&sd->src_tile_names[idx], src_tile->name);
        INF("source : %s", sd->src_tile_names[idx]);
        idx++;
     }
   idx = 0;
   sd->src_route_names = calloc((eina_list_count(sd->src_routes) + 1),
                                sizeof(const char *));
   EINA_LIST_FOREACH(sd->src_routes, l, src_route)
     {
        eina_stringshare_replace(&sd->src_route_names[idx], src_route->name);
        INF("source : %s", sd->src_route_names[idx]);
        idx++;
     }
   idx = 0;
   sd->src_name_names = calloc((eina_list_count(sd->src_names) + 1),
                               sizeof(const char *));
   EINA_LIST_FOREACH(sd->src_names, l, src_name)
     {
        eina_stringshare_replace(&sd->src_name_names[idx], src_name->name);
        INF("source : %s", sd->src_name_names[idx]);
        idx++;
     }
}

static void
_zoom_mode_set(Elm_Map_Data *sd, double zoom)
{
   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (sd->mode == EFL_UI_ZOOM_MODE_MANUAL)
     {
        if (sd->paused) _zoom_do(sd, zoom);
        else _zoom_with_animation(sd, zoom, 10);
     }
   else
     {
        double diff;
        Evas_Coord w, h;
        Evas_Coord vw, vh;

        w = sd->size.w;
        h = sd->size.h;
        _viewport_coord_get(sd, NULL, NULL, &vw, &vh);

        if (sd->mode == EFL_UI_ZOOM_MODE_AUTO_FIT)
          {
             if ((w < vw) && (h < vh))
               {
                  diff = 0.01;
                  while ((w < vw) && (h < vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * sd->tsize;
                       h = pow(2.0, zoom) * sd->tsize;
                    }
               }
             else
               {
                  diff = -0.01;
                  while ((w > vw) || (h > vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * sd->tsize;
                       h = pow(2.0, zoom) * sd->tsize;
                    }
                  zoom -= diff;
               }
          }
        else if (sd->mode == EFL_UI_ZOOM_MODE_AUTO_FILL)
          {
             if ((w < vw) || (h < vh))
               {
                  diff = 0.01;
                  while ((w < vw) || (h < vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * sd->tsize;
                       h = pow(2.0, zoom) * sd->tsize;
                    }
               }
             else
               {
                  diff = -0.01;
                  while ((w > vw) && (h > vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * sd->tsize;
                       h = pow(2.0, zoom) * sd->tsize;
                    }
                  zoom -= diff;
               }
          }
        _zoom_do(sd, zoom);
     }
}

static void
_region_show_bring_in(Elm_Map_Data *wsd, double lon, double lat, Eina_Bool bring_in)
{
   int x, y, w, h;

   EINA_SAFETY_ON_NULL_RETURN(wsd);

   _region_to_coord_convert
     (wsd, lon, lat, wsd->size.w, &x, &y);
   _viewport_coord_get(wsd, NULL, NULL, &w, &h);
   x = x - (w / 2);
   y = y - (h / 2);

   if (bring_in) elm_interface_scrollable_region_bring_in(wsd->obj, x, y, w, h);
   else elm_interface_scrollable_content_region_show(wsd->obj, x, y, w, h);
}

static void
_overlays_show(Elm_Map_Data *sd, Eina_List *overlays)
{
   double max_lon, min_lon, max_lat, min_lat, lon, lat;
   int zoom, zoom_max;
   Evas_Coord vw, vh;

   EINA_SAFETY_ON_NULL_RETURN(sd);
   EINA_SAFETY_ON_NULL_RETURN(overlays);

   _region_max_min_get(overlays, &max_lon, &min_lon, &max_lat, &min_lat);
   lon = (max_lon + min_lon) / 2;
   lat = (max_lat + min_lat) / 2;

   zoom = sd->src_tile->zoom_min;
   _viewport_coord_get(sd, NULL, NULL, &vw, &vh);
   if (sd->src_tile->zoom_max < sd->zoom_max)
     zoom_max = sd->src_tile->zoom_max;
   else zoom_max = sd->zoom_max;
   while (zoom <= zoom_max)
     {
        Evas_Coord size, max_x, max_y, min_x, min_y;

        size = pow(2.0, zoom) * sd->tsize;
        _region_to_coord_convert
          (sd, min_lon, max_lat, size, &min_x, &max_y);
        _region_to_coord_convert
          (sd, max_lon, min_lat, size, &max_x, &min_y);
        if ((max_x - min_x) > vw || (max_y - min_y) > vh) break;
        zoom++;
     }
   zoom--;

   _zoom_do(sd, zoom);
   _region_show_bring_in(sd, lon, lat, EINA_FALSE);
}

EOLIAN static void
_elm_map_pan_elm_pan_pos_set(Eo *obj, Elm_Map_Pan_Data *psd, Evas_Coord x, Evas_Coord y)
{
   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;

   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;

   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_map_pan_elm_pan_pos_get(const Eo *obj EINA_UNUSED, Elm_Map_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

EOLIAN static void
_elm_map_pan_elm_pan_pos_max_get(const Eo *obj, Elm_Map_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = psd->wsd->size.w - ow;
   oh = psd->wsd->size.h - oh;

   if (ow < 0) ow = 0;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

EOLIAN static void
_elm_map_pan_elm_pan_pos_min_get(const Eo *obj EINA_UNUSED, Elm_Map_Pan_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

EOLIAN static void
_elm_map_pan_elm_pan_content_size_get(const Eo *obj EINA_UNUSED, Elm_Map_Pan_Data *psd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = psd->wsd->size.w;
   if (h) *h = psd->wsd->size.h;
}

EOLIAN static void
_elm_map_pan_efl_gfx_entity_size_set(Eo *obj, Elm_Map_Pan_Data *psd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_PAN_CLASS), sz);

   _sizing_eval(psd->wsd->obj);
   efl_ui_zoom_mode_set(psd->wobj, psd->wsd->mode);
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_map_pan_efl_canvas_group_group_calculate(Eo *obj, Elm_Map_Pan_Data *psd)
{
   Evas_Coord w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w <= 0 || h <= 0) return;

   _grid_place(psd->wsd);
   if (psd->wsd->copyright)
     _copyright_place(psd->wsd);
   _overlay_place(psd->wsd);
   _track_place(psd->wsd);
   _calc_job(psd->wsd);
}

EOLIAN static void
_elm_map_pan_efl_gfx_entity_position_set(Eo *obj, Elm_Map_Pan_Data *_pd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_PAN_CLASS), pos);
   evas_object_smart_changed(obj);
}

EOLIAN static Eina_Bool
_elm_map_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Map_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

EOLIAN static void
_elm_map_pan_efl_object_destructor(Eo *obj, Elm_Map_Pan_Data *psd)
{
   efl_data_unref(psd->wobj, psd->wsd);
   efl_destructor(efl_super(obj, MY_PAN_CLASS));
}

static void
_elm_map_pan_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_PAN_CLASS_NAME_LEGACY, klass);
}

#include "elm_map_pan_eo.c"

EOLIAN static Eina_Error
_elm_map_efl_ui_widget_theme_apply(Eo *obj, Elm_Map_Data *sd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;


   elm_widget_theme_object_set
      (obj, wd->resize_obj, "map", "base", elm_widget_style_get(obj));

   _sizing_eval(obj);

   return int_ret;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   const char *dir = params;

   Evas_Coord vh;
   Evas_Coord x, y;
   Evas_Coord step_x, step_y, page_x, page_y;

   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   elm_interface_scrollable_step_size_get(obj, &step_x, &step_y);
   elm_interface_scrollable_page_size_get(obj, &page_x, &page_y);
   elm_interface_scrollable_content_viewport_geometry_get
         (obj, NULL, NULL, NULL, &vh);

   if (!strcmp(dir, "left"))
     {
        x -= step_x;
     }
   else if (!strcmp(dir, "right"))
     {
        x += step_x;
     }
   else if (!strcmp(dir, "up"))
     {
        y -= step_y;
     }
   else if (!strcmp(dir, "down"))
     {
        y += step_y;
     }
   else if (!strcmp(dir, "prior"))
     {
        if (page_y < 0)
          y -= -(page_y * vh) / 100;
        else
          y -= page_y;
     }
   else if (!strcmp(dir, "next"))
     {
        if (page_y < 0)
          y += -(page_y * vh) / 100;
        else
          y += page_y;
     }
   else return EINA_FALSE;

   elm_interface_scrollable_content_pos_set(obj, x, y, EINA_TRUE);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_zoom(Evas_Object *obj, const char *params)
{
   ELM_MAP_DATA_GET(obj, sd);
   const char *dir = params;

   if (!strcmp(dir, "in"))
     {
        _zoom_with_animation(sd, sd->zoom + 1, 10);
     }
   else if (!strcmp(dir, "out"))
     {
        _zoom_with_animation(sd, sd->zoom - 1, 10);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static void
_elm_map_efl_canvas_group_group_add(Eo *obj, Elm_Map_Data *priv)
{
   Evas_Coord minw, minh;
   Elm_Map_Pan_Data *pan_data;
   Evas_Object *edje;

   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_theme_object_set
     (obj, edje, "map", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, obj);

   elm_interface_scrollable_objects_set(obj, edje, priv->hit_rect);
   elm_interface_scrollable_wheel_disabled_set(obj, EINA_TRUE);
   elm_interface_scrollable_bounce_allow_set(obj, _elm_config->thumbscroll_bounce_enable, _elm_config->thumbscroll_bounce_enable);
   elm_interface_scrollable_single_direction_set(obj, ELM_SCROLLER_SINGLE_DIRECTION_NONE);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints_cb, obj);

   elm_interface_scrollable_animate_start_cb_set(obj, _scroll_animate_start_cb);
   elm_interface_scrollable_animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   elm_interface_scrollable_scroll_cb_set(obj, _scroll_cb);

   priv->pan_obj = efl_add(MY_PAN_CLASS, obj);
   pan_data = efl_data_scope_get(priv->pan_obj, MY_PAN_CLASS);
   efl_data_ref(obj, MY_CLASS);
   pan_data->wobj = obj;
   pan_data->wsd = priv;

   elm_interface_scrollable_extern_pan_set(obj, priv->pan_obj);

   edje_object_size_min_calc(edje, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   priv->g_layer = elm_gesture_layer_add(obj);
   if (!priv->g_layer)
     ERR("elm_gesture_layer_add() failed");

   elm_gesture_layer_attach(priv->g_layer, obj);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START,
     _pinch_zoom_start_cb, obj);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE,
     _pinch_zoom_cb, obj);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_MOVE,
     _pinch_rotate_cb, obj);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_END,
     _pinch_rotate_end_cb, obj);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_ABORT,
     _pinch_rotate_end_cb, obj);

   priv->sep_maps_overlays =
     evas_object_rectangle_add(evas_object_evas_get(obj));
   elm_widget_sub_object_add(obj, priv->sep_maps_overlays);
   evas_object_smart_member_add(priv->sep_maps_overlays, priv->pan_obj);

   priv->map = evas_map_new(EVAS_MAP_POINT);

   _source_all_load(priv);
   priv->zoom_min = priv->src_tile->zoom_min;
   priv->zoom_max = priv->src_tile->zoom_max;
   // FIXME: Tile Provider is better to provide tile size!
   priv->tsize = DEFAULT_TILE_SIZE;

   srand(time(NULL));

   priv->id = ((int)getpid() << 16) | id_num;
   id_num++;
   _grid_all_create(priv);

   _zoom_do(priv, 0);

   priv->mode = EFL_UI_ZOOM_MODE_MANUAL;

   if (!elm_need_efreet())
     ERR("Efreet initialization failed!");

   if (!ecore_file_download_protocol_available("http://"))
     ERR("Ecore must be built with curl support for the map widget!");

   if (priv->src_tile->copyright_cb)
     {
        priv->copyright = priv->src_tile->copyright_cb(obj);
        if (priv->copyright)
          {
             evas_object_smart_member_add(priv->copyright, priv->pan_obj);
             evas_object_stack_above(priv->copyright, priv->sep_maps_overlays);
          }
     }
}

EOLIAN static void
_elm_map_efl_object_invalidate(Eo *obj, Elm_Map_Data *sd)
{
   Evas_Object *track;

   EINA_LIST_FREE(sd->track, track)
     evas_object_del(track);

   ELM_SAFE_FREE(sd->copyright, evas_object_del);

   // pan_obj was created with efl_add and will die after invalidate.
   // Technically it will bring down all the object above, and there is
   // no need to explicitely destroy them, but we were doing that
   // explicitely in the past, so keep it explicit.
   sd->pan_obj = NULL;

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_map_efl_canvas_group_group_del(Eo *obj, Elm_Map_Data *sd)
{
   Elm_Map_Route *r;
   Elm_Map_Name *na;
   Eina_List *l, *ll;
   Elm_Map_Overlay *overlay;

   EINA_LIST_FOREACH_SAFE(sd->routes, l, ll, r)
     elm_map_route_del(r);

   eina_list_free(sd->routes);

   EINA_LIST_FOREACH_SAFE(sd->names, l, ll, na)
     elm_map_name_del(na);

   eina_list_free(sd->names);

   EINA_LIST_FOREACH_SAFE(sd->overlays, l, ll, overlay)
     elm_map_overlay_del(overlay);

   eina_list_free(sd->overlays);
   eina_list_free(sd->group_overlays);
   eina_list_free(sd->all_overlays);

   ecore_timer_del(sd->scr_timer);
   ecore_timer_del(sd->long_timer);
   eina_stringshare_del(sd->user_agent);
   eina_hash_free(sd->ua);
   ecore_timer_del(sd->zoom_timer);
   _zoom_animator_set(sd, NULL);

   _grid_all_clear(sd);
   // Removal of download list should be after grid clear.
   ecore_idler_del(sd->download_idler);
   eina_list_free(sd->download_list);

   _source_all_unload(sd);

   {
      char buf[4096];

      snprintf(buf, sizeof(buf), "%s" CACHE_ROOT, efreet_cache_home_get());
      ecore_file_recursive_rm(buf);
   }

   ecore_timer_del(sd->loaded_timer);
   if (sd->map) evas_map_free(sd->map);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_map_efl_gfx_entity_position_set(Eo *obj, Elm_Map_Data *sd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_gfx_entity_position_set(sd->hit_rect, pos);
}

EOLIAN static void
_elm_map_efl_gfx_entity_size_set(Eo *obj, Elm_Map_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   efl_gfx_entity_size_set(sd->hit_rect, sz);
}

EOLIAN static void
_elm_map_efl_canvas_group_group_member_add(Eo *obj, Elm_Map_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EAPI Evas_Object *
elm_map_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_map_efl_object_constructor(Eo *obj, Elm_Map_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_IMAGE_MAP);
   legacy_object_focus_handle(obj);

   return obj;
}

EOLIAN static double
_elm_map_efl_ui_zoom_zoom_level_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   return sd->zoom;
}

EOLIAN static void
_elm_map_efl_ui_zoom_zoom_level_set(Eo *obj, Elm_Map_Data *sd, double zoom)
{
   ELM_MAP_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   if (sd->mode != EFL_UI_ZOOM_MODE_MANUAL) return;
   if (zoom < 0) zoom = 0;
   if (sd->zoom == zoom) return;

   sd->calc_job.zoom = zoom;
   sd->calc_job.zoom_mode_set = _zoom_mode_set;

   evas_object_smart_changed(sd->pan_obj);
}

EAPI void
elm_map_zoom_set(Eo *obj, int zoom)
{
   efl_ui_zoom_level_set(obj, zoom);
}

EAPI int
elm_map_zoom_get(const Eo *obj)
{
   return efl_ui_zoom_level_get(obj);
}

EOLIAN static Efl_Ui_Zoom_Mode
_elm_map_efl_ui_zoom_zoom_mode_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   return sd->mode;
}

EOLIAN static void
_elm_map_efl_ui_zoom_zoom_mode_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Efl_Ui_Zoom_Mode mode)
{
   if ((mode == EFL_UI_ZOOM_MODE_MANUAL) && (sd->mode == !!mode)) return;

   sd->mode = mode;
   sd->calc_job.zoom = sd->zoom_detail;
   sd->calc_job.zoom_mode_set = _zoom_mode_set;

   evas_object_smart_changed(sd->pan_obj);
}

static inline void
_convert_map_zoom_mode(Elm_Map_Zoom_Mode *legacy_mode, Efl_Ui_Zoom_Mode *mode, Eina_Bool to_legacy)
{
   #define CONVERT(LEGACY_MODE, NEW_MODE) \
      if (to_legacy  && *mode == NEW_MODE) \
        { \
           *legacy_mode =LEGACY_MODE; \
           return; \
        } \
      if (!to_legacy && *legacy_mode == LEGACY_MODE) \
        { \
           *mode = NEW_MODE; \
           return; \
        } \

   CONVERT(ELM_MAP_ZOOM_MODE_MANUAL,    EFL_UI_ZOOM_MODE_MANUAL)
   CONVERT(ELM_MAP_ZOOM_MODE_AUTO_FIT,  EFL_UI_ZOOM_MODE_AUTO_FIT)
   CONVERT(ELM_MAP_ZOOM_MODE_AUTO_FILL, EFL_UI_ZOOM_MODE_AUTO_FILL)
   CONVERT(ELM_MAP_ZOOM_MODE_LAST,      EFL_UI_ZOOM_MODE_LAST)
   CONVERT(ELM_MAP_ZOOM_MODE_LAST,      EFL_UI_ZOOM_MODE_AUTO_FIT_IN)

   #undef CONVERT
}

EAPI void
elm_map_zoom_mode_set(Eo *obj, Elm_Map_Zoom_Mode mode)
{
   Efl_Ui_Zoom_Mode new_mode = EFL_UI_ZOOM_MODE_MANUAL;

   _convert_map_zoom_mode(&mode, &new_mode, EINA_FALSE);

   efl_ui_zoom_mode_set(obj, new_mode);
}

EAPI Elm_Map_Zoom_Mode
elm_map_zoom_mode_get(const Eo *obj)
{
   Efl_Ui_Zoom_Mode new_mode = efl_ui_zoom_mode_get(obj);
   Elm_Map_Zoom_Mode mode = ELM_MAP_ZOOM_MODE_MANUAL;

   _convert_map_zoom_mode(&mode, &new_mode, EINA_TRUE);

   return mode;
}

EOLIAN static void
_elm_map_zoom_max_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, int zoom)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   sd->zoom_max = zoom;
}

EOLIAN static int
_elm_map_zoom_max_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_tile, -1);

   return sd->zoom_max;
}

EOLIAN static void
_elm_map_zoom_min_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, int zoom)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   sd->zoom_min = zoom;
}

EOLIAN static int
_elm_map_zoom_min_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_tile, -1);

   return sd->zoom_min;
}

EOLIAN static void
_elm_map_map_region_bring_in(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double lon, double lat)
{
   sd->calc_job.bring_in = EINA_TRUE;
   sd->calc_job.lon = lon;
   sd->calc_job.lat = lat;
   sd->calc_job.region_show_bring_in = _region_show_bring_in;

   evas_object_smart_changed(sd->pan_obj);
}

EOLIAN static void
_elm_map_region_show(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double lon, double lat)
{
   sd->calc_job.bring_in = EINA_FALSE;
   sd->calc_job.lon = lon;
   sd->calc_job.lat = lat;
   sd->calc_job.region_show_bring_in = _region_show_bring_in;

   evas_object_smart_changed(sd->pan_obj);
}

EOLIAN static void
_elm_map_region_zoom_bring_in(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, int zoom, double lon, double lat)
{
   _zoom_bring_with_animation(sd, zoom, lon, lat, ZOOM_ANIM_CNT, ZOOM_BRING_CNT);
}

EOLIAN static void
_elm_map_region_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double *lon, double *lat)
{
   double tlon, tlat;
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0;

   _viewport_coord_get(sd, &vx, &vy, &vw, &vh);
   _coord_to_region_convert
     (sd, vx + vw / 2, vy + vh / 2, sd->size.w, &tlon, &tlat);
   if (lon) *lon = tlon;
   if (lat) *lat = tlat;
}

EOLIAN static void
_elm_map_efl_ui_zoom_zoom_animation_set(Eo *obj, Elm_Map_Data *sd, Eina_Bool paused)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->paused == !!paused) return;
   sd->paused = !!paused;
   if (sd->paused)
     {
        if (_zoom_animator_set(sd, NULL))
          {
             _zoom_do(sd, sd->zoom);
          }
        edje_object_signal_emit(wd->resize_obj,
                                "elm,state,busy,stop", "elm");
     }
   else
     {
        if (sd->download_num >= 1)
          edje_object_signal_emit(wd->resize_obj,
                                  "elm,state,busy,start", "elm");
     }
}

EOLIAN static Eina_Bool
_elm_map_efl_ui_zoom_zoom_animation_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   return sd->paused;
}

EOLIAN static void
_elm_map_map_rotate_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double degree, Evas_Coord cx, Evas_Coord cy)
{
   sd->rotate.d = degree;
   sd->rotate.cx = cx;
   sd->rotate.cy = cy;

   evas_object_smart_changed(sd->pan_obj);
}

EOLIAN static void
_elm_map_map_rotate_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double *degree, Evas_Coord *cx, Evas_Coord *cy)
{
   if (degree) *degree = sd->rotate.d;
   if (cx) *cx = sd->rotate.cx;
   if (cy) *cy = sd->rotate.cy;
}

EOLIAN static void
_elm_map_elm_interface_scrollable_wheel_disabled_set(Eo *obj, Elm_Map_Data *sd, Eina_Bool disabled)
{
   if ((!sd->wheel_disabled) && (disabled))
     evas_object_event_callback_del_full
       (obj, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, obj);
   else if ((sd->wheel_disabled) && (!disabled))
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, obj);
   sd->wheel_disabled = !!disabled;
   elm_interface_scrollable_wheel_disabled_set(efl_super(obj, ELM_MAP_CLASS), disabled);
}

EOLIAN static void
_elm_map_tile_load_status_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, int *try_num, int *finish_num)
{
   if (try_num) *try_num = sd->try_num;
   if (finish_num) *finish_num = sd->finish_num;
}

EOLIAN static void
_elm_map_canvas_to_region_convert(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Evas_Coord x, Evas_Coord y, double *lon, double *lat)
{
   EINA_SAFETY_ON_NULL_RETURN(lon);
   EINA_SAFETY_ON_NULL_RETURN(lat);

   _canvas_to_coord(sd, x, y, &x, &y);
   _coord_to_region_convert(sd, x, y, sd->size.w, lon, lat);
}

EOLIAN static void
_elm_map_region_to_canvas_convert(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double lon, double lat, Evas_Coord *x, Evas_Coord *y)
{
   EINA_SAFETY_ON_NULL_RETURN(x);
   EINA_SAFETY_ON_NULL_RETURN(y);

   _region_to_coord_convert(sd, lon, lat, sd->size.w, x, y);
   _coord_to_canvas(sd, *x, *y, x, y);
}

EOLIAN static void
_elm_map_user_agent_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, const char *user_agent)
{
   EINA_SAFETY_ON_NULL_RETURN(user_agent);

   eina_stringshare_replace(&sd->user_agent, user_agent);

   if (!sd->ua) sd->ua = eina_hash_string_small_new(NULL);
   eina_hash_set(sd->ua, "User-Agent", sd->user_agent);
}

EOLIAN static const char*
_elm_map_user_agent_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   return sd->user_agent;
}

EOLIAN static void
_elm_map_source_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Elm_Map_Source_Type type, const char *source_name)
{
   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (type == ELM_MAP_SOURCE_TYPE_TILE) _source_tile_set(sd, source_name);
   else if (type == ELM_MAP_SOURCE_TYPE_ROUTE)
     _source_route_set(sd, source_name);
   else if (type == ELM_MAP_SOURCE_TYPE_NAME)
     _source_name_set(sd, source_name);
   else ERR("Not supported map source type: %d", type);

}

EOLIAN static const char*
_elm_map_source_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Elm_Map_Source_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_tile, NULL);

   if (type == ELM_MAP_SOURCE_TYPE_TILE) return sd->src_tile->name;
   else if (type == ELM_MAP_SOURCE_TYPE_ROUTE)
     return sd->src_route->name;
   else if (type == ELM_MAP_SOURCE_TYPE_NAME)
     return sd->src_name->name;
   else ERR("Not supported map source type: %d", type);
   return NULL;
}

EOLIAN static const char**
_elm_map_sources_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Elm_Map_Source_Type type)
{
   const char **ret;
   ret = NULL;

   if (type == ELM_MAP_SOURCE_TYPE_TILE) ret = sd->src_tile_names;
   else if (type == ELM_MAP_SOURCE_TYPE_ROUTE)
     ret = sd->src_route_names;
   else if (type == ELM_MAP_SOURCE_TYPE_NAME)
     ret = sd->src_name_names;
   else ERR("Not supported map source type: %d", type);

   return ret;
}

EOLIAN static Elm_Map_Route*
_elm_map_route_add(Eo *obj, Elm_Map_Data *sd, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data)
{
   Elm_Map_Route *ret;
   ret = NULL;
   char *url;
   char *type_name;
   Elm_Map_Route *route;
   char fname[PATH_MAX], fname2[PATH_MAX + 128];

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_route, NULL);

   {
      snprintf(fname, sizeof(fname), "%s" CACHE_ROUTE_ROOT,
               efreet_cache_home_get());
      if (!ecore_file_exists(fname)) ecore_file_mkpath(fname);
   }

   if (type == ELM_MAP_ROUTE_TYPE_MOTOCAR)
     type_name = strdup(ROUTE_TYPE_MOTORCAR);
   else if (type == ELM_MAP_ROUTE_TYPE_BICYCLE)
     type_name = strdup(ROUTE_TYPE_BICYCLE);
   else if (type == ELM_MAP_ROUTE_TYPE_FOOT)
     type_name = strdup(ROUTE_TYPE_FOOT);
   else type_name = NULL;

   url = sd->src_route->url_cb(obj, type_name, method, flon, flat, tlon, tlat);
   if (!url)
     {
        ERR("Route URL is NULL");
        free(type_name);
        return ret;
     }
   free(type_name);

   route = ELM_NEW(Elm_Map_Route);
   route->wsd = sd;
   snprintf(fname2, sizeof(fname2), "%s/%d", fname, rand());
   route->fname = strdup(fname2);
   route->type = type;
   route->method = method;
   route->flon = flon;
   route->flat = flat;
   route->tlon = tlon;
   route->tlat = tlat;
   route->cb = route_cb;
   route->data = data;

   if (!ecore_file_download_full(url, route->fname, _route_cb, NULL, route,
                                 &(route->job), sd->ua) || !(route->job))
     {
        ERR("Can't request Route from %s to %s", url, route->fname);
        free(route->fname);
        free(route);
        free(url);
        return ret;
     }
   INF("Route requested from %s to %s", url, route->fname);
   free(url);

   sd->routes = eina_list_append(sd->routes, route);
   efl_event_callback_legacy_call
     (sd->obj, ELM_MAP_EVENT_ROUTE_LOAD, route);
   edje_object_signal_emit(wd->resize_obj,
                           "elm,state,busy,start", "elm");
   ret = route;

   return ret;
}

EAPI void
elm_map_route_del(Elm_Map_Route *route)
{
   Path_Waypoint *w;
   Path_Node *n;

   EINA_SAFETY_ON_NULL_RETURN(route);
   EINA_SAFETY_ON_NULL_RETURN(route->wsd);
   ELM_MAP_CHECK((route->wsd)->obj);

   if (route->job) ecore_file_download_abort(route->job);

   EINA_LIST_FREE(route->waypoint, w)
     {
        eina_stringshare_del(w->point);
        free(w);
     }

   EINA_LIST_FREE(route->nodes, n)
     {
        eina_stringshare_del(n->pos.address);
        free(n);
     }

   if (route->fname)
     {
        ecore_file_remove(route->fname);
        free(route->fname);
     }

   route->wsd->routes = eina_list_remove(route->wsd->routes, route);
   free(route);
}

EAPI double
elm_map_route_distance_get(const Elm_Map_Route *route)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(route, 0.0);
   return route->info.distance;
}

EAPI const char *
elm_map_route_node_get(const Elm_Map_Route *route)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);
   return route->info.nodes;
}

EAPI const char *
elm_map_route_waypoint_get(const Elm_Map_Route *route)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);
   return route->info.waypoints;
}

EOLIAN static Elm_Map_Name*
_elm_map_name_add(const Eo *obj, Elm_Map_Data *_pd EINA_UNUSED, const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data)
{
   if (address)
     return _name_request(obj, ELM_MAP_NAME_METHOD_SEARCH, address, 0, 0,
                          name_cb, data);
   else
     return _name_request(obj, ELM_MAP_NAME_METHOD_REVERSE, NULL, lon, lat,
                          name_cb, data);
}

EOLIAN static void
_elm_map_name_search(const Eo *obj, Elm_Map_Data *_pd EINA_UNUSED, const char *address, Elm_Map_Name_List_Cb name_cb, void *data)
{
   if (address)
     _name_list_request(obj, ELM_MAP_NAME_METHOD_SEARCH, address, 0, 0,
                        name_cb, data);
}

EAPI void
elm_map_name_del(Elm_Map_Name *name)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   EINA_SAFETY_ON_NULL_RETURN(name->wsd);
   ELM_MAP_CHECK((name->wsd)->obj);

   ecore_file_download_abort(name->job);
   free(name->address);
   if (name->fname)
     {
        ecore_file_remove(name->fname);
        free(name->fname);
     }

   name->wsd->names = eina_list_remove(name->wsd->names, name);
   free(name);
}

EAPI const char *
elm_map_name_address_get(const Elm_Map_Name *name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name->wsd, NULL);
   ELM_MAP_CHECK((name->wsd)->obj) NULL;

   return name->address;
}

EAPI void
elm_map_name_region_get(const Elm_Map_Name *name,
                        double *lon,
                        double *lat)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   EINA_SAFETY_ON_NULL_RETURN(name->wsd);
   ELM_MAP_CHECK((name->wsd)->obj);

   if (lon) *lon = name->lon;
   if (lat) *lat = name->lat;
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double lon, double lat)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_DEFAULT;
   overlay->c.r = 0xff;
   overlay->c.g = 0xff;
   overlay->c.b = 0xff;
   overlay->c.a = 0xff;
   overlay->ovl = _overlay_default_new(overlay, lon, lat, overlay->c, 1);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);

   return overlay;
}

EOLIAN static Eina_List *
_elm_map_overlays_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   Eina_List *l;
   Elm_Map_Overlay *ovl;

   sd->all_overlays = eina_list_free(sd->all_overlays);

   EINA_LIST_FOREACH(sd->overlays, l, ovl)
     sd->all_overlays = eina_list_append(sd->all_overlays, ovl);
   EINA_LIST_FOREACH(sd->group_overlays, l, ovl)
     sd->all_overlays = eina_list_append(sd->all_overlays, ovl);

   return sd->all_overlays;
}

EAPI void
elm_map_overlay_del(Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   efl_event_callback_legacy_call
     ((overlay->wsd)->obj, ELM_MAP_EVENT_OVERLAY_DEL, overlay);
   if (overlay->del_cb)
     overlay->del_cb
       (overlay->del_cb_data, (overlay->wsd)->obj, overlay);

   if (overlay->grp)
     {
        if (overlay->grp->klass)
          elm_map_overlay_class_remove(overlay->grp->klass, overlay);
        _overlay_group_free(overlay->grp);
     }

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
     _overlay_bubble_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_ROUTE)
     _overlay_route_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_LINE)
     _overlay_line_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_POLYGON)
     _overlay_polygon_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CIRCLE)
     _overlay_circle_free(overlay->ovl);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_SCALE)
     _overlay_scale_free(overlay->ovl);
   else ERR("Invalid overlay type: %d", overlay->type);

   overlay->wsd->overlays = eina_list_remove(overlay->wsd->overlays, overlay);
   evas_object_smart_changed(overlay->wsd->pan_obj);

   free(overlay);
}

EAPI Elm_Map_Overlay_Type
elm_map_overlay_type_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, ELM_MAP_OVERLAY_TYPE_NONE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, ELM_MAP_OVERLAY_TYPE_NONE);
   ELM_MAP_CHECK((overlay->wsd)->obj) ELM_MAP_OVERLAY_TYPE_NONE;

   return overlay->type;
}

EAPI void
elm_map_overlay_data_set(Elm_Map_Overlay *overlay,
                         void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   overlay->data = data;
}

EAPI void *
elm_map_overlay_data_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, NULL);
   ELM_MAP_CHECK((overlay->wsd)->obj) NULL;

   return overlay->data;
}

EAPI void
elm_map_overlay_hide_set(Elm_Map_Overlay *overlay,
                         Eina_Bool hide)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->hide == !!hide) return;
   overlay->hide = hide;

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI Eina_Bool
elm_map_overlay_hide_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, EINA_FALSE);
   ELM_MAP_CHECK((overlay->wsd)->obj) EINA_FALSE;

   return overlay->hide;
}

EAPI void
elm_map_overlay_displayed_zoom_min_set(Elm_Map_Overlay *overlay,
                                       int zoom)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   overlay->zoom_min = zoom;
   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI int
elm_map_overlay_displayed_zoom_min_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, 0);
   ELM_MAP_CHECK((overlay->wsd)->obj) 0;

   return overlay->zoom_min;
}

EAPI void
elm_map_overlay_paused_set(Elm_Map_Overlay *overlay,
                           Eina_Bool paused)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->paused == !!paused) return;
   overlay->paused = paused;

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI Eina_Bool
elm_map_overlay_paused_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, EINA_FALSE);
   ELM_MAP_CHECK((overlay->wsd)->obj) EINA_FALSE;

   return overlay->paused;
}

EAPI Eina_Bool
elm_map_overlay_visible_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, EINA_FALSE);
   ELM_MAP_CHECK((overlay->wsd)->obj) EINA_FALSE;

   return overlay->visible;
}

EAPI void
elm_map_overlay_show(Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        Overlay_Default *ovl = overlay->ovl;

        elm_map_region_show
          ((overlay->wsd)->obj, ovl->lon, ovl->lat);
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
     {
        Overlay_Bubble *ovl = overlay->ovl;

        elm_map_region_show
          ((overlay->wsd)->obj, ovl->lon, ovl->lat);
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     {
        Overlay_Class *ovl = overlay->ovl;
        double lon, lat, max_lo, min_lo, max_la, min_la;

        _region_max_min_get(ovl->members, &max_lo, &min_lo, &max_la, &min_la);
        lon = (max_lo + min_lo) / 2;
        lat = (max_la + min_la) / 2;
        elm_map_region_show((overlay->wsd)->obj, lon, lat);
     }
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI void
elm_map_overlays_show(Eina_List *overlays)
{
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(overlays);
   EINA_SAFETY_ON_FALSE_RETURN(eina_list_count(overlays));

   overlay = eina_list_data_get(overlays);

   overlay->wsd->calc_job.overlays = overlays;
   overlay->wsd->calc_job.overlays_show = _overlays_show;

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI void
elm_map_overlay_region_set(Elm_Map_Overlay *overlay,
                           double lon,
                           double lat)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        Overlay_Default *ovl = overlay->ovl;

        ovl->lon = lon;
        ovl->lat = lat;
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
     {
        Overlay_Bubble *ovl = overlay->ovl;

        ovl->pobj = NULL;
        ovl->lon = lon;
        ovl->lat = lat;
     }
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI void
elm_map_overlay_region_get(const Elm_Map_Overlay *overlay,
                           double *lon,
                           double *lat)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_GROUP)
     {
        Overlay_Group *ovl = overlay->ovl;

        if (lon) *lon = ovl->lon;
        if (lat) *lat = ovl->lat;
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        const Overlay_Default *ovl = overlay->ovl;

        if (lon) *lon = ovl->lon;
        if (lat) *lat = ovl->lat;
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
     {
        const Overlay_Bubble *ovl = overlay->ovl;

        if (lon) *lon = ovl->lon;
        if (lat) *lat = ovl->lat;
     }
   else ERR("Not supported overlay type: %d", overlay->type);
}

EAPI void
elm_map_overlay_icon_set(Elm_Map_Overlay *overlay,
                         Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_icon_update(overlay->ovl, icon);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_icon_update(overlay->ovl, icon);
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI const Evas_Object *
elm_map_overlay_icon_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, NULL);
   ELM_MAP_CHECK((overlay->wsd)->obj) NULL;

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        const Overlay_Default *ovl = overlay->ovl;

        return ovl->icon;
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     {
        const Overlay_Class *ovl = overlay->ovl;

        return ovl->icon;
     }
   else
     {
        ERR("Not supported overlay type: %d", overlay->type);
        return NULL;
     }
}

EAPI void
elm_map_overlay_content_set(Elm_Map_Overlay *overlay,
                            Evas_Object *content)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(content);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_content_update(overlay->ovl, content, overlay);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_content_update(overlay->ovl, content);
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
}

EAPI const Evas_Object *
elm_map_overlay_content_get(const Elm_Map_Overlay *overlay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, NULL);

   ELM_MAP_CHECK((overlay->wsd)->obj) NULL;

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        const Overlay_Default *ovl = overlay->ovl;
        return ovl->content;
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     {
        const Overlay_Class *ovl = overlay->ovl;
        return ovl->content;
     }
   else
     {
        ERR("Not supported overlay type: %d", overlay->type);
        return NULL;
     }
}

EAPI void
elm_map_overlay_color_set(Elm_Map_Overlay *overlay,
                          int r,
                          int g,
                          int b,
                          int a)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   overlay->c.r = r;
   overlay->c.g = g;
   overlay->c.b = b;
   overlay->c.a = a;

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_color_update(overlay->ovl, overlay->c);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_color_update(overlay->ovl, overlay->c);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_ROUTE)
     _overlay_route_color_update(overlay->ovl, overlay->c);
   else
     {
        ERR("Not supported overlay type: %d", overlay->type);
     }
}

EAPI void
elm_map_overlay_color_get(const Elm_Map_Overlay *overlay,
                          int *r,
                          int *g,
                          int *b,
                          int *a)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   switch (overlay->type)
     {
        case ELM_MAP_OVERLAY_TYPE_ROUTE:
        case ELM_MAP_OVERLAY_TYPE_DEFAULT:
        case ELM_MAP_OVERLAY_TYPE_CLASS:
           if (r) *r = overlay->c.r;
           if (g) *g = overlay->c.g;
           if (b) *b = overlay->c.b;
           if (a) *a = overlay->c.a;
           break;

        default:
           ERR("Not supported overlay type: %d", overlay->type);
           break;
     }
}

EAPI void
elm_map_overlay_get_cb_set(Elm_Map_Overlay *overlay,
                           Elm_Map_Overlay_Get_Cb get_cb,
                           void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   overlay->cb = get_cb;
   overlay->cb_data = data;

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_cb_set(overlay->ovl, get_cb, data);
}

EAPI void
elm_map_overlay_del_cb_set(Elm_Map_Overlay *overlay,
                           Elm_Map_Overlay_Del_Cb del_cb,
                           void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK((overlay->wsd)->obj);

   overlay->del_cb = del_cb;
   overlay->del_cb_data = data;
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_class_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_CLASS;
   overlay->ovl = _overlay_class_new(sd);
   overlay->c.r = 0xff;
   overlay->c.g = 0xff;
   overlay->c.b = 0xff;
   overlay->c.a = 0xff;
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EAPI void
elm_map_overlay_class_append(Elm_Map_Overlay *klass,
                             Elm_Map_Overlay *overlay)
{
   Overlay_Class *class_ovl;

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(klass->wsd);
   ELM_MAP_CHECK((klass->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(klass->type == ELM_MAP_OVERLAY_TYPE_CLASS);

   class_ovl = klass->ovl;
   if (eina_list_data_find(class_ovl->members, overlay))
     {
        ERR("Already added overlay into clas");
        return;
     }
   class_ovl->members = eina_list_append(class_ovl->members, overlay);

   // Update group by class
   overlay->grp->klass = klass;
   _overlay_group_icon_update(overlay->grp, class_ovl->icon);
   _overlay_group_content_update(overlay->grp, class_ovl->content, overlay);
   _overlay_group_color_update(overlay->grp, klass->c);
   _overlay_group_cb_set(overlay->grp, klass->cb, klass->data);
   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        _overlay_default_class_icon_update(overlay->ovl, class_ovl->icon);
        _overlay_default_class_content_update
          (overlay->ovl, class_ovl->content);
     }

   evas_object_smart_changed(klass->wsd->pan_obj);
}

EAPI void
elm_map_overlay_class_remove(Elm_Map_Overlay *klass,
                             Elm_Map_Overlay *overlay)
{
   Overlay_Class *ovl;

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(klass->wsd);
   ELM_MAP_CHECK((klass->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(klass->type == ELM_MAP_OVERLAY_TYPE_CLASS);

   ovl = klass->ovl;
   ovl->members = eina_list_remove(ovl->members, overlay);

   overlay->grp->klass = NULL;
   _overlay_group_icon_update(overlay->grp, NULL);
   _overlay_group_content_update(overlay->grp, NULL, NULL);
   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        _overlay_default_class_icon_update(overlay->ovl, NULL);
        _overlay_default_class_content_update(overlay->ovl, NULL);
     }

   evas_object_smart_changed(klass->wsd->pan_obj);
}

EAPI void
elm_map_overlay_class_zoom_max_set(Elm_Map_Overlay *klass,
                                   int zoom)
{
   Overlay_Class *ovl;

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(klass->wsd);
   ELM_MAP_CHECK((klass->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(klass->type == ELM_MAP_OVERLAY_TYPE_CLASS);

   ovl = klass->ovl;
   if (ovl->zoom_max == !!zoom) return;
   ovl->zoom_max = zoom;

   evas_object_smart_changed(klass->wsd->pan_obj);
}

EAPI int
elm_map_overlay_class_zoom_max_get(const Elm_Map_Overlay *klass)
{
   const Overlay_Class *ovl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(klass, OVERLAY_CLASS_ZOOM_MAX);
   EINA_SAFETY_ON_NULL_RETURN_VAL(klass->wsd, OVERLAY_CLASS_ZOOM_MAX);
   ELM_MAP_CHECK((klass->wsd)->obj) OVERLAY_CLASS_ZOOM_MAX;

   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (klass->type == ELM_MAP_OVERLAY_TYPE_CLASS, OVERLAY_CLASS_ZOOM_MAX);

   ovl = klass->ovl;
   return ovl->zoom_max;
}

EAPI Eina_List *
elm_map_overlay_group_members_get(const Elm_Map_Overlay *grp)
{
   Overlay_Group *ovl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(grp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(grp->wsd, NULL);
   ELM_MAP_CHECK((grp->wsd)->obj) NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (grp->type == ELM_MAP_OVERLAY_TYPE_GROUP, NULL);

   ovl = grp->ovl;
   return ovl->members;
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_bubble_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_BUBBLE;
   overlay->c.r = 0xff;
   overlay->c.g = 0xff;
   overlay->c.b = 0xff;
   overlay->c.a = 0xff;
   overlay->ovl = _overlay_bubble_new(overlay);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EAPI void
elm_map_overlay_bubble_follow(Elm_Map_Overlay *bubble,
                              const Elm_Map_Overlay *parent)
{
   Overlay_Bubble *ovl;
   Evas_Object *pobj;

   EINA_SAFETY_ON_NULL_RETURN(bubble);
   EINA_SAFETY_ON_NULL_RETURN(parent);
   ELM_MAP_CHECK((bubble->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(bubble->type == ELM_MAP_OVERLAY_TYPE_BUBBLE);

   ovl = bubble->ovl;
   pobj = _overlay_obj_get(parent);
   if (!pobj) return;

   ovl->pobj = pobj;
   evas_object_smart_changed(bubble->wsd->pan_obj);
}

EAPI void
elm_map_overlay_bubble_content_append(Elm_Map_Overlay *bubble,
                                      Evas_Object *content)
{
   Overlay_Bubble *bb;

   EINA_SAFETY_ON_NULL_RETURN(bubble);
   EINA_SAFETY_ON_NULL_RETURN(content);
   ELM_MAP_CHECK((bubble->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(bubble->type == ELM_MAP_OVERLAY_TYPE_BUBBLE);

   bb = bubble->ovl;
   elm_box_pack_end(bb->bx, content);

   evas_object_smart_changed(bubble->wsd->pan_obj);
}

EAPI void
elm_map_overlay_bubble_content_clear(Elm_Map_Overlay *bubble)
{
   Overlay_Bubble *bb;

   EINA_SAFETY_ON_NULL_RETURN(bubble);
   ELM_MAP_CHECK((bubble->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(bubble->type == ELM_MAP_OVERLAY_TYPE_BUBBLE);

   bb = bubble->ovl;
   elm_box_clear(bb->bx);

   evas_object_smart_changed(bubble->wsd->pan_obj);
}

EAPI void
elm_map_wheel_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   elm_interface_scrollable_wheel_disabled_set(obj, disabled);
}

EAPI Eina_Bool
elm_map_wheel_disabled_get(const Evas_Object *obj)
{
   return elm_interface_scrollable_wheel_disabled_get(obj);
}

EAPI void
elm_map_paused_set(Evas_Object *obj, Eina_Bool paused)
{
   efl_ui_zoom_animation_set(obj, paused);
}

EAPI Eina_Bool
elm_map_paused_get(const Evas_Object *obj)
{
   return efl_ui_zoom_animation_get(obj);
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_route_add(Eo *obj, Elm_Map_Data *sd, const Elm_Map_Route *route)
{
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(route->wsd, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (obj == (route->wsd)->obj, NULL);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_ROUTE;
   overlay->c.r = 0x11;
   overlay->c.g = 0x11;
   overlay->c.b = 0xcc;
   overlay->c.a = 0xcc;
   overlay->ovl = _overlay_route_new(sd, route, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_line_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double flon, double flat, double tlon, double tlat)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_LINE;
   overlay->c.r = 0x11;
   overlay->c.g = 0xcc;
   overlay->c.b = 0x11;
   overlay->c.a = 0xcc;
   overlay->ovl = _overlay_line_new(sd, flon, flat, tlon, tlat, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_polygon_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_POLYGON;
   overlay->c.r = 0xcc;
   overlay->c.g = 0x77;
   overlay->c.b = 0x11;
   overlay->c.a = 0xcc;
   overlay->ovl = _overlay_polygon_new(sd, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);
   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EAPI void
elm_map_overlay_polygon_region_add(Elm_Map_Overlay *overlay,
                                   double lon,
                                   double lat)
{
   Overlay_Polygon *ovl;
   Region *r;

   EINA_SAFETY_ON_NULL_RETURN(overlay);
   ELM_MAP_CHECK((overlay->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(overlay->type == ELM_MAP_OVERLAY_TYPE_POLYGON);

   ovl = overlay->ovl;
   r = ELM_NEW(Region);
   r->lon = lon;
   r->lat = lat;
   ovl->regions = eina_list_append(ovl->regions, r);

   evas_object_smart_changed(ovl->wsd->pan_obj);
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_circle_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, double lon, double lat, double radius)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_CIRCLE;
   overlay->c.r = 0xcc;
   overlay->c.g = 0x11;
   overlay->c.b = 0x11;
   overlay->c.a = 0xcc;
   overlay->ovl = _overlay_circle_new(sd, lon, lat, radius, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_scale_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Evas_Coord x, Evas_Coord y)
{
   Elm_Map_Overlay *overlay;

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_SCALE;
   overlay->c.r = 0xff;
   overlay->c.g = 0xff;
   overlay->c.b = 0xff;
   overlay->c.a = 0xff;
   overlay->ovl = _overlay_scale_new(sd, x, y, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
}

EOLIAN static Evas_Object*
_elm_map_track_add(Eo *obj, Elm_Map_Data *sd, void *emap)
{
#ifdef ELM_EMAP
   EMap_Route *emapr = emap;
   Evas_Object *route;

   route = elm_route_add(obj);
   elm_route_emap_set(route, emapr);
   sd->track = eina_list_append(sd->track, route);

   return route;
#else
   (void)obj;
   (void)sd;
   (void)emap;
   return NULL;
#endif
}

EOLIAN static void
_elm_map_track_remove(Eo *obj EINA_UNUSED, Elm_Map_Data *sd, Evas_Object *route)
{
#ifdef ELM_EMAP
   sd->track = eina_list_remove(sd->track, route);
   evas_object_del(route);
#else
   (void)obj;
   (void)sd;
   (void)route;
#endif
}

static void
_elm_map_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN const Efl_Access_Action_Data *
_elm_map_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "move,prior", "move", "prior", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,left", "move", "left", _key_action_move},
          { "move,right", "move", "right", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { "zoom,in", "zoom", "in", _key_action_zoom},
          { "zoom,out", "zoom", "out", _key_action_zoom},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(elm_map, Elm_Map_Data)

/* Internal EO APIs and hidden overrides */

#define ELM_MAP_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_map)

#include "elm_map_eo.c"
