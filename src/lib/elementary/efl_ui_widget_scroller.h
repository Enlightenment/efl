#ifndef EFL_UI_WIDGET_SCROLLER_H
#define EFL_UI_WIDGET_SCROLLER_H

#include "Elementary.h"

typedef struct _Efl_Ui_Scroller_Data
{
   Eo *content;
   Eo *pan_obj;
   Eo *smanager;

   Eina_Bool  freeze_want : 1;
   Eina_Bool  match_content_w: 1;
   Eina_Bool  match_content_h: 1;
} Efl_Ui_Scroller_Data;

#endif
