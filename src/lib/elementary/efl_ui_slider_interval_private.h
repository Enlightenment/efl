#ifndef EFL_UI_SLIDER_INTERVAL_PRIVATE_H
#define EFL_UI_SLIDER_INTERVAL_PRIVATE_H

#include "Elementary.h"
typedef struct _Efl_Ui_Slider_Interval_Data Efl_Ui_Slider_Interval_Data;
struct _Efl_Ui_Slider_Interval_Data
{
   Ecore_Timer          *delay;
   double                intvl_from, intvl_to;
   int                   intvl_flag;
};

/**
 * @}
 */

#define EFL_UI_SLIDER_INTERVAL_DATA_GET(o, sd) \
  Efl_Ui_Slider_Interval_Data * sd = efl_data_scope_get(o, EFL_UI_SLIDER_INTERVAL_CLASS)

#define EFL_UI_SLIDER_INTERVAL_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Slider_Interval_Data * sd = efl_data_scope_safe_get(o, EFL_UI_SLIDER_INTERVAL_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#endif
