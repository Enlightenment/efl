#ifndef ELM_WIDGET_INDEX_H
#define ELM_WIDGET_INDEX_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-index-class The Elementary Index Class
 *
 * Elementary, besides having the @ref Index widget, exposes its
 * foundation -- the Elementary Index Class -- in order to create other
 * widgets which are a index with some more logic on top.
 */

/**
 * Base layout smart data extended with index instance data.
 */
typedef struct _Elm_Index_Smart_Data Elm_Index_Smart_Data;
struct _Elm_Index_Smart_Data
{
   Evas_Object          *event[2];
   Evas_Object          *bx[2]; // 2 - for now all that's supported
   Eina_List            *items;  /* 1 list. N levels, but only 2
                                  * for now and # of items will be
                                  * small */
   int                   level;
   Evas_Coord            dx, dy;
   Ecore_Timer          *delay;
   Eina_Bool             level_active[2];

   Eina_Bool             down : 1;
   Eina_Bool             horizontal : 1;
   Eina_Bool             autohide_disabled : 1;
   Eina_Bool             indicator_disabled : 1;
};

typedef struct _Elm_Index_Item       Elm_Index_Item;
struct _Elm_Index_Item
{
   ELM_WIDGET_ITEM;

   const char   *letter;
   int           level;
   Evas_Smart_Cb func;

   Eina_Bool     selected : 1;
};

/**
 * @}
 */
#define ELM_INDEX_DATA_GET(o, sd) \
  Elm_Index_Smart_Data * sd = eo_data_get(o, ELM_OBJ_INDEX_CLASS)

#define ELM_INDEX_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_INDEX_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_INDEX_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_INDEX_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_INDEX_CHECK(obj)                                                 \
  if (!eo_isa((obj), ELM_OBJ_INDEX_CLASS)) \
    return

#define ELM_INDEX_ITEM_CHECK(it)                            \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_INDEX_CHECK(it->base.widget);

#define ELM_INDEX_ITEM_CHECK_OR_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_INDEX_CHECK(it->base.widget) __VA_ARGS__;

#endif
