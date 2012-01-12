#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define LOGFNS 1

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

# include <Eina.h>
# include <Evas.h>
# include <Ecore.h>

# include "ecore_evas_private.h"
# include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
# include <Evas_Engine_Wayland_Shm.h>
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
static void _ecore_evas_wl_move(Ecore_Evas *ee, int x, int y);
static void _ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_show(Ecore_Evas *ee);
static void _ecore_evas_wl_hide(Ecore_Evas *ee);
static void _ecore_evas_wl_raise(Ecore_Evas *ee);
static void _ecore_evas_wl_lower(Ecore_Evas *ee);
static void _ecore_evas_wl_activate(Ecore_Evas *ee);
static void _ecore_evas_wl_title_set(Ecore_Evas *ee, const char *t);
static void _ecore_evas_wl_name_class_set(Ecore_Evas *ee, const char *n, const char *c);
static void _ecore_evas_wl_size_min_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_size_max_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_size_base_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_size_step_set(Ecore_Evas *ee, int w, int h);
static void _ecore_evas_wl_object_cursor_set(Ecore_Evas *ee, Evas_Object  *obj, int layer, int hot_x, int hot_y);
static void _ecore_evas_wl_object_cursor_del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__);
static void _ecore_evas_wl_layer_set(Ecore_Evas *ee, int layer);
static void _ecore_evas_wl_focus_set(Ecore_Evas *ee, int focus __UNUSED__);
static void _ecore_evas_wl_iconified_set(Ecore_Evas *ee, int iconify);
static void _ecore_evas_wl_maximized_set(Ecore_Evas *ee, int max);
static int _ecore_evas_wl_render(Ecore_Evas *ee);
static void _ecore_evas_wl_screen_geometry_get(const Ecore_Evas *ee __UNUSED__, int *x, int *y, int *w, int *h);
static void _ecore_evas_wl_buffer_new(Ecore_Evas *ee, void **dest);

static Eina_Bool _ecore_evas_wl_event_mouse_down(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_mouse_up(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_focus_in(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _ecore_evas_wl_event_focus_out(void *data __UNUSED__, int type __UNUSED__, void *event);

static void _ecore_evas_wl_handle_configure(void *data, struct wl_shell_surface *shell_surface __UNUSED__, uint32_t timestamp __UNUSED__, uint32_t edges __UNUSED__, int32_t width, int32_t height);
static void _ecore_evas_wl_handle_popup_done(void *data __UNUSED__, struct wl_shell_surface *shell_surface __UNUSED__);

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
static Ecore_Event_Handler *_ecore_evas_wl_event_handlers[8];
static uint32_t _ecore_evas_wl_btn_timestamp;
static const struct wl_shell_surface_listener _ecore_evas_wl_shell_surface_listener = 
{
   _ecore_evas_wl_handle_configure,
   _ecore_evas_wl_handle_popup_done
};

static Ecore_Evas_Engine_Func _ecore_wl_engine_func = 
{
   _ecore_evas_wl_free, 
   _ecore_evas_wl_callback_resize_set, 
   _ecore_evas_wl_callback_move_set, 
   NULL, // callback show set
   NULL, // callback hide set
   _ecore_evas_wl_callback_delete_request_set, 
   NULL, // callback destroy set
   _ecore_evas_wl_callback_focus_in_set, 
   _ecore_evas_wl_callback_focus_out_set, 
   _ecore_evas_wl_callback_mouse_in_set, 
   NULL, // callback mouse out set
   NULL, // callback sticky set
   NULL, // callback unsticky set
   NULL, // callback pre render set
   NULL, // callback post render set
   _ecore_evas_wl_move, 
   NULL, // func managed move
   _ecore_evas_wl_resize, 
   NULL, // func move_resize
   NULL, // func rotation set
   NULL, // func shaped set
   _ecore_evas_wl_show, 
   _ecore_evas_wl_hide, 
   _ecore_evas_wl_raise, 
   _ecore_evas_wl_lower, 
   _ecore_evas_wl_activate, 
   _ecore_evas_wl_title_set, 
   _ecore_evas_wl_name_class_set, 
   _ecore_evas_wl_size_min_set, 
   _ecore_evas_wl_size_max_set, 
   _ecore_evas_wl_size_base_set, 
   _ecore_evas_wl_size_step_set, 
   _ecore_evas_wl_object_cursor_set, 
   _ecore_evas_wl_layer_set, 
   _ecore_evas_wl_focus_set, 
   _ecore_evas_wl_iconified_set, 
   NULL, // func borderless set
   NULL, // func override set
   _ecore_evas_wl_maximized_set, 
   NULL, // func fullscreen set
   NULL, // func avoid_damage set
   NULL, // func withdrawn set
   NULL, // func sticky set
   NULL, // func ignore_events set
   NULL, // func alpha set
   NULL, // func transparent set
   _ecore_evas_wl_render, 
   _ecore_evas_wl_screen_geometry_get
};

/* external variables */
#endif

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
EAPI Ecore_Evas *
ecore_evas_wayland_shm_new(const char *disp_name, int x, int y, int w, int h, int frame)
{
   Evas_Engine_Info_Wayland_Shm *einfo;
   Ecore_Evas *ee;
   int method = 0;
   static int _win_id = 1;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(method = evas_render_method_lookup("wayland_shm"))) 
     {
        ERR("Render method lookup failed.");
        return NULL;
     }

   if (!(ecore_wl_init(disp_name))) 
     {
        ERR("Failed to initialize Ecore Wayland.");
        return NULL;
     }

   if (!(ee = calloc(1, sizeof(Ecore_Evas)))) 
     {
        ERR("Failed to allocate Ecore_Evas.");
        ecore_wl_shutdown();
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_wl_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_wl_engine_func;

   ee->driver = "wayland_shm";
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->req.x = ee->x = x;
   ee->req.y = ee->y = y;
   ee->req.w = ee->w = w;
   ee->req.h = ee->h = h;
   ee->rotation = 0;
   ee->prop.max.w = ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->prop.draw_frame = frame;
   ee->prop.window = _win_id++;

   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, method);
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->prop.draw_frame) 
     evas_output_framespace_set(ee->evas, 4, 18, 8, 22);

   if ((einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas))) 
     {
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

   /* NB: we need to be notified before 'free' so we can munmap the evas 
    * engine destination */
   ecore_evas_callback_pre_free_set(ee, _ecore_evas_wl_pre_free);

   if (ee->prop.draw_frame) 
     {
        ee->engine.wl.frame = _ecore_evas_wl_frame_add(ee->evas);
        evas_object_is_frame_object_set(ee->engine.wl.frame, EINA_TRUE);
        evas_object_move(ee->engine.wl.frame, 0, 0);
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

/* local functions */
static int 
_ecore_evas_wl_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_ecore_evas_wl_init_count != 1)
     return _ecore_evas_wl_init_count;

   _ecore_evas_wl_event_handlers[0] = 
     ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, 
                             _ecore_evas_wl_event_mouse_down, NULL);
   _ecore_evas_wl_event_handlers[1] = 
     ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, 
                             _ecore_evas_wl_event_mouse_up, NULL);
   _ecore_evas_wl_event_handlers[2] = 
     ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, 
                             _ecore_evas_wl_event_mouse_move, NULL);
   _ecore_evas_wl_event_handlers[3] = 
     ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL, 
                             _ecore_evas_wl_event_mouse_wheel, NULL);
   _ecore_evas_wl_event_handlers[4] = 
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_IN, 
                             _ecore_evas_wl_event_mouse_in, NULL);
   _ecore_evas_wl_event_handlers[5] = 
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_OUT, 
                             _ecore_evas_wl_event_mouse_out, NULL);
   _ecore_evas_wl_event_handlers[6] = 
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_IN, 
                             _ecore_evas_wl_event_focus_in, NULL);
   _ecore_evas_wl_event_handlers[7] = 
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_OUT, 
                             _ecore_evas_wl_event_focus_out, NULL);

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

   for (i = 0; i < sizeof(_ecore_evas_wl_event_handlers) / sizeof(Ecore_Event_Handler *); i++) 
     {
        if (_ecore_evas_wl_event_handlers[i])
          ecore_event_handler_del(_ecore_evas_wl_event_handlers[i]);
     }

   ecore_event_evas_shutdown();

   return _ecore_evas_wl_init_count;
}

static void 
_ecore_evas_wl_pre_free(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* destroy frame */
   if (ee->engine.wl.frame) evas_object_del(ee->engine.wl.frame);

   /* get engine info */
   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if ((einfo) && (einfo->info.dest))
     {
        int ret = 0;

        /* munmap previous engine destination */
        ret = munmap(einfo->info.dest, ((ee->w * sizeof(int)) * ee->h));
     }
}

static void 
_ecore_evas_wl_free(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* destroy buffer */
   if (ee->engine.wl.buffer) wl_buffer_destroy(ee->engine.wl.buffer);
   ee->engine.wl.buffer = NULL;

   /* destroy shell surface */
   if (ee->engine.wl.shell_surface)
     wl_shell_surface_destroy(ee->engine.wl.shell_surface);
   ee->engine.wl.shell_surface = NULL;

   /* destroy surface */
   if (ee->engine.wl.surface) wl_surface_destroy(ee->engine.wl.surface);
   ee->engine.wl.surface = NULL;

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
_ecore_evas_wl_move(Ecore_Evas *ee, int x, int y) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
//   if ((ee->x == x) && (ee->y == y)) return;
   ee->req.x = x;
   ee->req.y = y;

   ee->x = x;
   ee->y = y;

   if (ee->engine.wl.shell_surface)
     {
        wl_shell_surface_move(ee->engine.wl.shell_surface, 
                              ecore_wl_input_device_get(), 
                              _ecore_evas_wl_btn_timestamp);
     }

   if (ee->func.fn_move) ee->func.fn_move(ee);
}

static void 
_ecore_evas_wl_resize(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->w == w) && (ee->h == h)) return;

   ee->req.w = w;
   ee->req.h = h;

   if (ee->visible) 
     {
        /* damage old surface, if it exists */

        /* NB: This removes any lingering screen artifacts in the compositor.
         * This may be a 'HACK' if the issue is actually in the wayland 
         * compositor, but for now lets implement this so we don't have screen 
         * artifacts laying around during a resize */
        if (ee->engine.wl.surface)
          wl_surface_damage(ee->engine.wl.surface, 0, 0, ee->w, ee->h);
     }

   /* get engine info */
   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if (einfo->info.dest)
     {
        int ret = 0;

        /* munmap previous engine destination */
        ret = munmap(einfo->info.dest, ((ee->w * sizeof(int)) * ee->h));
     }

   /* free old buffer */
   if (ee->engine.wl.buffer) wl_buffer_destroy(ee->engine.wl.buffer);
   ee->engine.wl.buffer = NULL;

   ee->w = w;
   ee->h = h;

   /* create buffer @ new size (also mmaps the new destination) */
   _ecore_evas_wl_buffer_new(ee, &einfo->info.dest);

   /* flush new buffer fd */
   ecore_wl_flush();

   /* change evas output & viewport sizes */
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   if (ee->engine.wl.frame)
     evas_object_resize(ee->engine.wl.frame, ee->w, ee->h);

   /* set new engine destination */
   evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

   /* damage buffer */
   wl_buffer_damage(ee->engine.wl.buffer, 0, 0, ee->w, ee->h);

   if (ee->visible) 
     {
        /* damage surface */
        wl_surface_damage(ee->engine.wl.surface, 0, 0, ee->w, ee->h);

        /* if visible, attach to surface */
        wl_surface_attach(ee->engine.wl.surface, ee->engine.wl.buffer, 0, 0);
     }

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void 
_ecore_evas_wl_show(Ecore_Evas *ee)
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->visible) return;

   /* get engine info */
   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);

   /* create new surface */
   ee->engine.wl.surface = 
     wl_compositor_create_surface(ecore_wl_compositor_get());
   wl_surface_set_user_data(ee->engine.wl.surface, (void *)ee->prop.window);

   /* get new shell surface */
   ee->engine.wl.shell_surface = 
     wl_shell_get_shell_surface(ecore_wl_shell_get(), ee->engine.wl.surface);

   /* add configure listener for wayland resize events */
   wl_shell_surface_add_listener(ee->engine.wl.shell_surface, 
                                 &_ecore_evas_wl_shell_surface_listener, ee);

   /* set toplevel */
   wl_shell_surface_set_toplevel(ee->engine.wl.shell_surface);

   /* create buffer @ new size (also mmaps the new destination) */
   _ecore_evas_wl_buffer_new(ee, &einfo->info.dest);

   if (ee->engine.wl.frame) 
     {
        evas_object_show(ee->engine.wl.frame);
        evas_object_resize(ee->engine.wl.frame, ee->w, ee->h);
     }

   /* set new engine destination */
   evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

   /* flush new buffer fd */
   ecore_wl_flush();

   /* attach buffer to surface */
   wl_surface_attach(ee->engine.wl.surface, ee->engine.wl.buffer, 0, 0);

   /* NB: No need to do a damage here. If we do, we end up w/ screen 
    * artifacts in the compositor */
   /* wl_surface_damage(ee->engine.wl.surface, 0, 0, ee->w, ee->h); */

   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void 
_ecore_evas_wl_hide(Ecore_Evas *ee) 
{
   Evas_Engine_Info_Wayland_Shm *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (!ee->visible) return;

   /* get engine info */
   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(ee->evas);
   if (einfo->info.dest)
     {
        int ret = 0;

        /* munmap previous engine destination */
        ret = munmap(einfo->info.dest, ((ee->w * sizeof(int)) * ee->h));
        einfo->info.dest = NULL;
     }

   /* free old buffer */
   if (ee->engine.wl.buffer) wl_buffer_destroy(ee->engine.wl.buffer);
   ee->engine.wl.buffer = NULL;

   /* set new engine destination */
   evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

   /* destroy shell surface */
   if (ee->engine.wl.shell_surface) 
     wl_shell_surface_destroy(ee->engine.wl.shell_surface);
   ee->engine.wl.shell_surface = NULL;

   /* destroy surface */
   if (ee->engine.wl.surface) wl_surface_destroy(ee->engine.wl.surface);
   ee->engine.wl.surface = NULL;

   ee->visible = 0;
   ee->should_be_visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void 
_ecore_evas_wl_raise(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   if (!ee->engine.wl.shell_surface) return;
   wl_shell_surface_set_toplevel(ee->engine.wl.shell_surface);
}

static void 
_ecore_evas_wl_lower(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   /* FIXME: Need a way to tell Wayland to lower */
}

static void 
_ecore_evas_wl_activate(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   if (!ee->engine.wl.shell_surface) return;
   wl_shell_surface_set_toplevel(ee->engine.wl.shell_surface);
}

static void 
_ecore_evas_wl_title_set(Ecore_Evas *ee, const char *t) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (t) ee->prop.title = strdup(t);

   if ((ee->prop.draw_frame) && (ee->engine.wl.frame)) 
     {
        EE_Wl_Smart_Data *sd;

        if (!(sd = evas_object_smart_data_get(ee->engine.wl.frame))) return;
        evas_object_text_text_set(sd->text, ee->prop.title);
     }
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
   /* FIXME: Forward these changes to Wayland somehow */
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
_ecore_evas_wl_object_cursor_set(Ecore_Evas *ee, Evas_Object  *obj, int layer, int hot_x, int hot_y) 
{
   int x = 0, y = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);
   ee->prop.cursor.object = NULL;

   if (!obj) 
     {
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        return;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object, 
                    x - ee->prop.cursor.hot.x, y - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, 
                                  _ecore_evas_wl_object_cursor_del, ee);
}

static void 
_ecore_evas_wl_object_cursor_del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__) 
{
   Ecore_Evas *ee;

   if (!(ee = data)) return;
   ee->prop.cursor.object = NULL;
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
_ecore_evas_wl_focus_set(Ecore_Evas *ee, int focus __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   if (!ee->engine.wl.shell_surface) return;
   wl_shell_surface_set_toplevel(ee->engine.wl.shell_surface);
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
   /* FIXME: Implement this in Wayland someshow */
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

             EINA_LIST_FOREACH(updates, l, r) 
               {
                  if (ee->engine.wl.buffer)
                    wl_buffer_damage(ee->engine.wl.buffer, 
                                     r->x, r->y, r->w, r->h);

                  if (ee->engine.wl.surface)
                    wl_surface_damage(ee->engine.wl.surface, 
                                      r->x, r->y, r->w, r->h);
               }

             evas_render_updates_free(updates);
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;

             ecore_wl_flush();
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

static Eina_Bool 
_ecore_evas_wl_event_mouse_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   _ecore_evas_wl_btn_timestamp = ev->timestamp;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   evas_event_feed_mouse_down(ee->evas, ev->buttons, ev->modifiers, 
                              ev->timestamp, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_mouse_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   evas_event_feed_mouse_up(ee->evas, ev->buttons, ev->modifiers, 
                            ev->timestamp, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Event_Mouse_Move *ev;

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ee->mouse.x = ev->x;
   ee->mouse.y = ev->y;
   evas_event_feed_mouse_move(ee->evas, ev->x, ev->y, ev->timestamp, NULL);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->timestamp);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Event_Mouse_Wheel *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   evas_event_feed_mouse_wheel(ee->evas, ev->direction, ev->z, 
                               ev->timestamp, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
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
   evas_event_feed_mouse_in(ee->evas, ev->time, NULL);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->time);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->time);
   evas_event_feed_mouse_out(ee->evas, ev->time, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_focus_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool 
_ecore_evas_wl_event_focus_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   evas_focus_out(ee->evas);
   ee->prop.focused = 0;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static void 
_ecore_evas_wl_handle_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t timestamp __UNUSED__, uint32_t edges __UNUSED__, int32_t width, int32_t height) 
{
   Ecore_Evas *ee;

   /* NB: Trap to prevent compositor from crashing */
   if ((width <= 0) || (height <= 0)) return;

   if (!(ee = data)) return;

   if ((shell_surface) && (ee->engine.wl.shell_surface)) 
     {
        if (ee->engine.wl.shell_surface != shell_surface) return;
        ecore_evas_resize(ee, width, height);
     }
}

static void 
_ecore_evas_wl_handle_popup_done(void *data __UNUSED__, struct wl_shell_surface *shell_surface __UNUSED__) 
{

}

static void 
_ecore_evas_wl_buffer_new(Ecore_Evas *ee, void **dest)
{
   static unsigned int format;
   char tmp[PATH_MAX];
   int fd = -1, stride = 0, size = 0;
   void *ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (dest) *dest = NULL;

   if (!format) format = ecore_wl_format_get();

   strcpy(tmp, "/tmp/ecore-wayland_shm-XXXXXX");
   if ((fd = mkstemp(tmp)) < 0) 
     {
        ERR("Could not create temporary file.");
        return;
     }

   stride = (ee->w * sizeof(int));
   size = (stride * ee->h);
   if (ftruncate(fd, size) < 0) 
     {
        ERR("Could not truncate temporary file.");
        close(fd);
        return;
     }

   ret = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
   unlink(tmp);

   if (ret == MAP_FAILED) 
     {
        ERR("mmap of temporary file failed.");
        close(fd);
        return;
     }

   if (dest) *dest = ret;

   ee->engine.wl.buffer = 
     wl_shm_create_buffer(ecore_wl_shm_get(), fd, ee->w, ee->h, stride, format);

   close(fd);
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

void 
_ecore_evas_wayland_shm_resize(Ecore_Evas *ee, int location)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->engine.wl.shell_surface)) return;
   wl_shell_surface_resize(ee->engine.wl.shell_surface, 
                           ecore_wl_input_device_get(), 
                           _ecore_evas_wl_btn_timestamp, location);
}

#else
EAPI Ecore_Evas *
ecore_evas_wayland_shm_new(const char *disp_name __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__, int frame __UNUSED__)
{
   return NULL;
}
#endif
