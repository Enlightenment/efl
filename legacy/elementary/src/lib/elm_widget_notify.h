#ifndef ELM_WIDGET_NOTIFY_H
#define ELM_WIDGET_NOTIFY_H

#include "Elementary.h"

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
typedef struct _Elm_Notify_Smart_Data Elm_Notify_Smart_Data;
struct _Elm_Notify_Smart_Data
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
  Elm_Notify_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_NOTIFY_CLASS)

#define ELM_NOTIFY_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_NOTIFY_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_NOTIFY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_NOTIFY_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_NOTIFY_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_NOTIFY_CLASS))) \
    return

#endif
