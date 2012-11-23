#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef alloca
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
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

#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
# include "ecore_evas_private.h"
# include <Evas_Engine_Wayland_Egl.h>
# include <Ecore_Wayland.h>

/* local function prototypes */
static void _ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static void _ecore_evas_wl_hide(Ecore_Evas *ee);
static void _ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha);
static void _ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent);

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
   _ecore_evas_wl_common_move,
   NULL, // managed_move
   _ecore_evas_wl_resize,
   NULL, // move_resize
   NULL, // rotation_set
   NULL, // shaped_set
   _ecore_evas_wl_show,
   _ecore_evas_wl_hide,
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
   _ecore_evas_wl_common_layer_set,
   NULL, // focus set
   _ecore_evas_wl_common_iconified_set,
   NULL, // borderless set
   NULL, // override set
   _ecore_evas_wl_common_maximized_set,
   _ecore_evas_wl_common_fullscreen_set,
   NULL, // func avoid_damage set
   NULL, // func withdrawn set
   NULL, // func sticky set
   _ecore_evas_wl_common_ignore_events_set,
   _ecore_evas_wl_alpha_set,
   _ecore_evas_wl_transparent_set,
   NULL, // func profiles set
   NULL, // window group set
   NULL, // aspect set
   NULL, // urgent set
   NULL, // modal set
   NULL, // demand attention set
   NULL, // focus skip set
   _ecore_evas_wl_common_render,
   _ecore_evas_wl_common_screen_geometry_get,
   _ecore_evas_wl_common_screen_dpi_get
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
        goto ee_err;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_wl_common_init();

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
             goto err;
          }
     }
   else 
     {
        ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
        goto err;
     }

   ecore_evas_callback_pre_free_set(ee, _ecore_evas_wl_common_pre_free);

   if (ee->prop.draw_frame) 
     {
        ee->engine.wl.frame = _ecore_evas_wl_common_frame_add(ee->evas);
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

   return ee;

 err:
   ecore_evas_free(ee);
   _ecore_evas_wl_common_shutdown();

 ee_err:
   ecore_wl_shutdown();
   return NULL;
}

static void 
_ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->req.w = w;
   ee->req.h = h;

   if (!ee->prop.fullscreen)
     {
        int fw = 0, fh = 0;

        if (ee->prop.min.w > w) w = ee->prop.min.w;
        else if (w > ee->prop.max.w) w = ee->prop.max.w;
        if (ee->prop.min.h > h) h = ee->prop.min.h;
        else if (h > ee->prop.max.h) h = ee->prop.max.h;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
        w += fw;
        h += fh;
     }

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
#else
EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name __UNUSED__, unsigned int parent __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__, Eina_Bool frame __UNUSED__)
{
   return NULL;
}
#endif
