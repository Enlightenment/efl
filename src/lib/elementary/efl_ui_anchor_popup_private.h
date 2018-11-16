#ifndef EFL_UI_ANCHOR_POPUP_H
#define EFL_UI_ANCHOR_POPUP_H

#include "Elementary.h"

typedef struct _Efl_Ui_Anchor_Popup_Data Efl_Ui_Anchor_Popup_Data;
struct _Efl_Ui_Anchor_Popup_Data
{
   Eo                 *anchor;
   Efl_Ui_Popup_Align  priority[5];
   Efl_Ui_Popup_Align  used_align;
};

#define EFL_UI_ANCHOR_POPUP_DATA_GET(o, sd) \
  Efl_Ui_Anchor_Popup_Data * sd = efl_data_scope_get(o, EFL_UI_ANCHOR_POPUP_CLASS)

#endif
