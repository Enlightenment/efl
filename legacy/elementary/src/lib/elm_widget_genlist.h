#ifndef ELM_WIDGET_GENLIST_H
#define ELM_WIDGET_GENLIST_H

#include "elm_gen_common.h"
#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-genlist-class The Elementary Genlist Class
 *
 * Elementary, besides having the @ref Genlist widget, exposes its
 * foundation -- the Elementary Genlist Class -- in order to create
 * other widgets which are a genlist with some more logic on top.
 */

/**
 * @def ELM_GENLIST_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Genlist_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_GENLIST_CLASS(x) ((Elm_Genlist_Smart_Class *)x)

/**
 * @def ELM_GENLIST_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Genlist_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_GENLIST_DATA(x)  ((Elm_Genlist_Smart_Data *)x)

/**
 * @def ELM_GENLIST_SMART_CLASS_VERSION
 *
 * Current version for Elementary genlist @b base smart class, a value
 * which goes to _Elm_Genlist_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_GENLIST_SMART_CLASS_VERSION 1

/**
 * @def ELM_GENLIST_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Genlist_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_GENLIST_SMART_CLASS_INIT_NULL
 * @see ELM_GENLIST_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_GENLIST_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_GENLIST_SMART_CLASS_VERSION}

/**
 * @def ELM_GENLIST_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Genlist_Smart_Class structure.
 *
 * @see ELM_GENLIST_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_GENLIST_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_GENLIST_SMART_CLASS_INIT_NULL \
  ELM_GENLIST_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_GENLIST_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Genlist_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_GENLIST_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Genlist_Smart_Class (base field)
 * to the latest #ELM_GENLIST_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_GENLIST_SMART_CLASS_INIT_NULL
 * @see ELM_GENLIST_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_GENLIST_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_GENLIST_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary genlist base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a genlist.
 *
 * All of the functions listed on @ref Genlist namespace will work for
 * objects deriving from #Elm_Genlist_Smart_Class.
 */
typedef struct _Elm_Genlist_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Genlist_Smart_Class;

/**
 * Base widget smart data extended with genlist instance data.
 */
typedef struct _Elm_Genlist_Smart_Data Elm_Genlist_Smart_Data;

typedef enum
{
   ELM_GENLIST_TREE_EFFECT_NONE = 0,
   ELM_GENLIST_TREE_EFFECT_EXPAND = 1,
   ELM_GENLIST_TREE_EFFECT_CONTRACT = 2
} Elm_Genlist_Item_Move_Effect_Mode;

struct _Elm_Genlist_Smart_Data
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

   Eina_Inlist                          *blocks; /* an inlist of all
                                                  * blocks. a block
                                                  * consists of a
                                                  * certain number of
                                                  * items. maximum
                                                  * number of items in
                                                  * a block is
                                                  * 'max_items_per_block'. */
   Evas_Coord                            reorder_old_pan_y, w, h, realminw,
                                         prev_viewport_w;
   Ecore_Job                            *update_job;
   Ecore_Idle_Enterer                   *queue_idle_enterer;
   Ecore_Idler                          *must_recalc_idler;
   Eina_List                            *queue;
   Elm_Gen_Item                         *show_item, *anchor_item, *mode_item,
                                        *reorder_rel, *expanded_item;
   Eina_Inlist                          *item_cache; /* an inlist of
                                                      * edje object it
                                                      * cache. */
   Evas_Coord                            anchor_y;
   Evas_Coord                            reorder_start_y; /* reorder
                                                           * it's
                                                           * initial y
                                                           * coordinate
                                                           * in the
                                                           * pan. */
   Elm_List_Mode                         mode;
   Ecore_Timer                          *multi_timer, *scr_hold_timer;
   Ecore_Animator                       *reorder_move_animator;
   const char                           *decorate_it_type;
   double                                start_time;
   Evas_Coord                            prev_x, prev_y, prev_mx, prev_my;
   Evas_Coord                            cur_x, cur_y, cur_mx, cur_my;

   struct
   {
      Evas_Coord x, y;
   } history[SWIPE_MOVES];

   int                                   multi_device;
   int                                   item_cache_count;
   int                                   item_cache_max; /* maximum
                                                         * number of
                                                         * cached
                                                         * items */
   int                                   movements;

   /* maximum number of items per block */
   int                                   max_items_per_block;

   /* longpress timeout. this value comes from _elm_config by
    * default. this can be changed by
    * elm_genlist_longpress_timeout_set() */
   double                                longpress_timeout;
   /* a generation of genlist. when genlist is cleared, this value
    * will be increased and a new generation will start */
   int                                   generation;
   Eina_Compare_Cb                       item_compare_cb;
   Eina_Compare_Cb                       item_compare_data_cb;

   /* a scrollto type which remembers where to scroll ex) in, top,
    * middle */
   Elm_Genlist_Item_Scrollto_Type        scroll_to_type;
   Evas_Object                          *alpha_bg; /* not to receive
                                                    * event when tree
                                                    * effect is not
                                                    * finished */
   Eina_List                            *move_items; /* items move for
                                                      * tree effect */
   Elm_Gen_Item                         *expanded_next_item;
   Ecore_Animator                       *tree_effect_animator;
   Elm_Genlist_Item_Move_Effect_Mode     move_effect_mode;

   Eina_Bool                             tree_effect_enabled : 1;
   Eina_Bool                             auto_scroll_enabled : 1;
   Eina_Bool                             decorate_all_mode : 1;
   Eina_Bool                             height_for_width : 1;
   Eina_Bool                             reorder_pan_move : 1;
   Eina_Bool                             multi_timeout : 1;
   Eina_Bool                             multi_touched : 1;
   Eina_Bool                             reorder_mode : 1; /* a flag
                                                            * for
                                                            * reorder
                                                            * mode
                                                            * enable/disable */
   /* this flag means genlist is supposed to be scrolled. if this flag
    * is set to EINA_TRUE, genlist checks whether it's ok to scroll
    * genlist now or not. */
   Eina_Bool                             check_scroll : 1;
   Eina_Bool                             pan_changed : 1;
   Eina_Bool                             wasselected : 1;
   Eina_Bool                             homogeneous : 1;
   Eina_Bool                             longpressed : 1;
   /* a flag for items can be highlighted or not. by default this flag
    * is true. */
   Eina_Bool                             mouse_down : 1;
   Eina_Bool                             multi_down : 1;
   Eina_Bool                             on_sub_del : 1;

   Eina_Bool                             highlight : 1;
   /* a flag whether genlist is marked as to be cleared or not. if
    * this flag is true, genlist clear was already deferred.  */
   Eina_Bool                             clear_me : 1;
   Eina_Bool                             h_bounce : 1;
   Eina_Bool                             v_bounce : 1;
   Eina_Bool                             bring_in : 1; /* a flag to
                                                        * describe the
                                                        * scroll
                                                        * animation. (show,
                                                        * bring in) */

   /* this is set to EINA_TRUE when the item is re-queued. this
    * happens when the item is un-queued but the rel item is still in
    * the queue. this item will be processed later. */
   Eina_Bool                             requeued : 1;
   Eina_Bool                             on_hold : 1;
   Eina_Bool                             multi : 1; /* a flag for item
                                                     * multi
                                                     * selection */

   Eina_Bool                             swipe : 1;
};

typedef struct _Item_Block Item_Block;
typedef struct _Item_Cache Item_Cache;

struct Elm_Gen_Item_Type
{
   Elm_Gen_Item           *it;

   Elm_Genlist_Smart_Data *wsd;

   Item_Block             *block;
   Eina_List              *items;
   Evas_Coord              w, h, minw, minh;
   Elm_Gen_Item           *group_item;
   Elm_Genlist_Item_Type   type;
   Eina_List              *deco_it_texts, *deco_it_contents,
                          *deco_it_states, *deco_it_content_objs;
   Eina_List              *deco_all_texts, *deco_all_contents,
                          *deco_all_states, *deco_all_content_objs;
   Eina_List              *flip_contents, *flip_content_objs;
   Ecore_Timer            *swipe_timer;
   Evas_Coord              scrl_x, scrl_y, old_scrl_y;

   Elm_Gen_Item           *rel;
   Evas_Object            *deco_it_view;
   int                     expanded_depth;
   int                     order_num_in;

   Eina_Bool               decorate_all_item_realized : 1;
   Eina_Bool               tree_effect_finished : 1; /* tree effect */
   Eina_Bool               move_effect_enabled : 1;
   Eina_Bool               tree_effect_hide_me : 1; /* item hide for
                                                    * tree effect */

   Eina_Bool               stacking_even : 1;
   Eina_Bool               want_realize : 1;
   Eina_Bool               nocache_once : 1; /* do not use cache for
                                              * this item only once */
   Eina_Bool               nostacking : 1;
   Eina_Bool               expanded : 1;
   Eina_Bool               mincalcd : 1;
   Eina_Bool               updateme : 1;
   Eina_Bool               nocache : 1; /* do not use cache for this item */
   Eina_Bool               queued : 1;
   Eina_Bool               before : 1;
   Eina_Bool               show_me : 1;
};

struct _Item_Block
{
   EINA_INLIST;

   int                     count;
   int                     num;
   int                     reorder_offset;
   Elm_Genlist_Smart_Data *sd;
   Eina_List              *items;
   Evas_Coord              x, y, w, h, minw, minh;
   int                     position;
   int                     item_position_stamp;

   Eina_Bool               position_update : 1;
   Eina_Bool               want_unrealize : 1;
   Eina_Bool               must_recalc : 1;
   Eina_Bool               realized : 1;
   Eina_Bool               updateme : 1;
   Eina_Bool               changed : 1;
   Eina_Bool               show_me : 1;
};

struct _Item_Cache
{
   EINA_INLIST;

   Evas_Object *base_view, *spacer;

   const char  *item_style; // it->itc->item_style

   Eina_Bool    selected : 1; // it->selected
   Eina_Bool    disabled : 1; // it->disabled
   Eina_Bool    expanded : 1; // it->item->expanded
   Eina_Bool    tree : 1; // it->group
};

typedef struct _Elm_Genlist_Pan_Smart_Class
{
   Elm_Pan_Smart_Class base;
} Elm_Genlist_Pan_Smart_Class;

typedef struct _Elm_Genlist_Pan_Smart_Data Elm_Genlist_Pan_Smart_Data;
struct _Elm_Genlist_Pan_Smart_Data
{
   Elm_Pan_Smart_Data      base;
   Elm_Genlist_Smart_Data *wsd;
   Ecore_Job              *resize_job;
};

/**
 * @}
 */

EAPI extern const char ELM_GENLIST_SMART_NAME[];
EAPI const Elm_Genlist_Smart_Class *elm_genlist_smart_class_get(void);

#define ELM_GENLIST_DATA_GET(o, sd) \
  Elm_Genlist_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_GENLIST_PAN_DATA_GET(o, sd) \
  Elm_Genlist_Pan_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_GENLIST_DATA_GET_OR_RETURN(o, ptr)       \
  ELM_GENLIST_DATA_GET(o, ptr);                      \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_GENLIST_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_GENLIST_DATA_GET(o, ptr);                         \
  if (!ptr)                                             \
    {                                                   \
       CRITICAL("No widget data for object %p (%s)",    \
                o, evas_object_type_get(o));            \
       return val;                                      \
    }

#define ELM_GENLIST_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_GENLIST_SMART_NAME, __func__)) \
    return

#define ELM_GENLIST_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_GENLIST_CHECK(it->base.widget);

#define ELM_GENLIST_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_GENLIST_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_GENLIST_ITEM_CHECK_OR_GOTO(it, label)              \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !elm_widget_type_check               \
        ((it->base.widget), ELM_GENLIST_SMART_NAME, __func__)) goto label;

#endif
