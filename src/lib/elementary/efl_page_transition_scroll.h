#ifndef EFL_PAGE_TRANSITION_SCROLL_H
#define EFL_PAGE_TRANSITION_SCROLL_H

//#include "efl_ui_widget_pager.h"


/**
  * This is a description on how scroll transition works.
  *
  * Regardless of the number of pages, scroll transition maintains a fixed number
  * of boxes which can contain pages. The boxes move along with scrolling and
  * some of them are emptied or filled as they come in or out of screen area.
  * When pager is scrolled left or right, each box takes the position of its
  * previous or next box. Since the leftmost and rightmost boxes might move in
  * reverse direction, they shouldn't be visible. This is why there are dummy
  * boxes at both ends.
  *
  * Here is an example of box arrangement change by scrolling.
  * The pager has 10 pages, numbered from 1 to 10.
  * The current page is page5, and side_page_num equals 1.
  *
  *     head          |----- screen area ----|          tail
  *    (dummy)        |                      |         (dummy)
  *  ----------  ----------  ----------  ----------  ----------
  *  |  BOX1  |  |  BOX2  |  |  BOX3  |  |  BOX4  |  |  BOX5  |
  *  |        |  |        |  |        |  |        |  |        |
  *  | (empty)|  |  page4 |  |  page5 |  |  page6 |  | (empty)|
  *  |        |  |        |  |        |  |        |  |        |
  *  |        |  |        |  |        |  |        |  |        |
  *  |        |  |        |  |        |  |        |  |        |
  *  ----------  ----------  ----------  ----------  ----------
  *                   |                      |
  *                   |----------------------|
  *
  * After scrolling left,
  * 1. each box takes the position of it's previous box.
  * 2. head and tail is changed.
  * 3. The box moved to the end is emptied.
  * 4. The box moved from the end is filled with content.
  *
  *     head                                            tail
  *  ----------  ----------  ----------  ----------  ----------
  *  |  BOX2  |  |  BOX3  |  |  BOX4  |  |  BOX5  |  |  BOX1  |
  *  |        |  |        |  |        |  |        |  |        |
  *  | (empty)|  |  page5 |  |  page6 |  |  page7 |  | (empty)|
  *  |*emptied|  |        |  |        |  |*filled |  |        |
  *  |        |  |        |  |        |  |        |  |        |
  *  |        |  |        |  |        |  |        |  |        |
  *  ----------  ----------  ----------  ----------  ----------
  *
  */


typedef struct _Page_Info
{
   Evas_Map                *map;
   int                      id;
   int                      pos;
   int                      content_num;
   Eo                      *obj;
   Eo                      *content;

   Eina_Rectangle           geometry;
   Eina_Rectangle           temp;

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
   Eina_Rectangle           viewport;
};

#define EFL_PAGE_TRANSITION_SCROLL_DATA_GET(o, pd) \
   Efl_Page_Transition_Scroll_Data *pd = \
   efl_data_scope_get(o, EFL_PAGE_TRANSITION_SCROLL_CLASS)

#endif
