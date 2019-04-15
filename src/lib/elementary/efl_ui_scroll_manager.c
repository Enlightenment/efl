#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroll_manager.h"

#define MY_CLASS EFL_UI_SCROLL_MANAGER_CLASS
#define MY_CLASS_NAME "Efl.Ui.Scroll.Manager"

#define ELM_ANIMATOR_CONNECT(Obj, Bool, Callback, Data)                 \
  efl_event_callback_del(Obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, Callback, Data); \
  efl_event_callback_add(Obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, Callback, Data); \
  Bool = 1;

#define ELM_ANIMATOR_DISCONNECT(Obj, Bool, Callback, Data)              \
  efl_event_callback_del(Obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, Callback, Data); \
  Bool = 0;


static double
_scroll_manager_linear_interp(void *data EINA_UNUSED, double progress)
{
   return progress;
}
static double
_scroll_manager_accel_interp(void *data EINA_UNUSED, double progress)
{
   return progress * progress;
}
static double
_scroll_manager_decel_interp(void *data EINA_UNUSED, double progress)
{
   return (1.0 - (1.0 - progress) * (1.0 - progress));
}

static Interpolator
_scroll_manager_interp_get(InterpType interp)
{
   if (interp == INTERP_ACCEL)
     return _scroll_manager_accel_interp;
   else if (interp == INTERP_DECEL)
     return _scroll_manager_decel_interp;
   return _scroll_manager_linear_interp;
}

// Prototypes --- //

// ANIMATORS - tick function
static void _efl_ui_scroll_manager_hold_animator(void *data, const Efl_Event *event);
static void _efl_ui_scroll_manager_on_hold_animator(void *data, const Efl_Event *event);
static void _efl_ui_scroll_manager_scroll_to_y_animator(void *data, const Efl_Event *event);
static void _efl_ui_scroll_manager_scroll_to_x_animator(void *data, const Efl_Event *event);
static void _efl_ui_scroll_manager_bounce_y_animator(void *data, const Efl_Event *event);
static void _efl_ui_scroll_manager_bounce_x_animator(void *data, const Efl_Event *event);

// ANIMATORS - manipulate function
static void _scroll_manager_hold_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord x, Evas_Coord y);
static Eina_Bool _scroll_manager_hold_animator_del(Efl_Ui_Scroll_Manager_Data *sd);

static void _scroll_manager_on_hold_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vx, double vy);
static Eina_Bool _scroll_manager_on_hold_animator_del(Efl_Ui_Scroll_Manager_Data *sd);

/// Constant scrolling
static void _scroll_manager_scrollto_animator_add(Efl_Ui_Scroll_Manager_Data *sd,  Evas_Coord cx, Evas_Coord cy, Evas_Coord x, Evas_Coord y, double tx, double ty, InterpType interp);
static Eina_Bool _scroll_manager_scrollto_animator_del(Efl_Ui_Scroll_Manager_Data *sd);
static void _scroll_manager_scrollto_x_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord cx, Evas_Coord x, double t, InterpType interp);
static Eina_Bool _scroll_manager_scrollto_x_animator_del(Efl_Ui_Scroll_Manager_Data *sd);
static void _scroll_manager_scrollto_y_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord cy, Evas_Coord y, double t, InterpType interp);
static Eina_Bool _scroll_manager_scrollto_y_animator_del(Efl_Ui_Scroll_Manager_Data *sd);

/// Flicking
static void _scroll_manager_momentum_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vx, double vy);

// Bounce
static void _scroll_manager_bounce_x_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vx);
static Eina_Bool _scroll_manager_bounce_x_animator_del(Efl_Ui_Scroll_Manager_Data *sd);
static void _scroll_manager_bounce_y_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vy);
static Eina_Bool _scroll_manager_bounce_y_animator_del(Efl_Ui_Scroll_Manager_Data *sd);

// Util
static void _scroll_manager_scrollto(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord x, Evas_Coord y);
static void _scroll_manager_animators_drop(Evas_Object *obj);

// ETC
static void _efl_ui_scroll_manager_wanted_coordinates_update(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord x,Evas_Coord y);
// --- Prototypes //

static inline double
_round(double value, int pos)
{
   double temp;

   temp = value * pow( 10, pos );
   temp = floor( temp + 0.5 );
   temp *= pow( 10, -pos );

   return temp;
}

#define EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(o, ptr)     \
  Efl_Ui_Scroll_Manager_Data *ptr =            \
    (!efl_isa(o, MY_CLASS) ? NULL :       \
     efl_data_scope_safe_get(o, MY_CLASS));    \
  if (!ptr)                                             \
    {                                                   \
       CRI("No interface data for object %p (%s)", \
                o, evas_object_type_get(o));            \
       return;                                          \
    }

#define EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  Efl_Ui_Scroll_Manager_Data *ptr =                 \
    (!efl_isa(o, MY_CLASS) ? NULL :            \
     efl_data_scope_safe_get(o, MY_CLASS));         \
  if (!ptr)                                                  \
    {                                                        \
       CRI("No interface data for object %p (%s)",      \
                o, evas_object_type_get(o));                 \
       return val;                                           \
    }

static void _efl_ui_scroll_manager_wanted_region_set(Evas_Object *obj);

#define LEFT               0
#define RIGHT              1
#define UP                 2
#define DOWN               3
//#define SCROLLDBG 1
/* smoothness debug calls - for debugging how much smooth your app is */

static inline Eina_Bool
_scroll_manager_thumb_scrollable_get(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (!sd) return EINA_FALSE;
   if ((sd->block & EFL_UI_SCROLL_BLOCK_VERTICAL) &&
       (sd->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL))
     return EINA_FALSE;

   if (!_elm_config->thumbscroll_enable) return EINA_FALSE;

   return EINA_TRUE;
}

static inline Eina_Bool
_scroll_manager_animating_get(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (!sd) return EINA_FALSE;
   return ((sd->bounce.x.animator) || (sd->bounce.y.animator) ||
           (sd->scrollto.x.animator) || (sd->scrollto.y.animator));
}

static void
_efl_ui_scroll_manager_scroll_start(Efl_Ui_Scroll_Manager_Data *sd)
{
   sd->scrolling = EINA_TRUE;
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_START, NULL);
}

static void
_efl_ui_scroll_manager_scroll_stop(Efl_Ui_Scroll_Manager_Data *sd)
{
   sd->scrolling = EINA_FALSE;
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_STOP, NULL);
}

static void
_efl_ui_scroll_manager_drag_start(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
   if (!sd->scrolling)
     _efl_ui_scroll_manager_scroll_start(sd);
}

static void
_efl_ui_scroll_manager_drag_stop(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_efl_ui_scroll_manager_anim_start(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_ANIM_START, NULL);
   if (!sd->scrolling)
     _efl_ui_scroll_manager_scroll_start(sd);
}

static void
_efl_ui_scroll_manager_anim_stop(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_ANIM_STOP, NULL);
   if (sd->scrolling)
     _efl_ui_scroll_manager_scroll_stop(sd);
}

static void
_efl_ui_scroll_manager_scroll(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL, NULL);
}

static void
_efl_ui_scroll_manager_scroll_up(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_UP, NULL);
}

static void
_efl_ui_scroll_manager_scroll_down(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_DOWN, NULL);
}

static void
_efl_ui_scroll_manager_scroll_left(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_LEFT, NULL);
}

static void
_efl_ui_scroll_manager_scroll_right(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_SCROLL_RIGHT, NULL);
}

static void
_efl_ui_scroll_manager_edge_up(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_EDGE_UP, NULL);
}

static void
_efl_ui_scroll_manager_edge_down(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_EDGE_DOWN, NULL);
}

static void
_efl_ui_scroll_manager_edge_left(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_EDGE_LEFT, NULL);
}

static void
_efl_ui_scroll_manager_edge_right(Efl_Ui_Scroll_Manager_Data *sd)
{
   efl_event_callback_call(sd->parent, EFL_UI_EVENT_EDGE_RIGHT, NULL);
}

EOLIAN static Eina_Size2D
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd)
{
   return efl_ui_pan_content_size_get(sd->pan_obj);
}

EOLIAN static Eina_Rect
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_viewport_geometry_get(const Eo *obj EINA_UNUSED,
                                                                       Efl_Ui_Scroll_Manager_Data *sd)
{
   if (!sd->pan_obj) return EINA_RECT(0, 0, 0, 0);

   return efl_gfx_entity_geometry_get(sd->pan_obj);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_match_content_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool w, Eina_Bool h)
{
   sd->match_content_w = !!w;
   sd->match_content_h = !!h;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_step_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Position2D step)
{
   sd->step.x = step.x * elm_config_scale_get();
   sd->step.y = step.y * elm_config_scale_get();
}

EOLIAN static Eina_Position2D
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_step_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd)
{
   return EINA_POSITION2D(sd->step.x, sd->step.y);
}

static Evas_Coord
_efl_ui_scroll_manager_x_mirrored_get(const Evas_Object *obj,
                           Evas_Coord x)
{
   Evas_Coord ret;
   Eina_Position2D min = {0, 0}, max = {0, 0};

   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN_VAL(obj, sd, x);

   if (!sd->pan_obj) return 0;

   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);
   ret = max.x - (x - min.x);

   return (ret >= min.x) ? ret : min.x;
}

/* Update the wanted coordinates according to the x, y passed
 * widget directionality, content size and etc. */
static void
_efl_ui_scroll_manager_wanted_coordinates_update(Efl_Ui_Scroll_Manager_Data *sd,
                                      Evas_Coord x,
                                      Evas_Coord y)
{
   Eina_Position2D min = {0, 0}, max = {0, 0};

   if (!sd->pan_obj) return;

   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);

   /* Update wx/y/w/h - and if the requested positions aren't legal
    * adjust a bit. */
   Eina_Rect r = efl_ui_scrollable_viewport_geometry_get(sd->obj);
   sd->ww = r.w;
   sd->wh = r.h;

   if (x < min.x && !sd->is_mirrored)
     {
        if (!sd->loop_h) sd->wx = min.x;
        else sd->wx = max.x;
     }
   else if (sd->is_mirrored)
     sd->wx = _efl_ui_scroll_manager_x_mirrored_get(sd->obj, x);
   else if (!sd->loop_h && (x > max.x)) sd->wx = max.x;
   else if (sd->loop_h && x >= (sd->ww + max.x)) sd->wx = min.x;
   else sd->wx = x;

   if (y < min.y)
     {
        if (!sd->loop_v) sd->wy = min.y;
        else sd->wy = max.y;
     }
   else if (!sd->loop_v && (y > max.y)) sd->wy = max.y;
   else if (sd->loop_v && y >= (sd->wh + max.y)) sd->wy = min.y;
   else sd->wy = y;
}

static void
_scroll_manager_animator_velocity_get(Efl_Ui_Scroll_Manager_Data *sd, double *velx, double *vely)
{
   Evas_Coord dx = 0, dy = 0;
   double vx = 0.0, vy = 0.0;
   double t = ecore_loop_time_get();
   Eina_Position2D cur = efl_ui_pan_position_get(sd->pan_obj);

   if (t < sd->scrollto.x.start_t + sd->scrollto.x.dur)
     {
        dx = sd->scrollto.x.end - cur.x;
        vx = (double)(dx /((sd->scrollto.x.start_t + sd->scrollto.x.dur) - t));

     if (sd->scrollto.x.interp)
         vx = sd->scrollto.x.interp(NULL, t/(sd->scrollto.x.start_t + sd->scrollto.x.dur)) * vx;
     }
   if (t < sd->scrollto.y.start_t + sd->scrollto.y.dur)
     {
        dy = sd->scrollto.y.end - cur.y;
        vy = (double)(dy /((sd->scrollto.y.start_t + sd->scrollto.y.dur) - t));

        if (sd->scrollto.y.interp)
         vy = sd->scrollto.y.interp(NULL, t/(sd->scrollto.y.start_t + sd->scrollto.y.dur)) * vy;
     }

   if (velx) *velx = vx;
   if (vely) *vely = vy;
}

static void
_efl_ui_scroll_manager_bounce_eval(Efl_Ui_Scroll_Manager_Data *sd)
{
   double vx = 0.0, vy = 0.0;
   if (!sd->pan_obj) return;

   if (sd->freeze) return;
   if ((!sd->bouncemex) && (!sd->bouncemey)) return;
   if (sd->down.now) return;  // down bounce while still held down

   _scroll_manager_on_hold_animator_del(sd);
   _scroll_manager_hold_animator_del(sd);

   _scroll_manager_animator_velocity_get(sd, &vx, &vy);
   if (!sd->bounce.x.animator)
     {
        if (sd->bouncemex)
          {
             _scroll_manager_scrollto_x_animator_del(sd);
             _scroll_manager_bounce_x_animator_add(sd,vx);
          }
     }
   if (!sd->bounce.y.animator)
     {
        if (sd->bouncemey)
          {
             _scroll_manager_scrollto_y_animator_del(sd);
             _scroll_manager_bounce_y_animator_add(sd,vy);
          }
     }
}

EOLIAN static Eina_Position2D
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_content_pos_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd)
{
   if (!sd->pan_obj) return EINA_POSITION2D(0, 0);

   return efl_ui_pan_position_get(sd->pan_obj);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_content_pos_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Position2D pos)
{
   Evas_Coord x = pos.x, y = pos.y;
   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};
   Eina_Size2D content = {0, 0};

   if (!sd->pan_obj) return;

   // FIXME: allow for bounce outsde of range
   max = efl_ui_pan_position_max_get(sd->pan_obj);
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   content = efl_ui_pan_content_size_get(sd->pan_obj);
   cur = efl_ui_pan_position_get(sd->pan_obj);

   if (sd->loop_h && content.w > 0)
     {
        if (x < 0) x = content.w + (x % content.w);
        else if (x >= content.w) x = (x % content.w);
     }
   if (sd->loop_v && content.h > 0)
     {
        if (y < 0) y = content.h + (y % content.h);
        else if (y >= content.h) y = (y % content.h);
     }

   if (!_elm_config->thumbscroll_bounce_enable)
     {

        if (x < min.x) x = min.x;
        if (!sd->loop_h && (x - min.x) > max.x) x = max.x + min.x;
        if (y < min.y) y = min.y;
        if (!sd->loop_v && (y - min.y) > max.y) y = max.y + min.y;
     }

   if (!sd->bounce_horiz)
     {
        if (x < min.x) x = min.x;
        if (!sd->loop_h && (x - min.x) > max.x) x = max.x + min.x;
     }
   if (!sd->bounce_vert)
     {
        if (y < min.y) y = min.y;
        if (!sd->loop_v && (y - min.y) > max.y) y = max.y + min.y;
     }

   efl_ui_pan_position_set(sd->pan_obj, EINA_POSITION2D(x, y));

   if (!sd->loop_h && !sd->bounce.x.animator)
     {
        if ((x < min.x) ||(x > max.x + min.x))
          {
             sd->bouncemex = EINA_TRUE;
             _efl_ui_scroll_manager_bounce_eval(sd);
          }
        else
          sd->bouncemex = EINA_FALSE;
     }
   if (!sd->loop_v && !sd->bounce.y.animator)
     {
        if ((y < min.y) ||(y > max.y + min.y))
          {
             sd->bouncemey = EINA_TRUE;
             _efl_ui_scroll_manager_bounce_eval(sd);
          }
        else
          sd->bouncemey = EINA_FALSE;
     }

     {
        if ((x != cur.x) || (y != cur.y))
          {
             _efl_ui_scroll_manager_scroll(sd);
             if (x < cur.x)
               {
                  _efl_ui_scroll_manager_scroll_left(sd);
               }
             if (x > cur.x)
               {
                  _efl_ui_scroll_manager_scroll_right(sd);
               }
             if (y < cur.y)
               {
                  _efl_ui_scroll_manager_scroll_up(sd);
               }
             if (y > cur.y)
               {
                  _efl_ui_scroll_manager_scroll_down(sd);
               }
          }
        if (x != cur.x)
          {
             if (x == min.x)
               {
                  _efl_ui_scroll_manager_edge_left(sd);
               }
             if (x == (max.x + min.x))
               {
                  _efl_ui_scroll_manager_edge_right(sd);
               }
          }
        if (y != cur.y)
          {
             if (y == min.y)
               {
                  _efl_ui_scroll_manager_edge_up(sd);
               }
             if (y == max.y + min.y)
               {
                  _efl_ui_scroll_manager_edge_down(sd);
               }
          }
     }
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_i18n_mirrored_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool mirrored)
{
   Evas_Coord wx;

   mirrored = !!mirrored;

   if (sd->is_mirrored == mirrored)
     return;

   sd->is_mirrored = mirrored;

   if (sd->is_mirrored)
     wx = _efl_ui_scroll_manager_x_mirrored_get(sd->obj, sd->wx);
   else
     wx = sd->wx;

   efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(wx, sd->wy));
}

static void
_scroll_manager_animators_drop(Evas_Object *obj)
{
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(obj, sd);
   if ((sd->bounce.x.animator) || (sd->bounce.y.animator) ||
       (sd->scrollto.x.animator) || (sd->scrollto.y.animator))
     {
        if (_scroll_manager_scrollto_x_animator_del(sd))
          {
          }
        if (_scroll_manager_scrollto_y_animator_del(sd))
          {
          }
        if (_scroll_manager_bounce_x_animator_del(sd))
          {
             sd->bouncemex = EINA_FALSE;
             if (sd->content_info.resized)
               _efl_ui_scroll_manager_wanted_region_set(sd->obj);
          }
        if (_scroll_manager_bounce_y_animator_del(sd))
         {
            sd->bouncemey = EINA_FALSE;
            if (sd->content_info.resized)
              _efl_ui_scroll_manager_wanted_region_set(sd->obj);
         }
        _efl_ui_scroll_manager_anim_stop(sd);
    }
  if (_scroll_manager_hold_animator_del(sd))
    {
        _efl_ui_scroll_manager_drag_stop(sd);
        if (sd->content_info.resized)
          _efl_ui_scroll_manager_wanted_region_set(sd->obj);
    }
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollbar_bar_mode_set(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Scroll_Manager_Data *sd,
                                                     Efl_Ui_Scrollbar_Mode hmode,
                                                     Efl_Ui_Scrollbar_Mode vmode)
{
   sd->hbar_mode = hmode;
   sd->vbar_mode = vmode;

   if (sd->hbar_timer &&
       hmode == EFL_UI_SCROLLBAR_MODE_ON)
     ELM_SAFE_FREE(sd->hbar_timer, ecore_timer_del);
   if (sd->vbar_timer &&
       vmode == EFL_UI_SCROLLBAR_MODE_ON)
     ELM_SAFE_FREE(sd->vbar_timer, ecore_timer_del);

   efl_ui_scrollbar_bar_visibility_update(sd->obj);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollbar_bar_mode_get(const Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Scroll_Manager_Data *sd,
                                                     Efl_Ui_Scrollbar_Mode *hmode,
                                                     Efl_Ui_Scrollbar_Mode *vmode)
{
    *hmode = sd->hbar_mode;
    *vmode = sd->vbar_mode;
}

/* returns TRUE when we need to move the scroller, FALSE otherwise.
 * Updates w and h either way, so save them if you need them. */
static Eina_Bool
_efl_ui_scroll_manager_content_region_show_internal(Evas_Object *obj,
                                                    Evas_Coord *_x,
                                                    Evas_Coord *_y,
                                                    Evas_Coord w,
                                                    Evas_Coord h)
{
   Evas_Coord  nx, ny, x = *_x, y = *_y;
   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};
   Eina_Size2D  pan = {0, 0};

   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (!sd->pan_obj) return EINA_FALSE;

   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);
   cur = efl_ui_pan_position_get(sd->pan_obj);
   pan = efl_gfx_entity_size_get(sd->pan_obj);

   nx = x;
   if ((x > cur.x) && (w < pan.w))
     {
        if ((cur.x + pan.w) < (x + w)) nx = x - pan.w + w;
        else nx = cur.x;
     }
   ny = y;
   if ((y > cur.y) && (h < pan.h))
     {
        if ((cur.y + pan.h) < (y + h)) ny = y - pan.h + h;
        else ny = cur.y;
     }

   x = nx;
   y = ny;

   if (!sd->loop_h)
     {
        if (x > max.x) x = max.x;
        if (x < min.x) x = min.x;
     }
   if (!sd->loop_v)
     {
        if (y > max.y) y = max.y;
        if (y < min.y) y = min.y;
     }

   if ((x == cur.x) && (y == cur.y)) return EINA_FALSE;
   *_x = x;
   *_y = y;
   return EINA_TRUE;
}

static void
_efl_ui_scroll_manager_content_region_set(Eo *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(obj, sd);

   _scroll_manager_animators_drop(obj);
   if (_efl_ui_scroll_manager_content_region_show_internal(obj, &x, &y, w, h))
     {
        efl_ui_scrollable_content_pos_set(obj, EINA_POSITION2D(x, y));
        sd->down.sx = x;
        sd->down.sy = y;
        sd->down.x = sd->down.history[0].x;
        sd->down.y = sd->down.history[0].y;
     }
}

static void
_efl_ui_scroll_manager_wanted_region_set(Evas_Object *obj)
{
   Evas_Coord ww, wh, wx;
   Eina_Position2D max = {0, 0};

   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(obj, sd);

   wx = sd->wx;

   if (_scroll_manager_animating_get(sd) || sd->down.now ||
       sd->down.hold_animator || sd->down.onhold_animator) return;

   sd->content_info.resized = EINA_FALSE;

   /* Flip to RTL cords only if init in RTL mode */
   if (sd->is_mirrored)
     wx = _efl_ui_scroll_manager_x_mirrored_get(obj, sd->wx);

   if (sd->ww == -1)
     {
        Eina_Rect r = efl_ui_scrollable_viewport_geometry_get(sd->obj);
        ww = r.w;
        wh = r.h;
     }
   else
     {
        ww = sd->ww;
        wh = sd->wh;
     }

   max = efl_ui_pan_position_max_get(sd->pan_obj);

   wx += (max.x - sd->prev_cw) * sd->gravity_x;
   sd->wy += (max.y - sd->prev_ch) * sd->gravity_y;

   sd->prev_cw = max.x;
   sd->prev_ch = max.y;

   _efl_ui_scroll_manager_content_region_set(obj, wx, sd->wy, ww, wh);
}

static Eina_Value
_scroll_wheel_post_event_job(void *data, const Eina_Value v,
                             const Eina_Future *ev EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;

   // Animations are disabled if we are here
   efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(sd->wx, sd->wy));

   return v;
}

static inline void
_scroll_wheel_post_event_go(Efl_Ui_Scroll_Manager_Data *sd, int x, int y)
{
   Eina_Position2D cur;
   if (sd->hold || sd->freeze) return;
   _efl_ui_scroll_manager_wanted_coordinates_update(sd, x, y);
   if (_elm_config->scroll_animation_disable)
     {
        Eina_Future *f;

        f = eina_future_then(efl_loop_job(efl_loop_get(sd->obj)),
                             _scroll_wheel_post_event_job, sd, NULL);
        efl_future_then(sd->obj, f);
     }
   else
     {
        cur =  efl_ui_pan_position_get(sd->pan_obj);
        _scroll_manager_scrollto_animator_add(sd, cur.x, cur.y, x, y, 0.5, 0.5, INTERP_LINEAR);
     }
}

static Eina_Bool
_scroll_wheel_post_event_cb(void *data, Evas *e EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Wheel *ev = sd->event_info;

   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};
   Eina_Size2D content = {0, 0};
   Evas_Coord x = 0, y = 0, vw = 0, vh = 0;
   Eina_Bool hold = EINA_FALSE;
   Evas_Coord pwx, pwy;
   double t;
   int direction;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ev, EINA_TRUE);

   sd->event_info = NULL;
   direction = ev->direction;

   pwx = sd->wx;
   pwy = sd->wy;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
     direction = !direction;

   cur = efl_ui_pan_position_get(sd->pan_obj);
   x = cur.x;
   y = cur.y;
   if (sd->scrollto.x.animator) x = sd->scrollto.x.end;
   if (sd->scrollto.y.animator) y = sd->scrollto.y.end;
   max = efl_ui_pan_position_max_get(sd->pan_obj);
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   if (x < min.x) x = min.x;
   if (x > max.x) x = max.x;
   if (y < min.y) y = min.y;
   if (y > max.y) y = max.y;

   t = ecore_loop_time_get();

   _scroll_manager_animators_drop(sd->obj);

   Eina_Rect r = efl_ui_scrollable_viewport_geometry_get(sd->obj);
   vw = r.w;
   vh = r.h;

   if (sd->pan_obj)
     content = efl_ui_pan_content_size_get(sd->pan_obj);

   int d = ev->z;
   double delta_t = (double)(ev->timestamp - sd->last_wheel) / 1000.0;
   double mul;

   if (delta_t > 0.2) sd->last_wheel_mul = 0.0;
   if (delta_t > 0.2) delta_t = 0.2;
   mul = 1.0 + (_elm_config->scroll_accel_factor * ((0.2 - delta_t) / 0.2));
   mul = mul * (1.0 + (0.15 * sd->last_wheel_mul));
   d *= mul;
   sd->last_wheel = ev->timestamp;
   sd->last_wheel_mul = mul;

   if (!direction)
     {
        if ((content.h > vh) || (content.w <= vw))
          y += d * sd->step.y;
        else
          {
             x += d * sd->step.x;
             direction = 1;
          }
     }
   else
     {
        if ((content.w > vw) || (content.h <= vh))
          x += d * sd->step.x;
        else
          {
             y += d * sd->step.y;
             direction = 0;
          }
     }
   _scroll_wheel_post_event_go(sd, x, y);

   if (direction)
     {
        if ((sd->bounce_horiz) ||
            (pwx != sd->wx) ||
            (((t - sd->down.last_time_x_wheel) < 0.5) &&
             (sd->down.last_hold_x_wheel)))
          {
             sd->down.last_hold_x_wheel = EINA_TRUE;
             hold = EINA_TRUE;
          }
        else sd->down.last_hold_x_wheel = EINA_FALSE;
        sd->down.last_time_x_wheel = t;
     }
   else
     {
        if ((sd->bounce_vert) ||
            (pwy != sd->wy) ||
            (((t - sd->down.last_time_y_wheel) < 0.5) &&
             (sd->down.last_hold_y_wheel)))
          {
             sd->down.last_hold_y_wheel = EINA_TRUE;
             hold = EINA_TRUE;
          }
        else sd->down.last_hold_y_wheel = EINA_FALSE;
        sd->down.last_time_y_wheel = t;
     }
   return !hold;
}

static void
_efl_ui_scroll_manager_wheel_event_cb(void *data,
                           Evas *e,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd;
   Evas_Event_Mouse_Wheel *ev;
   int direction;

   sd = data;
   ev = event_info;
   sd->event_info = event_info;
   direction = ev->direction;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if ((evas_key_modifier_is_set(ev->modifiers, "Control")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Alt")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Meta")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Hyper")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Super")))
     return;
   if (direction)
     {
        if (sd->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL) return;
     }
   else
     {
        if (sd->block & EFL_UI_SCROLL_BLOCK_VERTICAL) return;
     }

   evas_post_event_callback_push(e, _scroll_wheel_post_event_cb, sd);
}

static void
_efl_ui_scroll_manager_scroll_to_x_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Eina_Position2D min = {0, 0}, max = {0, 0};
   Evas_Coord nx = 0;
   double t = 0.0, dt = 0.0, progx = 0.0, rx = 0.0;
   Interpolator interp = NULL;
   Eina_Bool no_bounce_x_end = EINA_FALSE;

   t = ecore_loop_time_get();
   dt = t - sd->scrollto.x.start_t;

   if ( dt > sd->scrollto.x.dur) progx = 1.0;
   else progx = dt / sd->scrollto.x.dur;

   if (sd->scrollto.x.interp) interp = sd->scrollto.x.interp;
   else interp = _scroll_manager_interp_get(INTERP_LINEAR);

   rx = interp(NULL, progx);
   nx = sd->scrollto.x.start + (sd->scrollto.x.end - sd->scrollto.x.start) * rx;

   Eina_Position2D cur = efl_ui_scrollable_content_pos_get(sd->obj);
   efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(nx, cur.y));
   _efl_ui_scroll_manager_wanted_coordinates_update(sd, nx, cur.y);

   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);

   if (!_elm_config->thumbscroll_bounce_enable || !sd->bounce_horiz)
     {
        if (nx < min.x) no_bounce_x_end = EINA_TRUE;
        if (!sd->loop_h && (nx - min.x) > max.x) no_bounce_x_end = EINA_TRUE;
     }
   if (dt >= sd->scrollto.x.dur || no_bounce_x_end)
   {
      if ((!sd->scrollto.y.animator) &&
          (!sd->bounce.y.animator))
        _efl_ui_scroll_manager_anim_stop(sd);
      ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.x.animator, _efl_ui_scroll_manager_scroll_to_x_animator, sd);
   }
}

static void
_efl_ui_scroll_manager_scroll_to_y_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Eina_Position2D min = {0, 0}, max = {0, 0};
   Evas_Coord ny = 0;
   double t = 0.0, dt = 0.0, progy = 0.0, ry = 0.0;
   Interpolator interp = NULL;
   Eina_Bool no_bounce_y_end = EINA_FALSE;

   t = ecore_loop_time_get();
   dt = t - sd->scrollto.y.start_t;

   if ( dt > sd->scrollto.y.dur) progy = 1.0;
   else progy = dt / sd->scrollto.y.dur;

   if (sd->scrollto.y.interp) interp = sd->scrollto.y.interp;
   else interp = _scroll_manager_interp_get(INTERP_LINEAR);

   ry = interp(NULL, progy);
   ny = sd->scrollto.y.start + (sd->scrollto.y.end - sd->scrollto.y.start) * ry;

   Eina_Position2D cur = efl_ui_scrollable_content_pos_get(sd->obj);
   efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(cur.x, ny));
   _efl_ui_scroll_manager_wanted_coordinates_update(sd, cur.x, ny);

   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);

   if (!_elm_config->thumbscroll_bounce_enable || !sd->bounce_vert)
     {
        if (ny < min.y) no_bounce_y_end = EINA_TRUE;
        if (!sd->loop_v && (ny - min.y) > max.y) no_bounce_y_end = EINA_TRUE;
     }
   if (dt >= sd->scrollto.y.dur || no_bounce_y_end)
   {
      if ((!sd->scrollto.x.animator) &&
          (!sd->bounce.x.animator))
          _efl_ui_scroll_manager_anim_stop(sd);
      ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.y.animator, _efl_ui_scroll_manager_scroll_to_y_animator, sd);
   }
}

static void
_efl_ui_scroll_manager_mouse_up_event_smooth(Efl_Ui_Scroll_Manager_Data *sd, double t, Evas_Coord *ox, Evas_Coord *oy, double *ot)
{
   static const unsigned int HISTORY_MAX = 60;
   unsigned int i = 0;
   double dt = 0, at = 0;
   Evas_Coord ax = 0, ay = 0;

   for (i = 0; i < HISTORY_MAX; i++)
     {
        dt = t - sd->down.history[i].timestamp;
        if (dt > 0.2) break;
#ifdef SCROLLDBG
        DBG("H: %i %i @ %1.3f\n",
        sd->down.history[i].x,
        sd->down.history[i].y, dt);
#endif
        ax = sd->down.history[i].x;
        ay = sd->down.history[i].y;
        at = sd->down.history[i].timestamp;
     }
   if (ox) *ox = ax;
   if (oy) *oy = ay;
   if (ot) *ot = t - at;
}

static void
_efl_ui_scroll_manager_mouse_up_event_momentum_eval(Efl_Ui_Scroll_Manager_Data *sd, Evas_Event_Mouse_Up *ev)
{
   double t, at;
   Evas_Coord dx, dy, ax, ay, vel;
   signed char sdx, sdy;

   t = ev->timestamp / 1000.0;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   ax = ev->canvas.x;
   ay = ev->canvas.y;
   at = 0.0;
#ifdef SCROLLDBG
   DBG("------ %i %i\n", ev->canvas.x, ev->canvas.y);
#endif
   _efl_ui_scroll_manager_mouse_up_event_smooth(sd, t, &ax, &ay, &at);
   dx = ev->canvas.x - ax;
   dy = ev->canvas.y - ay;

   sdx = (dx > 0) - (dx < 0);
   sdy = (dy > 0) - (dy < 0);

   dx = abs(dx);
   dy = abs(dy);
   if (at > 0)
     {
        vel = (Evas_Coord)(sqrt((dx * dx) + (dy * dy)) / at);
        if ((_elm_config->thumbscroll_friction > 0.0) &&
            (vel > _elm_config->thumbscroll_momentum_threshold))
          {
             _scroll_manager_momentum_animator_add(sd, -sdx*dx/at, -sdy*dy/at);
          }
        else if (!sd->bouncemex && !sd->bouncemey)
          {
             _efl_ui_scroll_manager_scroll_stop(sd);
          }
     }
}

static void
_efl_ui_scroll_manager_mouse_up_event_cb(void *data,
                              Evas *e,
                              Evas_Object *obj EINA_UNUSED,
                              void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Up *ev;

   if (!sd->pan_obj) return;
   if (!_scroll_manager_thumb_scrollable_get(sd)) return;

   ev = event_info;

   if (ev->button == 1)
     {
        _scroll_manager_on_hold_animator_del(sd);

        if (sd->down.dragged)
          {
             _efl_ui_scroll_manager_drag_stop(sd);
             if ((!sd->hold) && (!sd->freeze))
               {
                  _efl_ui_scroll_manager_mouse_up_event_momentum_eval(sd, ev);
               }
             evas_event_feed_hold(e, 0, ev->timestamp, ev->data);
          }

        _scroll_manager_hold_animator_del(sd);

        if (sd->down.scroll)
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
             sd->down.scroll = EINA_FALSE;
          }
        if (sd->down.hold)
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             sd->down.hold = EINA_FALSE;
          }

        sd->down.dragged_began = EINA_FALSE;
        sd->down.dir_x = EINA_FALSE;
        sd->down.dir_y = EINA_FALSE;
        sd->down.dragged = EINA_FALSE;
        sd->down.now = EINA_FALSE;

        Eina_Position2D cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, cur);
        _efl_ui_scroll_manager_wanted_coordinates_update(sd, cur.x, cur.y);

        if (sd->content_info.resized)
          _efl_ui_scroll_manager_wanted_region_set(sd->obj);
     }
}

static void
_efl_ui_scroll_manager_mouse_down_event_cb(void *data,
                                Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd;
   Evas_Event_Mouse_Down *ev;
   Eina_Position2D cur = {0, 0};

   sd = data;
   ev = event_info;

   if (!_scroll_manager_thumb_scrollable_get(sd)) return;

   sd->down.hold = EINA_FALSE;
   if (_scroll_manager_animating_get(sd))
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL |
          EVAS_EVENT_FLAG_ON_HOLD;
        sd->down.scroll = EINA_TRUE;
        sd->down.hold = EINA_TRUE;

        _scroll_manager_animators_drop(sd->obj);
     }

   if (ev->button == 1)
     {
        sd->down.est_timestamp_diff =
          ecore_loop_time_get() - ((double)ev->timestamp / 1000.0);
        sd->down.now = EINA_TRUE;
        sd->down.dragged = EINA_FALSE;
        sd->down.dir_x = EINA_FALSE;
        sd->down.dir_y = EINA_FALSE;
        sd->down.x = ev->canvas.x;
        sd->down.y = ev->canvas.y;
        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        sd->down.sx = cur.x;
        sd->down.sy = cur.y;
        memset(&(sd->down.history[0]), 0,
               sizeof(sd->down.history[0]) * 60);
        sd->down.history[0].timestamp = ev->timestamp / 1000.0;
        sd->down.dragged_began_timestamp = sd->down.history[0].timestamp;
        sd->down.history[0].x = ev->canvas.x;
        sd->down.history[0].y = ev->canvas.y;
     }
   sd->down.dragged_began = EINA_FALSE;
   if (sd->hold || sd->freeze)
     sd->down.want_reset = EINA_TRUE;
   else
     sd->down.want_reset = EINA_FALSE;
}

static Eina_Bool
_efl_ui_scroll_manager_can_scroll(Efl_Ui_Scroll_Manager_Data *sd,
                       int dir)
{
   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};

   if (!sd->pan_obj) return EINA_FALSE;

   max = efl_ui_pan_position_max_get(sd->pan_obj);
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   cur = efl_ui_pan_position_get(sd->pan_obj);
   switch (dir)
     {
      case LEFT:
        if (cur.x > min.x) return EINA_TRUE;
        break;

      case RIGHT:
        if ((cur.x - min.x) < max.x) return EINA_TRUE;
        break;

      case UP:
        if (cur.y > min.y) return EINA_TRUE;
        break;

      case DOWN:
        if ((cur.y - min.y) < max.y) return EINA_TRUE;
        break;

      default:
        break;
     }
   return EINA_FALSE;
}

static void
_efl_ui_scroll_manager_bounce_weight_apply(Efl_Ui_Scroll_Manager_Data *sd,
                                           Evas_Coord *x,
                                           Evas_Coord *y)
{
   Eina_Position2D min = {0, 0}, max = {0, 0};
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);

   if (!sd->loop_h && *x < min.x)
     *x += (min.x - *x) * _elm_config->thumbscroll_border_friction;
   else if (!sd->loop_h && max.x <= 0)
     *x += (sd->down.sx - *x) * _elm_config->thumbscroll_border_friction;
   else if (!sd->loop_h && (max.x + min.x) < *x)
     *x += (max.x + min.x - *x) *
       _elm_config->thumbscroll_border_friction;

   if (!sd->loop_v && *y < min.y)
     *y += (min.y - *y) * _elm_config->thumbscroll_border_friction;
   else if (!sd->loop_v && max.y <= 0)
     *y += (sd->down.sy - *y) * _elm_config->thumbscroll_border_friction;
   else if (!sd->loop_v && (max.y + min.y) < *y)
     *y += (max.y + min.y - *y) *
       _elm_config->thumbscroll_border_friction;
}

static inline double
_scroll_manager_animation_duration_get(Evas_Coord dx, Evas_Coord dy)
{
  double dist = 0.0, vel = 0.0, dur = 0.0;
  dist = sqrt(dx * dx + dy *dy);
  vel = _elm_config->thumbscroll_friction_standard / _elm_config->thumbscroll_friction;
  dur = dist / vel;
  dur = (dur > _elm_config->thumbscroll_friction) ? _elm_config->thumbscroll_friction : dur;
  return dur;
}

static Eina_Bool
_scroll_manager_on_hold_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
  if (sd->down.onhold_animator)
    {
      ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->down.onhold_animator, _efl_ui_scroll_manager_on_hold_animator, sd);
      if (sd->content_info.resized)
        _efl_ui_scroll_manager_wanted_region_set(sd->obj);
      return EINA_TRUE;
    }
  return EINA_FALSE;
}

static void
_scroll_manager_on_hold_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vx, double vy)
{
  sd->down.onhold_vx = vx;
  sd->down.onhold_vy = vy;
  if (!sd->down.onhold_animator)
    {
      sd->down.onhold_vxe = 0.0;
      sd->down.onhold_vye = 0.0;
      sd->down.onhold_tlast = 0.0;

      ELM_ANIMATOR_CONNECT(sd->event_rect, sd->down.onhold_animator, _efl_ui_scroll_manager_on_hold_animator, sd);
    }
}

static void
_scroll_manager_hold_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord x, Evas_Coord y)
{
   sd->down.hold_x = x;
   sd->down.hold_y = y;
   ELM_ANIMATOR_CONNECT(sd->event_rect, sd->down.hold_animator, _efl_ui_scroll_manager_hold_animator, sd);
}

static Eina_Bool
_scroll_manager_hold_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (sd->down.hold_animator || sd->down.hold_enterer)
     {
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->down.hold_animator, _efl_ui_scroll_manager_hold_animator, sd);
        ELM_SAFE_FREE(sd->down.hold_enterer, ecore_idle_enterer_del);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_scroll_manager_momentum_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vx, double vy)
{
#define FRICTION 5000
#define INVERSE_MASS 1
#define ACCEL (FRICTION * INVERSE_MASS)
   double dur = 0.0;
   signed char sdx = 0, sdy = 0;
   Evas_Coord dstx = 0, dsty = 0;

/*
   if (_scroll_manager_scrollto_animator_del(sd))
     {
        restore current veolocity
        add to vx/vy
     }
*/
   Eina_Position2D cur =  efl_ui_pan_position_get(sd->pan_obj);

   sdx = (vx > 0) - (vx < 0);
   sdy = (vy > 0) - (vy < 0);

   dstx = cur.x + ((sdx * vx * vx) / (double)(2 * ACCEL));
   dsty = cur.y + ((sdy * vy * vy) / (double)(2 * ACCEL));

   dur = sqrt((vx * vx) + (vy * vy)) / (double)ACCEL;

   _scroll_manager_scrollto_animator_add(sd, cur.x, cur.y, dstx, dsty,
                                         dur, dur, INTERP_DECEL);
}

static void
_efl_ui_scroll_manager_bounce_y_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord ny = 0;
   Eina_Position2D cur = {0, 0};
   double t = 0.0, dt = 0.0, r = 0.0;

   t = ecore_loop_time_get();
   if (sd->bounce.y.start_t + sd->bounce.y.t01 >= t)
     {
        dt = sd->bounce.y.start_t + sd->bounce.y.t01 - t;
        r = 1.0 - (dt / sd->bounce.y.t01);
        r = _scroll_manager_decel_interp(NULL, r);
        ny = sd->bounce.y.p0 + (sd->bounce.y.p1 - sd->bounce.y.p0) * r;

        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(cur.x, ny));
     }
   else if (sd->bounce.y.start_t + sd->bounce.y.t01 + sd->bounce.y.t12 >= t)
     {
        dt = sd->bounce.y.start_t + sd->bounce.y.t01  + sd->bounce.y.t12 - t;
        r = 1.0 - (dt / sd->bounce.y.t12);
        r = _scroll_manager_decel_interp(NULL, r);
        ny = sd->bounce.y.p1 + (sd->bounce.y.p2 - sd->bounce.y.p1) * r;

        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(cur.x, ny));
     }
   else
     {
        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(cur.x, sd->bounce.y.p2));
        if ((!sd->scrollto.x.animator) &&
            (!sd->bounce.x.animator))
          _efl_ui_scroll_manager_anim_stop(sd);
        _scroll_manager_bounce_y_animator_del(sd);
     }
}

static void
_efl_ui_scroll_manager_bounce_x_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord nx;
   Eina_Position2D cur = {0, 0};
   double t = 0.0, dt = 0.0, r = 0.0;

   t = ecore_loop_time_get();

   if (sd->bounce.x.start_t + sd->bounce.x.t01 >= t)
     {
        dt = sd->bounce.x.start_t + sd->bounce.x.t01 - t;
        r = 1.0 - (dt / sd->bounce.x.t01);
        r = _scroll_manager_decel_interp(NULL, r);
        nx = sd->bounce.x.p0 + (sd->bounce.x.p1 - sd->bounce.x.p0) * r;

        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(nx, cur.y));
     }
   else if (sd->bounce.x.start_t + sd->bounce.x.t01 + sd->bounce.x.t12 >= t)
     {
        dt = sd->bounce.x.start_t + sd->bounce.x.t01  + sd->bounce.x.t12 - t;
        r = 1.0 - (dt / sd->bounce.x.t12);
        r = _scroll_manager_decel_interp(NULL, r);
        nx = sd->bounce.x.p1 + (sd->bounce.x.p2 - sd->bounce.x.p1) * r;

        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(nx, cur.y));
     }
   else
     {
        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(sd->bounce.x.p2, cur.y));
        if ((!sd->scrollto.y.animator) &&
            (!sd->bounce.y.animator))
          _efl_ui_scroll_manager_anim_stop(sd);
        _scroll_manager_bounce_x_animator_del(sd);
     }
}

static void _scroll_manager_bounce_x_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vx)
{
   static const double spring_k = 1000;
   static const double mass = 1;
   char sign = (vx > 0) - (vx < 0);
   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};

   _scroll_manager_bounce_x_animator_del(sd);

   cur = efl_ui_pan_position_get(sd->pan_obj);
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);

   double max_x = sqrt((mass * vx * vx) / spring_k);
   sd->bounce.x.start_t = ecore_loop_time_get();
   sd->bounce.x.vel = vx;
   sd->bounce.x.p0 = cur.x;
   if (fabs(vx) > 0.0)
    sd->bounce.x.t01 = 0.2;
   else
    sd->bounce.x.t01 = 0.0;
   sd->bounce.x.p1 = cur.x + sign * max_x;;
   sd->bounce.x.t12 = 0.2;
   if ( cur.x < min.x )
   {
      sd->bounce.x.p2 = min.x;
   }
   else if ( cur.x > max.x)
   {
      sd->bounce.x.p2 = max.x;
   }

   if ((!sd->bounce.y.animator) &&
       (!sd->scrollto.y.animator))
     _efl_ui_scroll_manager_anim_start(sd);
   ELM_ANIMATOR_CONNECT(sd->event_rect, sd->bounce.x.animator, _efl_ui_scroll_manager_bounce_x_animator, sd);
}

static Eina_Bool _scroll_manager_bounce_x_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (sd->bounce.x.animator)
     {
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->bounce.x.animator, _efl_ui_scroll_manager_bounce_x_animator, sd);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void _scroll_manager_bounce_y_animator_add(Efl_Ui_Scroll_Manager_Data *sd, double vy)
{
   static const double spring_k = 1000;
   static const double mass = 1;
   char sign = (vy > 0) - (vy < 0);
   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};

   _scroll_manager_bounce_y_animator_del(sd);

   cur = efl_ui_pan_position_get(sd->pan_obj);
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);

   double max_y = sqrt((mass * vy * vy) / spring_k);
   sd->bounce.y.start_t = ecore_loop_time_get();
   sd->bounce.y.vel = vy;
   sd->bounce.y.p0 = cur.y;
   if (fabs(vy) > 0.0)
    sd->bounce.y.t01 = 0.2;
   else
    sd->bounce.y.t01 = 0.0;

   sd->bounce.y.p1 = cur.y + sign * max_y;
   sd->bounce.y.t12 = 0.2;
   if ( cur.y < min.y )
   {
      sd->bounce.y.p2 = min.y;
   }
   else if ( cur.y > max.y)
   {
      sd->bounce.y.p2 = max.y;
   }

   if ((!sd->bounce.x.animator) &&
       (!sd->scrollto.x.animator))
     _efl_ui_scroll_manager_anim_start(sd);
   ELM_ANIMATOR_CONNECT(sd->event_rect, sd->bounce.y.animator, _efl_ui_scroll_manager_bounce_y_animator, sd);
}

static Eina_Bool _scroll_manager_bounce_y_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (sd->bounce.y.animator)
     {
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->bounce.y.animator, _efl_ui_scroll_manager_bounce_y_animator, sd);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_scroll_manager_scrollto_x_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord sx, Evas_Coord ex, double t, InterpType interp)
{
   sd->scrollto.x.start_t = ecore_loop_time_get();
   sd->scrollto.x.dur = t;

   sd->scrollto.x.start = sx;
   sd->scrollto.x.end = ex;

   sd->scrollto.x.interp = _scroll_manager_interp_get(interp);
    if (!sd->scrollto.x.animator)
      {
         ELM_ANIMATOR_CONNECT(sd->event_rect, sd->scrollto.x.animator, _efl_ui_scroll_manager_scroll_to_x_animator, sd);
         if (!sd->scrollto.y.animator) _efl_ui_scroll_manager_anim_start(sd);
      }
}

static Eina_Bool
_scroll_manager_scrollto_x_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
 if (sd->scrollto.x.animator)
     {
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.x.animator, _efl_ui_scroll_manager_scroll_to_x_animator, sd);
        return EINA_TRUE;
     }
     return EINA_FALSE;
}

static void
_scroll_manager_scrollto_y_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord sy, Evas_Coord ey, double t, InterpType interp)
{
   sd->scrollto.y.start_t = ecore_loop_time_get();
   sd->scrollto.y.dur = t;

   sd->scrollto.y.start = sy;
   sd->scrollto.y.end = ey;

   sd->scrollto.y.interp = _scroll_manager_interp_get(interp);
    if (!sd->scrollto.y.animator)
      {
         ELM_ANIMATOR_CONNECT(sd->event_rect, sd->scrollto.y.animator, _efl_ui_scroll_manager_scroll_to_y_animator, sd);
         if (!sd->scrollto.x.animator) _efl_ui_scroll_manager_anim_start(sd);
      }
}

static Eina_Bool
_scroll_manager_scrollto_y_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
 if (sd->scrollto.y.animator)
     {
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.y.animator, _efl_ui_scroll_manager_scroll_to_y_animator, sd);
        return EINA_TRUE;
     }
     return EINA_FALSE;
}

static void
_scroll_manager_scrollto_animator_add(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord sx, Evas_Coord sy, Evas_Coord x, Evas_Coord y, double tx, double ty, InterpType interp)
{
   _scroll_manager_scrollto_animator_del(sd);

   if (!sd->pan_obj) return;
   if (sd->freeze) return;
   _scroll_manager_scrollto_x_animator_add(sd, sx, x, tx, interp);
   _scroll_manager_scrollto_y_animator_add(sd, sy, y, ty, interp);
}

static Eina_Bool
_scroll_manager_scrollto_animator_del(Efl_Ui_Scroll_Manager_Data *sd)
{
 if ((sd->scrollto.x.animator) || (sd->scrollto.y.animator))
     {
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.x.animator, _efl_ui_scroll_manager_scroll_to_x_animator, sd);
        ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.y.animator, _efl_ui_scroll_manager_scroll_to_y_animator, sd);
        return EINA_TRUE;
     }
     return EINA_FALSE;
}

static void
_scroll_manager_scrollto(Efl_Ui_Scroll_Manager_Data *sd, Evas_Coord x, Evas_Coord y)
{
   double dur = 0.0;
   Eina_Position2D cur = efl_ui_scrollable_content_pos_get(sd->obj);
   dur = _scroll_manager_animation_duration_get(x - cur.x, y - cur.y);
   _scroll_manager_scrollto_animator_add(sd, cur.x, cur.y, x, y, dur, dur, INTERP_LINEAR);
}

static void
_efl_ui_scroll_manager_post_event_move_on_hold_eval(Efl_Ui_Scroll_Manager_Data *sd, Evas_Event_Mouse_Move *ev)
{
   Evas_Coord x = 0, y = 0;
   double vx = 0.0, vy = 0.0;
   char sx = 0, sy = 0;

   Eina_Rect r = efl_ui_scrollable_viewport_geometry_get(sd->obj);

   x = (r.x - ev->cur.canvas.x) > 0 ? (r.x - ev->cur.canvas.x) : 0;
   y = (r.y - ev->cur.canvas.y) > 0 ? (r.y - ev->cur.canvas.y) : 0;
   x = (ev->cur.canvas.x - (r.x + r.w)) > 0 ? (ev->cur.canvas.x - (r.x + r.w)) : x;
   y = (ev->cur.canvas.y - (r.y + r.h)) > 0 ? (ev->cur.canvas.y - (r.y + r.h)) : y;
   sx = r.x - ev->cur.canvas.x > 0 ? -1 : 1;
   sy = r.y - ev->cur.canvas.y > 0 ? -1 : 1;

   if (x > _elm_config->thumbscroll_hold_threshold)
     {
        vx = 1.0;
        if (_elm_config->thumbscroll_hold_threshold > 0.0)
          vx = (double)(x - _elm_config->thumbscroll_hold_threshold) /
               _elm_config->thumbscroll_hold_threshold;
     }

   if (y > _elm_config->thumbscroll_hold_threshold)
     {
        vy = 1.0;
        if (_elm_config->thumbscroll_hold_threshold > 0.0)
          vy = (double)(y - _elm_config->thumbscroll_hold_threshold) /
               _elm_config->thumbscroll_hold_threshold;
     }

   if ((vx != 0.0) || (vy != 0.0)) _scroll_manager_on_hold_animator_add(sd, vx*sx, vy*sy);
   else _scroll_manager_on_hold_animator_del(sd);
}


static Eina_Bool
_efl_ui_scroll_manager_post_event_move_direction_restrict_eval(Efl_Ui_Scroll_Manager_Data *sd, Evas_Event_Mouse_Move *ev EINA_UNUSED,
                                                                Evas_Coord dx, Evas_Coord dy)
{
   if (sd->down.dragged) return EINA_FALSE;

   sd->down.hdir = -1;
   sd->down.vdir = -1;

   if (dx > 0) sd->down.hdir = LEFT;
   else if (dx < 0)
      sd->down.hdir = RIGHT;
   if (dy > 0) sd->down.vdir = UP;
   else if (dy < 0)
      sd->down.vdir = DOWN;

   if (!(sd->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL))
     sd->down.dir_x = EINA_TRUE;

   if (!(sd->block & EFL_UI_SCROLL_BLOCK_VERTICAL))
     sd->down.dir_y = EINA_TRUE;

   return EINA_TRUE;
}

static Eina_Bool
_efl_ui_scroll_manager_post_event_move_hold_eval(Efl_Ui_Scroll_Manager_Data *sd, Evas_Event_Mouse_Move *ev,
                                                 Evas_Coord dx, Evas_Coord dy)
{
  if (!sd->down.dragged)
    {
       if(((dx * dx) + (dy * dy)) >
          (_elm_config->thumbscroll_threshold * _elm_config->thumbscroll_threshold))
         {
            if (_elm_config->scroll_smooth_start_enable)
              {
                 sd->down.x = ev->cur.canvas.x;
                 sd->down.y = ev->cur.canvas.y;
                 sd->down.dragged_began_timestamp = ev->timestamp / 1000.0;
              }
            // TODO 다른조건들도 can_scroll 안쪽으로 넣는다?
            if ((((_efl_ui_scroll_manager_can_scroll(sd, sd->down.hdir) || sd->bounce_horiz) && sd->down.dir_x) ||
                ((_efl_ui_scroll_manager_can_scroll(sd, sd->down.vdir) || sd->bounce_vert) && sd->down.dir_y)))
              {
                 _efl_ui_scroll_manager_drag_start(sd);
                 sd->down.dragged = EINA_TRUE;
                 ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
              }
            else if (!sd->down.dragged)
              return EINA_FALSE;
         }
      return EINA_TRUE;
    }

    if (sd->down.want_reset)
      {
         sd->down.x = ev->cur.canvas.x;
         sd->down.y = ev->cur.canvas.y;
         sd->down.want_reset = EINA_FALSE;
      }

    if (sd->down.dir_x) dx = sd->down.sx - (ev->cur.canvas.x - sd->down.x);
    else dx = sd->down.sx;
    if (sd->down.dir_y) dy = sd->down.sy - (ev->cur.canvas.y - sd->down.y);
    else dy = sd->down.sy;

    _efl_ui_scroll_manager_bounce_weight_apply(sd, &dx, &dy);
    _scroll_manager_hold_animator_add(sd, dx, dy);

    return EINA_TRUE;
}


static Eina_Bool
_efl_ui_scroll_manager_post_event_move(void *data,
                            Evas *e EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Move *ev = sd->event_info;
   sd->event_info = NULL;
   Evas_Coord dx, dy;

   dx = ev->cur.canvas.x - sd->down.x;
   dy = ev->cur.canvas.y - sd->down.y;

   _efl_ui_scroll_manager_post_event_move_direction_restrict_eval(sd, ev, dx, dy);

   if (!sd->freeze)
     {
        if (!sd->hold)
          {
             if (!_efl_ui_scroll_manager_post_event_move_hold_eval(sd, ev, dx, dy))
               return EINA_TRUE;
          }
        else
          {
             _efl_ui_scroll_manager_post_event_move_on_hold_eval(sd, ev);
          }
     }

   return EINA_FALSE;
}

static void
_efl_ui_scroll_manager_down_coord_eval(Efl_Ui_Scroll_Manager_Data *sd,
                            Evas_Coord *x,
                            Evas_Coord *y)
{
   if (!sd->pan_obj) return;

   if (sd->down.dir_x) *x = sd->down.sx - (*x - sd->down.x);
   else *x = sd->down.sx;
   if (sd->down.dir_y) *y = sd->down.sy - (*y - sd->down.y);
   else *y = sd->down.sy;

   _efl_ui_scroll_manager_bounce_weight_apply(sd, x, y);
}

static Eina_Bool
_efl_ui_scroll_manager_hold_enterer(void *data)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Coord fx = 0, fy = 0;

   sd->down.hold_enterer = NULL;
   fx = sd->down.hold_x;
   fy = sd->down.hold_y;

   if ((_elm_config->scroll_smooth_amount > 0.0) &&
       (_elm_config->scroll_smooth_time_window > 0.0))
     {
        int i, count = 0;
        Evas_Coord basex = 0, basey = 0, x, y;
        double dt, tdiff, tnow, twin, ttot;
        double xx, yy, tot;
        struct
          {
             Evas_Coord x, y;
             double t;
          } pos[100];

        tdiff = sd->down.est_timestamp_diff;
        tnow = ecore_loop_time_get();
        twin = _elm_config->scroll_smooth_time_window;
        for (i = 0; i < 60; i++)
          {
             if ((sd->down.history[i].timestamp - tdiff) > tnow)
               continue;
             if ((sd->down.history[i].timestamp >
                 sd->down.dragged_began_timestamp) || (count == 0))
               {
                  x = sd->down.history[i].x;
                  y = sd->down.history[i].y;
                  _efl_ui_scroll_manager_down_coord_eval(sd, &x, &y);
                  if (count == 0)
                    {
                       basex = x;
                       basey = y;
                    }
                  dt = (tnow + tdiff) - sd->down.history[i].timestamp;
                  if ((dt > twin) && (count > 0)) break;
                  if ((dt > 0.0) && (count == 0))
                    {
                       pos[count].x = x - basex;
                       pos[count].y = y - basey;
                       pos[count].t = 0.0;
                       count++;
                    }
                  pos[count].x = x - basex;
                  pos[count].y = y - basey;
                  pos[count].t = dt;
                  count++;
               }
          }
        if (count > 0)
          {
             xx = 0.0;
             yy = 0.0;
             tot = 0.0;
             ttot = pos[count - 1].t;
             for (i = 0; i < count; i++)
               {
                  double wt;

                  if (ttot > 0.0)
                    {
                       if (i < (count - 1))
                         wt = (ttot - pos[i].t) * (pos[i + 1].t - pos[i].t);
                       else
                         wt = 0.0;
                    }
                  else wt = 1.0;

                  xx += ((double)(pos[i].x)) * wt;
                  yy += ((double)(pos[i].y)) * wt;
                  tot += wt;
               }
             if (tot > 0.0)
               {
                  xx = basex + (xx / tot);
                  yy = basey + (yy / tot);
                  fx =
                    (_elm_config->scroll_smooth_amount * xx) +
                    ((1.0 - _elm_config->scroll_smooth_amount) * fx);
                  fy =
                    (_elm_config->scroll_smooth_amount * yy) +
                    ((1.0 - _elm_config->scroll_smooth_amount) * fy);
               }
          }
     }

   Eina_Position2D cur = efl_ui_scrollable_content_pos_get(sd->obj);
   if (sd->down.dir_x)
     cur.x = fx;
   if (sd->down.dir_y)
     cur.y = fy;

   efl_ui_scrollable_content_pos_set(sd->obj, cur);

   return EINA_FALSE;
}

static void
_efl_ui_scroll_manager_hold_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;

   ecore_idle_enterer_del(sd->down.hold_enterer);
   sd->down.hold_enterer =
     ecore_idle_enterer_before_add(_efl_ui_scroll_manager_hold_enterer, sd);
}

static void
_efl_ui_scroll_manager_on_hold_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   double t, td;
   double vx, vy;
   Evas_Coord x, y;
   Eina_Position2D cur = {0, 0};
   Efl_Ui_Scroll_Manager_Data *sd;

   sd = data;
   t = ecore_loop_time_get();
   if (sd->down.onhold_tlast > 0.0)
     {
        td = t - sd->down.onhold_tlast;
        vx = sd->down.onhold_vx * td *
          (double)_elm_config->thumbscroll_hold_threshold * 2.0;
        vy = sd->down.onhold_vy * td *
          (double)_elm_config->thumbscroll_hold_threshold * 2.0;
        cur = efl_ui_scrollable_content_pos_get(sd->obj);
        x = cur.x;
        y = cur.y;

        if (sd->down.dir_x)
          {
             sd->down.onhold_vxe += vx;
             x = cur.x + (int)sd->down.onhold_vxe;
             sd->down.onhold_vxe -= (int)sd->down.onhold_vxe;
          }

        if (sd->down.dir_y)
          {
             sd->down.onhold_vye += vy;
             y = cur.y + (int)sd->down.onhold_vye;
             sd->down.onhold_vye -= (int)sd->down.onhold_vye;
          }

        efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(x, y));
     }
   sd->down.onhold_tlast = t;
}

static void
_efl_ui_scroll_manager_mouse_move_event_cb(void *data,
                                Evas *e,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Evas_Event_Mouse_Move *ev;
   Eina_Position2D cur = {0, 0};

   if (!sd->pan_obj) return;
   if (!_scroll_manager_thumb_scrollable_get(sd)) return;
   if (!sd->down.now) return;

   ev = event_info;

   if ((!sd->hold) && (!sd->freeze))
     {
        if (_scroll_manager_animating_get(sd))
          {
            _scroll_manager_animators_drop(sd->obj);
            cur = efl_ui_pan_position_get(sd->pan_obj);
            sd->down.sx = cur.x;
            sd->down.sy = cur.y;
            sd->down.x = sd->down.history[0].x;
            sd->down.y = sd->down.history[0].y;
          }
   }

#ifdef SCROLLDBG
   DBG("::: %i %i\n", ev->cur.canvas.x, ev->cur.canvas.y);
#endif
   memmove(&(sd->down.history[1]), &(sd->down.history[0]),
           sizeof(sd->down.history[0]) * (60 - 1));
   sd->down.history[0].timestamp = ev->timestamp / 1000.0;
   sd->down.history[0].x = ev->cur.canvas.x;
   sd->down.history[0].y = ev->cur.canvas.y;
   sd->event_info = event_info;

   if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
     evas_post_event_callback_push(e, _efl_ui_scroll_manager_post_event_move, sd);

   if (sd->down.dragged)
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_scroll_event_object_attach(Evas_Object *obj)
{
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(obj, sd);

   evas_object_event_callback_add
     (sd->event_rect, EVAS_CALLBACK_MOUSE_WHEEL, _efl_ui_scroll_manager_wheel_event_cb,
     sd);
   evas_object_event_callback_add
     (sd->event_rect, EVAS_CALLBACK_MOUSE_DOWN,
     _efl_ui_scroll_manager_mouse_down_event_cb, sd);
   evas_object_event_callback_add
     (sd->event_rect, EVAS_CALLBACK_MOUSE_UP,
     _efl_ui_scroll_manager_mouse_up_event_cb, sd);
   evas_object_event_callback_add
     (sd->event_rect, EVAS_CALLBACK_MOUSE_MOVE,
     _efl_ui_scroll_manager_mouse_move_event_cb, sd);
}

static void
_efl_ui_scroll_manager_pan_content_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;
   Eina_Size2D content = {0, 0};

   if (!sd->pan_obj) return;

   content = efl_ui_pan_content_size_get(sd->pan_obj);
   if ((content.w != sd->content_info.w) || (content.h != sd->content_info.h))
     {
        sd->content_info.w = content.w;
        sd->content_info.h = content.h;

        sd->content_info.resized = EINA_TRUE;
        efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED, NULL);
        efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED, NULL);
        efl_ui_scrollbar_bar_visibility_update(sd->obj);
        _efl_ui_scroll_manager_wanted_region_set(sd->obj);
     }
}

static void
_efl_ui_scroll_manager_pan_viewport_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;

   if (!sd->pan_obj) return;

   efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED, NULL);
   efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED, NULL);
   efl_ui_scrollbar_bar_visibility_update(sd->obj);
   _efl_ui_scroll_manager_wanted_region_set(sd->obj);
}

static void
_efl_ui_scroll_manager_pan_position_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Scroll_Manager_Data *sd = data;

   if (!sd->pan_obj) return;

   efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED, NULL);
   efl_ui_scrollbar_bar_visibility_update(sd->obj);
}

static void
_efl_ui_scroll_manager_pan_resized_cb(void *data,
                                      Evas *e EINA_UNUSED,
                                      Evas_Object *obj EINA_UNUSED,
                                      void *event_info EINA_UNUSED)
{
   Eo *manager = data;
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(manager, sd);

   efl_gfx_entity_size_set(sd->event_rect, efl_gfx_entity_size_get(obj));
}

static void
_efl_ui_scroll_manager_pan_moved_cb(void *data,
                                      Evas *e EINA_UNUSED,
                                      Evas_Object *obj EINA_UNUSED,
                                      void *event_info EINA_UNUSED)
{
   Eo *manager = data;
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(manager, sd);

   efl_gfx_entity_position_set(sd->event_rect, efl_gfx_entity_position_get(obj));
}

static void
_efl_ui_scroll_manager_scrollbar_h_visibility_apply(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (sd->hbar_visible)
     {
        Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
        efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW, &type);
     }
   else
     {
        Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
        efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE, &type);
     }
}

static void
_efl_ui_scroll_manager_scrollbar_v_visibility_apply(Efl_Ui_Scroll_Manager_Data *sd)
{
   if (sd->vbar_visible)
     {
        Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
        efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW, &type);
     }
   else
     {
        Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
        efl_event_callback_call(sd->parent, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE, &type);
     }
}

static void
_efl_ui_scrollbar_h_visibility_adjust(Eo *obj)
{
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(obj, sd);
   int scroll_h_vis_change = 0;
   Evas_Coord w;

   w = sd->content_info.w;
   Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(sd->obj);

   switch (sd->hbar_mode)
     {
      case EFL_UI_SCROLLBAR_MODE_AUTO:
         if (sd->hbar_visible)
           {
              if (w <= view.w)
                {
                   scroll_h_vis_change = 1;
                   sd->hbar_visible = EINA_FALSE;
                }
           }
         else
           {
              if (w > view.w)
                {
                   scroll_h_vis_change = 1;
                   sd->hbar_visible = EINA_TRUE;
                }
           }
         break;
      case EFL_UI_SCROLLBAR_MODE_ON:
         if (!sd->hbar_visible)
           {
              scroll_h_vis_change = 1;
              sd->hbar_visible = EINA_TRUE;
           }
         break;
      case EFL_UI_SCROLLBAR_MODE_OFF:
         if (sd->hbar_visible)
           {
              scroll_h_vis_change = 1;
              sd->hbar_visible = EINA_FALSE;
           }
         break;
      default:
         break;
     }

   if (scroll_h_vis_change) _efl_ui_scroll_manager_scrollbar_h_visibility_apply(sd);
}

static void
_efl_ui_scrollbar_v_visibility_adjust(Eo *obj)
{
   EFL_UI_SCROLL_MANAGER_DATA_GET_OR_RETURN(obj, sd);
   int scroll_v_vis_change = 0;
   Evas_Coord h;

   h = sd->content_info.h;
   Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(sd->obj);

   switch (sd->vbar_mode)
     {
      case EFL_UI_SCROLLBAR_MODE_AUTO:
         if (sd->vbar_visible)
           {
              if (h <= view.h)
                {
                   scroll_v_vis_change = 1;
                   sd->vbar_visible = EINA_FALSE;
                }
           }
         else
           {
              if (h > view.h)
                {
                   scroll_v_vis_change = 1;
                   sd->vbar_visible = EINA_TRUE;
                }
           }
         break;
      case EFL_UI_SCROLLBAR_MODE_ON:
         if (!sd->vbar_visible)
           {
              scroll_v_vis_change = 1;
              sd->vbar_visible = EINA_TRUE;
           }
         break;
      case EFL_UI_SCROLLBAR_MODE_OFF:
         if (sd->vbar_visible)
           {
              scroll_v_vis_change = 1;
              sd->vbar_visible = EINA_FALSE;
           }
         break;
      default:
         break;
     }
   if (scroll_v_vis_change) _efl_ui_scroll_manager_scrollbar_v_visibility_apply(sd);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollbar_bar_visibility_update(Eo *obj, Efl_Ui_Scroll_Manager_Data *sd EINA_UNUSED)
{
   _efl_ui_scrollbar_h_visibility_adjust(obj);
   _efl_ui_scrollbar_v_visibility_adjust(obj);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollbar_bar_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, double posx, double posy)
{
   Evas_Coord x, y;
   Eina_Position2D min = {0, 0}, max = {0, 0};

   if (sd->down.dragged || _scroll_manager_animating_get(sd)) return;

   max = efl_ui_pan_position_max_get(sd->pan_obj);
   min = efl_ui_pan_position_min_get(sd->pan_obj);
   x = _round(posx * (double)max.x + min.x, 1);
   y = _round(posy * (double)max.y + min.y, 1);
   efl_ui_scrollable_content_pos_set(sd->obj, EINA_POSITION2D(x, y));
   _efl_ui_scroll_manager_wanted_coordinates_update(sd, x, y);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollbar_bar_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, double *posx, double *posy)
{
   if (!sd->pan_obj) return;

   double vx = 0, vy = 0;
   Eina_Position2D min = {0, 0}, max = {0, 0}, cur = {0, 0};

   min = efl_ui_pan_position_min_get(sd->pan_obj);
   max = efl_ui_pan_position_max_get(sd->pan_obj);
   cur = efl_ui_pan_position_get(sd->pan_obj);

   if (max.x > 0) vx = (double)(cur.x - min.x) / (double)max.x;
   else vx = 0.0;

   if (vx < 0.0) vx = 0.0;
   else if (vx > 1.0)
     vx = 1.0;

   if (posx) *posx = vx;

   if (max.y > 0) vy = (double)(cur.y - min.y) / (double)max.y;
   else vy = 0.0;

   if (vy < 0.0) vy = 0.0;
   else if (vy > 1.0)
     vy = 1.0;

   if (posy) *posy = vy;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollbar_bar_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, double *width, double *height)
{
   if (!sd->pan_obj) return;

   Evas_Coord w = 0, h = 0, vw = 0, vh = 0;
   double size = 0.0;

   Eina_Rect r = efl_ui_scrollable_viewport_geometry_get(sd->obj);
   vw = r.w;
   vh = r.h;

   w = sd->content_info.w;
   if (w < 1) w = 1;
   size = (double)vw / (double)w;
   if (size > 1.0) size = 1.0;

   if (width) *width = size;

   h = sd->content_info.h;
   if (h < 1) h = 1;
   size = (double)vh / (double)h;
   if (size > 1.0) size = 1.0;

   if (height) *height = size;
}

EOLIAN static void
_efl_ui_scroll_manager_pan_set(Eo *obj, Efl_Ui_Scroll_Manager_Data *sd, Eo *pan)
{
   if (sd->pan_obj)
     {
        efl_event_callback_del
           (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_CONTENT_CHANGED, _efl_ui_scroll_manager_pan_content_changed_cb, sd);
        efl_event_callback_del
           (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_VIEWPORT_CHANGED, _efl_ui_scroll_manager_pan_viewport_changed_cb, sd);
        efl_event_callback_del
           (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_POSITION_CHANGED, _efl_ui_scroll_manager_pan_position_changed_cb, sd);
     }

   if (!pan)
     return;

   sd->pan_obj = pan;

   efl_event_callback_add
     (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_CONTENT_CHANGED, _efl_ui_scroll_manager_pan_content_changed_cb, sd);
   efl_event_callback_add
     (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_VIEWPORT_CHANGED, _efl_ui_scroll_manager_pan_viewport_changed_cb, sd);
   efl_event_callback_add
     (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_POSITION_CHANGED, _efl_ui_scroll_manager_pan_position_changed_cb, sd);
   evas_object_event_callback_add(sd->pan_obj, EVAS_CALLBACK_RESIZE,
                                       _efl_ui_scroll_manager_pan_resized_cb, obj);
   evas_object_event_callback_add(sd->pan_obj, EVAS_CALLBACK_MOVE,
                                       _efl_ui_scroll_manager_pan_moved_cb, obj);
}

EOLIAN static Eina_Bool
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_scroll_hold_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd)
{
   return sd->hold;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_scroll_hold_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool hold)
{
   sd->hold = hold;
}

EOLIAN static Eina_Bool
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_scroll_freeze_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd)
{
   return sd->freeze;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_scroll_freeze_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool freeze)
{
   sd->freeze = freeze;
   if (sd->freeze)
     {
        _scroll_manager_on_hold_animator_del(sd);
     }
   else
     _efl_ui_scroll_manager_bounce_eval(sd);
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_bounce_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool horiz, Eina_Bool vert)
{
   sd->bounce_horiz = !!horiz;
   sd->bounce_vert = !!vert;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_bounce_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool *horiz, Eina_Bool *vert)
{
   if (horiz) *horiz = sd->bounce_horiz;
   if (vert) *vert = sd->bounce_vert;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_scroll(Eo *obj, Efl_Ui_Scroll_Manager_Data *sd, Eina_Rect rect, Eina_Bool animation)
{
   _scroll_manager_animators_drop(obj);
   if (animation)
     {
        if (_efl_ui_scroll_manager_content_region_show_internal(obj, &(rect.x), &(rect.y), rect.w, rect.h))
          {
             _scroll_manager_scrollto(sd, rect.x, rect.y);
          }
     }
   else
     {
        sd->wx = (sd->is_mirrored ? _efl_ui_scroll_manager_x_mirrored_get(sd->obj, rect.x) : rect.x);
        sd->wy = rect.y;
        sd->ww = rect.w;
        sd->wh = rect.h;
        if (_efl_ui_scroll_manager_content_region_show_internal(obj, &(rect.x), &(rect.y), rect.w, rect.h))
          {
             efl_ui_scrollable_content_pos_set(obj, EINA_POSITION2D(rect.x, rect.y));
             sd->down.sx = rect.x;
             sd->down.sy = rect.y;
             sd->down.x = sd->down.history[0].x;
             sd->down.y = sd->down.history[0].y;
          }
     }
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_gravity_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, double x, double y)
{
   sd->gravity_x = x;
   sd->gravity_y = y;
   Eina_Position2D max = efl_ui_pan_position_max_get(sd->pan_obj);
   sd->prev_cw = max.x;
   sd->prev_ch = max.y;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_gravity_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, double *x, double *y)
{
   if (x) *x = sd->gravity_x;
   if (y) *y = sd->gravity_y;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_movement_block_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Efl_Ui_Scroll_Block block)
{
   sd->block = block;
}

EOLIAN static Efl_Ui_Scroll_Block
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_movement_block_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd)
{
   return sd->block;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_looping_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool loop_h, Eina_Bool loop_v)
{
   if (sd->loop_h == loop_h && sd->loop_v == loop_v) return;

   sd->loop_h = loop_h;
   sd->loop_v = loop_v;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_ui_scrollable_interactive_looping_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Manager_Data *sd, Eina_Bool *loop_h, Eina_Bool *loop_v)
{
   *loop_h = sd->loop_h;
   *loop_v = sd->loop_v;
}

EOLIAN static Eo *
_efl_ui_scroll_manager_efl_object_constructor(Eo *obj, Efl_Ui_Scroll_Manager_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   memset(sd, 0, sizeof(*sd));

   sd->parent = efl_parent_get(obj);

   sd->obj = obj;

   sd->step.x = 32 * elm_config_scale_get();
   sd->step.y = 32 * elm_config_scale_get();
   sd->page.x = -50;
   sd->page.y = -50;
   sd->loop_h = EINA_FALSE;
   sd->loop_v = EINA_FALSE;

   sd->hbar_mode = EFL_UI_SCROLLBAR_MODE_AUTO;
   sd->vbar_mode = EFL_UI_SCROLLBAR_MODE_AUTO;
   sd->hbar_visible = EINA_TRUE;
   sd->vbar_visible = EINA_TRUE;

   sd->bounce_horiz = _elm_config->thumbscroll_bounce_enable;
   sd->bounce_vert = _elm_config->thumbscroll_bounce_enable;

   sd->block = EFL_UI_SCROLL_BLOCK_NONE;
   sd->scrolling = EINA_FALSE;

   sd->event_rect = evas_object_rectangle_add(evas_object_evas_get(sd->parent));
   efl_canvas_group_member_add(sd->parent, sd->event_rect);
   efl_ui_widget_sub_object_add(sd->parent, sd->event_rect);

   efl_gfx_color_set(sd->event_rect, 0, 0, 0, 0);
   efl_gfx_entity_visible_set(sd->event_rect, EINA_TRUE);
   efl_canvas_object_repeat_events_set(sd->event_rect, EINA_TRUE);

   _scroll_event_object_attach(obj);

   //FIXME : mostly bar-related callback is added after scroll manager creation,
   // so when constructor of manager is called, callback is not registered.
   //efl_ui_scrollbar_bar_visibility_update(sd->obj);

   return obj;
}

EOLIAN static void
_efl_ui_scroll_manager_efl_object_destructor(Eo *obj, Efl_Ui_Scroll_Manager_Data *sd)
{
   ecore_idle_enterer_del(sd->down.hold_enterer);
   ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->down.hold_animator, _efl_ui_scroll_manager_hold_animator, sd);
   ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->down.onhold_animator, _efl_ui_scroll_manager_on_hold_animator, sd);
   ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->bounce.x.animator, _efl_ui_scroll_manager_bounce_x_animator, sd);
   ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->bounce.y.animator, _efl_ui_scroll_manager_bounce_y_animator, sd);
   ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.x.animator, _efl_ui_scroll_manager_scroll_to_x_animator, sd);
   ELM_ANIMATOR_DISCONNECT(sd->event_rect, sd->scrollto.y.animator, _efl_ui_scroll_manager_scroll_to_y_animator, sd);

   if (!efl_invalidating_get(sd->pan_obj))
     {
        evas_object_event_callback_del_full(sd->pan_obj, EVAS_CALLBACK_RESIZE,
                                            _efl_ui_scroll_manager_pan_resized_cb, obj);
        evas_object_event_callback_del_full(sd->pan_obj, EVAS_CALLBACK_MOVE,
                                            _efl_ui_scroll_manager_pan_moved_cb, obj);
        efl_event_callback_del
           (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_CONTENT_CHANGED, _efl_ui_scroll_manager_pan_content_changed_cb, sd);
        efl_event_callback_del
           (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_VIEWPORT_CHANGED, _efl_ui_scroll_manager_pan_viewport_changed_cb, sd);
        efl_event_callback_del
           (sd->pan_obj, EFL_UI_PAN_EVENT_PAN_POSITION_CHANGED, _efl_ui_scroll_manager_pan_position_changed_cb, sd);
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "efl_ui_scroll_manager.eo.c"
