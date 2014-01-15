#ifndef ELM_WIDGET_CHECK_H
#define ELM_WIDGET_CHECK_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-check-class The Elementary Check Class
 *
 * Elementary, besides having the @ref Check widget, exposes its
 * foundation -- the Elementary Check Class -- in order to create other
 * widgets which are a check with some more logic on top.
 */

/**
 * Base layout smart data extended with check instance data.
 */
typedef struct _Elm_Check_Smart_Data Elm_Check_Smart_Data;
struct _Elm_Check_Smart_Data
{
   Eina_Bool             state;
   Eina_Bool            *statep;
};

/**
 * @}
 */

#define ELM_CHECK_DATA_GET(o, sd) \
  Elm_Check_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_CHECK_CLASS)

#define ELM_CHECK_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_CHECK_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_CHECK_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CHECK_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_CHECK_CHECK(obj)                       \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_CHECK_CLASS))) \
    return

#endif
