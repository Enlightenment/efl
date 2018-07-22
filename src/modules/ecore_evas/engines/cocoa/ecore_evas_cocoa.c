#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>

#include <Ecore_Cocoa.h>
#include <Evas_Engine_GL_Cocoa.h>

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include "ecore_evas_cocoa.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

static int                      _ecore_evas_init_count = 0;
static Ecore_Event_Handler      *ecore_evas_event_handlers[4];

static const char *_iface_name = "opengl_cocoa";
static const int _iface_version = 1;

static inline Ecore_Evas *
_ecore_evas_cocoa_match(Ecore_Cocoa_Object *cocoa_win)
{
   return ecore_event_window_match((Ecore_Window)cocoa_win);
}

static Eina_Bool
_ecore_evas_cocoa_event_got_focus(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window_Focused *e = event;
   Ecore_Evas                       *ee;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;

   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_cocoa_event_lost_focus(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window_Unfocused *e = event;
   Ecore_Evas                         *ee;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;

   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
   return ECORE_CALLBACK_PASS_ON;
}

static void
_ecore_evas_resize_common(Ecore_Evas *ee,
                          int         w,
                          int         h,
                          Eina_Bool   resize_cocoa)
{
   DBG("%p (old: %i %i, new: %i %i, resize_cocoa: %s)",
       ee, ee->w, ee->h, w, h, resize_cocoa ? "yes" : "no");

   ee->req.w = w;
   ee->req.h = h;

   if ((ee->w != w) || (ee->h != h))
     {
        ee->w = w;
        ee->h = h;
        if (resize_cocoa)
          ecore_cocoa_window_resize((Ecore_Cocoa_Window *)ee->prop.window, w, h);

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static Eina_Bool
_ecore_evas_cocoa_event_window_resize(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window_Resize_Request *e = event;
   Ecore_Evas                   *ee;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if (EINA_UNLIKELY(!ee))
     {
        ERR("Unregistered Ecore_Evas for Cocoa window %p", e->cocoa_window);
        return ECORE_CALLBACK_PASS_ON;
     }
   DBG("%p", ee);

   /* Do the resize */
   _ecore_evas_resize_common(ee, e->w, e->h, EINA_FALSE);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_cocoa_event_window_destroy(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window_Destroy *e = event;
   Ecore_Evas                       *ee;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if (!ee)
     {
        ERR("Unregistered Ecore_Evas for Cocoa window %p", e->cocoa_window);
        return ECORE_CALLBACK_PASS_ON;
     }
   DBG("%p", ee);

   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static int
_ecore_evas_cocoa_init(void)
{
   DBG("%i", _ecore_evas_init_count);

   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1)
     return _ecore_evas_init_count;

   ecore_event_evas_init();

   ecore_evas_event_handlers[0] =
      ecore_event_handler_add(ECORE_COCOA_EVENT_WINDOW_UNFOCUSED,
                              _ecore_evas_cocoa_event_lost_focus, NULL);
   ecore_evas_event_handlers[1] =
      ecore_event_handler_add(ECORE_COCOA_EVENT_WINDOW_FOCUSED,
                              _ecore_evas_cocoa_event_got_focus, NULL);
   ecore_evas_event_handlers[2] =
      ecore_event_handler_add(ECORE_COCOA_EVENT_WINDOW_RESIZE_REQUEST,
                              _ecore_evas_cocoa_event_window_resize, NULL);
   ecore_evas_event_handlers[3] =
      ecore_event_handler_add(ECORE_COCOA_EVENT_WINDOW_DESTROY,
                              _ecore_evas_cocoa_event_window_destroy, NULL);

   return _ecore_evas_init_count;
}

static int
_ecore_evas_cocoa_shutdown(void)
{
   DBG("%i", _ecore_evas_init_count);
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        unsigned int i;

        for (i = 0; i < EINA_C_ARRAY_LENGTH(ecore_evas_event_handlers); i++)
          ecore_event_handler_del(ecore_evas_event_handlers[i]);

        ecore_event_evas_shutdown();
     }
   return _ecore_evas_init_count;
}

static void
_ecore_evas_cocoa_free(Ecore_Evas *ee)
{
   DBG("%p", ee);

   ecore_cocoa_window_free((Ecore_Cocoa_Window *)ee->prop.window);
   ecore_event_window_unregister(ee->prop.window);
   _ecore_evas_cocoa_shutdown();
   ecore_cocoa_shutdown();
}

static void
_ecore_evas_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   if (ee) ee->func.fn_resize = func;
}

static void
_ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h)
{
   ecore_cocoa_window_size_min_set((Ecore_Cocoa_Window *)ee->prop.window, w, h);
}

static void
_ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h)
{
   ecore_cocoa_window_size_max_set((Ecore_Cocoa_Window *)ee->prop.window, w, h);
}

static void
_ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h)
{
   ecore_cocoa_window_size_step_set((Ecore_Cocoa_Window *)ee->prop.window, w, h);
}

static void
_ecore_evas_move(Ecore_Evas *ee, int x, int y)
{
   DBG("%p", ee);
   ecore_cocoa_window_move((Ecore_Cocoa_Window *)ee->prop.window, x, y);
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   DBG("%p", ee);
   _ecore_evas_resize_common(ee, w, h, EINA_TRUE);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   DBG("%p", ee);
   if ((ee->w == w) && (ee->h == h) && (x == ee->x) && (y == ee->y))
     return;

   ee->req.x = x;
   ee->req.y = y;
   ee->req.w = w;
   ee->req.h = h;
   ee->w = w;
   ee->h = h;
   ee->x = x;
   ee->y = y;

   ecore_cocoa_window_move_resize((Ecore_Cocoa_Window *)ee->prop.window, x, y, w, h);

   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}


static void
_ecore_evas_show(Ecore_Evas *ee)
{
   DBG("%p", ee);

   ecore_cocoa_window_show((Ecore_Cocoa_Window *)ee->prop.window);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);

   if (ee->visible) return;
   ee->visible = 1;
   ee->should_be_visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}


static void
_ecore_evas_hide(Ecore_Evas *ee)
{
   DBG("%p", ee);

   ecore_cocoa_window_hide((Ecore_Cocoa_Window *)ee->prop.window);

   if (ee->prop.override)
     {
        ee->prop.withdrawn = EINA_TRUE;
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   if (!ee->visible) return;
   ee->visible = 0;
   ee->should_be_visible = 0;

   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void
_ecore_evas_raise(Ecore_Evas *ee)
{
   DBG("%p", ee);

   ecore_cocoa_window_raise((Ecore_Cocoa_Window *)ee->prop.window);
}

static void
_ecore_evas_lower(Ecore_Evas *ee)
{
   DBG("%p", ee);

   ecore_cocoa_window_lower((Ecore_Cocoa_Window *)ee->prop.window);
}

static void
_ecore_evas_activate(Ecore_Evas *ee)
{
   DBG("%p", ee);

   ecore_cocoa_window_activate((Ecore_Cocoa_Window *)ee->prop.window);
}

static void
_ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   DBG("%p", ee);

   ecore_cocoa_window_iconified_set((Ecore_Cocoa_Window *)ee->prop.window, on);
}

static void
_ecore_evas_title_set(Ecore_Evas *ee, const char *title)
{
   DBG("%p: %s", ee, title);

   if (eina_streq(ee->prop.title, title)) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   ecore_cocoa_window_title_set((Ecore_Cocoa_Window *)ee->prop.window,
                                ee->prop.title);
}

static void
_ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj,
                              int layer EINA_UNUSED, int hot_x EINA_UNUSED,
                              int hot_y EINA_UNUSED)
{
   Ecore_Cocoa_Window *const win = (Ecore_Cocoa_Window *)(ee->prop.window);

   if (obj != _ecore_evas_default_cursor_image_get(ee))
     ecore_cocoa_window_cursor_show(win, EINA_FALSE);
}

static void
_ecore_evas_object_cursor_unset(Ecore_Evas *ee)
{
   Ecore_Cocoa_Window *const win = (Ecore_Cocoa_Window *)(ee->prop.window);
   ecore_cocoa_window_cursor_show(win, EINA_TRUE);
}

static void
_ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (on)
     _ecore_evas_hide(ee);
   else
     _ecore_evas_show(ee);
}

static int
_ecore_evas_engine_cocoa_init(Ecore_Evas *ee)
{
   Evas_Engine_Info_GL_Cocoa *einfo;
   const char                *driver;
   int                        rmethod;

   DBG("%p", ee);

   driver = "gl_cocoa";

   rmethod = evas_render_method_lookup(driver);
   if (!rmethod)
     return 0;

   ee->driver = driver;
   evas_output_method_set(ee->evas, rmethod);

   einfo = (Evas_Engine_Info_GL_Cocoa *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->window = (void *)ee->prop.window;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             return 0;
          }
        ecore_cocoa_window_view_set(einfo->window, einfo->view);
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        return 0;
     }

   return 1;
}

static void
_ecore_evas_screen_geometry_get(const Ecore_Evas *ee EINA_UNUSED, int *x, int *y, int *w, int *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   ecore_cocoa_screen_size_get(NULL, w, h);
   DBG("screen geometry_get  %dx%d", *w, *h);
}


static void
_ecore_evas_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_delete_request = func;
}

static Ecore_Evas_Engine_Func _ecore_cocoa_engine_func =
  {
    _ecore_evas_cocoa_free,
    _ecore_evas_callback_resize_set,
    NULL,
    NULL,
    NULL,
    _ecore_evas_callback_delete_request_set,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _ecore_evas_move, //move
    NULL,
    _ecore_evas_resize,
    _ecore_evas_move_resize,
    NULL, //rotation
    NULL, //shaped
    _ecore_evas_show,
    _ecore_evas_hide,
    _ecore_evas_raise,
    _ecore_evas_lower,
    _ecore_evas_activate,
    _ecore_evas_title_set,
    NULL,
    _ecore_evas_size_min_set,
    _ecore_evas_size_max_set,
    NULL,
    _ecore_evas_size_step_set,
    _ecore_evas_object_cursor_set,
    _ecore_evas_object_cursor_unset,
    NULL,
    NULL,
    _ecore_evas_iconified_set,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _ecore_evas_withdrawn_set,
    NULL,
    NULL,
    NULL,
    NULL, //transparent
    NULL, // profiles_set
    NULL, // profile_set

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    _ecore_evas_screen_geometry_get,
    NULL, // screen_dpi_get
    NULL,
    NULL,  // msg_send

    NULL, // fn_pointer_xy_get
    NULL, // fn_pointer_warp

    NULL, // fn_wm_rot_preferred_rotation_set
    NULL, // fn_wm_rot_available_rotations_set
    NULL, // fn_wm_rot_manual_rotation_done_set
    NULL, // fn_wm_rot_manual_rotation_done

    NULL, // fn_aux_hints_set
    NULL, // fn_animator_register
    NULL, // fn_animator_unregister

    NULL, // fn_evas_changed
    NULL, //fn_focus_device_set
    NULL, //fn_callback_focus_device_in_set
    NULL, //fn_callback_focus_device_out_set
    NULL, //fn_callback_device_mouse_in_set
    NULL, //fn_callback_device_mouse_out_set
    NULL, //fn_pointer_device_xy_get
    NULL, //fn_prepare
    NULL, //fn_last_tick_get
  };

static Ecore_Cocoa_Window *
_ecore_evas_cocoa_window_get(const Ecore_Evas *ee)
{
   /* See affectation of ee->prop.window in ecore_evas_cocoa_new_internal */
   return (Ecore_Cocoa_Window *)(ee->prop.window);
}


EAPI Ecore_Evas *
ecore_evas_cocoa_new_internal(Ecore_Cocoa_Window *parent EINA_UNUSED, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas_Interface_Cocoa *iface;

   if (!ecore_cocoa_init())
     return NULL;

   INF("x,y,w,h = %i,%i,%i,%i", x, y, w, h);

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     goto shutdown_ecore_cocoa;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_cocoa_init();

   ee->engine.func = &_ecore_cocoa_engine_func;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->visible = 0;
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
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;
   ee->prop.withdrawn = EINA_TRUE;
   ee->can_async_render = EINA_FALSE;

   ecore_evas_evas_new(ee, w, h);
   INF("ecore_evas: %p, evas: %p", ee, ee->evas);

   if (!ee->evas)
     goto free_name;

   ee->prop.window = (Ecore_Window)ecore_cocoa_window_new(x, y, w, h);

   if (!ee->prop.window)
     {
        _ecore_evas_cocoa_shutdown();
        free(ee);
        return NULL;
     }

   if (!_ecore_evas_engine_cocoa_init(ee))
     {
        _ecore_evas_cocoa_shutdown();
        free(ee);
        return NULL;
     }

   /* Interface setup */
   iface = calloc(1, sizeof(*iface));
   if (EINA_UNLIKELY(!iface))
     {
        _ecore_evas_cocoa_shutdown();
        free(ee);
        return NULL;
     }
   iface->base.name = _iface_name;
   iface->base.version = _iface_version;
   iface->window_get = _ecore_evas_cocoa_window_get;
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ecore_evas_done(ee, EINA_TRUE);

   return ee;

   //free_window:
   /* FIXME: free window here */
   //free_evas:
   // free(ee->evas);
 free_name:
   free(ee->name);
   //free_ee:
   _ecore_evas_cocoa_shutdown();
   free(ee);
 shutdown_ecore_cocoa:
   ecore_cocoa_shutdown();

   return NULL;
}
