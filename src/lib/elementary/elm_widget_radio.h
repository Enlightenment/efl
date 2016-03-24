#ifndef ELM_WIDGET_RADIO_H
#define ELM_WIDGET_RADIO_H

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
 * @section elm-radio-class The Elementary Radio Class
 *
 * Elementary, besides having the @ref Radio widget, exposes its
 * foundation -- the Elementary Radio Class -- in order to create other
 * widgets which are a radio with some more logic on top.
 */

/**
 * Base layout smart data extended with radio instance data.
 */
typedef struct _Elm_Radio_Data       Elm_Radio_Data;
typedef struct _Group                Group;

struct _Group
{
   int        value;
   int       *valuep;
   Eina_List *radios;
};

struct _Elm_Radio_Data
{
   int                   value;
   Eina_Bool             state;

   Group                *group;
};

/**
 * @}
 */

#define ELM_RADIO_DATA_GET(o, sd) \
  Elm_Radio_Data * sd = eo_data_scope_get(o, ELM_RADIO_CLASS)

#define ELM_RADIO_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_RADIO_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_RADIO_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_RADIO_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_RADIO_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_RADIO_CLASS))) \
    return

#endif
