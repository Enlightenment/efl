#ifndef EFL_UI_POPUP_ALERT_SCROLL_H
#define EFL_UI_POPUP_ALERT_SCROLL_H

#define EFL_UI_POPUP_ALERT_SCROLL_BETA

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Alert_Scroll_Data Efl_Ui_Popup_Alert_Scroll_Data;
struct _Efl_Ui_Popup_Alert_Scroll_Data
{
   Eo         *scroller;
   Eo         *content;
   Eina_Size2D size;
   Eina_Size2D max_size;
   Eina_Bool   needs_size_calc : 1;
};

#endif
