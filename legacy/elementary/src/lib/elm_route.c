#include <Elementary.h>
#include "elm_priv.h"

static const char ROUTE_SMART_NAME[] = "elm_route";

/**
 * @defgroup Route MapRoute
 *
 * For displaying a route on the map widget
 *
 */

typedef struct _Elm_Route_Smart_Data Elm_Route_Smart_Data;
typedef struct Segment               Segment;

struct _Elm_Route_Smart_Data
{
   Elm_Widget_Smart_Data base;

#ifdef ELM_EMAP
   EMap_Route           *emap;
#endif

   double                lon_min, lon_max;
   double                lat_min, lat_max;

   Eina_List            *segments; //list of *Segment

   Eina_Bool             must_calc_segments : 1;
};

struct Segment
{
   Evas_Object     *obj;

#ifdef ELM_EMAP
   EMap_Route_Node *node_start;
   EMap_Route_Node *node_end;
#endif

   double           start_x, start_y;
   double           end_x, end_y;

   Eina_Bool        must_calc : 1;
};

#define ELM_ROUTE_DATA_GET(o, sd) \
  Elm_Route_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_ROUTE_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_ROUTE_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_ROUTE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ROUTE_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_ROUTE_CHECK(obj)                                             \
  if (!obj || !elm_widget_type_check((obj), ROUTE_SMART_NAME, __func__)) \
    return

EVAS_SMART_SUBCLASS_NEW
  (ROUTE_SMART_NAME, _elm_route, Elm_Widget_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, NULL);

static void
_clear_route(Evas_Object *obj)
{
   Segment *segment;

   ELM_ROUTE_DATA_GET(obj, sd);

#ifdef ELM_EMAP
   sd->lon_min = EMAP_LON_MAX;
   sd->lon_max = EMAP_LON_MIN;
   sd->lat_min = EMAP_LAT_MAX;
   sd->lat_max = EMAP_LAT_MIN;
#endif

   EINA_LIST_FREE (sd->segments, segment)
     {
        evas_object_del(segment->obj);
        free(segment);
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Eina_List *l;
   Segment *segment;
   Evas_Coord x, y, w, h;
   Evas_Coord start_x, start_y, end_x, end_y;

   ELM_ROUTE_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   EINA_LIST_FOREACH (sd->segments, l, segment)
     {
        if (sd->must_calc_segments || segment->must_calc)
          {
#ifdef ELM_EMAP
             segment->start_x =
               (emap_route_node_lon_get(segment->node_start) - sd->lon_min)
               / (float)(sd->lon_max - sd->lon_min);
             segment->start_y =
               1 - (emap_route_node_lat_get(segment->node_start)
                    - sd->lat_min) / (float)(sd->lat_max - sd->lat_min);
             segment->end_x =
               (emap_route_node_lon_get(segment->node_end) - sd->lon_min)
               / (float)(sd->lon_max - sd->lon_min);
             segment->end_y =
               1 - (emap_route_node_lat_get(segment->node_end)
                    - sd->lat_min) / (float)(sd->lat_max - sd->lat_min);
#endif
             segment->must_calc = EINA_FALSE;
          }

        start_x = x + (int)(segment->start_x * w);
        start_y = y + (int)(segment->start_y * h);
        end_x = x + (int)(segment->end_x * w);
        end_y = y + (int)(segment->end_y * h);

        evas_object_line_xy_set(segment->obj, start_x, start_y, end_x, end_y);
     }

   sd->must_calc_segments = EINA_FALSE;
}

static void
_move_resize_cb(void *data __UNUSED__,
                Evas *e __UNUSED__,
                Evas_Object *obj,
                void *event_info __UNUSED__)
{
   _sizing_eval(obj);
}

static Eina_Bool
_elm_route_smart_theme(Evas_Object *obj)
{
   if (!_elm_route_parent_sc->theme(obj)) return EINA_FALSE;

   //TODO

   _sizing_eval(obj);

   return EINA_TRUE;
}

#ifdef ELM_EMAP
static void
_update_lon_lat_min_max(Evas_Object *obj,
                        double lon,
                        double lat)
{
   ELM_ROUTE_DATA_GET(obj, sd);

   if (sd->lon_min > lon)
     {
        sd->lon_min = lon;
        sd->must_calc_segments = EINA_TRUE;
     }
   if (sd->lat_min > lat)
     {
        sd->lat_min = lat;
        sd->must_calc_segments = EINA_TRUE;
     }

   if (sd->lon_max < lon)
     {
        sd->lon_max = lon;
        sd->must_calc_segments = EINA_TRUE;
     }
   if (sd->lat_max < lat)
     {
        sd->lat_max = lat;
        sd->must_calc_segments = EINA_TRUE;
     }
}

#endif

static void
_elm_route_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Route_Smart_Data);

   _elm_route_parent_sc->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _move_resize_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _move_resize_cb, obj);

#ifdef ELM_EMAP
   priv->lon_min = EMAP_LON_MAX;
   priv->lon_max = EMAP_LON_MIN;
   priv->lat_min = EMAP_LAT_MAX;
   priv->lat_max = EMAP_LAT_MIN;
#endif

   _sizing_eval(obj);
}

static void
_elm_route_smart_del(Evas_Object *obj)
{
   _clear_route(obj);

   ELM_WIDGET_CLASS(_elm_route_parent_sc)->base.del(obj);
}

static void
_elm_route_smart_set_user(Elm_Widget_Smart_Class *sc)
{
   sc->base.add = _elm_route_smart_add;
   sc->base.del = _elm_route_smart_del;

   sc->theme = _elm_route_smart_theme;
}

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
   Evas *e;
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   e = evas_object_evas_get(parent);
   if (!e) return NULL;

   obj = evas_object_smart_add(e, _elm_route_smart_class_new());

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

#ifdef ELM_EMAP
/**
 * Set the emap object which describes the route
 *
 * @param obj The photo object
 * @param emap the route
 *
 * @ingroup Route
 */
EAPI void
elm_route_emap_set(Evas_Object *obj,
                   EMap_Route *emap)
{
   EMap_Route_Node *node, *node_prev = NULL;
   Evas_Object *o;
   Eina_List *l;

   ELM_ROUTE_CHECK(obj);
   ELM_ROUTE_DATA_GET(obj, sd);

   sd->emap = emap;

   _clear_route(obj);

   EINA_LIST_FOREACH (emap_route_nodes_get(sd->emap), l, node)
     {
        if (node_prev)
          {
             Segment *segment = calloc(1, sizeof(Segment));

             segment->node_start = node_prev;
             segment->node_end = node;

             o = evas_object_line_add(evas_object_evas_get(obj));
             segment->obj = o;
             evas_object_smart_member_add(o, obj);

             segment->must_calc = EINA_TRUE;

             _update_lon_lat_min_max
               (obj, emap_route_node_lon_get(node_prev),
               emap_route_node_lat_get(node_prev));
             _update_lon_lat_min_max
               (obj, emap_route_node_lon_get(node),
               emap_route_node_lat_get(node));

             sd->segments = eina_list_append(sd->segments, segment);
          }

        node_prev = node;
     }

   _sizing_eval(obj);
}

#endif

EAPI void
elm_route_longitude_min_max_get(const Evas_Object *obj,
                                double *min,
                                double *max)
{
   ELM_ROUTE_CHECK(obj);
   ELM_ROUTE_DATA_GET(obj, sd);

   if (min) *min = sd->lon_min;
   if (max) *max = sd->lon_max;
}

EAPI void
elm_route_latitude_min_max_get(const Evas_Object *obj,
                               double *min,
                               double *max)
{
   ELM_ROUTE_CHECK(obj);
   ELM_ROUTE_DATA_GET(obj, sd);

   if (min) *min = sd->lat_min;
   if (max) *max = sd->lat_max;
}
