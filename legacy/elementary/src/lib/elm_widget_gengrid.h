#ifndef ELM_WIDGET_GENGRID_H
#define ELM_WIDGET_GENGRID_H

#include "elm_gen_common.h"
#include "Elementary.h"

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
 * Base widget smart data extended with gengrid instance data.
 */
typedef struct _Elm_Gengrid_Smart_Data Elm_Gengrid_Smart_Data;
struct _Elm_Gengrid_Smart_Data
{
   Eina_Inlist_Sorted_State             *state;
   Evas_Object                          *hit_rect;
   Evas_Object                          *pan_obj;
   Evas_Object                          *obj; // the object itself
   Evas_Object                          *stack; // stacking markers in pan for selectraise feature

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
   Elm_Gen_Item                         *show_it;
   Elm_Gen_Item                         *bring_in_it;
   Elm_Gengrid_Item_Scrollto_Type        scroll_to_type;

   Ecore_Job                            *calc_job;
   int                                   walking;
   int                                   item_width, item_height;
   int                                   group_item_width, group_item_height;
   int                                   minw, minh;
   unsigned int                          item_count;
   Evas_Coord                            pan_x, pan_y;
   Elm_Object_Select_Mode                select_mode;
   Elm_Object_Multi_Select_Mode          multi_select_mode; /**< select mode for multiple selection */

   Ecore_Cb                              calc_cb;
   Ecore_Cb                              clear_cb;

   /* longpress timeout. this value comes from _elm_config by
    * default. this can be changed by
    * elm_config_longpress_timeout_set() */
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
   Eina_Bool                             show_region : 1;
   Eina_Bool                             bring_in : 1;
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

typedef struct _Elm_Gengrid_Pan_Smart_Data Elm_Gengrid_Pan_Smart_Data;
struct _Elm_Gengrid_Pan_Smart_Data
{
   Evas_Object            *wobj;
   Elm_Gengrid_Smart_Data *wsd;
   Ecore_Job              *resize_job;
};

/**
 * @}
 */

#define ELM_GENGRID_DATA_GET(o, sd) \
  Elm_Gengrid_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_GENGRID_CLASS)

#define ELM_GENGRID_PAN_DATA_GET(o, sd) \
  Elm_Gengrid_Pan_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_GENGRID_PAN_CLASS)

#define ELM_GENGRID_DATA_GET_OR_RETURN(o, ptr)       \
  ELM_GENGRID_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_GENGRID_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_GENGRID_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       CRI("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#define ELM_GENGRID_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_GENGRID_CLASS))) \
    return

#define ELM_GENGRID_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_GENGRID_CHECK(it->base.widget);

#define ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_GENGRID_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_GENGRID_ITEM_CHECK_OR_GOTO(it, label)              \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !eo_isa                              \
        ((it->base.widget), ELM_OBJ_GENGRID_CLASS)) goto label;

#endif
