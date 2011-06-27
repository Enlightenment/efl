#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Route MapRoute
 *
 * For displaying a route on the map widget
 *
 */

typedef struct _Widget_Data Widget_Data;
typedef struct Segment Segment;

struct _Widget_Data
{
   Evas_Object *obj;
#ifdef ELM_EMAP
   EMap_Route *emap;
#endif

   double lon_min, lon_max;
   double lat_min, lat_max;

   Eina_List *segments; //list of *Segment

   Eina_Bool must_calc_segments :1;
};

struct Segment
{
   Evas_Object *obj;

#ifdef ELM_EMAP
   EMap_Route_Node *node_start;
   EMap_Route_Node *node_end;
#endif

   double start_x, start_y;
   double end_x, end_y;

   Eina_Bool must_calc :1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _clear_route(Evas_Object *obj);
#ifdef ELM_EMAP
static void _update_lon_lat_min_max(Evas_Object *obj, double lon, double lat);
#endif

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   _clear_route(obj);

   free(wd);
}

static void
_resize_cb(void *data __UNUSED__ , Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _sizing_eval(obj);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   //TODO
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Eina_List *l;
   Segment *segment;
   Evas_Coord x, y, w, h;
   Evas_Coord start_x, start_y, end_x, end_y;

   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   EINA_LIST_FOREACH(wd->segments, l, segment)
     {
        if(wd->must_calc_segments || segment->must_calc)
          {

#ifdef ELM_EMAP
             segment->start_x = (emap_route_node_lon_get(segment->node_start)- wd->lon_min) / (float)(wd->lon_max - wd->lon_min);
             segment->start_y = 1 - (emap_route_node_lat_get(segment->node_start) - wd->lat_min) / (float)(wd->lat_max - wd->lat_min);
             segment->end_x = (emap_route_node_lon_get(segment->node_end) - wd->lon_min) / (float)(wd->lon_max - wd->lon_min);
             segment->end_y = 1 - (emap_route_node_lat_get(segment->node_end) - wd->lat_min) / (float)(wd->lat_max - wd->lat_min);
#endif
             segment->must_calc = EINA_FALSE;
          }

        start_x = x+(int)(segment->start_x*w);
        start_y = y+(int)(segment->start_y*h);
        end_x = x+(int)(segment->end_x*w);
        end_y = y+(int)(segment->end_y*h);
        evas_object_line_xy_set(segment->obj, start_x, start_y, end_x, end_y);
     }

   wd->must_calc_segments = EINA_FALSE;
}

static void
_clear_route(Evas_Object *obj)
{
   Segment *segment;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

#ifdef ELM_EMAP
   wd->lon_min = EMAP_LON_MAX;
   wd->lon_max = EMAP_LON_MIN;
   wd->lat_min = EMAP_LAT_MAX;
   wd->lat_max = EMAP_LAT_MIN;
#endif

   EINA_LIST_FREE(wd->segments, segment)
     {
        evas_object_del(segment->obj);
        free(segment);
     }
}

#ifdef ELM_EMAP
static void
_update_lon_lat_min_max(Evas_Object *obj, double lon, double lat)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(wd->lon_min > lon)
     {
        wd->lon_min = lon;
        wd->must_calc_segments = EINA_TRUE;
     }
   if(wd->lat_min > lat)
     {
        wd->lat_min = lat;
        wd->must_calc_segments = EINA_TRUE;
     }

   if(wd->lon_max < lon)
     {
        wd->lon_max = lon;
        wd->must_calc_segments = EINA_TRUE;
     }
   if(wd->lat_max < lat)
     {
        wd->lat_max = lat;
        wd->must_calc_segments = EINA_TRUE;
     }
}
#endif

/**
 * Add a new route to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Route
 */
EAPI Evas_Object *
elm_route_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "map_route");
   elm_widget_type_set(obj, "map_route");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE,
                                  _resize_cb, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _resize_cb, obj);

#ifdef ELM_EMAP
   wd->lon_min = EMAP_LON_MAX;
   wd->lon_max = EMAP_LON_MIN;
   wd->lat_min = EMAP_LAT_MAX;
   wd->lat_max = EMAP_LAT_MIN;
#endif

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

#ifdef ELM_EMAP
/**
 * Set the emap object which describes the route
 *
 * @param obj The photo object
 * @param emap the route
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Route
 */
EAPI Eina_Bool
elm_route_emap_set(Evas_Object *obj, EMap_Route *emap)
{
   EMap_Route_Node *node, *node_prev = NULL;
   Evas_Object *o;
   Eina_List *l;

   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   wd->emap = emap;

   _clear_route(obj);

   EINA_LIST_FOREACH(emap_route_nodes_get(wd->emap), l, node)
     {
        if(node_prev)
          {
             Segment *segment = calloc(1, sizeof(Segment));
             segment->node_start = node_prev;
             segment->node_end = node;

             o = evas_object_line_add(evas_object_evas_get(obj));
             segment->obj = o;
             evas_object_smart_member_add(o, obj);
             evas_object_show(o);

             segment->must_calc = EINA_TRUE;

             _update_lon_lat_min_max(obj, emap_route_node_lon_get(node_prev), emap_route_node_lat_get(node_prev));
             _update_lon_lat_min_max(obj, emap_route_node_lon_get(node), emap_route_node_lat_get(node));

             wd->segments = eina_list_append(wd->segments, segment);
          }
        node_prev = node;
     }

   _sizing_eval(obj);
}
#endif

EAPI double
elm_route_lon_min_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->lon_min;
}

EAPI double
elm_route_lat_min_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->lat_min;
}

EAPI double
elm_route_lon_max_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->lon_max;
}

EAPI double
elm_route_lat_max_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->lat_max;
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
