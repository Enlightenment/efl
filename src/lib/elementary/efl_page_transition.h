#ifndef EFL_PAGE_TRANSITION_H
#define EFL_PAGE_TRANSITION_H


typedef struct _Efl_Page_Transition_Data Efl_Page_Transition_Data;

struct _Efl_Page_Transition_Data
{
   struct {
      Efl_Object           *obj;
      Efl_Canvas_Group     *group;
      Evas_Coord            x, y, w, h;
   } pager;

   struct {
      Eina_Size2D           sz;
      Evas_Coord            padding;
   } page_spec;

   Efl_Ui_Pager_Loop        loop;
};

#define EFL_PAGE_TRANSITION_DATA_GET(o, pd) \
   Efl_Page_Transition_Data *pd = \
   efl_data_scope_get(o, EFL_PAGE_TRANSITION_CLASS)

#endif
