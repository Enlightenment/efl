#ifndef ELM_WIDGET_PLUG_H
#define ELM_WIDGET_PLUG_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-plug-class The Elementary Plug Class
 *
 * Elementary, besides having the @ref Plug widget, exposes its
 * foundation -- the Elementary Plug Class -- in order to create other
 * widgets which are a plug with some more logic on top.
 */
/**
 * @}
 */

#define ELM_PLUG_DATA_GET(o, sd) \
     Elm_Plug_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_PLUG_CLASS)

#define ELM_PLUG_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_PLUG_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PLUG_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PLUG_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_PLUG_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_PLUG_CLASS))) \
    return

#endif
