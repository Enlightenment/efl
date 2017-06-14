#ifndef EFL_UI_WIDGET_SCROLL_MANAGER_H
#define EFL_UI_WIDGET_SCROLL_MANAGER_H

#include "Elementary.h"

typedef struct _Efl_Ui_Scroll_Manager_Data
{
   Evas_Coord                  prev_x, prev_y;

   struct {
      Evas_Object             *obj;
      Evas_Coord               x, y, w, h;
   } edje_obj;

   struct {
      Evas_Object             *obj;
      Evas_Coord               w, h;
   } content;

   struct {
      Evas_Coord               x, y;   //mouse cursor
      Evas_Coord               cx, cy; //content position
      Eina_Bool                on : 1;
   } down;

   Eo                         *parent;

   Evas_Object                *event_obj;
   Evas_Object                *pan_obj;

} Efl_Ui_Scroll_Manager_Data;

#endif
