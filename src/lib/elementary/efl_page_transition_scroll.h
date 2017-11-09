#ifndef EFL_PAGE_TRANSITION_SCROLL_H
#define EFL_PAGE_TRANSITION_SCROLL_H

//#include "efl_ui_widget_pager.h"

typedef struct _Page_Info
{
   Evas_Map                *map;
   int                      id;
   int                      pos;
   int                      content_num;
   Eo                      *obj;
   Eo                      *content;

   Eina_Rectangle          *geometry;
   Eina_Rectangle          *temp;

   struct _Page_Info       *prev, *next;

   Eina_Bool                visible;
   Eina_Bool                vis_page;

} Page_Info;

typedef struct _Efl_Page_Transition_Scroll_Data Efl_Page_Transition_Scroll_Data;

struct _Efl_Page_Transition_Scroll_Data
{
   Eina_List               *page_infos;

   Page_Info               *head, *tail;

   int                      page_info_num;
   int                      side_page_num;

   Eo                      *foreclip;
   Eo                      *backclip;
   Eina_Rectangle          *viewport;
};

#define EFL_PAGE_TRANSITION_SCROLL_DATA_GET(o, pd) \
   Efl_Page_Transition_Scroll_Data *pd = \
   efl_data_scope_get(o, EFL_PAGE_TRANSITION_SCROLL_CLASS)

#endif
