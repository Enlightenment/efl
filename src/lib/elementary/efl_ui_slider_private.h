#ifndef EFL_UI_SLIDER_PRIVATE_H
#define EFL_UI_SLIDER_PRIVATE_H

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
 * @section elm-slider-class The Elementary Slider Class
 *
 * Elementary, besides having the @ref Slider widget, exposes its
 * foundation -- the Elementary Slider Class -- in order to create other
 * widgets which are a slider with some more logic on top.
 */

/**
 * Base layout smart data extended with slider instance data.
 */
typedef struct _Efl_Ui_Slider_Data Efl_Ui_Slider_Data;
struct _Efl_Ui_Slider_Data
{
   Evas_Object          *spacer, *popup, *popup2, *track, *track2;
   Ecore_Timer          *delay;

   const char           *units;
   const char           *indicator;

   char                 *(*indicator_format_func)(double val);
   void                  (*indicator_format_free)(char *str);

   char                 *(*units_format_func)(double val);
   void                  (*units_format_free)(char *str);

   double                val, val_min, val_max, val2, step;

   Ecore_Timer           *wheel_indicator_timer;
   double                wheel_indicator_duration;
   Elm_Slider_Indicator_Visible_Mode indicator_visible_mode; /**< indicator_visible_mode of the slider.
                                                                This indicates when to show an indicator */

   Evas_Coord            size;
   Evas_Coord            downx, downy;
   Efl_Orient            orientation;

   double                intvl_from, intvl_to;

   Eina_Bool             indicator_show : 1;
   Eina_Bool             spacer_down : 1;
   Eina_Bool             frozen : 1;
   Eina_Bool             units_show : 1;
   Eina_Bool             popup_visible : 1;
   Eina_Bool             intvl_enable : 1;
};

/**
 * @}
 */

#define ELM_SLIDER_DATA_GET(o, sd) \
  Efl_Ui_Slider_Data * sd = efl_data_scope_get(o, EFL_UI_SLIDER_CLASS)

#define ELM_SLIDER_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_SLIDER_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_SLIDER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SLIDER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       CRI("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_SLIDER_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_SLIDER_CLASS))) \
    return

#endif
