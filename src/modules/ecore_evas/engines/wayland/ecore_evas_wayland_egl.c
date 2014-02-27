#include "ecore_evas_wayland_private.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/mman.h>
# include <Evas_Engine_Wayland_Egl.h>

/* local function prototypes */
static void _ecore_evas_wl_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static void _ecore_evas_wl_hide(Ecore_Evas *ee);
static void _ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha);
static void _ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent);
static void _ecore_evas_wl_rotation_set(Ecore_Evas *ee, int rotation, int resize);

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
   _ecore_evas_wl_common_resize,
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
   _ecore_evas_wl_common_borderless_set,
   NULL, // override set
   _ecore_evas_wl_common_maximized_set,
   _ecore_evas_wl_common_fullscreen_set,
   NULL, // func avoid_damage set
   _ecore_evas_wl_common_withdrawn_set,
   NULL, // func sticky set
   _ecore_evas_wl_common_ignore_events_set,
   _ecore_evas_wl_alpha_set,
   _ecore_evas_wl_transparent_set,
   NULL, // func profiles set
   NULL, // func profile set
   NULL, // window group set
   _ecore_evas_wl_common_aspect_set,
   NULL, // urgent set
   NULL, // modal set
   NULL, // demand attention set
   NULL, // focus skip set
   NULL, // _ecore_evas_wl_common_render,
   _ecore_evas_wl_common_screen_geometry_get,
   _ecore_evas_wl_common_screen_dpi_get,
   NULL, // msg parent send
   NULL, // msg_send

   _ecore_evas_wl_common_pointer_xy_get,
   NULL, // pointer warp

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL  // aux_hints_set
};

/* external variables */

/* external functions */
EAPI Ecore_Evas *
ecore_evas_wayland_egl_new_internal(const char *disp_name, unsigned int parent,
				      int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Wl_Window *p = NULL;
   Evas_Engine_Info_Wayland_Egl *einfo;
   Ecore_Evas_Interface_Wayland *iface;
   Ecore_Evas_Engine_Wl_Data *wdata;
   Ecore_Evas *ee;
   int method = 0, count = 0;
   int fx = 0, fy = 0, fw = 0, fh = 0;

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
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;
   ee->prop.draw_frame = frame;
   ee->alpha = EINA_FALSE;

   /* NB: Disabled for right now as it causes textgrid (terminology) 
    * to not draw text anymore */
   /* if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER")) */
   /*   ee->can_async_render = 0; */
   /* else */
   /*   ee->can_async_render = 1; */

   /* frame offset and size */
   if (ee->prop.draw_frame)
     {
        fx = 4;
        fy = 18;
        fw = 8;
        fh = 22;
     }

   if (parent)
     p = ecore_wl_window_find(parent);

   wdata->parent = p;
   wdata->win = 
     ecore_wl_window_new(p, x, y, w + fw, h + fh, 
                         ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW);
   ee->prop.window = ecore_wl_window_id_get(wdata->win);

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

   /* FIXME: Get if parent is alpha, and set */

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.display = ecore_wl_display_get();
        einfo->info.destination_alpha = EINA_TRUE;
        einfo->info.rotation = ee->rotation;
        einfo->info.depth = 32;
        einfo->info.surface = ecore_wl_window_surface_create(wdata->win);
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
_ecore_evas_wl_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->x != x) || (ee->y != y))
     _ecore_evas_wl_common_move(ee, x, y);
   if ((ee->w != w) || (ee->h != h))
     _ecore_evas_wl_common_resize(ee, w, h);
}

static void
_ecore_evas_wl_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->rotation == rotation) return;

   _ecore_evas_wl_common_rotation_set(ee, rotation, resize);

   einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   einfo->info.rotation = rotation;

   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
}

static void 
_ecore_evas_wl_show(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Egl *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (ee->visible)) return;

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   wdata = ee->engine.data;
   if (wdata->win)
     {
        ecore_wl_window_show(wdata->win);
        ecore_wl_window_alpha_set(wdata->win, ee->alpha);
        ecore_wl_window_update_size(wdata->win, ee->w + fw, ee->h + fh);

        einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             struct wl_surface *surf;

             surf = ecore_wl_window_surface_get(wdata->win);
             if ((!einfo->info.surface) || (einfo->info.surface != surf))
               {
                  einfo->info.surface = surf;
                  evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
                  evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
               }
          }
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
   Ecore_Evas_Engine_Wl_Data *wdata;
   Evas_Engine_Info_Wayland_Egl *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   wdata = ee->engine.data;

   evas_sync(ee->evas);

   einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.surface = NULL;
        evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
     }

   if (wdata->win) 
     ecore_wl_window_hide(wdata->win);

   ee->visible = 0;
   ee->should_be_visible = 0;
   _ecore_evas_wl_common_frame_callback_clean(ee);

   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void 
_ecore_evas_wayland_egl_alpha_do(Ecore_Evas *ee, int alpha)
{
   Evas_Engine_Info_Wayland_Egl *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->alpha == alpha)) return;
   ee->alpha = alpha;
   wdata = ee->engine.data;

   if (wdata->win) ecore_wl_window_alpha_set(wdata->win, ee->alpha);

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = EINA_TRUE;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
     }
}

static void
_ecore_evas_wl_alpha_set(Ecore_Evas *ee, int alpha)
{
   if (ee->in_async_render)
     {
        ee->delayed.alpha = alpha;
        ee->delayed.alpha_changed = EINA_TRUE;
        return;
     }

   _ecore_evas_wayland_egl_alpha_do(ee, alpha);
}

static void 
_ecore_evas_wayland_egl_transparent_do(Ecore_Evas *ee, int transparent)
{
   Evas_Engine_Info_Wayland_Egl *einfo;
   Ecore_Evas_Engine_Wl_Data *wdata;
   int fw, fh;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if ((ee->transparent == transparent)) return;
   ee->transparent = transparent;

   wdata = ee->engine.data;
   if (wdata->win)
     ecore_wl_window_transparent_set(wdata->win, ee->transparent);

   evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.destination_alpha = EINA_TRUE;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w + fw, ee->h + fh);
     }
}

static void 
_ecore_evas_wl_transparent_set(Ecore_Evas *ee, int transparent)
{
   if (ee->in_async_render)
     {
        ee->delayed.transparent = transparent;
        ee->delayed.transparent_changed = EINA_TRUE;
        return;
     }

   _ecore_evas_wayland_egl_transparent_do(ee, transparent);
}

void 
_ecore_evas_wayland_egl_resize(Ecore_Evas *ee, int location)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (wdata->win) 
     {
        int fw, fh;

        _ecore_evas_wayland_egl_resize_edge_set(ee, location);

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);

        if (ECORE_EVAS_PORTRAIT(ee))
          ecore_wl_window_resize(wdata->win, ee->w + fw, ee->h + fh, location);
        else
          ecore_wl_window_resize(wdata->win, ee->w + fh, ee->h + fw, location);
     }
}

void 
_ecore_evas_wayland_egl_resize_edge_set(Ecore_Evas *ee, int edge)
{
   Evas_Engine_Info_Wayland_Egl *einfo;

   if ((einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(ee->evas)))
     einfo->info.edges = edge;
}

#endif
