#ifndef ELM_WIDGET_SCROLLER_H
#define ELM_WIDGET_SCROLLER_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-scroller-class The Elementary Scroller Class
 *
 * Elementary, besides having the @ref Scroller widget, exposes its
 * foundation -- the Elementary Scroller Class -- in order to create
 * other widgets which are a scroller with some more logic on top.
 */

/**
 * Base widget smart data extended with scroller instance data.
 */
typedef struct _Elm_Scroller_Smart_Data Elm_Scroller_Smart_Data;
struct _Elm_Scroller_Smart_Data
{
   Evas_Object                          *hit_rect;
   Evas_Object                          *g_layer;

   Evas_Object                          *content;

   Eina_Bool                             min_w : 1;
   Eina_Bool                             min_h : 1;
};

/**
 * @}
 */

#define ELM_SCROLLER_DATA_GET(o, sd) \
  Elm_Scroller_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_SCROLLER_CLASS)

#define ELM_SCROLLER_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_SCROLLER_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_SCROLLER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SCROLLER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       CRI("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_SCROLLER_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_SCROLLER_CLASS))) \
    return

#endif
