#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_map.h"

EAPI const char ELM_MAP_SMART_NAME[] = "elm_map";
EAPI const char ELM_MAP_PAN_SMART_NAME[] = "elm_map_pan";

#define OVERLAY_CLASS_ZOOM_MAX  255
#define MAX_CONCURRENT_DOWNLOAD 10

#define ROUND(z) (((z) < 0) ? (int)ceil((z) - 0.005) : (int)floor((z) + 0.005))
#define EVAS_MAP_POINT         4
#define DEFAULT_TILE_SIZE      256
#define MARER_MAX_NUMBER       30
#define OVERLAY_GROUPING_SCALE 2

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

#ifdef HAVE_ELEMENTARY_ECORE_CON

static char *
_mapnik_url_cb(const Evas_Object *obj __UNUSED__,
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
_osmarender_url_cb(const Evas_Object *obj __UNUSED__,
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
_cyclemap_url_cb(const Evas_Object *obj __UNUSED__,
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
_mapquest_url_cb(const Evas_Object *obj __UNUSED__,
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
_mapquest_aerial_url_cb(const Evas_Object *obj __UNUSED__,
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
_yours_url_cb(const Evas_Object *obj __UNUSED__,
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
   _monav_url_cb(const Evas_Object *obj __UNUSED__,
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
   _ors_url_cb(const Evas_Object *obj __UNUSED__,
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
   char buf[PATH_MAX];
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

        if (str && str[0])
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
const double _osm_scale_meter[] =
{
   78206, 39135.758482, 19567.879241, 9783.939621, 4891.969810,
   2445.984905, 1222.992453, 611.496226, 305.748113, 152.874057, 76.437028,
   38.218514, 19.109257, 9.554629, 4.777314, 2.388657, 1.194329, 0.597164,
   0.29858
};

static double
_scale_cb(const Evas_Object *obj __UNUSED__,
          double lon __UNUSED__,
          double lat,
          int zoom)
{
   if (zoom < 0 || zoom >= (int)sizeof(_osm_scale_meter)) return 0;
   return _osm_scale_meter[zoom] / cos(lat * ELM_PI / 180.0);
}

const Source_Tile src_tiles[] =
{
   {"Mapnik", 0, 18, _mapnik_url_cb, NULL, NULL, _scale_cb},
   {"Osmarender", 0, 17, _osmarender_url_cb, NULL, NULL, _scale_cb},
   {"CycleMap", 0, 16, _cyclemap_url_cb, NULL, NULL, _scale_cb},
   {"MapQuest", 0, 18, _mapquest_url_cb, NULL, NULL, _scale_cb},
   {"MapQuest Open Aerial", 0, 11, _mapquest_aerial_url_cb, NULL, NULL,
    _scale_cb}
};

// FIXME: Fix more open sources
const Source_Route src_routes[] =
{
   {"Yours", _yours_url_cb}    // http://www.yournavigation.org/
   //{"Monav", _monav_url_cb},
   //{"ORS", _ors_url_cb},     // http://www.openrouteservice.org
};

// Scale in meters
const double _scale_tb[] =
{
   10000000, 5000000, 2000000, 1000000, 500000, 200000, 100000, 50000,
   20000, 10000, 5000, 2000, 1000, 500, 500, 200, 100, 50, 20, 10, 5, 2, 1
};

// FIXME: Add more open sources
const Source_Name src_names[] =
{
   {"Nominatim", _nominatim_url_cb}
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
static const char SIG_ZOOM_START[] = "zoom,start";
static const char SIG_ZOOM_STOP[] = "zoom,stop";
static const char SIG_ZOOM_CHANGE[] = "zoom,change";
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
   {SIG_ZOOM_START, ""},
   {SIG_ZOOM_STOP, ""},
   {SIG_ZOOM_CHANGE, ""},
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
   {NULL, NULL}
};

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_MAP_SMART_NAME, _elm_map, Elm_Map_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks,
  _smart_interfaces);

ELM_INTERNAL_SMART_SUBCLASS_NEW
  (ELM_MAP_PAN_SMART_NAME, _elm_map_pan, Elm_Map_Pan_Smart_Class,
  Elm_Pan_Smart_Class, elm_pan_smart_class_get, NULL);

static void
_edj_overlay_size_get(Elm_Map_Smart_Data *sd,
                      Evas_Coord *w,
                      Evas_Coord *h)
{
   Evas_Object *edj;
   const char *s;

   EINA_SAFETY_ON_NULL_RETURN(w);
   EINA_SAFETY_ON_NULL_RETURN(h);

   edj = edje_object_add(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   elm_widget_theme_object_set
     (ELM_WIDGET_DATA(sd)->obj, edj, "map/marker", "radio",
     elm_widget_style_get(ELM_WIDGET_DATA(sd)->obj));

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
_obj_rotate(Elm_Map_Smart_Data *sd,
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

   evas_object_move(obj, x, y);
   evas_object_resize(obj, w, h);
   evas_object_show(obj);
}

static void
_coord_to_region_convert(Elm_Map_Smart_Data *sd,
                         Evas_Coord x,
                         Evas_Coord y,
                         Evas_Coord size,
                         double *lon,
                         double *lat)
{
   int zoom;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   zoom = floor(log(size / sd->size.tile) / log(2));
   if ((sd->src_tile) && (sd->src_tile->coord_to_geo))
     {
        if (sd->src_tile->coord_to_geo
              (ELM_WIDGET_DATA(sd)->obj, zoom, x, y, size, lon, lat))
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
_region_to_coord_convert(Elm_Map_Smart_Data *sd,
                         double lon,
                         double lat,
                         Evas_Coord size,
                         Evas_Coord *x,
                         Evas_Coord *y)
{
   int zoom;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   zoom = floor(log(size / 256) / log(2));
   if ((sd->src_tile) && (sd->src_tile->geo_to_coord))
     {
        if (sd->src_tile->geo_to_coord
              (ELM_WIDGET_DATA(sd)->obj, zoom, lon, lat, size, x, y)) return;
     }

   if (x) *x = floor((lon + 180.0) / 360.0 * size);
   if (y)
     *y = floor((1.0 - log(tan(lat * ELM_PI / 180.0) +
                           (1.0 / cos(lat * ELM_PI / 180.0)))
                 / ELM_PI) / 2.0 * size);
}

static void
_viewport_coord_get(Elm_Map_Smart_Data *sd,
                    Evas_Coord *vx,
                    Evas_Coord *vy,
                    Evas_Coord *vw,
                    Evas_Coord *vh)
{
   Evas_Coord x, y, w, h;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   sd->s_iface->content_pos_get(ELM_WIDGET_DATA(sd)->obj, &x, &y);
   sd->s_iface->content_viewport_size_get(ELM_WIDGET_DATA(sd)->obj, &w, &h);

   if (w > sd->size.w) x -= ((w - sd->size.w) / 2);
   if (h > sd->size.h) y -= ((h - sd->size.h) / 2);
   if (vx) *vx = x;
   if (vy) *vy = y;
   if (vw) *vw = w;
   if (vh) *vh = h;
}

// Map coordinates to canvas geometry without rotate
static void
_coord_to_canvas_no_rotation(Elm_Map_Smart_Data *sd,
                             Evas_Coord x,
                             Evas_Coord y,
                             Evas_Coord *xx,
                             Evas_Coord *yy)
{
   Evas_Coord vx, vy, sx, sy;

   _viewport_coord_get(sd, &vx, &vy, NULL, NULL);
   evas_object_geometry_get(sd->pan_obj, &sx, &sy, NULL, NULL);
   if (xx) *xx = x - vx + sx;
   if (yy) *yy = y - vy + sy;
}

// Map coordinates to canvas geometry
static void
_coord_to_canvas(Elm_Map_Smart_Data *sd,
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
_canvas_to_coord(Elm_Map_Smart_Data *sd,
                 Evas_Coord x,
                 Evas_Coord y,
                 Evas_Coord *xx,
                 Evas_Coord *yy)
{
   Evas_Coord vx, vy, sx, sy;

   _viewport_coord_get(sd, &vx, &vy, NULL, NULL);
   evas_object_geometry_get(sd->pan_obj, &sx, &sy, NULL, NULL);
   _rotate_do(x - sx + vx, y - sy + vy, sd->rotate.cx - sx + vx,
              sd->rotate.cy - sy + vy, -sd->rotate.d, &x, &y);
   if (xx) *xx = x;
   if (yy) *yy = y;
}

static void
_region_show(void *data)
{
   Delayed_Data *dd = data;
   int x, y, w, h;

   EINA_SAFETY_ON_NULL_RETURN(data);

   _region_to_coord_convert
     (dd->wsd, dd->lon, dd->lat, dd->wsd->size.w, &x, &y);
   _viewport_coord_get(dd->wsd, NULL, NULL, &w, &h);
   x = x - (w / 2);
   y = y - (h / 2);
   dd->wsd->s_iface->content_region_show
     (ELM_WIDGET_DATA(dd->wsd)->obj, x, y, w, h);
   evas_object_smart_changed(dd->wsd->pan_obj);
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
   Evas_Coord vx, vy, vw, vh;
   Evas_Coord x, y, w, h;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gi, EINA_FALSE);

   _viewport_coord_get(gi->wsd, &vx, &vy, &vw, &vh);
   _grid_item_coord_get(gi, &x, &y, &w, &h);

   return ELM_RECTS_INTERSECT(x, y, w, h, vx, vy, vw, vh);
}

static Eina_Bool
_loaded_timeout_cb(void *data)
{
   Elm_Map_Smart_Data *sd = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);

   sd->loaded_timer = NULL;
   if (!(sd->download_num) && !(sd->download_idler))
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_LOADED, NULL);
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

   if (gi->wsd->loaded_timer) ecore_timer_del(gi->wsd->loaded_timer);
   gi->wsd->loaded_timer = ecore_timer_add(0.25, _loaded_timeout_cb, gi->wsd);
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
   char buf2[PATH_MAX];
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
       (evas_object_evas_get(ELM_WIDGET_DATA(g->wsd)->obj));
   evas_object_image_smooth_scale_set(gi->img, EINA_FALSE);
   evas_object_image_scale_hint_set(gi->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
   evas_object_image_filled_set(gi->img, EINA_TRUE);
   evas_object_smart_member_add(gi->img, g->wsd->pan_obj);
   evas_object_pass_events_set(gi->img, EINA_TRUE);
   evas_object_stack_below(gi->img, g->wsd->sep_maps_overlays);

   {
      const char *cachedir;

#ifdef ELM_EFREET
      snprintf(buf, sizeof(buf), "%s" CACHE_TILE_ROOT, efreet_cache_home_get(),
               g->wsd->id, g->zoom, x);
      (void)cachedir;
#else
      cachedir = getenv("XDG_CACHE_HOME");
      snprintf(buf, sizeof(buf), "%s/%s" CACHE_TILE_ROOT, getenv("HOME"),
               cachedir ? : "/.config", g->wsd->id, g->zoom, x);
#endif
   }

   snprintf(buf2, sizeof(buf2), CACHE_TILE_PATH, buf, y);
   if (!ecore_file_exists(buf)) ecore_file_mkpath(buf);

   eina_stringshare_replace(&gi->file, buf2);
   url = g->wsd->src_tile->url_cb(ELM_WIDGET_DATA(g->wsd)->obj, x, y, g->zoom);
   if ((!url) || (!strlen(url)))
     {
        eina_stringshare_replace(&gi->url, NULL);
        ERR("Getting source url failed: %s", gi->file);
     }
   else eina_stringshare_replace(&gi->url, url);

   if (url) free(url);
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
   if (gi->url) eina_stringshare_del(gi->url);
   if (gi->file_have) ecore_file_remove(gi->file);
   if (gi->file) eina_stringshare_del(gi->file);
   if (gi->img) evas_object_del(gi->img);

   free(gi);
}

static void
_downloaded_cb(void *data,
               const char *file __UNUSED__,
               int status)
{
   Grid_Item *gi = data;

   if (status == 200)
     {
        DBG("Download success from %s to %s", gi->url, gi->file);

        _grid_item_update(gi);
        gi->wsd->finish_num++;
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(gi->wsd)->obj, SIG_TILE_LOADED, NULL);
     }
   else
     {
        WRN("Download failed from %s to %s (%d) ", gi->url, gi->file, status);

        ecore_file_remove(gi->file);
        gi->file_have = EINA_FALSE;
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(gi->wsd)->obj, SIG_TILE_LOADED_FAIL, NULL);
     }

   gi->job = NULL;
   gi->wsd->download_num--;
   if (!gi->wsd->download_num)
     edje_object_signal_emit(ELM_WIDGET_DATA(gi->wsd)->resize_obj,
                             "elm,state,busy,stop", "elm");
}

static Eina_Bool
_download_job(void *data)
{
   Elm_Map_Smart_Data *sd = data;
   Eina_List *l, *ll;
   Grid_Item *gi;

   if (!eina_list_count(sd->download_list))
     {
        sd->download_idler = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   EINA_LIST_REVERSE_FOREACH_SAFE(sd->download_list, l, ll, gi)
   {
      Eina_Bool ret;

      if ((gi->g->zoom != sd->zoom) || !(_grid_item_in_viewport(gi)))
        {
           sd->download_list = eina_list_remove(sd->download_list, gi);
           continue;
        }
      if (sd->download_num >= MAX_CONCURRENT_DOWNLOAD)
        return ECORE_CALLBACK_RENEW;

      ret = ecore_file_download_full
          (gi->url, gi->file, _downloaded_cb, NULL, gi, &(gi->job), sd->ua);

      if ((!ret) || (!gi->job))
        ERR("Can't start to download from %s to %s", gi->url, gi->file);
      else
        {
           sd->download_list = eina_list_remove(sd->download_list, gi);
           sd->try_num++;
           sd->download_num++;
           evas_object_smart_callback_call
             (ELM_WIDGET_DATA(sd)->obj, SIG_TILE_LOAD, NULL);
           if (sd->download_num == 1)
             edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                                     "elm,state,busy,start", "elm");
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
   Evas_Coord vx, vy, vw, vh;

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
   EINA_ITERATOR_FOREACH (it, cell)
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
   EINA_ITERATOR_FOREACH (it, cell)
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
_grid_place(Elm_Map_Smart_Data *sd)
{
   Eina_List *l;
   Grid *g;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   EINA_LIST_FOREACH (sd->grids, l, g)
     {
        if (sd->zoom == g->zoom) _grid_load(g);
        else _grid_unload(g);
     }
   if (!sd->download_idler)
     sd->download_idler = ecore_idler_add(_download_job, sd);
}

static void
_grid_all_create(Elm_Map_Smart_Data *sd)
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
_grid_all_clear(Elm_Map_Smart_Data *sd)
{
   Grid *g;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   EINA_LIST_FREE (sd->grids, g)
     {
        Eina_Matrixsparse_Cell *cell;
        Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);

        EINA_ITERATOR_FOREACH (it, cell)
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
_track_place(Elm_Map_Smart_Data *sd)
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

   EINA_LIST_FOREACH (sd->track, l, route)
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
                  evas_object_move(route, xmin - px, ymin - py);
                  evas_object_resize(route, xmax - xmin, ymax - ymin);

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
_delayed_do(Elm_Map_Smart_Data *sd)
{
   Delayed_Data *dd;

   dd = eina_list_nth(sd->delayed_jobs, 0);
   if (dd && !dd->wsd->zoom_animator)
     {
        dd->func(dd);
        sd->delayed_jobs = eina_list_remove(sd->delayed_jobs, dd);
        free(dd);
     }
}

static void
_smooth_update(Elm_Map_Smart_Data *sd)
{
   Eina_List *l;
   Grid *g;

   EINA_LIST_FOREACH (sd->grids, l, g)
     {
        Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);
        Eina_Matrixsparse_Cell *cell;

        EINA_ITERATOR_FOREACH (it, cell)
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
   Elm_Map_Smart_Data *sd = data;

   _smooth_update(sd);
   sd->zoom_timer = NULL;
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_ZOOM_STOP, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_zoom_do(Elm_Map_Smart_Data *sd,
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
   // from zooming with (0,0) as the cetner. (scroller default behavior)
   _viewport_coord_get(sd, &vx, &vy, &vw, &vh);
   if ((vw > 0) && (vh > 0) && (ow > 0) && (oh > 0))
     {
        Evas_Coord x, y;
        double sx, sy;

        if (vw > ow) sx = 0.5;
        else sx = (double)(vx + (double)(vw / 2)) / ow;
        if (vh > oh) sy = 0.5;
        else sy = (double)(vy + (double)(vh / 2)) / oh;

        if (sx > 1.0) sx = 1.0;
        if (sy > 1.0) sy = 1.0;

        x = ceil((sx * sd->size.w) - (vw / 2));
        y = ceil((sy * sd->size.h) - (vh / 2));
        if (x < 0) x = 0;
        else if (x > (sd->size.w - vw))
          x = sd->size.w - vw;
        if (y < 0) y = 0;
        else if (y > (sd->size.h - vh))
          y = sd->size.h - vh;
        sd->s_iface->content_region_show
          (ELM_WIDGET_DATA(sd)->obj, x, y, vw, vh);
     }

   if (sd->zoom_timer) ecore_timer_del(sd->zoom_timer);
   else
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_ZOOM_START, NULL);

   sd->zoom_timer = ecore_timer_add(0.25, _zoom_timeout_cb, sd);
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_ZOOM_CHANGE, NULL);

   evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
   evas_object_smart_changed(sd->pan_obj);
}

static Eina_Bool
_zoom_anim_cb(void *data)
{
   Elm_Map_Smart_Data *sd = data;

   if (sd->ani.cnt <= 0)
     {
        sd->zoom_animator = NULL;
        evas_object_smart_changed(sd->pan_obj);

        return ECORE_CALLBACK_CANCEL;
     }
   else
     {
        sd->ani.zoom += sd->ani.diff;
        sd->ani.cnt--;
        _zoom_do(sd, sd->ani.zoom);

        return ECORE_CALLBACK_RENEW;
     }
}

static void
_zoom_with_animation(Elm_Map_Smart_Data *sd,
                     double zoom,
                     int cnt)
{
   if (cnt == 0) return;

   sd->ani.cnt = cnt;
   sd->ani.zoom = sd->zoom;
   sd->ani.diff = (double)(zoom - sd->zoom) / cnt;
   if (sd->zoom_animator) ecore_animator_del(sd->zoom_animator);
   sd->zoom_animator = ecore_animator_add(_zoom_anim_cb, sd);
}

static void
_sizing_eval(Elm_Map_Smart_Data *sd)
{
   Evas_Coord maxw = -1, maxh = -1;

   evas_object_size_hint_max_get(ELM_WIDGET_DATA(sd)->obj, &maxw, &maxh);
   evas_object_size_hint_max_set(ELM_WIDGET_DATA(sd)->obj, maxw, maxh);
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static Eina_Bool
_scr_timeout_cb(void *data)
{
   Elm_Map_Smart_Data *sd = data;

   _smooth_update(sd);
   sd->scr_timer = NULL;
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_SCROLL_DRAG_STOP, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_scroll_cb(Evas_Object *obj,
           void *data __UNUSED__)
{
   ELM_MAP_DATA_GET(obj, sd);

   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   else
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_SCROLL_DRAG_START, NULL);
   sd->scr_timer = ecore_timer_add(0.25, _scr_timeout_cb, sd);
   evas_object_smart_callback_call(ELM_WIDGET_DATA(sd)->obj, SIG_SCROLL, NULL);
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data __UNUSED__)
{
   ELM_MAP_DATA_GET(obj, sd);

   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data __UNUSED__)
{
   ELM_MAP_DATA_GET(obj, sd);

   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static Eina_Bool
_long_press_cb(void *data)
{
   Elm_Map_Smart_Data *sd = data;

   sd->long_timer = NULL;
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_LONGPRESSED, &sd->ev);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Elm_Map_Smart_Data *sd = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_CLICKED_DOUBLE, ev);
   else
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_PRESS, ev);

   if (sd->long_timer) ecore_timer_del(sd->long_timer);
   sd->ev = *ev;
   sd->long_timer =
     ecore_timer_add(_elm_config->longpress_timeout, _long_press_cb, sd);
}

static void
_mouse_up_cb(void *data,
             Evas *evas __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Elm_Map_Smart_Data *sd = data;
   Evas_Event_Mouse_Up *ev = event_info;

   EINA_SAFETY_ON_NULL_RETURN(ev);

   if (ev->button != 1) return;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (sd->long_timer)
     {
        ecore_timer_del(sd->long_timer);
        sd->long_timer = NULL;
     }

   if (!sd->on_hold)
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_CLICKED, ev);
   sd->on_hold = EINA_FALSE;
}

static void
_mouse_wheel_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info)
{
   Elm_Map_Smart_Data *sd = data;

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

   EINA_LIST_FREE (overlays, overlay)
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
_icon_dup(const Evas_Object *icon,
          Evas_Object *parent)
{
   // Evas_Object do not support object duplication??
   const char *file = NULL, *group = NULL;
   Eina_Bool size_up, size_down;
   Evas_Object *dupp;

   if (!icon || !parent) return NULL;

   dupp = elm_icon_add(parent);
   elm_image_file_get(icon, &file, &group);
   elm_image_file_set(dupp, file, group);
   elm_image_animated_set(dupp, elm_image_animated_get(icon));
   elm_image_animated_play_set(dupp, elm_image_animated_play_get(icon));
   elm_icon_standard_set(dupp, elm_icon_standard_get(icon));
   elm_icon_order_lookup_set(dupp, elm_icon_order_lookup_get(icon));
   elm_image_no_scale_set(dupp, elm_image_no_scale_get(icon));
   elm_image_resizable_get(icon, &size_up, &size_down);
   elm_image_resizable_set(dupp, size_up, size_down);
   elm_image_fill_outside_set(dupp, elm_image_fill_outside_get(icon));
   elm_image_prescale_set(dupp, elm_image_prescale_get(icon));
   elm_image_aspect_fixed_set(dupp, elm_image_aspect_fixed_get(icon));

   return dupp;
}

static void
_overlay_clicked_cb(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *ev __UNUSED__)
{
   Elm_Map_Overlay *overlay = data;

   EINA_SAFETY_ON_NULL_RETURN(data);

   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(overlay->wsd)->obj, SIG_OVERLAY_CLICKED, overlay);
   if (overlay->cb)
     overlay->cb(overlay->cb_data, ELM_WIDGET_DATA(overlay->wsd)->obj, overlay);
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
        if (w <= 0 || h <= 0) evas_object_size_hint_min_get(disp, &w, &h);
     }
   else if (!(ovl->icon) && ovl->clas_content)
     {
        disp = ovl->clas_content;

        evas_object_geometry_get(disp, NULL, NULL, &w, &h);
        if (w <= 0 || h <= 0) evas_object_size_hint_min_get(disp, &w, &h);
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
   if (ovl->content) evas_object_del(ovl->content);
   ovl->content = content;

   if (ovl->content)
     evas_object_event_callback_add(ovl->content, EVAS_CALLBACK_MOUSE_DOWN,
                                    _overlay_clicked_cb, overlay);
}

static void
_overlay_default_class_content_update(Overlay_Default *ovl,
                                      const Evas_Object *content __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->clas_content) evas_object_del(ovl->clas_content);
   // Evas_Object can not be duplicated.
   //ovl->clas_content = _content_dup(content);
}

static void
_overlay_default_layout_update(Overlay_Default *ovl)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->icon)
     {
        evas_object_color_set(ovl->layout, 255, 255, 255, 255);
        elm_layout_theme_set
          (ovl->layout, "map/marker", "empty",
          elm_widget_style_get(ELM_WIDGET_DATA(ovl->wsd)->obj));
        elm_object_part_content_set(ovl->layout, "elm.icon", ovl->icon);
     }
   else if (!ovl->icon && ovl->clas_icon)
     {
        evas_object_color_set(ovl->layout, 255, 255, 255, 255);
        elm_layout_theme_set
          (ovl->layout, "map/marker", "empty",
          elm_widget_style_get(ELM_WIDGET_DATA(ovl->wsd)->obj));
        elm_object_part_content_set(ovl->layout, "elm.icon", ovl->clas_icon);
     }
   else
     {
        evas_object_color_set
          (ovl->layout, ovl->c.r, ovl->c.g, ovl->c.b, ovl->c.a);
        elm_layout_theme_set
          (ovl->layout, "map/marker", "radio",
          elm_widget_style_get(ELM_WIDGET_DATA(ovl->wsd)->obj));
     }
}

static void
_overlay_default_icon_update(Overlay_Default *ovl,
                             Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->icon == icon) return;

   if (ovl->icon) evas_object_del(ovl->icon);
   ovl->icon = icon;
   _overlay_default_layout_update(ovl);
}

static void
_overlay_default_class_icon_update(Overlay_Default *ovl,
                                   const Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(ovl);

   if (ovl->clas_icon) evas_object_del(ovl->clas_icon);
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

   if (ovl->content) evas_object_del(ovl->content);
   if (ovl->icon) evas_object_del(ovl->icon);
   if (ovl->clas_content) evas_object_del(ovl->clas_content);
   if (ovl->clas_icon) evas_object_del(ovl->clas_icon);
   if (ovl->layout) evas_object_del(ovl->layout);

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
   ovl->layout = elm_layout_add(ELM_WIDGET_DATA(ovl->wsd)->obj);
   evas_object_smart_member_add(ovl->layout, ovl->wsd->pan_obj);
   evas_object_stack_above(ovl->layout, ovl->wsd->sep_maps_overlays);
   elm_layout_theme_set(ovl->layout, "map/marker", "radio",
                        elm_widget_style_get(ELM_WIDGET_DATA(ovl->wsd)->obj));
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

   if (grp->members) eina_list_free(grp->members);
   grp->members = members;
   snprintf(text, sizeof(text), "%d", eina_list_count(members));

   _overlay_default_layout_text_update(grp->ovl, text);
}

static void
_overlay_group_icon_update(Overlay_Group *grp,
                           const Evas_Object *icon)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   if (grp->ovl)
     _overlay_default_icon_update
       (grp->ovl, _icon_dup(icon, ELM_WIDGET_DATA(grp->wsd)->obj));
}

static void
_overlay_group_content_update(Overlay_Group *grp __UNUSED__,
                              const Evas_Object *content __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(grp);

   // Evas_Object can not be duplicated.
   //_overlay_default_content_update(grp->ovl, _content_dup(content));
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

   if (grp->overlay) free(grp->overlay);
   if (grp->ovl) _overlay_default_free(grp->ovl);
   if (grp->members) eina_list_free(grp->members);

   free(grp);
}

static Overlay_Group *
_overlay_group_new(Elm_Map_Smart_Data *sd)
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
   EINA_LIST_FOREACH (ovl->members, l, overlay)
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
   if (ovl->icon) evas_object_del(ovl->icon);
   ovl->icon = icon;

   // Update class members' class icons
   EINA_LIST_FOREACH (ovl->members, l, overlay)
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
   if (ovl->content) evas_object_del(ovl->content);
   ovl->content = content;

   // Update class members' class contents
   EINA_LIST_FOREACH (ovl->members, l, overlay)
     {
        _overlay_group_content_update(overlay->grp, content);

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
   EINA_LIST_FOREACH (ovl->members, l, overlay)
     _overlay_group_color_update(overlay->grp, c);
}

static void
_overlay_class_free(Overlay_Class *clas)
{
   Eina_List *l;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(clas);

   // Update class members' class contents
   EINA_LIST_FOREACH (clas->members, l, overlay)
     {
        overlay->grp->klass = NULL;
        _overlay_group_content_update(overlay->grp, NULL);
        _overlay_group_icon_update(overlay->grp, NULL);

        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          {
             _overlay_default_class_content_update(overlay->ovl, NULL);
             _overlay_default_class_icon_update(overlay->ovl, NULL);
          }
     }
   if (clas->icon) evas_object_del(clas->icon);
   if (clas->members) eina_list_free(clas->members);

   free(clas);
}

static Overlay_Class *
_overlay_class_new(Elm_Map_Smart_Data *sd)
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
     edje_object_add(evas_object_evas_get(ELM_WIDGET_DATA(overlay->wsd)->obj));
   elm_widget_theme_object_set
     (ELM_WIDGET_DATA(overlay->wsd)->obj, bubble->obj, "map", "marker_bubble",
     elm_widget_style_get(ELM_WIDGET_DATA(overlay->wsd)->obj));

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
   EINA_LIST_FOREACH (r->nodes, l, n)
     {
        p = eina_list_nth(r->paths, n->idx);
        if (!p) continue;

        _region_to_coord_convert
          (r->wsd, n->pos.lon, n->pos.lat, r->wsd->size.w, &p->x, &p->y);
        _coord_to_canvas(r->wsd, p->x, p->y, &p->x, &p->y);
     }
   EINA_LIST_FOREACH (r->paths, l, p)
     evas_object_polygon_point_add(r->obj, p->x - 3, p->y - 3);

   EINA_LIST_REVERSE_FOREACH (r->paths, l, p)
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

   EINA_LIST_FREE (route->paths, p)
     free(p);

   EINA_LIST_FREE (route->nodes, n)
     {
        if (n->pos.address) eina_stringshare_del(n->pos.address);
        free(n);
     }

   free(route);
}

static Overlay_Route *
_overlay_route_new(Elm_Map_Smart_Data *sd,
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
     evas_object_polygon_add(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   _overlay_route_color_update(ovl, c);

   EINA_LIST_FOREACH (route->nodes, l, n)
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
   Elm_Map_Smart_Data *sd = ovl->wsd;

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
_overlay_line_new(Elm_Map_Smart_Data *sd,
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
     evas_object_line_add(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
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
   Elm_Map_Smart_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   sd = ovl->wsd;

   evas_object_polygon_points_clear(ovl->obj);
   EINA_LIST_FOREACH (ovl->regions, l, r)
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

   EINA_LIST_FREE (ovl->regions, r)
     free(r);

   free(ovl);
}

static Overlay_Polygon *
_overlay_polygon_new(Elm_Map_Smart_Data *sd, Color c)
{
   Overlay_Polygon *ovl;

   ovl = ELM_NEW(Overlay_Polygon);
   ovl->wsd = sd;
   ovl->obj =
     evas_object_polygon_add(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
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
   Elm_Map_Smart_Data *sd;

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
_overlay_circle_new(Elm_Map_Smart_Data *sd,
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

   ovl->obj = elm_layout_add(ELM_WIDGET_DATA(sd)->obj);
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   evas_object_stack_above(ovl->obj, sd->sep_maps_overlays);
   elm_layout_theme_set(ovl->obj, "map/circle", "base",
                        elm_widget_style_get(ELM_WIDGET_DATA(sd)->obj));
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
   Elm_Map_Smart_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(ovl);

   sd = ovl->wsd;

   if ((int)sizeof(_scale_tb) <= sd->zoom)
     {
        ERR("Zoom level is too high");
        return;
     }

   elm_map_region_get(ELM_WIDGET_DATA(sd)->obj, &lon, &lat);
   meter =
     sd->src_tile->scale_cb(ELM_WIDGET_DATA(sd)->obj, lon, lat, sd->zoom);

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
_overlay_scale_new(Elm_Map_Smart_Data *sd,
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

   ovl->obj = elm_layout_add(ELM_WIDGET_DATA(sd)->obj);
   evas_object_smart_member_add(ovl->obj, sd->pan_obj);
   evas_object_stack_above(ovl->obj, sd->sep_maps_overlays);
   elm_layout_theme_set(ovl->obj, "map/scale", "base",
                        elm_widget_style_get(ELM_WIDGET_DATA(sd)->obj));
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

   EINA_LIST_FOREACH (clas_membs, l, memb)
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
   Elm_Map_Smart_Data *sd = overlay->wsd;

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
      default:
        ERR("Invalid overlay type to show: %d", overlay->type);
     }
}

static void
_overlay_place(Elm_Map_Smart_Data *sd)
{
   Eina_List *l, *ll;
   Elm_Map_Overlay *overlay;

   eina_list_free(sd->group_overlays);
   sd->group_overlays = NULL;

   EINA_LIST_FOREACH (sd->overlays, l, overlay)
     {
        // Reset groups
        if ((overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS) ||
            (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS))
          continue;
        overlay->grp->in = EINA_FALSE;
        overlay->grp->boss = EINA_FALSE;
        _overlay_group_hide(overlay->grp);

        // Update overlays' coord
        if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
          _overlay_default_coord_update(overlay->ovl);
        else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
          _overlay_bubble_coord_update(overlay->ovl);
     }

   // Classify into group boss or follwer
   EINA_LIST_FOREACH (sd->overlays, l, overlay)
     {
        Elm_Map_Overlay *boss;
        Overlay_Class *clas;

        if (overlay->type != ELM_MAP_OVERLAY_TYPE_CLASS) continue;
        if (overlay->hide || (overlay->zoom_min > sd->zoom)) continue;

        clas = overlay->ovl;
        if (clas->zoom_max < sd->zoom) continue;
        EINA_LIST_FOREACH (clas->members, ll, boss)
          {
             if (boss->type == ELM_MAP_OVERLAY_TYPE_CLASS) continue;
             if (boss->hide || (boss->zoom_min > sd->zoom)) continue;
             if (boss->grp->in) continue;
             _overlay_grouping(clas->members, boss);
          }
     }

   // Place group overlays and overlays
   EINA_LIST_FOREACH (sd->group_overlays, l, overlay)
     _overlay_show(overlay);
   EINA_LIST_FOREACH (sd->overlays, l, overlay)
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

static void
_overlays_show(void *data)
{
   double max_lon, min_lon, max_lat, min_lat;
   Delayed_Data *dd = data;
   int zoom, zoom_max;
   Evas_Coord vw, vh;

   EINA_SAFETY_ON_NULL_RETURN(data);

   _region_max_min_get(dd->overlays, &max_lon, &min_lon, &max_lat, &min_lat);
   dd->lon = (max_lon + min_lon) / 2;
   dd->lat = (max_lat + min_lat) / 2;

   zoom = dd->wsd->src_tile->zoom_min;
   _viewport_coord_get(dd->wsd, NULL, NULL, &vw, &vh);
   if (dd->wsd->src_tile->zoom_max < dd->wsd->zoom_max)
     zoom_max = dd->wsd->src_tile->zoom_max;
   else zoom_max = dd->wsd->zoom_max;
   while (zoom <= zoom_max)
     {
        Evas_Coord size, max_x, max_y, min_x, min_y;

        size = pow(2.0, zoom) * dd->wsd->tsize;
        _region_to_coord_convert
          (dd->wsd, min_lon, max_lat, size, &min_x, &max_y);
        _region_to_coord_convert
          (dd->wsd, max_lon, min_lat, size, &max_x, &min_y);
        if ((max_x - min_x) > vw || (max_y - min_y) > vh) break;
        zoom++;
     }
   zoom--;

   _zoom_do(dd->wsd, zoom);
   _region_show(dd);
   evas_object_smart_changed(dd->wsd->pan_obj);
}

static void
_elm_map_pan_smart_pos_set(Evas_Object *obj,
                           Evas_Coord x,
                           Evas_Coord y)
{
   ELM_MAP_PAN_DATA_GET(obj, psd);

   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;

   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;

   evas_object_smart_changed(obj);
}

static void
_elm_map_pan_smart_pos_get(const Evas_Object *obj,
                           Evas_Coord *x,
                           Evas_Coord *y)
{
   ELM_MAP_PAN_DATA_GET(obj, psd);

   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

static void
_elm_map_pan_smart_pos_max_get(const Evas_Object *obj,
                               Evas_Coord *x,
                               Evas_Coord *y)
{
   Evas_Coord ow, oh;

   ELM_MAP_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = psd->wsd->size.w - ow;
   oh = psd->wsd->size.h - oh;

   if (ow < 0) ow = 0;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_elm_map_pan_smart_pos_min_get(const Evas_Object *obj __UNUSED__,
                               Evas_Coord *x,
                               Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_elm_map_pan_smart_content_size_get(const Evas_Object *obj,
                                    Evas_Coord *w,
                                    Evas_Coord *h)
{
   ELM_MAP_PAN_DATA_GET(obj, psd);

   if (w) *w = psd->wsd->size.w;
   if (h) *h = psd->wsd->size.h;
}

static void
_elm_map_pan_smart_add(Evas_Object *obj)
{
   /* here just to allocate our extended data */
   EVAS_SMART_DATA_ALLOC(obj, Elm_Map_Pan_Smart_Data);

   ELM_PAN_CLASS(_elm_map_pan_parent_sc)->base.add(obj);
}

static void
_elm_map_pan_smart_resize(Evas_Object *obj,
                          Evas_Coord w __UNUSED__,
                          Evas_Coord h __UNUSED__)
{
   ELM_MAP_PAN_DATA_GET(obj, psd);

   _sizing_eval(psd->wsd);
   elm_map_zoom_mode_set(ELM_WIDGET_DATA(psd->wsd)->obj, psd->wsd->mode);
   evas_object_smart_changed(obj);
}

static void
_elm_map_pan_smart_calculate(Evas_Object *obj)
{
   Evas_Coord w, h;

   ELM_MAP_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w <= 0 || h <= 0) return;

   _grid_place(psd->wsd);
   _overlay_place(psd->wsd);
   _track_place(psd->wsd);
   _delayed_do(psd->wsd);
}

static void
_elm_map_pan_smart_move(Evas_Object *obj,
                        Evas_Coord x __UNUSED__,
                        Evas_Coord y __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   evas_object_smart_changed(obj);
}

static void
_elm_map_pan_smart_set_user(Elm_Map_Pan_Smart_Class *sc)
{
   ELM_PAN_CLASS(sc)->base.add = _elm_map_pan_smart_add;
   ELM_PAN_CLASS(sc)->base.move = _elm_map_pan_smart_move;
   ELM_PAN_CLASS(sc)->base.resize = _elm_map_pan_smart_resize;
   ELM_PAN_CLASS(sc)->base.calculate = _elm_map_pan_smart_calculate;

   ELM_PAN_CLASS(sc)->pos_set = _elm_map_pan_smart_pos_set;
   ELM_PAN_CLASS(sc)->pos_get = _elm_map_pan_smart_pos_get;
   ELM_PAN_CLASS(sc)->pos_max_get = _elm_map_pan_smart_pos_max_get;
   ELM_PAN_CLASS(sc)->pos_min_get = _elm_map_pan_smart_pos_min_get;
   ELM_PAN_CLASS(sc)->content_size_get =
     _elm_map_pan_smart_content_size_get;
}

static Eina_Bool
_xml_name_attrs_dump_cb(void *data,
                        const char *key,
                        const char *value)
{
   Name_Dump *dump = (Name_Dump *)data;

   if (!dump) return EINA_FALSE;

   if (!strncmp(key, NOMINATIM_ATTR_LON, sizeof(NOMINATIM_ATTR_LON)))
     dump->lon = _elm_atof(value);
   else if (!strncmp(key, NOMINATIM_ATTR_LAT, sizeof(NOMINATIM_ATTR_LAT)))
     dump->lat = _elm_atof(value);

   return EINA_TRUE;
}

static Eina_Bool
_xml_route_dump_cb(void *data,
                   Eina_Simple_XML_Type type,
                   const char *value,
                   unsigned offset __UNUSED__,
                   unsigned length)
{
   Route_Dump *dump = data;

   if (!dump) return EINA_FALSE;

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
         if (dump->id == ROUTE_XML_DISTANCE) dump->distance = _elm_atof(buf);
         else if (!(dump->description) && (dump->id == ROUTE_XML_DESCRIPTION))
           dump->description = strdup(buf);
         else if (dump->id == ROUTE_XML_COORDINATES)
           dump->coordinates = strdup(buf);

         free(buf);
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
                  unsigned offset __UNUSED__,
                  unsigned length)
{
   Name_Dump *dump = data;

   if (!dump) return EINA_FALSE;

   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
      case EINA_SIMPLE_XML_OPEN_EMPTY:
      {
         const char *attrs;
         attrs = eina_simple_xml_tag_attributes_find(value, length);
         if (attrs)
           {
              if (!strncmp(value, NOMINATIM_RESULT,
                           sizeof(NOMINATIM_RESULT) - 1))
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
         if (dump->id == NAME_XML_NAME) dump->address = strdup(buf);
         free(buf);
      }
      break;

      default:
        break;
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
             char *buf;

             fseek(f, 0, SEEK_SET);
             buf = malloc(sz);
             if (buf)
               {
                  if (fread(buf, 1, sz, f))
                    {
                       eina_simple_xml_parse
                         (buf, sz, EINA_TRUE, _xml_route_dump_cb, &dump);
                       free(buf);
                    }
               }
          }
        fclose(f);

        if (dump.distance) r->info.distance = dump.distance;
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
             char *buf;

             fseek(f, 0, SEEK_SET);
             buf = malloc(sz);
             if (buf)
               {
                  if (fread(buf, 1, sz, f))
                    {
                       eina_simple_xml_parse
                         (buf, sz, EINA_TRUE, _xml_name_dump_cb, &dump);
                       free(buf);
                    }
               }
          }
        fclose(f);

        if (dump.address)
          {
             INF("[%lf : %lf] ADDRESS : %s", n->lon, n->lat, dump.address);
             n->address = strdup(dump.address);
          }
        n->lon = dump.lon;
        n->lat = dump.lat;
     }
}

static void
_route_cb(void *data,
          const char *file,
          int status)
{
   Elm_Map_Route *route;
   Elm_Map_Smart_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(file);

   route = data;
   sd = route->wsd;

   route->job = NULL;
   if (status == 200)
     {
        _kml_parse(route);
        INF("Route request success from (%lf, %lf) to (%lf, %lf)",
            route->flon, route->flat, route->tlon, route->tlat);
        if (route->cb) route->cb(route->data, ELM_WIDGET_DATA(sd)->obj, route);
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(sd)->obj, SIG_ROUTE_LOADED, NULL);
     }
   else
     {
        ERR("Route request failed: %d", status);
        if (route->cb) route->cb(route->data, ELM_WIDGET_DATA(sd)->obj, NULL);
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(sd)->obj, SIG_ROUTE_LOADED_FAIL, NULL);
     }

   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                           "elm,state,busy,stop", "elm");
}

static void
_name_cb(void *data,
         const char *file,
         int status)
{
   Elm_Map_Name *name;
   Elm_Map_Smart_Data *sd;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(file);

   name = data;
   sd = name->wsd;

   name->job = NULL;
   if (status == 200)
     {
        _name_parse(name);
        INF("Name request success address:%s, lon:%lf, lat:%lf",
            name->address, name->lon, name->lat);
        if (name->cb) name->cb(name->data, ELM_WIDGET_DATA(sd)->obj, name);
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(sd)->obj, SIG_NAME_LOADED, NULL);
     }
   else
     {
        ERR("Name request failed: %d", status);
        if (name->cb) name->cb(name->data, ELM_WIDGET_DATA(sd)->obj, NULL);
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(sd)->obj, SIG_NAME_LOADED_FAIL, NULL);
     }
   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                           "elm,state,busy,stop", "elm");
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
   char fname[PATH_MAX], fname2[PATH_MAX];

   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_name, NULL);

   {
      const char *cachedir;

#ifdef ELM_EFREET
      snprintf(fname, sizeof(fname), "%s" CACHE_NAME_ROOT,
               efreet_cache_home_get());
      (void)cachedir;
#else
      cachedir = getenv("XDG_CACHE_HOME");
      snprintf(fname, sizeof(fname), "%s/%s" CACHE_NAME_ROOT, getenv("HOME"),
               cachedir ? : "/.config");
#endif
      if (!ecore_file_exists(fname)) ecore_file_mkpath(fname);
   }

   url = sd->src_name->url_cb
       (ELM_WIDGET_DATA(sd)->obj, method, address, lon, lat);
   if (!url)
     {
        ERR("Name URL is NULL");
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
        if (name->address) free(name->address);
        free(name->fname);
        free(name);
        return NULL;
     }
   INF("Name requested from %s to %s", url, name->fname);
   free(url);

   sd->names = eina_list_append(sd->names, name);
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_NAME_LOAD, name);
   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                           "elm,state,busy,start", "elm");
   return name;
}

static Evas_Event_Flags
_pinch_zoom_start_cb(void *data,
                     void *event_info __UNUSED__)
{
   Elm_Map_Smart_Data *sd = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EVAS_EVENT_FLAG_NONE);

   sd->pinch_zoom = sd->zoom_detail;

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_pinch_zoom_cb(void *data,
               void *event_info)
{
   Elm_Map_Smart_Data *sd = data;

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
   Elm_Map_Smart_Data *sd = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EVAS_EVENT_FLAG_NONE);

   if (!sd->paused)
     {
        int x, y, w, h;
        Elm_Gesture_Rotate_Info *ei = event_info;

        evas_object_geometry_get(ELM_WIDGET_DATA(sd)->obj, &x, &y, &w, &h);

        sd->rotate.d = sd->rotate.a + ei->angle - ei->base_angle;
        sd->rotate.cx = x + ((double)w * 0.5);
        sd->rotate.cy = y + ((double)h * 0.5);

        evas_object_smart_changed(sd->pan_obj);
     }

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_pinch_rotate_end_cb(void *data,
                     void *event_info __UNUSED__)
{
   Elm_Map_Smart_Data *sd = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd, EVAS_EVENT_FLAG_NONE);

   sd->rotate.a = sd->rotate.d;

   return EVAS_EVENT_FLAG_NONE;
}

static void
_source_tile_set(Elm_Map_Smart_Data *sd,
                 const char *source_name)
{
   Source_Tile *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (sd->src_tile && !strcmp(sd->src_tile->name, source_name))
     return;

   EINA_LIST_FOREACH (sd->src_tiles, l, s)
     {
        if (!strcmp(s->name, source_name))
          {
             sd->src_tile = s;
             break;
          }
     }
   if (!sd->src_tile)
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

   _grid_all_clear(sd);
   _grid_all_create(sd);
   _zoom_do(sd, sd->zoom);
}

static void
_source_route_set(Elm_Map_Smart_Data *sd,
                  const char *source_name)
{
   Source_Route *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (sd->src_route && !strcmp(sd->src_route->name, source_name))
     return;

   EINA_LIST_FOREACH (sd->src_routes, l, s)
     {
        if (!strcmp(s->name, source_name))
          {
             sd->src_route = s;
             break;
          }
     }
   if (!sd->src_route)
     {
        ERR("source name (%s) is not found", source_name);
        return;
     }
}

static void
_source_name_set(Elm_Map_Smart_Data *sd,
                 const char *source_name)
{
   Source_Name *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (sd->src_name && !strcmp(sd->src_name->name, source_name))
     return;

   EINA_LIST_FOREACH (sd->src_names, l, s)
     {
        if (!strcmp(s->name, source_name))
          {
             sd->src_name = s;
             break;
          }
     }
   if (!sd->src_name)
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
   Elm_Map_Smart_Data *sd = data;

   Elm_Map_Module_Source_Name_Func name_cb;
   Elm_Map_Module_Tile_Url_Func tile_url_cb;
   Elm_Map_Module_Tile_Scale_Func scale_cb;
   Elm_Map_Module_Tile_Zoom_Min_Func zoom_min;
   Elm_Map_Module_Tile_Zoom_Max_Func zoom_max;
   Elm_Map_Module_Tile_Geo_to_Coord_Func geo_to_coord;
   Elm_Map_Module_Tile_Coord_to_Geo_Func coord_to_geo;
   Elm_Map_Module_Route_Url_Func route_url_cb;
   Elm_Map_Module_Name_Url_Func name_url_cb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);

   file = eina_module_file_get(m);
   if (!eina_module_load(m))
     {
        ERR("Could not load module \"%s\": %s", file,
            eina_error_msg_get(eina_error_get()));
        return EINA_FALSE;
     }
   name_cb = eina_module_symbol_get(m, "map_module_source_name_get");
   if ((!name_cb))
     {
        WRN("Could not find map module name from module \"%s\": %s",
            file, eina_error_msg_get(eina_error_get()));
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
        sd->src_tiles = eina_list_append(sd->src_tiles, s);
     }

   // Find ROUTE module
   route_url_cb = eina_module_symbol_get(m, "map_module_route_url_get");
   if (route_url_cb)
     {
        INF("Map ROUTE module is loaded \"%s\"",  file);
        Source_Route *s;
        s = ELM_NEW(Source_Tile);
        s->name = name_cb();
        s->url_cb = route_url_cb;
        sd->src_routes = eina_list_append(sd->src_routes, s);
     }

   // Find NAME module
   name_url_cb = eina_module_symbol_get(m, "map_module_name_url_get");
   if (name_url_cb)
     {
        INF("Map NAME module is loaded \"%s\"",  file);
        Source_Name *s;
        s = ELM_NEW(Source_Tile);
        s->name = name_cb();
        s->url_cb = name_url_cb;
        sd->src_names = eina_list_append(sd->src_names, s);
     }
   return EINA_TRUE;
}

static void
_source_all_unload(Elm_Map_Smart_Data *sd)
{
   int idx = 0;
   Source_Tile *s;

   for (idx = 0; sd->src_tile_names[idx]; idx++)
     eina_stringshare_del(sd->src_tile_names[idx]);
   for (idx = 0; sd->src_route_names[idx]; idx++)
     eina_stringshare_del(sd->src_route_names[idx]);
   for (idx = 0; sd->src_name_names[idx]; idx++)
     eina_stringshare_del(sd->src_name_names[idx]);

   EINA_LIST_FREE(sd->src_tiles, s) free(s);
   EINA_LIST_FREE(sd->src_routes, s) free(s);
   EINA_LIST_FREE(sd->src_names, s) free(s);

   eina_module_list_free(sd->src_mods);
}

static void
_source_all_load(Elm_Map_Smart_Data *sd)
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
        src_tile->name = src_tiles[idx].name;
        src_tile->zoom_min = src_tiles[idx].zoom_min;
        src_tile->zoom_max = src_tiles[idx].zoom_max;
        src_tile->url_cb = src_tiles[idx].url_cb;
        src_tile->geo_to_coord = src_tiles[idx].geo_to_coord;
        src_tile->coord_to_geo = src_tiles[idx].coord_to_geo;
        src_tile->scale_cb = src_tiles[idx].scale_cb;
        sd->src_tiles = eina_list_append(sd->src_tiles, src_tile);
     }
   // Load hard coded ROUTE source
   for (idx = 0; idx < (sizeof(src_routes) / sizeof(Source_Route)); idx++)
     {
        src_route = ELM_NEW(Source_Route);
        src_route->name = src_routes[idx].name;
        src_route->url_cb = src_routes[idx].url_cb;
        sd->src_routes = eina_list_append(sd->src_routes, src_route);
     }
   // Load from hard coded NAME source
   for (idx = 0; idx < (sizeof(src_names) / sizeof(Source_Name)); idx++)
     {
        src_name = ELM_NEW(Source_Name);
        src_name->name = src_names[idx].name;
        src_name->url_cb = src_names[idx].url_cb;
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
   EINA_LIST_FOREACH (sd->src_tiles, l, src_tile)
     {
        eina_stringshare_replace(&sd->src_tile_names[idx], src_tile->name);
        INF("source : %s", sd->src_tile_names[idx]);
        idx++;
     }
   idx = 0;
   sd->src_route_names = calloc((eina_list_count(sd->src_routes) + 1),
                                sizeof(const char *));
   EINA_LIST_FOREACH (sd->src_routes, l, src_route)
     {
        eina_stringshare_replace(&sd->src_route_names[idx], src_route->name);
        INF("source : %s", sd->src_route_names[idx]);
        idx++;
     }
   idx = 0;
   sd->src_name_names = calloc((eina_list_count(sd->src_names) + 1),
                               sizeof(const char *));
   EINA_LIST_FOREACH (sd->src_names, l, src_name)
     {
        eina_stringshare_replace(&sd->src_name_names[idx], src_name->name);
        INF("source : %s", sd->src_name_names[idx]);
        idx++;
     }
}

static void
_zoom_mode_set(void *data)
{
   Delayed_Data *dd = data;

   EINA_SAFETY_ON_NULL_RETURN(data);

   dd->wsd->mode = dd->mode;
   if (dd->mode != ELM_MAP_ZOOM_MODE_MANUAL)
     {
        double zoom;
        double diff;
        Evas_Coord w, h;
        Evas_Coord vw, vh;

        w = dd->wsd->size.w;
        h = dd->wsd->size.h;
        zoom = dd->wsd->zoom_detail;
        _viewport_coord_get(dd->wsd, NULL, NULL, &vw, &vh);

        if (dd->mode == ELM_MAP_ZOOM_MODE_AUTO_FIT)
          {
             if ((w < vw) && (h < vh))
               {
                  diff = 0.01;
                  while ((w < vw) && (h < vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * dd->wsd->tsize;
                       h = pow(2.0, zoom) * dd->wsd->tsize;
                    }
               }
             else
               {
                  diff = -0.01;
                  while ((w > vw) || (h > vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * dd->wsd->tsize;
                       h = pow(2.0, zoom) * dd->wsd->tsize;
                    }
               }
          }
        else if (dd->mode == ELM_MAP_ZOOM_MODE_AUTO_FILL)
          {
             if ((w < vw) || (h < vh))
               {
                  diff = 0.01;
                  while ((w < vw) || (h < vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * dd->wsd->tsize;
                       h = pow(2.0, zoom) * dd->wsd->tsize;
                    }
               }
             else
               {
                  diff = -0.01;
                  while ((w > vw) && (h > vh))
                    {
                       zoom += diff;
                       w = pow(2.0, zoom) * dd->wsd->tsize;
                       h = pow(2.0, zoom) * dd->wsd->tsize;
                    }
               }
          }
        _zoom_do(dd->wsd, zoom);
     }
}

static void
_zoom_set(void *data)
{
   Delayed_Data *dd = data;

   EINA_SAFETY_ON_NULL_RETURN(data);

   if (dd->wsd->paused) _zoom_do(dd->wsd, dd->zoom);
   else _zoom_with_animation(dd->wsd, dd->zoom, 10);

   evas_object_smart_changed(dd->wsd->pan_obj);
}

static void
_region_bring_in(void *data)
{
   Delayed_Data *dd = data;
   int x, y, w, h;

   EINA_SAFETY_ON_NULL_RETURN(data);

   _region_to_coord_convert
     (dd->wsd, dd->lon, dd->lat, dd->wsd->size.w, &x, &y);
   _viewport_coord_get(dd->wsd, NULL, NULL, &w, &h);
   x = x - (w / 2);
   y = y - (h / 2);
   dd->wsd->s_iface->region_bring_in
     (ELM_WIDGET_DATA(dd->wsd)->obj, x, y, w, h);
   evas_object_smart_changed(dd->wsd->pan_obj);
}

static Eina_Bool
_elm_map_smart_on_focus(Evas_Object *obj)
{
   ELM_MAP_DATA_GET(obj, sd);

   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,focus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_map_smart_theme(Evas_Object *obj)
{
   ELM_MAP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_map_parent_sc)->theme(obj))
     return EINA_FALSE;

   _sizing_eval(sd);

   return EINA_TRUE;
}

static Eina_Bool
_elm_map_smart_event(Evas_Object *obj,
                     Evas_Object *src __UNUSED__,
                     Evas_Callback_Type type,
                     void *event_info)
{
   Evas_Coord vh;
   Evas_Coord x, y;
   Evas_Event_Key_Down *ev = event_info;
   Evas_Coord step_x, step_y, page_x, page_y;

   ELM_MAP_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->step_size_get(obj, &step_x, &step_y);
   sd->s_iface->page_size_get(obj, &page_x, &page_y);
   sd->s_iface->content_viewport_size_get(obj, NULL, &vh);

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            ((!strcmp(ev->keyname, "KP_Prior")) && (!ev->string)))
     {
        if (page_y < 0)
          y -= -(page_y * vh) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) ||
            ((!strcmp(ev->keyname, "KP_Next")) && (!ev->string)))
     {
        if (page_y < 0)
          y += -(page_y * vh) / 100;
        else
          y += page_y;
     }
   else if (!strcmp(ev->keyname, "KP_Add"))
     {
        _zoom_with_animation(sd, sd->zoom + 1, 10);
        return EINA_TRUE;
     }
   else if (!strcmp(ev->keyname, "KP_Subtract"))
     {
        _zoom_with_animation(sd, sd->zoom - 1, 10);
        return EINA_TRUE;
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   sd->s_iface->content_pos_set(obj, x, y);

   return EINA_TRUE;
}

#endif

static void
_elm_map_smart_add(Evas_Object *obj)
{
   Evas_Coord minw, minh;
   Elm_Map_Pan_Smart_Data *pan_data;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Map_Smart_Data);

   ELM_WIDGET_DATA(priv)->resize_obj =
     edje_object_add(evas_object_evas_get(obj));

   ELM_WIDGET_CLASS(_elm_map_parent_sc)->base.add(obj);

   elm_widget_theme_object_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, "map", "base",
     elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, priv);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, priv);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, priv);

   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, priv->hit_rect);

   priv->s_iface->wheel_disabled_set(obj, EINA_TRUE);
   priv->s_iface->bounce_allow_set(obj, _elm_config->thumbscroll_bounce_enable,
                                   _elm_config->thumbscroll_bounce_enable);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints_cb, priv);

   priv->s_iface->animate_start_cb_set(obj, _scroll_animate_start_cb);
   priv->s_iface->animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   priv->s_iface->scroll_cb_set(obj, _scroll_cb);

   priv->pan_obj = evas_object_smart_add
       (evas_object_evas_get(obj), _elm_map_pan_smart_class_new());
   pan_data = evas_object_smart_data_get(priv->pan_obj);
   pan_data->wsd = priv;

   priv->s_iface->extern_pan_set(obj, priv->pan_obj);

   edje_object_size_min_calc(ELM_WIDGET_DATA(priv)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   priv->g_layer = elm_gesture_layer_add(obj);
   if (!priv->g_layer)
     ERR("elm_gesture_layer_add() failed");

   elm_gesture_layer_attach(priv->g_layer, obj);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START,
     _pinch_zoom_start_cb, priv);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE,
     _pinch_zoom_cb, priv);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_MOVE,
     _pinch_rotate_cb, priv);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_END,
     _pinch_rotate_end_cb, priv);
   elm_gesture_layer_cb_set
     (priv->g_layer, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_ABORT,
     _pinch_rotate_end_cb, priv);

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

   priv->mode = ELM_MAP_ZOOM_MODE_MANUAL;

   if (!ecore_file_download_protocol_available("http://"))
     ERR("Ecore must be built with curl support for the map widget!");
}

static void
_elm_map_smart_del(Evas_Object *obj)
{
   Elm_Map_Route *r;
   Elm_Map_Name *na;
   Delayed_Data *dd;
   Eina_List *l, *ll;
   Evas_Object *track;
   Elm_Map_Overlay *overlay;

   ELM_MAP_DATA_GET(obj, sd);

   EINA_LIST_FOREACH_SAFE (sd->routes, l, ll, r)
     elm_map_route_del(r);

   eina_list_free(sd->routes);

   EINA_LIST_FOREACH_SAFE (sd->names, l, ll, na)
     elm_map_name_del(na);

   eina_list_free(sd->names);

   EINA_LIST_FOREACH_SAFE (sd->overlays, l, ll, overlay)
     elm_map_overlay_del(overlay);

   eina_list_free(sd->overlays);
   eina_list_free(sd->group_overlays);
   eina_list_free(sd->all_overlays);

   EINA_LIST_FREE (sd->track, track)
     evas_object_del(track);

   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   if (sd->long_timer) ecore_timer_del(sd->long_timer);

   if (sd->delayed_jobs)
     EINA_LIST_FREE (sd->delayed_jobs, dd)
       free(dd);

   if (sd->user_agent) eina_stringshare_del(sd->user_agent);
   if (sd->ua) eina_hash_free(sd->ua);

   if (sd->zoom_timer) ecore_timer_del(sd->zoom_timer);
   if (sd->zoom_animator) ecore_animator_del(sd->zoom_animator);

   _grid_all_clear(sd);
   // Removal of download list should be after grid clear.
   if (sd->download_idler) ecore_idler_del(sd->download_idler);
   eina_list_free(sd->download_list);

   _source_all_unload(sd);

   {
      char buf[4096];
      const char *cachedir;

#ifdef ELM_EFREET
      snprintf(buf, sizeof(buf), "%s" CACHE_ROOT, efreet_cache_home_get());
      (void)cachedir;
#else
      cachedir = getenv("XDG_CACHE_HOME");
      snprintf(buf, sizeof(buf), "%s/%s" CACHE_ROOT, getenv("HOME"),
               cachedir ? : "/.config");
#endif
      if (!ecore_file_recursive_rm(buf))
        ERR("Deletion of %s failed", buf);
   }

   if (sd->loaded_timer)
     {
        ecore_timer_del(sd->loaded_timer);
        sd->loaded_timer = NULL;
     }
   if (sd->map) evas_map_free(sd->map);

   ELM_WIDGET_CLASS(_elm_map_parent_sc)->base.del(obj);
}

static void
_elm_map_smart_move(Evas_Object *obj,
                    Evas_Coord x,
                    Evas_Coord y)
{
   ELM_MAP_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_map_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_map_smart_resize(Evas_Object *obj,
                      Evas_Coord w,
                      Evas_Coord h)
{
   ELM_MAP_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_map_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_map_smart_member_add(Evas_Object *obj,
                          Evas_Object *member)
{
   ELM_MAP_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_map_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_elm_map_smart_set_user(Elm_Map_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_map_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_map_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_map_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_map_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_map_smart_member_add;

   ELM_WIDGET_CLASS(sc)->on_focus = _elm_map_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_map_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_map_smart_event;
}

EAPI const Elm_Map_Smart_Class *
elm_map_smart_class_get(void)
{
   static Elm_Map_Smart_Class _sc =
     ELM_MAP_SMART_CLASS_INIT_NAME_VERSION(ELM_MAP_SMART_NAME);
   static const Elm_Map_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_map_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_map_add(Evas_Object *parent)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_map_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
#else
   (void)parent;
   return NULL;
#endif
}

EAPI void
elm_map_zoom_set(Evas_Object *obj,
                 int zoom)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Delayed_Data *data;

   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   if (sd->mode != ELM_MAP_ZOOM_MODE_MANUAL) return;
   if (zoom < 0) zoom = 0;
   if (sd->zoom == zoom) return;

   data = ELM_NEW(Delayed_Data);
   data->func = _zoom_set;
   data->wsd = sd;
   data->zoom = zoom;
   data->wsd->delayed_jobs = eina_list_append(data->wsd->delayed_jobs, data);
   evas_object_smart_changed(data->wsd->pan_obj);
#else
   (void)obj;
   (void)zoom;
#endif
}

EAPI int
elm_map_zoom_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) 0;
   ELM_MAP_DATA_GET(obj, sd);

   return sd->zoom;
#else
   (void)obj;
   return 0;
#endif
}

EAPI void
elm_map_zoom_mode_set(Evas_Object *obj,
                      Elm_Map_Zoom_Mode mode)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Delayed_Data *data;

   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   if ((mode == ELM_MAP_ZOOM_MODE_MANUAL) && (sd->mode == !!mode)) return;

   data = ELM_NEW(Delayed_Data);
   data->mode = mode;
   data->func = _zoom_mode_set;
   data->wsd = sd;
   data->wsd->delayed_jobs = eina_list_append(data->wsd->delayed_jobs, data);
   evas_object_smart_changed(data->wsd->pan_obj);
#else
   (void)obj;
   (void)mode;
#endif
}

EAPI Elm_Map_Zoom_Mode
elm_map_zoom_mode_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) ELM_MAP_ZOOM_MODE_MANUAL;
   ELM_MAP_DATA_GET(obj, sd);

   return sd->mode;
#else
   (void)obj;
   return ELM_MAP_ZOOM_MODE_MANUAL;
#endif
}

EAPI void
elm_map_zoom_max_set(Evas_Object *obj,
                     int zoom)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   sd->zoom_max = zoom;
#else
   (void)obj;
   (void)zoom;
#endif
}

EAPI int
elm_map_zoom_max_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) - 1;
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_tile, -1);

   return sd->zoom_max;
#else
   (void)obj;
   return -1;
#endif
}

EAPI void
elm_map_zoom_min_set(Evas_Object *obj,
                     int zoom)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(sd->src_tile);

   sd->zoom_min = zoom;
#else
   (void)obj;
   (void)zoom;
#endif
}

EAPI int
elm_map_zoom_min_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) - 1;
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_tile, -1);

   return sd->zoom_min;
#else
   (void)obj;
   return -1;
#endif
}

EAPI void
elm_map_region_bring_in(Evas_Object *obj,
                        double lon,
                        double lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Delayed_Data *data;

   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   data = ELM_NEW(Delayed_Data);
   data->func = _region_bring_in;
   data->wsd = sd;
   data->lon = lon;
   data->lat = lat;
   data->wsd->delayed_jobs = eina_list_append(data->wsd->delayed_jobs, data);
   evas_object_smart_changed(data->wsd->pan_obj);
#else
   (void)obj;
   (void)lon;
   (void)lat;
#endif
}

EAPI void
elm_map_region_show(Evas_Object *obj,
                    double lon,
                    double lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Delayed_Data *data;

   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   data = ELM_NEW(Delayed_Data);
   data->func = _region_show;
   data->wsd = sd;
   data->lon = lon;
   data->lat = lat;
   data->wsd->delayed_jobs = eina_list_append(data->wsd->delayed_jobs, data);
   evas_object_smart_changed(data->wsd->pan_obj);
#else
   (void)obj;
   (void)lon;
   (void)lat;
#endif
}

EAPI void
elm_map_region_get(const Evas_Object *obj,
                   double *lon,
                   double *lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   double tlon, tlat;
   Evas_Coord vx, vy, vw, vh;

   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   _viewport_coord_get(sd, &vx, &vy, &vw, &vh);
   _coord_to_region_convert
     (sd, vx + vw / 2, vy + vh / 2, sd->size.w, &tlon, &tlat);
   if (lon) *lon = tlon;
   if (lat) *lat = tlat;
#else
   (void)obj;
   (void)lon;
   (void)lat;
#endif
}

EAPI void
elm_map_paused_set(Evas_Object *obj,
                   Eina_Bool paused)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   if (sd->paused == !!paused) return;
   sd->paused = !!paused;
   if (sd->paused)
     {
        if (sd->zoom_animator)
          {
             if (sd->zoom_animator) ecore_animator_del(sd->zoom_animator);
             sd->zoom_animator = NULL;
             _zoom_do(sd, sd->zoom);
          }
        edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                                "elm,state,busy,stop", "elm");
     }
   else
     {
        if (sd->download_num >= 1)
          edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                                  "elm,state,busy,start", "elm");
     }
#else
   (void)obj;
   (void)paused;
#endif
}

EAPI Eina_Bool
elm_map_paused_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) EINA_FALSE;
   ELM_MAP_DATA_GET(obj, sd);

   return sd->paused;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

EAPI void
elm_map_rotate_set(Evas_Object *obj,
                   double degree,
                   Evas_Coord cx,
                   Evas_Coord cy)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   sd->rotate.d = degree;
   sd->rotate.cx = cx;
   sd->rotate.cy = cy;

   evas_object_smart_changed(sd->pan_obj);
#else
   (void)obj;
   (void)degree;
   (void)cx;
   (void)cy;
#endif
}

EAPI void
elm_map_rotate_get(const Evas_Object *obj,
                   double *degree,
                   Evas_Coord *cx,
                   Evas_Coord *cy)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   if (degree) *degree = sd->rotate.d;
   if (cx) *cx = sd->rotate.cx;
   if (cy) *cy = sd->rotate.cy;
#else
   (void)obj;
   (void)degree;
   (void)cx;
   (void)cy;
#endif
}

EAPI void
elm_map_wheel_disabled_set(Evas_Object *obj,
                           Eina_Bool disabled)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   if ((!sd->wheel_disabled) && (disabled))
     evas_object_event_callback_del_full
       (obj, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, sd);
   else if ((sd->wheel_disabled) && (!disabled))
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, sd);
   sd->wheel_disabled = !!disabled;
#else
   (void)obj;
   (void)disabled;
#endif
}

EAPI Eina_Bool
elm_map_wheel_disabled_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) EINA_FALSE;
   ELM_MAP_DATA_GET(obj, sd);

   return sd->wheel_disabled;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

EAPI void
elm_map_tile_load_status_get(const Evas_Object *obj,
                             int *try_num,
                             int *finish_num)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   if (try_num) *try_num = sd->try_num;
   if (finish_num) *finish_num = sd->finish_num;
#else
   (void)obj;
   (void)try_num;
   (void)finish_num;
#endif
}

EAPI void
elm_map_canvas_to_region_convert(const Evas_Object *obj,
                                 Evas_Coord x,
                                 Evas_Coord y,
                                 double *lon,
                                 double *lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(lon);
   EINA_SAFETY_ON_NULL_RETURN(lat);

   _canvas_to_coord(sd, x, y, &x, &y);
   _coord_to_region_convert(sd, x, y, sd->size.w, lon, lat);
#else
   (void)obj;
   (void)x;
   (void)y;
   (void)lon;
   (void)lat;
#endif
}

EAPI void
elm_map_region_to_canvas_convert(const Evas_Object *obj,
                                 double lon,
                                 double lat,
                                 Evas_Coord *x,
                                 Evas_Coord *y)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(x);
   EINA_SAFETY_ON_NULL_RETURN(y);

   _region_to_coord_convert(sd, lon, lat, sd->size.w, x, y);
   _coord_to_canvas(sd, *x, *y, x, y);
#else
   (void)obj;
   (void)lon;
   (void)lat;
   (void)x;
   (void)y;
#endif
}

EAPI void
elm_map_user_agent_set(Evas_Object *obj,
                       const char *user_agent)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(user_agent);

   eina_stringshare_replace(&sd->user_agent, user_agent);

   if (!sd->ua) sd->ua = eina_hash_string_small_new(NULL);
   eina_hash_set(sd->ua, "User-Agent", sd->user_agent);
#else
   (void)obj;
   (void)user_agent;
#endif
}

EAPI const char *
elm_map_user_agent_get(const Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   return sd->user_agent;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_map_source_set(Evas_Object *obj,
                   Elm_Map_Source_Type type,
                   const char *source_name)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(source_name);

   if (type == ELM_MAP_SOURCE_TYPE_TILE) _source_tile_set(sd, source_name);
   else if (type == ELM_MAP_SOURCE_TYPE_ROUTE)
     _source_route_set(sd, source_name);
   else if (type == ELM_MAP_SOURCE_TYPE_NAME)
     _source_name_set(sd, source_name);
   else ERR("Not supported map source type: %d", type);

#else
   (void)obj;
   (void)source_name;
#endif
}

EAPI const char *
elm_map_source_get(const Evas_Object *obj,
                   Elm_Map_Source_Type type)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_tile, NULL);

   if (type == ELM_MAP_SOURCE_TYPE_TILE) return sd->src_tile->name;
   else if (type == ELM_MAP_SOURCE_TYPE_ROUTE)
     return sd->src_route->name;
   else if (type == ELM_MAP_SOURCE_TYPE_NAME)
     return sd->src_name->name;
   else ERR("Not supported map source type: %d", type);

   return NULL;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI const char **
elm_map_sources_get(const Evas_Object *obj,
                    Elm_Map_Source_Type type)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   if (type == ELM_MAP_SOURCE_TYPE_TILE) return sd->src_tile_names;
   else if (type == ELM_MAP_SOURCE_TYPE_ROUTE)
     return sd->src_route_names;
   else if (type == ELM_MAP_SOURCE_TYPE_NAME)
     return sd->src_name_names;
   else ERR("Not supported map source type: %d", type);

   return NULL;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI Elm_Map_Route *
elm_map_route_add(Evas_Object *obj,
                  Elm_Map_Route_Type type,
                  Elm_Map_Route_Method method,
                  double flon,
                  double flat,
                  double tlon,
                  double tlat,
                  Elm_Map_Route_Cb route_cb,
                  void *data)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   char *url;
   char *type_name;
   Elm_Map_Route *route;
   char fname[PATH_MAX], fname2[PATH_MAX];

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->src_route, NULL);

   {
      const char *cachedir;

#ifdef ELM_EFREET
      snprintf(fname, sizeof(fname), "%s" CACHE_ROUTE_ROOT,
               efreet_cache_home_get());
      (void)cachedir;
#else
      cachedir = getenv("XDG_CACHE_HOME");
      snprintf(fname, sizeof(fname), "%s/%s" CACHE_ROUTE_ROOT, getenv("HOME"),
               cachedir ? : "/.config");
#endif
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
        if (type_name) free(type_name);
        return NULL;
     }
   if (type_name) free(type_name);

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
        return NULL;
     }
   INF("Route requested from %s to %s", url, route->fname);
   free(url);

   sd->routes = eina_list_append(sd->routes, route);
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(sd)->obj, SIG_ROUTE_LOAD, route);
   edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
                           "elm,state,busy,start", "elm");
   return route;
#else
   (void)obj;
   (void)type;
   (void)method;
   (void)flon;
   (void)flat;
   (void)tlon;
   (void)tlat;
   (void)route_cb;
   (void)data;
   return NULL;
#endif
}

EAPI void
elm_map_route_del(Elm_Map_Route *route)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Path_Waypoint *w;
   Path_Node *n;

   EINA_SAFETY_ON_NULL_RETURN(route);
   EINA_SAFETY_ON_NULL_RETURN(route->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(route->wsd)->obj);

   if (route->job) ecore_file_download_abort(route->job);

   EINA_LIST_FREE (route->waypoint, w)
     {
        if (w->point) eina_stringshare_del(w->point);
        free(w);
     }

   EINA_LIST_FREE (route->nodes, n)
     {
        if (n->pos.address) eina_stringshare_del(n->pos.address);
        free(n);
     }

   if (route->fname)
     {
        ecore_file_remove(route->fname);
        free(route->fname);
     }

   route->wsd->routes = eina_list_remove(route->wsd->routes, route);
   free(route);
#else
   (void)route;
#endif
}

EAPI double
elm_map_route_distance_get(const Elm_Map_Route *route)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(route, 0.0);
   return route->info.distance;
#else
   (void)route;
   return 0.0;
#endif
}

EAPI const char *
elm_map_route_node_get(const Elm_Map_Route *route)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);
   return route->info.nodes;
#else
   (void)route;
   return NULL;
#endif
}

EAPI const char *
elm_map_route_waypoint_get(const Elm_Map_Route *route)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);
   return route->info.waypoints;
#else
   (void)route;
   return NULL;
#endif
}

EAPI Elm_Map_Name *
elm_map_name_add(const Evas_Object *obj,
                 const char *address,
                 double lon,
                 double lat,
                 Elm_Map_Name_Cb name_cb,
                 void *data)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj) NULL;

   if (address)
     return _name_request(obj, ELM_MAP_NAME_METHOD_SEARCH, address, 0, 0,
                          name_cb, data);
   else
     return _name_request(obj, ELM_MAP_NAME_METHOD_REVERSE, NULL, lon, lat,
                          name_cb, data);
#else
   (void)obj;
   (void)address;
   (void)lon;
   (void)lat;
   (void)name_cb;
   (void)data;
   return NULL;
#endif
}

EAPI void
elm_map_name_del(Elm_Map_Name *name)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(name);
   EINA_SAFETY_ON_NULL_RETURN(name->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(name->wsd)->obj);

   if (name->job) ecore_file_download_abort(name->job);
   if (name->address) free(name->address);
   if (name->fname)
     {
        ecore_file_remove(name->fname);
        free(name->fname);
     }

   name->wsd->names = eina_list_remove(name->wsd->names, name);
   free(name);
#else
   (void)name;
#endif
}

EAPI const char *
elm_map_name_address_get(const Elm_Map_Name *name)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name->wsd, NULL);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(name->wsd)->obj) NULL;

   return name->address;
#else
   (void)name;
   return NULL;
#endif
}

EAPI void
elm_map_name_region_get(const Elm_Map_Name *name,
                        double *lon,
                        double *lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(name);
   EINA_SAFETY_ON_NULL_RETURN(name->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(name->wsd)->obj);

   if (lon) *lon = name->lon;
   if (lat) *lat = name->lat;
#else
   (void)name;
   (void)lon;
   (void)lat;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_add(Evas_Object *obj,
                    double lon,
                    double lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_DEFAULT;
   overlay->c.r = 0x87;
   overlay->c.g = 0xce;
   overlay->c.b = 0xeb;
   overlay->c.a = 255;
   overlay->ovl = _overlay_default_new(overlay, lon, lat, overlay->c, 1);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);

   return overlay;
#else
   (void)obj;
   (void)lon;
   (void)lat;
   return NULL;
#endif
}

EAPI Eina_List *
elm_map_overlays_get(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Eina_List *l;
   Elm_Map_Overlay *ovl;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   eina_list_free(sd->all_overlays);
   sd->all_overlays = NULL;

   EINA_LIST_FOREACH (sd->overlays, l, ovl)
     sd->all_overlays = eina_list_append(sd->all_overlays, ovl);
   EINA_LIST_FOREACH (sd->group_overlays, l, ovl)
     sd->all_overlays = eina_list_append(sd->all_overlays, ovl);

   return sd->all_overlays;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_del(Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(overlay->wsd)->obj, SIG_OVERLAY_DEL, overlay);
   if (overlay->del_cb)
     overlay->del_cb
       (overlay->del_cb_data, ELM_WIDGET_DATA(overlay->wsd)->obj, overlay);

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
#else
   (void)overlay;
#endif
}

EAPI Elm_Map_Overlay_Type
elm_map_overlay_type_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, ELM_MAP_OVERLAY_TYPE_NONE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, ELM_MAP_OVERLAY_TYPE_NONE);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) ELM_MAP_OVERLAY_TYPE_NONE;

   return overlay->type;
#else
   (void)overlay;
   return ELM_MAP_OVERLAY_TYPE_NONE;
#endif
}

EAPI void
elm_map_overlay_data_set(Elm_Map_Overlay *overlay,
                         void *data)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   overlay->data = data;
#else
   (void)overlay;
   (void)data;
#endif
}

EAPI void *
elm_map_overlay_data_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, NULL);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) NULL;

   return overlay->data;
#else
   (void)overlay;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_hide_set(Elm_Map_Overlay *overlay,
                         Eina_Bool hide)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   if (overlay->hide == !!hide) return;
   overlay->hide = hide;

   evas_object_smart_changed(overlay->wsd->pan_obj);
#else
   (void)overlay;
   (void)hide;
#endif
}

EAPI Eina_Bool
elm_map_overlay_hide_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, EINA_FALSE);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) EINA_FALSE;

   return overlay->hide;
#else
   (void)overlay;
   return EINA_FALSE;
#endif
}

EAPI void
elm_map_overlay_displayed_zoom_min_set(Elm_Map_Overlay *overlay,
                                       int zoom)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   overlay->zoom_min = zoom;
   evas_object_smart_changed(overlay->wsd->pan_obj);
#else
   (void)overlay;
   (void)zoom;
#endif
}

EAPI int
elm_map_overlay_displayed_zoom_min_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, 0);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) 0;

   return overlay->zoom_min;
#else
   (void)overlay;
   return 0;
#endif
}

EAPI void
elm_map_overlay_paused_set(Elm_Map_Overlay *overlay,
                           Eina_Bool paused)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   if (overlay->paused == !!paused) return;
   overlay->paused = paused;

   evas_object_smart_changed(overlay->wsd->pan_obj);
#else
   (void)overlay;
   (void)paused;
#endif
}

EAPI Eina_Bool
elm_map_overlay_paused_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, EINA_FALSE);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) EINA_FALSE;

   return overlay->paused;
#else
   (void)overlay;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_map_overlay_visible_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, EINA_FALSE);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) EINA_FALSE;

   return overlay->visible;
#else
   (void)overlay;
   return EINA_FALSE;
#endif
}

EAPI void
elm_map_overlay_show(Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        Overlay_Default *ovl = overlay->ovl;

        elm_map_region_show
          (ELM_WIDGET_DATA(overlay->wsd)->obj, ovl->lon, ovl->lat);
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_BUBBLE)
     {
        Overlay_Bubble *ovl = overlay->ovl;

        elm_map_region_show
          (ELM_WIDGET_DATA(overlay->wsd)->obj, ovl->lon, ovl->lat);
     }
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     {
        Overlay_Class *ovl = overlay->ovl;
        double lon, lat, max_lo, min_lo, max_la, min_la;

        _region_max_min_get(ovl->members, &max_lo, &min_lo, &max_la, &min_la);
        lon = (max_lo + min_lo) / 2;
        lat = (max_la + min_la) / 2;
        elm_map_region_show(ELM_WIDGET_DATA(overlay->wsd)->obj, lon, lat);
     }
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
#else
   (void)overlay;
#endif
}

EAPI void
elm_map_overlays_show(Eina_List *overlays)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Delayed_Data *data;
   Elm_Map_Overlay *overlay;

   EINA_SAFETY_ON_NULL_RETURN(overlays);
   EINA_SAFETY_ON_FALSE_RETURN(eina_list_count(overlays));

   overlay = eina_list_data_get(overlays);

   data = ELM_NEW(Delayed_Data);
   data->func = _overlays_show;
   data->wsd = overlay->wsd;
   data->overlays = eina_list_clone(overlays);
   data->wsd->delayed_jobs = eina_list_append(data->wsd->delayed_jobs, data);
   evas_object_smart_changed(data->wsd->pan_obj);
#else
   (void)overlays;
#endif
}

EAPI void
elm_map_overlay_region_set(Elm_Map_Overlay *overlay,
                           double lon,
                           double lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

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
#else
   (void)overlay;
   (void)lon;
   (void)lat;
#endif
}

EAPI void
elm_map_overlay_region_get(const Elm_Map_Overlay *overlay,
                           double *lon,
                           double *lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

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
#else
   (void)overlay;
   (void)lon;
   (void)lat;
#endif
}

EAPI void
elm_map_overlay_icon_set(Elm_Map_Overlay *overlay,
                         Evas_Object *icon)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(icon);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_icon_update(overlay->ovl, icon);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_icon_update(overlay->ovl, icon);
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
#else
   (void)overlay;
   (void)icon;
#endif
}

EAPI const Evas_Object *
elm_map_overlay_icon_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, NULL);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) NULL;

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
#else
   (void)overlay;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_content_set(Elm_Map_Overlay *overlay,
                            Evas_Object *content)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(content);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     _overlay_default_content_update(overlay->ovl, content, overlay);
   else if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_content_update(overlay->ovl, content);
   else ERR("Not supported overlay type: %d", overlay->type);

   evas_object_smart_changed(overlay->wsd->pan_obj);
#else
   (void)overlay;
   (void)obj;
#endif
}

EAPI const Evas_Object *
elm_map_overlay_content_get(const Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(overlay->wsd, NULL);

   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj) NULL;

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
#else
   (void)overlay;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_color_set(Elm_Map_Overlay *overlay,
                          int r,
                          int g,
                          int b,
                          int a)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

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

#else
   (void)overlay;
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#endif
}

EAPI void
elm_map_overlay_color_get(const Elm_Map_Overlay *overlay,
                          int *r,
                          int *g,
                          int *b,
                          int *a)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_ROUTE)
     {
        if (r) *r = overlay->c.r;
        if (g) *g = overlay->c.g;
        if (b) *b = overlay->c.b;
        if (a) *a = overlay->c.a;
     }
   else ERR("Not supported overlay type: %d", overlay->type);
#else
   (void)overlay;
   (void)r;
   (void)g;
   (void)b;
   (void)a;
#endif
}

EAPI void
elm_map_overlay_get_cb_set(Elm_Map_Overlay *overlay,
                           Elm_Map_Overlay_Get_Cb get_cb,
                           void *data)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   overlay->cb = get_cb;
   overlay->cb_data = data;

   if (overlay->type == ELM_MAP_OVERLAY_TYPE_CLASS)
     _overlay_class_cb_set(overlay->ovl, get_cb, data);
#else
   (void)overlay;
   (void)get_cb;
   (void)data;
#endif
}

EAPI void
elm_map_overlay_del_cb_set(Elm_Map_Overlay *overlay,
                           Elm_Map_Overlay_Del_Cb del_cb,
                           void *data)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(overlay->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);

   overlay->del_cb = del_cb;
   overlay->del_cb_data = data;
#else
   (void)overlay;
   (void)del_cb;
   (void)data;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_class_add(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_CLASS;
   overlay->ovl = _overlay_class_new(sd);
   overlay->c.r = 0x90;
   overlay->c.g = 0xee;
   overlay->c.b = 0x90;
   overlay->c.a = 0xff;
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_class_append(Elm_Map_Overlay *klass,
                             Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Class *class_ovl;

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(klass->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(klass->wsd)->obj);
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
   _overlay_group_content_update(overlay->grp, class_ovl->content);
   _overlay_group_color_update(overlay->grp, klass->c);
   _overlay_group_cb_set(overlay->grp, klass->cb, klass->data);
   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        _overlay_default_class_icon_update(overlay->ovl, class_ovl->icon);
        _overlay_default_class_content_update
          (overlay->ovl, class_ovl->content);
     }

   evas_object_smart_changed(klass->wsd->pan_obj);
#else
   (void)klass;
   (void)overlay;
#endif
}

EAPI void
elm_map_overlay_class_remove(Elm_Map_Overlay *klass,
                             Elm_Map_Overlay *overlay)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Class *ovl;

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(overlay);
   EINA_SAFETY_ON_NULL_RETURN(klass->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(klass->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(klass->type == ELM_MAP_OVERLAY_TYPE_CLASS);

   ovl = klass->ovl;
   ovl->members = eina_list_remove(ovl->members, overlay);

   overlay->grp->klass = NULL;
   _overlay_group_icon_update(overlay->grp, NULL);
   _overlay_group_content_update(overlay->grp, NULL);
   if (overlay->type == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        _overlay_default_class_icon_update(overlay->ovl, NULL);
        _overlay_default_class_content_update(overlay->ovl, NULL);
     }

   evas_object_smart_changed(klass->wsd->pan_obj);
#else
   (void)klass;
   (void)overlay;
#endif
}

EAPI void
elm_map_overlay_class_zoom_max_set(Elm_Map_Overlay *klass,
                                   int zoom)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Class *ovl;

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(klass->wsd);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(klass->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(klass->type == ELM_MAP_OVERLAY_TYPE_CLASS);

   ovl = klass->ovl;
   if (ovl->zoom_max == !!zoom) return;
   ovl->zoom_max = zoom;

   evas_object_smart_changed(klass->wsd->pan_obj);
#else
   (void)klass;
   (void)zoom;
#endif
}

EAPI int
elm_map_overlay_class_zoom_max_get(const Elm_Map_Overlay *klass)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   const Overlay_Class *ovl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(klass, OVERLAY_CLASS_ZOOM_MAX);
   EINA_SAFETY_ON_NULL_RETURN_VAL(klass->wsd, OVERLAY_CLASS_ZOOM_MAX);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(klass->wsd)->obj) OVERLAY_CLASS_ZOOM_MAX;

   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (klass->type == ELM_MAP_OVERLAY_TYPE_CLASS, OVERLAY_CLASS_ZOOM_MAX);

   ovl = klass->ovl;
   return ovl->zoom_max;
#else
   (void)klass;
   return OVERLAY_CLASS_ZOOM_MAX;
#endif
}

EAPI Eina_List *
elm_map_overlay_group_members_get(const Elm_Map_Overlay *grp)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Group *ovl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(grp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(grp->wsd, NULL);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(grp->wsd)->obj) NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (grp->type == ELM_MAP_OVERLAY_TYPE_GROUP, NULL);

   ovl = grp->ovl;
   return ovl->members;
#else
   (void)clas;
   return OVERLAY_CLASS_ZOOM_MAX;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_bubble_add(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_BUBBLE;
   overlay->c.r = 255;
   overlay->c.g = 255;
   overlay->c.b = 255;
   overlay->c.a = 255;
   overlay->ovl = _overlay_bubble_new(overlay);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_bubble_follow(Elm_Map_Overlay *bubble,
                              const Elm_Map_Overlay *parent)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Bubble *ovl;
   Evas_Object *pobj;

   EINA_SAFETY_ON_NULL_RETURN(bubble);
   EINA_SAFETY_ON_NULL_RETURN(parent);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(bubble->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(bubble->type == ELM_MAP_OVERLAY_TYPE_BUBBLE);

   ovl = bubble->ovl;
   pobj = _overlay_obj_get(parent);
   if (!pobj) return;

   ovl->pobj = pobj;
   evas_object_smart_changed(bubble->wsd->pan_obj);
#else
   (void)bubble;
   (void)parent;
#endif
}

EAPI void
elm_map_overlay_bubble_content_append(Elm_Map_Overlay *bubble,
                                      Evas_Object *content)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Bubble *bb;

   EINA_SAFETY_ON_NULL_RETURN(bubble);
   EINA_SAFETY_ON_NULL_RETURN(content);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(bubble->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(bubble->type == ELM_MAP_OVERLAY_TYPE_BUBBLE);

   bb = bubble->ovl;
   elm_box_pack_end(bb->bx, content);

   evas_object_smart_changed(bubble->wsd->pan_obj);
#else
   (void)bubble;
   (void)content;
#endif
}

EAPI void
elm_map_overlay_bubble_content_clear(Elm_Map_Overlay *bubble)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Bubble *bb;

   EINA_SAFETY_ON_NULL_RETURN(bubble);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(bubble->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(bubble->type == ELM_MAP_OVERLAY_TYPE_BUBBLE);

   bb = bubble->ovl;
   elm_box_clear(bb->bx);

   evas_object_smart_changed(bubble->wsd->pan_obj);
#else
   (void)bubble;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_route_add(Evas_Object *obj,
                          const Elm_Map_Route *route)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(route, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(route->wsd, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (obj == ELM_WIDGET_DATA(route->wsd)->obj, NULL);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_ROUTE;
   overlay->c.r = 0xff;
   overlay->c.g = 0x00;
   overlay->c.b = 0x00;
   overlay->c.a = 0xff;
   overlay->ovl = _overlay_route_new(sd, route, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   (void)route;
   return NULL;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_line_add(Evas_Object *obj,
                         double flon,
                         double flat,
                         double tlon,
                         double tlat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_LINE;
   overlay->c.r = 0xff;
   overlay->c.g = 0x00;
   overlay->c.b = 0x00;
   overlay->c.a = 0xff;
   overlay->ovl = _overlay_line_new(sd, flon, flat, tlon, tlat, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   (void)flon;
   (void)flat
     (void) tlon;
   (void)tlat;
   return NULL;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_polygon_add(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_POLYGON;
   overlay->c.r = 0xdc;
   overlay->c.g = 0x14;
   overlay->c.b = 0x3c;
   overlay->c.a = 220;
   overlay->ovl = _overlay_polygon_new(sd, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);
   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_map_overlay_polygon_region_add(Elm_Map_Overlay *overlay,
                                   double lon,
                                   double lat)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Overlay_Polygon *ovl;
   Region *r;

   EINA_SAFETY_ON_NULL_RETURN(overlay);
   ELM_MAP_CHECK(ELM_WIDGET_DATA(overlay->wsd)->obj);
   EINA_SAFETY_ON_FALSE_RETURN(overlay->type == ELM_MAP_OVERLAY_TYPE_POLYGON);

   ovl = overlay->ovl;
   r = ELM_NEW(Region);
   r->lon = lon;
   r->lat = lat;
   ovl->regions = eina_list_append(ovl->regions, r);

   evas_object_smart_changed(ovl->wsd->pan_obj);
#else
   (void)overlay;
   (void)lon;
   (void)lat
 #endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_circle_add(Evas_Object *obj,
                           double lon,
                           double lat,
                           double radius)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_CIRCLE;
   overlay->c.r = 0xdc;
   overlay->c.g = 0x14;
   overlay->c.b = 0x3c;
   overlay->c.a = 220;
   overlay->ovl = _overlay_circle_new(sd, lon, lat, radius, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI Elm_Map_Overlay *
elm_map_overlay_scale_add(Evas_Object *obj,
                          Evas_Coord x,
                          Evas_Coord y)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   Elm_Map_Overlay *overlay;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   overlay = ELM_NEW(Elm_Map_Overlay);
   overlay->wsd = sd;
   overlay->type = ELM_MAP_OVERLAY_TYPE_SCALE;
   overlay->c.r = 0;
   overlay->c.g = 0;
   overlay->c.b = 0;
   overlay->c.a = 255;
   overlay->ovl = _overlay_scale_new(sd, x, y, overlay->c);
   overlay->grp = _overlay_group_new(sd);
   sd->overlays = eina_list_append(sd->overlays, overlay);

   evas_object_smart_changed(sd->pan_obj);
   return overlay;
#else
   (void)obj;
   return NULL;
#endif
}

#ifdef ELM_EMAP
EAPI Evas_Object *
elm_map_track_add(Evas_Object *obj,
                  void *emap)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   EMap_Route *emapr = emap;
   Evas_Object *route;

   ELM_MAP_CHECK(obj) NULL;
   ELM_MAP_DATA_GET(obj, sd);

   route = elm_route_add(obj);
   elm_route_emap_set(route, emapr);
   sd->track = eina_list_append(sd->track, route);

   return route;
#else
   (void)obj;
   (void)emap;
   return NULL;
#endif
}

EAPI void
elm_map_track_remove(Evas_Object *obj,
                     Evas_Object *route)
{
#ifdef HAVE_ELEMENTARY_ECORE_CON
   ELM_MAP_CHECK(obj);
   ELM_MAP_DATA_GET(obj, sd);

   sd->track = eina_list_remove(sd->track, route);
   evas_object_del(route);
#else
   (void)obj;
   (void)route;
#endif
}

#else
EAPI Evas_Object *
elm_map_track_add(Evas_Object *obj __UNUSED__,
                  void *emap __UNUSED__)
{
   return NULL;
}

EAPI void
elm_map_track_remove(Evas_Object *obj __UNUSED__,
                     Evas_Object *route __UNUSED__)
{
}
#endif
