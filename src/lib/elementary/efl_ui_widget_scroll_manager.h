#ifndef EFL_UI_WIDGET_SCROLL_MANAGER_H
#define EFL_UI_WIDGET_SCROLL_MANAGER_H

#include "Elementary.h"

typedef double (*Interpolator)(void *data, double progress);

typedef enum {
  INTERP_LINEAR,
  INTERP_ACCEL,
  INTERP_DECEL,
} InterpType;

typedef struct _Efl_Ui_Scroll_Manager_Data
{
   Evas_Coord                    x, y, w, h;
   Evas_Coord                    wx, wy, ww, wh; /**< Last "wanted" geometry */

   Evas_Object                  *obj;
   Evas_Object                  *pan_obj;
   Evas_Object                  *event_rect;

   Evas_Object                  *parent;

   Efl_Ui_Scrollbar_Mode         hbar_mode, vbar_mode;

   Ecore_Timer                  *hbar_timer, *vbar_timer;
   Efl_Ui_Scroll_Block block;

   struct
   {
      Evas_Coord x, y;
      Evas_Coord sx, sy;

      struct
      {
         Evas_Coord x, y;
         double     timestamp;
      } history[60];

      double est_timestamp_diff;

      double          onhold_vx, onhold_vy;
      double          onhold_vxe, onhold_vye;
      double          onhold_tlast;

      double          last_time_x_wheel;
      double          last_time_y_wheel;

      int             hdir, vdir;

      Evas_Coord      hold_x, hold_y;
      Ecore_Idle_Enterer *hold_enterer;

      double          dragged_began_timestamp;
      Eina_Bool       dragged : 1;
      Eina_Bool       dragged_began : 1;
      Eina_Bool       hold_animator : 1;
      Eina_Bool       onhold_animator : 1;
      Eina_Bool       last_hold_x_wheel : 1;
      Eina_Bool       last_hold_y_wheel : 1;
      Eina_Bool       dir_x : 1;
      Eina_Bool       dir_y : 1;

      Eina_Bool       scroll : 1;
      Eina_Bool       hold : 1;

      Eina_Bool       now : 1;
      Eina_Bool       want_reset : 1;
   } down;

   struct
   {
      Evas_Coord w, h;
      Eina_Bool  resized : 1;
   } content_info;

   struct
   {
      Evas_Coord x, y;
   } step, page;

   struct
   {
      struct
      {
         Evas_Coord      start, end;
         Eina_Bool       animator;
         Interpolator interp;
         double start_t, dur;
      } x, y;
   } scrollto;

   struct
   {
      struct
      {
         Evas_Coord      p0, p1, p2;
         double          vel;
         Eina_Bool       animator;
         double start_t;
         double t01, t12;
      } x, y;
   } bounce;

   double       last_wheel_mul;
   unsigned int last_wheel;

   void         *event_info;

   double                         gravity_x, gravity_y;
   Evas_Coord                     prev_cw, prev_ch;

   Eina_Bool  hbar_visible : 1;
   Eina_Bool  vbar_visible : 1;
   Eina_Bool  bounce_horiz : 1;
   Eina_Bool  bounce_vert : 1;
   Eina_Bool  is_mirrored : 1;
   Eina_Bool  bouncemey : 1;
   Eina_Bool  bouncemex : 1;
   Eina_Bool  freeze : 1;
   Eina_Bool  freeze_want : 1;
   Eina_Bool  hold : 1;
   Eina_Bool  match_content_w : 1;
   Eina_Bool  match_content_h : 1;
   Eina_Bool  loop_h : 1;
   Eina_Bool  loop_v : 1;
   Eina_Bool  scrolling : 1;
} Efl_Ui_Scroll_Manager_Data;

#endif
