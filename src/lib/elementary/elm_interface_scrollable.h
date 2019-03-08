#ifndef ELM_INTEFARCE_SCROLLER_H
#define ELM_INTEFARCE_SCROLLER_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-scrollable-interface The Elementary Scrollable Interface
 *
 * This is a common interface for widgets having @b scrollable views.
 * Widgets using/implementing this must use the
 * @c EVAS_SMART_SUBCLASS_IFACE_NEW macro (instead of the
 * @c EVAS_SMART_SUBCLASS_NEW one) when declaring its smart class,
 * so an interface is also declared.
 *
 * The scrollable interface comes built with Elementary and is exposed
 * as #ELM_SCROLLABLE_IFACE.
 *
 * The interface API is explained in details at
 * #Elm_Scrollable_Smart_Interface.
 *
 * An Elementary scrollable interface will handle an internal @b
 * panning object. It has the function of clipping and moving the
 * actual scrollable content around, by the command of the scrollable
 * interface calls. Though it's not the common case, one might
 * want/have to change some aspects of the internal panning object
 * behavior.  For that, we have it also exposed here --
 * #Elm_Pan_Smart_Class. Use elm_pan_smart_class_get() to build your
 * custom panning object, when creating a scrollable widget (again,
 * only if you need a custom panning object) and set it with
 * Elm_Scrollable_Smart_Interface::extern_pan_set.
 */

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_pan_eo.legacy.h"
#endif

/**
 * Elementary scroller panning base smart data.
 */
typedef struct _Elm_Pan_Smart_Data Elm_Pan_Smart_Data;
struct _Elm_Pan_Smart_Data
{
   Evas_Object                   *self;
   Evas_Object                   *content;
   Evas_Object                   *interface_object;
   Evas_Coord                     x, y, w, h;
   Evas_Coord                     content_w, content_h, px, py;
};

/**
 * Elementary scrollable interface base data.
 */
typedef void      (*Elm_Interface_Scrollable_Cb)(Evas_Object *, void *data);
typedef void      (*Elm_Interface_Scrollable_Min_Limit_Cb)(Evas_Object *obj, Eina_Bool w, Eina_Bool h);
typedef void      (*Elm_Interface_Scrollable_Resize_Cb)(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

typedef struct _Elm_Scrollable_Smart_Interface_Data
  Elm_Scrollable_Smart_Interface_Data;

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_scrollable.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_scrollable_eo.legacy.h"
#endif

struct _Elm_Scrollable_Smart_Interface_Data
{
   Evas_Coord                    x, y, w, h;
   Evas_Coord                    wx, wy, ww, wh; /**< Last "wanted" geometry */

   Evas_Object                  *obj;
   Evas_Object                  *content;
   Evas_Object                  *pan_obj;
   Evas_Object                  *edje_obj;
   Evas_Object                  *event_rect;

   Evas_Object                  *parent_widget;

   Elm_Scroller_Policy           hbar_flags, vbar_flags;
   Elm_Scroller_Single_Direction one_direction_at_a_time;
   Efl_Ui_Scroll_Block           block;

   struct
   {
      Evas_Coord x, y;
      Evas_Coord sx, sy;
      Evas_Coord dx, dy;
      Evas_Coord pdx, pdy;
      Evas_Coord bx, by;
      Evas_Coord ax, ay;
      Evas_Coord bx0, by0;
      Evas_Coord b0x, b0y;
      Evas_Coord b2x, b2y;

      struct
      {
         Evas_Coord x, y;
         double     timestamp;
      } history[60];

      double est_timestamp_diff;

      double          dragged_began_timestamp;
      double          anim_start;
      double          anim_start2;
      double          anim_start3;
      double          anim_dur;

      double          onhold_vx, onhold_vy, onhold_tlast,
                      onhold_vxe, onhold_vye;

      double          last_time_x_wheel;
      double          last_time_y_wheel;

      Evas_Coord      hold_x, hold_y;
      Evas_Coord      locked_x, locked_y;
      int             hdir, vdir;

      Ecore_Idle_Enterer *hold_enterer;

      Eina_Bool       hold_animator : 1;
      Eina_Bool       onhold_animator : 1;
      Eina_Bool       momentum_animator : 1; /**< an animator which is called whenever a scroller is moving due to a flick action(mouse down, move, up) */
      Eina_Bool       bounce_x_animator : 1; /**< an animator to express the bouncing animation on x axis. */
      Eina_Bool       bounce_y_animator : 1; /**< an animator to express the bouncing animation on y axis. */

      Eina_Bool       last_hold_x_wheel : 1;
      Eina_Bool       last_hold_y_wheel : 1;
      Eina_Bool       bounce_x_hold : 1;
      Eina_Bool       bounce_y_hold : 1;
      Eina_Bool       dragged_began : 1;
      Eina_Bool       want_dragged : 1;
      Eina_Bool       hold_parent : 1;
      Eina_Bool       want_reset : 1;
      Eina_Bool       cancelled : 1;
      Eina_Bool       dragged : 1;
      Eina_Bool       locked : 1;
      Eina_Bool       scroll : 1;
      Eina_Bool       dir_x : 1;
      Eina_Bool       dir_y : 1;
      Eina_Bool       hold : 1;
      Eina_Bool       now : 1;
   } down;

   struct
   {
      Evas_Coord w, h;
      Eina_Bool  resized : 1;
   } content_info;

   struct
   {
      Evas_Coord x, y;
   } step, page, current_page;

   struct
   {
      Elm_Interface_Scrollable_Cb drag_start;
      Elm_Interface_Scrollable_Cb drag_stop;
      Elm_Interface_Scrollable_Cb animate_start;
      Elm_Interface_Scrollable_Cb animate_stop;
      Elm_Interface_Scrollable_Cb scroll;
      Elm_Interface_Scrollable_Cb scroll_left;
      Elm_Interface_Scrollable_Cb scroll_right;
      Elm_Interface_Scrollable_Cb scroll_up;
      Elm_Interface_Scrollable_Cb scroll_down;
      Elm_Interface_Scrollable_Cb edge_left;
      Elm_Interface_Scrollable_Cb edge_right;
      Elm_Interface_Scrollable_Cb edge_top;
      Elm_Interface_Scrollable_Cb edge_bottom;
      Elm_Interface_Scrollable_Cb vbar_drag;
      Elm_Interface_Scrollable_Cb vbar_press;
      Elm_Interface_Scrollable_Cb vbar_unpress;
      Elm_Interface_Scrollable_Cb hbar_drag;
      Elm_Interface_Scrollable_Cb hbar_press;
      Elm_Interface_Scrollable_Cb hbar_unpress;
      Elm_Interface_Scrollable_Cb page_change;

      Elm_Interface_Scrollable_Min_Limit_Cb content_min_limit;
      Elm_Interface_Scrollable_Resize_Cb content_viewport_resize;
   } cb_func;

   struct
   {
      struct
      {
         Evas_Coord      start, end;
         double          t_start, t_end;
         Eina_Bool       animator;
      } x, y;
   } scrollto;

   double     pagerel_h, pagerel_v;
   Evas_Coord pagesize_h, pagesize_v;
   int        page_limit_h, page_limit_v;
   int        current_calc;

   double       last_wheel_mul;
   unsigned int last_wheel;

   unsigned char size_adjust_recurse;
   unsigned char size_count;
   void         *event_info;

   double                         gravity_x, gravity_y;
   Evas_Coord                     prev_cw, prev_ch;

   Eina_Bool  size_adjust_recurse_abort : 1;

   Eina_Bool  momentum_animator_disabled : 1;
   Eina_Bool  bounce_animator_disabled : 1;
   Eina_Bool  page_snap_horiz : 1;
   Eina_Bool  page_snap_vert : 1;
   Eina_Bool  wheel_disabled : 1;
   Eina_Bool  hbar_visible : 1;
   Eina_Bool  vbar_visible : 1;
   Eina_Bool  bounce_horiz : 1;
   Eina_Bool  bounce_vert : 1;
   Eina_Bool  is_mirrored : 1;
   Eina_Bool  extern_pan : 1;
   Eina_Bool  bouncemey : 1;
   Eina_Bool  bouncemex : 1;
   Eina_Bool  freeze : 1;
   Eina_Bool  freeze_want : 1;
   Eina_Bool  hold : 1;
   Eina_Bool  min_w : 1;
   Eina_Bool  min_h : 1;
   Eina_Bool  go_left : 1;
   Eina_Bool  go_right : 1;
   Eina_Bool  go_up : 1;
   Eina_Bool  go_down : 1;
   Eina_Bool  loop_h : 1;
   Eina_Bool  loop_v : 1;

   Efl_Ui_Focus_Manager *manager;
};

#define ELM_SCROLLABLE_CHECK(obj, ...)                                       \
                                                                             \
  if (!efl_isa(obj, ELM_INTERFACE_SCROLLABLE_MIXIN))                    \
    {                                                                        \
       ERR("The object (%p) doesn't implement the Elementary scrollable"     \
            " interface", obj);                                              \
       if (getenv("ELM_ERROR_ABORT")) abort();                               \
       return __VA_ARGS__;                                                   \
    }

#if defined(EFL_EO_API_SUPPORT) && defined(EFL_BETA_API_SUPPORT)
EAPI void elm_pan_gravity_set(Elm_Pan *, double x, double) EINA_DEPRECATED;
EAPI void elm_pan_gravity_get(const Elm_Pan *, double *, double *) EINA_DEPRECATED;
#endif

/**
 * @}
 */

#endif
