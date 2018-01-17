#ifndef EFL_UI_PROGRESSBAR_PRIVATE_H
#define EFL_UI_PROGRESSBAR_PRIVATE_H

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
typedef struct _Efl_Ui_Progressbar_Data Efl_Ui_Progressbar_Data;
typedef struct _Efl_Ui_Progress_Status Efl_Ui_Progress_Status;

struct _Efl_Ui_Progressbar_Data
{
   Evas_Object          *spacer; /**< The rect actual progressbar area, gets the progressbar size and gets the events */

   Evas_Coord            size; /**< Width or height of progressbar */
   double                val; /**< Value of progressbar */
   
   double                val_min;
   double                val_max;

   Eina_List            *progress_status; /**< The list of _Elm_Progress_Status. To save the progress value(in percentage) each part of given progress bar */

   Efl_Ui_Format_Func_Cb format_cb;
   Eina_Free_Cb          format_free_cb;
   void                  *format_cb_data;
   Eina_Strbuf           *format_strbuf;

   Efl_Ui_Dir            dir; /**< Orientation of the progressbar  */

   Eina_Bool             pulse : 1; /**< Whether object is put in the pulsing mode */
   Eina_Bool             pulse_state : 1; /**< To start the pulsing animation, otherwise to stop it */
   Eina_Bool             is_legacy_format_string : 1;
   Eina_Bool             is_legacy_format_cb : 1;
};

struct _Efl_Ui_Progress_Status
{
   const char           *part_name;
   double                val;
   double                val_min, val_max;
};

/**
 * @}
 */

#define EFL_UI_PROGRESSBAR_DATA_GET(o, sd) \
  Efl_Ui_Progressbar_Data * sd = efl_data_scope_get(o, EFL_UI_PROGRESSBAR_CLASS)

#define EFL_UI_PROGRESSBAR_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Progressbar_Data *sd = efl_data_scope_safe_get(o, EFL_UI_PROGRESSBAR_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#endif
