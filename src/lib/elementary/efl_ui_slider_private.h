#ifndef EFL_UI_SLIDER_PRIVATE_H
#define EFL_UI_SLIDER_PRIVATE_H

#include "Elementary.h"

typedef struct _Efl_Ui_Slider_Data Efl_Ui_Slider_Data;
struct _Efl_Ui_Slider_Data
{
   Evas_Object          *spacer;
   Ecore_Timer          *delay;

   double                val, val_min, val_max, step;

   Evas_Coord            downx, downy;
   Efl_Ui_Dir            dir;

   Eina_Bool             spacer_down : 1;
   Eina_Bool             frozen : 1;
};

/**
 * @}
 */

# define EFL_UI_SLIDER_VAL_FETCH_OPS(_pfx) \
   EFL_OBJECT_OP_FUNC(efl_ui_slider_val_fetch, _##_pfx##_val_fetch)

# define EFL_UI_SLIDER_VAL_SET_OPS(_pfx) \
   EFL_OBJECT_OP_FUNC(efl_ui_slider_val_set, _##_pfx##_val_set)

# define EFL_UI_SLIDER_DOWN_KNOB_OPS(_pfx) \
   EFL_OBJECT_OP_FUNC(efl_ui_slider_down_knob, _##_pfx##_down_knob)

# define EFL_UI_SLIDER_MOVE_KNOB_OPS(_pfx) \
   EFL_OBJECT_OP_FUNC(efl_ui_slider_move_knob, _##_pfx##_move_knob)

#define EFL_UI_SLIDER_DATA_GET(o, sd) \
  Efl_Ui_Slider_Data * sd = efl_data_scope_get(o, EFL_UI_SLIDER_CLASS)

#define EFL_UI_SLIDER_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Slider_Data * sd = efl_data_scope_safe_get(o, EFL_UI_SLIDER_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }



#endif
