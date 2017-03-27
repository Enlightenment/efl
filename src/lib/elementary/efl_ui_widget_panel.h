#ifndef EFL_UI_WIDGET_PANEL_H
#define EFL_UI_WIDGET_PANEL_H

#include "Elementary.h"

typedef struct _Efl_Ui_Panel_Data
{
   Evas_Object          *scroller;
   Evas_Object          *box;
   Evas_Object          *content_table;
   Evas_Object          *content_spacer;
   Evas_Object          *event_spacer;
   Evas_Object          *content;
   Evas_Object          *event_block;

   Evas_Coord            x, y, w, h;

   Efl_Orient            orient;
   Efl_Ui_Panel_State    state;

} Efl_Ui_Panel_Data;

#endif
