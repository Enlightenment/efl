#ifndef EFL_UI_SCROLL_ALERT_POPUP_H
#define EFL_UI_SCROLL_ALERT_POPUP_H

#include "Elementary.h"

typedef struct _Efl_Ui_Scroll_Alert_Popup_Data Efl_Ui_Scroll_Alert_Popup_Data;
struct _Efl_Ui_Scroll_Alert_Popup_Data
{
   Eo         *scroller;
   Eo         *content;
   Eina_Size2D size;
   Eina_Size2D max_size;
};

#endif
