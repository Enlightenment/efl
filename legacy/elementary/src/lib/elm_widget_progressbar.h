#ifndef ELM_WIDGET_PROGRESSBAR_H
#define ELM_WIDGET_PROGRESSBAR_H

#include "Elementary.h"

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
typedef struct _Elm_Progressbar_Smart_Data Elm_Progressbar_Smart_Data;
typedef struct _Elm_Progress_Status Elm_Progress_Status;

struct _Elm_Progressbar_Smart_Data
{
   Evas_Object          *spacer;
   const char           *units;

   Evas_Coord            size;
   double                val;

   Eina_Bool             horizontal : 1;
   Eina_Bool             inverted : 1;
   Eina_Bool             pulse : 1;
   Eina_Bool             pulse_state : 1;
   Eina_List            *progress_status;

   char                 *(*unit_format_func)(double val);
   void                  (*unit_format_free)(char *str);
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
  Elm_Progressbar_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_PROGRESSBAR_CLASS)

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
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_PROGRESSBAR_CLASS))) \
    return

#endif
