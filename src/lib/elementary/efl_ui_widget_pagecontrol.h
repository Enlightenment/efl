
#ifndef EFL_UI_WIDGET_PAGECONTROL_H
#define EFL_UI_WIDGET_PAGECONTROL_H


#include <Elementary.h>


typedef struct _Page_Info
{
   int                   id;           // Identifier
   Evas_Coord            x, y, w, h;   // Geometry
   Evas_Coord            px, py, pz;   // Map perspective
   Evas_Coord            cx, cy, cz;   // Map rotation center
   double                dx, dy, dz;   // Map rotation

} Page_Info;

typedef struct _Efl_Ui_Pagecontrol_Data
{
   Eina_List            *content_list;
   Eina_List            *items;
   Eina_List            *page_info;
   Evas_Object          *event;
   Evas_Object          *hidden_clip;
   Ecore_Animator       *animator;
   Ecore_Job            *job;

   Evas_Coord            x, y, w, h;
   Evas_Coord            mouse_x, mouse_y;
   Evas_Coord            mouse_down_x, mouse_down_y;

   struct {
      Evas_Object       *foreclip;
      Evas_Object       *backclip;
      Evas_Coord         x, y, w, h;
   } viewport;

   struct {
      Evas_Coord         x, y;
      int                page;
      double             ratio;
      Eina_Bool          enabled;
   } mouse_down;

   int                   num_of_pages;
   int                   cnt;
   int                   dir;
   int                   page;
   double                ratio;

   Eina_Bool             move_started : 1;
   Eina_Bool             map_enabled : 1;

} Efl_Ui_Pagecontrol_Data;

#define EFL_UI_PAGECONTROL_DATA_GET(o, sd) \
   Efl_Ui_Pagecontrol_Data *sd = efl_data_scope_get(o, EFL_UI_PAGECONTROL_CLASS)

#endif
