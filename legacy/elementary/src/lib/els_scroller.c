#include <Elementary.h>
#include "elm_priv.h"
#include "els_scroller.h"
#include "els_pan.h"

#define SMART_NAME "els_scroller"
#define API_ENTRY Smart_Data *sd; sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY Smart_Data *sd; sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _Smart_Data Smart_Data;

#define EVTIME 1
//#define SCROLLDBG 1

struct _Smart_Data
{
   Evas_Coord   x, y, w, h;
   Evas_Coord   wx, wy, ww, wh; /* Last "wanted" geometry */

   Evas_Object *smart_obj;
   Evas_Object *child_obj;
   Evas_Object *pan_obj;
   Evas_Object *edje_obj;
   Evas_Object *event_obj;

   Evas_Object *widget;

   Elm_Smart_Scroller_Policy hbar_flags, vbar_flags;

   struct {
        Evas_Coord x, y;
        Evas_Coord sx, sy;
        Evas_Coord dx, dy;
        Evas_Coord pdx, pdy;
        Evas_Coord bx, by;
        Evas_Coord ax, ay;
        Evas_Coord bx0, by0;
        Evas_Coord b0x, b0y;
        Evas_Coord b2x, b2y;
        struct {
             Evas_Coord    x, y;
             double        timestamp, localtimestamp;
        } history[60];
        struct {
           double tadd, dxsum, dysum;
           double est_timestamp_diff;
        } hist;
      double anim_start;
      double anim_start2;
      double anim_start3;
      double onhold_vx, onhold_vy, onhold_tlast, onhold_vxe, onhold_vye;
      double extra_time;
      Evas_Coord hold_x, hold_y;
      Ecore_Animator *hold_animator;
      Ecore_Animator *onhold_animator;
      Ecore_Animator *momentum_animator;
      Ecore_Animator *bounce_x_animator;
      Ecore_Animator *bounce_y_animator;
      Evas_Coord locked_x, locked_y;
      int hdir, vdir;
      unsigned char now : 1;
      unsigned char cancelled : 1;
      unsigned char hold : 1;
      unsigned char hold_parent : 1;
      unsigned char want_dragged : 1;
      unsigned char dragged : 1;
      unsigned char dragged_began : 1;
      unsigned char dir_x : 1;
      unsigned char dir_y : 1;
      unsigned char locked : 1;
      unsigned char bounce_x_hold : 1;
      unsigned char bounce_y_hold : 1;
      unsigned char scroll : 1;
   } down;

   struct {
      Evas_Coord w, h;
      Eina_Bool resized : 1;
   } child;
   struct {
      Evas_Coord x, y;
   } step, page;

   struct {
      void (*set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
      void (*get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
      void (*max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
      void (*min_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
      void (*child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
      void (*gravity_set) (Evas_Object *obj, double x, double y);
      void (*gravity_get) (Evas_Object *obj, double *x, double *y);
   } pan_func;

   struct {
      struct {
         Evas_Coord start, end;
         double t_start, t_end;
         Ecore_Animator *animator;
      } x, y;
   } scrollto;

   double pagerel_h, pagerel_v;
   Evas_Coord pagesize_h, pagesize_v;

   unsigned char hbar_visible : 1;
   unsigned char vbar_visible : 1;
   unsigned char extern_pan : 1;
   unsigned char one_dir_at_a_time : 1;
   unsigned char hold : 1;
   unsigned char freeze : 1;
   unsigned char bouncemex : 1;
   unsigned char bouncemey : 1;
   unsigned char bounce_horiz : 1;
   unsigned char bounce_vert : 1;
   Eina_Bool momentum_animator_disabled :1;
   Eina_Bool bounce_animator_disabled :1;
   Eina_Bool is_mirrored : 1;
   Eina_Bool wheel_disabled : 1;
};

/* local subsystem functions */
static void _smart_child_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_pan_changed_hook(void *data, Evas_Object *obj, void *event_info);
static void _smart_pan_pan_changed_hook(void *data, Evas_Object *obj, void *event_info);
static void _smart_event_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_event_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool  _smart_hold_animator(void *data);
static Eina_Bool  _smart_momentum_animator(void *data);
static void _smart_event_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool  _smart_onhold_animator(void *data);
static void _smart_event_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_edje_drag_v_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_edje_drag_v_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_edje_drag_v(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_edje_drag_h_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_edje_drag_h_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_edje_drag_h(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_scrollbar_read(Smart_Data *sd);
static void _smart_scrollbar_reset(Smart_Data *sd);
static int  _smart_scrollbar_bar_h_visibility_adjust(Smart_Data *sd);
static int  _smart_scrollbar_bar_v_visibility_adjust(Smart_Data *sd);
static void _smart_scrollbar_bar_visibility_adjust(Smart_Data *sd);
static void _smart_scrollbar_size_adjust(Smart_Data *sd);
static void _smart_reconfigure(Smart_Data *sd);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _smart_clip_unset(Evas_Object *obj);
static void _smart_init(void);

static void _elm_smart_scroller_wanted_region_set(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_smart = NULL;

/* externally accessible functions */
Evas_Object *
elm_smart_scroller_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _smart);
}

static Evas_Coord
_elm_smart_scroller_x_mirrored_get(Evas_Object *obj, Evas_Coord x)
{
   API_ENTRY return x;

   Evas_Coord cw, ch, w, ret;
   elm_smart_scroller_child_viewport_size_get(obj, &w, NULL);
   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);
   ret = (cw - (x + w));
   return (ret >= 0) ? ret : 0;
}

void
elm_smart_scroller_mirrored_set(Evas_Object *obj, Eina_Bool mirrored)
{
   API_ENTRY return;
   Evas_Coord wx;
   if (sd->is_mirrored == mirrored)
     return;

   sd->is_mirrored = mirrored;
   edje_object_mirrored_set(sd->edje_obj, mirrored);

   if (sd->is_mirrored)
     wx = _elm_smart_scroller_x_mirrored_get(sd->smart_obj, sd->wx);
   else
     wx = sd->wx;

   elm_smart_scroller_child_pos_set(sd->smart_obj, wx, sd->wy);
}

void
elm_smart_scroller_child_set(Evas_Object *obj, Evas_Object *child)
{
   Evas_Coord w, h;
   Evas_Object *o;

   API_ENTRY return;
   if (sd->child_obj)
     {
        _elm_smart_pan_child_set(sd->pan_obj, NULL);
        evas_object_event_callback_del_full(sd->child_obj, EVAS_CALLBACK_DEL, _smart_child_del_hook, sd);
     }

   sd->child_obj = child;
   sd->wx = sd->wy = 0;
   /* (-1) means want viewports size */
   sd->ww = sd->wh = -1;
   if (!child) return;

   if (!sd->pan_obj)
     {
        o = _elm_smart_pan_add(evas_object_evas_get(obj));
        sd->pan_obj = o;
        evas_object_smart_callback_add(o, "changed", _smart_pan_changed_hook, sd);
        evas_object_smart_callback_add(o, "pan_changed", _smart_pan_pan_changed_hook, sd);
        edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", o);
     }

   sd->pan_func.set = _elm_smart_pan_set;
   sd->pan_func.get = _elm_smart_pan_get;
   sd->pan_func.max_get = _elm_smart_pan_max_get;
   sd->pan_func.min_get = _elm_smart_pan_min_get;
   sd->pan_func.child_size_get = _elm_smart_pan_child_size_get;
   sd->pan_func.gravity_set = _elm_smart_pan_gravity_set;
   sd->pan_func.gravity_get = _elm_smart_pan_gravity_get;

   evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _smart_child_del_hook, sd);
   _elm_smart_pan_child_set(sd->pan_obj, child);
   sd->pan_func.child_size_get(sd->pan_obj, &w, &h);
   sd->child.w = w;
   sd->child.h = h;
   _smart_scrollbar_size_adjust(sd);
   _smart_scrollbar_reset(sd);
}

void
elm_smart_scroller_extern_pan_set(Evas_Object *obj, Evas_Object *pan,
                                  void (*pan_set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y),
                                  void (*pan_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y),
                                  void (*pan_max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y),
                                  void (*pan_min_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y),
                                  void (*pan_child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y))
{
   API_ENTRY return;

   elm_smart_scroller_child_set(obj, NULL);

   if (sd->pan_obj)
     {
        evas_object_smart_callback_del(sd->pan_obj, "changed", _smart_pan_changed_hook);
        evas_object_smart_callback_del(sd->pan_obj, "pan_changed", _smart_pan_pan_changed_hook);
     }

   if (sd->extern_pan)
     {
        if (sd->pan_obj)
          {
             edje_object_part_unswallow(sd->edje_obj, sd->pan_obj);
             sd->pan_obj = NULL;
          }
     }
   else
     {
        if (sd->pan_obj)
          {
             evas_object_del(sd->pan_obj);
             sd->pan_obj = NULL;
          }
     }
   if (!pan)
     {
        sd->extern_pan = 0;
        return;
     }

   sd->pan_obj = pan;
   sd->pan_func.set = pan_set;
   sd->pan_func.get = pan_get;
   sd->pan_func.max_get = pan_max_get;
   sd->pan_func.min_get = pan_min_get;
   sd->pan_func.child_size_get = pan_child_size_get;
   sd->pan_func.gravity_set = _elm_smart_pan_gravity_set;
   sd->pan_func.gravity_get = _elm_smart_pan_gravity_get;
   sd->extern_pan = 1;
   evas_object_smart_callback_add(sd->pan_obj, "changed", _smart_pan_changed_hook, sd);
   evas_object_smart_callback_add(sd->pan_obj, "pan_changed", _smart_pan_pan_changed_hook, sd);
   edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", sd->pan_obj);
   evas_object_show(sd->pan_obj);
}

void
elm_smart_scroller_custom_edje_file_set(Evas_Object *obj, char *file, char *group)
{
   API_ENTRY return;

   edje_object_file_set(sd->edje_obj, file, group);
   if (sd->pan_obj)
     edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", sd->pan_obj);
   sd->vbar_visible = !sd->vbar_visible;
   sd->hbar_visible = !sd->hbar_visible;
   _smart_scrollbar_bar_visibility_adjust(sd);
   if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,hbar", "elm");
   else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,hbar", "elm");
   if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,vbar", "elm");
   else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,vbar", "elm");
}

Eina_Bool
elm_smart_scroller_momentum_animator_disabled_get(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->momentum_animator_disabled;
}

void
elm_smart_scroller_momentum_animator_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   API_ENTRY return;
   sd->momentum_animator_disabled = disabled;
   if (sd->momentum_animator_disabled)
     {
        if (sd->down.momentum_animator)
          {
             ecore_animator_del(sd->down.momentum_animator);
             sd->down.momentum_animator = NULL;
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
          }
     }
}

Eina_Bool
elm_smart_scroller_bounce_animator_disabled_get(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->bounce_animator_disabled;
}

void
elm_smart_scroller_bounce_animator_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   API_ENTRY return;
   sd->bounce_animator_disabled = disabled;
   if (sd->bounce_animator_disabled)
     {
        if (sd->scrollto.x.animator)
          {
             ecore_animator_del(sd->scrollto.x.animator);
             sd->scrollto.x.animator = NULL;
          }

        if (sd->scrollto.y.animator)
          {
             ecore_animator_del(sd->scrollto.y.animator);
             sd->scrollto.y.animator = NULL;
          }
     }
}

Eina_Bool
elm_smart_scroller_wheel_disabled_get(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->wheel_disabled;
}

void
elm_smart_scroller_wheel_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   API_ENTRY return;
   if ((!sd->wheel_disabled) && (disabled))
     evas_object_event_callback_del_full(sd->event_obj, EVAS_CALLBACK_MOUSE_WHEEL, _smart_event_wheel, sd);
   else if ((sd->wheel_disabled) && (!disabled))
     evas_object_event_callback_add(sd->event_obj, EVAS_CALLBACK_MOUSE_WHEEL, _smart_event_wheel, sd);
   sd->wheel_disabled = disabled;
}

/* Update the wanted coordinates according to the x, y passed
 * widget directionality, child size and etc. */
static void
_update_wanted_coordinates(Smart_Data *sd, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord cw, ch;

   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);

   /* Update wx/y/w/h - and if the requested positions aren't legal
    * adjust a bit. */
   elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &sd->ww, &sd->wh);
   if (x < 0)
     sd->wx = 0;
   else if ((x + sd->ww) > cw)
     sd->wx = cw - sd->ww;
   else if (sd->is_mirrored)
     sd->wx = _elm_smart_scroller_x_mirrored_get(sd->smart_obj, x);
   else
     sd->wx = x;
   if (y < 0) sd->wy = 0;
   else if ((y + sd->wh) > ch) sd->wy = ch - sd->wh;
   else sd->wy = y;
}

static void
_smart_anim_start(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "animate,start", NULL);
}

static void
_smart_anim_stop(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "animate,stop", NULL);
}

static void
_smart_drag_start(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "drag,start", NULL);
}

static void
_smart_drag_stop(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "drag,stop", NULL);
}

static Eina_Bool
_smart_scrollto_x_animator(void *data)
{
   Smart_Data *sd = data;
   Evas_Coord px, py;
   double t, tt;

   t = ecore_loop_time_get();
   tt = (t - sd->scrollto.x.t_start) / (sd->scrollto.x.t_end - sd->scrollto.x.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   px = (sd->scrollto.x.start * (1.0 - tt)) +
      (sd->scrollto.x.end * tt);
   if (t >= sd->scrollto.x.t_end)
     {
        px = sd->scrollto.x.end;
        elm_smart_scroller_child_pos_set(sd->smart_obj, px, py);
        _update_wanted_coordinates(sd, px, py);
        sd->scrollto.x.animator = NULL;
        if ((!sd->scrollto.y.animator) && (!sd->down.bounce_y_animator))
          _smart_anim_stop(sd->smart_obj);
        return ECORE_CALLBACK_CANCEL;
     }
   elm_smart_scroller_child_pos_set(sd->smart_obj, px, py);
   _update_wanted_coordinates(sd, px, py);
   return ECORE_CALLBACK_RENEW;
}

static void
_smart_momentum_end(Smart_Data *sd)
{
   if ((sd->down.bounce_x_animator) || (sd->down.bounce_y_animator)) return;
   if (sd->down.momentum_animator)
     {
        Evas_Coord px, py;
        elm_smart_scroller_child_pos_get(sd->smart_obj, &px, &py);
        _update_wanted_coordinates(sd, px, py);

        ecore_animator_del(sd->down.momentum_animator);
        sd->down.momentum_animator = NULL;
        sd->down.bounce_x_hold = 0;
        sd->down.bounce_y_hold = 0;
        sd->down.ax = 0;
        sd->down.ay = 0;
        sd->down.dx = 0;
        sd->down.dy = 0;
        sd->down.pdx = 0;
        sd->down.pdy = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
}

static void
_smart_scrollto_x(Smart_Data *sd, double t_in, Evas_Coord pos_x)
{
   Evas_Coord px, py, x, y, w, h;
   double t;

   if (sd->freeze) return;
   if (t_in <= 0.0)
     {
        elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
        elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);
        x = pos_x;
        elm_smart_scroller_child_region_set(sd->smart_obj, x, y, w, h);
        return;
     }
   t = ecore_loop_time_get();
   sd->pan_func.get(sd->pan_obj, &px, &py);
   sd->scrollto.x.start = px;
   sd->scrollto.x.end = pos_x;
   sd->scrollto.x.t_start = t;
   sd->scrollto.x.t_end = t + t_in;
   if (!sd->scrollto.x.animator)
     {
        sd->scrollto.x.animator = ecore_animator_add(_smart_scrollto_x_animator, sd);
        if (!sd->scrollto.y.animator)
          _smart_anim_start(sd->smart_obj);
     }
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        _smart_momentum_end(sd);
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   sd->bouncemex = 0;
}

static Eina_Bool
_smart_scrollto_y_animator(void *data)
{
   Smart_Data *sd = data;
   Evas_Coord px, py;
   double t, tt;

   t = ecore_loop_time_get();
   tt = (t - sd->scrollto.y.t_start) / (sd->scrollto.y.t_end - sd->scrollto.y.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   py = (sd->scrollto.y.start * (1.0 - tt)) +
      (sd->scrollto.y.end * tt);
   if (t >= sd->scrollto.y.t_end)
     {
        py = sd->scrollto.y.end;
        elm_smart_scroller_child_pos_set(sd->smart_obj, px, py);
        _update_wanted_coordinates(sd, px, py);
        sd->scrollto.y.animator = NULL;
        if ((!sd->scrollto.x.animator) && (!sd->down.bounce_x_animator))
          _smart_anim_stop(sd->smart_obj);
        return ECORE_CALLBACK_CANCEL;
     }
   elm_smart_scroller_child_pos_set(sd->smart_obj, px, py);
   _update_wanted_coordinates(sd, px, py);

   return ECORE_CALLBACK_RENEW;
}

static void
_smart_scrollto_y(Smart_Data *sd, double t_in, Evas_Coord pos_y)
{
   Evas_Coord px, py, x, y, w, h;
   double t;

   if (sd->freeze) return;
   if (t_in <= 0.0)
     {
        elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
        elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);
        y = pos_y;
        elm_smart_scroller_child_region_set(sd->smart_obj, x, y, w, h);
        return;
     }
   t = ecore_loop_time_get();
   sd->pan_func.get(sd->pan_obj, &px, &py);
   sd->scrollto.y.start = py;
   sd->scrollto.y.end = pos_y;
   sd->scrollto.y.t_start = t;
   sd->scrollto.y.t_end = t + t_in;
   if (!sd->scrollto.y.animator)
     {
        sd->scrollto.y.animator = ecore_animator_add(_smart_scrollto_y_animator, sd);
        if (!sd->scrollto.x.animator)
          _smart_anim_start(sd->smart_obj);
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        _smart_momentum_end(sd);
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   sd->bouncemey = 0;
}

static Eina_Bool
_smart_do_page(Smart_Data *sd)
{
   if ((sd->pagerel_h == 0.0) && (!sd->pagesize_h) &&
       (sd->pagerel_v == 0.0) && (!sd->pagesize_v))
     return EINA_FALSE;
   return EINA_TRUE;
}

static Evas_Coord
_smart_page_x_get(Smart_Data *sd, int offset)
{
   Evas_Coord x, y, w, h, cw, ch, minx = 0;

   elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
   elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);
   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);
   sd->pan_func.min_get(sd->pan_obj, &minx, NULL);

   x += offset;

   if (sd->pagerel_h > 0.0)
     sd->pagesize_h = w * sd->pagerel_h;
   if (sd->pagesize_h > 0)
     {
        x = x + (sd->pagesize_h * 0.5);
        x = x / (sd->pagesize_h);
        x = x * (sd->pagesize_h);
     }
   if ((x + w) > cw) x = cw - w;
   if (x < minx) x = minx;
   return x;
}

static Evas_Coord
_smart_page_y_get(Smart_Data *sd, int offset)
{
   Evas_Coord x, y, w, h, cw, ch, miny = 0;

   elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
   elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);
   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);
   sd->pan_func.min_get(sd->pan_obj, NULL, &miny);

   y += offset;

   if (sd->pagerel_v > 0.0)
     sd->pagesize_v = h * sd->pagerel_v;
   if (sd->pagesize_v > 0)
     {
        y = y + (sd->pagesize_v * 0.5);
        y = y / (sd->pagesize_v);
        y = y * (sd->pagesize_v);
     }
   if ((y + h) > ch) y = ch - h;
   if (y < miny) y = miny;
   return y;
}

static void
_smart_page_adjust(Smart_Data *sd)
{
   Evas_Coord x, y, w, h;

   if (!_smart_do_page(sd)) return;

   elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);

   x = _smart_page_x_get(sd, 0);
   y = _smart_page_y_get(sd, 0);

   elm_smart_scroller_child_region_set(sd->smart_obj, x, y, w, h);
}

static Eina_Bool
_smart_bounce_x_animator(void *data)
{
   Smart_Data *sd;
   Evas_Coord x, y, dx, w, odx, ed, md;
   double t, p, dt, pd, r;

   sd = data;
   t = ecore_loop_time_get();
   dt = t - sd->down.anim_start2;
   if (dt >= 0.0)
     {
        dt = dt / _elm_config->thumbscroll_bounce_friction;
        odx = sd->down.b2x - sd->down.bx;
        elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, NULL);
        if (!sd->down.momentum_animator && (w > abs(odx)))
          {
             pd = (double)odx / (double)w;
             pd = (pd > 0) ? pd : -pd;
             pd = 1.0 - ((1.0 - pd) * (1.0 - pd));
             dt = dt / pd;
          }
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
        dx = (odx * p);
        r = 1.0;
        if (sd->down.momentum_animator)
          {
             ed = abs(sd->down.dx * (_elm_config->thumbscroll_friction + sd->down.extra_time) - sd->down.b0x);
             md = abs(_elm_config->thumbscroll_friction * 5 * w);
             if (ed > md) r = (double)(md)/(double)ed;
          }
        x = sd->down.b2x + (int)((double)(dx - odx) * r);
        if (!sd->down.cancelled)
          elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
        if (dt >= 1.0)
          {
             if (sd->down.momentum_animator)
               sd->down.bounce_x_hold = 1;
             if ((!sd->down.bounce_y_animator) &&
                 (!sd->scrollto.y.animator))
               _smart_anim_stop(sd->smart_obj);
             sd->down.bounce_x_animator = NULL;
             sd->down.pdx = 0;
             sd->bouncemex = 0;
             _smart_momentum_end(sd);
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
             return ECORE_CALLBACK_CANCEL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_smart_bounce_y_animator(void *data)
{
   Smart_Data *sd;
   Evas_Coord x, y, dy, h, ody, ed, md;
   double t, p, dt, pd, r;

   sd = data;
   t = ecore_loop_time_get();
   dt = t - sd->down.anim_start3;
   if (dt >= 0.0)
     {
        dt = dt / _elm_config->thumbscroll_bounce_friction;
        ody = sd->down.b2y - sd->down.by;
        elm_smart_scroller_child_viewport_size_get(sd->smart_obj, NULL, &h);
        if (!sd->down.momentum_animator && (h > abs(ody)))
          {
             pd = (double)ody / (double)h;
             pd = (pd > 0) ? pd : -pd;
             pd = 1.0 - ((1.0 - pd) * (1.0 - pd));
             dt = dt / pd;
          }
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
        dy = (ody * p);
        r = 1.0;
        if (sd->down.momentum_animator)
          {
             ed = abs(sd->down.dy * (_elm_config->thumbscroll_friction + sd->down.extra_time) - sd->down.b0y);
             md = abs(_elm_config->thumbscroll_friction * 5 * h);
             if (ed > md) r = (double)(md)/(double)ed;
          }
        y = sd->down.b2y + (int)((double)(dy - ody) * r);
        if (!sd->down.cancelled)
          elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
        if (dt >= 1.0)
          {
             if (sd->down.momentum_animator)
               sd->down.bounce_y_hold = 1;
             if ((!sd->down.bounce_x_animator) &&
                 (!sd->scrollto.y.animator))
               _smart_anim_stop(sd->smart_obj);
             sd->down.bounce_y_animator = NULL;
             sd->down.pdy = 0;
             sd->bouncemey = 0;
             _smart_momentum_end(sd);
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
             return ECORE_CALLBACK_CANCEL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
static Eina_Bool
can_scroll(Smart_Data *sd, int dir)
{
   Evas_Coord mx = 0, my = 0, px = 0, py = 0, minx = 0, miny = 0;

   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   switch (dir)
     {
      case LEFT:
        if (px > minx) return EINA_TRUE;
        break;
      case RIGHT:
        if ((px - minx) < mx) return EINA_TRUE;
        break;
      case UP:
        if (py > miny) return EINA_TRUE;
        break;
      case DOWN:
        if ((py - miny) < my) return EINA_TRUE;
        break;
      default:
        break;
     }
   return EINA_FALSE;
}

static Eina_Bool
_smart_momentum_animator(void *data)
{
   Smart_Data *sd;
   double t, dt, p;
   Evas_Coord x, y, dx, dy, px, py, maxx, maxy, minx, miny;
   Eina_Bool no_bounce_x_end = EINA_FALSE, no_bounce_y_end = EINA_FALSE;

   sd = data;
   t = ecore_loop_time_get();
   dt = t - sd->down.anim_start;
   if (dt >= 0.0)
     {
        /*
        if (sd->down.hold_parent)
          {
             if ((sd->down.dir_x) && !can_scroll(sd, sd->down.hdir))
               {
                  sd->down.dir_x = 0;
               }
             if ((sd->down.dir_y) && !can_scroll(sd, sd->down.vdir))
               {
                  sd->down.dir_y = 0;
               }
           }
        if ((!sd->down.dir_x) && (!sd->down.dir_y))
          {
             sd->down.cancelled = 1;
          }
        */
        dt = dt / (_elm_config->thumbscroll_friction + sd->down.extra_time);
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        dx = (sd->down.dx * (_elm_config->thumbscroll_friction + sd->down.extra_time) * p);
        dy = (sd->down.dy * (_elm_config->thumbscroll_friction + sd->down.extra_time) * p);
        sd->down.ax = dx;
        sd->down.ay = dy;
        x = sd->down.sx - dx;
        y = sd->down.sy - dy;
        elm_smart_scroller_child_pos_get(sd->smart_obj, &px, &py);
        if ((sd->down.bounce_x_animator) ||
            (sd->down.bounce_x_hold))
          {
             sd->down.bx = sd->down.bx0 - dx + sd->down.b0x;
             x = px;
          }
        if ((sd->down.bounce_y_animator) ||
            (sd->down.bounce_y_hold))
          {
             sd->down.by = sd->down.by0 - dy + sd->down.b0y;
             y = py;
          }
        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
        _update_wanted_coordinates(sd, x, y);
        sd->pan_func.max_get(sd->pan_obj, &maxx, &maxy);
        sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
        if (!sd->bounce_horiz)
          {
             if (x <= minx) no_bounce_x_end = EINA_TRUE;
             if ((x - minx) >= maxx) no_bounce_x_end = EINA_TRUE;
          }
        if (!sd->bounce_vert)
          {
             if (y <= miny) no_bounce_y_end = EINA_TRUE;
             if ((y - miny) >= maxy) no_bounce_y_end = EINA_TRUE;
          }
        if ((dt >= 1.0) ||
            ((sd->down.bounce_x_hold) && (sd->down.bounce_y_hold)) ||
            (no_bounce_x_end && no_bounce_y_end))
          {
             _smart_anim_stop(sd->smart_obj);

             sd->down.momentum_animator = NULL;
             sd->down.bounce_x_hold = 0;
             sd->down.bounce_y_hold = 0;
             sd->down.ax = 0;
             sd->down.ay = 0;
             sd->down.pdx = 0;
             sd->down.pdy = 0;
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
             return ECORE_CALLBACK_CANCEL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static void
bounce_eval(Smart_Data *sd)
{
   Evas_Coord mx, my, px, py, bx, by, b2x, b2y, minx = 0, miny = 0;

   if (sd->freeze) return;
   if ((!sd->bouncemex) && (!sd->bouncemey)) return;
   if (sd->down.now) return; // down bounce while still held down
   if (sd->down.onhold_animator)
     {
        ecore_animator_del(sd->down.onhold_animator);
        sd->down.onhold_animator = NULL;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (sd->down.hold_animator)
     {
        ecore_animator_del(sd->down.hold_animator);
        sd->down.hold_animator = NULL;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   bx = px;
   by = py;
   if (px < minx) px = minx;
   if ((px - minx) > mx) px = mx + minx;
   if (py < miny) py = miny;
   if ((py - miny) > my) py = my + miny;
   b2x = px;
   b2y = py;
   if ((!sd->widget) ||
       (!elm_widget_drag_child_locked_x_get(sd->widget)))
     {
        if ((!sd->down.bounce_x_animator) && (!sd->bounce_animator_disabled))
          {
             if (sd->bouncemex)
               {
                  if (sd->scrollto.x.animator)
                    {
                       ecore_animator_del(sd->scrollto.x.animator);
                       sd->scrollto.x.animator = NULL;
                    }
                  sd->down.bounce_x_animator = ecore_animator_add(_smart_bounce_x_animator, sd);
                  sd->down.anim_start2 = ecore_loop_time_get();
                  sd->down.bx = bx;
                  sd->down.bx0 = bx;
                  sd->down.b2x = b2x;
                  if (sd->down.momentum_animator) sd->down.b0x = sd->down.ax;
                  else sd->down.b0x = 0;
               }
          }
     }
   if ((!sd->widget) ||
       (!elm_widget_drag_child_locked_y_get(sd->widget)))
     {
        if ((!sd->down.bounce_y_animator) && (!sd->bounce_animator_disabled))
          {
             if (sd->bouncemey)
               {
                  if (sd->scrollto.y.animator)
                    {
                       ecore_animator_del(sd->scrollto.y.animator);
                       sd->scrollto.y.animator = NULL;
                    }
                  sd->down.bounce_y_animator = ecore_animator_add(_smart_bounce_y_animator, sd);
                  sd->down.anim_start3 = ecore_loop_time_get();
                  sd->down.by = by;
                  sd->down.by0 = by;
                  sd->down.b2y = b2y;
                  if (sd->down.momentum_animator) sd->down.b0y = sd->down.ay;
                  else sd->down.b0y = 0;
               }
          }
     }
}

void
elm_smart_scroller_child_pos_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord mx = 0, my = 0, px = 0, py = 0, minx = 0, miny = 0;
   double vx, vy;

   API_ENTRY return;
   // FIXME: allow for bounce outside of range
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
   if (mx > 0) vx = (double)(x - minx) / (double)mx;
   else vx = 0.0;
   if (vx < 0.0) vx = 0.0;
   else if (vx > 1.0) vx = 1.0;
   if (my > 0) vy = (double)(y - miny) / (double)my;
   else vy = 0.0;
   if (vy < 0.0) vy = 0.0;
   else if (vy > 1.0) vy = 1.0;
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.vbar", 0.0, vy);
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.hbar", vx, 0.0);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   if (!_elm_config->thumbscroll_bounce_enable)
     {
        if (x < minx) x = minx;
        if ((x - minx) > mx) x = mx + minx;
        if (y < miny) y = miny;
        if ((y - miny) > my) y = my + miny;
     }

   if (!sd->bounce_horiz)
     {
        if (x < minx) x = minx;
        if ((x - minx) > mx) x = mx + minx;
     }
   if (!sd->bounce_vert)
     {
        if (y < miny) y = miny;
        if (y - miny > my) y = my + miny;
     }

   sd->pan_func.set(sd->pan_obj, x, y);
   if ((px != x) || (py != y))
     edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
   if (!sd->down.bounce_x_animator)
     {
        if (((x < minx) && (0 <= sd->down.dx)) ||
            ((x > (mx + minx)) && (0 >= sd->down.dx)))
          {
             sd->bouncemex = 1;
             bounce_eval(sd);
          }
        else
          sd->bouncemex = 0;
     }
   if (!sd->down.bounce_y_animator)
     {
        if (((y < miny) && (0 <= sd->down.dy)) ||
            ((y > (my + miny)) && (0 >= sd->down.dy)))
          {
             sd->bouncemey = 1;
             bounce_eval(sd);
          }
        else
          sd->bouncemey = 0;
     }
   if ((x != px) || (y != py))
     {
        evas_object_smart_callback_call(obj, "scroll", NULL);
     }
   if ((x != px)/* && (!sd->bouncemex)*/)
     {
        if (x == minx)
          evas_object_smart_callback_call(obj, "edge,left", NULL);
        if (x == (mx + minx))
          evas_object_smart_callback_call(obj, "edge,right", NULL);
     }
   if ((y != py)/* && (!sd->bouncemey)*/)
     {
        if (y == miny)
          evas_object_smart_callback_call(obj, "edge,top", NULL);
        if (y == my + miny)
          evas_object_smart_callback_call(obj, "edge,bottom", NULL);
     }
}

void
elm_smart_scroller_child_pos_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   sd->pan_func.get(sd->pan_obj, x, y);
}

/* returns TRUE when we need to move the scroller, FALSE otherwise.
 * Updates w and h either way, so save them if you need them. */
static Eina_Bool
_elm_smart_scroller_child_region_show_internal(Evas_Object *obj, Evas_Coord *_x, Evas_Coord *_y, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord mx = 0, my = 0, cw = 0, ch = 0, px = 0, py = 0, nx, ny, minx = 0, miny = 0, pw = 0, ph = 0, x = *_x, y = *_y;

   API_ENTRY return EINA_FALSE;
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &pw, &ph);

   nx = px;
   if ((x < px) && ((x + w) < (px + (cw - mx)))) nx = x;
   else if ((x > px) && ((x + w) > (px + (cw - mx)))) nx = x + w - (cw - mx);
   ny = py;
   if ((y < py) && ((y + h) < (py + (ch - my)))) ny = y;
   else if ((y > py) && ((y + h) > (py + (ch - my)))) ny = y + h - (ch - my);

   if ((sd->down.bounce_x_animator) || (sd->down.bounce_y_animator) ||
       (sd->scrollto.x.animator) || (sd->scrollto.y.animator))
     {
        _smart_anim_stop(sd->smart_obj);
     }
   if (sd->scrollto.x.animator)
     {
        ecore_animator_del(sd->scrollto.x.animator);
        sd->scrollto.x.animator = NULL;
     }
   if (sd->scrollto.y.animator)
     {
        ecore_animator_del(sd->scrollto.y.animator);
        sd->scrollto.y.animator = NULL;
     }
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        sd->bouncemex = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (sd->down.hold_animator)
     {
        ecore_animator_del(sd->down.hold_animator);
        sd->down.hold_animator = NULL;
        _smart_drag_stop(sd->smart_obj);
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (sd->down.momentum_animator)
     {
        ecore_animator_del(sd->down.momentum_animator);
        sd->down.momentum_animator = NULL;
        sd->down.bounce_x_hold = 0;
        sd->down.bounce_y_hold = 0;
        sd->down.ax = 0;
        sd->down.ay = 0;
        sd->down.pdx = 0;
        sd->down.pdy = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }

   x = nx;
   if ((x + pw) > cw) x = cw - pw;
   if (x < minx) x = minx;
   y = ny;
   if ((y + ph) > ch) y = ch - ph;
   if (y < miny) y = miny;

   if ((x == px) && (y == py)) return EINA_FALSE;
   *_x = x;
   *_y = y;
   return EINA_TRUE;
}

/* Set should be used for calculated positions, for example, when we move
 * because of an animation or because this is the correct position after
 * constraints. */
void
elm_smart_scroller_child_region_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (_elm_smart_scroller_child_region_show_internal(obj, &x, &y, w, h))
     elm_smart_scroller_child_pos_set(obj, x, y);
}

/* Set should be used for setting the wanted position, for example a user scroll
 * or moving the cursor in an entry. */
void
elm_smart_scroller_child_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   API_ENTRY return;
   sd->wx = x;
   sd->wy = y;
   sd->ww = w;
   sd->wh = h;
   if (_elm_smart_scroller_child_region_show_internal(obj, &x, &y, w, h))
     elm_smart_scroller_child_pos_set(obj, x, y);
}

void
elm_smart_scroller_child_viewport_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   API_ENTRY return;
   if (!sd->pan_obj) return;
   edje_object_calc_force(sd->edje_obj);
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, w, h);
}

void
elm_smart_scroller_step_size_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   API_ENTRY return;
   if (x < 1) x = 1;
   if (y < 1) y = 1;
   sd->step.x = x;
   sd->step.y = y;
   _smart_scrollbar_size_adjust(sd);
}

void
elm_smart_scroller_step_size_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   if (x) *x = sd->step.x;
   if (y) *y = sd->step.y;
}

void
elm_smart_scroller_page_size_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   API_ENTRY return;
   sd->page.x = x;
   sd->page.y = y;
   _smart_scrollbar_size_adjust(sd);
}

void
elm_smart_scroller_page_size_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   if (x) *x = sd->page.x;
   if (y) *y = sd->page.y;
}

void
elm_smart_scroller_policy_set(Evas_Object *obj, Elm_Smart_Scroller_Policy hbar, Elm_Smart_Scroller_Policy vbar)
{
   API_ENTRY return;
   if ((sd->hbar_flags == hbar) && (sd->vbar_flags == vbar)) return;
   sd->hbar_flags = hbar;
   sd->vbar_flags = vbar;
   if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,hbar", "elm");
   else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,hbar", "elm");
   if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,vbar", "elm");
   else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,vbar", "elm");
   _smart_scrollbar_size_adjust(sd);
}

void
elm_smart_scroller_policy_get(Evas_Object *obj, Elm_Smart_Scroller_Policy *hbar, Elm_Smart_Scroller_Policy *vbar)
{
   API_ENTRY return;
   if (hbar) *hbar = sd->hbar_flags;
   if (vbar) *vbar = sd->vbar_flags;
}

Evas_Object *
elm_smart_scroller_edje_object_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->edje_obj;
}

void
elm_smart_scroller_single_dir_set(Evas_Object *obj, Eina_Bool single_dir)
{
   API_ENTRY return;
   sd->one_dir_at_a_time = single_dir;
}

Eina_Bool
elm_smart_scroller_single_dir_get(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->one_dir_at_a_time;
}

void
elm_smart_scroller_object_theme_set(Evas_Object *parent, Evas_Object *obj, const char *clas, const char *group, const char *style)
{
   API_ENTRY return;
   Evas_Coord mw, mh;
   //Does this API require parent object absolutely? if then remove this exception.
   double parent_scale = parent ? elm_widget_scale_get(parent) : 1;
   _elm_theme_object_set(parent, sd->edje_obj, clas, group, style);
   edje_object_scale_set(sd->edje_obj, parent_scale * _elm_config->scale);
   if (sd->pan_obj)
     edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", sd->pan_obj);
   mw = mh = -1;
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   if (edje_object_part_exists(sd->edje_obj, "elm.scrollbar.base"))
     {
        Evas_Object *base;
        base = edje_object_part_swallow_get(sd->edje_obj, "elm.scrollbar.base");
        if (!base)
          {
             base = evas_object_rectangle_add(evas_object_evas_get(sd->edje_obj));
             evas_object_color_set(base, 0, 0, 0, 0);
             edje_object_part_swallow(sd->edje_obj, "elm.scrollbar.base", base);
          }
        if (!_elm_config->thumbscroll_enable)
          evas_object_size_hint_min_set(base, mw, mh);
     }
   sd->vbar_visible = !sd->vbar_visible;
   sd->hbar_visible = !sd->hbar_visible;
   _smart_scrollbar_bar_visibility_adjust(sd);
}

void
elm_smart_scroller_hold_set(Evas_Object *obj, Eina_Bool hold)
{
   API_ENTRY return;
   sd->hold = hold;
}

void
elm_smart_scroller_freeze_set(Evas_Object *obj, Eina_Bool freeze)
{
   API_ENTRY return;
   sd->freeze = freeze;
   if (sd->freeze)
     {
        if (sd->down.onhold_animator)
          {
             ecore_animator_del(sd->down.onhold_animator);
             sd->down.onhold_animator = NULL;
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
          }
     }
   else
     bounce_eval(sd);
}

void
elm_smart_scroller_bounce_allow_set(Evas_Object *obj, Eina_Bool horiz, Eina_Bool vert)
{
   API_ENTRY return;
   sd->bounce_horiz = horiz;
   sd->bounce_vert = vert;
}

void
elm_smart_scroller_bounce_allow_get(const Evas_Object *obj, Eina_Bool *horiz, Eina_Bool *vert)
{
   API_ENTRY return;
   if (horiz) *horiz = sd->bounce_horiz;
   if (vert) *vert = sd->bounce_vert;
}

void
elm_smart_scroller_paging_set(Evas_Object *obj, double pagerel_h, double pagerel_v, Evas_Coord pagesize_h, Evas_Coord pagesize_v)
{
   API_ENTRY return;
   sd->pagerel_h = pagerel_h;
   sd->pagerel_v = pagerel_v;
   sd->pagesize_h = pagesize_h;
   sd->pagesize_v = pagesize_v;
   _smart_page_adjust(sd);
}

void
elm_smart_scroller_paging_get(Evas_Object *obj, double *pagerel_h, double *pagerel_v, Evas_Coord *pagesize_h, Evas_Coord *pagesize_v)
{
   API_ENTRY return;
   if (pagerel_h) *pagerel_h = sd->pagerel_h;
   if (pagerel_v) *pagerel_v = sd->pagerel_v;
   if (pagesize_h) *pagesize_h = sd->pagesize_h;
   if (pagesize_v) *pagesize_v = sd->pagesize_v;
}

void
elm_smart_scroller_current_page_get(Evas_Object *obj, int *pagenumber_h, int *pagenumber_v)
{
   API_ENTRY return;
   Evas_Coord x, y;
   elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
   if (pagenumber_h)
     {
        if (sd->pagesize_h > 0)
          *pagenumber_h = (x + sd->pagesize_h - 1) / sd->pagesize_h;
        else
          *pagenumber_h = 0;
     }
   if (pagenumber_v)
     {
        if (sd->pagesize_v > 0)
          *pagenumber_v = (y + sd->pagesize_v - 1) / sd->pagesize_v;
        else
          *pagenumber_v = 0;
     }
}

void
elm_smart_scroller_last_page_get(Evas_Object *obj, int *pagenumber_h, int *pagenumber_v)
{
   API_ENTRY return;
   Evas_Coord cw, ch;
   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);
   if (pagenumber_h)
     {
        if (sd->pagesize_h > 0)
          *pagenumber_h = cw / sd->pagesize_h + 1;
        else
          *pagenumber_h = 0;
     }
   if (pagenumber_v)
     {
        if (sd->pagesize_v > 0)
          *pagenumber_v = ch / sd->pagesize_v + 1;
        else
          *pagenumber_v = 0;
     }
}

void
elm_smart_scroller_page_show(Evas_Object *obj, int pagenumber_h, int pagenumber_v)
{
   API_ENTRY return;
   Evas_Coord x, y, w, h;
   elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);
   if (pagenumber_h >= 0) x = sd->pagesize_h * pagenumber_h;
   if (pagenumber_v >= 0) y = sd->pagesize_v * pagenumber_v;
   if (_elm_smart_scroller_child_region_show_internal(obj, &x, &y, w, h))
     elm_smart_scroller_child_pos_set(obj, x, y);
}

void
elm_smart_scroller_page_bring_in(Evas_Object *obj, int pagenumber_h, int pagenumber_v)
{
   API_ENTRY return;
   Evas_Coord x, y, w, h;
   elm_smart_scroller_child_viewport_size_get(sd->smart_obj, &w, &h);
   if (pagenumber_h >= 0) x = sd->pagesize_h * pagenumber_h;
   if (pagenumber_v >= 0) y = sd->pagesize_v * pagenumber_v;
   if (_elm_smart_scroller_child_region_show_internal(obj, &x, &y, w, h))
     {
        _smart_scrollto_x(sd, _elm_config->bring_in_scroll_friction, x);
        _smart_scrollto_y(sd, _elm_config->bring_in_scroll_friction, y);
     }
}

void
elm_smart_scroller_region_bring_in(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   API_ENTRY return;
   if (_elm_smart_scroller_child_region_show_internal(obj, &x, &y, w, h))
     {
        _smart_scrollto_x(sd, _elm_config->bring_in_scroll_friction, x);
        _smart_scrollto_y(sd, _elm_config->bring_in_scroll_friction, y);
     }
}

void
elm_smart_scroller_widget_set(Evas_Object *obj, Evas_Object *wid)
{
   API_ENTRY return;
   sd->widget = wid;
}

static void
_elm_smart_scroller_wanted_region_set(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   Evas_Coord ww, wh, wx = sd->wx;

   if (sd->down.now || sd->down.momentum_animator ||
       sd->down.bounce_x_animator || sd->down.bounce_y_animator ||
       sd->down.hold_animator || sd->down.onhold_animator) return;

   sd->child.resized = EINA_FALSE;

   /* Flip to RTL cords only if init in RTL mode */
   if (sd->is_mirrored)
     wx = _elm_smart_scroller_x_mirrored_get(obj, sd->wx);

   if (sd->ww == -1)
     {
        elm_smart_scroller_child_viewport_size_get(obj, &ww, &wh);
     }
   else
     {
        ww = sd->ww;
        wh = sd->wh;
     }

   elm_smart_scroller_child_region_set(obj, wx, sd->wy, ww, wh);
}

/* local subsystem functions */
static void
_smart_edje_drag_v_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   _smart_scrollbar_read(sd);
   _smart_drag_start(sd->smart_obj);
   sd->freeze = EINA_TRUE;
}

static void
_smart_edje_drag_v_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   _smart_scrollbar_read(sd);
   _smart_drag_stop(sd->smart_obj);
   sd->freeze = EINA_FALSE;
}

static void
_smart_edje_drag_v(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   _smart_scrollbar_read(sd);
}

static void
_smart_edje_drag_h_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   _smart_scrollbar_read(sd);
   _smart_drag_start(sd->smart_obj);
   sd->freeze = EINA_TRUE;
}

static void
_smart_edje_drag_h_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   _smart_scrollbar_read(sd);
   _smart_drag_stop(sd->smart_obj);
   sd->freeze = EINA_FALSE;
}

static void
_smart_edje_drag_h(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   _smart_scrollbar_read(sd);
}

static void
_smart_child_del_hook(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   sd->child_obj = NULL;
   _smart_scrollbar_size_adjust(sd);
   _smart_scrollbar_reset(sd);
}

static void
_smart_pan_changed_hook(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   Smart_Data *sd;

   sd = data;
   sd->pan_func.child_size_get(sd->pan_obj, &w, &h);
   if ((w != sd->child.w) || (h != sd->child.h))
     {
        sd->child.w = w;
        sd->child.h = h;
        _smart_scrollbar_size_adjust(sd);
        evas_object_size_hint_min_set(sd->smart_obj, sd->child.w, sd->child.h);
        sd->child.resized = EINA_TRUE;
        _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
}

static void
_smart_pan_pan_changed_hook(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Smart_Data *sd;

   sd = data;
   if ((sd->down.bounce_x_animator) || (sd->down.bounce_y_animator) ||
       (sd->scrollto.x.animator) || (sd->scrollto.y.animator))
     {
        _smart_anim_stop(sd->smart_obj);
     }
   if (sd->scrollto.x.animator)
     {
        ecore_animator_del(sd->scrollto.x.animator);
        sd->scrollto.x.animator = NULL;
     }
   if (sd->scrollto.y.animator)
     {
        ecore_animator_del(sd->scrollto.y.animator);
        sd->scrollto.y.animator = NULL;
     }
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        sd->bouncemex = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   _elm_smart_scroller_wanted_region_set(sd->smart_obj);
}

void
elm_smart_scroller_gravity_set(Evas_Object *obj, double x, double y)
{
   API_ENTRY return;

   sd->pan_func.gravity_set(sd->pan_obj, x, y);
}

void
elm_smart_scroller_gravity_get(Evas_Object *obj, double *x, double *y)
{
   API_ENTRY return;

   sd->pan_func.gravity_get(sd->pan_obj, x, y);
}

static void
_smart_event_wheel(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   if ((evas_key_modifier_is_set(ev->modifiers, "Control")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Alt")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Shift")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Meta")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Hyper")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Super")))
     return;
   elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
   if ((sd->down.bounce_x_animator) || (sd->down.bounce_y_animator) ||
       (sd->scrollto.x.animator) || (sd->scrollto.y.animator))
     {
        _smart_anim_stop(sd->smart_obj);
     }
   if (sd->scrollto.x.animator)
     {
        ecore_animator_del(sd->scrollto.x.animator);
        sd->scrollto.x.animator = NULL;
     }
   if (sd->scrollto.y.animator)
     {
        ecore_animator_del(sd->scrollto.y.animator);
        sd->scrollto.y.animator = NULL;
     }
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        sd->bouncemex = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
        if (sd->child.resized)
          _elm_smart_scroller_wanted_region_set(sd->smart_obj);
     }
   if (!ev->direction)
     y += ev->z * sd->step.y;
   else if (ev->direction == 1)
     x += ev->z * sd->step.x;

   if ((!sd->hold) && (!sd->freeze))
     {
        _update_wanted_coordinates(sd, x, y);
        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
     }
}

static void
_smart_event_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   //   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   if (_elm_config->thumbscroll_enable)
     {
        sd->down.hold = 0;
        if ((sd->down.bounce_x_animator) || (sd->down.bounce_y_animator) ||
            (sd->down.momentum_animator) || (sd->scrollto.x.animator) ||
            (sd->scrollto.y.animator))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL | EVAS_EVENT_FLAG_ON_HOLD;
             sd->down.scroll = 1;
             sd->down.hold = 1;
             _smart_anim_stop(sd->smart_obj);
          }
        if (sd->scrollto.x.animator)
          {
             ecore_animator_del(sd->scrollto.x.animator);
             sd->scrollto.x.animator = NULL;
          }
        if (sd->scrollto.y.animator)
          {
             ecore_animator_del(sd->scrollto.y.animator);
             sd->scrollto.y.animator = NULL;
          }
        if (sd->down.bounce_x_animator)
          {
             ecore_animator_del(sd->down.bounce_x_animator);
             sd->down.bounce_x_animator = NULL;
             sd->bouncemex = 0;
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
          }
        if (sd->down.bounce_y_animator)
          {
             ecore_animator_del(sd->down.bounce_y_animator);
             sd->down.bounce_y_animator = NULL;
             sd->bouncemey = 0;
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
          }
        if (sd->down.hold_animator)
          {
             ecore_animator_del(sd->down.hold_animator);
             sd->down.hold_animator = NULL;
             _smart_drag_stop(sd->smart_obj);
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
          }
        if (sd->down.momentum_animator)
          {
             ecore_animator_del(sd->down.momentum_animator);
             sd->down.momentum_animator = NULL;
             sd->down.bounce_x_hold = 0;
             sd->down.bounce_y_hold = 0;
             sd->down.ax = 0;
             sd->down.ay = 0;
             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);
          }
        if (ev->button == 1)
          {
             sd->down.hist.est_timestamp_diff =
                ecore_loop_time_get() - ((double)ev->timestamp / 1000.0);
             sd->down.hist.tadd = 0.0;
             sd->down.hist.dxsum = 0.0;
             sd->down.hist.dysum = 0.0;
             sd->down.now = 1;
             sd->down.dragged = 0;
             sd->down.dir_x = 0;
             sd->down.dir_y = 0;
             sd->down.x = ev->canvas.x;
             sd->down.y = ev->canvas.y;
             elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
             sd->down.sx = x;
             sd->down.sy = y;
             sd->down.locked = 0;
             memset(&(sd->down.history[0]), 0, sizeof(sd->down.history[0]) * 60);
#ifdef EVTIME
             sd->down.history[0].timestamp = ev->timestamp / 1000.0;
             sd->down.history[0].localtimestamp = ecore_loop_time_get();
#else
             sd->down.history[0].timestamp = ecore_loop_time_get();
#endif
             sd->down.history[0].x = ev->canvas.x;
             sd->down.history[0].y = ev->canvas.y;
          }
        sd->down.dragged_began = 0;
        sd->down.hold_parent = 0;
        sd->down.cancelled = 0;
     }
}

static void
_down_coord_eval(Smart_Data *sd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord minx, miny;

   if (sd->down.dir_x) *x = sd->down.sx - (*x - sd->down.x);
   else *x = sd->down.sx;
   if (sd->down.dir_y) *y = sd->down.sy - (*y - sd->down.y);
   else *y = sd->down.sy;

   if ((sd->down.dir_x) || (sd->down.dir_y))
     {
        if (!((sd->down.dir_x) && (sd->down.dir_y)))
          {
             if (sd->down.dir_x) *y = sd->down.locked_y;
             else *x = sd->down.locked_x;
          }
     }

   sd->pan_func.min_get(sd->pan_obj, &minx, &miny);

   if (*x < minx)
     *x += (minx - *x) * _elm_config->thumbscroll_border_friction;
   else if (sd->child.w <= sd->w)
     *x += (sd->down.sx - *x) * _elm_config->thumbscroll_border_friction;
   else if ((sd->child.w - sd->w + minx) < *x)
     *x += (sd->child.w - sd->w + minx - *x) *
        _elm_config->thumbscroll_border_friction;

   if (*y < miny)
     *y += (miny - *y) * _elm_config->thumbscroll_border_friction;
   else if (sd->child.h <= sd->h)
     *y += (sd->down.sy - *y) * _elm_config->thumbscroll_border_friction;
   else if ((sd->child.h - sd->h + miny) < *y)
     *y += (sd->child.h - sd->h + miny - *y) *
        _elm_config->thumbscroll_border_friction;
}

static Eina_Bool
_smart_hold_animator(void *data)
{
   Smart_Data *sd = data;
   Evas_Coord ox = 0, oy = 0, fx, fy;

   fx = sd->down.hold_x;
   fy = sd->down.hold_y;
   if (_elm_config->scroll_smooth_amount > 0.0)
     {
        int i, count = 0;
        Evas_Coord basex = 0, basey = 0, x, y;
        double dt, t, tdiff, tnow, twin;
        struct {
             Evas_Coord x, y, dx, dy;
             double t, dt;
        } pos[60];

        tdiff = sd->down.hist.est_timestamp_diff;
        tnow = ecore_time_get() - tdiff;
        t = tnow;
        twin = _elm_config->scroll_smooth_time_window;
        for (i = 0; i < 60; i++)
          {
             // oldest point is sd->down.history[i]
             // newset is sd->down.history[0]
             dt = t - sd->down.history[i].timestamp;
             if (dt > twin)
               {
                  i--;
                  break;
               }
             x = sd->down.history[i].x;
             y = sd->down.history[i].y;
             _down_coord_eval(sd, &x, &y);
             if (i == 0)
               {
                  basex = x;
                  basey = y;
               }
             pos[i].x = x - basex;
             pos[i].y = y - basey;
             pos[i].t =
                sd->down.history[i].timestamp - sd->down.history[0].timestamp;
             count++;
          }
        count = i;
        if (count >= 2)
          {
             double dtsum = 0.0, tadd, maxdt;
             double dxsum = 0.0, dysum = 0.0, xsum = 0.0, ysum = 0.0;

             for (i = 0; i < (count - 1); i++)
               {
                  pos[i].dx = pos[i].x - pos[i + 1].x;
                  pos[i].dy = pos[i].y - pos[i + 1].y;
                  pos[i].dt = pos[i].t - pos[i + 1].t;
                  dxsum += pos[i].dx;
                  dysum += pos[i].dy;
                  dtsum += pos[i].dt;
                  xsum += pos[i].x;
                  ysum += pos[i].y;
               }
             maxdt = pos[i].t;
             dxsum /= (double)i;
             dysum /= (double)i;
             dtsum /= (double)i;
             xsum /= (double)i;
             ysum /= (double)i;
             tadd = tnow - sd->down.history[0].timestamp + _elm_config->scroll_smooth_future_time;
             tadd = tadd - (maxdt / 2);
#define WEIGHT(n, o, v) n = (((double)o * (1.0 - v)) + ((double)n * v))
             WEIGHT(tadd, sd->down.hist.tadd, _elm_config->scroll_smooth_history_weight);
             WEIGHT(dxsum, sd->down.hist.dxsum, _elm_config->scroll_smooth_history_weight);
             WEIGHT(dysum, sd->down.hist.dysum, _elm_config->scroll_smooth_history_weight);
             fx = basex + xsum + ((dxsum * tadd) / dtsum);
             fy = basey + ysum + ((dysum * tadd) / dtsum);
             sd->down.hist.tadd = tadd;
             sd->down.hist.dxsum = dxsum;
             sd->down.hist.dysum = dysum;
             WEIGHT(fx, sd->down.hold_x, _elm_config->scroll_smooth_amount);
             WEIGHT(fy, sd->down.hold_y, _elm_config->scroll_smooth_amount);
          }
        //        printf("%3.5f %i %i\n", ecore_time_get(), sd->down.hold_y, fy);
     }

   elm_smart_scroller_child_pos_get(sd->smart_obj, &ox, &oy);
   if (sd->down.dir_x)
     {
        if ((!sd->widget) ||
            (!elm_widget_drag_child_locked_x_get(sd->widget)))
          ox = fx;
     }
   if (sd->down.dir_y)
     {
        if ((!sd->widget) ||
            (!elm_widget_drag_child_locked_y_get(sd->widget)))
          oy = fy;
     }

   elm_smart_scroller_child_pos_set(sd->smart_obj, ox, oy);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_smart_event_post_up(void *data, Evas *e __UNUSED__)
{
   Smart_Data *sd = data;
   if (sd->widget)
     {
        if (sd->down.dragged)
          {
             elm_widget_drag_lock_x_set(sd->widget, 0);
             elm_widget_drag_lock_y_set(sd->widget, 0);
          }
     }
   return EINA_TRUE;
}

static void
_smart_event_mouse_up(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0, ox = 0, oy = 0;

   sd = data;
   ev = event_info;
   sd->down.hold_parent = 0;
   sd->down.dx = 0;
   sd->down.dy = 0;
   //   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   evas_post_event_callback_push(e, _smart_event_post_up, sd);
   // FIXME: respect elm_widget_scroll_hold_get of parent container
   if (_elm_config->thumbscroll_enable)
     {
        if (ev->button == 1)
          {
             if (sd->down.onhold_animator)
               {
                  ecore_animator_del(sd->down.onhold_animator);
                  sd->down.onhold_animator = NULL;
                  if (sd->child.resized)
                    _elm_smart_scroller_wanted_region_set(sd->smart_obj);
               }
             x = ev->canvas.x - sd->down.x;
             y = ev->canvas.y - sd->down.y;
             if (sd->down.dragged)
               {
                  _smart_drag_stop(sd->smart_obj);
                  if ((!sd->hold) && (!sd->freeze))
                    {
                       double t, at, dt;
                       int i;
                       Evas_Coord ax, ay, dx, dy, vel;

#ifdef EVTIME
                       t = ev->timestamp / 1000.0;
#else
                       t = ecore_loop_time_get();
#endif
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                       ax = ev->canvas.x;
                       ay = ev->canvas.y;
                       at = 0.0;
#ifdef SCROLLDBG
                       printf("------ %i %i\n", ev->canvas.x, ev->canvas.y);
#endif
                       for (i = 0; i < 60; i++)
                         {
                            dt = t - sd->down.history[i].timestamp;
                            if (dt > 0.2) break;
#ifdef SCROLLDBG
                            printf("H: %i %i @ %1.3f\n",
                                   sd->down.history[i].x,
                                   sd->down.history[i].y, dt);
#endif
                            at += dt;
                            ax += sd->down.history[i].x;
                            ay += sd->down.history[i].y;
                         }
                       ax /= (i + 1);
                       ay /= (i + 1);
                       at /= (i + 1);
                       at /= _elm_config->thumbscroll_sensitivity_friction;
                       dx = ev->canvas.x - ax;
                       dy = ev->canvas.y - ay;
                       if (at > 0)
                         {
                            vel = sqrt((dx * dx) + (dy * dy)) / at;
                            if ((_elm_config->thumbscroll_friction > 0.0) &&
                                (vel > _elm_config->thumbscroll_momentum_threshold))
                              {
                                 int minx, miny, mx, my, px, py;
                                 sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
                                 sd->pan_func.max_get(sd->pan_obj, &mx, &my);
                                 sd->pan_func.get(sd->pan_obj, &px, &py);
                                 sd->down.dx = ((double)dx / at);
                                 sd->down.dy = ((double)dy / at);
                                 if (((sd->down.dx > 0) && (sd->down.pdx > 0)) ||
                                     ((sd->down.dx < 0) && (sd->down.pdx < 0)))
                                   if (px > minx && px < mx)
                                     sd->down.dx += (double)sd->down.pdx * 1.5; // FIXME: * 1.5 - probably should be config
                                 if (((sd->down.dy > 0) && (sd->down.pdy > 0)) ||
                                     ((sd->down.dy < 0) && (sd->down.pdy < 0)))
                                   if (py > miny && py < my)
                                     sd->down.dy += (double)sd->down.pdy * 1.5; // FIXME: * 1.5 - probably should be config
                                 if (((sd->down.dx > 0) && (sd->down.pdx > 0)) ||
                                     ((sd->down.dx < 0) && (sd->down.pdx < 0)) ||
                                     ((sd->down.dy > 0) && (sd->down.pdy > 0)) ||
                                     ((sd->down.dy < 0) && (sd->down.pdy < 0)))
                                   {
                                      double t = ecore_loop_time_get();
                                      double dt = t - sd->down.anim_start;

                                      if (dt < 0.0) dt = 0.0;
                                      else if (dt > _elm_config->thumbscroll_friction)
                                        dt = _elm_config->thumbscroll_friction;
                                      sd->down.extra_time = _elm_config->thumbscroll_friction - dt;
                                   }
                                 else
                                   sd->down.extra_time = 0.0;
                                 sd->down.pdx = sd->down.dx;
                                 sd->down.pdy = sd->down.dy;
                                 ox = -sd->down.dx;
                                 oy = -sd->down.dy;
                                 if (!_smart_do_page(sd))
                                   {
                                      if ((!sd->down.momentum_animator) && (!sd->momentum_animator_disabled))
                                        {
                                           sd->down.momentum_animator = ecore_animator_add(_smart_momentum_animator, sd);
                                           ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                                           _smart_anim_start(sd->smart_obj);
                                        }
                                      sd->down.anim_start = ecore_loop_time_get();
                                      elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
                                      sd->down.sx = x;
                                      sd->down.sy = y;
                                      sd->down.b0x = 0;
                                      sd->down.b0y = 0;
                                   }
                              }
                         }
                    }
                  else
                    {
                       sd->down.pdx = 0;
                       sd->down.pdy = 0;
                    }
                  evas_event_feed_hold(e, 0, ev->timestamp, ev->data);
                  if (_smart_do_page(sd))
                    {
                       Evas_Coord pgx, pgy;

                       elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
                       if ((!sd->widget) ||
                           (!elm_widget_drag_child_locked_x_get(sd->widget)))
                         {
                            pgx = _smart_page_x_get(sd, ox);
                            if (pgx != x)
                              {
                                 ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                                 _smart_scrollto_x(sd, _elm_config->page_scroll_friction, pgx);
                              }
                         }
                       if ((!sd->widget) ||
                           (!elm_widget_drag_child_locked_y_get(sd->widget)))
                         {
                            pgy = _smart_page_y_get(sd, oy);
                            if (pgy != y)
                              {
                                 ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                                 _smart_scrollto_y(sd, _elm_config->page_scroll_friction, pgy);
                              }
                         }
                    }
               }
             else
               {
                  sd->down.pdx = 0;
                  sd->down.pdy = 0;
                  if (_smart_do_page(sd))
                    {
                       Evas_Coord pgx, pgy;

                       elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
                       if ((!sd->widget) ||
                           (!elm_widget_drag_child_locked_x_get(sd->widget)))
                         {
                            pgx = _smart_page_x_get(sd, ox);
                            if (pgx != x) _smart_scrollto_x(sd, _elm_config->page_scroll_friction, pgx);
                         }
                       if ((!sd->widget) ||
                           (!elm_widget_drag_child_locked_y_get(sd->widget)))
                         {
                            pgy = _smart_page_y_get(sd, oy);
                            if (pgy != y) _smart_scrollto_y(sd, _elm_config->page_scroll_friction, pgy);
                         }
                    }
               }
             if (sd->down.hold_animator)
               {
                  ecore_animator_del(sd->down.hold_animator);
                  sd->down.hold_animator = NULL;
                  if (sd->child.resized)
                    _elm_smart_scroller_wanted_region_set(sd->smart_obj);
               }
             if (sd->down.scroll)
               {
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                  sd->down.scroll = 0;
               }
             if (sd->down.hold)
               {
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                  sd->down.hold = 0;
               }
             sd->down.dragged_began = 0;
             sd->down.dir_x = 0;
             sd->down.dir_y = 0;
             sd->down.want_dragged = 0;
             sd->down.dragged = 0;
             sd->down.now = 0;
             elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
             elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
             _update_wanted_coordinates(sd, x, y);

             if (sd->child.resized)
               _elm_smart_scroller_wanted_region_set(sd->smart_obj);

             if (!_smart_do_page(sd))
               bounce_eval(sd);
          }
     }
}

static Eina_Bool
_smart_onhold_animator(void *data)
{
   Smart_Data *sd;
   double t, td;
   double vx, vy;
   Evas_Coord x, y, ox, oy;

   sd = data;
   t = ecore_loop_time_get();
   if (sd->down.onhold_tlast > 0.0)
     {
        td = t - sd->down.onhold_tlast;
        vx = sd->down.onhold_vx * td * (double)_elm_config->thumbscroll_threshold * 2.0;
        vy = sd->down.onhold_vy * td * (double)_elm_config->thumbscroll_threshold * 2.0;
        elm_smart_scroller_child_pos_get(sd->smart_obj, &ox, &oy);
        x = ox;
        y = oy;

        if (sd->down.dir_x)
          {
             if ((!sd->widget) ||
                 (!elm_widget_drag_child_locked_x_get(sd->widget)))
               {
                  sd->down.onhold_vxe += vx;
                  x = ox + (int)sd->down.onhold_vxe;
                  sd->down.onhold_vxe -= (int)sd->down.onhold_vxe;
               }
          }

        if (sd->down.dir_y)
          {
             if ((!sd->widget) ||
                 (!elm_widget_drag_child_locked_y_get(sd->widget)))
               {
                  sd->down.onhold_vye += vy;
                  y = oy + (int)sd->down.onhold_vye;
                  sd->down.onhold_vye -= (int)sd->down.onhold_vye;
               }
          }

        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
     }
   sd->down.onhold_tlast = t;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_smart_event_post_move(void *data, Evas *e __UNUSED__)
{
   Smart_Data *sd = data;

   if (sd->down.want_dragged)
     {
        int start = 0;

        if (sd->down.hold_parent)
          {
             if ((sd->down.dir_x) && !can_scroll(sd, sd->down.hdir))
               {
                  sd->down.dir_x = 0;
               }
             if ((sd->down.dir_y) && !can_scroll(sd, sd->down.vdir))
               {
                  sd->down.dir_y = 0;
               }
          }
        if (sd->down.dir_x)
          {
             if ((!sd->widget) ||
                 (!elm_widget_drag_child_locked_x_get(sd->widget)))
               {
                  sd->down.want_dragged = 0;
                  sd->down.dragged = 1;
                  if (sd->widget)
                    {
                       elm_widget_drag_lock_x_set(sd->widget, 1);
                    }
                  start = 1;
               }
             else
               sd->down.dir_x = 0;
          }
        if (sd->down.dir_y)
          {
             if ((!sd->widget) ||
                 (!elm_widget_drag_child_locked_y_get(sd->widget)))
               {
                  sd->down.want_dragged = 0;
                  sd->down.dragged = 1;
                  if (sd->widget)
                    {
                       elm_widget_drag_lock_y_set(sd->widget, 1);
                    }
                  start = 1;
               }
             else
               sd->down.dir_y = 0;
          }
        if ((!sd->down.dir_x) && (!sd->down.dir_y))
          {
             sd->down.cancelled = 1;
          }
        if (start) _smart_drag_start(sd->smart_obj);
     }
   return EINA_TRUE;
}

static void
_smart_event_mouse_move(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   //   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->down.hold_parent = 1;
   evas_post_event_callback_push(e, _smart_event_post_move, sd);
   // FIXME: respect elm_widget_scroll_hold_get of parent container
   if (_elm_config->thumbscroll_enable)
     {
        if (sd->down.now)
          {
             int dodir = 0;

#ifdef SCROLLDBG
             printf("::: %i %i\n", ev->cur.canvas.x, ev->cur.canvas.y);
#endif
             memmove(&(sd->down.history[1]), &(sd->down.history[0]),
                     sizeof(sd->down.history[0]) * (60 - 1));
#ifdef EVTIME
             sd->down.history[0].timestamp = ev->timestamp / 1000.0;
             sd->down.history[0].localtimestamp = ecore_loop_time_get();
#else
             sd->down.history[0].timestamp = ecore_loop_time_get();
#endif
             sd->down.history[0].x = ev->cur.canvas.x;
             sd->down.history[0].y = ev->cur.canvas.y;

             if (!sd->down.dragged_began)
               {
                  x = ev->cur.canvas.x - sd->down.x;
                  y = ev->cur.canvas.y - sd->down.y;

                  sd->down.hdir = -1;
                  sd->down.vdir = -1;

                  if      (x > 0) sd->down.hdir = LEFT;
                  else if (x < 0) sd->down.hdir = RIGHT;
                  if      (y > 0) sd->down.vdir = UP;
                  else if (y < 0) sd->down.vdir = DOWN;

                  if (x < 0) x = -x;
                  if (y < 0) y = -y;

                  if ((sd->one_dir_at_a_time) &&
                      (!((sd->down.dir_x) || (sd->down.dir_y))))
                    {
                       if (x > _elm_config->thumbscroll_threshold)
                         {
                            if (x > (y * 2))
                              {
                                 sd->down.dir_x = 1;
                                 sd->down.dir_y = 0;
                                 dodir++;
                              }
                         }
                       if (y > _elm_config->thumbscroll_threshold)
                         {
                            if (y > (x * 2))
                              {
                                 sd->down.dir_x = 0;
                                 sd->down.dir_y = 1;
                                 dodir++;
                              }
                         }
                       if (!dodir)
                         {
                            sd->down.dir_x = 1;
                            sd->down.dir_y = 1;
                         }
                    }
                  else
                    {
                       //                       can_scroll(sd, LEFT);
                       //                       can_scroll(sd, RIGHT);
                       //                       can_scroll(sd, UP);
                       //                       can_scroll(sd, DOWN);
                       sd->down.dir_x = 1;
                       sd->down.dir_y = 1;
                    }
               }
             if ((!sd->hold) && (!sd->freeze))
               {
                  if ((sd->down.dragged) ||
                      (((x * x) + (y * y)) >
                       (_elm_config->thumbscroll_threshold *
                        _elm_config->thumbscroll_threshold)))
                    {
                       sd->down.dragged_began = 1;
                       if (!sd->down.dragged)
                         {
                            sd->down.want_dragged = 1;
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                            //                            evas_event_feed_hold(e, 1, ev->timestamp, ev->data);
                            //                            _smart_drag_start(sd->smart_obj);
                         }
                       if (sd->down.dragged)
                         {
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                         }
                       //                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                       //                       sd->down.dragged = 1;
                       if (sd->down.dir_x)
                         x = sd->down.sx - (ev->cur.canvas.x - sd->down.x);
                       else
                         x = sd->down.sx;
                       if (sd->down.dir_y)
                         y = sd->down.sy - (ev->cur.canvas.y - sd->down.y);
                       else
                         y = sd->down.sy;
                       if ((sd->down.dir_x) || (sd->down.dir_y))
                         {
                            if (!sd->down.locked)
                              {
                                 sd->down.locked_x = x;
                                 sd->down.locked_y = y;
                                 sd->down.locked = 1;
                              }
                            if (!((sd->down.dir_x) && (sd->down.dir_y)))
                              {
                                 if (sd->down.dir_x) y = sd->down.locked_y;
                                 else x = sd->down.locked_x;
                              }
                         }
                         {
                            Evas_Coord minx, miny;
                            sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
                            if (y < miny)
                              y += (miny - y) *
                                 _elm_config->thumbscroll_border_friction;
                            else if (sd->child.h <= sd->h)
                              y += (sd->down.sy - y) *
                                 _elm_config->thumbscroll_border_friction;
                            else if ((sd->child.h - sd->h + miny) < y)
                              y += (sd->child.h - sd->h + miny - y) *
                                 _elm_config->thumbscroll_border_friction;
                            if (x < minx)
                              x += (minx - x) *
                                 _elm_config->thumbscroll_border_friction;
                            else if (sd->child.w <= sd->w)
                              x += (sd->down.sx - x) *
                                 _elm_config->thumbscroll_border_friction;
                            else if ((sd->child.w - sd->w + minx) < x)
                              x += (sd->child.w - sd->w + minx - x) *
                                 _elm_config->thumbscroll_border_friction;
                         }

                       sd->down.hold_x = x;
                       sd->down.hold_y = y;
                       if (!sd->down.hold_animator)
                         sd->down.hold_animator =
                            ecore_animator_add(_smart_hold_animator, sd);
                       //                       printf("a %i %i\n", sd->down.hold_x, sd->down.hold_y);
                       //                       _smart_onhold_animator(sd);
                       //                       elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
                    }
                  else
                    {
                       if (sd->down.dragged_began)
                         {
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                            if (!sd->down.hold)
                              {
                                 sd->down.hold = 1;
                                 evas_event_feed_hold(e, 1, ev->timestamp, ev->data);
                              }
                         }
                    }
               }
             else if (!sd->freeze)
               {
                  Evas_Coord ex, ey, ew, eh;
                  double vx = 0.0, vy = 0.0;

                  evas_object_geometry_get(sd->event_obj, &ex, &ey, &ew, &eh);
                  x = ev->cur.canvas.x - ex;
                  y = ev->cur.canvas.y - ey;
                  if (x < _elm_config->thumbscroll_threshold)
                    {
                       if (_elm_config->thumbscroll_threshold > 0.0)
                         vx = -(double)(_elm_config->thumbscroll_threshold - x) /
                            _elm_config->thumbscroll_threshold;
                       else
                         vx = -1.0;
                    }
                  else if (x > (ew - _elm_config->thumbscroll_threshold))
                    {
                       if (_elm_config->thumbscroll_threshold > 0.0)
                         vx = (double)(_elm_config->thumbscroll_threshold - (ew - x)) /
                            _elm_config->thumbscroll_threshold;
                       else
                         vx = 1.0;
                    }
                  if (y < _elm_config->thumbscroll_threshold)
                    {
                       if (_elm_config->thumbscroll_threshold > 0.0)
                         vy = -(double)(_elm_config->thumbscroll_threshold - y) /
                            _elm_config->thumbscroll_threshold;
                       else
                         vy = -1.0;
                    }
                  else if (y > (eh - _elm_config->thumbscroll_threshold))
                    {
                       if (_elm_config->thumbscroll_threshold > 0.0)
                         vy = (double)(_elm_config->thumbscroll_threshold - (eh - y)) /
                            _elm_config->thumbscroll_threshold;
                       else
                         vy = 1.0;
                    }
                  if ((vx != 0.0) || (vy != 0.0))
                    {
                       sd->down.onhold_vx = vx;
                       sd->down.onhold_vy = vy;
                       if (!sd->down.onhold_animator)
                         {
                            sd->down.onhold_vxe = 0.0;
                            sd->down.onhold_vye = 0.0;
                            sd->down.onhold_tlast = 0.0;
                            sd->down.onhold_animator = ecore_animator_add(_smart_onhold_animator, sd);
                         }
                       //                       printf("b %i %i\n", sd->down.hold_x, sd->down.hold_y);
                    }
                  else
                    {
                       if (sd->down.onhold_animator)
                         {
                            ecore_animator_del(sd->down.onhold_animator);
                            sd->down.onhold_animator = NULL;
                            if (sd->child.resized)
                              _elm_smart_scroller_wanted_region_set(sd->smart_obj);
                         }
                    }
               }
          }
     }
}

static void
_smart_scrollbar_read(Smart_Data *sd)
{
   Evas_Coord x, y, mx = 0, my = 0, px, py, minx = 0, miny = 0;
   double vx, vy;

   edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.vbar", NULL, &vy);
   edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.hbar", &vx, NULL);
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
   x = vx * (double)mx + minx;
   y = vy * (double)my + miny;
   sd->pan_func.get(sd->pan_obj, &px, &py);
   sd->pan_func.set(sd->pan_obj, x, y);
   if ((px != x) || (py != y))
     edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
}

static void
_smart_scrollbar_reset(Smart_Data *sd)
{
   Evas_Coord px = 0, py = 0, minx = 0, miny = 0;

   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.vbar", 0.0, 0.0);
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.hbar", 0.0, 0.0);
   if ((!sd->child_obj) && (!sd->extern_pan))
     {
        edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.vbar", 1.0, 1.0);
        edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.hbar", 1.0, 1.0);
     }
   if (sd->pan_obj)
     {
        sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
        sd->pan_func.get(sd->pan_obj, &px, &py);
        sd->pan_func.set(sd->pan_obj, minx, miny);
     }
   if ((px != minx) || (py != miny))
     edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
}

static int
_smart_scrollbar_bar_v_visibility_adjust(Smart_Data *sd)
{
   int scroll_v_vis_change = 0;
   Evas_Coord h, vw = 0, vh = 0;

   h = sd->child.h;
   if (sd->pan_obj)
     evas_object_geometry_get(sd->pan_obj, NULL, NULL, &vw, &vh);
   if (sd->vbar_visible)
     {
        if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
          {
             if ((sd->child_obj) || (sd->extern_pan))
               {
                  if (h <= vh)
                    {
                       scroll_v_vis_change = 1;
                       sd->vbar_visible = 0;
                    }
               }
             else
               {
                  scroll_v_vis_change = 1;
                  sd->vbar_visible = 0;
               }
          }
        else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
          {
             scroll_v_vis_change = 1;
             sd->vbar_visible = 0;
          }
     }
   else
     {
        if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
          {
             if ((sd->child_obj) || (sd->extern_pan))
               {
                  if (h > vh)
                    {
                       scroll_v_vis_change = 1;
                       sd->vbar_visible = 1;
                    }
               }
          }
        else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
          {
             scroll_v_vis_change = 1;
             sd->vbar_visible = 1;
          }
     }
   if (scroll_v_vis_change)
     {
        if (sd->vbar_flags != ELM_SMART_SCROLLER_POLICY_OFF)
          {
             if (sd->vbar_visible)
               edje_object_signal_emit(sd->edje_obj, "elm,action,show,vbar", "elm");
             else
               edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
             edje_object_message_signal_process(sd->edje_obj);
             _smart_scrollbar_size_adjust(sd);
          }
        else
          edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
     }
   return scroll_v_vis_change;
}

static int
_smart_scrollbar_bar_h_visibility_adjust(Smart_Data *sd)
{
   int scroll_h_vis_change = 0;
   Evas_Coord w, vw = 0, vh = 0;

   w = sd->child.w;
   if (sd->pan_obj)
     evas_object_geometry_get(sd->pan_obj, NULL, NULL, &vw, &vh);
   if (sd->hbar_visible)
     {
        if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
          {
             if ((sd->child_obj) || (sd->extern_pan))
               {
                  if (w <= vw)
                    {
                       scroll_h_vis_change = 1;
                       sd->hbar_visible = 0;
                    }
               }
             else
               {
                  scroll_h_vis_change = 1;
                  sd->hbar_visible = 0;
               }
          }
        else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
          {
             scroll_h_vis_change = 1;
             sd->hbar_visible = 0;
          }
     }
   else
     {
        if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
          {
             if ((sd->child_obj) || (sd->extern_pan))
               {
                  if (w > vw)
                    {
                       scroll_h_vis_change = 1;
                       sd->hbar_visible = 1;
                    }
               }
          }
        else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
          {
             scroll_h_vis_change = 1;
             sd->hbar_visible = 1;
          }
     }
   if (scroll_h_vis_change)
     {
        if (sd->hbar_flags != ELM_SMART_SCROLLER_POLICY_OFF)
          {
             if (sd->hbar_visible)
               edje_object_signal_emit(sd->edje_obj, "elm,action,show,hbar", "elm");
             else
               edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
             edje_object_message_signal_process(sd->edje_obj);
             _smart_scrollbar_size_adjust(sd);
          }
        else
          edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
        _smart_scrollbar_size_adjust(sd);
     }
   return scroll_h_vis_change;
}

static void
_smart_scrollbar_bar_visibility_adjust(Smart_Data *sd)
{
   int changed = 0;

   changed |= _smart_scrollbar_bar_h_visibility_adjust(sd);
   changed |= _smart_scrollbar_bar_v_visibility_adjust(sd);
   if (changed)
     {
        _smart_scrollbar_bar_h_visibility_adjust(sd);
        _smart_scrollbar_bar_v_visibility_adjust(sd);
     }
}

static void
_smart_scrollbar_size_adjust(Smart_Data *sd)
{
   if ((sd->child_obj) || (sd->extern_pan))
     {
        Evas_Coord x, y, w, h, mx = 0, my = 0, vw = 0, vh = 0, px, py, minx = 0, miny = 0;
        double vx, vy, size;

        edje_object_part_geometry_get(sd->edje_obj, "elm.swallow.content",
                                      NULL, NULL, &vw, &vh);
        w = sd->child.w;
        if (w < 1) w = 1;
        size = (double)vw / (double)w;
        if (size > 1.0)
          {
             size = 1.0;
             edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.hbar", 0.0, 0.0);
          }
        edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.hbar", size, 1.0);

        h = sd->child.h;
        if (h < 1) h = 1;
        size = (double)vh / (double)h;
        if (size > 1.0)
          {
             size = 1.0;
             edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.vbar", 0.0, 0.0);
          }
        edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.vbar", 1.0, size);

        edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.hbar", &vx, NULL);
        edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.vbar", NULL, &vy);
        sd->pan_func.max_get(sd->pan_obj, &mx, &my);
        sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
        x = vx * mx + minx;
        y = vy * my + miny;

        edje_object_part_drag_step_set(sd->edje_obj, "elm.dragable.hbar", (double)sd->step.x / (double)w, 0.0);
        edje_object_part_drag_step_set(sd->edje_obj, "elm.dragable.vbar", 0.0, (double)sd->step.y / (double)h);
        if (sd->page.x > 0)
          edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.hbar", (double)sd->page.x / (double)w, 0.0);
        else
          edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.hbar", -((double)sd->page.x * ((double)vw / (double)w)) / 100.0, 0.0);
        if (sd->page.y > 0)
          edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.vbar", 0.0, (double)sd->page.y / (double)h);
        else
          edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.vbar", 0.0, -((double)sd->page.y * ((double)vh / (double)h)) / 100.0);

        sd->pan_func.get(sd->pan_obj, &px, &py);
        if (vx != mx) x = px;
        if (vy != my) y = py;
        sd->pan_func.set(sd->pan_obj, x, y);
        //	if ((px != 0) || (py != 0))
        //	  edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
     }
   else
     {
        Evas_Coord px = 0, py = 0, minx = 0, miny = 0;

        edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.vbar", 1.0, 1.0);
        edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.hbar", 1.0, 1.0);
        sd->pan_func.min_get(sd->pan_obj, &minx, &miny);
        sd->pan_func.get(sd->pan_obj, &px, &py);
        sd->pan_func.set(sd->pan_obj, minx, miny);
        if ((px != minx) || (py != miny))
          edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
     }
   _smart_scrollbar_bar_visibility_adjust(sd);
}

static void
_smart_reconfigure(Smart_Data *sd)
{
   evas_object_move(sd->edje_obj, sd->x, sd->y);
   evas_object_resize(sd->edje_obj, sd->w, sd->h);
   evas_object_move(sd->event_obj, sd->x, sd->y);
   evas_object_resize(sd->event_obj, sd->w, sd->h);
   _smart_scrollbar_size_adjust(sd);
   _smart_page_adjust(sd);
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   Evas_Object *o;

   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   evas_object_smart_data_set(obj, sd);

   sd->smart_obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->step.x = 32;
   sd->step.y = 32;
   sd->page.x = -50;
   sd->page.y = -50;
   sd->hbar_flags = ELM_SMART_SCROLLER_POLICY_AUTO;
   sd->vbar_flags = ELM_SMART_SCROLLER_POLICY_AUTO;
   sd->hbar_visible = 1;
   sd->vbar_visible = 1;

   sd->bounce_horiz = 1;
   sd->bounce_vert = 1;

   sd->one_dir_at_a_time = 1;
   sd->momentum_animator_disabled = EINA_FALSE;
   sd->bounce_animator_disabled = EINA_FALSE;

   o = edje_object_add(evas_object_evas_get(obj));
   evas_object_propagate_events_set(o, 0);
   sd->edje_obj = o;
   elm_smart_scroller_object_theme_set(NULL, obj, "scroller", "base", "default");
   edje_object_signal_callback_add(o, "drag", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag,start", "elm.dragable.vbar", _smart_edje_drag_v_start, sd);
   edje_object_signal_callback_add(o, "drag,stop", "elm.dragable.vbar", _smart_edje_drag_v_stop, sd);
   edje_object_signal_callback_add(o, "drag,step", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag,page", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   edje_object_signal_callback_add(o, "drag,start", "elm.dragable.hbar", _smart_edje_drag_h_start, sd);
   edje_object_signal_callback_add(o, "drag,stop", "elm.dragable.hbar", _smart_edje_drag_h_stop, sd);
   edje_object_signal_callback_add(o, "drag,step", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   edje_object_signal_callback_add(o, "drag,page", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   evas_object_smart_member_add(o, obj);

   o = evas_object_rectangle_add(evas_object_evas_get(obj));
   sd->event_obj = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _smart_event_wheel, sd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _smart_event_mouse_down, sd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _smart_event_mouse_up, sd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _smart_event_mouse_move, sd);
   evas_object_smart_member_add(o, obj);
   evas_object_repeat_events_set(o, 1);

   sd->pan_func.set = _elm_smart_pan_set;
   sd->pan_func.get = _elm_smart_pan_get;
   sd->pan_func.max_get = _elm_smart_pan_max_get;
   sd->pan_func.min_get = _elm_smart_pan_min_get;
   sd->pan_func.child_size_get = _elm_smart_pan_child_size_get;

   _smart_scrollbar_reset(sd);
}

static void
_smart_del(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   elm_smart_scroller_child_set(obj, NULL);
   if (!sd->extern_pan) evas_object_del(sd->pan_obj);
   evas_object_del(sd->edje_obj);
   evas_object_del(sd->event_obj);
   if (sd->down.hold_animator) ecore_animator_del(sd->down.hold_animator);
   if (sd->down.onhold_animator) ecore_animator_del(sd->down.onhold_animator);
   if (sd->down.momentum_animator) ecore_animator_del(sd->down.momentum_animator);
   if (sd->down.bounce_x_animator) ecore_animator_del(sd->down.bounce_x_animator);
   if (sd->down.bounce_y_animator) ecore_animator_del(sd->down.bounce_y_animator);
   if (sd->scrollto.x.animator) ecore_animator_del(sd->scrollto.x.animator);
   if (sd->scrollto.y.animator) ecore_animator_del(sd->scrollto.y.animator);
   free(sd);
   evas_object_smart_data_set(obj, NULL);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
   _elm_smart_scroller_wanted_region_set(obj);
}

static void
_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_show(sd->edje_obj);
   evas_object_show(sd->event_obj);
}

static void
_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_hide(sd->edje_obj);
   evas_object_hide(sd->event_obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->edje_obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->edje_obj, clip);
   evas_object_clip_set(sd->event_obj, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->edje_obj);
   evas_object_clip_unset(sd->event_obj);
}

/* never need to touch this */

static void
_smart_init(void)
{
   if (_smart) return;
     {
        static const Evas_Smart_Class sc =
          {
             SMART_NAME,
             EVAS_SMART_CLASS_VERSION,
             _smart_add,
             _smart_del,
             _smart_move,
             _smart_resize,
             _smart_show,
             _smart_hide,
             _smart_color_set,
             _smart_clip_set,
             _smart_clip_unset,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL
          };
        _smart = evas_smart_class_new(&sc);
     }
}
