#ifndef EFL_PAGE_INDICATOR_H
#define EFL_PAGE_INDICATOR_H

typedef struct _Efl_Page_Indicator_Data Efl_Page_Indicator_Data;

struct _Efl_Page_Indicator_Data
{
   struct {
      Efl_Object           *obj;
      Efl_Object           *group;
      Evas_Coord            x, y, w, h;
   } pager;

   Efl_Ui_Box              *idbox;

   int                      cnt;
};

#define EFL_PAGE_INDICATOR_DATA_GET(o, pd) \
   Efl_Page_Indicator_Data *pd = \
   efl_data_scope_get(o, EFL_PAGE_INDICATOR_CLASS)

#endif
