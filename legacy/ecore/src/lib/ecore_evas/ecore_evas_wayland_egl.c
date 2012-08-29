#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//#define LOGFNS 1

#ifdef LOGFNS
# include <stdio.h>
# define LOGFN(fl, ln, fn) \
   printf("-ECORE_EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
#else
# define LOGFN(fl, ln, fn)
#endif

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/mman.h>
#endif

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
# include <Evas_Engine_Wayland_Egl.h>
# include <Ecore_Wayland.h>

/* local structures */
typedef struct _EE_Wl_Smart_Data EE_Wl_Smart_Data;
struct _EE_Wl_Smart_Data 
{
   Evas_Object *frame;
   Evas_Object *text;
   Evas_Coord x, y, w, h;
};

/* local function prototypes */
static int _ecore_evas_wl_init(void);
static int _ecore_evas_wl_shutdown(void);
static void _ecore_evas_wl_pre_free(Ecore_Evas *ee);
static void _ecore_evas_wl_free(Ecore_Evas *ee);
static void _ecore_evas_wl_callback_resize_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_callback_move_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_callback_delete_request_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_callback_focus_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_callback_focus_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_callback_mouse_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_callback_mouse_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee));
static void _ecore_evas_wl_move(Ecore_Evas *ee, int x, int y);
static void _ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static void _ecore_evas_wl_hide(Ecore_Evas *ee);
static void _ecore_evas_wl_raise(Ecore_Evas *ee);
static void _ecore_evas_wl_title_set(Ecore_Evas *ee, const char *title);
static void _ecore_evas_wl_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
static void _ecore_evas_wl_size_min_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_size_max_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_size_base_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_size_step_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_layer_set(Ecore_Evas *ee, int layer);
static void _ecore_evas_wl_iconified_set(Ecore_Evas *ee, int iconify);
static void _ecore_evas_wl_maximized_set(Ecore_Evas *ee, int max);
static void _ecore_evas_wl_fullscreen_set(Ecore_Evas *ee, int full);
static void _ecore_evas_wl_ignore_events_set(Ecore_Evas *ee, int ignore);
static void _ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha);
static void _ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent);
static int _ecore_evas_wl_render(Ecore_Evas *ee);
static void _ecore_evas_wl_screen_geometry_get(const Ecore_Evas *ee __UNUSED__, int *x, int *y, int *w, int *h);
static void _ecore_evas_wl_screen_dpi_get(const Ecore_Evas *ee __UNUSED__, int *xdpi, int *ydpi);
static Eina_Bool _ecore_evas_wl_cb_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_cb_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_cb_focus_in(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_cb_focus_out(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_cb_window_configure(void *data __UNUSED__, int type __UNUSED__, void *event);

/* SMART stuff for frame */
static Evas_Smart *_ecore_evas_wl_smart = NULL;

static void _ecore_evas_wl_smart_init(void);
static void _ecore_evas_wl_smart_add(Evas_Object *obj);
static void _ecore_evas_wl_smart_del(Evas_Object *obj);
static void _ecore_evas_wl_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _ecore_evas_wl_smart_show(Evas_Object *obj);
static void _ecore_evas_wl_smart_hide(Evas_Object *obj);

static Evas_Object *_ecore_evas_wl_frame_add(Evas *evas);

/* local variables */
static int _ecore_evas_wl_init_count = 0;
static Ecore_Event_Handler *_ecore_evas_wl_event_hdls[5];

static Ecore_Evas_Engine_Func _ecore_wl_engine_func = 
{
   _ecore_evas_wl_free,
   _ecore_evas_wl_callback_resize_set,
   _ecore_evas_wl_callback_move_set,
   NULL, 
   NULL,
   _ecore_evas_wl_callback_delete_request_set,
   NULL,
   _ecore_evas_wl_callback_focus_in_set,
   _ecore_evas_wl_callback_focus_out_set,
   _ecore_evas_wl_callback_mouse_in_set,
   _ecore_evas_wl_callback_mouse_out_set,
   NULL, // sticky_set
   NULL, // unsticky_set
   NULL, // pre_render_set
   NULL, // post_render_set
   _ecore_evas_wl_move,
   NULL, // managed_move
   _ecore_evas_wl_resize,
   NULL, // move_resize
   NULL, // rotation_set
   NULL, // shaped_set
   _ecore_evas_wl_show,
   _ecore_evas_wl_hide,
   _ecore_evas_wl_raise,
   NULL, // lower
   NULL, // activate
   _ecore_evas_wl_title_set,
   _ecore_evas_wl_name_class_set,
   _ecore_evas_wl_size_min_set,
   _ecore_evas_wl_size_max_set,
   _ecore_evas_wl_size_base_set,
   _ecore_evas_wl_size_step_set,
   NULL, // object_cursor_set
   _ecore_evas_wl_layer_set,
   NULL, // focus set
   _ecore_evas_wl_iconified_set,
   NULL, // borderless set
   NULL, // override set
   _ecore_evas_wl_maximized_set,
   _ecore_evas_wl_fullscreen_set,
   NULL, // func avoid_damage set
   NULL, // func withdrawn set
   NULL, // func sticky set
   _ecore_evas_wl_ignore_events_set,
   _ecore_evas_wl_alpha_set,
   _ecore_evas_wl_transparent_set,
   NULL, // func profiles set
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_wl_render, 
   _ecore_evas_wl_screen_geometry_get,
   _ecore_evas_wl_screen_dpi_get
};

/* external variables */

/* external functions */
EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Wl_Window *p = NULL;
   Evas_Engine_Info_Wayland_Egl *einfo;
   Ecore_Evas *ee;
   int method = 0, count = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(method = evas_render_method_lookup("wayland_egl")))
     {
        ERR("Render method lookup failed for Wayland_Egl");
        return NULL;
     }

   count = ecore_wl_init(disp_name);
   if (!count)
     {
        ERR("Failed to initialize Ecore_Wayland");
        return NULL;
     }
   else if (count == 1)
     ecore_wl_display_iterate();

   if (!(ee = calloc(1, sizeof(Ecore_Evas))))
     {
        ERR("Failed to allocate Ecore_Evas");
        ecore_wl_shutdown();
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_wl_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_wl_engine_func;

   ee->driver = "wayland_egl";
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->x = x;
   ee->y = y;
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
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->prop.draw_frame = frame;
   ee->alpha = EINA_FALSE;

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);

   /* FIXME: This needs to be set based on theme & scale */
   if (ee->prop.draw_frame)
     evas_output_framespace_set(ee->evas, 4, 18, 8, 22);

   if (parent)
     p = ecore_wl_window_find(parent);

   /* FIXME: Get if parent is alpha, and set */

   ee->engine.wl.parent = p;
   ee->engine.wl.win = 
     ecore_wl_window_new(p, x, y, w, h, ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW);
   ee->prop.window = ee->engine.wl.win->id;

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.display = ecore_wl_display_get();
        einfo->info.destination_alpha = ee->alpha;
        einfo->info.rotation = ee->rotation;
        einfo->info.depth = 32;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("Failed to set Evas Engine Info for '%s'", ee->driver);
             ecore_evas_free(ee);
             _ecore_evas_wl_shutdown();
             ecore_wl_shutdown();
             return NULL;
          }
     }
   else 
     {
        ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
        ecore_evas_free(ee);
        _ecore_evas_wl_shutdown();
        ecore_wl_shutdown();
        return NULL;
     }

   ecore_evas_callback_pre_free_set(ee, _ecore_evas_wl_pre_free);

   if (ee->prop.draw_frame) 
     {
        ee->engine.wl.frame = _ecore_evas_wl_frame_add(ee->evas);
        evas_object_is_frame_object_set(ee->engine.wl.frame, EINA_TRUE);
        evas_object_move(ee->engine.wl.frame, 0, 0);
     }

   _ecore_evas_register(ee);
   ecore_evas_input_event_register(ee);

   ecore_event_window_register(ee->prop.window, ee, ee->evas, 
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process, 
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process, 
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process, 
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   /* evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL); */

   return ee;
}

/* local functions */
static int 
_ecore_evas_wl_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_ecore_evas_wl_init_count != 1)
     return _ecore_evas_wl_init_count;

   _ecore_evas_wl_event_hdls[0] = 
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_IN, 
                             _ecore_evas_wl_cb_mouse_in, NULL);
   _ecore_evas_wl_event_hdls[1] = 
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_OUT, 
                             _ecore_evas_wl_cb_mouse_out, NULL);
   _ecore_evas_wl_event_hdls[2] = 
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_IN, 
                             _ecore_evas_wl_cb_focus_in, NULL);
   _ecore_evas_wl_event_hdls[3] = 
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_OUT, 
                             _ecore_evas_wl_cb_focus_out, NULL);
   _ecore_evas_wl_event_hdls[4] = 
     ecore_event_handler_add(ECORE_WL_EVENT_WINDOW_CONFIGURE, 
                             _ecore_evas_wl_cb_window_configure, NULL);

   ecore_event_evas_init();

   return _ecore_evas_wl_init_count;
}

static int 
_ecore_evas_wl_shutdown(void)
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

static void 
_ecore_evas_wl_pre_free(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->engine.wl.frame) evas_object_del(ee->engine.wl.frame);
}

static void 
_ecore_evas_wl_free(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->engine.wl.win) ecore_wl_window_free(ee->engine.wl.win);
   ee->engine.wl.win = NULL;

   ecore_event_window_unregister(ee->prop.window);
   ecore_evas_input_event_unregister(ee);

   _ecore_evas_wl_shutdown();
   ecore_wl_shutdown();
}

static void 
_ecore_evas_wl_callback_resize_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_resize = func;
}

static void 
_ecore_evas_wl_callback_move_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_move = func;
}

static void 
_ecore_evas_wl_callback_delete_request_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_delete_request = func;
}

static void 
_ecore_evas_wl_callback_focus_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_focus_in = func;
}

static void 
_ecore_evas_wl_callback_focus_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_focus_out = func;
}

static void 
_ecore_evas_wl_callback_mouse_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_mouse_in = func;
}

static void 
_ecore_evas_wl_callback_mouse_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->func.fn_mouse_out = func;
}

static void 
_ecore_evas_wl_move(Ecore_Evas *ee, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->req.x = x;
   ee->req.y = y;
   if ((ee->x != x) || (ee->y != y))
     {
        ee->x = x;
        ee->y = y;
        if (ee->engine.wl.win) 
          ecore_wl_window_update_location(ee->engine.wl.win, x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

static void 
_ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   int fw = 0, fh = 0;

   if (!ee) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->req.w = w;
   ee->req.h = h;

   if (!ee->prop.fullscreen)
     {
        if (ee->prop.min.w > w) w = ee->prop.min.w;
        else if (w > ee->prop.max.w) w = ee->prop.max.w;
        if (ee->prop.min.h > h) h = ee->prop.min.h;
        else if (h > ee->prop.max.h) h = ee->prop.max.h;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
        w += fw;
        h += fh;
     }

//   ecore_wl_window_damage(ee->engine.wl.win, 0, 0, ee->w, ee->h);

   if ((ee->w != w) || (ee->h != h))
     {
        ee->w = w;
        ee->h = h;

        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, h, w);
             evas_output_viewport_set(ee->evas, 0, 0, h, w);
          }
        else 
          {
             evas_output_size_set(ee->evas, w, h);
             evas_output_viewport_set(ee->evas, 0, 0, w, h);
          }

        if (ee->prop.avoid_damage)
          {
             int pdam = 0;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }

        if (ee->engine.wl.frame)
          evas_object_resize(ee->engine.wl.frame, w, h);

        /* set new engine destination */
        /* evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo); */

        /* ecore_wl_window_damage(ee->engine.wl.win, 0, 0, ee->w, ee->h); */

        // WAS ACTIVE
        /* ecore_wl_flush(); */

        if (ee->engine.wl.win)
          {
             ecore_wl_window_update_size(ee->engine.wl.win, w, h);
             ecore_wl_window_buffer_attach(ee->engine.wl.win, NULL, 0, 0);
          }

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void 
_ecore_evas_wl_show(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (ee->visible)) return;

   if (ee->engine.wl.win)
     {
        ecore_wl_window_show(ee->engine.wl.win);
        ecore_wl_window_update_size(ee->engine.wl.win, ee->w, ee->h);
        ecore_wl_window_buffer_attach(ee->engine.wl.win, NULL, 0, 0);

        if ((ee->prop.clas) && (ee->engine.wl.win->shell_surface))
          wl_shell_surface_set_class(ee->engine.wl.win->shell_surface, 
                                     ee->prop.clas);
        if ((ee->prop.title) && (ee->engine.wl.win->shell_surface))
          wl_shell_surface_set_title(ee->engine.wl.win->shell_surface, 
                                     ee->prop.title);
     }

   if (ee->engine.wl.frame)
     {
        evas_object_show(ee->engine.wl.frame);
        evas_object_resize(ee->engine.wl.frame, ee->w, ee->h);
     }

   if (ee->engine.wl.win)
     {
        einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas);
        if (!einfo)
          {
             ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
             return;
          }

        einfo->info.surface = ecore_wl_window_surface_get(ee->engine.wl.win);
        /* if (einfo->info.surface) */
        evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
        /* else */
        /*   printf("Failed to get a Surface from Ecore_Wl\n"); */
     }

   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void 
_ecore_evas_wl_hide(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;

   einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.surface = NULL;
        evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
     }

   if (ee->engine.wl.win) 
     ecore_wl_window_hide(ee->engine.wl.win);

   ee->visible = 0;
   ee->should_be_visible = 0;

   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void 
_ecore_evas_wl_raise(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   ecore_wl_window_raise(ee->engine.wl.win);
}

static void 
_ecore_evas_wl_title_set(Ecore_Evas *ee, const char *title)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   if ((ee->prop.draw_frame) && (ee->engine.wl.frame)) 
     {
        EE_Wl_Smart_Data *sd;

        if (!(sd = evas_object_smart_data_get(ee->engine.wl.frame))) return;
        evas_object_text_text_set(sd->text, ee->prop.title);
     }

   if ((ee->prop.title) && (ee->engine.wl.win->shell_surface))
     wl_shell_surface_set_title(ee->engine.wl.win->shell_surface, 
                                ee->prop.title);
}

static void 
_ecore_evas_wl_name_class_set(Ecore_Evas *ee, const char *n, const char *c) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   if (n) ee->prop.name = strdup(n);
   if (c) ee->prop.clas = strdup(c);

   if ((ee->prop.clas) && (ee->engine.wl.win->shell_surface))
     wl_shell_surface_set_class(ee->engine.wl.win->shell_surface, 
                                ee->prop.clas);
}

static void 
_ecore_evas_wl_size_min_set(Ecore_Evas *ee, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
}

static void 
_ecore_evas_wl_size_max_set(Ecore_Evas *ee, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
}

static void 
_ecore_evas_wl_size_base_set(Ecore_Evas *ee, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
}

static void 
_ecore_evas_wl_size_step_set(Ecore_Evas *ee, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
}

static void 
_ecore_evas_wl_layer_set(Ecore_Evas *ee, int layer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.layer == layer) return;
   if (layer < 1) layer = 1;
   else if (layer > 255) layer = 255;
   ee->prop.layer = layer;
}

static void 
_ecore_evas_wl_iconified_set(Ecore_Evas *ee, int iconify)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.iconified == iconify) return;
   ee->prop.iconified = iconify;
   /* FIXME: Implement this in Wayland someshow */
}

static void 
_ecore_evas_wl_maximized_set(Ecore_Evas *ee, int max)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.maximized == max) return;
   ee->prop.maximized = max;
   ecore_wl_window_maximized_set(ee->engine.wl.win, max);
}

static void 
_ecore_evas_wl_fullscreen_set(Ecore_Evas *ee, int full)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.fullscreen == full) return;
   ee->prop.fullscreen = full;
   ecore_wl_window_fullscreen_set(ee->engine.wl.win, full);
}

static void 
_ecore_evas_wl_ignore_events_set(Ecore_Evas *ee, int ignore)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->ignore_events = ignore;
   /* NB: Hmmm, may need to pass this to ecore_wl_window in the future */
}

static void 
_ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->alpha == alpha)) return;
   ee->alpha = alpha;
   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = alpha;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
}

static void 
_ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->transparent == transparent)) return;
   ee->transparent = transparent;
   if (!ee->visible) return;
   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = transparent;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
}

static int 
_ecore_evas_wl_render(Ecore_Evas *ee)
{
   int rend = 0;

   if (!ee) return 0;
   if (!ee->visible)
     evas_norender(ee->evas);
   else
     {
        Eina_List *ll = NULL, *updates = NULL;
        Ecore_Evas *ee2 = NULL;

        if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

        EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2) 
          {
             if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
             if (ee2->engine.func->fn_render)
               rend |= ee2->engine.func->fn_render(ee2);
             if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
          }

        if ((updates = evas_render_updates(ee->evas))) 
          {
             Eina_List *l = NULL;
             Eina_Rectangle *r;

             LOGFN(__FILE__, __LINE__, __FUNCTION__);

             EINA_LIST_FOREACH(updates, l, r) 
               ecore_wl_window_damage(ee->engine.wl.win, 
                                      r->x, r->y, r->w, r->h);

             ecore_wl_flush();

             evas_render_updates_free(updates);
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }

        if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
     }
   return rend;
}

static void 
_ecore_evas_wl_screen_geometry_get(const Ecore_Evas *ee __UNUSED__, int *x, int *y, int *w, int *h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   ecore_wl_screen_size_get(w, h);
}

static void 
_ecore_evas_wl_screen_dpi_get(const Ecore_Evas *ee __UNUSED__, int *xdpi, int *ydpi)
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

void 
_ecore_evas_wayland_egl_resize(Ecore_Evas *ee, int location)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->engine.wl.win) 
     {
        Evas_Engine_Info_Wayland_Egl *einfo;

        if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
          {
             einfo->info.edges = location;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }

        ee->engine.wl.win->resizing = EINA_TRUE;
        ecore_wl_window_resize(ee->engine.wl.win, ee->w, ee->h, location);
     }
}

void 
_ecore_evas_wayland_egl_move(Ecore_Evas *ee, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->engine.wl.win) 
     {
        ee->engine.wl.win->moving = EINA_TRUE;
        ecore_wl_window_move(ee->engine.wl.win, x, y);
     }
}

static Eina_Bool 
_ecore_evas_wl_cb_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->timestamp);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_cb_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->timestamp);
   evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_cb_focus_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_cb_focus_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   evas_focus_out(ee->evas);
   ee->prop.focused = 0;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_cb_window_configure(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Window_Configure *ev;
   int nw = 0, nh = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   if (ee->prop.fullscreen)
     {
        _ecore_evas_wl_move(ee, ev->x, ev->y);
        _ecore_evas_wl_resize(ee, ev->w, ev->h);

        return ECORE_CALLBACK_PASS_ON;
     }

   if ((ee->x != ev->x) || (ee->y != ev->y))
     {
        ee->req.x = ee->x;
        ee->req.y = ee->y;
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }

   nw = ev->w;
   nh = ev->h;

   if ((ee->prop.maximized) || (!ee->prop.fullscreen))
     {
        int fw = 0, fh = 0;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
        nw = ev->w - fw;
        nh = ev->h - fh;
     }

   if (ee->prop.min.w > nw) nw = ee->prop.min.w;
   else if (nw > ee->prop.max.w) nw = ee->prop.max.w;
   if (ee->prop.min.h > nh) nh = ee->prop.min.h;
   else if (nh > ee->prop.max.h) nh = ee->prop.max.h;

   if ((ee->w != nw) || (ee->h != nh))
     {
        ee->req.w = nw;
        ee->req.h = nh;
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void 
_ecore_evas_wl_smart_init(void) 
{
   if (_ecore_evas_wl_smart) return;
     {
        static const Evas_Smart_Class sc = 
          {
             "ecore_evas_wl_frame", EVAS_SMART_CLASS_VERSION, 
             _ecore_evas_wl_smart_add, 
             _ecore_evas_wl_smart_del, 
             NULL, 
             _ecore_evas_wl_smart_resize, 
             _ecore_evas_wl_smart_show, 
             _ecore_evas_wl_smart_hide, 
             NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
          };
        _ecore_evas_wl_smart = evas_smart_class_new(&sc);
     }
}

static void 
_ecore_evas_wl_smart_add(Evas_Object *obj) 
{
   EE_Wl_Smart_Data *sd;
   Evas *evas;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = calloc(1, sizeof(EE_Wl_Smart_Data)))) return;

   evas = evas_object_evas_get(obj);

   sd->x = 0;
   sd->y = 0;
   sd->w = 1;
   sd->h = 1;

   sd->frame = evas_object_rectangle_add(evas);
   evas_object_is_frame_object_set(sd->frame, EINA_TRUE);
   evas_object_color_set(sd->frame, 249, 249, 249, 255);
   evas_object_smart_member_add(sd->frame, obj);

   sd->text = evas_object_text_add(evas);
   evas_object_color_set(sd->text, 0, 0, 0, 255);
   evas_object_text_style_set(sd->text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(sd->text, "Sans", 10);
   evas_object_text_text_set(sd->text, "Smart Test");

   evas_object_smart_data_set(obj, sd);
}

static void 
_ecore_evas_wl_smart_del(Evas_Object *obj) 
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_del(sd->text);
   evas_object_del(sd->frame);
   free(sd);
}

static void 
_ecore_evas_wl_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h) 
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   evas_object_resize(sd->frame, w, h);
}

static void 
_ecore_evas_wl_smart_show(Evas_Object *obj) 
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_show(sd->frame);
   evas_object_show(sd->text);
}

static void 
_ecore_evas_wl_smart_hide(Evas_Object *obj) 
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_hide(sd->text);
   evas_object_hide(sd->frame);
}

static Evas_Object *
_ecore_evas_wl_frame_add(Evas *evas) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_evas_wl_smart_init();
   return evas_object_smart_add(evas, _ecore_evas_wl_smart);
}

#else
EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name __UNUSED__, unsigned int parent __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__, Eina_Bool frame __UNUSED__)
{
   return NULL;
}
#endif
