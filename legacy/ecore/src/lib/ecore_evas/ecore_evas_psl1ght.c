#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_PSL1GHT
#include <Ecore_Psl1ght.h>
#include <Evas_Engine_PSL1GHT.h>

static int _ecore_evas_init_count = 0;

static Ecore_Evas *psl1ght_ee = NULL;
static Ecore_Event_Handler *ecore_evas_event_handlers[4] = {
   NULL, NULL, NULL, NULL
};

static const char *ecore_evas_psl1ght_default = "EFL PSL1GHT";
static int _ecore_evas_fps_debug = 0;
static Ecore_Poller *ecore_evas_event;

static unsigned int
_ecore_evas_time_get()
{
   return (unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff;
}

static Ecore_Evas *
_ecore_evas_psl1ght_match(void)
{
   return psl1ght_ee;
}

static Eina_Bool
_ecore_evas_psl1ght_event_got_focus(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Ecore_Evas *ee;

   ee = _ecore_evas_psl1ght_match();

   if (!ee) return ECORE_CALLBACK_PASS_ON;
   /* pass on event */
   ee->prop.focused = 1;

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_evas_psl1ght_event_lost_focus(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Ecore_Evas *ee;

   ee = _ecore_evas_psl1ght_match();

   if (!ee) return ECORE_CALLBACK_PASS_ON;
   /* pass on event */
   ee->prop.focused = 0;

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_evas_psl1ght_event_video_expose(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Ecore_Evas *ee;
   int w;
   int h;

   ee = _ecore_evas_psl1ght_match();

   if (!ee) return ECORE_CALLBACK_PASS_ON;
   evas_output_size_get(ee->evas, &w, &h);
   evas_damage_rectangle_add(ee->evas, 0, 0, w, h);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_evas_psl1ght_event_key_modifiers(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Psl1ght_Event_Key_Modifiers *e = event;

   ee = _ecore_evas_psl1ght_match();

   if (!ee) return ECORE_CALLBACK_PASS_ON;
   ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);

   return ECORE_CALLBACK_DONE;
}

static int
_ecore_evas_render(Ecore_Evas *ee)
{
   Eina_List *updates;

   updates = evas_render_updates(ee->evas);
   if (updates)
     {
        evas_render_updates_free(updates);
        _ecore_evas_idle_timeout_update(ee);
     }
   return updates ? 1 : 0;
}

static int
_ecore_evas_psl1ght_render(Ecore_Evas *ee)
{
   int rend = 0;

#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Eina_List *ll;
   Ecore_Evas *ee2;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        rend |= _ecore_evas_buffer_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
#endif

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   if (ee->prop.avoid_damage) rend = _ecore_evas_render(ee);
   else if ((ee->visible) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     rend |= _ecore_evas_render(ee);
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
   return rend;
}

static Eina_Bool
_ecore_evas_psl1ght_event(void *data __UNUSED__)
{
   ecore_psl1ght_poll_events();
   return ECORE_CALLBACK_RENEW;
}

static int
_ecore_evas_psl1ght_init(int w __UNUSED__, int h __UNUSED__)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

   _ecore_evas_fps_debug = 1;

   // this is pretty bad: poller? and set poll time? pol time is meant to be
   // adjustable for things like polling battery state, or amoutn of spare
   // memory etc.
   //
   ecore_evas_event = ecore_poller_add(ECORE_POLLER_CORE, 1, _ecore_evas_psl1ght_event, NULL);
   ecore_poller_poll_interval_set(ECORE_POLLER_CORE, 0.006);

   if (_ecore_evas_fps_debug)
     _ecore_evas_fps_debug_init();

   ecore_event_evas_init();

   ecore_evas_event_handlers[0] = ecore_event_handler_add(ECORE_PSL1GHT_EVENT_GOT_FOCUS, _ecore_evas_psl1ght_event_got_focus, NULL);
   ecore_evas_event_handlers[1] = ecore_event_handler_add(ECORE_PSL1GHT_EVENT_LOST_FOCUS, _ecore_evas_psl1ght_event_lost_focus, NULL);
   ecore_evas_event_handlers[2] = ecore_event_handler_add(ECORE_PSL1GHT_EVENT_EXPOSE, _ecore_evas_psl1ght_event_video_expose, NULL);
   ecore_evas_event_handlers[3] = ecore_event_handler_add(ECORE_PSL1GHT_EVENT_KEY_MODIFIERS, _ecore_evas_psl1ght_event_key_modifiers, NULL);

   return _ecore_evas_init_count;
}

static int
_ecore_evas_psl1ght_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        unsigned int i;

        for (i = 0; i < sizeof (ecore_evas_event_handlers) / sizeof (Ecore_Event_Handler *); i++)
          ecore_event_handler_del(ecore_evas_event_handlers[i]);
        ecore_event_evas_shutdown();
        ecore_poller_del(ecore_evas_event);
        ecore_evas_event = NULL;
        if (_ecore_evas_fps_debug)
          _ecore_evas_fps_debug_shutdown();
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_psl1ght_free(Ecore_Evas *ee)
{
   if (psl1ght_ee == ee) psl1ght_ee = NULL;

   ecore_event_window_unregister(0);
   _ecore_evas_psl1ght_shutdown();
   ecore_psl1ght_shutdown();
}

static void
_ecore_evas_screen_resized(Ecore_Evas *ee)
{
   int w, h;

   /* Do not resize if the window is not fullscreen */
   if (ee->prop.fullscreen == 0) return;

   ecore_psl1ght_screen_resolution_get (&w, &h);

   if (w != ee->w || h != ee->h)
     {
        ee->req.w = ee->w = w;
        ee->req.h = ee->h = h;
        evas_output_size_set(ee->evas, ee->w, ee->h);
        evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
        ecore_psl1ght_resolution_set (w, h);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       _ecore_evas_time_get());
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;

   evas_output_size_set(ee->evas, ee->w, ee->h);

   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   ecore_psl1ght_resolution_set (w, h);

   if (ee->func.fn_resize) ee->func.fn_resize(ee);

   _ecore_evas_screen_resized (ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x __UNUSED__, int y __UNUSED__, int w, int h)
{
   _ecore_evas_resize (ee, w, h);
}

static void
_ecore_evas_screen_geometry_get(const Ecore_Evas *ee __UNUSED__, int *x, int *y, int *w, int *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   ecore_psl1ght_screen_resolution_get (w, h);
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee)
     ee->prop.cursor.object = NULL;
}

static void
_ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (!obj)
     {
        ee->prop.cursor.object = NULL;
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
                    x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
}

static Ecore_Evas_Engine_Func _ecore_psl1ght_engine_func =
{
   _ecore_evas_psl1ght_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_resize,
   _ecore_evas_move_resize,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_object_cursor_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL, //transparent

   NULL, // render
   _ecore_evas_screen_geometry_get  // screen_geometry_get
};

EAPI Ecore_Evas *
ecore_evas_psl1ght_new(const char *name, int w, int h)
{
   void *einfo;
   Ecore_Evas *ee;

   if (!name)
     name = ecore_evas_psl1ght_default;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_psl1ght_engine_func;

   ee->driver = "psl1ght";
   if (name) ee->name = strdup(name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->visible = 1;
   ee->w = w;
   ee->h = h;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 1;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 1;
   ee->prop.fullscreen = 0;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;
   ee->prop.window = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, evas_render_method_lookup("psl1ght"));

   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   einfo = evas_engine_info_get(ee->evas);
   if (einfo)
     {
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }

   if (!ecore_psl1ght_init(name))
     {
        evas_free(ee->evas);
        if (ee->name) free(ee->name);
        free(ee);
        return NULL;
     }
   ecore_psl1ght_resolution_set (w, h);

   _ecore_evas_psl1ght_init(w, h);

   ecore_event_window_register(0, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   ee->engine.func->fn_render = _ecore_evas_psl1ght_render;
   _ecore_evas_register(ee);

   psl1ght_ee = ee;

   evas_event_feed_mouse_in(ee->evas, _ecore_evas_time_get (), NULL);
   evas_focus_in(ee->evas);
   _ecore_evas_screen_resized (ee);

   if (getenv("ECORE_EVAS_PSL1GHT_CURSOR_PATH"))
     ecore_evas_cursor_set(ee, getenv("ECORE_EVAS_PSL1GHT_CURSOR_PATH"), EVAS_LAYER_MAX, 0, 0);

   return ee;
}

#else /* BUILD_ECORE_EVAS_PSL1GHT */

EAPI Ecore_Evas *
ecore_evas_psl1ght_new(const char *name __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   ERR("OUTCH !");
   return NULL;
}

#endif /* BUILD_ECORE_EVAS_PSL1GHT */
