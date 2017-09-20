#ifndef EFL_UI_POPUP_ANCHOR_H
#define EFL_UI_POPUP_ANCHOR_H

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Anchor_Data Efl_Ui_Popup_Anchor_Data;
struct _Efl_Ui_Popup_Anchor_Data
{
   Evas_Object        *anchor;
   Efl_Ui_Popup_Align  align;
   Efl_Ui_Popup_Align  priority[5];
   Efl_Ui_Popup_Align  used_align;
};

#define EFL_UI_POPUP_ANCHOR_DATA_GET(o, sd) \
  Efl_Ui_Popup_Anchor_Data * sd = efl_data_scope_get(o, EFL_UI_POPUP_ANCHOR_CLASS)

#endif
