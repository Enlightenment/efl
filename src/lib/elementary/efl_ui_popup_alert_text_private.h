#ifndef EFL_UI_POPUP_ALERT_TEXT_H
#define EFL_UI_POPUP_ALERT_TEXT_H

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Alert_Text_Data Efl_Ui_Popup_Alert_Text_Data;
struct _Efl_Ui_Popup_Alert_Text_Data
{
   Eo        *scroller;
   Eo        *message;
   Evas_Coord message_minw;
   Evas_Coord pre_minh;
   Evas_Coord max_scroll_w;
   Evas_Coord max_scroll_h;
   Eina_Size2D prev_geom;
   Eina_Size2D prev_min;
   Eina_Bool  is_expandable_w : 1;
   Eina_Bool  is_expandable_h : 1;
   Eina_Bool  is_sizing_eval: 1;
};

#endif
