#include "ecore_evas_wayland_private.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
# include <Evas_Engine_Wayland_Shm.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/mman.h>

/* local function prototypes */
static void _ecore_evas_wl_free(Ecore_Evas *ee);
static void _ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static void _ecore_evas_wl_hide(Ecore_Evas *ee);
static void _ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha);
static void _ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent);
static void _ecore_evas_wl_rotation_set(Ecore_Evas *ee, int rotation, int resize);

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
   _ecore_evas_wl_rotation_set,
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
   NULL, // func profile set
   NULL, // window group set
   NULL, // aspect set
   NULL, // urgent set
   NULL, // modal set
   NULL, // demand attention set
   NULL, // focus skip set
   NULL, //_ecore_evas_wl_common_render,
   _ecore_evas_wl_common_screen_geometry_get,
   _ecore_evas_wl_common_screen_dpi_get
};

/* external variables */

/* external functions */
EAPI Ecore_Evas *
ecore_evas_wayland_shm_new_internal(const char *disp_name, unsigned int parent, int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Wl_Window *p = NULL;
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Evas_Interface_Wayland *iface;
   Ecore_Evas *ee;
   int method = 0, count = 0;
   int fx, fy, fw, fh;

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
   if (!(wdata = calloc(1, sizeof(Ecore_Evas_Engine_Wl_Data))))
     {
	ERR("Failed to allocate Ecore_Evas_Engine_Wl_Data");
	free(ee);
	goto ee_err;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_wl_common_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_wl_engine_func;
   ee->engine.data = wdata;

   iface = _ecore_evas_wl_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

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

   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;
   else
     ee->can_async_render = 1;

   /* frame offset and size */
   fx = 4;
   fy = 18;
   fw = 8;
   fh = 22;

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, ee->w + fw, ee->h + fh);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w + fw, ee->h + fh);

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
			     _ecore_evas_wl_common_render_updates, ee);

   /* FIXME: This needs to be set based on theme & scale */
   if (ee->prop.draw_frame)
     evas_output_framespace_set(ee->evas, fx, fy, fw, fh);

   if (parent)
     p = ecore_wl_window_find(parent);

   /* FIXME: Get if parent is alpha, and set */

   wdata->parent = p;
   wdata->win = 
     ecore_wl_window_new(p, x, y, w, h, ECORE_WL_WINDOW_BUFFER_TYPE_SHM);
   ee->prop.window = wdata->win->id;

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.wl_shm = ecore_wl_shm_get();
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
        wdata->frame = _ecore_evas_wl_common_frame_add(ee->evas);
        _ecore_evas_wl_common_frame_border_size_set(wdata->frame, fx, fy, fw, fh);
        evas_object_move(wdata->frame, -fx, -fy);
        evas_object_layer_set(wdata->frame, EVAS_LAYER_MAX - 1);
     }

   ee->engine.func->fn_render = _ecore_evas_wl_common_render;

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
   _ecore_evas_wl_common_free(ee);
}

static void 
_ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Wl_Data *wdata;
   int orig_w, orig_h;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->req.w = w;
   ee->req.h = h;
   orig_w = w;
   orig_h = h;

   wdata = ee->engine.data;

   if (!ee->prop.fullscreen)
     {
        int fw = 0, fh = 0;

        if (ee->prop.min.w > w) w = ee->prop.min.w;
        else if (w > ee->prop.max.w) w = ee->prop.max.w;
        if (ee->prop.min.h > h) h = ee->prop.min.h;
        else if (h > ee->prop.max.h) h = ee->prop.max.h;

        orig_w = w;
        orig_h = h;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             w += fh;
             h += fw;
          }
        else
          {
             w += fw;
             h += fh;
          }
     }

   if ((ee->w != w) || (ee->h != h))
     {
        ee->w = orig_w;
        ee->h = orig_h;

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

        if (wdata->frame)
          evas_object_resize(wdata->frame, w, h);

        if (wdata->win)
          {
             Ecore_Wl_Window *win;
             Evas_Engine_Info_Wayland_Shm *einfo;

             win = wdata->win;

             if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
               einfo->info.edges = win->edges;

             win->edges = 0;

             win->server_allocation = win->allocation;
             ecore_wl_window_update_size(wdata->win, w, h);
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
_ecore_evas_wl_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   Evas_Engine_Info_Wayland_Shm *einfo;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->rotation == rotation) return;

   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if (!einfo)
     return;
   einfo->info.rotation = rotation;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
   {
       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
   }

   _ecore_evas_wl_common_rotation_set(ee, rotation, resize);
}

static void
_ecore_evas_wl_show(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (ee->visible)) return;

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
   wdata = ee->engine.data;

   if (wdata->win)
     {
        ecore_wl_window_show(wdata->win);
        ecore_wl_window_update_size(wdata->win, ee->w + fw, ee->h + fh);

        einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             einfo->info.wl_shm = ecore_wl_shm_get();
             einfo->info.wl_surface = ecore_wl_window_surface_get(wdata->win);
             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
          }

        if ((ee->prop.clas) && (wdata->win->shell_surface))
          wl_shell_surface_set_class(wdata->win->shell_surface, 
                                     ee->prop.clas);
        if ((ee->prop.title) && (wdata->win->shell_surface))
          wl_shell_surface_set_title(wdata->win->shell_surface, 
                                     ee->prop.title);
     }

   if (wdata->frame)
     {
        evas_object_show(wdata->frame);
        evas_object_resize(wdata->frame, ee->w + fw, ee->h + fh);
     }

   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void 
_ecore_evas_wl_hide(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   wdata = ee->engine.data;

   evas_sync(ee->evas);

   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.wl_surface = NULL;
        evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
     }

   if (wdata->win) 
     ecore_wl_window_hide(wdata->win);

   ee->visible = 0;
   ee->should_be_visible = 0;

   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void 
_ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha)
{
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Wl_Window *win = NULL;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->alpha == alpha)) return;
   ee->alpha = alpha;
   wdata = ee->engine.data;

   /* FIXME: NB: We should really add a ecore_wl_window_alpha_set function
    * but we are in API freeze, so just hack it in for now and fix when 
    * freeze is over */
   if ((win = wdata->win))
     win->alpha = alpha;

   /* if (wdata->win) */
   /*   ecore_wl_window_transparent_set(wdata->win, alpha); */

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = alpha;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
     }

   if (win)
     ecore_wl_window_update_size(win, ee->w + fw, ee->h + fh);
}

static void 
_ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent)
{
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->transparent == transparent)) return;
   ee->transparent = transparent;

   wdata = ee->engine.data;
   if (wdata->win)
     ecore_wl_window_transparent_set(wdata->win, transparent);

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = transparent;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
     }

   if (wdata->win)
     ecore_wl_window_update_size(wdata->win, ee->w + fw, ee->h + fh);
}

void 
_ecore_evas_wayland_shm_resize(Ecore_Evas *ee, int location)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (wdata->win) 
     {
        int fw, fh;
        Evas_Engine_Info_Wayland_Shm *einfo;

        wdata->win->resizing = EINA_TRUE;
        if ((ee->rotation == 90) || (ee->rotation == 270))
          evas_output_framespace_get(ee->evas, NULL, NULL, &fh, &fw);
        else
          evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

        if ((ee->rotation == 90) || (ee->rotation == 270))
          ecore_wl_window_resize(wdata->win, ee->w + fh, ee->h + fw, location);
        else
          ecore_wl_window_resize(wdata->win, ee->w + fw, ee->h + fh, location);

        if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas)))
          einfo->info.edges = location;
     }
}
#endif
