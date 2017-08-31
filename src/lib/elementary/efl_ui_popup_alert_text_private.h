#ifndef EFL_UI_POPUP_ALERT_TEXT_H
#define EFL_UI_POPUP_ALERT_TEXT_H

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Alert_Text_Data Efl_Ui_Popup_Alert_Text_Data;
struct _Efl_Ui_Popup_Alert_Text_Data
{
   Evas_Object *scroller;
   Evas_Object *message;
   Evas_Coord max_scroll_h;
   Eina_Bool is_expandable_h;
};

#endif
