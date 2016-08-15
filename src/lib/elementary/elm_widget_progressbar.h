#ifndef ELM_WIDGET_PROGRESSBAR_H
#define ELM_WIDGET_PROGRESSBAR_H

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
 * @section elm-progressbar-class The Elementary Progressbar Class
 *
 * Elementary, besides having the @ref Progressbar widget, exposes its
 * foundation -- the Elementary Progressbar Class -- in order to create other
 * widgets which are a progressbar with some more logic on top.
 */

/**
 * Base layout smart data extended with progressbar instance data.
 */
typedef struct _Elm_Progressbar_Data Elm_Progressbar_Data;
typedef struct _Elm_Progress_Status Elm_Progress_Status;

struct _Elm_Progressbar_Data
{
   Evas_Object          *spacer; /**< The rect actual progressbar area, gets the progressbar size and gets the events */
   const char           *units; /**< The units will be displayed on progressbar */

   Evas_Coord            size; /**< Width or height of progressbar */
   double                val; /**< Value of progressbar */

   Efl_Orient            orientation; /**< Orientation of the progressbar  */
   Eina_Bool             pulse : 1; /**< Whether object is put in the pulsing mode */
   Eina_Bool             pulse_state : 1; /**< To start the pulsing animation, otherwise to stop it */
   Eina_List            *progress_status; /**< The list of _Elm_Progress_Status. To save the progress value(in percentage) each part of given progress bar */

   char                 *(*unit_format_func)(double val); /**< The function pointer will be called when user wants to set a customized unit format */
   void                  (*unit_format_free)(char *str); /**< The freeing function for the format string */
};

struct _Elm_Progress_Status
{
   const char           *part_name;
   double                val;
};

/**
 * @}
 */

#define ELM_PROGRESSBAR_DATA_GET(o, sd) \
  Elm_Progressbar_Data * sd = efl_data_scope_get(o, ELM_PROGRESSBAR_CLASS)

#define ELM_PROGRESSBAR_DATA_GET_OR_RETURN(o, ptr)   \
  ELM_PROGRESSBAR_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PROGRESSBAR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PROGRESSBAR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                  \
    {                                                       \
       CRI("No widget data for object %p (%s)",             \
           o, evas_object_type_get(o));                     \
       return val;                                          \
    }

#define ELM_PROGRESSBAR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_PROGRESSBAR_CLASS))) \
    return

#endif
