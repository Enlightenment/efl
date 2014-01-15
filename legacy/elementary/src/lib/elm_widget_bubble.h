#ifndef ELM_WIDGET_BUBBLE_H
#define ELM_WIDGET_BUBBLE_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-bubble-class The Elementary Bubble Class
 *
 * Elementary, besides having the @ref Bubble widget, exposes its
 * foundation -- the Elementary Bubble Class -- in order to create other
 * widgets which are a bubble with some more logic on top.
 */


/**
 * Base layout smart data extended with bubble instance data.
 */
typedef struct _Elm_Bubble_Smart_Data Elm_Bubble_Smart_Data;
struct _Elm_Bubble_Smart_Data
{
   Elm_Bubble_Pos        pos;
};

/**
 * @}
 */

#define ELM_BUBBLE_DATA_GET(o, sd) \
  Elm_Bubble_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_BUBBLE_CLASS)

#define ELM_BUBBLE_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_BUBBLE_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_BUBBLE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_BUBBLE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_BUBBLE_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_BUBBLE_CLASS))) \
    return

#endif
