#ifndef ELM_WIDGET_FLIPSELECTOR_H
#define ELM_WIDGET_FLIPSELECTOR_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-flipselector-class The Elementary Flipselector Class
 *
 * Elementary, besides having the @ref Flipselector widget, exposes its
 * foundation -- the Elementary Flipselector Class -- in order to create other
 * widgets which are a flipselector with some more logic on top.
 */

/**
 * Base layout smart data extended with flipselector instance data.
 */
typedef struct _Elm_Flipselector_Smart_Data Elm_Flipselector_Smart_Data;
struct _Elm_Flipselector_Smart_Data
{
   Evas_Object          *obj;
   Eina_List            *items;
   Eina_List            *current;
   Eina_List            *sentinel; /* item containing the largest
                                    * label string */
   Ecore_Timer          *spin;

   unsigned int          max_len;
   double                interval, first_interval;

   int                   walking;
   Eina_Bool             evaluating : 1;
   Eina_Bool             deleting : 1;
};

typedef struct _Elm_Flipselector_Item       Elm_Flipselector_Item;
struct _Elm_Flipselector_Item
{
   ELM_WIDGET_ITEM;

   const char   *label;
   Evas_Smart_Cb func;
   int           deleted : 1;
};

#define ELM_FLIPSELECTOR_DATA_GET(o, sd) \
  Elm_Flipselector_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_FLIPSELECTOR_CLASS)

#define ELM_FLIPSELECTOR_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_FLIPSELECTOR_DATA_GET(o, ptr);                 \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_FLIPSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FLIPSELECTOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                   \
    {                                                        \
       CRI("No widget data for object %p (%s)",              \
           o, evas_object_type_get(o));                      \
       return val;                                           \
    }

#define ELM_FLIPSELECTOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_FLIPSELECTOR_CLASS))) \
    return

#define ELM_FLIPSELECTOR_ITEM_CHECK(it)                     \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_FLIPSELECTOR_CHECK(it->base.widget);

#define ELM_FLIPSELECTOR_ITEM_CHECK_OR_RETURN(it, ...)                 \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_FLIPSELECTOR_CHECK(it->base.widget) __VA_ARGS__;

#endif
