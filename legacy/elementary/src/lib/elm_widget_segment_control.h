#ifndef ELM_WIDGET_SEGMENT_CONTROL_H
#define ELM_WIDGET_SEGMENT_CONTROL_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-segment-control-class The Elementary Segment control Class
 *
 * Elementary, besides having the @ref SegmentControl widget, exposes
 * its foundation -- the Elementary Segment control Class -- in order
 * to create other widgets which are a segment control with some more
 * logic on top.
 */
/**
 * Base layout smart data extended with segment control instance data.
 */
typedef struct _Elm_Segment_Control_Smart_Data Elm_Segment_Control_Smart_Data;
typedef struct _Elm_Segment_Item               Elm_Segment_Item;

struct _Elm_Segment_Control_Smart_Data
{
   Evas_Object          *obj;
   Eina_List            *items;
   Elm_Segment_Item     *selected_item;

   int                   item_width;
};

struct _Elm_Segment_Item
{
   ELM_WIDGET_ITEM;

   Evas_Object *icon;
   const char  *label;
   int          seg_index;
};

/**
 * @}
 */

#define ELM_SEGMENT_CONTROL_DATA_GET(o, sd) \
  Elm_Segment_Control_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_SEGMENT_CONTROL_CLASS)

#define ELM_SEGMENT_CONTROL_DATA_GET_OR_RETURN(o, ptr) \
  ELM_SEGMENT_CONTROL_DATA_GET(o, ptr);                \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return;                                         \
    }

#define ELM_SEGMENT_CONTROL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SEGMENT_CONTROL_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                      \
    {                                                           \
       CRI("No widget data for object %p (%s)",                 \
           o, evas_object_type_get(o));                         \
       return val;                                              \
    }

#define ELM_SEGMENT_CONTROL_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_SEGMENT_CONTROL_CLASS))) \
    return

#define ELM_SEGMENT_CONTROL_ITEM_CHECK(it)                  \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_SEGMENT_CONTROL_CHECK(it->base.widget);

#define ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, ...)              \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_SEGMENT_CONTROL_CHECK(it->base.widget) __VA_ARGS__;

#endif
