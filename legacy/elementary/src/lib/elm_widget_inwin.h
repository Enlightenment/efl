#ifndef ELM_WIDGET_INWIN_H
#define ELM_WIDGET_INWIN_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-inwin-class The Elementary Inwin Class
 *
 * Elementary, besides having the @ref Inwin widget, exposes its
 * foundation -- the Elementary Inwin Class -- in order to create other
 * widgets which are a inwin with some more logic on top.
 */

/**
 * Base layout smart data extended with inwin instance data.
 */
typedef struct _Elm_Inwin_Smart_Data Elm_Inwin_Smart_Data;
struct _Elm_Inwin_Smart_Data
{
};

/**
 * @}
 */

#define ELM_INWIN_DATA_GET(o, sd) \
  Elm_Inwin_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_WIN_INWIN_CLASS)

#define ELM_INWIN_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_INWIN_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_INWIN_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_INWIN_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_INWIN_CHECK(obj)                                  \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_WIN_INWIN_CLASS))) \
    return

#endif
