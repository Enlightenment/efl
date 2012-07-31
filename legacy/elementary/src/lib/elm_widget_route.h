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
 * @def ELM_ROUTE_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Route_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ROUTE_CLASS(x) ((Elm_Route_Smart_Class *)x)

/**
 * @def ELM_ROUTE_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Route_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_ROUTE_DATA(x)  ((Elm_Route_Smart_Data *)x)

/**
 * @def ELM_ROUTE_SMART_CLASS_VERSION
 *
 * Current version for Elementary route @b base smart class, a value
 * which goes to _Elm_Route_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_ROUTE_SMART_CLASS_VERSION 1

/**
 * @def ELM_ROUTE_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Route_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ROUTE_SMART_CLASS_INIT_NULL
 * @see ELM_ROUTE_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_ROUTE_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_ROUTE_SMART_CLASS_VERSION}

/**
 * @def ELM_ROUTE_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Route_Smart_Class structure.
 *
 * @see ELM_ROUTE_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_ROUTE_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ROUTE_SMART_CLASS_INIT_NULL \
  ELM_ROUTE_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_ROUTE_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Route_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_ROUTE_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Route_Smart_Class (base field)
 * to the latest #ELM_ROUTE_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_ROUTE_SMART_CLASS_INIT_NULL
 * @see ELM_ROUTE_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_ROUTE_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_ROUTE_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary route base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a route.
 *
 * All of the functions listed on @ref Route namespace will work for
 * objects deriving from #Elm_Route_Smart_Class.
 */
typedef struct _Elm_Route_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Route_Smart_Class;

/**
 * Base widget smart data extended with route instance data.
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
/**
 * @}
 */

EAPI extern const char ELM_ROUTE_SMART_NAME[];
EAPI const Elm_Route_Smart_Class *elm_route_smart_class_get(void);

#define ELM_ROUTE_DATA_GET(o, sd) \
  Elm_Route_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_ROUTE_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_ROUTE_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_ROUTE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ROUTE_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_ROUTE_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_ROUTE_SMART_NAME, __func__)) \
    return

#endif
