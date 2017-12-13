#ifndef EFL_UI_POPUP_ALERT_TEXT_H
#define EFL_UI_POPUP_ALERT_TEXT_H

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Alert_Text_Data Efl_Ui_Popup_Alert_Text_Data;
struct _Efl_Ui_Popup_Alert_Text_Data
{
   Eo *scroller;
   Eo *message;
   Eina_Size2D size;
   Eina_Size2D max_size;
};

#endif
