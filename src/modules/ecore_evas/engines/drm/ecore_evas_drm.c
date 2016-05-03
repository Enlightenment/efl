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
   int fd;
   int cw, ch;
   int clockid;
   int x, y, w, h;
   int depth, bpp;
   unsigned int format;
   Ecore_Drm2_Device *dev;
   Ecore_Drm2_Output *output;
} Ecore_Evas_Engine_Drm_Data;

static int _drm_init_count = 0;

static int
_ecore_evas_drm_init(Ecore_Evas_Engine_Drm_Data *edata, const char *device)
{
   int mw, mh;

   if (++_drm_init_count != 1) return _drm_init_count;

   if (!ecore_drm2_init())
     {
        ERR("Failed to init Ecore_Drm2 library");
        goto init_err;
     }

   if (!device) device = "seat0";

   edata->dev = ecore_drm2_device_find(device, 0, EINA_TRUE);
   if (!edata->dev)
     {
        ERR("Failed to create device");
        goto dev_err;
     }

   edata->fd = ecore_drm2_device_open(edata->dev);
   if (edata->fd < 0)
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
   if (!edata->output)
     WRN("Could not find output at %d %d", edata->x, edata->y);

   ecore_drm2_output_crtc_size_get(edata->output, &mw, &mh);
   ecore_drm2_device_pointer_max_set(edata->dev, mw, mh);

   ecore_event_evas_init();

   return _drm_init_count;

output_err:
   ecore_drm2_device_close(edata->dev);
open_err:
   ecore_drm2_device_free(edata->dev);
dev_err:
   ecore_drm2_shutdown();
init_err:
   return --_drm_init_count;
}

static int
_ecore_evas_drm_shutdown(Ecore_Evas_Engine_Drm_Data *edata)
{
   if (--_drm_init_count != 0) return _drm_init_count;

   ecore_drm2_outputs_destroy(edata->dev);
   ecore_drm2_device_close(edata->dev);
   ecore_drm2_device_free(edata->dev);
   ecore_drm2_shutdown();
   ecore_event_evas_shutdown();

   return _drm_init_count;
}

static void
_drm_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   ecore_evas_input_event_unregister(ee);

   edata = ee->engine.data;
   _ecore_evas_drm_shutdown(edata);
   free(edata);
}

static int
_drm_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;

   if ((ee->visible) && (updates))
     {
        _ecore_evas_idle_timeout_update(ee);
        rend = 1;
     }
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}

static void
_drm_render_updates(void *data, Evas *evas EINA_UNUSED, void *event)
{
   Evas_Event_Render_Post *ev;
   Ecore_Evas *ee;

   ev = event;
   if (!ev) return;

   ee = data;
   if (!ee) return;

   ee->in_async_render = EINA_FALSE;
   _drm_render_updates_process(ee, ev->updated_area);
}

static int
_drm_render(Ecore_Evas *ee)
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
        rend = _drm_render_updates_process(ee, updates);
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
_drm_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   Ecore_Evas_Engine_Drm_Data *edata;

   edata = ee->engine.data;
   ecore_drm2_output_geometry_get(edata->output, x, y, w, h);
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
_drm_show(Ecore_Evas *ee)
{
   if ((!ee) || (ee->visible)) return;

   ee->should_be_visible = 1;

   if (ee->prop.avoid_damage)
     _drm_render(ee);

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_FALSE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   if (ee->visible) return;

   ee->visible = 1;
   if (ee->prop.fullscreen)
     {
        evas_focus_in(ee->evas);
        if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
     }
   if (ee->func.fn_show) ee->func.fn_show(ee);
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
_drm_rotation_set(Ecore_Evas *ee, int rotation, int resize EINA_UNUSED)
{
   Evas_Engine_Info_Drm *einfo;

   if (ee->rotation == rotation) return;
   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (!einfo) return;
   einfo->info.rotation = rotation;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);
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
_drm_object_cursor_del(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee) ee->prop.cursor.object = NULL;
}

static void
_drm_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   Evas_Object *old;
   int x, y;

   old = ee->prop.cursor.object;
   if (!obj)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        goto end;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_evas_pointer_xy_get(ee, &x, &y);

   if (obj != old)
     {
        evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
        evas_object_pass_events_set(ee->prop.cursor.object, 1);
        if (evas_pointer_inside_get(ee->evas))
          evas_object_show(ee->prop.cursor.object);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _drm_object_cursor_del, ee);
     }

   evas_object_move(ee->prop.cursor.object, x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);

end:
   if ((old) && (obj != old))
     {
        evas_object_event_callback_del_full
          (old, EVAS_CALLBACK_DEL, _drm_object_cursor_del, ee);
        evas_object_del(old);
     }
}

static void
_drm_object_cursor_unset(Ecore_Evas *ee)
{
   evas_object_event_callback_del_full(ee->prop.cursor.object,
                                       EVAS_CALLBACK_DEL,
                                       _drm_object_cursor_del, ee);
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

        ecore_drm2_output_geometry_get(edata->output, NULL, NULL, &ow, &oh);
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
   if (ee->prop.aspect == aspect) return;
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
   _drm_object_cursor_set,
   _drm_object_cursor_unset,
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

   _drm_render,

   _drm_screen_geometry_get,
   NULL, //void (*fn_screen_dpi_get) (const Ecore_Evas *ee, int *xdpi, int *ydpi);
   NULL, //void (*fn_msg_parent_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);
   NULL, //void (*fn_msg_send) (Ecore_Evas *ee, int maj, int min, void *data, int size);

   _drm_pointer_xy_get,
   _drm_pointer_warp,

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL, // aux_hints_set

   NULL, // animator_register
   NULL // animator_unregister
};

EAPI Ecore_Evas *
ecore_evas_drm_new_internal(const char *device, unsigned int parent EINA_UNUSED, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Evas_Engine_Info_Drm *einfo;
   Ecore_Evas_Interface_Drm *iface;
   Ecore_Evas_Engine_Drm_Data *edata;
   int method;

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
   edata->depth = 24; // FIXME: Remove hardcode
   edata->bpp = 32; // FIXME: Remove hardcode
   edata->format = DRM_FORMAT_XRGB8888;

   if (_ecore_evas_drm_init(edata, device) < 1)
     {
        free(edata);
        free(ee);
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->driver = "drm";
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

   ee->can_async_render = 1;
   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
                             _drm_render_updates, ee);

   einfo = (Evas_Engine_Info_Drm *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.fd = edata->fd;
        einfo->info.bpp = edata->bpp;
        einfo->info.depth = edata->depth;
        einfo->info.format = edata->format;
        einfo->info.rotation = ee->rotation;
        einfo->info.output = edata->output;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);
             goto eng_err;
          }
     }

   ee->prop.window = ecore_drm2_output_crtc_get(edata->output);
   ecore_drm2_device_window_set(edata->dev, ee->prop.window);

   ecore_evas_data_set(ee, "device", edata->dev);

   _ecore_evas_register(ee);
   ecore_evas_input_event_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   return ee;

eng_err:
   ecore_evas_free(ee);
   return NULL;
}

#ifdef BUILD_ECORE_EVAS_GL_DRM
EAPI Ecore_Evas *
ecore_evas_gl_drm_new_internal(const char *device, unsigned int parent EINA_UNUSED, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Evas_Engine_Info_GL_Drm *einfo;
   Ecore_Evas_Interface_Drm *iface;
   Ecore_Evas_Engine_Drm_Data *edata;
   int method;

   method = evas_render_method_lookup("gl_drm");
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
   edata->depth = 24; // FIXME: Remove hardcode
   edata->bpp = 32; // FIXME: Remove hardcode
   edata->format = DRM_FORMAT_XRGB8888;

   dlopen("libglapi.so.0", RTLD_LAZY | RTLD_GLOBAL);
   if (dlerror())
     {
        free(edata);
        free(ee);
        return NULL;
     }

   if (_ecore_evas_drm_init(edata, device) < 1)
     {
        free(edata);
        free(ee);
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->driver = "gl_drm";
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

   ee->can_async_render = 0; // FIXME ??
   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
                             _drm_render_updates, ee);

   einfo = (Evas_Engine_Info_GL_Drm *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        char *num;

        einfo->info.vsync = EINA_TRUE;

        num = getenv("EVAS_DRM_VSYNC");
        if ((num) && (!atoi(num)))
          einfo->info.vsync = EINA_FALSE;

        einfo->info.fd = edata->fd;
        einfo->info.bpp = edata->bpp;
        einfo->info.depth = edata->depth;
        einfo->info.format = edata->format;
        einfo->info.rotation = ee->rotation;
        einfo->info.output = edata->output;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed", ee->driver);
             goto eng_err;
          }
     }

   ee->prop.window = ecore_drm2_output_crtc_get(edata->output);
   ecore_drm2_device_window_set(edata->dev, ee->prop.window);

   ecore_evas_data_set(ee, "device", edata->dev);

   _ecore_evas_register(ee);
   ecore_evas_input_event_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   return ee;

eng_err:
   ecore_evas_free(ee);
   return NULL;
}
#endif
