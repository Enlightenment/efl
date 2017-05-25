#ifndef EFL_UI_WIDGET_SCROLL_MANAGER_H
#define EFL_UI_WIDGET_SCROLL_MANAGER_H

#include "Elementary.h"

typedef struct _Efl_Ui_Scroll_Manager_Data
{
   Eo                         *parent;

   Evas_Object                *hit_rect;
   Evas_Object                *content;

} Efl_Ui_Scroll_Manager_Data;

#endif
