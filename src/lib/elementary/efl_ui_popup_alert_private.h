#ifndef EFL_UI_POPUP_ALERT_H
#define EFL_UI_POPUP_ALERT_H

#include "Elementary.h"

typedef enum
{
   EFL_UI_POPUP_ALERT_BUTTON_1 = 0,
   EFL_UI_POPUP_ALERT_BUTTON_2,
   EFL_UI_POPUP_ALERT_BUTTON_3,
   EFL_UI_POPUP_ALERT_BUTTON_COUNT
} Efl_Ui_Popup_Alert_Button_Type;

typedef struct _Efl_Ui_Popup_Alert_Data Efl_Ui_Popup_Alert_Data;
struct _Efl_Ui_Popup_Alert_Data
{
   const char *title_text;
   Evas_Object *button[EFL_UI_POPUP_ALERT_BUTTON_COUNT];
   Evas_Object *button_layout[EFL_UI_POPUP_ALERT_BUTTON_COUNT];
};

#endif
