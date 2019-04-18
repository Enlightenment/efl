#ifndef ELM_WIDGET_CLOCK_H
#define ELM_WIDGET_CLOCK_H

#include "Elementary.h"

#include <Eio.h>

#include "elm_clock_eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-clock-class The Elementary Clock Class
 *
 * Elementary, besides having the @ref Clock widget, exposes its
 * foundation -- the Elementary Clock Class -- in order to create other
 * widgets which are a clock with some more logic on top.
 */

/**
 * Base layout smart data extended with clock instance data.
 */
typedef struct _Elm_Clock_Data Elm_Clock_Data;
struct _Elm_Clock_Data
{
   double                interval, first_interval;
   Elm_Clock_Edit_Mode   digedit;
   int                   hrs, min, sec, timediff;
   Evas_Object          *digit[6];
   Evas_Object          *am_pm_obj;
   Evas_Object          *sel_obj;
   Ecore_Timer          *ticker, *spin;

   struct
   {
      int                 hrs, min, sec;
      char                ampm;
      Elm_Clock_Edit_Mode digedit;

      Eina_Bool           seconds : 1;
      Eina_Bool           am_pm : 1;
      Eina_Bool           edit : 1;
   } cur;

   Eina_Bool paused : 1; /**< a flag whether clock is paused or not */
   Eina_Bool seconds : 1;
   Eina_Bool am_pm : 1;
   Eina_Bool edit : 1;
};

/**
 * @}
 */

#define ELM_CLOCK_DATA_GET(o, sd) \
  Elm_Clock_Data * sd = efl_data_scope_get(o, ELM_CLOCK_CLASS)

#define ELM_CLOCK_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_CLOCK_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_CLOCK_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CLOCK_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_CLOCK_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_CLOCK_CLASS))) \
    return

#endif
