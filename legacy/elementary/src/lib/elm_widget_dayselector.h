#ifndef ELM_WIDGET_DAYSELECTOR_H
#define ELM_WIDGET_DAYSELECTOR_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-dayselector-class The Elementary Dayselector Class
 *
 * Elementary, besides having the @ref Dayselector widget, exposes its
 * foundation -- the Elementary Dayselector Class -- in order to create other
 * widgets which are a dayselector with some more logic on top.
 */

/**
 * Base layout smart data extended with dayselector instance data.
 */
typedef struct _Elm_Dayselector_Smart_Data Elm_Dayselector_Smart_Data;
struct _Elm_Dayselector_Smart_Data
{
   Eina_List            *items;
   Elm_Dayselector_Day   week_start;
   Elm_Dayselector_Day   weekend_start;
   unsigned int          weekend_len;
   Eina_Bool weekdays_names_set : 1;
};

typedef struct _Elm_Dayselector_Item       Elm_Dayselector_Item;
struct _Elm_Dayselector_Item
{
   ELM_WIDGET_ITEM;
   Elm_Dayselector_Day day;
   const char         *day_style;
};

/**
 * @}
 */

#define ELM_DAYSELECTOR_DATA_GET(o, sd) \
  Elm_Dayselector_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_DAYSELECTOR_CLASS)

#define ELM_DAYSELECTOR_DATA_GET_OR_RETURN(o, ptr)   \
  ELM_DAYSELECTOR_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_DAYSELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_DAYSELECTOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                  \
    {                                                       \
       CRI("No widget data for object %p (%s)",             \
           o, evas_object_type_get(o));                     \
       return val;                                          \
    }

#define ELM_DAYSELECTOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_DAYSELECTOR_CLASS))) \
    return

#endif
