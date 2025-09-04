#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Ecore_Evas.h>
#include "ecore_evas_private.h"
#include "ecore_evas_drm.h"
#include <Ecore_Drm2.h>
#include <Evas_Engine_Drm.h>
#include <drm_fourcc.h>

#ifdef _WIN32
# ifndef EFL_MODULE_STATIC
#  define EMODAPI __declspec(dllexport)
# else
#  define EMODAPI
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EMODAPI __attribute__ ((visibility("default")))
#  endif
# endif
#endif /* ! _WIN32 */

#ifndef EMODAPI
# define EMODAPI
#endif

typedef struct _Ecore_Evas_Engine_Drm_Data
{
   int x, y, w, h;
   int depth, bpp, rotation;
   unsigned int format;

   double tick_offset;
   double tick_timestamp;
   Ecore_Job *tick_job;

   Ecore_Job *focus_job;

   Ecore_Drm2_Device *dev;
   Ecore_Drm2_Display *disp;
   Ecore_Drm2_Event_Context ctx;

   Ecore_Fd_Handler *fd_hdlr;

   Eina_Bool ticking : 1;
   Eina_Bool blanktime : 1;
} Ecore_Evas_Engine_Drm_Data;

static int _drm_init_count = 0;

static Ecore_Evas_Interface_Drm *
_ecore_evas_drm_interface_new(void)
{
   Ecore_Evas_Interface_Drm *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Drm));
   if (!iface) return NULL;

   iface->base.name = "drm";
   iface->base.version = 1;

   return iface;
}

static int
_ecore_evas_drm_init(Ecore_Evas *ee EINA_UNUSED, Ecore_Evas_Engine_Drm_Data *edata, const char *device)
{
   _drm_init_count++;
   if (_drm_init_count > 1) return _drm_init_count;

   if (!ecore_drm2_init())
     {
        ERR("Failed to init Ecore_Drm2 library");
        goto init_err;
     }

   if (!device) device = getenv("XDG_SEAT");
   if (!device) device = "seat0";

   edata->dev = ecore_drm2_device_open(device, 0);
   if (!edata->dev)
     {
        ERR("Failed to open Ecore_Drm2 device");
        goto open_err;
     }

   edata->disp = ecore_drm2_display_find(edata->dev, edata->x, edata->y);

   ecore_drm2_device_preferred_depth_get(edata->dev,
                                         &edata->depth, &edata->bpp);

   ecore_event_evas_init();

   /* TODO: handlers for elput_event_device_change */

   return _drm_init_count;

open_err:
   ecore_drm2_shutdown();
init_err:
   return --_drm_init_count;
}

static int
_ecore_evas_drm_shutdown(Ecore_Evas_Engine_Drm_Data *edata)
{
   _drm_init_count--;
   if (_drm_init_count == 0)
     {
        if (edata->dev)
          {
             ecore_drm2_device_close(edata->dev);
             edata->dev = NULL;
          }

        ecore_event_evas_shutdown();
        ecore_drm2_shutdown();

        /* TODO: free handlers */
     }

   if (_drm_init_count < 0) _drm_init_count = 0;
   return _drm_init_count;
}

static void
_drm_focus_job(void *data)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;

   ee = data;
   edata = ee->engine.data;

   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
   edata->focus_job = NULL;
}

static void
_drm_avoid_damage_do(Ecore_Evas *ee, int on)
{
   ee->prop.avoid_damage = on;
   if (ee->prop.avoid_damage)
     {
        if (ECORE_EVAS_PORTRAIT(ee))
          evas_damage_rectangle_add(ee->evas, ee->x, ee->y, ee->w, ee->h);
        else
          evas_damage_rectangle_add(ee->evas, ee->y, ee->x, ee->h, ee->w);
     }
}

static void
_drm_alpha_do(Ecore_Evas *ee, int on)
{
   Evas_Engine_Info_Drm *einfo;

   if (ee->alpha == on) return;

   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   ee->alpha = on;
   einfo->alpha = on;

   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);

   if (ECORE_EVAS_PORTRAIT(ee))
     evas_damage_rectangle_add(ee->evas, ee->x, ee->y, ee->w, ee->h);
   else
     evas_damage_rectangle_add(ee->evas, ee->y, ee->x, ee->h, ee->w);
}

static void
_drm_transparent_do(Ecore_Evas *ee, int on)
{
   Evas_Engine_Info_Drm *einfo;

   if (ee->transparent == on) return;

   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   ee->transparent = on;
   einfo->alpha = on;

   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);

   if (ECORE_EVAS_PORTRAIT(ee))
     evas_damage_rectangle_add(ee->evas, ee->x, ee->y, ee->w, ee->h);
   else
     evas_damage_rectangle_add(ee->evas, ee->y, ee->x, ee->h, ee->w);
}

static void
_drm_rotation_do(Ecore_Evas *ee, int rotation, int resize)
{
   Evas_Engine_Info_Drm *einfo;
   int diff;

   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   diff = ee->rotation - rotation;
   if (diff < 0) diff = -diff;

   einfo->rotation = rotation;

   if (diff != 180)
     {
        int minw, minh, maxw, maxh, basew, baseh, stepw, steph;

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);

        if (!resize)
          {
             if (!ee->prop.fullscreen)
               {
                  ee->expecting_resize.w = ee->h;
                  ee->expecting_resize.h = ee->w;
                  evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
                  evas_output_viewport_set(ee->evas, ee->x, ee->y, ee->req.h, ee->req.w);
               }
             else
               {
                  if (PORTRAIT_CHECK(rotation))
                    {
                       evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
                       evas_output_viewport_set(ee->evas, ee->x, ee->y, ee->req.w, ee->req.h);
                    }
                  else
                    {
                       evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
                       evas_output_viewport_set(ee->evas, ee->y, ee->x, ee->req.h, ee->req.w);
                    }
                  if (ee->func.fn_resize) ee->func.fn_resize(ee);
               }
             if (PORTRAIT_CHECK(rotation))
               evas_damage_rectangle_add(ee->evas, ee->x, ee->y, ee->req.w, ee->req.h);
             else
               evas_damage_rectangle_add(ee->evas, ee->y, ee->x, ee->req.h, ee->req.w);
          }
        else
          {
             if (PORTRAIT_CHECK(rotation))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, ee->x, ee->y, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, ee->y, ee->x, ee->h, ee->w);
               }

             if (ee->func.fn_resize) ee->func.fn_resize(ee);

             if (PORTRAIT_CHECK(rotation))
               evas_damage_rectangle_add(ee->evas, ee->x, ee->y, ee->w, ee->h);
             else
               evas_damage_rectangle_add(ee->evas, ee->y, ee->x, ee->h, ee->w);
          }
        ecore_evas_size_min_get(ee, &minw, &minh);
        ecore_evas_size_max_get(ee, &maxw, &maxh);
        ecore_evas_size_base_get(ee, &basew, &baseh);
        ecore_evas_size_step_get(ee, &stepw, &steph);
        ee->rotation = rotation;
        ecore_evas_size_min_set(ee, minh, minw);
        ecore_evas_size_max_set(ee, maxh, maxw);
        ecore_evas_size_base_set(ee, baseh, basew);
        ecore_evas_size_step_set(ee, steph, stepw);
        /* TODO: mouse_move_process ?? */
     }
   else
     {
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);

        ee->rotation = rotation;

        /* TODO: mouse_move_process ?? */

        if (ee->func.fn_resize) ee->func.fn_resize(ee);

        if (PORTRAIT_CHECK(rotation))
          evas_damage_rectangle_add(ee->evas, ee->x, ee->y, ee->w, ee->h);
        else
          evas_damage_rectangle_add(ee->evas, ee->y, ee->x, ee->h, ee->w);
     }
}

static int
_drm_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;

   if (ee->prop.avoid_damage)
     {
        if (updates)
          {
             /* TODO: process updates ? */
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }
     }
   else if (((ee->visible) && (!ee->draw_block)) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     {
        if (updates)
          {
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }
     }
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}

/* local ecore_evas engine functions */
static int
_drm_render(Ecore_Evas *ee)
{
   int rend = 0;

   if (ee->in_async_render) return 0;

   rend = ecore_evas_render_prepare(ee);

   if (!ee->can_async_render)
     {
        Eina_List *updates;

        updates = evas_render_updates(ee->evas);
        rend = _drm_render_updates_process(ee, updates);
        evas_render_updates_free(updates);
     }
   else if (evas_render_async(ee->evas))
     {
        ee->in_async_render = EINA_TRUE;
        rend = 1;
     }
   else if (ee->func.fn_post_render)
     ee->func.fn_post_render(ee);

   return rend;
}

static void
_drm_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   ecore_evas_input_event_unregister(ee);

   edata = ee->engine.data;
   ecore_main_fd_handler_del(edata->fd_hdlr);
   edata->fd_hdlr = NULL;
   _ecore_evas_drm_shutdown(edata);
   free(edata);
}

static void
_drm_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;

   ee->req.x = x;
   ee->req.y = y;
   if ((ee->x == x) && (ee->y == y)) return;
   ee->x = x;
   ee->y = y;
   edata->x = x;
   edata->y = y;
   if (ee->func.fn_move) ee->func.fn_move(ee);
}

static void
_drm_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;

   ee->req.w = w;
   ee->req.h = h;
   if ((ee->w == w) && (ee->h == h)) return;
   ee->w = w;
   ee->h = h;
   edata->w = w;
   edata->h = h;

   if (ECORE_EVAS_PORTRAIT(ee))
     {
        evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
        evas_output_viewport_set(ee->evas, ee->x, ee->y, ee->req.w, ee->req.h);
     }
   else
     {
        evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
        evas_output_viewport_set(ee->evas, ee->y, ee->x, ee->req.h, ee->req.w);
     }

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_drm_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   _drm_move(ee, x, y);
   _drm_resize(ee, w, h);
}

static void
_drm_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   if (ee->rotation == rotation) return;
   if (ee->in_async_render)
     {
        ee->delayed.rotation = rotation;
        ee->delayed.rotation_resize = resize;
        ee->delayed.rotation_changed = EINA_TRUE;
        return;
     }
   _drm_rotation_do(ee, rotation, resize);
}

static void
_drm_show(Ecore_Evas *ee)
{
   if (ee->visible) return;

   ee->should_be_visible = EINA_TRUE;

   if (ee->prop.avoid_damage)
     {
        _drm_render(ee);
        /* ecore_evas_render(ee); */
        /* ecore_evas_render_wait(ee); */
     }

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_FALSE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   ee->visible = EINA_TRUE;
   if (ee->func.fn_show) ee->func.fn_show(ee);

   if (ee->prop.fullscreen)
     {
        Ecore_Evas_Engine_Drm_Data *edata;

        edata = ee->engine.data;
        if (!edata->focus_job)
          edata->focus_job = ecore_job_add(_drm_focus_job, ee);
     }

   /* TODO: flip ? */
}

static void
_drm_hide(Ecore_Evas *ee)
{
   if (!ee->visible) return;

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_TRUE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   ee->visible = EINA_FALSE;
   ee->should_be_visible = EINA_FALSE;
   evas_sync(ee->evas);
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void
_drm_title_set(Ecore_Evas *ee, const char *title)
{
   if (eina_streq(ee->prop.title, title)) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
}

static void
_drm_name_class_set(Ecore_Evas *ee, const char *name, const char *cls)
{
   if (!eina_streq(ee->prop.name, name))
     {
        if (ee->prop.name) free(ee->prop.name);
        ee->prop.name = NULL;
        if (name) ee->prop.name = strdup(name);
     }

   if (!eina_streq(ee->prop.clas, cls))
     {
        if (ee->prop.clas) free(ee->prop.clas);
        ee->prop.clas = NULL;
        if (cls) ee->prop.clas = strdup(cls);
     }
}

static void
_drm_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
}

static void
_drm_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
}

static void
_drm_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
}

static void
_drm_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
}

static void
_drm_layer_set(Ecore_Evas *ee, int layer)
{
   if (ee->prop.layer == layer) return;
   if (layer < 1) layer = 1;
   else if (layer > 255) layer = 255;
   ee->prop.layer = layer;
}

static void
_drm_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.iconified == on) return;
   ee->prop.iconified = on;
   if (!on) ecore_evas_activate(ee);
}

static void
_drm_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.borderless == on) return;
   ee->prop.borderless = on;
}

static void
_drm_maximized_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.maximized == on) return;
   ee->prop.maximized = on;
}

static void
_drm_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   Eina_Bool resized = EINA_FALSE;

   edata = ee->engine.data;

   if (ee->prop.fullscreen == on) return;
   ee->prop.fullscreen = on;

   if (on)
     {
        /* TODO: get output size */
     }
   else
     {
        if ((edata->w != ee->w) || (edata->h != ee->h))
          resized = EINA_TRUE;
        ee->w = edata->w;
        ee->h = edata->h;
     }

   ee->req.w = ee->w;
   ee->req.h = ee->h;

   evas_output_size_set(ee->evas, ee->w, ee->h);

   if (ECORE_EVAS_PORTRAIT(ee))
     {
        evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
        evas_output_viewport_set(ee->evas, ee->x, ee->y, ee->req.w, ee->req.h);
     }
   else
     {
        evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
        evas_output_viewport_set(ee->evas, ee->y, ee->x, ee->req.h, ee->req.w);
     }

   if (resized)
     if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_drm_avoid_damage_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.avoid_damage == on) return;
   if (ee->in_async_render)
     {
        ee->delayed.avoid_damage = on;
        return;
     }
   _drm_avoid_damage_do(ee, on);
}

static void
_drm_withdrawn_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.withdrawn == on) return;
   ee->prop.withdrawn = on;
   if (on)
     ecore_evas_hide(ee);
   else
     ecore_evas_show(ee);
}

static void
_drm_ignore_events_set(Ecore_Evas *ee, int on)
{
   if (ee->ignore_events == on) return;
   ee->ignore_events = on;
}

static void
_drm_alpha_set(Ecore_Evas *ee, int on)
{
   if (ee->in_async_render)
     {
        if (ee->visible)
          {
             ee->delayed.alpha = on;
             ee->delayed.alpha_changed = EINA_TRUE;
             return;
          }
        evas_sync(ee->evas);
     }
   _drm_alpha_do(ee, on);
}

static void
_drm_transparent_set(Ecore_Evas *ee, int on)
{
   if (ee->in_async_render)
     {
        if (ee->visible)
          {
             ee->delayed.transparent = on;
             ee->delayed.transparent_changed = EINA_TRUE;
             return;
          }
        evas_sync(ee->evas);
     }
   _drm_transparent_do(ee, on);
}

static void
_drm_aspect_set(Ecore_Evas *ee, double aspect)
{
   if (EINA_FLT_EQ(ee->prop.aspect, aspect)) return;
   ee->prop.aspect = aspect;
}

/* TODO: use relative mode for these */
//screen_geom
//screen_dpi

static void
_drm_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   ecore_drm2_device_pointer_xy_get(edata->dev, x, y);
}

static Eina_Bool
_drm_pointer_warp(const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   ecore_drm2_device_pointer_warp(edata->dev, x, y);
   return EINA_TRUE;
}

static void
_drm_tick_job(void *data)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   int x, y, w, h;

   ee = data;
   edata = ee->engine.data;
   edata->tick_job = NULL;

   ecore_drm2_display_info_get(edata->disp, &x, &y, &w, &h, NULL);

   ecore_evas_animator_tick(ee, &(Eina_Rectangle){x, y, w, h},
                            edata->tick_timestamp - edata->tick_offset);
}

static void
_drm_animator_register(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   Eina_Bool ret = EINA_FALSE;
   double timestamp;
   long sec, usec;

   edata = ee->engine.data;

   if ((ee->animator_ticked) || (ee->animator_ran))
     {
        edata->ticking = EINA_TRUE;
        return;
     }

   if (!edata->blanktime)
     {
        /* FIXME: Account for multiple outputs ? */
        ret = ecore_drm2_display_blanktime_get(edata->disp, 1, &sec, &usec);
        if (ret)
          {
             timestamp = (double)sec + ((double)usec / 1000000.0);
             edata->tick_offset = timestamp - ecore_time_get();
             if (fabs(edata->tick_offset) < 0.010)
               edata->tick_offset = 0.0;
             edata->blanktime = EINA_TRUE;
          }
     }

   if ((!edata->ticking) || (ee->in_async_render))
     {
        /* FIXME: Account for multiple outputs ? */
        ret = ecore_drm2_display_blanktime_get(edata->disp, 0, &sec, &usec);
        if (ret)
          {
             edata->tick_timestamp = (double)sec + ((double)usec / 1000000);
             edata->tick_job = ecore_job_add(_drm_tick_job, ee);
          }
        else
          {
             /* TODO: flip ?? */
          }
     }

   edata->ticking = EINA_TRUE;
}

static void
_drm_animator_unregister(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   edata->ticking = EINA_FALSE;
   if (edata->tick_job)
     {
        ecore_job_del(edata->tick_job);
        edata->tick_job = NULL;
     }
}

static void
_drm_evas_changed(Ecore_Evas *ee, Eina_Bool changed)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   if (changed) return;

   edata = ee->engine.data;
   if (edata->ticking)
     {
        /* TODO: flip ? */
     }
}

static double
_drm_last_tick_get(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   long sec, usec;

   edata = ee->engine.data;

   /* FIXME: Account for multiple outputs */

   /* get blank time */
   if (!ecore_drm2_display_blanktime_get(edata->disp, 0, &sec, &usec))
     return -1.0;

   return (double)sec + ((double)usec / 1000000);
}

static Ecore_Evas_Engine_Func _ecore_evas_drm_engine_func =
{
   _drm_free,
   NULL, //void (*fn_callback_resize_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_move_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_show_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_hide_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //_ecore_evas_drm_delete_request_set,
   NULL, //void (*fn_callback_destroy_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //_ecore_evas_drm_callback_focus_in_set,
   NULL, //_ecore_evas_drm_callback_focus_out_set,
   NULL, //_ecore_evas_drm_callback_mouse_in_set,
   NULL, //_ecore_evas_drm_callback_mouse_out_set,
   NULL, //void (*fn_callback_sticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_unsticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_pre_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_post_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   _drm_move,
   NULL, //void (*fn_managed_move) (Ecore_Evas *ee, int x, int y);
   _drm_resize,
   _drm_move_resize,
   _drm_rotation_set,
   NULL, //void (*fn_shaped_set) (Ecore_Evas *ee, int shaped);
   _drm_show,
   _drm_hide,
   NULL, //void (*fn_raise) (Ecore_Evas *ee);
   NULL, //void (*fn_lower) (Ecore_Evas *ee);
   NULL, //void (*fn_activate) (Ecore_Evas *ee);
   _drm_title_set,
   _drm_name_class_set,
   _drm_size_min_set,
   _drm_size_max_set,
   _drm_size_base_set,
   _drm_size_step_set,
   NULL, // object_cursor_set
   NULL, // object_cursor_unset
   _drm_layer_set,
   NULL, //void (*fn_focus_set) (Ecore_Evas *ee, Eina_Bool on);
   _drm_iconified_set,
   _drm_borderless_set,
   NULL, //void (*fn_override_set) (Ecore_Evas *ee, Eina_Bool on);
   _drm_maximized_set,
   _drm_fullscreen_set,
   _drm_avoid_damage_set,
   _drm_withdrawn_set,
   NULL, //void (*fn_sticky_set) (Ecore_Evas *ee, Eina_Bool on);
   _drm_ignore_events_set,
   _drm_alpha_set,
   _drm_transparent_set,
   NULL, //void (*fn_profiles_set) (Ecore_Evas *ee, const char **profiles, int count);
   NULL, //void (*fn_profile_set) (Ecore_Evas *ee, const char *profile);
   NULL, //void (*fn_window_group_set) (Ecore_Evas *ee, const Ecore_Evas *ee_group);
   _drm_aspect_set,
   NULL, //void (*fn_urgent_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_modal_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_demands_attention_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_focus_skip_set) (Ecore_Evas *ee, Eina_Bool on);
   _drm_render,
   NULL, //_drm_screen_geometry_get,
   NULL, //_drm_screen_dpi_get,
   NULL, //void (*fn_msg_parent_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   NULL, //void (*fn_msg_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   _drm_pointer_xy_get,
   _drm_pointer_warp,
   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done
   NULL, // aux_hints_set
#ifndef VSYNC_ANIMATOR
   NULL,
   NULL,
#else
   _drm_animator_register, // animator_register
   _drm_animator_unregister, // animator_unregister
#endif
   _drm_evas_changed,
   NULL, //fn_focus_device_set
   NULL, //fn_callback_focus_device_in_set
   NULL, //fn_callback_focus_device_out_set
   NULL, //fn_callback_device_mouse_in_set
   NULL, //fn_callback_device_mouse_out_set
   NULL, //fn_pointer_device_xy_get
   NULL, //fn_prepare
   _drm_last_tick_get,
};

static Eina_Bool
_cb_drm_event(void *data EINA_UNUSED, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   int ret;

   ee = data;
   edata = ee->engine.data;

   ret = ecore_drm2_device_context_event_handle(edata->dev, &edata->ctx);
   if (ret) return EINA_FALSE;
   return EINA_TRUE;
}

static void
_cb_drm_pageflip2(int fd EINA_UNUSED, unsigned int seq EINA_UNUSED, unsigned int tv_sec EINA_UNUSED, unsigned int tv_usec EINA_UNUSED, unsigned int crtc_id EINA_UNUSED, void *data EINA_UNUSED)
{
   /* Ecore_Drm2_Display *disp; */

   /* disp = data; */

   /* TODO */
}

static void
_cb_drm_render_flush_post(void *data, Evas *evas EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;

   ee = data;
   edata = ee->engine.data;

   /* TODO: flip buffers */
}

static void
_cb_drm_render_updates(void *data, Evas *evas EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Evas_Event_Render_Post *ev;

   ee = data;
   ev = event;
   if (!ev) return;

   ee->in_async_render = EINA_FALSE;

   _drm_render_updates_process(ee, ev->updated_area);

   if (ee->delayed.alpha_changed)
     {
        _drm_alpha_do(ee, ee->delayed.alpha);
        ee->delayed.alpha_changed = EINA_FALSE;
     }

   if (ee->delayed.transparent_changed)
     {
        _drm_transparent_do(ee, ee->delayed.transparent);
        ee->delayed.transparent_changed = EINA_FALSE;
     }

   if (ee->delayed.avoid_damage != ee->prop.avoid_damage)
     _drm_avoid_damage_do(ee, ee->delayed.avoid_damage);

   if (ee->delayed.rotation_changed)
     ee->delayed.rotation_changed = EINA_FALSE;
}

static Ecore_Evas *
_ecore_evas_new_internal(const char *device, int x, int y, int w, int h, Eina_Bool gl)
{
   Ecore_Evas *ee;
   Ecore_Evas_Interface_Drm *iface;
   Ecore_Evas_Engine_Drm_Data *edata;
   int method = 0;

   if (gl)
     {
        /* TODO */
     }
   else
     method = evas_render_method_lookup("drm");

   if (!method) return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   edata = calloc(1, sizeof(Ecore_Evas_Engine_Drm_Data));
   if (!edata)
     {
        free(ee);
        return NULL;
     }

   edata->x = x;
   edata->y = y;
   edata->w = w;
   edata->h = h;
   edata->format = DRM_FORMAT_XRGB8888;

   if (_ecore_evas_drm_init(ee, edata, device) < 1)
     {
        free(edata);
        free(ee);
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   if (gl) ee->driver = "gl_drm";
   else ee->driver = "drm";

   ee->engine.func->fn_animator_register = NULL;
   ee->engine.func->fn_animator_unregister = NULL;
   if (ecore_drm2_device_vblank_supported(edata->dev))
     {
        ee->engine.func->fn_animator_register = _drm_animator_register;
        ee->engine.func->fn_animator_unregister = _drm_animator_unregister;
     }

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_evas_drm_engine_func;
   ee->engine.data = edata;

   iface = _ecore_evas_drm_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->x = ee->req.x = x;
   ee->y = ee->req.y = y;
   ee->w = ee->req.w = w;
   ee->h = ee->req.h = h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->prop.withdrawn = EINA_TRUE;
   ee->alpha = EINA_FALSE;

   ee->can_async_render = !gl;
   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;

   if (!ecore_evas_evas_new(ee, w, h))
     {
        ERR("Can not create a Canvas.");
        goto eng_err;
     }

   evas_output_method_set(ee->evas, method);
   evas_output_viewport_set(ee->evas, x, y, w, h);

   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST,
                           _cb_drm_render_flush_post, ee);
   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
                             _cb_drm_render_updates, ee);

#ifdef BUILD_ECORE_EVAS_GL_DRM
   if (gl)
     {
        /* TODO */
     }
   else
#endif
     {
        Evas_Engine_Info_Drm *einfo;

        einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
        if (!einfo) goto eng_err;

        einfo->dev = edata->dev;
        einfo->bpp = edata->bpp;
        einfo->depth = edata->depth;
        einfo->format = edata->format;
        einfo->rotation = edata->rotation;
        einfo->alpha = ee->alpha;

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);
             goto eng_err;
          }
     }

   /* TODO: prop window */

   ecore_evas_data_set(ee, "device", edata->dev);

   /* FIXME: Call ecore_evas_done when we have ee->prop.window */
   /* ecore_evas_done(ee, EINA_FALSE); */

   /* TODO: finish: (drm2_device_calibrate, etc) */
   memset(&edata->ctx, 0, sizeof(edata->ctx));
   edata->ctx.page_flip_handler2 = _cb_drm_pageflip2;

   edata->fd_hdlr =
     ecore_main_fd_handler_add(ecore_drm2_device_fd_get(edata->dev),
                               ECORE_FD_READ, _cb_drm_event, ee, NULL, NULL);

   return ee;

eng_err:
   ecore_evas_free(ee);
   return NULL;
}

EMODAPI Ecore_Evas *
ecore_evas_drm_new_internal(const char *device, unsigned int parent EINA_UNUSED, int x, int y, int w, int h)
{
   return _ecore_evas_new_internal(device, x, y, w, h, EINA_FALSE);
}
