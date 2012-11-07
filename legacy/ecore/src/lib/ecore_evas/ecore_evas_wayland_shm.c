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

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/mman.h>
#endif

#include <Eina.h>

#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
# include "ecore_evas_private.h"
# include <Evas_Engine_Wayland_Shm.h>
# include <Ecore_Wayland.h>

/* local function prototypes */
static void _ecore_evas_wl_free(Ecore_Evas *ee);
static void _ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static void _ecore_evas_wl_hide(Ecore_Evas *ee);
static void _ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha);
static void _ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent);
static int  _ecore_evas_wl_render(Ecore_Evas *ee);

/* SHM Only */
static void _ecore_evas_wl_shm_pool_free(Ecore_Evas *ee);
static void _ecore_evas_wl_shm_pool_create(Ecore_Evas *ee, size_t size);
static void _ecore_evas_wl_buffer_free(Ecore_Evas *ee);
static void _ecore_evas_wl_buffer_new(Ecore_Evas *ee, int w, int h);

/* Frame listener */
static void _ecore_evas_wl_frame_complete(void *data, struct wl_callback *callback, uint32_t time);
static const struct wl_callback_listener frame_listener =
{
   _ecore_evas_wl_frame_complete,
};

static Ecore_Evas_Engine_Func _ecore_wl_engine_func = 
{
   _ecore_evas_wl_free,
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
   _ecore_evas_wl_move_resize,
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
   _ecore_evas_wl_common_object_cursor_set, // object_cursor_set
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
   _ecore_evas_wl_render,
   _ecore_evas_wl_common_screen_geometry_get,
   _ecore_evas_wl_common_screen_dpi_get
};

/* external variables */

/* external functions */
EAPI Ecore_Evas *
ecore_evas_wayland_shm_new(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Wl_Window *p = NULL;
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas *ee;
   int method = 0, count = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(method = evas_render_method_lookup("wayland_shm")))
     {
        ERR("Render method lookup failed for Wayland_Shm");
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

   ee->driver = "wayland_shm";
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
     ecore_wl_window_new(p, x, y, w, h, ECORE_WL_WINDOW_BUFFER_TYPE_SHM);
   ee->prop.window = ee->engine.wl.win->id;

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = ee->alpha;
        einfo->info.rotation = ee->rotation;
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
_ecore_evas_wl_free(Ecore_Evas *ee)
{
   _ecore_evas_wl_buffer_free(ee);
   _ecore_evas_wl_shm_pool_free(ee);
   _ecore_evas_wl_common_free(ee);
}

static void 
_ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

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

        _ecore_evas_wl_buffer_new(ee, w, h);

        einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
        if (!einfo)
          {
            ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
            return;
          }

        einfo->info.dest = ee->engine.wl.pool_data;
        evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

        if (ee->engine.wl.win)
          {
             ecore_wl_window_update_size(ee->engine.wl.win, w, h);
             ecore_wl_window_buffer_attach(ee->engine.wl.win, 
                                           ee->engine.wl.buffer, 0, 0);
          }

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void 
_ecore_evas_wl_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->x != x) || (ee->y != y))
     _ecore_evas_wl_common_move(ee, x, y);
   if ((ee->w != w) || (ee->h != h))
     _ecore_evas_wl_resize(ee, w, h);
}

static void
_ecore_evas_wl_show(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (ee->visible)) return;

   _ecore_evas_wl_buffer_new(ee, ee->w, ee->h);

   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if (!einfo)
     {
        ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
        return;
     }

   einfo->info.dest = ee->engine.wl.pool_data;
   evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

   if (ee->engine.wl.win)
     {
        ecore_wl_window_show(ee->engine.wl.win);
        ecore_wl_window_update_size(ee->engine.wl.win, ee->w, ee->h);
        ecore_wl_window_buffer_attach(ee->engine.wl.win, 
                                      ee->engine.wl.buffer, 0, 0);

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

   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void 
_ecore_evas_wl_hide(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;

   _ecore_evas_wl_buffer_free(ee);

   munmap(ee->engine.wl.pool_data, ee->engine.wl.pool_size);

   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if ((einfo) && (einfo->info.dest))
     {
        einfo->info.dest = NULL;
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
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Wl_Window *win = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->alpha == alpha)) return;
   ee->alpha = alpha;

   /* FIXME: NB: We should really add a ecore_wl_window_alpha_set function
    * but we are in API freeze, so just hack it in for now and fix when 
    * freeze is over */
   if ((win = ee->engine.wl.win))
     win->alpha = alpha;

   /* if (ee->engine.wl.win) */
   /*   ecore_wl_window_transparent_set(ee->engine.wl.win, alpha); */

   _ecore_evas_wl_buffer_new(ee, ee->w, ee->h);

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = alpha;
        einfo->info.dest = ee->engine.wl.pool_data;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }

   if (win)
     {
        ecore_wl_window_update_size(win, ee->w, ee->h);
        ecore_wl_window_buffer_attach(win, ee->engine.wl.buffer, 0, 0);
     }
}

static void 
_ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->transparent == transparent)) return;
   ee->transparent = transparent;

   if (ee->engine.wl.win)
     ecore_wl_window_transparent_set(ee->engine.wl.win, transparent);

   _ecore_evas_wl_buffer_new(ee, ee->w, ee->h);

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = transparent;
        einfo->info.dest = ee->engine.wl.pool_data;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }

   if (ee->engine.wl.win)
     {
        ecore_wl_window_update_size(ee->engine.wl.win, ee->w, ee->h);
        ecore_wl_window_buffer_attach(ee->engine.wl.win, 
                                      ee->engine.wl.buffer, 0, 0);
     }
}

static void
_ecore_evas_wl_frame_complete(void *data, struct wl_callback *callback, uint32_t time __UNUSED__)
{
   Ecore_Evas *ee = data;
   Ecore_Wl_Window *win = NULL;

   if (!ee) return;
   if (!(win = ee->engine.wl.win)) return;

   win->frame_callback = NULL;
   win->frame_pending = EINA_FALSE;
   wl_callback_destroy(callback);

   if (win->surface)
     {
        win->frame_callback = wl_surface_frame(win->surface);
        wl_callback_add_listener(win->frame_callback, &frame_listener, ee);
     }
}

static int
_ecore_evas_wl_render(Ecore_Evas *ee)
{
   int rend = 0;
   Ecore_Wl_Window *win = NULL;

   if (!ee) return 0;
   if (!ee->visible)
     {
        evas_norender(ee->evas);
        return 0;
     }

   if (!(win = ee->engine.wl.win)) return 0;

   rend = _ecore_evas_wl_common_pre_render(ee);
   if (!(win->frame_pending))
     {
        /* FIXME - ideally have an evas_changed_get to return the value
         * of evas->changed to avoid creating this callback and
         * destroying it again
         */

        if (!win->frame_callback)
          {
             win->frame_callback = wl_surface_frame(win->surface);
             wl_callback_add_listener(win->frame_callback, &frame_listener, ee);
          }

        rend |= _ecore_evas_wl_common_render_updates(ee);
        if (rend)
           win->frame_pending = EINA_TRUE;
     }
   _ecore_evas_wl_common_post_render(ee);
   return rend;
}

static void
_ecore_evas_wl_shm_pool_free(Ecore_Evas *ee)
{
   if (!ee->engine.wl.pool) return;

   wl_shm_pool_destroy(ee->engine.wl.pool);
   ee->engine.wl.pool = NULL;
   ee->engine.wl.pool_size = 0;
   ee->engine.wl.pool_data = NULL;
}

static void
_ecore_evas_wl_shm_pool_create(Ecore_Evas *ee, size_t size)
{
   struct wl_shm *shm;
   void *data;
   char tmp[PATH_MAX];
   int fd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (size <= ee->engine.wl.pool_size)
      return;

   size *= 1.5;
   _ecore_evas_wl_shm_pool_free(ee);

   if (!(shm = ecore_wl_shm_get()))
     {
        ERR("ecore_wl_shm_get returned NULL");
        return;
     }

   strcpy(tmp, "/tmp/ecore-evas-wayland_shm-XXXXXX");
   if ((fd = mkstemp(tmp)) < 0) 
     {
        ERR("Could not create temporary file.");
        return;
     }

   if (ftruncate(fd, size) < 0) 
     {
        ERR("Could not truncate temporary file.");
        goto end;
     }

   data = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
   unlink(tmp);

   if (data == MAP_FAILED)
     {
        ERR("mmap of temporary file failed.");
        goto end;
     }

   ee->engine.wl.pool_size = size;
   ee->engine.wl.pool_data = data;
   ee->engine.wl.pool = wl_shm_create_pool(shm, fd, size);

 end:
   close(fd);
}

static void
_ecore_evas_wl_buffer_free(Ecore_Evas *ee)
{
   if (!ee->engine.wl.buffer) return;

   wl_buffer_destroy(ee->engine.wl.buffer);
   ee->engine.wl.buffer = NULL;
}

static void 
_ecore_evas_wl_buffer_new(Ecore_Evas *ee, int w, int h)
{
   unsigned int format;
   int stride = 0;

   stride = (w * sizeof(int));

   _ecore_evas_wl_shm_pool_create(ee, stride * h);

   if ((ee->alpha) || (ee->transparent))
     format = WL_SHM_FORMAT_ARGB8888;
   else
     format = WL_SHM_FORMAT_XRGB8888;

   _ecore_evas_wl_buffer_free(ee);
   ee->engine.wl.buffer = 
     wl_shm_pool_create_buffer(ee->engine.wl.pool, 0, w, h, stride, format);
}

void 
_ecore_evas_wayland_shm_resize(Ecore_Evas *ee, int location)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->engine.wl.win) 
     {
        ee->engine.wl.win->resizing = EINA_TRUE;
        ecore_wl_window_resize(ee->engine.wl.win, ee->w, ee->h, location);
     }
}
#else
EAPI Ecore_Evas *
ecore_evas_wayland_shm_new(const char *disp_name __UNUSED__, unsigned int parent __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__, Eina_Bool frame __UNUSED__)
{
   return NULL;
}
#endif
