#ifndef ELM_WIDGET_DISKSELECTOR_H
#define ELM_WIDGET_DISKSELECTOR_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-diskselector-class The Elementary Diskselector Class
 *
 * Elementary, besides having the @ref Diskselector widget, exposes its
 * foundation -- the Elementary Diskselector Class -- in order to create other
 * widgets which are a diskselector with some more logic on top.
 */

/**
 * Base widget smart data extended with diskselector instance data.
 */
typedef struct _Elm_Diskselector_Smart_Data Elm_Diskselector_Smart_Data;
typedef struct _Elm_Diskselector_Item       Elm_Diskselector_Item;

struct _Elm_Diskselector_Smart_Data
{
   Evas_Object                          *hit_rect;

   Evas_Object                          *main_box;
   Elm_Diskselector_Item                *selected_item;
   Elm_Diskselector_Item                *first;
   Elm_Diskselector_Item                *second;
   Elm_Diskselector_Item                *s_last;
   Elm_Diskselector_Item                *last;
   Eina_List                            *items;
   Eina_List                            *r_items;
   Eina_List                            *over_items;
   Eina_List                            *under_items;
   Eina_List                            *left_blanks;
   Eina_List                            *right_blanks;
   Ecore_Idle_Enterer                   *scroller_move_idle_enterer; // idle enterer for scroller move
   Ecore_Idle_Enterer                   *string_check_idle_enterer; // idle enterer for string check

   int                                   item_count, len_threshold, len_side,
                                         display_item_num;
   Evas_Coord                            minw, minh;

   Eina_Bool                             init : 1;
   Eina_Bool                             round : 1;
   Eina_Bool                             display_item_num_by_api : 1;
   Eina_Bool                             left_boundary_reached:1;
   Eina_Bool                             right_boundary_reached:1;
};

struct _Elm_Diskselector_Item
{
   ELM_WIDGET_ITEM;

   Eina_List    *node;
   Evas_Object  *icon;
   const char   *label;
   Evas_Smart_Cb func;
};

/**
 * @}
 */

#define ELM_DISKSELECTOR_DATA_GET(o, sd) \
  Elm_Diskselector_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_DISKSELECTOR_CLASS)

#define ELM_DISKSELECTOR_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_DISKSELECTOR_DATA_GET(o, ptr);                 \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_DISKSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_DISKSELECTOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                   \
    {                                                        \
       CRI("No widget data for object %p (%s)",              \
           o, evas_object_type_get(o));                      \
       return val;                                           \
    }

#define ELM_DISKSELECTOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_DISKSELECTOR_CLASS))) \
    return

#define ELM_DISKSELECTOR_ITEM_CHECK(it)                     \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_DISKSELECTOR_CHECK(it->base.widget);

#define ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)                 \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_DISKSELECTOR_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_DISKSELECTOR_ITEM_CHECK_OR_GOTO(it, label)         \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !eo_isa((it->base.widget), ELM_OBJ_DISKSELECTOR_CLASS)) \
           goto label;

#endif
