#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_evas_wayland_private.h"
#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
# include <Evas_Engine_Wayland_Egl.h>
#endif

#define _smart_frame_type "ecore_evas_wl_frame"

static const char *interface_wl_name = "wayland";
static const int interface_wl_version = 1;

/* local structures for the frame smart object */
typedef struct _EE_Wl_Smart_Data EE_Wl_Smart_Data;
struct _EE_Wl_Smart_Data
{
   Evas_Object_Smart_Clipped_Data base;
   Evas_Object *text;
   Evas_Coord x, y, w, h;
   Evas_Object *border[4]; // 0 = top, 1 = bottom, 2 = left, 3 = right
   Evas_Coord border_size[4]; // same as border
};

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
     {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW(_smart_frame_type, _ecore_evas_wl_frame,
                        Evas_Smart_Class, Evas_Smart_Class,
                        evas_object_smart_clipped_class_get, _smart_callbacks);

/* local variables */
static int _ecore_evas_wl_init_count = 0;
static Ecore_Event_Handler *_ecore_evas_wl_event_hdls[5];

static void _ecore_evas_wayland_resize(Ecore_Evas *ee, int location);

/* local function prototypes */
static int _ecore_evas_wl_common_render_updates_process(Ecore_Evas *ee, Eina_List *updates);
void _ecore_evas_wl_common_render_updates(void *data, Evas *evas EINA_UNUSED, void *event);
static void _rotation_do(Ecore_Evas *ee, int rotation, int resize);
static void _ecore_evas_wayland_alpha_do(Ecore_Evas *ee, int alpha);
static void _ecore_evas_wayland_transparent_do(Ecore_Evas *ee, int transparent);
static void _ecore_evas_wl_common_border_update(Ecore_Evas *ee);

/* Frame listener */
static void _ecore_evas_wl_frame_complete(void *data, struct wl_callback *callback, uint32_t tm);

/* Frame listener */
static const struct wl_callback_listener frame_listener =
{
   _ecore_evas_wl_frame_complete,
};

/* local functions */
static void 
_ecore_evas_wl_common_state_update(Ecore_Evas *ee)
{
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
}

static int 
_ecore_evas_wl_common_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;

   if ((ee->visible) && (updates))
     {
        /* Eina_List *l = NULL; */
        /* Eina_Rectangle *r; */
        /* Ecore_Evas_Engine_Wl_Data *wdata; */

        /* if (!(wdata = ee->engine.data)) return 0; */

        /* EINA_LIST_FOREACH(updates, l, r) */
        /*   ecore_wl_window_damage(wdata->win, */
        /*                          r->x, r->y, r->w, r->h); */

        /* ecore_wl_window_commit(wdata->win); */
        /* ecore_wl_flush(); */

        _ecore_evas_idle_timeout_update(ee);
        rend = 1;
     }
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}

static Eina_Bool
_ecore_evas_wl_common_cb_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->in) return ECORE_CALLBACK_PASS_ON;

   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->timestamp);
   ee->in = EINA_TRUE;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   if (ee->in)
     {
        ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
        _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->timestamp);
        evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
        if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
        if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
        ee->in = EINA_FALSE;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->prop.focused) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = EINA_TRUE;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (!ee->prop.focused) return ECORE_CALLBACK_PASS_ON;
   evas_focus_out(ee->evas);
   ee->prop.focused = EINA_FALSE;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_window_configure(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Wl_Event_Window_Configure *ev;
   int nw = 0, nh = 0;
   int orig_w = 0, orig_h = 0;
   Eina_Bool prev_max, prev_full;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   wdata = ee->engine.data;
   if (!wdata) return ECORE_CALLBACK_PASS_ON;

   prev_max = ee->prop.maximized;
   prev_full = ee->prop.fullscreen;
   ee->prop.maximized = ecore_wl_window_maximized_get(wdata->win);
   ee->prop.fullscreen = ecore_wl_window_fullscreen_get(wdata->win);

   nw = ev->w;
   nh = ev->h;
   if (nw < 1) nw = 1;
   if (nh < 1) nh = 1;

   if (!ee->prop.fullscreen)
     {
        int fw = 0, fh = 0;
        int maxw, maxh;
        int minw, minh;
        double a;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             minw = (ee->prop.min.w + fw);
             minh = (ee->prop.min.h + fh);
             maxw = (ee->prop.max.w + fw);
             maxh = (ee->prop.max.h + fh);
          }
        else
          {
             minw = (ee->prop.min.w + fh);
             minh = (ee->prop.min.h + fw);
             maxw = (ee->prop.max.w + fh);
             maxh = (ee->prop.max.h + fw);
          }

        /* adjust size using aspect */
        if ((ee->prop.base.w >= 0) && (ee->prop.base.h >= 0))
          {
             int bw, bh;

             bw = (nw - ee->prop.base.w);
             bh = (nh - ee->prop.base.h);
             if (bw < 1) bw = 1;
             if (bh < 1) bh = 1;
             a = ((double)bw / (double)bh);
             if ((ee->prop.aspect != 0.0) && (a < ee->prop.aspect))
               {
                  if ((nh < ee->h) > 0)
                    bw = bh * ee->prop.aspect;
                  else
                    bw = bw / ee->prop.aspect;

                  nw = bw + ee->prop.base.w;
                  nh = bh + ee->prop.base.h;
               }
             else if ((ee->prop.aspect != 0.0) && (a > ee->prop.aspect))
               {
                  bw = bh * ee->prop.aspect;
                  nw = bw + ee->prop.base.w;
               }
          }
        else
          {
             a = ((double)nw / (double)nh);
             if ((ee->prop.aspect != 0.0) && (a < ee->prop.aspect))
               {
                  if ((nh < ee->h) > 0)
                    nw = nh * ee->prop.aspect;
                  else
                    nh = nw / ee->prop.aspect;
               }
             else if ((ee->prop.aspect != 0.0) && (a > ee->prop.aspect))
               nw = nh * ee->prop.aspect;
          }

        /* adjust size using base size & step size */
        if (ee->prop.step.w > 0)
          {
             if (ee->prop.base.w >= 0)
               nw = (ee->prop.base.w + 
                     (((nw - ee->prop.base.w) / ee->prop.step.w) * 
                         ee->prop.step.w));
             else
               nw = (minw + (((nw - minw) / ee->prop.step.w) * ee->prop.step.w));
          }

        if (ee->prop.step.h > 0)
          {
             if (ee->prop.base.h >= 0)
               nh = (ee->prop.base.h + 
                     (((nh - ee->prop.base.h) / ee->prop.step.h) * 
                         ee->prop.step.h));
             else
               nh = (minh + (((nh - minh) / ee->prop.step.h) * ee->prop.step.h));
          }

        if (nw > maxw) nw = maxw;
        else if (nw < minw) nw = minw;
        if (nh > maxh) nh = maxh;
        else if (nh < minh) nh = minh;

        orig_w = nw;
        orig_h = nh;

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             nw -= fw;
             nh -= fh;
          }
        else
          {
             nw -= fh;
             nh -= fw;
          }
     }

   if (prev_full != ee->prop.fullscreen)
     _ecore_evas_wl_common_border_update(ee);

   if (ee->prop.fullscreen)
     {
        _ecore_evas_wl_common_move(ee, ev->x, ev->y);
        _ecore_evas_wl_common_resize(ee, nw, nh);

        if (prev_full != ee->prop.fullscreen)
          _ecore_evas_wl_common_state_update(ee);

        return ECORE_CALLBACK_PASS_ON;
     }

   if ((ee->x != ev->x) || (ee->y != ev->y))
     _ecore_evas_wl_common_move(ee, ev->x, ev->y);

   if ((ee->req.w != orig_w) || (ee->req.h != orig_h))
     _ecore_evas_wl_common_resize(ee, nw, nh);

   if ((prev_max != ee->prop.maximized) ||
       (prev_full != ee->prop.fullscreen))
     _ecore_evas_wl_common_state_update(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static void
_rotation_do(Ecore_Evas *ee, int rotation, int resize)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   int rot_dif;

   wdata = ee->engine.data;

   /* calculate difference in rotation */
   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;

   /* set ecore_wayland window rotation */
   ecore_wl_window_rotation_set(wdata->win, rotation);

   /* check if rotation is just a flip */
   if (rot_dif != 180)
     {
        int minw, minh, maxw, maxh;
        int basew, baseh, stepw, steph;

        /* check if we are rotating with resize */
        if (!resize)
          {
             int fw, fh;
             int ww, hh;

             /* grab framespace width & height */
             evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

             /* check for fullscreen */
             if (!ee->prop.fullscreen)
               {
                  /* resize the ecore_wayland window */
                  ecore_wl_window_resize(wdata->win,
                                         ee->req.h + fw, ee->req.w + fh, 0);
               }
             else
               {
                  /* resize the canvas based on rotation */
                  if ((rotation == 0) || (rotation == 180))
                    {
                       /* resize the ecore_wayland window */
                       ecore_wl_window_resize(wdata->win, 
                                              ee->req.w, ee->req.h, 0);

                       /* resize the canvas */
                       evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
                       evas_output_viewport_set(ee->evas, 0, 0, 
                                                ee->req.w, ee->req.h);
                    }
                  else
                    {
                       /* resize the ecore_wayland window */
                       ecore_wl_window_resize(wdata->win, 
                                              ee->req.h, ee->req.w, 0);

                       /* resize the canvas */
                       evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
                       evas_output_viewport_set(ee->evas, 0, 0, 
                                                ee->req.h, ee->req.w);
                    }
               }

             /* add canvas damage */
             if (ECORE_EVAS_PORTRAIT(ee))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.h, ee->req.w);
             ww = ee->h;
             hh = ee->w;
             ee->w = ww;
             ee->h = hh;
             ee->req.w = ww;
             ee->req.h = hh;
          }
        else
          {
             /* resize the canvas based on rotation */
             if ((rotation == 0) || (rotation == 180))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }

             /* call the ecore_evas' resize function */
             if (ee->func.fn_resize) ee->func.fn_resize(ee);

             /* add canvas damage */
             if (ECORE_EVAS_PORTRAIT(ee))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
          }

        /* get min, max, base, & step sizes */
        ecore_evas_size_min_get(ee, &minw, &minh);
        ecore_evas_size_max_get(ee, &maxw, &maxh);
        ecore_evas_size_base_get(ee, &basew, &baseh);
        ecore_evas_size_step_get(ee, &stepw, &steph);

        /* record the current rotation of the ecore_evas */
        ee->rotation = rotation;

        /* reset min, max, base, & step sizes */
        ecore_evas_size_min_set(ee, minh, minw);
        ecore_evas_size_max_set(ee, maxh, maxw);
        ecore_evas_size_base_set(ee, baseh, basew);
        ecore_evas_size_step_set(ee, steph, stepw);

        /* send a mouse_move process
         *
         * NB: Is This Really Needed ?
         * Yes, it's required to update the mouse position, relatively to
         * widgets. After a rotation change, e.g., the mouse might not be over
         * a button anymore. */
        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       ecore_loop_time_get());
     }
   else
     {
        /* resize the ecore_wayland window */
        ecore_wl_window_resize(wdata->win, ee->w, ee->h, 0);

        /* record the current rotation of the ecore_evas */
        ee->rotation = rotation;

        /* send a mouse_move process
         *
         * NB: Is This Really Needed ? Yes, it's required to update the mouse
         * position, relatively to widgets. */
        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       ecore_loop_time_get());

        /* call the ecore_evas' resize function */
        if (ee->func.fn_resize) ee->func.fn_resize(ee);

        /* add canvas damage */
        if (ECORE_EVAS_PORTRAIT(ee))
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
        else
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
}

void
_ecore_evas_wl_common_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   if (ee->in_async_render)
     {
        ee->delayed.rotation = rotation;
        ee->delayed.rotation_resize = resize;
        ee->delayed.rotation_changed = EINA_TRUE;
        return;
     }
   _rotation_do(ee, rotation, resize);
}

int
_ecore_evas_wl_common_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_ecore_evas_wl_init_count != 1)
     return _ecore_evas_wl_init_count;

   _ecore_evas_wl_event_hdls[0] =
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_IN,
                             _ecore_evas_wl_common_cb_mouse_in, NULL);
   _ecore_evas_wl_event_hdls[1] =
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_OUT,
                             _ecore_evas_wl_common_cb_mouse_out, NULL);
   _ecore_evas_wl_event_hdls[2] =
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_IN,
                             _ecore_evas_wl_common_cb_focus_in, NULL);
   _ecore_evas_wl_event_hdls[3] =
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_OUT,
                             _ecore_evas_wl_common_cb_focus_out, NULL);
   _ecore_evas_wl_event_hdls[4] =
     ecore_event_handler_add(ECORE_WL_EVENT_WINDOW_CONFIGURE,
                             _ecore_evas_wl_common_cb_window_configure, NULL);

   ecore_event_evas_init();

   return _ecore_evas_wl_init_count;
}

int
_ecore_evas_wl_common_shutdown(void)
{
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (--_ecore_evas_wl_init_count != 0)
     return _ecore_evas_wl_init_count;

   for (i = 0; i < sizeof(_ecore_evas_wl_event_hdls) / sizeof(Ecore_Event_Handler *); i++)
     {
        if (_ecore_evas_wl_event_hdls[i])
          ecore_event_handler_del(_ecore_evas_wl_event_hdls[i]);
     }

   ecore_event_evas_shutdown();

   return _ecore_evas_wl_init_count;
}

void
_ecore_evas_wl_common_pre_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (wdata->frame) evas_object_del(wdata->frame);
}

void
_ecore_evas_wl_common_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wdata = ee->engine.data;
   if (wdata->frame_callback) wl_callback_destroy(wdata->frame_callback);
   wdata->frame_callback = NULL;
   if (wdata->win) ecore_wl_window_free(wdata->win);
   wdata->win = NULL;
   free(wdata);

   ecore_event_window_unregister(ee->prop.window);
   ecore_evas_input_event_unregister(ee);

   _ecore_evas_wl_common_shutdown();
   ecore_wl_shutdown();
}

void
_ecore_evas_wl_common_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;
   int orig_w, orig_h;
   int ow, oh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->req.w = w;
   ee->req.h = h;
   orig_w = w;
   orig_h = h;

   if (!ee->prop.fullscreen)
     {
        int fw = 0, fh = 0;
        int maxw, maxh;
        int minw, minh;
        double a;
        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             minw = (ee->prop.min.w - fw);
             minh = (ee->prop.min.h - fh);
             maxw = (ee->prop.max.w + fw);
             maxh = (ee->prop.max.h + fh);
          }
        else
          {
             minw = (ee->prop.min.w - fh);
             minh = (ee->prop.min.h - fw);
             maxw = (ee->prop.max.w + fh);
             maxh = (ee->prop.max.h + fw);
          }

        /* adjust size using aspect */
        if ((ee->prop.base.w >= 0) && (ee->prop.base.h >= 0))
          {
             int bw, bh;

             bw = (w - ee->prop.base.w);
             bh = (h - ee->prop.base.h);
             if (bw < 1) bw = 1;
             if (bh < 1) bh = 1;
             a = ((double)bw / (double)bh);
             if ((ee->prop.aspect != 0.0) && (a < ee->prop.aspect))
               {
                  if ((h < ee->h) > 0)
                    bw = bh * ee->prop.aspect;
                  else
                    bw = bw / ee->prop.aspect;

                  w = bw + ee->prop.base.w;
                  h = bh + ee->prop.base.h;
               }
             else if ((ee->prop.aspect != 0.0) && (a > ee->prop.aspect))
               {
                  bw = bh * ee->prop.aspect;
                  w = bw + ee->prop.base.w;
               }
          }
        else
          {
             a = ((double)w / (double)h);
             if ((ee->prop.aspect != 0.0) && (a < ee->prop.aspect))
               {
                  if ((h < ee->h) > 0)
                    w = h * ee->prop.aspect;
                  else
                    h = w / ee->prop.aspect;
               }
             else if ((ee->prop.aspect != 0.0) && (a > ee->prop.aspect))
               w = h * ee->prop.aspect;
          }

        /* calc new size using base size & step size */
        if (ee->prop.step.w > 0)
          {
             if (ee->prop.base.w >= 0)
               w = (ee->prop.base.w + 
                    (((w - ee->prop.base.w) / ee->prop.step.w) * 
                        ee->prop.step.w));
             else
               w = (minw + (((w - minw) / ee->prop.step.w) * ee->prop.step.w));
          }

        if (ee->prop.step.h > 0)
          {
             if (ee->prop.base.h >= 0)
               h = (ee->prop.base.h + 
                    (((h - ee->prop.base.h) / ee->prop.step.h) * 
                        ee->prop.step.h));
             else
               h = (minh + (((h - minh) / ee->prop.step.h) * ee->prop.step.h));
          }

        if (w > maxw) w = maxw;
        else if (w < minw) w = minw;
        if (h > maxh) h = maxh;
        else if (h < minh) h = minh;

        orig_w = w;
        orig_h = h;

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             w += fw;
             h += fh;
          }
        else
          {
             w += fh;
             h += fw;
          }
     }

   evas_output_size_get(ee->evas, &ow, &oh);
   if ((ow != w) || (oh != h))
     {
        ee->w = orig_w;
        ee->h = orig_h;
        ee->req.w = orig_w;
        ee->req.h = orig_h;

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             evas_output_size_set(ee->evas, w, h);
             evas_output_viewport_set(ee->evas, 0, 0, w, h);
          }
        else
          {
             evas_output_size_set(ee->evas, h, w);
             evas_output_viewport_set(ee->evas, 0, 0, h, w);
          }

        if (ee->prop.avoid_damage)
          {
             int pdam = 0;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }

        if (wdata->frame)
          evas_object_resize(wdata->frame, w, h);

        if (wdata->win)
          ecore_wl_window_update_size(wdata->win, w, h);

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

void
_ecore_evas_wl_common_callback_resize_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_resize = func;
}

void
_ecore_evas_wl_common_callback_move_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_move = func;
}

void
_ecore_evas_wl_common_callback_delete_request_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_delete_request = func;
}

void
_ecore_evas_wl_common_callback_focus_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_focus_in = func;
}

void
_ecore_evas_wl_common_callback_focus_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_focus_out = func;
}

void
_ecore_evas_wl_common_callback_mouse_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_mouse_in = func;
}

void
_ecore_evas_wl_common_callback_mouse_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_mouse_out = func;
}

void
_ecore_evas_wl_common_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;

   wdata = ee->engine.data;
   ee->req.x = x;
   ee->req.y = y;

   if ((ee->x != x) || (ee->y != y))
     {
        ee->x = x;
        ee->y = y;
        if (wdata->win)
          ecore_wl_window_update_location(wdata->win, x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

/* Frame border:
 *
 * |------------------------------------------|
 * |                top border                |
 * |------------------------------------------|
 * |       |                         |        |
 * |       |                         |        |
 * |       |                         |        |
 * |       |                         |        |
 * |left   |                         | right  |
 * |border |                         | border |
 * |       |                         |        |
 * |       |                         |        |
 * |       |                         |        |
 * |------------------------------------------|
 * |                bottom border             |
 * |------------------------------------------|
 */
static void
_border_size_eval(Evas_Object *obj EINA_UNUSED, EE_Wl_Smart_Data *sd)
{

   /* top border */
   if (sd->border[0])
     {
        evas_object_move(sd->border[0], sd->x, sd->y);
        evas_object_resize(sd->border[0], sd->w, sd->border_size[0]);
     }

   /* bottom border */
   if (sd->border[1])
     {
        evas_object_move(sd->border[1], sd->x, sd->y + sd->h - sd->border_size[1]);
        evas_object_resize(sd->border[1], sd->w, sd->border_size[1]);
     }

   /* left border */
   if (sd->border[2])
     {
        evas_object_move(sd->border[2], sd->x, sd->y + sd->border_size[0]);
        evas_object_resize(sd->border[2], sd->border_size[2],
                           sd->h - sd->border_size[0] - sd->border_size[1]);
     }

   /* right border */
   if (sd->border[3])
     {
        evas_object_move(sd->border[3], sd->x + sd->w - sd->border_size[3],
                         sd->y + sd->border_size[0]);
        evas_object_resize(sd->border[3], sd->border_size[3],
                           sd->h - sd->border_size[0] - sd->border_size[1]);
     }
}

static void
_ecore_evas_wl_common_smart_add(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;
   Evas *evas;
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   EVAS_SMART_DATA_ALLOC(obj, EE_Wl_Smart_Data);

   _ecore_evas_wl_frame_parent_sc->add(obj);

   sd = priv;

   evas = evas_object_evas_get(obj);

   sd->x = 0;
   sd->y = 0;
   sd->w = 1;
   sd->h = 1;

   for (i = 0; i < 4; i++)
     {
        sd->border[i] = NULL;
        sd->border_size[i] = 0;
     }

   sd->text = evas_object_text_add(evas);
   evas_object_color_set(sd->text, 0, 0, 0, 255);
   evas_object_text_style_set(sd->text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(sd->text, "Sans", 10);
   evas_object_text_text_set(sd->text, "Smart Test");
   evas_object_show(sd->text);
   evas_object_smart_member_add(sd->text, obj);
}

static void
_ecore_evas_wl_common_smart_del(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_del(sd->text);
   for (i = 0; i < 4; i++)
     {
        evas_object_del(sd->border[i]);
     }
   _ecore_evas_wl_frame_parent_sc->del(obj);
}

static void
_ecore_evas_wl_common_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_evas_wl_frame_parent_sc->move(obj, x, y);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;

   evas_object_smart_changed(obj);
}

static void
_ecore_evas_wl_common_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;

   evas_object_smart_changed(obj);
}

void
_ecore_evas_wl_common_smart_calculate(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;

   _border_size_eval(obj, sd);
}

static void
_ecore_evas_wl_frame_smart_set_user(Evas_Smart_Class *sc)
{
   sc->add = _ecore_evas_wl_common_smart_add;
   sc->del = _ecore_evas_wl_common_smart_del;
   sc->move = _ecore_evas_wl_common_smart_move;
   sc->resize = _ecore_evas_wl_common_smart_resize;
   sc->calculate = _ecore_evas_wl_common_smart_calculate;
}

Evas_Object *
_ecore_evas_wl_common_frame_add(Evas *evas)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return evas_object_smart_add(evas, _ecore_evas_wl_frame_smart_class_new());
}

/*
 * Size is received in the same format as it is used to set the framespace
 * offset size.
 */
void
_ecore_evas_wl_common_frame_border_size_set(Evas_Object *obj, int fx, int fy, int fw, int fh)
{
   EE_Wl_Smart_Data *sd;
   Evas *e;
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;

   e = evas_object_evas_get(obj);

   sd->border_size[0] = fy;
   sd->border_size[1] = fh - fy;
   sd->border_size[2] = fx;
   sd->border_size[3] = fw - fx;

   for (i = 0; i < 4; i++)
     {
        if ((sd->border_size[i] <= 0) && (sd->border[i]))
          {
             evas_object_del(sd->border[i]);
             sd->border[i] = NULL;
          }
        else if ((sd->border_size[i] > 0) && (!sd->border[i]))
          {
             sd->border[i] = evas_object_rectangle_add(e);
             evas_object_color_set(sd->border[i], 249, 249, 249, 255);
             evas_object_show(sd->border[i]);
             evas_object_smart_member_add(sd->border[i], obj);
          }
     }
   evas_object_raise(sd->text);
}

void
_ecore_evas_wl_common_frame_border_size_get(Evas_Object *obj, int *fx, int *fy, int *fw, int *fh)
{
   EE_Wl_Smart_Data *sd;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;

   if (fx) *fx = sd->border_size[2];
   if (fy) *fy = sd->border_size[0];
   if (fw) *fw = sd->border_size[2] + sd->border_size[3];
   if (fh) *fh = sd->border_size[0] + sd->border_size[1];
}

void 
_ecore_evas_wl_common_pointer_xy_get(const Ecore_Evas *ee EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_wl_pointer_xy_get(x, y);
}

void
_ecore_evas_wl_common_raise(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   wdata = ee->engine.data;
   ecore_wl_window_raise(wdata->win);
}

void
_ecore_evas_wl_common_title_set(Ecore_Evas *ee, const char *title)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   wdata = ee->engine.data;
   if ((ee->prop.draw_frame) && (wdata->frame))
     {
        EE_Wl_Smart_Data *sd;

        if ((sd = evas_object_smart_data_get(wdata->frame)))
          evas_object_text_text_set(sd->text, ee->prop.title);
     }

   if (ee->prop.title)
     ecore_wl_window_title_set(wdata->win, ee->prop.title);
}

void
_ecore_evas_wl_common_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   if (n) ee->prop.name = strdup(n);
   if (c) ee->prop.clas = strdup(c);

   if (ee->prop.clas)
     ecore_wl_window_class_name_set(wdata->win, ee->prop.clas);
}

void
_ecore_evas_wl_common_size_min_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
}

void
_ecore_evas_wl_common_size_max_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
}

void
_ecore_evas_wl_common_size_base_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
}

void
_ecore_evas_wl_common_size_step_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
}

void 
_ecore_evas_wl_common_aspect_set(Ecore_Evas *ee, double aspect)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->prop.aspect == aspect) return;
   ee->prop.aspect = aspect;
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee) ee->prop.cursor.object = NULL;
}

void
_ecore_evas_wl_common_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y, fx, fy;
   Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;
   Evas_Object *old;
   
   old = ee->prop.cursor.object;
   if (obj == NULL)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        goto end;
     }
   
   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;
   
   if (obj != old)
     {
        ecore_wl_window_pointer_set(wdata->win, NULL, 0, 0);
        evas_pointer_output_xy_get(ee->evas, &x, &y);
        evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
        evas_object_pass_events_set(ee->prop.cursor.object, 1);
        if (evas_pointer_inside_get(ee->evas))
          evas_object_show(ee->prop.cursor.object);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _ecore_evas_object_cursor_del, ee);
     }
   evas_output_framespace_get(ee->evas, &fx, &fy, NULL, NULL);
   evas_object_move(ee->prop.cursor.object,
                    x - fx - ee->prop.cursor.hot.x,
                    y - fy - ee->prop.cursor.hot.y);
end:
   if ((old) && (obj != old))
     {
        evas_object_event_callback_del_full
          (old, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
        evas_object_del(old);
     }
}

void
_ecore_evas_wl_common_layer_set(Ecore_Evas *ee, int layer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.layer == layer) return;
   if (layer < 1) layer = 1;
   else if (layer > 255) layer = 255;
   ee->prop.layer = layer;
   _ecore_evas_wl_common_state_update(ee);
}

void
_ecore_evas_wl_common_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.iconified == on) return;
   ee->prop.iconified = on;
   /* FIXME: Implement this in Wayland someshow */
}

static void
_ecore_evas_wl_common_border_update(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   wdata = ee->engine.data;
   if (!wdata->frame)
     return;

   if ((ee->prop.borderless) || (ee->prop.fullscreen))
     {
        evas_object_hide(wdata->frame);
        evas_output_framespace_set(ee->evas, 0, 0, 0, 0);
     }
   else
     {
        int fx = 0, fy = 0, fw = 0, fh = 0;

        evas_object_show(wdata->frame);
        _ecore_evas_wl_common_frame_border_size_get(wdata->frame,
                                                    &fx, &fy, &fw, &fh);
        evas_output_framespace_set(ee->evas, fx, fy, fw, fh);
     }
}

void
_ecore_evas_wl_common_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.borderless == on) return;
   ee->prop.borderless = on;

   _ecore_evas_wl_common_border_update(ee);
   _ecore_evas_wl_common_state_update(ee);
}

void
_ecore_evas_wl_common_maximized_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.maximized == on) return;
   wdata = ee->engine.data;
   ecore_wl_window_maximized_set(wdata->win, on);
//   _ecore_evas_wl_common_state_update(ee);
}

void
_ecore_evas_wl_common_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.fullscreen == on) return;
   wdata = ee->engine.data;
   ecore_wl_window_fullscreen_set(wdata->win, on);
//   _ecore_evas_wl_common_state_update(ee);
}

void
_ecore_evas_wl_common_ignore_events_set(Ecore_Evas *ee, int ignore)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->ignore_events = ignore;
   /* NB: Hmmm, may need to pass this to ecore_wl_window in the future */
}

int
_ecore_evas_wl_common_pre_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *ll = NULL;
   Ecore_Evas *ee2 = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->in_async_render)
     {
        /* EDBG("ee=%p is rendering asynchronously, skip", ee); */
        return 0;
     }

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   return rend;
}

void 
_ecore_evas_wl_common_render_updates(void *data, Evas *evas EINA_UNUSED, void *event)
{
   Evas_Event_Render_Post *ev = event;
   Ecore_Evas *ee = data;

   if (!(ee) || !(ev)) return;

   ee->in_async_render = EINA_FALSE;

   _ecore_evas_wl_common_render_updates_process(ee, ev->updated_area);

   if (ee->delayed.alpha_changed)
     {
        _ecore_evas_wayland_alpha_do(ee, ee->delayed.alpha);
        ee->delayed.alpha_changed = EINA_FALSE;
     }
   if (ee->delayed.transparent_changed)
     {
        _ecore_evas_wayland_transparent_do(ee, ee->delayed.transparent);
        ee->delayed.transparent_changed = EINA_FALSE;
     }
   if (ee->delayed.rotation_changed)
     {
        _rotation_do(ee, ee->delayed.rotation, ee->delayed.rotation_resize);
        ee->delayed.rotation_changed = EINA_FALSE;
     }
}

void
_ecore_evas_wl_common_post_render(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_evas_idle_timeout_update(ee);
   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
}

void
_ecore_evas_wl_common_frame_callback_clean(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   wdata = ee->engine.data;

   if (!wdata->frame_pending)
     return;
   wl_callback_destroy(wdata->frame_callback);
   wdata->frame_callback = NULL;
   wdata->frame_pending = EINA_FALSE;
}

static void
_ecore_evas_wl_frame_complete(void *data, struct wl_callback *callback EINA_UNUSED, uint32_t tm EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Wl_Window *win = NULL;
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;

   _ecore_evas_wl_common_frame_callback_clean(ee);

   wdata = ee->engine.data;
   if (!(win = wdata->win)) return;

   if (ecore_wl_window_surface_get(win))
     {
        wdata->frame_callback = 
          wl_surface_frame(ecore_wl_window_surface_get(win));
        wl_callback_add_listener(wdata->frame_callback, &frame_listener, ee);
     }
}

int
_ecore_evas_wl_common_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *l;
   Ecore_Evas *ee2;
   Ecore_Wl_Window *win = NULL;
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!(wdata = ee->engine.data)) return 0;
   if (!(win = wdata->win)) return 0;

   /* TODO: handle comp no sync */

   if (ee->in_async_render) return 0;
   if (!ee->visible)
     {
        evas_norender(ee->evas);
        return 0;
     }

   EINA_LIST_FOREACH(ee->sub_ecore_evas, l, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   if (!ee->can_async_render)
     {
        if (!wdata->frame_pending)
          {
             Eina_List *updates;

             if (!wdata->frame_callback)
               {
                  wdata->frame_callback = 
                    wl_surface_frame(ecore_wl_window_surface_get(win));
                  wl_callback_add_listener(wdata->frame_callback, 
                                           &frame_listener, ee);
               }

             updates = evas_render_updates(ee->evas);
             rend = _ecore_evas_wl_common_render_updates_process(ee, updates);
             evas_render_updates_free(updates);

             if (rend) 
               wdata->frame_pending = EINA_TRUE;
          }
     }
   else if (evas_render_async(ee->evas))
     {
        ee->in_async_render = EINA_TRUE;
        rend = 1;
     }

   return rend;
}

void
_ecore_evas_wl_common_withdrawn_set(Ecore_Evas *ee, Eina_Bool on)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->prop.withdrawn == on) return;

   ee->prop.withdrawn = on;

   if (on)
     ecore_evas_hide(ee);
   else
     ecore_evas_show(ee);

   _ecore_evas_wl_common_state_update(ee);
}

void
_ecore_evas_wl_common_screen_geometry_get(const Ecore_Evas *ee EINA_UNUSED, int *x, int *y, int *w, int *h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   ecore_wl_screen_size_get(w, h);
}

void
_ecore_evas_wl_common_screen_dpi_get(const Ecore_Evas *ee EINA_UNUSED, int *xdpi, int *ydpi)
{
   int dpi = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (xdpi) *xdpi = 0;
   if (ydpi) *ydpi = 0;
   /* FIXME: Ideally this needs to get the DPI from a specific screen */
   dpi = ecore_wl_dpi_get();
   if (xdpi) *xdpi = dpi;
   if (ydpi) *ydpi = dpi;
}

static void
_ecore_evas_wayland_resize(Ecore_Evas *ee, int location)
{
   if (!ee) return;
   if (!strcmp(ee->driver, "wayland_shm"))
     {
#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
        _ecore_evas_wayland_shm_resize(ee, location);
#endif
     }
   else if (!strcmp(ee->driver, "wayland_egl"))
     {
#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
        _ecore_evas_wayland_egl_resize(ee, location);
#endif
     }
}

static void
_ecore_evas_wayland_alpha_do(Ecore_Evas *ee, int alpha)
{
   if (!ee) return;
   if (!strcmp(ee->driver, "wayland_shm"))
     {
#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
        _ecore_evas_wayland_shm_alpha_do(ee, alpha);
#endif
     }
}

static void
_ecore_evas_wayland_transparent_do(Ecore_Evas *ee, int transparent)
{
   if (!ee) return;
   if (!strcmp(ee->driver, "wayland_shm"))
     {
#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
        _ecore_evas_wayland_shm_transparent_do(ee, transparent);
#endif
     }
}

static void
_ecore_evas_wayland_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   if (!strncmp(ee->driver, "wayland", 7))
     {
	wdata = ee->engine.data;
        if (wdata->win)
          ecore_wl_window_move(wdata->win, x, y);
     }
}

static void
_ecore_evas_wayland_type_set(Ecore_Evas *ee, int type)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   wdata = ee->engine.data;
   ecore_wl_window_type_set(wdata->win, type);
}

static Ecore_Wl_Window *
_ecore_evas_wayland_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!(!strncmp(ee->driver, "wayland", 7)))
     return NULL;

   wdata = ee->engine.data;
   return wdata->win;
}


#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
static void
_ecore_evas_wayland_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e))
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   if (!(!strcmp(ee->driver, "wayland_egl"))) return;

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->callback.pre_swap = pre_cb;
        einfo->callback.post_swap = post_cb;
        einfo->callback.data = data;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
     }
}
#endif

static void
_ecore_evas_wayland_pointer_set(Ecore_Evas *ee EINA_UNUSED, int hot_x EINA_UNUSED, int hot_y EINA_UNUSED)
{

}

Ecore_Evas_Interface_Wayland *
_ecore_evas_wl_interface_new(void)
{
   Ecore_Evas_Interface_Wayland *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Wayland));
   if (!iface) return NULL;

   iface->base.name = interface_wl_name;
   iface->base.version = interface_wl_version;

   iface->resize = _ecore_evas_wayland_resize;
   iface->move = _ecore_evas_wayland_move;
   iface->pointer_set = _ecore_evas_wayland_pointer_set;
   iface->type_set = _ecore_evas_wayland_type_set;
   iface->window_get = _ecore_evas_wayland_window_get;

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
   iface->pre_post_swap_callback_set = 
     _ecore_evas_wayland_pre_post_swap_callback_set;
#endif

   return iface;
}
