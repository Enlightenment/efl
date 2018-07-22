#ifndef ELM_WIDGET_INWIN_H
#define ELM_WIDGET_INWIN_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
  Elm_Inwin_Smart_Data * sd = efl_data_scope_get(o, ELM_INWIN_CLASS)

#define ELM_INWIN_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_INWIN_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_INWIN_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_INWIN_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_INWIN_CHECK(obj)                                  \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_INWIN_CLASS))) \
    return

#endif
