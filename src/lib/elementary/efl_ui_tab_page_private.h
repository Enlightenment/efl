#ifndef EFL_UI_TAB_PAGE_PRIVATE_H
#define EFL_UI_TAB_PAGE_PRIVATE_H

typedef struct _Efl_Ui_Tab_Page_Data                    Efl_Ui_Tab_Page_Data;

struct _Efl_Ui_Tab_Page_Data
{
   Eo                                   *content;
   const char                           *tab_label;
   const char                           *tab_icon;
};


#define EFL_UI_TAB_PAGE_DATA_GET(o, sd) \
  Efl_Ui_Tab_Page_Data *sd = efl_data_scope_get(o, EFL_UI_TAB_PAGE_CLASS)

#define EFL_UI_TAB_PAGE_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Tab_Page_Data *sd = efl_data_scope_safe_get(o, EFL_UI_TAB_PAGE_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#include "efl_ui_tab_page_part_tab.eo.h"

#endif