#ifndef ELM_WIDGET_GENGRID_H
#define ELM_WIDGET_GENGRID_H

#include "elm_gen_common.h"
#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-gengrid-class The Elementary Gengrid Class
 *
 * Elementary, besides having the @ref Gengrid widget, exposes its
 * foundation -- the Elementary Gengrid Class -- in order to create
 * other widgets which are a gengrid with some more logic on top.
 */

/**
 * @def ELM_GENGRID_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Gengrid_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_GENGRID_CLASS(x) ((Elm_Gengrid_Smart_Class *)x)

/**
 * @def ELM_GENGRID_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Gengrid_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_GENGRID_DATA(x)  ((Elm_Gengrid_Smart_Data *)x)

/**
 * @def ELM_GENGRID_SMART_CLASS_VERSION
 *
 * Current version for Elementary gengrid @b base smart class, a value
 * which goes to _Elm_Gengrid_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_GENGRID_SMART_CLASS_VERSION 1

/**
 * @def ELM_GENGRID_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Gengrid_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_GENGRID_SMART_CLASS_INIT_NULL
 * @see ELM_GENGRID_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_GENGRID_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_GENGRID_SMART_CLASS_VERSION}

/**
 * @def ELM_GENGRID_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Gengrid_Smart_Class structure.
 *
 * @see ELM_GENGRID_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_GENGRID_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_GENGRID_SMART_CLASS_INIT_NULL \
  ELM_GENGRID_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_GENGRID_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Gengrid_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_GENGRID_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Gengrid_Smart_Class (base field)
 * to the latest #ELM_GENGRID_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_GENGRID_SMART_CLASS_INIT_NULL
 * @see ELM_GENGRID_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_GENGRID_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_GENGRID_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary gengrid base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a gengrid.
 *
 * All of the functions listed on @ref Gengrid namespace will work for
 * objects deriving from #Elm_Gengrid_Smart_Class.
 */
typedef struct _Elm_Gengrid_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Gengrid_Smart_Class;

/**
 * Base widget smart data extended with gengrid instance data.
 */
typedef struct _Elm_Gengrid_Smart_Data Elm_Gengrid_Smart_Data;
struct _Elm_Gengrid_Smart_Data
{
   Elm_Layout_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   const Elm_Scrollable_Smart_Interface *s_iface;

   Eina_Inlist_Sorted_State             *state;
   Evas_Object                          *hit_rect;
   Evas_Object                          *pan_obj;

   Eina_List                            *selected; /* a list of
                                                    * selected
                                                    * items */
   Eina_List                            *group_items; /* a list of
                                                       * groups index
                                                       * items */
   Eina_Inlist                          *items; /* an inlist of all items */
   Elm_Gen_Item                         *reorder_it; /* item currently
                                                     * being
                                                     * repositioned */
   Elm_Object_Item                      *last_selected_item;

   Ecore_Job                            *calc_job;
   int                                   walking;
   int                                   item_width, item_height;
   int                                   group_item_width, group_item_height;
   int                                   minw, minh;
   unsigned int                          item_count;
   Evas_Coord                            pan_x, pan_y;
   Elm_Object_Select_Mode                select_mode;

   Ecore_Cb                              calc_cb;
   Ecore_Cb                              clear_cb;

   /* longpress timeout. this value comes from _elm_config by
    * default. this can be changed by
    * elm_gengrid_longpress_timeout_set() */
   double                                longpress_timeout;

   /* a generation of gengrid. when gengrid is cleared, this value
    * will be increased and a new generation will start */
   int                                   generation;
   Eina_Compare_Cb                       item_compare_cb;

   /* The stuff below directly come from gengrid without any thinking */
   unsigned int                          nmax;
   Evas_Coord                            reorder_item_x, reorder_item_y;
   Evas_Coord                            old_pan_x, old_pan_y;
   long                                  items_lost;
   double                                align_x, align_y;

   Eina_Bool                             reorder_item_changed : 1;
   Eina_Bool                             move_effect_enabled : 1;

   Eina_Bool                             old_h_bounce;
   Eina_Bool                             old_v_bounce;

   /* a flag for reorder mode enable/disable */
   Eina_Bool                             reorder_mode : 1;
   Eina_Bool                             was_selected : 1;

   Eina_Bool                             longpressed : 1;
   Eina_Bool                             pan_changed : 1;
   Eina_Bool                             horizontal : 1;

   /* a flag for items can be highlighted or not. by default this flag
    * is true. */
   Eina_Bool                             highlight : 1;

   Eina_Bool                             clear_me : 1; /**< a flag whether
                                                        * gengrid is marked
                                                        * as to be cleared or
                                                        * not. if this flag
                                                        * is true, gengrid
                                                        * clear was already
                                                        * deferred.  */

   Eina_Bool                             on_hold : 1;

   Eina_Bool                             filled : 1;
   Eina_Bool                             multi : 1; /* a flag for item
                                                     * multi
                                                     * selection */
};

struct Elm_Gen_Item_Type
{
   Elm_Gen_Item           *it;

   Elm_Gengrid_Smart_Data *wsd;

   Ecore_Animator         *item_reorder_move_animator;
   Evas_Coord              gx, gy, ox, oy, tx, ty, rx, ry;
   unsigned int            moving_effect_start_time;
   int                     prev_group;

   Eina_Bool               group_realized : 1;
   Eina_Bool               moving : 1;
};

typedef struct _Elm_Gengrid_Pan_Smart_Class
{
   Elm_Pan_Smart_Class base;
} Elm_Gengrid_Pan_Smart_Class;

typedef struct _Elm_Gengrid_Pan_Smart_Data Elm_Gengrid_Pan_Smart_Data;
struct _Elm_Gengrid_Pan_Smart_Data
{
   Elm_Pan_Smart_Data      base;
   Elm_Gengrid_Smart_Data *wsd;
   Ecore_Job              *resize_job;
};

/**
 * @}
 */

EAPI extern const char ELM_GENGRID_SMART_NAME[];
EAPI const Elm_Gengrid_Smart_Class *elm_gengrid_smart_class_get(void);

#define ELM_GENGRID_DATA_GET(o, sd) \
  Elm_Gengrid_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_GENGRID_PAN_DATA_GET(o, sd) \
  Elm_Gengrid_Pan_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_GENGRID_DATA_GET_OR_RETURN(o, ptr)       \
  ELM_GENGRID_DATA_GET(o, ptr);                      \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_GENGRID_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_GENGRID_DATA_GET(o, ptr);                         \
  if (!ptr)                                             \
    {                                                   \
       CRITICAL("No widget data for object %p (%s)",    \
                o, evas_object_type_get(o));            \
       return val;                                      \
    }

#define ELM_GENGRID_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_GENGRID_SMART_NAME, __func__)) \
    return

#define ELM_GENGRID_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_GENGRID_CHECK(it->base.widget);

#define ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_GENGRID_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_GENGRID_ITEM_CHECK_OR_GOTO(it, label)              \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !elm_widget_type_check               \
        ((it->base.widget), ELM_GENGRID_SMART_NAME, __func__)) goto label;

#endif
