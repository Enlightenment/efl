#ifndef EFL_UI_WIDGET_PAGER_H
#define EFL_UI_WIDGET_PAGER_H


#include <Elementary.h>

#include "efl_page_transition.h"

typedef struct _Page_Info
{
   Evas_Map *map;
   int id;
   int pos;
   Eo *obj;
   Evas_Coord x, y, w, h;
   Evas_Coord tx, ty, tw, th;
   struct _Page_Info *prev, *next;
   int a;

} Page_Info;

typedef struct _Efl_Ui_Pager_Data
{
   Eo                      *obj;
   Eina_List               *page_infos;
   Eina_List               *content_list;
   Evas_Object             *event;
   Evas_Object             *hidden_clip;
   Ecore_Animator          *animator;
   Ecore_Job               *job;
   Ecore_Job               *page_info_job;

   Evas_Coord               x, y, w, h;
   Evas_Coord               mouse_x, mouse_y;
   Evas_Coord               mouse_down_x, mouse_down_y;

   struct {
      Evas_Object          *foreclip;
      Evas_Object          *backclip;
   } viewport;

   struct {
      int                   w, h;
      int                   padding;
   } page_spec;

   struct {
      Evas_Coord            x, y;
      int                   page;
      double                ratio;
      Eina_Bool             enabled;
   } mouse_down;

   int                      cnt;
   int                      page;
   int                      page_info_num;
   double                   ratio;
   double                   move;

   struct {
      Ecore_Animator       *animator;
      double                src;
      double                dst;
      double                delta;
      Eina_Bool             jump;
   } change;

   Efl_Orient               orientation;
   Efl_Page_Transition     *transition;

   Eina_Bool                move_started : 1;
   Eina_Bool                map_enabled : 1;
   Eina_Bool                prev_block : 1;
   Eina_Bool                next_block: 1;
   Eina_Bool                loop : 1;

} Efl_Ui_Pager_Data;

#define EFL_UI_PAGER_DATA_GET(o, sd) \
   Efl_Ui_Pager_Data *sd = efl_data_scope_get(o, EFL_UI_PAGER_CLASS)

#endif
