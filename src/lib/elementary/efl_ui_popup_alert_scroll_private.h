#ifndef EFL_UI_POPUP_ALERT_SCROLL_H
#define EFL_UI_POPUP_ALERT_SCROLL_H

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Alert_Scroll_Data Efl_Ui_Popup_Alert_Scroll_Data;
struct _Efl_Ui_Popup_Alert_Scroll_Data
{
   Evas_Object *scroller;
   Evas_Object *content;
   Evas_Coord max_scroll_w;
   Evas_Coord max_scroll_h;
   Eina_Bool is_expandable_w;
   Eina_Bool is_expandable_h;
   Eina_Bool is_sizing_eval;
};

#endif
