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
/* # include <Evas_Engine_GL_Drm.h> */
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

typedef struct _Ecore_Evas_Engine_Drm_Tick
{
   Ecore_Drm2_Output *output;
   Efl_Canvas_Output *canvas;
   double offset, timestamp;
} Ecore_Evas_Engine_Drm_Tick;

typedef struct _Ecore_Evas_Engine_Drm_Data
{
   Ecore_Drm2_Device *dev;
   Ecore_Drm2_Context ctx;

   Ecore_Fd_Handler *hdlr;

   Evas_Device *seat;

   Ecore_Job *tick_job;

   Eina_List *ticks;

   Eina_Bool ticking : 1;
   Eina_Bool once : 1;
} Ecore_Evas_Engine_Drm_Data;

static int _drm_shutdown(Ecore_Evas_Engine_Drm_Data *edata);

static int _drm_init_count = 0;
static Eina_List *canvases, *outputs;
static Ecore_Event_Handler *devhdlr;

static void
_cb_tick(void *data)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   Ecore_Evas_Engine_Drm_Tick *etick;
   Eina_List *l;

   ee = data;
   edata = ee->engine.data;
   edata->tick_job = NULL;

   EINA_LIST_FOREACH(edata->ticks, l, etick)
     {
        Eina_Rectangle rect;
        int ox, oy, ow, oh;

        ecore_drm2_output_info_get(etick->output, &ox, &oy, &ow, &oh, NULL);
        EINA_RECTANGLE_SET(&rect, ox, oy, ow, oh);
        /* efl_canvas_output_unlock(etick->canvas); */
        ecore_evas_animator_tick(ee, &rect, etick->timestamp);
     }
}

static inline Ecore_Evas_Engine_Drm_Tick *
_drm_tick_get(Ecore_Evas_Engine_Drm_Data *edata, Ecore_Drm2_Output *output)
{
   Ecore_Evas_Engine_Drm_Tick *etick;
   Eina_List *l;

   EINA_LIST_FOREACH(edata->ticks, l, etick)
     if (etick->output == output) return etick;

   return NULL;
}

static void
_drm_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   Ecore_Evas_Engine_Drm_Tick *etick;

   edata = ee->engine.data;

   EINA_LIST_FREE(edata->ticks, etick)
     free(etick);

   ecore_job_del(edata->tick_job);
   ecore_evas_input_event_unregister(ee);

   ecore_main_fd_handler_del(edata->hdlr);
   edata->hdlr = NULL;

   canvases = eina_list_remove(canvases, ee);

   _drm_shutdown(edata);
   free(edata);
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
   /* Ecore_Drm2_Output *output; */
   Ecore_Evas_Engine_Drm_Tick *etick;
   Ecore_Evas_Engine_Drm_Data *edata;
   Eina_List *l;

   if ((!ee) || (ee->visible)) return;

   edata = ee->engine.data;

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

   ee->visible = 1;
   if (ee->prop.fullscreen)
     {
        evas_focus_in(ee->evas);
        if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
     }

   if (ee->func.fn_show) ee->func.fn_show(ee);

   /* HACK: sometimes we still have an animator ticking when we vc switch
    * so for now we just fire off a flip here to kick it when we come back.
    * This is just papering over a bug for now until I have time to track
    * it down properly. :(
    */
   /* EINA_LIST_FOREACH(outputs, l, output) */
   /*   ecore_drm2_fb_flip(NULL, output); */

   EINA_LIST_FOREACH(edata->ticks, l, etick)
     {
        /* efl_canvas_output_lock(etick->canvas); */
        ecore_drm2_fb_flip(NULL, etick->output);
     }
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

static void
_drm_screen_geometry_get(const Ecore_Evas *ee EINA_UNUSED, int *x, int *y, int *w, int *h)
{
   Ecore_Drm2_Output *output;
   Eina_List *l;
   int ox, oy, ow, oh;

   if (x) *x = 0;
   if (y) *y = 0;

   EINA_LIST_FOREACH(outputs, l, output)
     {
        ecore_drm2_output_info_get(output, &ox, &oy, &ow, &oh, NULL);
        if (w) *w += MAX(*w, ow);
        if (h) *h = MAX(*h, oh);
     }
}

static void
_drm_screen_dpi_get(const Ecore_Evas *ee EINA_UNUSED, int *xdpi, int *ydpi)
{
   Ecore_Drm2_Output *output;

   if (xdpi) *xdpi = 0;
   if (ydpi) *ydpi = 0;

   output = eina_list_data_get(outputs);
   if (!output) return;

   ecore_drm2_output_dpi_get(output, xdpi, ydpi);
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
_drm_animator_register(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;
   Ecore_Evas_Engine_Drm_Tick *etick;
   Ecore_Drm2_Output *output;
   Eina_List *l;

   if (ee->manual_render)
     {
        ERR("Attempt to schedule tick for manually rendered canvas");
        /* return; */
     }

   edata = ee->engine.data;

   if (!edata->once)
     {
        EINA_LIST_FOREACH(outputs, l, output)
          {
             Eina_Bool r = EINA_FALSE;
             double t = 0.0;
             long sec, usec;

             r = ecore_drm2_output_blanktime_get(output, 1, &sec, &usec);
             if (!r) continue;

             etick = _drm_tick_get(edata, output);
             if (!etick) continue;

             t = (double)sec + ((double)usec / 1000000.0);
             etick->offset = t - ecore_time_get();
             if (fabs(etick->offset) < 0.010)
               etick->offset = 0.0;
          }
        edata->once = EINA_TRUE;
     }

   if (edata->tick_job)
     {
        ERR("Double animator register");
        return;
     }

   EINA_LIST_FOREACH(outputs, l, output)
     {
        Eina_Bool r = EINA_FALSE;
        long sec, usec;

        if (!edata->ticking &&
            !(ecore_drm2_output_pending_get(output) || ee->in_async_render))
          {
             r = ecore_drm2_output_blanktime_get(output, 0, &sec, &usec);
             if (r)
               {
                  etick = _drm_tick_get(edata, output);
                  if (!etick) continue;

                  etick->timestamp = (double)sec + ((double)usec / 1000000.0);
               }
          }
        else
          ecore_drm2_fb_flip(NULL, edata->output);
     }

   if (!edata->ticking)
     edata->tick_job = ecore_job_add(_cb_tick, ee);

   EINA_LIST_FOREACH(edata->ticks, l, etick)
     {
        if ((!ecore_drm2_output_pending_get(etick->output)) &&
            !ee->in_async_render)
          {
             /* efl_canvas_output_lock(etick->canvas); */
             ecore_drm2_fb_flip(NULL, etick->output);
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
   Ecore_Evas_Engine_Drm_Tick *etick;
   Eina_List *l;

   if (changed) return;

   edata = ee->engine.data;
   if (!edata->ticking) return;

   EINA_LIST_FOREACH(edata->ticks, l, etick)
     {
        if (!ecore_drm2_output_pending_get(etick->output))
          {
             /* efl_canvas_output_lock(etick->canvas); */
             ecore_drm2_fb_flip(NULL, etick->output);
          }
     }
}

static double
_drm_last_tick_get(Ecore_Evas *ee EINA_UNUSED)
{
   Ecore_Drm2_Output *output;
   Eina_List *l;
   long sec, usec;
   double tmp = 0.0;

   EINA_LIST_FOREACH(outputs, l, output)
     {
        if (!ecore_drm2_output_blanktime_get(output, 0, &sec, &usec))
          tmp = -1.0;
        else
          {
             if ((sec + usec / 1000000.0) > tmp)
               tmp = sec + usec / 1000000.0;
          }
     }

   return tmp;
}

static Ecore_Evas_Engine_Func _drm_engine_func =
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
   NULL, //_drm_rotation_set,
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
   NULL, //_drm_fullscreen_set,
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
   _drm_animator_register,
   _drm_animator_unregister,
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
_cb_drm_event(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   int ret = 0;

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
   Ecore_Evas_Engine_Drm_Data *edata;
   Ecore_Drm2_Output *output;
   int ret = 0;

   output = data;
   ee = ecore_drm2_output_user_data_get(output);
   edata = ee->engine.data;

   fprintf(stderr, "Pageflip Complete For Output %s\n",
           ecore_drm2_output_name_get(output));

   ret = ecore_drm2_fb_flip_complete(output);
   if (edata->ticking)
     {
        Ecore_Evas_Engine_Drm_Tick *etick;
        double t;

        t = (double)sec + ((double)usec / 1000000);
        if (!edata->once) t = ecore_time_get();

        etick = _drm_tick_get(edata, output);
        if (etick)
          {
             Eina_Rectangle rect;
             int ox, oy, ow, oh;

             ecore_drm2_output_info_get(output, &ox, &oy, &ow, &oh, NULL);
             EINA_RECTANGLE_SET(&rect, ox, oy, ow, oh);
             /* efl_canvas_output_unlock(etick->canvas); */
             ecore_evas_animator_tick(ee, &rect, t - etick->offset);
          }
     }
   else if (ret)
     {
        Ecore_Evas_Engine_Drm_Tick *etick;

        etick = _drm_tick_get(edata, output);
        if (etick)
          {
             /* efl_canvas_output_lock(etick->canvas); */
             ecore_drm2_fb_flip(NULL, etick->output);
          }
     }
}

static Eina_Bool
_cb_device_change(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
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
                Eina_Stringshare *name;

                name = elput_seat_name_get(seat);
                edata->seat =
                  evas_device_add_full(ee->evas, name, "drm seat", NULL, NULL,
                                       EVAS_DEVICE_CLASS_SEAT,
                                       EVAS_DEVICE_SUBCLASS_NONE);
                evas_device_seat_id_set(edata->seat, strtol(name, NULL, 10));
             }

           dev =
             evas_device_add_full(ee->evas,
                                  elput_device_output_name_get(ev->device),
                                  "drm device", edata->seat, NULL, devclass,
                                  EVAS_DEVICE_SUBCLASS_NONE);
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
_drm_init(Ecore_Evas_Engine_Drm_Data *edata, const char *device)
{
   if (++_drm_init_count != 1) return _drm_init_count;

   if (!ecore_drm2_init())
     {
        ERR("Failed to init Ecore_Drm2 library");
        goto init_err;
     }

   if (!device) device = "seat0";

   edata->dev = ecore_drm2_device_open(device, 0);
   if (!edata->dev)
     {
        ERR("Failed to open drm device");
        goto open_err;
     }

   if (!ecore_drm2_outputs_create(edata->dev))
     {
        ERR("Failed to create drm outputs");
        goto output_err;
     }

   ecore_event_evas_init();

   if (!devhdlr)
     {
        devhdlr =
          ecore_event_handler_add(ELPUT_EVENT_DEVICE_CHANGE,
                                  _cb_device_change, NULL);
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
_drm_shutdown(Ecore_Evas_Engine_Drm_Data *edata)
{
   if (--_drm_init_count != 0) return _drm_init_count;

   ecore_event_handler_del(devhdlr);

   ecore_event_evas_shutdown();
   ecore_drm2_outputs_destroy(edata->dev);
   ecore_drm2_device_close(edata->dev);
   ecore_drm2_shutdown();

   return _drm_init_count;
}

static Ecore_Evas_Interface_Drm *
_drm_interface_new(void)
{
   Ecore_Evas_Interface_Drm *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Drm));
   if (!iface) return NULL;

   iface->base.name = "drm";
   iface->base.version = 1;

   return iface;
}

static void
_ecore_evas_drm_canvas_setup(Ecore_Evas *ee, Ecore_Evas_Engine_Drm_Data *edata)
{
   Ecore_Drm2_Output *output;
   Eina_List *outs, *l;

   outs = (Eina_List *)ecore_drm2_outputs_get(edata->dev);
   EINA_LIST_FOREACH(outs, l, output)
     {
        Efl_Canvas_Output *eout;
        Evas_Engine_Info_Drm *einfo;
        Ecore_Evas_Engine_Drm_Tick *etick;
        int ox, oy, ow, oh;

        if (!ecore_drm2_output_connected_get(output)) continue;
        if (!ecore_drm2_output_enabled_get(output)) continue;

        eout = efl_canvas_output_add(ee->evas);
        if (!eout) continue;

        /* efl_canvas_output_lock(eout); */

        einfo = (Evas_Engine_Info_Drm *)efl_canvas_output_engine_info_get(eout);
        if (!einfo)
          {
             efl_canvas_output_del(eout);
             continue;
          }

        einfo->info.dev = edata->dev;
        einfo->info.bpp = 32;
        einfo->info.depth = 24;
        einfo->info.alpha = ee->alpha;
        einfo->info.rotation = ee->rotation;
        einfo->info.format = DRM_FORMAT_XRGB8888;
        einfo->info.output = output;

        ecore_drm2_output_info_get(output, &ox, &oy, &ow, &oh, NULL);

        fprintf(stderr, "Ecore_Evas_Drm: Adding Canvas for Output %s At %d %d %d %d\n",
                ecore_drm2_output_name_get(output), ox, oy, ow, oh);

        efl_canvas_output_view_set(eout, ox, oy, ow, oh);
        efl_canvas_output_engine_info_set(eout, (Evas_Engine_Info *)einfo);

        ecore_drm2_output_user_data_set(output, ee);

        if (ecore_drm2_output_primary_get(output))
          {
             ee->prop.window = ecore_drm2_output_crtc_get(output);
             ecore_drm2_device_window_set(edata->dev, ee->prop.window);
          }

        outputs = eina_list_append(outputs, output);

        etick = calloc(1, sizeof(Ecore_Evas_Engine_Drm_Tick));
        if (!etick) continue;

        etick->canvas = eout;
        etick->output = output;
        if (edata->once) etick->offset = 0.0;

        edata->ticks = eina_list_append(edata->ticks, etick);
     }
}

static Ecore_Evas *
_ecore_evas_new_internal(const char *device, int x, int y, int w, int h, Eina_Bool gl)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Drm_Data *edata;
   int method = 0;

   if (gl)
     method = evas_render_method_lookup("gl_drm");
   else
     method = evas_render_method_lookup("drm");

   if (!method)
     {
        ERR("Could not find evas render method");
        return NULL;
     }

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   edata = calloc(1, sizeof(Ecore_Evas_Engine_Drm_Data));
   if (!edata) goto edata_err;

   if (_drm_init(edata, device) < 1)
     goto init_err;

   if (!getenv("ECORE_EVAS_DRM_GPU_CLOCK_WRONG"))
     edata->once = EINA_TRUE;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   if (gl)
     ee->driver = "gl_drm";
   else
     ee->driver = "drm";

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_drm_engine_func;
   ee->engine.data = edata;
   ee->engine.ifaces =
     eina_list_append(ee->engine.ifaces, _drm_interface_new());

   ee->x = ee->req.x = x;
   ee->y = ee->req.y = y;
   ee->w = ee->req.w = w;
   ee->h = ee->req.h = h;
   ee->alpha = EINA_FALSE;

   ee->prop.layer = 4;
   ee->prop.max.w = ee->prop.max.h = 32767;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;
   ee->prop.withdrawn = EINA_TRUE;

   ee->can_async_render = !gl;
   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = EINA_FALSE;

   if (!ecore_evas_evas_new(ee, w, h))
     {
        ERR("Can not create a Canvas.");
>>>>>>> ecore-evas-drm: Refactor ecore-evas drm engine to work with multiple outputs
        goto eng_err;
     }

   evas_output_method_set(ee->evas, method);

   /* TODO */
   /* if (ee->can_async_render) */
   /*   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST, */
   /*                           _cb_render_updates, ee); */

   /* TODO: setup efl_canvas outputs */
   if (!gl)
     _ecore_evas_drm_canvas_setup(ee, edata);

   ecore_evas_data_set(ee, "device", edata->dev);

   ecore_evas_done(ee, EINA_FALSE);

   memset(&edata->ctx, 0, sizeof(Ecore_Drm2_Context));
   edata->ctx.page_flip_handler = _cb_pageflip;

   edata->hdlr =
     ecore_main_fd_handler_add(ecore_drm2_device_fd_get(edata->dev),
                               ECORE_FD_READ, _cb_drm_event, ee, NULL, NULL);

   canvases = eina_list_append(canvases, ee);

   return ee;

init_err:
   free(edata);
edata_err:
   free(ee);
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
