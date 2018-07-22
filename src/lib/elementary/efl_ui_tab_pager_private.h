#ifndef EFL_UI_TAB_PAGER_PRIVATE_H
#define EFL_UI_TAB_PAGER_PRIVATE_H

typedef struct _Efl_Ui_Tab_Pager_Data                   Efl_Ui_Tab_Pager_Data;

struct _Efl_Ui_Tab_Pager_Data
{
   Eo                                      *tab_bar;

   int                                      cnt;
   int                                      cur;
   Eina_List                               *tab_pages;
};


#define EFL_UI_TAB_PAGER_DATA_GET(o, sd) \
  Efl_Ui_Tab_Pager_Data *sd = efl_data_scope_get(o, EFL_UI_TAB_PAGER_CLASS)

#define EFL_UI_TAB_PAGER_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Tab_Pager_Data *sd = efl_data_scope_safe_get(o, EFL_UI_TAB_PAGER_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#endif
