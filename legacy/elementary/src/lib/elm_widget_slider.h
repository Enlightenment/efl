#ifndef ELM_WIDGET_SLIDER_H
#define ELM_WIDGET_SLIDER_H

#include "Elementary.h"

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
typedef struct _Elm_Slider_Smart_Data Elm_Slider_Smart_Data;
struct _Elm_Slider_Smart_Data
{
   Evas_Object          *spacer, *popup, *track;
   Ecore_Timer          *delay;

   const char           *units;
   const char           *indicator;

   char                 *(*indicator_format_func)(double val);
   void                  (*indicator_format_free)(char *str);

   char                 *(*units_format_func)(double val);
   void                  (*units_format_free)(char *str);

   double                val, val_min, val_max, val2, step;
   Evas_Coord            size;
   Evas_Coord            downx, downy;

   Eina_Bool             horizontal : 1;
   Eina_Bool             inverted : 1;
   Eina_Bool             indicator_show : 1;
   Eina_Bool             spacer_down : 1;
   Eina_Bool             frozen : 1;
   Eina_Bool             units_show : 1;
   Eina_Bool             always_popup_show : 1; /*this flag is to set the visiblity of slider indicator
                                                * if this flag is set  then the indicator always show
                                                */
};

/**
 * @}
 */

#define ELM_SLIDER_DATA_GET(o, sd) \
  Elm_Slider_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_SLIDER_CLASS)

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
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_SLIDER_CLASS))) \
    return

#endif
