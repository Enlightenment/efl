#ifndef ELM_WIDGET_CHECK_H
#define ELM_WIDGET_CHECK_H

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
 * @section elm-check-class The Elementary Check Class
 *
 * Elementary, besides having the @ref Check widget, exposes its
 * foundation -- the Elementary Check Class -- in order to create other
 * widgets which are a check with some more logic on top.
 */

/**
 * Base layout smart data extended with check instance data.
 */
typedef struct _Elm_Check_Data Elm_Check_Data;
struct _Elm_Check_Data
{
   Eina_Bool             state;
   Eina_Bool            *statep;
};

/**
 * @}
 */

#define ELM_CHECK_DATA_GET(o, sd) \
  Elm_Check_Data * sd = eo_data_scope_get(o, ELM_CHECK_CLASS)

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
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_CHECK_CLASS))) \
    return

#endif
