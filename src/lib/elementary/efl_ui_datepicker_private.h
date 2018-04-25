#ifndef EFL_UI_DATEPICKER_PRIVATE_H
#define EFL_UI_DATEPICKER_PRIVATE_H

#define EFL_UI_DATEPICKER_TYPE_COUNT 3

typedef enum _Efl_Ui_Datepicker_Field_Type
{
   DATEPICKER_YEAR,
   DATEPICKER_MONTH,
   DATEPICKER_DAY
} Efl_Ui_Datepicker_Field_Type;

typedef struct _Efl_Ui_Datepicker_Data    Efl_Ui_Datepicker_Data;
struct _Efl_Ui_Datepicker_Data
{
   Eo *dt_manager, *year, *month, *day;
   int cur_date[EFL_UI_DATEPICKER_TYPE_COUNT],
       min_date[EFL_UI_DATEPICKER_TYPE_COUNT],
       max_date[EFL_UI_DATEPICKER_TYPE_COUNT];
};

#endif
