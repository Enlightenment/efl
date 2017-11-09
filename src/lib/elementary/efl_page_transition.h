#ifndef EFL_PAGE_TRANSITION_H
#define EFL_PAGE_TRANSITION_H


typedef struct _Efl_Page_Transition_Data Efl_Page_Transition_Data;

struct _Efl_Page_Transition_Data
{
   struct {
      Efl_Object           *obj;
      Evas_Coord            x, y, w, h;
   } pager;

   struct {
      Evas_Coord            w, h;
      Evas_Coord            padding;
   } page_spec;

   Eina_Bool                loop : 1;
};

#define EFL_PAGE_TRANSITION_DATA_GET(o, pd) \
   Efl_Page_Transition_Data *pd = \
   efl_data_scope_get(o, EFL_PAGE_TRANSITION_CLASS)

#endif
