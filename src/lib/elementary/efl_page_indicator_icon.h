#ifndef EFL_PAGE_INDICATOR_ICON_H
#define EFL_PAGE_INDICATOR_ICON_H

typedef struct _Efl_Page_Indicator_Icon_Data Efl_Page_Indicator_Icon_Data;

struct _Efl_Page_Indicator_Icon_Data
{
   Eo                   *curr, *adj;

   Eina_List            *items;
   Eina_Value           *v;
};


#define EFL_PAGE_INDICATOR_ICON_DATA_GET(o, pd) \
   Efl_Page_Indicator_Icon_Data *pd = \
   efl_data_scope_get(o, EFL_PAGE_INDICATOR_ICON_CLASS)

#endif
