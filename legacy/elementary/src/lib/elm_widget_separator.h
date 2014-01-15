#ifndef ELM_WIDGET_SEPARATOR_H
#define ELM_WIDGET_SEPARATOR_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-separator-class The Elementary Separator Class
 *
 * Elementary, besides having the @ref Separator widget, exposes its
 * foundation -- the Elementary Separator Class -- in order to create other
 * widgets which are a separator with some more logic on top.
 */

/**
 * Base layout smart data extended with separator instance data.
 */
typedef struct _Elm_Separator_Smart_Data Elm_Separator_Smart_Data;
struct _Elm_Separator_Smart_Data
{
   Eina_Bool             horizontal : 1;
};

/**
 * @}
 */

#define ELM_SEPARATOR_DATA_GET(o, sd) \
  Elm_Separator_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_SEPARATOR_CLASS)

#define ELM_SEPARATOR_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_SEPARATOR_DATA_GET(o, ptr);                    \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_SEPARATOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SEPARATOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                \
    {                                                     \
       CRI("No widget data for object %p (%s)",           \
           o, evas_object_type_get(o));                   \
       return val;                                        \
    }

#define ELM_SEPARATOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_SEPARATOR_CLASS))) \
    return

#endif
