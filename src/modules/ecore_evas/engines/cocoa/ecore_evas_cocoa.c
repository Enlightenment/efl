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
// FIXME: In case we have a lot of windows per app, we should probably use another container
// like a rbtree or a dictionnary-based container
static Eina_List                *ecore_evases = NULL;
static Ecore_Event_Handler      *ecore_evas_event_handlers[4];

static const char *_iface_name = "opengl_cocoa";
static const int _iface_version = 1;

static int
_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;

   if (ee->prop.avoid_damage)
     {
        if (updates)
          {
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }

     }
   else if (((ee->visible) && (ee->draw_ok)) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     {
        if (updates)
          {
             if (ee->shaped)
               {
                  //TODO
               }
             if (ee->alpha)
               {
                  //TODO
               }
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }
     }
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);


   if (rend)
     {
        static int frames = 0;
        static double t0 = 0.0;
        double t, td;

        t = ecore_time_get();
        frames++;
        if ((t - t0) > 1.0)
          {
             td = t - t0;
             DBG("FPS: %3.3f", (double)frames / td);
             frames = 0;
             t0 = t;
          }
     }

   return rend;
}

static int
_ecore_evas_cocoa_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *ll;
   Ecore_Evas *ee2;
   static int render2 = -1;

   if ((!ee->no_comp_sync) && (_ecore_evas_app_comp_sync))
     return 0;

   if (ee->in_async_render)
     {
        DBG("ee=%p is rendering asynchronously, skip.", ee);
        return 0;
     }

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
   if (render2 == -1)
     {
        if (getenv("RENDER2")) render2 = 1;
        else render2 = 0;
     }
   if (render2)
     {
        if (!ee->can_async_render)
          {
             Eina_List *updates = evas_render2_updates(ee->evas);
             rend = _render_updates_process(ee, updates);
             evas_render_updates_free(updates);
          }
        else
          {
             ee->in_async_render = EINA_TRUE;
             if (evas_render2(ee->evas)) rend = 1;
             else ee->in_async_render = EINA_FALSE;
          }
     }
   else
     {
        if (!ee->can_async_render)
          {
             Eina_List *updates = evas_render_updates(ee->evas);
             rend = _render_updates_process(ee, updates);
             evas_render_updates_free(updates);
          }
        else if (evas_render_async(ee->evas))
          {
             DBG("ee=%p started asynchronous render.", ee);
             ee->in_async_render = EINA_TRUE;
             rend = 1;
          }
        else if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
     }
   return rend;
}


static Ecore_Evas *
_ecore_evas_cocoa_match(Ecore_Cocoa_Object *cocoa_win)
{
   Eina_List *it;
   Ecore_Evas *ee;

   DBG("");
   EINA_LIST_FOREACH(ecore_evases, it, ee)
     {
        if (ecore_cocoa_window_get((Ecore_Cocoa_Window *)ee->prop.window) == cocoa_win)
          return ee;
     }
   return NULL;
}


static Eina_Bool
_ecore_evas_cocoa_event_got_focus(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window     *e = event;
   Ecore_Evas                   *ee;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;

   ee->prop.focused = EINA_TRUE;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_cocoa_event_lost_focus(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window     *e = event;
   Ecore_Evas                   *ee;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;

   evas_focus_out(ee->evas);
   ee->prop.focused = EINA_FALSE;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static void
_ecore_evas_resize_common(Ecore_Evas *ee,
                          int         w,
                          int         h,
                          Eina_Bool   resize_cocoa)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->req.w = w;
   ee->req.h = h;
   ee->w = w;
   ee->h = h;

   DBG("Ecore_Evas Resize %d %d", w, h);

   if (resize_cocoa)
     ecore_cocoa_window_resize((Ecore_Cocoa_Window *)ee->prop.window, w, h);

   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static Eina_Bool
_ecore_evas_cocoa_event_video_resize(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Video_Resize *e = event;
   Ecore_Evas                   *ee;

   DBG("");

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if (EINA_UNLIKELY(!ee))
     {
        ERR("Unregistered Ecore_Evas for Cocoa window %p", e->cocoa_window);
        return ECORE_CALLBACK_PASS_ON;
     }

   /* Do the resize */
   _ecore_evas_resize_common(ee, e->w, e->h, EINA_FALSE);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_cocoa_event_window_destroy(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Cocoa_Event_Window     *e = event;
   Ecore_Evas                   *ee;

   DBG("");

   if (!e->cocoa_window)
     return ECORE_CALLBACK_PASS_ON;

   ee = _ecore_evas_cocoa_match(e->cocoa_window);
   if (!ee)
     {
        ERR("Unregistered Ecore_Evas for Cocoa window %p", e->cocoa_window);
        return ECORE_CALLBACK_PASS_ON;
     }

   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static int
_ecore_evas_cocoa_init(void)
{
   DBG("");
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1)
     return _ecore_evas_init_count;

   ecore_event_evas_init();

   ecore_evas_event_handlers[0] = ecore_event_handler_add(ECORE_COCOA_EVENT_GOT_FOCUS, _ecore_evas_cocoa_event_got_focus, NULL);
   ecore_evas_event_handlers[1] = ecore_event_handler_add(ECORE_COCOA_EVENT_LOST_FOCUS, _ecore_evas_cocoa_event_lost_focus, NULL);
   ecore_evas_event_handlers[2] = ecore_event_handler_add(ECORE_COCOA_EVENT_RESIZE, _ecore_evas_cocoa_event_video_resize, NULL);
   ecore_evas_event_handlers[3] = ecore_event_handler_add(ECORE_COCOA_EVENT_WINDOW_DESTROY, _ecore_evas_cocoa_event_window_destroy, NULL);

   return _ecore_evas_init_count;
}

static int
_ecore_evas_cocoa_shutdown(void)
{
   Ecore_Evas *ee;
   DBG("");
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        unsigned int i;

        EINA_LIST_FREE(ecore_evases, ee)
          _ecore_evas_free(ee);

        for (i = 0; i < EINA_C_ARRAY_LENGTH(ecore_evas_event_handlers); i++)
          ecore_event_handler_del(ecore_evas_event_handlers[i]);

        ecore_event_evas_shutdown();
     }
   return _ecore_evas_init_count;
}

static void
_ecore_evas_cocoa_free(Ecore_Evas *ee)
{
   DBG("");
   ecore_evases = eina_list_remove(ecore_evases, ee);
   ecore_event_window_unregister(0);
   _ecore_evas_cocoa_shutdown();
   ecore_cocoa_shutdown();
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
   DBG("");
   ecore_cocoa_window_move((Ecore_Cocoa_Window *)ee->prop.window, x, y);
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   DBG("");
   _ecore_evas_resize_common(ee, w, h, EINA_TRUE);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   DBG("");
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
   DBG("");
   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_cocoa_render(ee);

   ecore_cocoa_window_show((Ecore_Cocoa_Window *)ee->prop.window);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
}


static void
_ecore_evas_hide(Ecore_Evas *ee)
{
   DBG("");

   ecore_cocoa_window_hide((Ecore_Cocoa_Window *)ee->prop.window);
   ee->should_be_visible = 0;
}

static void
_ecore_evas_raise(Ecore_Evas *ee)
{
   DBG("");

   ecore_cocoa_window_raise((Ecore_Cocoa_Window *)ee->prop.window);
}

static void
_ecore_evas_lower(Ecore_Evas *ee)
{
   DBG("");

   ecore_cocoa_window_lower((Ecore_Cocoa_Window *)ee->prop.window);
}

static void
_ecore_evas_activate(Ecore_Evas *ee)
{
   DBG("");

   ecore_cocoa_window_activate((Ecore_Cocoa_Window *)ee->prop.window);
}

static void
_ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   DBG("");

   ecore_cocoa_window_iconified_set((Ecore_Cocoa_Window *)ee->prop.window, on);
}

static void
_ecore_evas_title_set(Ecore_Evas *ee, const char *title)
{
   INF("ecore evas title set");

   if (eina_streq(ee->prop.title, title)) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   ecore_cocoa_window_title_set((Ecore_Cocoa_Window *)ee->prop.window,
                                ee->prop.title);
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   DBG("");

   ee = data;
   if (ee)
     ee->prop.cursor.object = NULL;
}

static void
_ecore_evas_object_cursor_unset(Ecore_Evas *ee)
{
   evas_object_event_callback_del_full(ee->prop.cursor.object, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
}

static void
_ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y;
   Evas_Object *old;
   Ecore_Cocoa_Window *win = (Ecore_Cocoa_Window *)(ee->prop.window);
   DBG("");

   old = ee->prop.cursor.object;
   if (obj == NULL)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        ecore_cocoa_window_cursor_show(win, EINA_TRUE);
        goto end;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   if (obj != old)
     {
        ecore_cocoa_window_cursor_show(win, EINA_FALSE);
        evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
        evas_object_pass_events_set(ee->prop.cursor.object, 1);
        if (evas_pointer_inside_get(ee->evas))
          evas_object_show(ee->prop.cursor.object);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _ecore_evas_object_cursor_del, ee);
     }
   evas_object_move(ee->prop.cursor.object,
                    x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);

end:
   if ((old) && (obj != old))
     {
        evas_object_event_callback_del_full(old, EVAS_CALLBACK_DEL,
                                            _ecore_evas_object_cursor_del, ee);
        evas_object_del(old);
     }
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

   DBG("");

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
    NULL,
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

    NULL, // render
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
    NULL  // fn_animator_unregister
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

   DBG("");
   
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     goto shutdown_ecore_cocoa;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_cocoa_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_cocoa_engine_func;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->visible = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w - ee->y;
   ee->req.h = ee->h;

   ee->semi_sync = 1;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;
   ee->prop.window = 0;
   ee->prop.withdrawn = EINA_TRUE;

   ee->evas = evas_new();

   if (!ee->evas)
     goto free_name;

   evas_data_attach_set(ee->evas, ee);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

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

   ee->engine.func->fn_render = _ecore_evas_cocoa_render;
   _ecore_evas_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   ecore_evases = eina_list_append(ecore_evases, ee);

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
