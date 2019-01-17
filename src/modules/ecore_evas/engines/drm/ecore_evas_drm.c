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

#define DRM2_NODEFS
#include "ecore_drm2_private.h"

#define ELPUT_NODEFS
#include "elput_private.h"

#ifdef BUILD_ECORE_EVAS_GL_DRM
# include <Evas_Engine_GL_Drm.h>
# include <dlfcn.h>
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

typedef struct _Ecore_Evas_Engine_Drm_Data
{
   int cw, ch;
   int clockid;
   int x, y, w, h;
   int depth, bpp;
   unsigned int format;
   double offset;
   double tick_job_timestamp;
   Ecore_Drm2_Context ctx;
   Ecore_Fd_Handler *hdlr;
   Ecore_Drm2_Device *dev;
   Ecore_Drm2_Output *output;
   Evas_Device *seat;
   Eina_Bool ticking : 1;
   Eina_Bool once : 1;
   Ecore_Job *tick_job;
   Ecore_Job *focus_job;
} Ecore_Evas_Engine_Drm_Data;

static int _drm_init_count = 0;
static Eina_List *handlers;
static Eina_List *canvases;

static Eina_Bool
_drm_device_change(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Elput_Event_Device_Change *ev = event;
   const Eina_List *l;
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   Elput_Seat *seat;
   Elput_Manager *manager;
   Eina_Bool found = EINA_FALSE;
   Elput_Device_Caps caps;
   Evas_Device_Class devclass = EVAS_DEVICE_CLASS_NONE;
   Eo *dev;

   seat = elput_device_seat_get(ev->device);
   manager = elput_seat_manager_get(seat);
   caps = elput_device_caps_get(ev->device);

   EINA_LIST_FOREACH(canvases, l, ee)
     {
        edata = ee->engine.data;
        found = edata->dev->em == manager;
        if (found) break;
     }

   if (!found) return ECORE_CALLBACK_RENEW;
   if (caps & ELPUT_DEVICE_CAPS_TABLET_TOOL)
     devclass = EVAS_DEVICE_CLASS_PEN; // idk how "pen" is a device class?
   else if (caps & ELPUT_DEVICE_CAPS_POINTER)
     devclass = EVAS_DEVICE_CLASS_MOUSE;
   else if (caps & ELPUT_DEVICE_CAPS_TOUCH)
     devclass = EVAS_DEVICE_CLASS_TOUCH;
   else if (caps & ELPUT_DEVICE_CAPS_KEYBOARD)
     devclass = EVAS_DEVICE_CLASS_KEYBOARD;
   switch (ev->type)
     {
      case ELPUT_DEVICE_ADDED:
        {
           if (!edata->seat)
             {
                Eina_Stringshare *name = elput_seat_name_get(seat);
                edata->seat = evas_device_add_full(ee->evas, name,
                  "drm seat", NULL, NULL, EVAS_DEVICE_CLASS_SEAT, EVAS_DEVICE_SUBCLASS_NONE);
                evas_device_seat_id_set(edata->seat, strtol(name, NULL, 10));
             }

           dev = evas_device_add_full(ee->evas, elput_device_output_name_get(ev->device),
             "drm device", edata->seat, NULL, devclass, EVAS_DEVICE_SUBCLASS_NONE);
           ev->device->evas_device = dev;
           break;
        }
      case ELPUT_DEVICE_REMOVED:
        {
           EINA_LIST_FOREACH(evas_device_list(ee->evas, edata->seat), l, dev)
             {
                if (dev != ev->device->evas_device) continue;
                evas_device_del(dev);
                ev->device->evas_device = NULL;
                break;
             }
           break;
        }
     }

   return ECORE_CALLBACK_RENEW;
}

static int
_ecore_evas_drm_init(Ecore_Evas *ee, Ecore_Evas_Engine_Drm_Data *edata, const char *device)
{
   if (++_drm_init_count != 1) return _drm_init_count;

   if (!ecore_drm2_init())
     {
        ERR("Failed to init Ecore_Drm2 library");
        goto init_err;
     }

   /* NB: No seat name passed in, try to get from env */
   if (!device) device = getenv("XDG_SEAT");

   /* NB: fallback to seat0 if env var is not set */
   if (!device) device = "seat0";

   edata->dev = ecore_drm2_device_open(device, 0);
   if (!edata->dev)
     {
        ERR("Failed to open device");
        goto open_err;
     }

   edata->clockid = ecore_drm2_device_clock_id_get(edata->dev);
   ecore_drm2_device_cursor_size_get(edata->dev, &edata->cw, &edata->ch);

   if (!ecore_drm2_outputs_create(edata->dev))
     {
        ERR("Could not create outputs");
        goto output_err;
     }

   edata->output = ecore_drm2_output_find(edata->dev, edata->x, edata->y);
   if (edata->output) ecore_drm2_output_user_data_set(edata->output, ee);
   else WRN("Could not find output at %d %d", edata->x, edata->y);

   ecore_event_evas_init();
   if (!handlers)
     {
        handlers =
          eina_list_append(handlers,
                           ecore_event_handler_add(ELPUT_EVENT_DEVICE_CHANGE,
                                                   _drm_device_change, NULL));
     }

   return _drm_init_count;

output_err:
   ecore_drm2_device_close(edata->dev);
open_err:
   ecore_drm2_shutdown();
init_err:
   return --_drm_init_count;
}

static int
_ecore_evas_drm_shutdown(Ecore_Evas_Engine_Drm_Data *edata)
{
   Ecore_Event_Handler *h;
   if (--_drm_init_count != 0) return _drm_init_count;

   if (edata->focus_job)
     {
        ecore_job_del(edata->focus_job);
        edata->focus_job = NULL;
     }
   ecore_drm2_outputs_destroy(edata->dev);
   ecore_drm2_device_close(edata->dev);
   ecore_drm2_shutdown();
   ecore_event_evas_shutdown();
   EINA_LIST_FREE(handlers, h)
     ecore_event_handler_del(h);

   return _drm_init_count;
}

static void
_drm_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   ecore_evas_input_event_unregister(ee);

   edata = ee->engine.data;
   canvases = eina_list_remove(canvases, ee);
   _ecore_evas_drm_shutdown(edata);
   free(edata);
}

static void
_drm_rotation_do(Ecore_Evas *ee, int rotation, int resize)
{
   Evas_Engine_Info_Drm *einfo;
   Ecore_Evas_Engine_Drm_Data *edata;
   Eina_Bool use_hw = EINA_FALSE;
   int diff, rotations = 0, orient = 0;

   if (ee->rotation == rotation) return;

   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   edata = ee->engine.data;

   rotations = ecore_drm2_output_supported_rotations_get(edata->output);
   if (rotations >= 0)
     {
        if (rotations & ECORE_DRM2_ROTATION_NORMAL)
          {
             if (rotation == 0)
               {
                  use_hw = EINA_TRUE;
                  orient = ECORE_DRM2_ROTATION_NORMAL;
               }
          }

        if (rotations & ECORE_DRM2_ROTATION_90)
          {
             if (rotation == 90)
               {
                  use_hw = EINA_TRUE;
                  orient = ECORE_DRM2_ROTATION_90;
               }
          }

        if (rotations & ECORE_DRM2_ROTATION_180)
          {
             if (rotation == 180)
               {
                  use_hw = EINA_TRUE;
                  orient = ECORE_DRM2_ROTATION_180;
               }
          }

        if (rotations & ECORE_DRM2_ROTATION_270)
          {
             if (rotation == 270)
               {
                  use_hw = EINA_TRUE;
                  orient = ECORE_DRM2_ROTATION_270;
               }
          }
     }

   if (use_hw)
     {
        ecore_drm2_output_rotation_set(edata->output, orient);
        ee->rotation = rotation;
        return;
     }

   einfo->info.rotation = rotation;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);

   diff = ee->rotation - rotation;
   if (diff < 0) diff = -diff;

   if (diff != 180)
     {
        if (!resize)
          {
             int ww, hh;

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
          }
     }

   ee->rotation = rotation;

   if (ee->func.fn_resize) ee->func.fn_resize(ee);

   if (ECORE_EVAS_PORTRAIT(ee))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   else
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
}

static void
_drm_render_updates(void *data, Evas *evas EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Evas *ee = data;

   if (ee->delayed.rotation_changed)
     {
        _drm_rotation_do(ee, ee->delayed.rotation, ee->delayed.rotation_resize);
        ee->delayed.rotation_changed = EINA_FALSE;
     }
}

static void
_drm_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   ecore_drm2_output_info_get(edata->output, x, y, w, h, NULL);
}

static void
_drm_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   ecore_drm2_output_dpi_get(edata->output, xdpi, ydpi);
}

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
_drm_show_focus_job(void *data)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Drm_Data *edata;

   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
   edata = ee->engine.data;
   edata->focus_job = NULL;
}

static void
_drm_show(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   if ((!ee) || (ee->visible)) return;

   ee->should_be_visible = 1;

   if (ee->prop.avoid_damage)
     {
        ecore_evas_render(ee);
        ecore_evas_render_wait(ee);
     }

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_FALSE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   if (ee->visible) return;

   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);

   edata = ee->engine.data;
   edata->focus_job = ecore_job_add(_drm_show_focus_job, ee);
   /* HACK: sometimes we still have an animator ticking when we vc switch
    * so for now we just fire off a flip here to kick it when we come back.
    * This is just papering over a bug for now until I have time to track
    * it down properly. :(
    */
   ecore_drm2_fb_flip(NULL, edata->output);
}

static void
_drm_hide(Ecore_Evas *ee)
{
   if ((!ee) || (!ee->visible)) return;

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_TRUE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   if (!ee->visible) return;

   ee->visible = 0;
   ee->should_be_visible = 0;
   evas_sync(ee->evas);
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void
_drm_move(Ecore_Evas *ee, int x, int y)
{
   ee->req.x = x;
   ee->req.y = y;
   if ((ee->x == x) && (ee->y == y)) return;
   ee->x = x;
   ee->y = y;
   if (ee->func.fn_move) ee->func.fn_move(ee);
}

static void
_drm_resize(Ecore_Evas *ee, int w, int h)
{
   ee->req.w = w;
   ee->req.h = h;
   if ((ee->w == w) && (ee->h == h)) return;
   ee->w = w;
   ee->h = h;
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_drm_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   if ((ee->x != x) || (ee->y != y))
     _drm_move(ee, x, y);
   if ((ee->w != w) || (ee->h != h))
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
     }
   else
     _drm_rotation_do(ee, rotation, resize);
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
_drm_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
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
}

static void
_drm_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
}

static void
_drm_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
}

static void
_drm_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
}

static void
_drm_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
}

static void
_drm_layer_set(Ecore_Evas *ee, int layer)
{
   if (layer < 1) layer = 1;
   else if (layer > 255) layer = 255;
   if (ee->prop.layer == layer) return;
   ee->prop.layer = layer;
}

static void
_drm_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.iconified == on) return;
   ee->prop.iconified = on;
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
   Eina_Bool resized = EINA_FALSE;
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   if (ee->prop.fullscreen == on) return;
   ee->prop.fullscreen = on;

   if (on)
     {
        int ow = 0, oh = 0;

        edata->w = ee->w;
        edata->h = ee->h;

        ecore_drm2_output_info_get(edata->output, NULL, NULL, &ow, &oh, NULL);
        if ((ow == 0) || (oh == 0))
          {
             ow = ee->w;
             oh = ee->h;
          }
        if ((ow != ee->w) || (oh != ee->h)) resized = EINA_TRUE;
        ee->w = ow;
        ee->h = oh;
     }
   else
     {
        if ((edata->w != ee->w) || (edata->h != ee->h)) resized = EINA_TRUE;
        ee->w = edata->w;
        ee->h = edata->h;
     }

   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->prop.fullscreen = on;
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (resized)
     {
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_drm_withdrawn_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.withdrawn == on) return;
   ee->prop.withdrawn = on;
   if (on) ecore_evas_hide(ee);
   else ecore_evas_show(ee);
}

static void
_drm_ignore_events_set(Ecore_Evas *ee, int on)
{
   if (ee->ignore_events == on) return;
   ee->ignore_events = on;
}

static void
_drm_alpha_set(Ecore_Evas *ee, int alpha)
{
   if (ee->in_async_render)
     {
        ee->delayed.alpha = alpha;
        ee->delayed.alpha_changed = EINA_TRUE;
     }
}

static void
_drm_transparent_set(Ecore_Evas *ee, int transparent)
{
   if (ee->in_async_render)
     {
        ee->delayed.transparent = transparent;
        ee->delayed.transparent_changed = EINA_TRUE;
     }
}

static void
_drm_aspect_set(Ecore_Evas *ee, double aspect)
{
   if (EINA_FLT_EQ(ee->prop.aspect, aspect)) return;
   ee->prop.aspect = aspect;
}

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

static Eina_Bool
_cb_drm_event(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   int ret;

   ee = data;
   edata = ee->engine.data;
   ret = ecore_drm2_event_handle(edata->dev, &edata->ctx);
   if (ret)
     {
        WRN("drmHandleEvent failed to read an event");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_cb_pageflip(int fd EINA_UNUSED, unsigned int frame EINA_UNUSED, unsigned int sec, unsigned int usec, void *data)
{
   Ecore_Evas *ee;
   Ecore_Drm2_Output *output;
   Ecore_Evas_Engine_Drm_Data *edata;
   int ret;

   output = data;

   ee = ecore_drm2_output_user_data_get(output);
   if (!ee) return;

   edata = ee->engine.data;

   ret = ecore_drm2_fb_flip_complete(output);

   if (edata->ticking)
     {
        int x, y, w, h;
        double t = (double)sec + ((double)usec / 1000000);

        ecore_drm2_output_info_get(output, &x, &y, &w, &h, NULL);

        if (!edata->once) t = ecore_time_get();
        ecore_evas_animator_tick(ee, &(Eina_Rectangle){x, y, w, h},
                                 t - edata->offset);
     }
   else if (ret)
     ecore_drm2_fb_flip(NULL, output);
}

static void
_drm_evas_changed(Ecore_Evas *ee, Eina_Bool changed)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   if (changed) return;

   edata = ee->engine.data;
   if (edata->ticking && !ecore_drm2_output_pending_get(edata->output))
     ecore_drm2_fb_flip(NULL, edata->output);
}

static void
_tick_job(void *data)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   Ecore_Evas *ee;
   int x, y, w, h;

   ee = data;
   edata = ee->engine.data;
   edata->tick_job = NULL;

   ecore_drm2_output_info_get(edata->output, &x, &y, &w, &h, NULL);

   ecore_evas_animator_tick(ee, &(Eina_Rectangle){x, y, w, h},
                            edata->tick_job_timestamp - edata->offset);
}

static void
_drm_animator_register(Ecore_Evas *ee)
{
   double t;
   long sec, usec;
   Ecore_Evas_Engine_Drm_Data *edata;
   Eina_Bool r;

   if (ee->manual_render)
     ERR("Attempt to schedule tick for manually rendered canvas");

   edata = ee->engine.data;

   /* Some graphics stacks appear to lie about their clock sources
    * so attempt to measure the difference between our clock and the
    * GPU's source of timestamps once at startup and apply that.
    * If it's tiny, just assume they're the same clock and it's
    * measurement error.
    *
    * <cedric> what happen when you suspend ?
    * <cedric> what about drift ?
    *
    * If someone could relay the message to cedric that I'm not
    * talking to him anymore, that would be helpful.
    */
   if (!edata->once)
     {
        r = ecore_drm2_output_blanktime_get(edata->output, 1, &sec, &usec);
        if (r)
          {
             t = (double)sec + ((double)usec / 1000000.0);
             edata->offset = t - ecore_time_get();
             if (fabs(edata->offset) < 0.010)
               edata->offset = 0.0;

             edata->once = EINA_TRUE;
          }
     }

   if (ee->animator_ticked || ee->animator_ran)
     {
        edata->ticking = EINA_TRUE;
        return;
     }

   if (edata->tick_job) ERR("Double animator register");
   else
   if (!edata->ticking &&
       !(ecore_drm2_output_pending_get(edata->output) || ee->in_async_render))
     {
        r = ecore_drm2_output_blanktime_get(edata->output, 0, &sec, &usec);
        if (r)
          {
             edata->tick_job_timestamp = (double)sec
                                       + ((double)usec / 1000000);
             edata->tick_job = ecore_job_add(_tick_job, ee);
          }
        else
          ecore_drm2_fb_flip(NULL, edata->output);
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
        ERR("Animator unregister before first tick");
        ecore_job_del(edata->tick_job);
        edata->tick_job = NULL;
     }
}

static double
_drm_last_tick_get(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   long sec, usec;

   edata = ee->engine.data;
   if (!ecore_drm2_output_blanktime_get(edata->output, 0, &sec, &usec))
     return -1.0;

   return sec + usec / 1000000.0;
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
   NULL,
   NULL,
   _drm_layer_set,
   NULL, //void (*fn_focus_set) (Ecore_Evas *ee, Eina_Bool on);
   _drm_iconified_set,
   _drm_borderless_set,
   NULL, //void (*fn_override_set) (Ecore_Evas *ee, Eina_Bool on);
   _drm_maximized_set,
   _drm_fullscreen_set,
   NULL, //void (*fn_avoid_damage_set) (Ecore_Evas *ee, int on);
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

   NULL,

   _drm_screen_geometry_get,
   _drm_screen_dpi_get,
   NULL, //void (*fn_msg_parent_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   NULL, //void (*fn_msg_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);

   _drm_pointer_xy_get,
   _drm_pointer_warp,

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL, // aux_hints_set

   _drm_animator_register, // animator_register
   _drm_animator_unregister, // animator_unregister

   _drm_evas_changed, // evas_changed
   NULL, //fn_focus_device_set
   NULL, //fn_callback_focus_device_in_set
   NULL, //fn_callback_focus_device_out_set
   NULL, //fn_callback_device_mouse_in_set
   NULL, //fn_callback_device_mouse_out_set
   NULL, //fn_pointer_device_xy_get
   NULL, //fn_prepare
   _drm_last_tick_get,
};

#ifdef BUILD_ECORE_EVAS_GL_DRM
static void *
_drm_gl_canvas_setup(Ecore_Evas *ee, Ecore_Evas_Engine_Drm_Data *edata)
{
   Evas_Engine_Info_GL_Drm *einfo;
   char *num;

   einfo = (Evas_Engine_Info_GL_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return NULL;

   einfo->info.vsync = EINA_TRUE;

   num = getenv("EVAS_DRM_VSYNC");
   if ((num) && (!atoi(num)))
     einfo->info.vsync = EINA_FALSE;

   einfo->info.dev = edata->dev;
   einfo->info.bpp = edata->bpp;
   einfo->info.depth = edata->depth;
   einfo->info.format = edata->format;
   einfo->info.rotation = ee->rotation;
   einfo->info.output = edata->output;

   return einfo;
}
#endif

static void *
_drm_canvas_setup(Ecore_Evas *ee, Ecore_Evas_Engine_Drm_Data *edata)
{
   Evas_Engine_Info_Drm *einfo;

   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return NULL;

   einfo->info.dev = edata->dev;
   einfo->info.bpp = edata->bpp;
   einfo->info.depth = edata->depth;
   einfo->info.format = edata->format;
   einfo->info.rotation = ee->rotation;
   einfo->info.output = edata->output;

   return einfo;
}

static Ecore_Evas *
_ecore_evas_new_internal(const char *device, int x, int y, int w, int h, Eina_Bool gl)
{
   Ecore_Evas *ee;
   Ecore_Evas_Interface_Drm *iface;
   Ecore_Evas_Engine_Drm_Data *edata;
   int method, mw, mh;
   void *tinfo;

   if (gl) method = evas_render_method_lookup("gl_drm");
   else method = evas_render_method_lookup("drm");

   if (!method) return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   edata = calloc(1, sizeof(Ecore_Evas_Engine_Drm_Data));
   if (!edata)
     {
        free(ee);
        return NULL;
     }

   if (!getenv("ECORE_EVAS_DRM_GPU_CLOCK_WRONG"))
     {
        edata->once = EINA_TRUE;
        edata->offset = 0.0;
     }
   edata->x = x;
   edata->y = y;
   edata->w = w;
   edata->h = h;
   edata->depth = 24; // FIXME: Remove hardcode
   edata->bpp = 32; // FIXME: Remove hardcode
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

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_evas_drm_engine_func;
   ee->engine.data = edata;

   /* FIXME */
   /* if (edata->device) ee->name = strdup(edata->device); */

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

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
                             _drm_render_updates, ee);

#ifdef BUILD_ECORE_EVAS_GL_DRM
   if (gl)
     tinfo = _drm_gl_canvas_setup(ee, edata);
   else
#endif
     tinfo = _drm_canvas_setup(ee, edata);

   if (!tinfo)
     {
        ERR("evas_engine_info_get() for engine '%s' failed", ee->driver);
        goto eng_err;
     }

   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)tinfo))
     {
        ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);
        goto eng_err;
     }

   ee->prop.window = ecore_drm2_output_crtc_get(edata->output);
   ecore_drm2_device_window_set(edata->dev, ee->prop.window);

   ecore_evas_data_set(ee, "device", edata->dev);

   ecore_evas_done(ee, EINA_FALSE);

   ecore_drm2_output_info_get(edata->output, NULL, NULL, &mw, &mh, NULL);

   ecore_drm2_device_calibrate(edata->dev, mw, mh);
   ecore_drm2_device_pointer_max_set(edata->dev, mw, mh);
   ecore_drm2_device_pointer_warp(edata->dev, mw / 2, mh / 2);

   /* setup vblank handler */
   memset(&edata->ctx, 0, sizeof(edata->ctx));
   edata->ctx.page_flip_handler = _cb_pageflip;

   edata->hdlr =
     ecore_main_fd_handler_add(ecore_drm2_device_fd_get(edata->dev),
                               ECORE_FD_READ, _cb_drm_event, ee,
                               NULL, NULL);

   canvases = eina_list_append(canvases, ee);
   return ee;

eng_err:
   ecore_evas_free(ee);
   return NULL;
}

EAPI Ecore_Evas *
ecore_evas_drm_new_internal(const char *device, unsigned int parent EINA_UNUSED, int x, int y, int w, int h)
{
   return _ecore_evas_new_internal(device, x, y, w, h, EINA_FALSE);
}

#ifdef BUILD_ECORE_EVAS_GL_DRM
EAPI Ecore_Evas *
ecore_evas_gl_drm_new_internal(const char *device, unsigned int parent EINA_UNUSED, int x, int y, int w, int h)
{
   dlopen("libglapi.so.0", RTLD_LAZY | RTLD_GLOBAL);
   if (dlerror()) return NULL;

   return _ecore_evas_new_internal(device, x, y, w, h, EINA_TRUE);
}
#endif
