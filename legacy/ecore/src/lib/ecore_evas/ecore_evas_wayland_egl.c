#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) \
   printf("-ECORE_EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/mman.h>

# include <Eina.h>
# include <Evas.h>
# include <Ecore.h>

# include "ecore_evas_private.h"
# include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
# include <Evas_Engine_Wayland_Egl.h>
# include <Ecore_Wayland.h>
#endif

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
/* local function prototypes */
static int _ecore_evas_wl_init(Ecore_Evas *ee);
static int _ecore_evas_wl_shutdown(void);
static void _ecore_evas_wl_free(Ecore_Evas *ee);
static void _ecore_evas_wl_move(Ecore_Evas *ee, int x, int y);
static void _ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static int _ecore_evas_wl_render(Ecore_Evas *ee);

/* local variables */
static int _ecore_evas_init_count = 0;

static Ecore_Evas_Engine_Func _ecore_wl_engine_func = 
{
   _ecore_evas_wl_free, 
   NULL, // _ecore_evas_wl_callback_resize_set, 
   NULL, // _ecore_evas_wl_callback_move_set, 
   NULL, // callback show set
   NULL, // callback hide set
   NULL, // _ecore_evas_wl_callback_delete_request_set, 
   NULL, // callback destroy set
   NULL, // _ecore_evas_wl_callback_focus_in_set, 
   NULL, // _ecore_evas_wl_callback_focus_out_set, 
   NULL, // callback mouse in set
   NULL, // callback mouse out set
   NULL, // callback sticky set
   NULL, // callback unsticky set
   NULL, // callback pre render set
   NULL, // callback post render set
   _ecore_evas_wl_move, 
   NULL, // func managed move
   _ecore_evas_wl_resize, 
   NULL, // _ecore_evas_wl_move_resize, 
   NULL, // func rotation set
   NULL, // func shaped set
   _ecore_evas_wl_show, 
   NULL, // _ecore_evas_wl_hide, 
   NULL, // _ecore_evas_wl_raise, 
   NULL, // _ecore_evas_wl_lower, 
   NULL, // _ecore_evas_wl_activate, 
   NULL, // _ecore_evas_wl_title_set, 
   NULL, // _ecore_evas_wl_name_class_set, 
   NULL, // _ecore_evas_wl_size_min_set, 
   NULL, // _ecore_evas_wl_size_max_set, 
   NULL, // _ecore_evas_wl_size_base_set, 
   NULL, // _ecore_evas_wl_size_step_set, 
   NULL, // _ecore_evas_wl_object_cursor_set, 
   NULL, // _ecore_evas_wl_layer_set, 
   NULL, // _ecore_evas_wl_focus_set, 
   NULL, // func iconified set
   NULL, // func borderless set
   NULL, // func override set
   NULL, // func maximized set
   NULL, // func fullscreen set
   NULL, // _ecore_evas_wl_avoid_damage_set, 
   NULL, // func withdrawn set
   NULL, // func sticky set
   NULL, // func ignore events set
   NULL, // func alpha set
   NULL, // func transparent set
   _ecore_evas_wl_render, 
   NULL // _ecore_evas_wl_screen_geometry_get
};

#endif

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name, int x, int y, int w, int h, int frame) 
{
   Evas_Engine_Info_Wayland_Egl *einfo;
   Ecore_Evas *ee;
   int method = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(method = evas_render_method_lookup("wayland_egl"))) 
     {
        ERR("Render method lookup failed for Wayland Egl");
        return NULL;
     }

   if (!ecore_wl_init(disp_name)) 
     {
        ERR("Failed to initialize Ecore Wayland");
        return NULL;
     }

   if (!(ee = calloc(1, sizeof(Ecore_Evas)))) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_wl_init(ee);

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
   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->prop.draw_frame = frame;
   ee->rotation = 0;

   ee->engine.wl.win = 
     ecore_wl_window_new(ECORE_WL_WINDOW_TYPE_EGL, x, y, w, h);
   ee->prop.window = ee->engine.wl.win->id;

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->prop.draw_frame) evas_output_framespace_set(ee->evas, 4, 18, 8, 22);

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas))) 
     {
        einfo->info.disp = ecore_wl_display_get();
        einfo->info.comp = ecore_wl_compositor_get();
        einfo->info.shell = ecore_wl_shell_get();
        einfo->info.rotation = ee->rotation;
        einfo->info.debug = EINA_FALSE;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo)) 
          {
             ERR("Failed to set Evas Engine Info for '%s'.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else 
     {
        ERR("Failed to get Evas Engine Info for '%s'.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }

   ecore_evas_input_event_register(ee);
   _ecore_evas_register(ee);

   ecore_event_window_register(ee->prop.window, ee, ee->evas, 
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process, 
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process, 
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process, 
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);

   return ee;
}
#else
EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__, int frame __UNUSED__) 
{
   return NULL;
}
#endif

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
/* local functions */
static int 
_ecore_evas_wl_init(Ecore_Evas *ee) 
{
   _ecore_evas_init_count++;

   /* TODO: Add handlers */

   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;
   ecore_event_evas_init();

   return _ecore_evas_init_count;
}

static int 
_ecore_evas_wl_shutdown(void) 
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0) 
     {
        /* TODO: Delete handlers */
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void 
_ecore_evas_wl_free(Ecore_Evas *ee) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee) 
     {
        if (ee->engine.wl.win) ecore_wl_window_free(ee->engine.wl.win);
        ecore_event_window_unregister(ee->prop.window);
        ecore_evas_input_event_unregister(ee);
     }
   _ecore_evas_wl_shutdown();
   ecore_wl_shutdown();
}

static void 
_ecore_evas_wl_move(Ecore_Evas *ee, int x, int y) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->req.x = x;
   ee->req.y = y;
   if ((ee->x == x) && (ee->y == y)) return;
   ee->x = x;
   ee->y = y;
   /* TODO: Actually move this window */
   if (!ee->should_be_visible) ee->prop.request_pos = 1;
   if (ee->func.fn_move) ee->func.fn_move(ee);
}

static void 
_ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->req.w = w;
   ee->req.h = h;
   if ((ee->w == w) && (ee->h == h)) return;

   ee->w = w;
   ee->h = h;
   ecore_wl_window_resize(ee->engine.wl.win, w, h);

   if ((ee->rotation == 90) || (ee->rotation == 270)) 
     {
        evas_output_size_set(ee->evas, h, w);
        evas_output_viewport_set(ee->evas, 0, 0, h, w);
        evas_damage_rectangle_add(ee->evas, 0, 0, h, w);
     }
   else 
     {
        evas_output_size_set(ee->evas, w, h);
        evas_output_viewport_set(ee->evas, 0, 0, w, h);
        evas_damage_rectangle_add(ee->evas, 0, 0, w, h);
     }

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void 
_ecore_evas_wl_show(Ecore_Evas *ee) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->visible = 1;
   ee->should_be_visible = 1;

   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static int 
_ecore_evas_wl_render(Ecore_Evas *ee) 
{
   int rend = 0;

   if (!ee) return 0;
   if (ee->visible) 
     {
        Eina_List *updates = NULL, *ll = NULL;
        Ecore_Evas *ee2;

        EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2) 
          {
             if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
             if (ee2->engine.func->fn_render)
               rend |= ee2->engine.func->fn_render(ee2);
             if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
          }

        if ((updates = evas_render_updates(ee->evas))) 
          {
             if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

             evas_render_updates_free(updates);
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;

             if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
          }
        else
          evas_norender(ee->evas);
     }
   else
     evas_norender(ee->evas);

   return rend;
}

#endif
