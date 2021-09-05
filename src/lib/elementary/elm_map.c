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

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"zoom", _key_action_move},
   {NULL, NULL}
};

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
_elm_map_pan_efl_gfx_entity_size_set(Eo *obj, Elm_Map_Pan_Data *psd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h)) return;
   efl_gfx_entity_size_set(efl_super(obj, MY_PAN_CLASS), sz);
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_map_pan_efl_canvas_group_group_calculate(Eo *obj, Elm_Map_Pan_Data *psd EINA_UNUSED)
{
   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
}

EOLIAN static void
_elm_map_pan_efl_gfx_entity_position_set(Eo *obj, Elm_Map_Pan_Data *_pd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y)) return;
   efl_gfx_entity_position_set(efl_super(obj, MY_PAN_CLASS), pos);
   evas_object_smart_changed(obj);
}

EOLIAN static Eina_Bool
_elm_map_efl_ui_focus_object_on_focus_update(Eo *obj EINA_UNUSED, Elm_Map_Data *_pd EINA_UNUSED)
{ return EINA_FALSE; }

EOLIAN static void
_elm_map_pan_efl_object_destructor(Eo *obj, Elm_Map_Pan_Data *psd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_PAN_CLASS));
}

static void
_elm_map_pan_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_PAN_CLASS_NAME_LEGACY, klass);
}

#include "elm_map_pan_eo.c"

EOLIAN static Eina_Error
_elm_map_efl_ui_widget_theme_apply(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return 0; }

static Eina_Bool
_key_action_move(Evas_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{ return EINA_TRUE; }

EOLIAN static void
_elm_map_efl_canvas_group_group_add(Eo *obj EINA_UNUSED, Elm_Map_Data *priv EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_efl_object_invalidate(Eo *obj, Elm_Map_Data *sd EINA_UNUSED)
{
   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_map_efl_canvas_group_group_del(Eo *obj, Elm_Map_Data *sd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_map_efl_gfx_entity_position_set(Eo *obj, Elm_Map_Data *sd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y)) return;
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_map_efl_gfx_entity_size_set(Eo *obj, Elm_Map_Data *sd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h)) return;
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_map_efl_canvas_group_group_member_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Evas_Object *member EINA_UNUSED)
{ }

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
_elm_map_efl_ui_zoom_zoom_level_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return 0; }

EOLIAN static void
_elm_map_efl_ui_zoom_zoom_level_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double zoom EINA_UNUSED)
{ }

EAPI void
elm_map_zoom_set(Eo *obj EINA_UNUSED, int zoom EINA_UNUSED)
{ }

EAPI int
elm_map_zoom_get(const Eo *obj EINA_UNUSED)
{ return 0; }

EOLIAN static Efl_Ui_Zoom_Mode
_elm_map_efl_ui_zoom_zoom_mode_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return 0; }

EOLIAN static void
_elm_map_efl_ui_zoom_zoom_mode_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Efl_Ui_Zoom_Mode mode EINA_UNUSED)
{ }

EAPI void
elm_map_zoom_mode_set(Eo *obj EINA_UNUSED, Elm_Map_Zoom_Mode mode EINA_UNUSED)
{ }

EAPI Elm_Map_Zoom_Mode
elm_map_zoom_mode_get(const Eo *obj EINA_UNUSED)
{ return 0; }

EOLIAN static void
_elm_map_zoom_max_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, int zoom EINA_UNUSED)
{ }

EOLIAN static int
_elm_map_zoom_max_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return 0; }

EOLIAN static void
_elm_map_zoom_min_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, int zoom EINA_UNUSED)
{ }

EOLIAN static int
_elm_map_zoom_min_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return 0; }

EOLIAN static void
_elm_map_map_region_bring_in(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_region_show(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_region_zoom_bring_in(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, int zoom EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_region_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double *lon EINA_UNUSED, double *lat EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_efl_ui_zoom_zoom_animation_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Eina_Bool paused EINA_UNUSED)
{ }

EOLIAN static Eina_Bool
_elm_map_efl_ui_zoom_zoom_animation_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return EINA_FALSE; }

EOLIAN static void
_elm_map_map_rotate_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double degree EINA_UNUSED, Evas_Coord cx EINA_UNUSED, Evas_Coord cy EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_map_rotate_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double *degree EINA_UNUSED, Evas_Coord *cx EINA_UNUSED, Evas_Coord *cy EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_elm_interface_scrollable_wheel_disabled_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Eina_Bool disabled EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_tile_load_status_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, int *try_num EINA_UNUSED, int *finish_num EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_canvas_to_region_convert(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED, double *lon EINA_UNUSED, double *lat EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_region_to_canvas_convert(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED, Evas_Coord *x EINA_UNUSED, Evas_Coord *y EINA_UNUSED)
{ }

EOLIAN static void
_elm_map_user_agent_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, const char *user_agent EINA_UNUSED)
{ }

EOLIAN static const char *
_elm_map_user_agent_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return NULL; }

EOLIAN static void
_elm_map_source_set(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Elm_Map_Source_Type type EINA_UNUSED, const char *source_name EINA_UNUSED)
{ }

EOLIAN static const char *
_elm_map_source_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Elm_Map_Source_Type type EINA_UNUSED)
{ return NULL; }

EOLIAN static const char **
_elm_map_sources_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Elm_Map_Source_Type type EINA_UNUSED)
{ return NULL; }

EOLIAN static Elm_Map_Route *
_elm_map_route_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Elm_Map_Route_Type type EINA_UNUSED, Elm_Map_Route_Method method EINA_UNUSED, double flon EINA_UNUSED, double flat EINA_UNUSED, double tlon EINA_UNUSED, double tlat EINA_UNUSED, Elm_Map_Route_Cb route_cb EINA_UNUSED, void *data EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_route_del(Elm_Map_Route *route EINA_UNUSED)
{ }

EAPI double
elm_map_route_distance_get(const Elm_Map_Route *route EINA_UNUSED)
{ return 0; }

EAPI const char *
elm_map_route_node_get(const Elm_Map_Route *route EINA_UNUSED)
{ return NULL; }

EAPI const char *
elm_map_route_waypoint_get(const Elm_Map_Route *route EINA_UNUSED)
{ return NULL; }

EOLIAN static Elm_Map_Name *
_elm_map_name_add(const Eo *obj EINA_UNUSED, Elm_Map_Data *_pd EINA_UNUSED, const char *address EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED, Elm_Map_Name_Cb name_cb EINA_UNUSED, void *data EINA_UNUSED)
{ return NULL; }

EOLIAN static void
_elm_map_name_search(const Eo *obj EINA_UNUSED, Elm_Map_Data *_pd EINA_UNUSED, const char *address EINA_UNUSED, Elm_Map_Name_List_Cb name_cb EINA_UNUSED, void *data EINA_UNUSED)
{ }

EAPI void
elm_map_name_del(Elm_Map_Name *name EINA_UNUSED)
{ }

EAPI const char *
elm_map_name_address_get(const Elm_Map_Name *name EINA_UNUSED)
{  return NULL; }

EAPI void
elm_map_name_region_get(const Elm_Map_Name *name EINA_UNUSED,
                        double *lon EINA_UNUSED,
                        double *lat EINA_UNUSED)
{ }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED)
{ return NULL; }

EOLIAN static Eina_List *
_elm_map_overlays_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_del(Elm_Map_Overlay *overlay EINA_UNUSED)
{ }

EAPI Elm_Map_Overlay_Type
elm_map_overlay_type_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return 0; }

EAPI void
elm_map_overlay_data_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                         void *data EINA_UNUSED)
{ }

EAPI void *
elm_map_overlay_data_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_hide_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                         Eina_Bool hide EINA_UNUSED)
{ }

EAPI Eina_Bool
elm_map_overlay_hide_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return EINA_FALSE; }

EAPI void
elm_map_overlay_displayed_zoom_min_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                                       int zoom EINA_UNUSED)
{ }

EAPI int
elm_map_overlay_displayed_zoom_min_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return 0; }

EAPI void
elm_map_overlay_paused_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                           Eina_Bool paused EINA_UNUSED)
{ }

EAPI Eina_Bool
elm_map_overlay_paused_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return EINA_FALSE; }

EAPI Eina_Bool
elm_map_overlay_visible_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return EINA_FALSE; }

EAPI void
elm_map_overlay_show(Elm_Map_Overlay *overlay EINA_UNUSED)
{ }

EAPI void
elm_map_overlays_show(Eina_List *overlays EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_region_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                           double lon EINA_UNUSED,
                           double lat EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_region_get(const Elm_Map_Overlay *overlay EINA_UNUSED,
                           double *lon EINA_UNUSED,
                           double *lat EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_icon_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                         Evas_Object *icon EINA_UNUSED)
{ }

EAPI const Evas_Object *
elm_map_overlay_icon_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_content_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                            Evas_Object *content EINA_UNUSED)
{ }

EAPI const Evas_Object *
elm_map_overlay_content_get(const Elm_Map_Overlay *overlay EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_color_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                          int r EINA_UNUSED,
                          int g EINA_UNUSED,
                          int b EINA_UNUSED,
                          int a EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_color_get(const Elm_Map_Overlay *overlay EINA_UNUSED,
                          int *r EINA_UNUSED,
                          int *g EINA_UNUSED,
                          int *b EINA_UNUSED,
                          int *a EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_get_cb_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                           Elm_Map_Overlay_Get_Cb get_cb EINA_UNUSED,
                           void *data EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_del_cb_set(Elm_Map_Overlay *overlay EINA_UNUSED,
                           Elm_Map_Overlay_Del_Cb del_cb EINA_UNUSED,
                           void *data EINA_UNUSED)
{ }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_class_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_class_append(Elm_Map_Overlay *klass EINA_UNUSED,
                             Elm_Map_Overlay *overlay EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_class_remove(Elm_Map_Overlay *klass EINA_UNUSED,
                             Elm_Map_Overlay *overlay EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_class_zoom_max_set(Elm_Map_Overlay *klass EINA_UNUSED,
                                   int zoom EINA_UNUSED)
{ }

EAPI int
elm_map_overlay_class_zoom_max_get(const Elm_Map_Overlay *klass EINA_UNUSED)
{ return 0; }

EAPI Eina_List *
elm_map_overlay_group_members_get(const Elm_Map_Overlay *grp EINA_UNUSED)
{ return NULL; }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_bubble_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_bubble_follow(Elm_Map_Overlay *bubble EINA_UNUSED,
                              const Elm_Map_Overlay *parent EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_bubble_content_append(Elm_Map_Overlay *bubble EINA_UNUSED,
                                      Evas_Object *content EINA_UNUSED)
{ }

EAPI void
elm_map_overlay_bubble_content_clear(Elm_Map_Overlay *bubble EINA_UNUSED)
{ }

EAPI void
elm_map_wheel_disabled_set(Evas_Object *obj EINA_UNUSED, Eina_Bool disabled EINA_UNUSED)
{ }

EAPI Eina_Bool
elm_map_wheel_disabled_get(const Evas_Object *obj EINA_UNUSED)
{ return EINA_FALSE; }

EAPI void
elm_map_paused_set(Evas_Object *obj EINA_UNUSED, Eina_Bool paused EINA_UNUSED)
{ }

EAPI Eina_Bool
elm_map_paused_get(const Evas_Object *obj EINA_UNUSED)
{ return EINA_FALSE; }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_route_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, const Elm_Map_Route *route EINA_UNUSED)
{ return NULL; }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_line_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double flon EINA_UNUSED, double flat EINA_UNUSED, double tlon EINA_UNUSED, double tlat EINA_UNUSED)
{ return NULL; }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_polygon_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED)
{ return NULL; }

EAPI void
elm_map_overlay_polygon_region_add(Elm_Map_Overlay *overlay EINA_UNUSED,
                                   double lon EINA_UNUSED,
                                   double lat EINA_UNUSED)
{ }

EOLIAN static Elm_Map_Overlay*
_elm_map_overlay_circle_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, double lon EINA_UNUSED, double lat EINA_UNUSED, double radius EINA_UNUSED)
{ return NULL; }

EOLIAN static Elm_Map_Overlay *
_elm_map_overlay_scale_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED)
{ return NULL; }

EOLIAN static Evas_Object *
_elm_map_track_add(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, void *emap EINA_UNUSED)
{ return NULL; }

EOLIAN static void
_elm_map_track_remove(Eo *obj EINA_UNUSED, Elm_Map_Data *sd EINA_UNUSED, Evas_Object *route EINA_UNUSED)
{}

static void
_elm_map_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN const Efl_Access_Action_Data *
_elm_map_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Elm_Map_Data *pd EINA_UNUSED)
{
   static const Efl_Access_Action_Data atspi_actions[] = {
          { "move,prior", "move", "prior", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,left", "move", "left", _key_action_move},
          { "move,right", "move", "right", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { "zoom,in", "zoom", "in", _key_action_move},
          { "zoom,out", "zoom", "out", _key_action_move},
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
