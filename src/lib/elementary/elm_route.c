#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_route.h"

#define MY_CLASS ELM_ROUTE_CLASS

#define MY_CLASS_NAME "Elm_Route"
#define MY_CLASS_NAME_LEGACY "elm_route"

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

   EINA_LIST_FREE(sd->segments, segment)
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

   EINA_LIST_FOREACH(sd->segments, l, segment)
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
_move_resize_cb(void *data EINA_UNUSED,
                Evas *e EINA_UNUSED,
                Evas_Object *obj,
                void *event_info EINA_UNUSED)
{
   _sizing_eval(obj);
}

EOLIAN static Eina_Error
_elm_route_efl_ui_widget_theme_apply(Eo *obj, Elm_Route_Data *sd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   //TODO

   _sizing_eval(obj);

   return int_ret;
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

EOLIAN static void
_elm_route_efl_canvas_group_group_add(Eo *obj, Elm_Route_Data *priv)
{

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
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
#else
   (void)priv;
#endif

   _sizing_eval(obj);
}

EOLIAN static void
_elm_route_efl_canvas_group_group_del(Eo *obj, Elm_Route_Data *_pd EINA_UNUSED)
{
   _clear_route(obj);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

/**
 * Add a new route to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Route
 */
EAPI Evas_Object *
elm_route_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_route_efl_object_constructor(Eo *obj, Elm_Route_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);

   return obj;
}

EOLIAN static void
_elm_route_emap_set(Eo *obj, Elm_Route_Data *sd, void *_emap)
{
#ifdef ELM_EMAP
   EMap_Route *emap = _emap;

   EMap_Route_Node *node, *node_prev = NULL;
   Evas_Object *o;
   Eina_List *l;

   sd->emap = emap;

   _clear_route(obj);

   EINA_LIST_FOREACH(emap_route_nodes_get(sd->emap), l, node)
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
#else
   (void)obj;
   (void)sd;
   (void)_emap;
#endif
}

EOLIAN static void
_elm_route_longitude_min_max_get(const Eo *obj EINA_UNUSED, Elm_Route_Data *sd, double *min, double *max)
{
   if (min) *min = sd->lon_min;
   if (max) *max = sd->lon_max;
}

EOLIAN static void
_elm_route_latitude_min_max_get(const Eo *obj EINA_UNUSED, Elm_Route_Data *sd, double *min, double *max)
{
   if (min) *min = sd->lat_min;
   if (max) *max = sd->lat_max;
}

EOLIAN static void
_elm_route_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Internal EO APIs and hidden overrides */

#define ELM_ROUTE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_route)

#include "elm_route_eo.c"
