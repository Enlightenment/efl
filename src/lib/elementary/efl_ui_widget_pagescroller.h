#ifndef EFL_UI_WIDGET_PAGESCROLLER_H
#define EFL_UI_WIDGET_PAGESCROLLER_H


#include <Elementary.h>

#include "efl_page_transition.h"

typedef struct _Efl_Ui_Pagescroller_Data
{
   Efl_Page_Transition *transition;

} Efl_Ui_Pagescroller_Data;

#define EFL_UI_PAGESCROLLER_DATA_GET(o, sd) \
   Efl_Ui_Pagescroller_Data *sd = efl_data_scope_get(o, EFL_UI_PAGESCROLLER_CLASS)

#endif
