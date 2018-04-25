#ifndef EFL_UI_TIMEPICKER_PRIVATE_H
#define EFL_UI_TIMEPICKER_PRIVATE_H

#define EFL_UI_TIMEPICKER_TYPE_COUNT 3

typedef enum _Efl_Ui_Timepicker_Field_Type
{
   TIMEPICKER_HOUR,
   TIMEPICKER_MIN,
   TIMEPICKER_AMPM
} Efl_Ui_Timepicker_Field_Type;

typedef struct _Efl_Ui_Timepicker_Data    Efl_Ui_Timepicker_Data;
struct _Efl_Ui_Timepicker_Data
{
   Eo *dt_manager, *hour, *min, *ampm;
   int cur_time[EFL_UI_TIMEPICKER_TYPE_COUNT];
   Eina_Bool is_24hour;
};

#endif
