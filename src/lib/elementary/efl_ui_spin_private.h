#ifndef EFL_UI_SPIN_PRIVATE_H
#define EFL_UI_SPIN_PRIVATE_H

#include "Elementary.h"

typedef enum _Efl_Ui_Spin_Format_Type
{
   SPIN_FORMAT_FLOAT,
   SPIN_FORMAT_INT,
   SPIN_FORMAT_INVALID
} Efl_Ui_Spin_Format_Type;

typedef struct _Efl_Ui_Spin_Data    Efl_Ui_Spin_Data;
struct _Efl_Ui_Spin_Data
{
   const char           *templates;
   double                val, val_min, val_max;
   double                step; /**< step for the value change. 1 by default. */
   int                   decimal_points;
   Ecore_Timer          *spin_timer; /**< a timer for a repeated spin value change on mouse down */
   Efl_Ui_Spin_Format_Type format_type;

   Efl_Ui_Format_Func_Cb format_cb;
   Eina_Free_Cb          format_free_cb;
   void                  *format_cb_data;
   Eina_Strbuf           *format_strbuf;

   Eina_Array            *special_values;
};

#endif
