#ifndef ELM_WIDGET_ROUTE_H
#define ELM_WIDGET_ROUTE_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-route-class The Elementary Route Class
 *
 * Elementary, besides having the @ref Route widget, exposes its
 * foundation -- the Elementary Route Class -- in order to create other
 * widgets which are a route with some more logic on top.
 */

/**
 * Base widget smart data extended with route instance data.
 */
typedef struct _Elm_Route_Smart_Data Elm_Route_Smart_Data;
typedef struct Segment               Segment;

struct _Elm_Route_Smart_Data
{
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
/**
 * @}
 */

#define ELM_ROUTE_DATA_GET(o, sd) \
  Elm_Route_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_ROUTE_CLASS)

#define ELM_ROUTE_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_ROUTE_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_ROUTE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ROUTE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_ROUTE_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_ROUTE_CLASS))) \
    return

#endif
