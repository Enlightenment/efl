#ifndef EFL_UI_WIDGET_PAN_H
#define EFL_UI_WIDGET_PAN_H

#include "Elementary.h"

typedef struct _Efl_Ui_Pan_Data Efl_Ui_Pan_Data;
struct _Efl_Ui_Pan_Data
{
   Evas_Object                   *content;
   Evas_Coord                     x, y, w, h;
   Evas_Coord                     content_w, content_h, px, py;
};

#endif
