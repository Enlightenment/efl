#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <Eina.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Ecore_Evas.h>
#include "ecore_evas_private.h"
#include "ecore_evas_drm.h"

//#ifdef BUILD_ECORE_EVAS_DRM
# include <Evas_Engine_Drm.h>
# include <Ecore_Drm.h>
//#endif

/* local function prototypes */
static int _ecore_evas_drm_init(const char *device);
static int _ecore_evas_drm_shutdown(void);
static Ecore_Evas_Interface_Drm *_ecore_evas_drm_interface_new(void);

static void _ecore_evas_drm_free(Ecore_Evas *ee);
static void _ecore_evas_drm_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
static void _ecore_evas_drm_move(Ecore_Evas *ee, int x, int y);
static void _ecore_evas_drm_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_drm_move_resize(Ecore_Evas *ee, int x, int y, int w, int h);
static void _ecore_evas_drm_rotation_set(Ecore_Evas *ee, int rotation, int resize);
static void _ecore_evas_drm_show(Ecore_Evas *ee);
static void _ecore_evas_drm_hide(Ecore_Evas *ee);
static void _ecore_evas_drm_title_set(Ecore_Evas *ee, const char *title);
static void _ecore_evas_drm_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
static int _ecore_evas_drm_render(Ecore_Evas *ee);
static void _ecore_evas_drm_render_updates(void *data, Evas *evas EINA_UNUSED, void *event);
static int _ecore_evas_drm_render_updates_process(Ecore_Evas *ee, Eina_List *updates);

/* local variables */
static int _ecore_evas_init_count = 0;
static Ecore_Drm_Device *dev = NULL;

static Ecore_Evas_Engine_Func _ecore_evas_drm_engine_func = 
{
   _ecore_evas_drm_free,
   _ecore_evas_drm_callback_resize_set,
   _ecore_evas_drm_callback_move_set,
   NULL, //void (*fn_callback_show_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_hide_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   _ecore_evas_drm_delete_request_set,
   NULL, //void (*fn_callback_destroy_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   _ecore_evas_drm_callback_focus_in_set,
   _ecore_evas_drm_callback_focus_out_set,
   _ecore_evas_drm_callback_mouse_in_set,
   _ecore_evas_drm_callback_mouse_out_set,
   NULL, //void (*fn_callback_sticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_unsticky_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_pre_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   NULL, //void (*fn_callback_post_render_set) (Ecore_Evas *ee, Ecore_Evas_Event_Cb func);
   _ecore_evas_drm_move,
   NULL, //void (*fn_managed_move) (Ecore_Evas *ee, int x, int y);
   _ecore_evas_drm_resize,
   _ecore_evas_drm_move_resize,
   _ecore_evas_drm_rotation_set,
   NULL, //void (*fn_shaped_set) (Ecore_Evas *ee, int shaped);
   _ecore_evas_drm_show,
   _ecore_evas_drm_hide,
   NULL, //void (*fn_raise) (Ecore_Evas *ee);
   NULL, //void (*fn_lower) (Ecore_Evas *ee);
   NULL, //void (*fn_activate) (Ecore_Evas *ee);
   _ecore_evas_drm_title_set,
   _ecore_evas_drm_name_class_set,
   NULL, //void (*fn_size_min_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_size_max_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_size_base_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_size_step_set) (Ecore_Evas *ee, int w, int h);
   NULL, //void (*fn_object_cursor_set) (Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y);
   NULL, //void (*fn_layer_set) (Ecore_Evas *ee, int layer);
   NULL, //void (*fn_focus_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_iconified_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_borderless_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_override_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_maximized_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_fullscreen_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_avoid_damage_set) (Ecore_Evas *ee, int on);
   NULL, //void (*fn_withdrawn_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_sticky_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_ignore_events_set) (Ecore_Evas *ee, int ignore);
   NULL, //void (*fn_alpha_set) (Ecore_Evas *ee, int alpha);
   NULL, //void (*fn_transparent_set) (Ecore_Evas *ee, int transparent);
   NULL, //void (*fn_profiles_set) (Ecore_Evas *ee, const char **profiles, int count);
   NULL, //void (*fn_profile_set) (Ecore_Evas *ee, const char *profile);
   
   NULL, //void (*fn_window_group_set) (Ecore_Evas *ee, const Ecore_Evas *ee_group);
   NULL, //void (*fn_aspect_set) (Ecore_Evas *ee, double aspect);
   NULL, //void (*fn_urgent_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_modal_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_demands_attention_set) (Ecore_Evas *ee, Eina_Bool on);
   NULL, //void (*fn_focus_skip_set) (Ecore_Evas *ee, Eina_Bool on);
   
   _ecore_evas_drm_render,

   NULL, //void (*fn_screen_geometry_get) (const Ecore_Evas *ee, int *x, int *y, int *w, int *h);
   NULL, //void (*fn_screen_dpi_get) (const Ecore_Evas *ee, int *xdpi, int *ydpi);
   NULL, //void (*fn_msg_parent_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   NULL, //void (*fn_msg_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);

   NULL, // pointer_xy_get
   NULL, // pointer_warp

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL  // aux_hints_set
};

EAPI Ecore_Evas *
ecore_evas_drm_new_internal(const char *device, unsigned int parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Evas_Engine_Info_Drm *einfo;
   Ecore_Evas_Interface_Drm *iface;
   int method;

   /* try to find the evas drm engine */
   if (!(method = evas_render_method_lookup("drm")))
     {
        ERR("Render method lookup failed for Drm");
        return NULL;
     }

   /* try to init drm */
   if (_ecore_evas_drm_init(device) < 1) return NULL;

   /* try to allocate space for new ecore_evas */
   if (!(ee = calloc(1, sizeof(Ecore_Evas))))
     {
        ERR("Failed to allocate space for new Ecore_Evas");
        goto ee_err;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_evas_drm_engine_func;

   iface = _ecore_evas_drm_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   /* set some engine properties */
   ee->driver = "drm";
   if (device) ee->name = strdup(device);
   else
     ee->name = strdup(ecore_drm_device_name_get(dev));

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->x = ee->req.x = x;
   ee->y = ee->req.y = y;
   ee->w = ee->req.w = w;
   ee->h = ee->req.h = h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->alpha = EINA_FALSE;

   ee->can_async_render = 1;
   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;

   /* try to initialize evas */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);

   /* FIXME: Support initial rotation ?? */
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST, 
                             _ecore_evas_drm_render_updates, ee);

   if ((einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas)))
     {
        einfo->info.depth = 32; // FIXME
        einfo->info.destination_alpha = ee->alpha;
        einfo->info.rotation = ee->rotation;
        einfo->info.vsync = EINA_FALSE;
        einfo->info.use_hw_accel = EINA_FALSE;
        einfo->info.fd = ecore_drm_device_fd_get(dev);
        einfo->info.tty = ecore_drm_tty_get(dev);

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             goto eng_err;
          }
     }
   else
     {
        ERR("Failed to get Evas Engine Info for '%s'", ee->driver);
        goto eng_err;
     }

   ee->prop.window = einfo->info.output;

   _ecore_evas_register(ee);
   ecore_evas_input_event_register(ee);

   ecore_drm_device_window_set(dev, ee->prop.window);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   evas_event_feed_mouse_in(ee->evas, 
                            (unsigned int)((unsigned long long)
                                           (ecore_time_get() * 1000.0) & 
                                           0xffffffff), NULL);

   return ee;

eng_err:
   ecore_evas_free(ee);
ee_err:
   _ecore_evas_drm_shutdown();
   return NULL;
}

/* local functions */
static int 
_ecore_evas_drm_init(const char *device)
{
   if (++_ecore_evas_init_count != 1) return _ecore_evas_init_count;

   /* try to init ecore_drm */
   if (!ecore_drm_init())
     {
        ERR("Could not initialize Ecore_Drm");
        return --_ecore_evas_init_count;
     }

   /* try to find the device */
   if (!(dev = ecore_drm_device_find(device, NULL)))
     {
        ERR("Could not find drm device with name: %s", device);

        /* try getting the default drm device */
        if (!(dev = ecore_drm_device_find(NULL, NULL)))
          goto dev_err;
     }

   /* try to open the graphics card */
   if (!ecore_drm_device_open(dev))
     {
        ERR("Could not open drm device");
        goto dev_open_err;
     }

   /* try to open the tty */
   if (!ecore_drm_tty_open(dev, NULL))
     {
        ERR("Could not open tty: %m");
        goto tty_open_err;
     }

   /* try to create sprites */
   if (!ecore_drm_sprites_create(dev))
     {
        ERR("Could not create sprites: %m");
        goto sprite_err;
     }

   /* NB: We don't need to create outputs here. Evas will create the 
    * framebuffers it needs */
   /* try to create outputs */
   /* if (!ecore_drm_outputs_create(dev)) */
   /*   { */
   /*      ERR("Could not create outputs: %m"); */
   /*      goto output_err; */
   /*   } */

   /* try to create inputs */
   if (!ecore_drm_inputs_create(dev))
     {
        ERR("Could not create inputs: %m");
        goto output_err;
     }

   ecore_event_evas_init();

   return _ecore_evas_init_count;

output_err:
   ecore_drm_sprites_destroy(dev);
sprite_err:
   ecore_drm_tty_close(dev);
tty_open_err:
   ecore_drm_device_close(dev);
dev_open_err:
   ecore_drm_device_free(dev);
dev_err:
   ecore_drm_shutdown();
   return --_ecore_evas_init_count;
}

static int 
_ecore_evas_drm_shutdown(void)
{
   if (--_ecore_evas_init_count != 0) return _ecore_evas_init_count;

   ecore_drm_sprites_destroy(dev);
   /* NB: No need to free outputs here. Is done in device free */
   ecore_drm_inputs_destroy(dev);
   ecore_drm_tty_close(dev);
   ecore_drm_device_close(dev);
   ecore_drm_device_free(dev);
   ecore_drm_shutdown();

   ecore_event_evas_shutdown();

   return _ecore_evas_init_count;
}

static Ecore_Evas_Interface_Drm *
_ecore_evas_drm_interface_new(void)
{
   Ecore_Evas_Interface_Drm *iface;

   if (!(iface = calloc(1, sizeof(Ecore_Evas_Interface_Drm))))
     return NULL;

   iface->base.name = "drm";
   iface->base.version = 1;

   /* iface->pixmap_visual_get; */
   /* iface->pixmap_colormap_get; */
   /* iface->pixmap_depth_get; */

   return iface;
}

/* local ecore_evas functions */
static void 
_ecore_evas_drm_free(Ecore_Evas *ee)
{
   ecore_evas_input_event_unregister(ee);
   _ecore_evas_drm_shutdown();
}

static void 
_ecore_evas_drm_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_resize = func;
}

static void 
_ecore_evas_drm_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_move = func;
}

static void 
_ecore_evas_drm_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_focus_in = func;
}

static void 
_ecore_evas_drm_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_focus_out = func;
}

static void 
_ecore_evas_drm_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_mouse_in = func;
}

static void 
_ecore_evas_drm_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_mouse_out = func;
}

static void 
_ecore_evas_drm_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_delete_request = func;
}

static void 
_ecore_evas_drm_move(Ecore_Evas *ee, int x, int y)
{
   ee->req.x = x;
   ee->req.y = y;
   if ((ee->x == x) && (ee->y == y)) return;
   ee->x = x;
   ee->y = y;
   if (ee->func.fn_move) ee->func.fn_move(ee);
}

static void 
_ecore_evas_drm_resize(Ecore_Evas *ee, int w, int h)
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
_ecore_evas_drm_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   if ((ee->x != x) || (ee->y != y))
     _ecore_evas_drm_move(ee, x, y);
   if ((ee->w != w) || (ee->h != h))
     _ecore_evas_drm_resize(ee, w, h);
}

static void 
_ecore_evas_drm_rotation_set(Ecore_Evas *ee, int rotation, int resize EINA_UNUSED)
{
   Evas_Engine_Info_Drm *einfo;

   if (ee->rotation == rotation) return;
   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return;
   einfo->info.rotation = rotation;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
}

static void 
_ecore_evas_drm_show(Ecore_Evas *ee)
{
   if ((!ee) || (ee->visible)) return;
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void 
_ecore_evas_drm_hide(Ecore_Evas *ee)
{
   if ((!ee) || (!ee->visible)) return;
   evas_sync(ee->evas);
   ee->visible = 0;
   ee->should_be_visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void 
_ecore_evas_drm_title_set(Ecore_Evas *ee, const char *title)
{
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
}

static void 
_ecore_evas_drm_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   if (n) ee->prop.name = strdup(n);
   if (c) ee->prop.clas = strdup(c);
}

static int 
_ecore_evas_drm_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *l;
   Ecore_Evas *ee2;

   if (ee->in_async_render) return 0;

   if (!ee->visible)
     {
        evas_norender(ee->evas);
        return 0;
     }

   EINA_LIST_FOREACH(ee->sub_ecore_evas, l, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   if (!ee->can_async_render)
     {
        Eina_List *updates;

        updates = evas_render_updates(ee->evas);
        rend = _ecore_evas_drm_render_updates_process(ee, updates);
        evas_render_updates_free(updates);
     }
   else if (evas_render_async(ee->evas))
     {
        ee->in_async_render = EINA_TRUE;
        rend = 1;
     }

   return rend;
}

static void 
_ecore_evas_drm_render_updates(void *data, Evas *evas EINA_UNUSED, void *event)
{
   Evas_Event_Render_Post *ev;
   Ecore_Evas *ee;

   if (!(ev = event)) return;
   if (!(ee = data)) return;

   ee->in_async_render = EINA_FALSE;

   _ecore_evas_drm_render_updates_process(ee, ev->updated_area);

   /* TODO: handle delayed changes */
}

static int 
_ecore_evas_drm_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;

   if ((ee->visible) && (updates))
     {
        Eina_List *l = NULL;
        Eina_Rectangle *r;

        EINA_LIST_FOREACH(updates, l, r)
          {
             /* TODO */
          }

        _ecore_evas_idle_timeout_update(ee);
        rend = 1;
     }
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}
