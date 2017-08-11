#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_evas_wayland_private.h"
#include <Evas_Engine_Wayland.h>

extern EAPI Eina_List *_evas_canvas_image_data_unset(Evas *eo_e);
extern EAPI void _evas_canvas_image_data_regenerate(Eina_List *list);

#define _smart_frame_type "ecore_evas_wl_frame"

static const char *interface_wl_name = "wayland";
static const int interface_wl_version = 1;

Eina_List *ee_list;

/* local structure for evas devices with IDs */
typedef struct _EE_Wl_Device EE_Wl_Device;
struct _EE_Wl_Device
{
   Evas_Device *seat;
   Evas_Device *pointer;
   Evas_Device *keyboard;
   Evas_Device *touch;
   unsigned int id;
};

/* local variables */
static int _ecore_evas_wl_init_count = 0;
static Ecore_Event_Handler *_ecore_evas_wl_event_hdls[17];

static void _ecore_evas_wayland_resize(Ecore_Evas *ee, int location);
static void _ecore_evas_wl_common_rotation_set(Ecore_Evas *ee, int rotation, int resize);

/* local functions */
static void
_ecore_evas_wl_common_state_update(Ecore_Evas *ee)
{
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
}

static void
_ecore_evas_wl_common_wm_rotation_protocol_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   wdata = ee->engine.data;

   ee->prop.wm_rot.supported =
     ecore_wl2_window_wm_rotation_supported_get(wdata->win);
}

static Eina_Bool
_ecore_evas_wl_common_cb_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Event_Mouse_IO *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (_ecore_evas_mouse_in_check(ee, ev->dev)) return ECORE_CALLBACK_PASS_ON;

   _ecore_evas_mouse_inout_set(ee, ev->dev, EINA_TRUE, EINA_FALSE);
   ecore_event_evas_seat_modifier_lock_update(ee->evas, ev->modifiers, ev->dev);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
   _ecore_evas_mouse_device_move_process(ee, ev->dev, ev->x, ev->y, ev->timestamp);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Event_Mouse_IO *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (!_ecore_evas_mouse_in_check(ee, ev->dev)) return ECORE_CALLBACK_PASS_ON;

   ecore_event_evas_seat_modifier_lock_update(ee->evas,
                                              ev->modifiers, ev->dev);
   _ecore_evas_mouse_device_move_process(ee, ev->dev, ev->x, ev->y, ev->timestamp);
   evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
   _ecore_evas_mouse_inout_set(ee, ev->dev, EINA_FALSE, EINA_FALSE);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_focus_device_set(ee, ev->dev, EINA_TRUE);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Focus_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_focus_device_set(ee, ev->dev, EINA_FALSE);
   return ECORE_CALLBACK_PASS_ON;
}

static void
_ee_display_unset(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;

   einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   einfo->info.wl_display = NULL;
   einfo->info.wl_surface = NULL;

   wdata = ee->engine.data;
   if (!strcmp(ee->driver, "wayland_egl"))
     wdata->regen_objs = _evas_canvas_image_data_unset(ecore_evas_get(ee));

   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     WRN("Failed to set Evas Engine Info for '%s'", ee->driver);
}

static Eina_Bool
_ecore_evas_wl_common_cb_disconnect(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Disconnect *ev = event;
   Eina_List *l;
   Ecore_Evas *ee;

   EINA_LIST_FOREACH(ee_list, l, ee)
     {
        Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;

        if (wdata->display != ev->display) continue;
        if (wdata->anim_callback) wl_callback_destroy(wdata->anim_callback);
        wdata->anim_callback = NULL;
        wdata->sync_done = EINA_FALSE;
        wdata->defer_show = EINA_TRUE;
        ee->visible = EINA_FALSE;
        wdata->reset_pending = 1;
        ecore_evas_manual_render_set(ee, 1);
        _ee_display_unset(ee);
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_evas_wl_common_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   int ow, oh, ew, eh;
   int diff = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;

   wdata = ee->engine.data;
   if (!wdata) return;

   ee->req.w = w;
   ee->req.h = h;

   /* TODO: wayland client can resize the ecore_evas directly.
    * In the future, we will remove ee->req value in wayland backend */
   ew = ee->w;
   eh = ee->h;
   ee->w = w;
   ee->h = h;

   if (!ee->prop.fullscreen)
     {
        int fw = 0, fh = 0;
        int maxw = 0, maxh = 0;
        int minw = 0, minh = 0;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             if (ee->prop.min.w > 0)
               minw = (ee->prop.min.w - fw);
             if (ee->prop.min.h > 0)
               minh = (ee->prop.min.h - fh);
             if (ee->prop.max.w > 0)
               maxw = (ee->prop.max.w + fw);
             if (ee->prop.max.h > 0)
               maxh = (ee->prop.max.h + fh);
          }
        else
          {
             if (ee->prop.min.w > 0)
               minw = (ee->prop.min.w - fh);
             if (ee->prop.min.h > 0)
               minh = (ee->prop.min.h - fw);
             if (ee->prop.max.w > 0)
               maxw = (ee->prop.max.w + fh);
             if (ee->prop.max.h > 0)
               maxh = (ee->prop.max.h + fw);
          }

        if ((maxw > 0) && (w > maxw))
          w = maxw;
        else if (w < minw)
          w = minw;

        if ((maxh > 0) && (h > maxh))
          h = maxh;
        else if (h < minh)
          h = minh;

        if (!ee->prop.maximized)
          {
             /* calc new size using base size & step size */
             if (ee->prop.step.w > 1)
               {
                  int bw = ee->prop.base.w ?: minw;
                  w = (bw + (((w - bw) / ee->prop.step.w) * ee->prop.step.w));
               }

             if (ee->prop.step.h > 1)
               {
                  int bh = ee->prop.base.h ?: minh;
                  h = (bh + (((h - bh) / ee->prop.step.h) * ee->prop.step.h));
               }

             if (!wdata->win->display->wl.efl_hints && EINA_DBL_NONZERO(ee->prop.aspect))
               {
                  /* copied from e_client.c */
                  Evas_Aspect_Control aspect;
                  int aw, ah;
                  double val, a = (double)w / h;

                  if (fabs(a - ee->prop.aspect) > 0.001)
                    {
                       int step_w = ee->prop.step.w ?: 1, step_h = ee->prop.step.h ?: 1;
                       if (wdata->resizing || wdata->win->resizing)
                         ew = wdata->cw, eh = wdata->ch;
                       if (abs(w - ew) > abs(h - eh))
                         aspect = EVAS_ASPECT_CONTROL_HORIZONTAL;
                       else
                         aspect = EVAS_ASPECT_CONTROL_VERTICAL;
                       switch (aspect)
                         {
                          case EVAS_ASPECT_CONTROL_HORIZONTAL:
                            val = ((h - (w / ee->prop.aspect)) * step_h) / step_h;
                            if (val > 0)
                              ah = ceil(val);
                            else
                              ah = floor(val);
                            if ((h - ah > minh) || (minh < 1))
                              {
                                 h -= ah;
                                 break;
                              }
                            EINA_FALLTHROUGH;
                            /* no break */
                          default:
                            val = (((h * ee->prop.aspect) - w) * step_w) / step_w;
                            if (val > 0)
                              aw = ceil(val);
                            else
                              aw = floor(val);
                            if ((w + aw < maxw) || (maxw < 1))
                              w += aw;
                            break;
                         }
                    }
               }
          }

        ee->w = w;
        ee->h = h;
        ee->req.w = w;
        ee->req.h = h;

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

   if (ECORE_EVAS_PORTRAIT(ee) && ((ow != w) || (oh != h)))
     diff = 1;
   if (!ECORE_EVAS_PORTRAIT(ee) && ((ow != h) || (oh != w)))
     diff = 1;

   if (diff)
     {
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

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_ecore_evas_wl_common_wm_rot_manual_rotation_done_job(void *data)
{
   Ecore_Evas *ee = (Ecore_Evas *)data;
   Ecore_Evas_Engine_Wl_Data *wdata;

   wdata = ee->engine.data;

   wdata->wm_rot.manual_mode_job = NULL;
   ee->prop.wm_rot.manual_mode.wait_for_done = EINA_FALSE;

   ecore_wl2_window_rotation_change_done_send
     (wdata->win, ee->rotation, ee->w, ee->h);

   wdata->wm_rot.done = EINA_FALSE;
}

static void
_ecore_evas_wl_common_wm_rot_manual_rotation_done(Ecore_Evas *ee)
{
   if ((ee->prop.wm_rot.supported) &&
       (ee->prop.wm_rot.app_set) &&
       (ee->prop.wm_rot.manual_mode.set))
     {
        if (ee->prop.wm_rot.manual_mode.wait_for_done)
          {
             Ecore_Evas_Engine_Wl_Data *wdata;

             wdata = ee->engine.data;

             if (ee->prop.wm_rot.manual_mode.timer)
               ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);
             ee->prop.wm_rot.manual_mode.timer = NULL;

             if (wdata->wm_rot.manual_mode_job)
               ecore_job_del(wdata->wm_rot.manual_mode_job);

             wdata->wm_rot.manual_mode_job = ecore_job_add
               (_ecore_evas_wl_common_wm_rot_manual_rotation_done_job, ee);
          }
     }
}

static Eina_Bool
_ecore_evas_wl_common_wm_rot_manual_rotation_done_timeout(void *data)
{
   Ecore_Evas *ee = data;

   ee->prop.wm_rot.manual_mode.timer = NULL;
   _ecore_evas_wl_common_wm_rot_manual_rotation_done(ee);
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_evas_wl_common_wm_rot_manual_rotation_done_timeout_update(Ecore_Evas *ee)
{
   if (ee->prop.wm_rot.manual_mode.timer)
     ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);

   ee->prop.wm_rot.manual_mode.timer = ecore_timer_add
     (4.0f, _ecore_evas_wl_common_wm_rot_manual_rotation_done_timeout, ee);
}

static Eina_Bool
_ecore_evas_wl_common_cb_window_configure(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Wl2_Event_Window_Configure *ev;
   int nw = 0, nh = 0, fw, fh, pfw, pfh;
   Eina_Bool active, prev_max, prev_full, state_change = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   wdata = ee->engine.data;
   if (!wdata) return ECORE_CALLBACK_PASS_ON;

   if ((!wdata->win->resizing) && (!wdata->resizing))
     wdata->cw = wdata->ch = 0;

   prev_max = ee->prop.maximized;
   prev_full = ee->prop.fullscreen;
   ee->prop.maximized =
     (ev->states & ECORE_WL2_WINDOW_STATE_MAXIMIZED) == ECORE_WL2_WINDOW_STATE_MAXIMIZED;
   ee->prop.fullscreen =
     (ev->states & ECORE_WL2_WINDOW_STATE_FULLSCREEN) == ECORE_WL2_WINDOW_STATE_FULLSCREEN;
   active = wdata->activated;
   wdata->activated = ecore_wl2_window_activated_get(wdata->win);

   nw = ev->w;
   nh = ev->h;

   pfw = fw = wdata->win->geometry.w - wdata->content.w;
   pfh = fh = wdata->win->geometry.h - wdata->content.h;

   if ((prev_max != ee->prop.maximized) ||
       (prev_full != ee->prop.fullscreen) ||
       (active != wdata->activated))
     {
        state_change = EINA_TRUE;
        _ecore_evas_wl_common_state_update(ee);
        fw = wdata->win->geometry.w - wdata->content.w;
        fh = wdata->win->geometry.h - wdata->content.h;
     }

   if ((!nw) && (!nh))
     {
        if (wdata->win->configure_serial && wdata->win->surface &&
            ((!state_change) || ((pfw == fw) && (pfh == fh))))
          {
             if (wdata->win->zxdg_configure_ack)
               wdata->win->zxdg_configure_ack(wdata->win->zxdg_surface,
                                              wdata->win->configure_serial);
             wdata->win->configure_serial = 0;
             wl_surface_commit(wdata->win->surface);
          }
        return ECORE_CALLBACK_RENEW;
     }

   nw -= fw;
   nh -= fh;

   if (ee->prop.fullscreen || (ee->req.w != nw) || (ee->req.h != nh))
     {
        if (wdata->win->resizing || wdata->resizing)
          {
             if ((wdata->cw != nw) || (wdata->ch != nh))
               _ecore_evas_wl_common_resize(ee, nw, nh);
             wdata->cw = nw, wdata->ch = nh;
          }
        else
          _ecore_evas_wl_common_resize(ee, nw, nh);
     }
   wdata->resizing = wdata->win->resizing;

   if (ee->prop.wm_rot.supported)
     {
        if (wdata->wm_rot.prepare)
          {
             if ((ee->prop.wm_rot.w == nw) &&
                 (ee->prop.wm_rot.h == nh))
               {
                  ee->prop.wm_rot.win_resize = EINA_FALSE;
                  wdata->wm_rot.configure_coming = EINA_FALSE;
               }
          }
        else if (wdata->wm_rot.request)
          {
             if ((wdata->wm_rot.configure_coming) &&
                 (ee->prop.wm_rot.w == nw) &&
                 (ee->prop.wm_rot.h == nh))
               {
                  wdata->wm_rot.configure_coming = EINA_FALSE;

                  if (ee->prop.wm_rot.manual_mode.set)
                    {
                       ee->prop.wm_rot.manual_mode.wait_for_done = EINA_TRUE;
                       _ecore_evas_wl_common_wm_rot_manual_rotation_done_timeout_update(ee);
                    }

                  _ecore_evas_wl_common_rotation_set(ee,
                                                     ee->prop.wm_rot.angle,
                                                     EINA_TRUE);
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_window_configure_complete(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Window_Configure_Complete *ev;
   Evas_Engine_Info_Wayland *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;

   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas);
   if (!einfo) return ECORE_CALLBACK_PASS_ON;

   einfo->info.hidden = EINA_FALSE;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("Failed to set Evas Engine Info for '%s'", ee->driver);

   return ECORE_CALLBACK_PASS_ON;
}

 static Eina_Bool
_ecore_evas_wl_common_cb_aux_hint_supported(void *data  EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Aux_Hint_Supported *ev;
   Eina_Stringshare *hint;
   Ecore_Evas_Engine_Wl_Data *wdata;

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   wdata = ee->engine.data;
   EINA_LIST_FREE(ee->prop.aux_hint.supported_list, hint) eina_stringshare_del(hint);
   ee->prop.aux_hint.supported_list = ecore_wl2_window_aux_hints_supported_get(wdata->win);
   return ECORE_CALLBACK_RENEW;
}

 static Eina_Bool
_ecore_evas_wl_common_cb_aux_hint_allowed(void *data  EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Aux_Hint_Allowed *ev;
   Eina_List *l;
   Ecore_Evas_Aux_Hint *aux;

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, l, aux)
     {
        if (aux->id == ev->id)
          {
             aux->allowed = 1;
             if (!aux->notified)
               {
                  _ecore_evas_wl_common_state_update(ee);
                  aux->notified = 1;
               }
             break;
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_window_rotate(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Window_Rotation *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_wl_common_rotation_set(ee, ev->rotation, ev->resize);
   return ECORE_CALLBACK_PASS_ON;
}

static void
_mouse_move_dispatch(Ecore_Evas *ee)
{
   Ecore_Evas_Cursor *cursor;
   Eina_Iterator *itr = eina_hash_iterator_data_new(ee->prop.cursors);

   EINA_SAFETY_ON_NULL_RETURN(itr);

   EINA_ITERATOR_FOREACH(itr, cursor)
     _ecore_evas_mouse_move_process(ee, cursor->pos_x, cursor->pos_y,
                                    ecore_loop_time_get());
   eina_iterator_free(itr);
}

static void
_rotation_do(Ecore_Evas *ee, int rotation, int resize)
{
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int rot_dif;

   wdata = ee->engine.data;

   einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.rotation = rotation;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
     }

   /* calculate difference in rotation */
   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;

   /* set ecore_wayland window rotation */
   ecore_wl2_window_rotation_set(wdata->win, rotation);

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
             if (ee->prop.fullscreen)
               {
                  /* resize the canvas based on rotation */
                  if ((rotation == 0) || (rotation == 180))
                    {
                       /* resize the canvas */
                       evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
                       evas_output_viewport_set(ee->evas, 0, 0, 
                                                ee->req.w, ee->req.h);
                    }
                  else
                    {
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
        _mouse_move_dispatch(ee);
     }
   else
     {
        /* record the current rotation of the ecore_evas */
        ee->rotation = rotation;

        /* send a mouse_move process
         *
         * NB: Is This Really Needed ? Yes, it's required to update the mouse
         * position, relatively to widgets. */
        _mouse_move_dispatch(ee);

        /* call the ecore_evas' resize function */
        if (ee->func.fn_resize) ee->func.fn_resize(ee);

        /* add canvas damage */
        if (ECORE_EVAS_PORTRAIT(ee))
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
        else
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
}

static Eina_Bool
_ecore_evas_wl_common_cb_www_drag(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Window_WWW_Drag *ev = event;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Evas *ee;

   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   wdata = ee->engine.data;
   wdata->dragging = !!ev->dragging;
   if (!ev->dragging)
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_evas_wl_common_cb_www(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Window_WWW *ev = event;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Evas *ee;

   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   wdata = ee->engine.data;
   wdata->x_rel += ev->x_rel;
   wdata->y_rel += ev->y_rel;
   wdata->timestamp = ev->timestamp;
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_evas_wl_common_cb_device_event_free(void *user_data, void *func_data)
{
   efl_unref(user_data);
   free(func_data);
}

static void
_ecore_evas_wl_common_device_event_add(int event_type, Ecore_Wl2_Device_Type device_type, unsigned int id, Evas_Device *dev, Ecore_Evas *ee)
{
   Ecore_Wl2_Event_Device *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Device));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->dev = efl_ref(dev);
   ev->type = device_type;
   ev->seat_id = id;
   ev->window_id = ee->prop.window;

   ecore_event_add(event_type, ev,
                   _ecore_evas_wl_common_cb_device_event_free, dev);
}

static EE_Wl_Device *
_ecore_evas_wl_common_seat_add(Ecore_Evas *ee, unsigned int id)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   EE_Wl_Device *device;
   Evas_Device *dev;
   char buf[32];

   device = calloc(1, sizeof(EE_Wl_Device));
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, NULL);

   snprintf(buf, sizeof(buf), "seat-%u", id);
   dev =
     evas_device_add_full(ee->evas, buf, "Wayland seat", NULL, NULL,
                          EVAS_DEVICE_CLASS_SEAT, EVAS_DEVICE_SUBCLASS_NONE);
   EINA_SAFETY_ON_NULL_GOTO(dev, err_dev);
   evas_device_seat_id_set(dev, id);
   device->seat = dev;
   device->id = id;

   wdata = ee->engine.data;
   wdata->devices_list = eina_list_append(wdata->devices_list, device);

   _ecore_evas_wl_common_device_event_add(ECORE_WL2_EVENT_DEVICE_ADDED,
                                          ECORE_WL2_DEVICE_TYPE_SEAT,
                                          id, dev, ee);
   return device;
 err_dev:
   free(device);
   return NULL;
}

static Eina_Bool
_ecore_evas_wl_common_cb_global_added(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Global *ev = event;
   Ecore_Evas *ee;
   Eina_List *l, *ll;
   EE_Wl_Device *device;

   if ((!ev->interface) || (strcmp(ev->interface, "wl_seat")))
       return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FOREACH(ee_list, l, ee)
     {
        Eina_Bool already_present = EINA_FALSE;
        Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;

        if (ev->display != wdata->display) continue;
        EINA_LIST_FOREACH(wdata->devices_list, ll, device)
          {
             if (device->id == ev->id)
               {
                  already_present = EINA_TRUE;
                  break;
               }
          }

        if (already_present)
          continue;

        if (!_ecore_evas_wl_common_seat_add(ee, ev->id))
          break;
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_ecore_evas_wl_common_device_free(EE_Wl_Device *device)
{
   if (device->seat)
     evas_device_del(device->seat);
   if (device->pointer)
     evas_device_del(device->pointer);
   if (device->keyboard)
     evas_device_del(device->keyboard);
   if (device->touch)
     evas_device_del(device->touch);
   free(device);
}

static Eina_Bool
_ecore_evas_wl_common_cb_global_removed(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Global *ev = event;
   Ecore_Evas *ee;
   Eina_List *l, *ll;

   if ((!ev->interface) || (strcmp(ev->interface, "wl_seat")))
       return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FOREACH(ee_list, l, ee)
     {
        Ecore_Evas_Engine_Wl_Data *wdata;
        EE_Wl_Device *device;
        Eina_Bool found = EINA_FALSE;

        wdata = ee->engine.data;
        if (ev->display != wdata->display) continue;

        EINA_LIST_FOREACH(wdata->devices_list, ll, device)
          {
             if (device->id == ev->id)
               {
                  found = EINA_TRUE;
                  break;
               }
          }

        if (found)
          {
             _ecore_evas_wl_common_device_event_add
               (ECORE_WL2_EVENT_DEVICE_REMOVED, ECORE_WL2_DEVICE_TYPE_SEAT,
                   ev->id, device->seat, ee);

             wdata->devices_list =
               eina_list_remove(wdata->devices_list, device);

             _ecore_evas_wl_common_device_free(device);
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_seat_name_changed(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Seat_Name *ev = event;
   Ecore_Evas *ee;
   Eina_List *l, *ll;

   EINA_LIST_FOREACH(ee_list, l, ee)
     {
        Ecore_Evas_Engine_Wl_Data *wdata;
        EE_Wl_Device *device;

        wdata = ee->engine.data;
        if (ev->display != wdata->display) continue;

        EINA_LIST_FOREACH(wdata->devices_list, ll, device)
          {
             if (device->id == ev->id)
               {
                  evas_device_name_set(device->seat, ev->name);
                  break;
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_seat_capabilities_changed(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Seat_Capabilities *ev = event;
   Ecore_Evas *ee;
   Eina_List *l, *ll;

   EINA_LIST_FOREACH(ee_list, l, ee)
     {
        Ecore_Evas_Engine_Wl_Data *wdata;
        EE_Wl_Device *device;

        wdata = ee->engine.data;
        if (ev->display != wdata->display) continue;

        EINA_LIST_FOREACH(wdata->devices_list, ll, device)
          {
             if (device->id == ev->id)
               {
                  if (ev->pointer_enabled && !device->pointer)
                    {
                       device->pointer =
                         evas_device_add_full(ee->evas, "Mouse",
                                              "A wayland pointer device",
                                              device->seat, NULL,
                                              EVAS_DEVICE_CLASS_MOUSE,
                                              EVAS_DEVICE_SUBCLASS_NONE);

                       _ecore_evas_wl_common_device_event_add
                         (ECORE_WL2_EVENT_DEVICE_ADDED,
                             ECORE_WL2_DEVICE_TYPE_POINTER,
                             ev->id, device->pointer, ee);
                    }
                  else if (!ev->pointer_enabled && device->pointer)
                    {
                       _ecore_evas_wl_common_device_event_add
                         (ECORE_WL2_EVENT_DEVICE_REMOVED,
                             ECORE_WL2_DEVICE_TYPE_POINTER, ev->id, NULL, ee);

                       evas_device_del(device->pointer);
                       device->pointer = NULL;
                    }

                  if (ev->keyboard_enabled && !device->keyboard)
                    {
                       device->keyboard =
                         evas_device_add_full(ee->evas, "Keyboard",
                                              "A wayland keyboard device",
                                              device->seat, NULL,
                                              EVAS_DEVICE_CLASS_KEYBOARD,
                                              EVAS_DEVICE_SUBCLASS_NONE);

                       _ecore_evas_wl_common_device_event_add
                         (ECORE_WL2_EVENT_DEVICE_ADDED,
                             ECORE_WL2_DEVICE_TYPE_KEYBOARD,
                             ev->id, device->keyboard, ee);
                    }
                  else if (!ev->keyboard_enabled && device->keyboard)
                    {
                       _ecore_evas_wl_common_device_event_add
                         (ECORE_WL2_EVENT_DEVICE_REMOVED,
                             ECORE_WL2_DEVICE_TYPE_KEYBOARD, ev->id, NULL, ee);

                       evas_device_del(device->keyboard);
                       device->keyboard = NULL;
                    }

                  if (ev->touch_enabled && !device->touch)
                    {
                       device->touch =
                         evas_device_add_full(ee->evas, "Touch",
                                              "A wayland touch device",
                                              device->seat, NULL,
                                              EVAS_DEVICE_CLASS_TOUCH,
                                              EVAS_DEVICE_SUBCLASS_NONE);

                       _ecore_evas_wl_common_device_event_add
                         (ECORE_WL2_EVENT_DEVICE_ADDED,
                             ECORE_WL2_DEVICE_TYPE_TOUCH,
                             ev->id, device->touch, ee);
                    }
                  else if (!ev->touch_enabled && device->touch)
                    {
                       _ecore_evas_wl_common_device_event_add
                         (ECORE_WL2_EVENT_DEVICE_REMOVED,
                             ECORE_WL2_DEVICE_TYPE_TOUCH,
                             ev->id, NULL, ee);

                       evas_device_del(device->touch);
                       device->touch = NULL;
                    }

                  break;
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_iconify_state_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl2_Event_Window_Iconify_State_Change *ev;

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (!ev->force) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   if (ee->prop.iconified == ev->iconified)
     return ECORE_CALLBACK_PASS_ON;

   ee->prop.iconified = ev->iconified;
   _ecore_evas_wl_common_state_update(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static int
_ecore_evas_wl_common_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_ecore_evas_wl_init_count != 1)
     return _ecore_evas_wl_init_count;

   _ecore_evas_wl_event_hdls[0] =
     ecore_event_handler_add(ECORE_EVENT_MOUSE_IN,
                             _ecore_evas_wl_common_cb_mouse_in, NULL);
   _ecore_evas_wl_event_hdls[1] =
     ecore_event_handler_add(ECORE_EVENT_MOUSE_OUT,
                             _ecore_evas_wl_common_cb_mouse_out, NULL);
   _ecore_evas_wl_event_hdls[2] =
     ecore_event_handler_add(ECORE_WL2_EVENT_FOCUS_IN,
                             _ecore_evas_wl_common_cb_focus_in, NULL);
   _ecore_evas_wl_event_hdls[3] =
     ecore_event_handler_add(ECORE_WL2_EVENT_FOCUS_OUT,
                             _ecore_evas_wl_common_cb_focus_out, NULL);
   _ecore_evas_wl_event_hdls[4] =
     ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE,
                             _ecore_evas_wl_common_cb_window_configure, NULL);
   _ecore_evas_wl_event_hdls[5] =
     ecore_event_handler_add(_ecore_wl2_event_window_www,
                             _ecore_evas_wl_common_cb_www, NULL);
   _ecore_evas_wl_event_hdls[6] =
     ecore_event_handler_add(_ecore_wl2_event_window_www_drag,
                             _ecore_evas_wl_common_cb_www_drag, NULL);
   _ecore_evas_wl_event_hdls[7] =
     ecore_event_handler_add(ECORE_WL2_EVENT_DISCONNECT,
                             _ecore_evas_wl_common_cb_disconnect, NULL);
   _ecore_evas_wl_event_hdls[8] =
     ecore_event_handler_add(ECORE_WL2_EVENT_GLOBAL_ADDED,
                             _ecore_evas_wl_common_cb_global_added, NULL);
   _ecore_evas_wl_event_hdls[9] =
     ecore_event_handler_add(ECORE_WL2_EVENT_GLOBAL_REMOVED,
                             _ecore_evas_wl_common_cb_global_removed, NULL);
   _ecore_evas_wl_event_hdls[10] =
     ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_NAME_CHANGED,
                             _ecore_evas_wl_common_cb_seat_name_changed, NULL);
   _ecore_evas_wl_event_hdls[11] =
     ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED,
                             _ecore_evas_wl_common_cb_seat_capabilities_changed,
                             NULL);
   _ecore_evas_wl_event_hdls[12] =
     ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE,
                             _ecore_evas_wl_common_cb_window_configure_complete,
                             NULL);
   _ecore_evas_wl_event_hdls[13] =
     ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_ROTATE,
                             _ecore_evas_wl_common_cb_window_rotate, NULL);
   _ecore_evas_wl_event_hdls[14] =
     ecore_event_handler_add(ECORE_WL2_EVENT_AUX_HINT_ALLOWED,
                             _ecore_evas_wl_common_cb_aux_hint_allowed, NULL);
   _ecore_evas_wl_event_hdls[15] =
     ecore_event_handler_add(ECORE_WL2_EVENT_AUX_HINT_SUPPORTED,
                             _ecore_evas_wl_common_cb_aux_hint_supported, NULL);
   _ecore_evas_wl_event_hdls[16] =
     ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_ICONIFY_STATE_CHANGE,
                             _ecore_evas_wl_common_cb_iconify_state_change, NULL);

   ecore_event_evas_init();

   return _ecore_evas_wl_init_count;
}

static int
_ecore_evas_wl_common_shutdown(void)
{
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (--_ecore_evas_wl_init_count != 0)
     return _ecore_evas_wl_init_count;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_ecore_evas_wl_event_hdls); i++)
     {
        if (_ecore_evas_wl_event_hdls[i])
          ecore_event_handler_del(_ecore_evas_wl_event_hdls[i]);
     }

   ecore_event_evas_shutdown();

   return _ecore_evas_wl_init_count;
}

static void
_ecore_evas_wl_common_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   EE_Wl_Device *device;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;

   wdata = ee->engine.data;
   ee_list = eina_list_remove(ee_list, ee);

   eina_list_free(wdata->regen_objs);
   if (wdata->anim_callback) wl_callback_destroy(wdata->anim_callback);
   ecore_event_handler_del(wdata->sync_handler);
   if (wdata->win) ecore_wl2_window_free(wdata->win);
   ecore_wl2_display_disconnect(wdata->display);

   EINA_LIST_FREE(wdata->devices_list, device)
      free(device);

   free(wdata);

   ecore_event_window_unregister(ee->prop.window);
   ecore_evas_input_event_unregister(ee);

   _ecore_evas_wl_common_shutdown();

   ecore_wl2_shutdown();
}

static void
_ecore_evas_wl_common_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->w != w) || (ee->h != h))
     _ecore_evas_wl_common_resize(ee, w, h);
}

static void
_ecore_evas_wl_common_callback_resize_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_resize = func;
}

static void
_ecore_evas_wl_common_callback_move_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_move = func;
}

static void
_ecore_evas_wl_common_callback_delete_request_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_delete_request = func;
}

static void
_ecore_evas_wl_common_callback_focus_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_focus_in = func;
}

static void
_ecore_evas_wl_common_callback_focus_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_focus_out = func;
}

static void
_ecore_evas_wl_common_callback_mouse_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_mouse_in = func;
}

static void
_ecore_evas_wl_common_callback_mouse_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_mouse_out = func;
}

static void
_ecore_evas_wl_common_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Wl2_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wdata = ee->engine.data;
   input = ecore_wl2_display_input_find_by_name(ecore_wl2_window_display_get(wdata->win), "default");
   if (input) ecore_wl2_input_pointer_xy_get(input, x, y);
}

static void
_ecore_evas_wl_common_wm_rot_preferred_rotation_set(Ecore_Evas *ee, int rot)
{
   if (ee->prop.wm_rot.supported)
     {
        Ecore_Evas_Engine_Wl_Data *wdata;

        wdata = ee->engine.data;
        if (!ee->prop.wm_rot.app_set)
          {
             ecore_wl2_window_rotation_app_set(wdata->win, EINA_TRUE);
             ee->prop.wm_rot.app_set = EINA_TRUE;
          }

        ecore_wl2_window_preferred_rotation_set(wdata->win, rot);
        ee->prop.wm_rot.preferred_rot = rot;
     }
}

static void
_ecore_evas_wl_common_wm_rot_available_rotations_set(Ecore_Evas *ee, const int *rots, unsigned int count)
{
   if (ee->prop.wm_rot.supported)
     {
        Ecore_Evas_Engine_Wl_Data *wdata;

        wdata = ee->engine.data;
        if (!ee->prop.wm_rot.app_set)
          {
             ecore_wl2_window_rotation_app_set(wdata->win, EINA_TRUE);
             ee->prop.wm_rot.app_set = EINA_TRUE;
          }

        if (ee->prop.wm_rot.available_rots)
          {
             free(ee->prop.wm_rot.available_rots);
             ee->prop.wm_rot.available_rots = NULL;
          }

        ee->prop.wm_rot.count = 0;

        if (count > 0)
          {
             ee->prop.wm_rot.available_rots = calloc(count, sizeof(int));
             if (!ee->prop.wm_rot.available_rots) return;

             memcpy(ee->prop.wm_rot.available_rots, rots, sizeof(int) * count);
          }

        ee->prop.wm_rot.count = count;

        ecore_wl2_window_available_rotations_set(wdata->win, rots, count);
     }
}

static void
_ecore_evas_wl_common_wm_rot_manual_rotation_done_set(Ecore_Evas *ee, Eina_Bool set)
{
   ee->prop.wm_rot.manual_mode.set = set;
}

static void
_ecore_evas_wl_common_pointer_device_xy_get(const Ecore_Evas *ee, const Efl_Input_Device *pointer, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Wl2_Input *input;
   const Eo *seat;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wdata = ee->engine.data;
   seat = evas_device_parent_get(pointer);
   EINA_SAFETY_ON_NULL_RETURN(seat);
   input = ecore_wl2_display_input_find(ecore_wl2_window_display_get(wdata->win), evas_device_seat_id_get(seat));
   EINA_SAFETY_ON_NULL_RETURN(input);
   ecore_wl2_input_pointer_xy_get(input, x, y);
}

 void
_ecore_evas_wl_common_aux_hints_supported_update(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   ee->prop.aux_hint.supported_list = ecore_wl2_window_aux_hints_supported_get(wdata->win);
}

static void
_ecore_evas_wl_common_raise(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   ecore_wl2_window_raise(wdata->win);
}

static void
_ecore_evas_wl_common_title_set(Ecore_Evas *ee, const char *title)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (eina_streq(ee->prop.title, title)) return;
   free(ee->prop.title);
   ee->prop.title = eina_strdup(title);

   if (ee->prop.title)
     {
        wdata = ee->engine.data;
        ecore_wl2_window_title_set(wdata->win, ee->prop.title);
     }
}

static void
_ecore_evas_wl_common_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (!eina_streq(ee->prop.name, n))
     {
        if (ee->prop.name) free(ee->prop.name);
        ee->prop.name = NULL;
        if (n) ee->prop.name = strdup(n);
     }
   if (!eina_streq(ee->prop.clas, c))
     {
        if (ee->prop.clas) free(ee->prop.clas);
        ee->prop.clas = NULL;
        if (c) ee->prop.clas = strdup(c);
     }

   if (ee->prop.clas)
     ecore_wl2_window_class_set(wdata->win, ee->prop.clas);
}

static void
_ecore_evas_wl_common_size_min_set(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;

   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
   wdata = ee->engine.data;
   if (wdata->win->zxdg_set_min_size && wdata->win->zxdg_toplevel)
     wdata->win->zxdg_set_min_size(wdata->win->zxdg_toplevel, w, h);
   _ecore_evas_wl_common_resize(ee, ee->w, ee->h);
}

static void
_ecore_evas_wl_common_size_max_set(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
   wdata = ee->engine.data;
   if (wdata->win->zxdg_set_max_size && wdata->win->zxdg_toplevel)
     wdata->win->zxdg_set_max_size(wdata->win->zxdg_toplevel, w, h);
   _ecore_evas_wl_common_resize(ee, ee->w, ee->h);
}

static void
_ecore_evas_wl_common_size_base_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
   _ecore_evas_wl_common_resize(ee, ee->w, ee->h);
}

static void
_ecore_evas_wl_common_size_step_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
   _ecore_evas_wl_common_resize(ee, ee->w, ee->h);
}

static void
_ecore_evas_wl_common_aspect_set(Ecore_Evas *ee, double aspect)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (EINA_FLT_EQ(ee->prop.aspect, aspect)) return;
   ee->prop.aspect = aspect;
   _ecore_evas_wl_common_resize(ee, ee->w, ee->h);
}

static void
_ecore_evas_wl_common_focus_skip_set(Ecore_Evas *ee, Eina_Bool skip)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   wdata = ee->engine.data;
   if (!wdata) return;
   if (ee->prop.focus_skip == skip) return;
   ee->prop.focus_skip = skip;
   ecore_wl2_window_focus_skip_set(wdata->win, skip);
}

static void
_ecore_evas_wl_common_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer EINA_UNUSED, int hot_x, int hot_y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Wl2_Input *input;

   wdata = ee->engine.data;
   if (obj == _ecore_evas_default_cursor_image_get(ee)) return;
   input = ecore_wl2_display_input_find_by_name(ecore_wl2_window_display_get(wdata->win), "default");
   if (input) ecore_wl2_input_pointer_set(input, NULL, hot_x, hot_y);
}

static void
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

static void
_ecore_evas_wl_common_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->prop.iconified = on;

   wdata = ee->engine.data;
   ecore_wl2_window_iconified_set(wdata->win, on);
}

static void
_ecore_evas_wl_common_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.borderless == on) return;
   ee->prop.borderless = on;

   _ecore_evas_wl_common_state_update(ee);
}

static void
_ecore_evas_wl_common_maximized_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.maximized == on) return;

   wdata = ee->engine.data;
   ecore_wl2_window_maximized_set(wdata->win, on);
}

static void
_ecore_evas_wl_common_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.fullscreen == on) return;

   wdata = ee->engine.data;
   ecore_wl2_window_fullscreen_set(wdata->win, on);
}

static void
_ecore_evas_wl_common_ignore_events_set(Ecore_Evas *ee, int ignore)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->ignore_events = ignore;
   /* NB: Hmmm, may need to pass this to ecore_wl_window in the future */
}

static void
_anim_cb_animate(void *data, struct wl_callback *callback, uint32_t serial EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Wl_Data *wdata;

   wdata = ee->engine.data;
   wl_callback_destroy(callback);
   wdata->anim_callback = NULL;
   ecore_evas_manual_render_set(ee, 0);
}

static const struct wl_callback_listener _anim_listener =
{
   _anim_cb_animate
};

static void
_ecore_evas_wl_common_render_flush_pre(void *data, Evas *evas, void *event EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   struct wl_surface *surf;
   int fx, fy;

   einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(evas);
   if (!einfo) return;

   surf = einfo->info.wl_surface;
   if (!surf) return;

   wdata = ee->engine.data;
   if (!wdata) return;

   if (wdata->win->pending.configure) return;

   wdata->anim_callback = wl_surface_frame(surf);
   wl_callback_add_listener(wdata->anim_callback, &_anim_listener, ee);
   ecore_evas_manual_render_set(ee, 1);

   if (!ecore_wl2_window_shell_surface_exists(wdata->win)) return;

   if (wdata->win->zxdg_configure_ack && wdata->win->configure_serial)
     wdata->win->zxdg_configure_ack(wdata->win->zxdg_surface,
                                    wdata->win->configure_serial);
   wdata->win->configure_serial = 0;

   /* Surviving bits of WWW - track interesting state we might want
    * to pass to clients to do client side effects
    */
   einfo->window.x = wdata->win->geometry.x;
   einfo->window.y = wdata->win->geometry.y;
   einfo->window.w = wdata->win->geometry.w;
   einfo->window.h = wdata->win->geometry.h;
   if (einfo->resizing)
     {
        einfo->x_rel = 0;
        einfo->y_rel = 0;
     }
   else
     {
        einfo->x_rel = wdata->x_rel;
        einfo->y_rel = wdata->y_rel;
     }
   einfo->timestamp = wdata->timestamp;
   evas_canvas_pointer_canvas_xy_get(evas, &einfo->x_cursor, &einfo->y_cursor);
   evas_output_framespace_get(evas, &fx, &fy, NULL, NULL);
   einfo->x_cursor -= fx;
   einfo->y_cursor -= fy;
   wdata->x_rel = wdata->y_rel = 0;
   einfo->resizing = wdata->win->resizing;
   einfo->dragging = wdata->dragging;
   einfo->drag_start = EINA_FALSE;
   einfo->drag_stop = EINA_FALSE;
   if (einfo->drag_ack && !einfo->dragging) einfo->drag_stop = EINA_TRUE;
   if (einfo->dragging && !einfo->drag_ack) einfo->drag_start = EINA_TRUE;
   einfo->drag_ack = wdata->dragging;
}

static void
_ecore_evas_wayland_alpha_do(Ecore_Evas *ee, int alpha)
{
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->alpha == alpha) return;
   ee->alpha = alpha;
   wdata = ee->engine.data;
   if (!wdata->sync_done) return;

   if (wdata->win) ecore_wl2_window_alpha_set(wdata->win, ee->alpha);

   _ecore_evas_wl_common_wm_rotation_protocol_set(ee);

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   if ((einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = EINA_TRUE;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
     }
}

static void
_ecore_evas_wayland_transparent_do(Ecore_Evas *ee, int transparent)
{
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->transparent == transparent) return;
   ee->transparent = transparent;

   wdata = ee->engine.data;
   if (!wdata->sync_done) return;

   if (wdata->win)
     ecore_wl2_window_transparent_set(wdata->win, ee->transparent);

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   if ((einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = EINA_TRUE;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
     }
}

static void
_ecore_evas_wl_common_render_updates(void *data, Evas *evas EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Evas *ee = data;

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

static Eina_Bool
_ecore_evas_wl_common_prepare(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!(wdata = ee->engine.data)) return EINA_FALSE;
   if (!wdata->sync_done) return EINA_FALSE;

   if (wdata->win->pending.configure) return EINA_FALSE;

   return EINA_TRUE;
}

static void
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

static void
_ecore_evas_wl_common_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;

   wdata = ee->engine.data;
   ecore_wl2_display_screen_size_get(wdata->display, w, h);
}

static void
_ecore_evas_wl_common_screen_dpi_get(const Ecore_Evas *ee EINA_UNUSED, int *xdpi, int *ydpi)
{
   int dpi = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (xdpi) *xdpi = 0;
   if (ydpi) *ydpi = 0;

   /* FIXME: Ideally this needs to get the DPI from a specific screen */

   /* TODO */
   /* dpi = ecore_wl_dpi_get(); */
   if (xdpi) *xdpi = dpi;
   if (ydpi) *ydpi = dpi;
}

static void
_ecore_evas_wayland_resize_edge_set(Ecore_Evas *ee, int edge)
{
   Evas_Engine_Info_Wayland *einfo;

   if ((einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas)))
     einfo->info.edges = edge;
}

static void
_ecore_evas_wayland_resize(Ecore_Evas *ee, int location)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (wdata->win)
     {
        _ecore_evas_wayland_resize_edge_set(ee, location);
        ecore_wl2_window_resize(wdata->win, NULL, location);
     }
}

static void
_ecore_evas_wayland_move(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (wdata->win) ecore_wl2_window_move(wdata->win, NULL);
}

static void
_ecore_evas_wayland_type_set(Ecore_Evas *ee, int type)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   wdata = ee->engine.data;

   ecore_wl2_window_type_set(wdata->win, type);
}

static Ecore_Wl2_Window *
_ecore_evas_wayland_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!(!strncmp(ee->driver, "wayland", 7)))
     return NULL;

   wdata = ee->engine.data;
   return wdata->win;
}

/* static void */
/* _ecore_evas_wayland_pointer_set(Ecore_Evas *ee EINA_UNUSED, int hot_x EINA_UNUSED, int hot_y EINA_UNUSED) */
/* { */

/* } */

static void
_ecore_evas_wayland_aux_hint_add(Ecore_Evas *ee, int id, const char *hint, const char *val)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   wdata = ee->engine.data;
   ecore_wl2_window_aux_hint_add(wdata->win, id, hint, val);
}

static void
_ecore_evas_wayland_aux_hint_change(Ecore_Evas *ee, int id, const char *val)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   wdata = ee->engine.data;
   ecore_wl2_window_aux_hint_change(wdata->win, id, val);
}

static void
_ecore_evas_wayland_aux_hint_del(Ecore_Evas *ee, int id)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   wdata = ee->engine.data;
   ecore_wl2_window_aux_hint_del(wdata->win, id);
}

static Ecore_Evas_Interface_Wayland *
_ecore_evas_wl_interface_new(void)
{
   Ecore_Evas_Interface_Wayland *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Wayland));
   if (!iface) return NULL;

   iface->base.name = interface_wl_name;
   iface->base.version = interface_wl_version;

   iface->resize = _ecore_evas_wayland_resize;
   iface->move = _ecore_evas_wayland_move;
   /* iface->pointer_set = _ecore_evas_wayland_pointer_set; */
   iface->type_set = _ecore_evas_wayland_type_set;
   iface->window2_get = _ecore_evas_wayland_window_get;
   iface->aux_hint_add = _ecore_evas_wayland_aux_hint_add;
   iface->aux_hint_change = _ecore_evas_wayland_aux_hint_change;
   iface->aux_hint_del = _ecore_evas_wayland_aux_hint_del;

   return iface;
}

static void
_ecore_evas_wl_common_show(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (ee->visible)) return;

   wdata = ee->engine.data;
   if (!wdata->sync_done)
     {
        wdata->defer_show = EINA_TRUE;
        return;
     }

   if (wdata->win)
     {
        int fw, fh;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

        ecore_wl2_window_geometry_set(wdata->win, 0, 0, ee->w, ee->h);
        ecore_wl2_window_show(wdata->win);
        ecore_wl2_window_alpha_set(wdata->win, ee->alpha);

        einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             einfo->info.wl_surface = ecore_wl2_window_surface_get(wdata->win);
             einfo->info.hidden = wdata->win->pending.configure; //EINA_FALSE;
             einfo->www_avail = !!wdata->win->www_surface;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               ERR("Failed to set Evas Engine Info for '%s'", ee->driver);
             if (ECORE_EVAS_PORTRAIT(ee))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->h + fh, ee->w + fw);
          }
     }

   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);

   if (ee->visible) return;
   ee->visible = 1;
   ee->should_be_visible = 1;
   ee->draw_ok = EINA_TRUE;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void
_ecore_evas_wl_common_hide(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   wdata = ee->engine.data;

   evas_sync(ee->evas);

   einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.hidden = EINA_TRUE;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("Failed to set Evas Engine Info for '%s'", ee->driver);
          }
     }

   if (wdata->win)
     ecore_wl2_window_hide(wdata->win);

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_TRUE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   if (!ee->visible) return;
   ee->visible = 0;
   ee->should_be_visible = 0;
   ee->draw_ok = EINA_FALSE;
   if (wdata->anim_callback) wl_callback_destroy(wdata->anim_callback);
   wdata->anim_callback = NULL;
   ecore_evas_manual_render_set(ee, 0);

   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void
_ecore_evas_wl_common_alpha_set(Ecore_Evas *ee, int alpha)
{
   if (ee->in_async_render)
     {
        ee->delayed.alpha = alpha;
        ee->delayed.alpha_changed = EINA_TRUE;
        return;
     }

   _ecore_evas_wayland_alpha_do(ee, alpha);
}

static void
_ecore_evas_wl_common_transparent_set(Ecore_Evas *ee, int transparent)
{
   if (ee->in_async_render)
     {
        ee->delayed.transparent = transparent;
        ee->delayed.transparent_changed = EINA_TRUE;
        return;
     }

   _ecore_evas_wayland_transparent_do(ee, transparent);
}

static void
_ecore_evas_wl_common_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->rotation == rotation) return;

   if (ee->in_async_render)
     {
        ee->delayed.rotation = rotation;
        ee->delayed.rotation_resize = resize;
        ee->delayed.rotation_changed = EINA_TRUE;
     }
   else
     _rotation_do(ee, rotation, resize);
}

static Eina_Bool
_ee_cb_sync_done(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Evas *ee;
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;

   ee = data;
   wdata = ee->engine.data;
   if (wdata->sync_done) return ECORE_CALLBACK_PASS_ON;
   wdata->sync_done = EINA_TRUE;

   if ((einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.wl_display = ecore_wl2_display_get(wdata->display);
        einfo->info.wl_dmabuf = ecore_wl2_display_dmabuf_get(wdata->display);
        einfo->info.wl_shm = ecore_wl2_display_shm_get(wdata->display);
        einfo->info.compositor_version =
          ecore_wl2_display_compositor_version_get(wdata->display);
        einfo->info.destination_alpha = EINA_TRUE;
        einfo->info.rotation = ee->rotation;
        einfo->info.wl_surface = ecore_wl2_window_surface_get(wdata->win);

        if (wdata->reset_pending)
          {
             ecore_evas_manual_render_set(ee, 0);
          }
        if (evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             if (wdata->reset_pending && !strcmp(ee->driver, "wayland_egl"))
               _evas_canvas_image_data_regenerate(wdata->regen_objs);
             wdata->regen_objs = NULL;
          }
        else
          ERR("Failed to set Evas Engine Info for '%s'", ee->driver);
        wdata->reset_pending = 0;
     }
   else
     {
        ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
     }

   if (wdata->defer_show)
     {
        wdata->defer_show = EINA_FALSE;
        _ecore_evas_wl_common_show(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_wl2_devices_setup(Ecore_Evas *ee, Ecore_Wl2_Display *display)
{
   Eina_Bool r = EINA_TRUE;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr = ecore_wl2_display_inputs_get(display);

   EINA_SAFETY_ON_NULL_RETURN_VAL(itr, EINA_FALSE);
   EINA_ITERATOR_FOREACH(itr, input)
     {
        EE_Wl_Device *device;
        Ecore_Wl2_Seat_Capabilities cap;
        unsigned int id;

        id = ecore_wl2_input_seat_id_get(input);
        cap = ecore_wl2_input_seat_capabilities_get(input);
        device = _ecore_evas_wl_common_seat_add(ee, id);

        if (!device)
          {
             r = EINA_FALSE;
             break;
          }

        if (cap & ECORE_WL2_SEAT_CAPABILITIES_KEYBOARD)
          {
             device->keyboard =
               evas_device_add_full(ee->evas, "Keyboard",
                                    "A wayland keyboard device",
                                    device->seat, NULL,
                                    EVAS_DEVICE_CLASS_KEYBOARD,
                                    EVAS_DEVICE_SUBCLASS_NONE);

             _ecore_evas_wl_common_device_event_add
               (ECORE_WL2_EVENT_DEVICE_ADDED, ECORE_WL2_DEVICE_TYPE_KEYBOARD,
                   id, device->keyboard, ee);
          }
        if (cap & ECORE_WL2_SEAT_CAPABILITIES_POINTER)
          {
             device->pointer =
               evas_device_add_full(ee->evas, "Mouse",
                                    "A wayland pointer device",
                                    device->seat, NULL,
                                    EVAS_DEVICE_CLASS_MOUSE,
                                    EVAS_DEVICE_SUBCLASS_NONE);

             _ecore_evas_wl_common_device_event_add
               (ECORE_WL2_EVENT_DEVICE_ADDED, ECORE_WL2_DEVICE_TYPE_POINTER,
                   id, device->pointer, ee);
          }
        if (cap & ECORE_WL2_SEAT_CAPABILITIES_TOUCH)
          {
             device->touch =
               evas_device_add_full(ee->evas, "Touch",
                                    "A wayland touch device",
                                    device->seat, NULL,
                                    EVAS_DEVICE_CLASS_TOUCH,
                                    EVAS_DEVICE_SUBCLASS_NONE);

             _ecore_evas_wl_common_device_event_add
               (ECORE_WL2_EVENT_DEVICE_ADDED, ECORE_WL2_DEVICE_TYPE_TOUCH,
                   id, device->touch, ee);
          }
     }
   eina_iterator_free(itr);
   return r;
}

static Ecore_Evas_Engine_Func _ecore_wl_engine_func =
{
   _ecore_evas_wl_common_free,
   _ecore_evas_wl_common_callback_resize_set,
   _ecore_evas_wl_common_callback_move_set,
   NULL,
   NULL,
   _ecore_evas_wl_common_callback_delete_request_set,
   NULL,
   _ecore_evas_wl_common_callback_focus_in_set,
   _ecore_evas_wl_common_callback_focus_out_set,
   _ecore_evas_wl_common_callback_mouse_in_set,
   _ecore_evas_wl_common_callback_mouse_out_set,
   NULL, // sticky_set
   NULL, // unsticky_set
   NULL, // pre_render_set
   NULL, // post_render_set
   NULL,
   NULL, // managed_move
   _ecore_evas_wl_common_resize,
   _ecore_evas_wl_common_move_resize,
   _ecore_evas_wl_common_rotation_set,
   NULL, // shaped_set
   _ecore_evas_wl_common_show,
   _ecore_evas_wl_common_hide,
   _ecore_evas_wl_common_raise,
   NULL, // lower
   NULL, // activate
   _ecore_evas_wl_common_title_set,
   _ecore_evas_wl_common_name_class_set,
   _ecore_evas_wl_common_size_min_set,
   _ecore_evas_wl_common_size_max_set,
   _ecore_evas_wl_common_size_base_set,
   _ecore_evas_wl_common_size_step_set,
   _ecore_evas_wl_common_object_cursor_set,
   NULL,
   _ecore_evas_wl_common_layer_set,
   NULL, // focus set
   _ecore_evas_wl_common_iconified_set,
   _ecore_evas_wl_common_borderless_set,
   NULL, // override set
   _ecore_evas_wl_common_maximized_set,
   _ecore_evas_wl_common_fullscreen_set,
   NULL, // func avoid_damage set
   _ecore_evas_wl_common_withdrawn_set,
   NULL, // func sticky set
   _ecore_evas_wl_common_ignore_events_set,
   _ecore_evas_wl_common_alpha_set,
   _ecore_evas_wl_common_transparent_set,
   NULL, // func profiles set
   NULL, // func profile set
   NULL, // window group set
   _ecore_evas_wl_common_aspect_set,
   NULL, // urgent set
   NULL, // modal set
   NULL, // demand attention set
   _ecore_evas_wl_common_focus_skip_set,
   NULL, //_ecore_evas_wl_common_render,
   _ecore_evas_wl_common_screen_geometry_get,
   _ecore_evas_wl_common_screen_dpi_get,
   NULL, // func msg parent send
   NULL, // func msg send

   _ecore_evas_wl_common_pointer_xy_get,
   NULL, // pointer_warp

   _ecore_evas_wl_common_wm_rot_preferred_rotation_set,
   _ecore_evas_wl_common_wm_rot_available_rotations_set,
   _ecore_evas_wl_common_wm_rot_manual_rotation_done_set,
   _ecore_evas_wl_common_wm_rot_manual_rotation_done,

   NULL, // aux_hints_set

   NULL, // fn_animator_register
   NULL, // fn_animator_unregister

   NULL, // fn_evas_changed
   NULL, //fn_focus_device_set
   NULL, //fn_callback_focus_device_in_set
   NULL, //fn_callback_focus_device_out_set
   NULL, //fn_callback_device_mouse_in_set
   NULL, //fn_callback_device_mouse_out_set
   _ecore_evas_wl_common_pointer_device_xy_get,
   _ecore_evas_wl_common_prepare,
   NULL, //fn_last_tick_get
};

Ecore_Evas *
_ecore_evas_wl_common_new_internal(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame, const char *engine_name)
{
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Window *p = NULL;
   Evas_Engine_Info_Wayland *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Evas_Interface_Wayland *iface;
   Ecore_Evas *ee = NULL;
   int method = 0;
   int fw = 0, fh = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(method = evas_render_method_lookup(engine_name)))
     {
        ERR("Render method lookup failed for %s", engine_name);
        return NULL;
     }

   if (!ecore_wl2_init())
     {
        ERR("Failed to initialize Ecore_Wl2");
        return NULL;
     }

   ewd = ecore_wl2_display_connect(disp_name);
   if (!ewd)
     {
        ERR("Failed to connect to Wayland Display %s", disp_name);
        goto conn_err;
     }

   if (!(ee = calloc(1, sizeof(Ecore_Evas))))
     {
        ERR("Failed to allocate Ecore_Evas");
        goto ee_err;
     }

   if (!(wdata = calloc(1, sizeof(Ecore_Evas_Engine_Wl_Data))))
     {
        ERR("Failed to allocate Ecore_Evas_Engine_Wl_Data");
        goto w_err;
     }

   if (frame) WRN("draw_frame is now deprecated and will have no effect");

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_wl_common_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_wl_engine_func;
   ee->engine.data = wdata;

   iface = _ecore_evas_wl_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->driver = engine_name;
   if (disp_name) ee->name = strdup(disp_name);

   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->rotation = 0;
   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;
   ee->prop.withdrawn = EINA_TRUE;
   ee->alpha = EINA_FALSE;

   /* Wayland egl engine can't async render */
   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER") || !strcmp(engine_name, "wayland_egl"))
     ee->can_async_render = 0;
   else
     ee->can_async_render = 1;

   if (parent)
     {
        p = ecore_wl2_display_window_find(ewd, parent);
        ee->alpha = ecore_wl2_window_alpha_get(p);
     }

   wdata->sync_done = EINA_FALSE;
   wdata->parent = p;
   wdata->display = ewd;

   wdata->win = ecore_wl2_window_new(ewd, p, x, y, w + fw, h + fh);
   ee->prop.window = ecore_wl2_window_id_get(wdata->win);

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, ee->w + fw, ee->h + fh);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w + fw, ee->h + fh);

   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
                           _ecore_evas_wl_common_render_updates, ee);

   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
                           _ecore_evas_wl_common_render_flush_pre, ee);

   if (ewd->sync_done)
     {
        wdata->sync_done = EINA_TRUE;
        if ((einfo = (Evas_Engine_Info_Wayland *)evas_engine_info_get(ee->evas)))
          {
             einfo->info.wl_display = ecore_wl2_display_get(ewd);
             einfo->info.destination_alpha = EINA_TRUE;
             einfo->info.rotation = ee->rotation;
             einfo->info.depth = 32;
             einfo->info.wl_surface = ecore_wl2_window_surface_get(wdata->win);
             einfo->info.wl_dmabuf = ecore_wl2_display_dmabuf_get(ewd);
             einfo->info.wl_shm = ecore_wl2_display_shm_get(ewd);
             einfo->info.compositor_version =
               ecore_wl2_display_compositor_version_get(ewd);
             einfo->info.hidden = EINA_TRUE;

             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("Failed to set Evas Engine Info for '%s'", ee->driver);
                  goto eng_err;
               }
          }
        else
          {
             ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
             goto eng_err;
          }
     }

   if (!_ecore_wl2_devices_setup(ee, ewd))
     {
        ERR("Failed to create the devices");
        goto eng_err;
     }

   _ecore_evas_wl_common_wm_rotation_protocol_set(ee);

   _ecore_evas_register(ee);
   ecore_evas_input_event_register(ee);

   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
   _ecore_event_window_direct_cb_set(ee->prop.window,
                                     _ecore_evas_input_direct_cb);

   wdata->sync_handler =
     ecore_event_handler_add(ECORE_WL2_EVENT_SYNC_DONE, _ee_cb_sync_done, ee);

   ee_list = eina_list_append(ee_list, ee);

   return ee;

eng_err:
   /* ecore_evas_free() will call ecore_wl2_display_disconnect()
    * and free(ee) */
   ecore_evas_free(ee);
   ee = NULL;
w_err:
   free(ee);
ee_err:
   if (ee) ecore_wl2_display_disconnect(ewd);
conn_err:
   ecore_wl2_shutdown();
   return NULL;
}
