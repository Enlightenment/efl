#ifndef ELM_WIDGET_PANES_H
#define ELM_WIDGET_PANES_H

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
 * @section elm-panes-class The Elementary Panes Class
 *
 * Elementary, besides having the @ref Panes widget, exposes its
 * foundation -- the Elementary Panes Class -- in order to create other
 * widgets which are a panes with some more logic on top.
 */

/**
 * Base layout smart data extended with panes instance data.
 */
typedef struct _Efl_Ui_Panes_Data Efl_Ui_Panes_Data;
struct _Efl_Ui_Panes_Data
{
   Evas_Object *event;
   struct
   {
      int       x_diff;
      int       y_diff;
      Eina_Bool move;
   } move;

   Efl_Ui_Dir            dir;
   double                left_min_relative_size;
   double                right_min_relative_size;
   Eina_Size2D           first_min, second_min;
   double                first_min_split_ratio, second_min_split_ratio;

   Evas_Coord            left_min_size;
   Evas_Coord            right_min_size;
   Eina_Bool             double_clicked : 1;
   Eina_Bool             fixed : 1;
   Eina_Bool             left_min_size_is_relative : 1;
   Eina_Bool             right_min_size_is_relative : 1;
   Eina_Bool             first_hint_min_allow : 1;
   Eina_Bool             second_hint_min_allow : 1;
};

/**
 * @}
 */

#define EFL_UI_PANES_DATA_GET(o, sd) \
  Efl_Ui_Panes_Data * sd = efl_data_scope_get(o, EFL_UI_PANES_CLASS)

#define EFL_UI_PANES_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_PANES_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_PANES_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_PANES_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define EFL_UI_PANES_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_PANES_CLASS))) \
    return

#endif
