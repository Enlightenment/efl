#ifndef ELM_WIDGET_NOTIFY_H
#define ELM_WIDGET_NOTIFY_H

#include "Elementary.h"

#include <elm_notify.eo.h>

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-notify-class The Elementary Notify Class
 *
 * Elementary, besides having the @ref Notify widget, exposes its
 * foundation -- the Elementary Notify Class -- in order to create other
 * widgets which are a notify with some more logic on top.
 */


/**
 * Base widget smart data extended with notify instance data.
 */
typedef struct _Elm_Notify_Data Elm_Notify_Data;
struct _Elm_Notify_Data
{
   Evas_Object             *notify, *content, *parent;
   Evas_Object             *block_events;
   double                   timeout;
   double                   horizontal_align, vertical_align;
   Ecore_Timer             *timer;

   Eina_Bool                allow_events : 1;
   Eina_Bool                had_hidden : 1;
   Eina_Bool                in_timeout : 1;
};

/**
 * @}
 */

#define ELM_NOTIFY_DATA_GET(o, sd) \
  Elm_Notify_Data * sd = efl_data_scope_get(o, ELM_NOTIFY_CLASS)

#define ELM_NOTIFY_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_NOTIFY_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_NOTIFY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_NOTIFY_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       ERR("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_NOTIFY_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_NOTIFY_CLASS))) \
    return

#endif
