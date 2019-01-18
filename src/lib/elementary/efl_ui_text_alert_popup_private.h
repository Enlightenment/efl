#ifndef EFL_UI_TEXT_ALERT_POPUP_H
#define EFL_UI_TEXT_ALERT_POPUP_H

#include "Elementary.h"

typedef struct _Efl_Ui_Text_Alert_Popup_Data Efl_Ui_Text_Alert_Popup_Data;
struct _Efl_Ui_Text_Alert_Popup_Data
{
   Eo *scroller;
   Eo *message;
   Eina_Size2D size;
   Eina_Size2D max_size;
};

#endif
