#include "config.h"
#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "Ecore_Evas.h"
#ifdef BUILD_ECORE_EVAS_SDL
#include "Ecore_Sdl.h"
#include "Evas_Engine_SDL.h"
#endif

#ifdef BUILD_ECORE_EVAS_SDL

/* static char *ecore_evas_default_display = "0"; */
/* static Ecore_List *ecore_evas_input_devices = NULL; */

static int                      _ecore_evas_init_count = 0;
#ifndef _WIN32
static int                      _ecore_evas_fps_debug = 0;
#endif /* _WIN32 */
static Ecore_Evas               *ecore_evases = NULL;
static Ecore_Event_Handler      *ecore_evas_event_handlers[10] = {
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static Ecore_Idle_Enterer       *ecore_evas_idle_enterer = NULL;
static Ecore_Idler              *ecore_evas_event = NULL;

static const char               *ecore_evas_sdl_default = "EFL SDL";

static void
_ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   ee->mouse.x = x;
   ee->mouse.y = y;
   if (ee->prop.cursor.object)
     {
	evas_object_show(ee->prop.cursor.object);
        evas_object_move(ee->prop.cursor.object,
                         x - ee->prop.cursor.hot.x,
                         y - ee->prop.cursor.hot.y);
     }
   evas_event_feed_mouse_move(ee->evas, x, y, timestamp, NULL);
}

static Ecore_Evas *
_ecore_evas_sdl_match(void)
{
   return ecore_evases;
}

static int
_ecore_evas_sdl_event_key_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Key_Down     *e;
   Ecore_Evas                   *ee;

   e = event;
   ee = _ecore_evas_sdl_match();

   if (!ee) return 1;
   /* pass on event */
   evas_event_feed_key_down(ee->evas, e->keyname, NULL, e->keycompose, NULL, e->time, NULL);

   return 0; /* dont pass it on */
}

static int
_ecore_evas_sdl_event_key_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Key_Up       *e;
   Ecore_Evas                   *ee;

   e = event;
   ee = _ecore_evas_sdl_match();

   if (!ee) return 1;
   /* pass on event */
   evas_event_feed_key_up(ee->evas, e->keyname, NULL, e->keycompose, NULL, e->time, NULL);

   return 0;
}

static int
_ecore_evas_sdl_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Mouse_Move *e;
   Ecore_Evas *ee;

   e = event;
   ee = _ecore_evas_sdl_match();

   if (!ee) return 1; /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);

   return 0;
}

static int
_ecore_evas_sdl_event_button_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Mouse_Button_Down    *e;
   Ecore_Evas                           *ee;
   Evas_Button_Flags                    flags;

   e = event;
   ee = _ecore_evas_sdl_match();
   flags = EVAS_BUTTON_NONE;

   if (!ee) return 1;
   /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   evas_event_feed_mouse_down(ee->evas, e->button, flags, e->time, NULL);

   return 0;
}

static int
_ecore_evas_sdl_event_button_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Mouse_Button_Up      *e;
   Ecore_Evas                           *ee;
   Evas_Button_Flags                    flags;

   e = event;
   ee = _ecore_evas_sdl_match();
   flags = EVAS_BUTTON_NONE;

   if (!ee) return 1;
   /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   evas_event_feed_mouse_up(ee->evas, e->button, flags, e->time, NULL);

   return 0;
}

static int
_ecore_evas_sdl_event_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Mouse_Wheel  *e;
   Ecore_Evas                   *ee;

   e = event;
   ee = _ecore_evas_sdl_match();

   if (!ee) return 1; /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
   evas_event_feed_mouse_wheel(ee->evas, e->direction, e->wheel, e->time, NULL);

   return 0;
}

static int
_ecore_evas_sdl_event_got_focus(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                   *ee;

   ee = _ecore_evas_sdl_match();

   if (!ee) return 1;
   /* pass on event */
   ee->prop.focused = 1;

   return 0;
}

static int
_ecore_evas_sdl_event_lost_focus(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                   *ee;

   ee = _ecore_evas_sdl_match();

   if (!ee) return 1;
   /* pass on event */
   ee->prop.focused = 0;

   return 0;
}

static int
_ecore_evas_sdl_event_video_resize(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Sdl_Event_Video_Resize *e;
   Ecore_Evas                   *ee;

   e = event;
   ee = _ecore_evas_sdl_match();

   if (!ee) return 1; /* pass on event */
   evas_output_size_set(ee->evas, e->w, e->h);

   return 0;
}

static int
_ecore_evas_sdl_event_video_expose(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Ecore_Evas                   *ee;
   int                          w;
   int                          h;

   ee = _ecore_evas_sdl_match();

   if (!ee) return 1;
   evas_output_size_get(ee->evas, &w, &h);
   evas_damage_rectangle_add(ee->evas, 0, 0, w, h);

   return 0;
}

static void
_ecore_evas_render(Ecore_Evas *ee)
{
   Eina_List *updates;

   updates = evas_render_updates(ee->evas);
   if (updates)
     {
	evas_render_updates_free(updates);
	_ecore_evas_idle_timeout_update(ee);
     }
}

static int
_ecore_evas_idle_enter(void *data __UNUSED__)
{
   Ecore_List2  *l;
   double       t1 = 0.;
   double       t2 = 0.;

#ifndef _WIN32
   if (_ecore_evas_fps_debug)
     {
	t1 = ecore_time_get();
     }
#endif /* _WIN32 */
   for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;

	ee = (Ecore_Evas *)l;

	if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

	if (ee->prop.avoid_damage) _ecore_evas_render(ee);
	else if ((ee->visible) ||
		 ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
		 ((ee->should_be_visible) && (ee->prop.override)))
	  _ecore_evas_render(ee);
	else
	  evas_norender(ee->evas);

	if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
     }
#ifndef _WIN32
   if (_ecore_evas_fps_debug)
     {
	t2 = ecore_time_get();
	_ecore_evas_fps_debug_rendertime_add(t2 - t1);
     }
#endif /* _WIN32 */
   return 1;
}

static int
_ecore_evas_sdl_event(void *data)
{
   ecore_sdl_feed_events();

   return 1;
}

static int
_ecore_evas_sdl_init(int w, int h)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

#ifndef _WIN32
   if (getenv("ECORE_EVAS_FPS_DEBUG")) _ecore_evas_fps_debug = 1;
#endif /* _WIN32 */
   ecore_evas_idle_enterer = ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   ecore_evas_event = ecore_timer_add(0.008, _ecore_evas_sdl_event, NULL);
#ifndef _WIN32
   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_init();
#endif /* _WIN32 */

   ecore_evas_event_handlers[0] = ecore_event_handler_add(ECORE_SDL_EVENT_KEY_DOWN, _ecore_evas_sdl_event_key_down, NULL);
   ecore_evas_event_handlers[1] = ecore_event_handler_add(ECORE_SDL_EVENT_KEY_UP, _ecore_evas_sdl_event_key_up, NULL);
   ecore_evas_event_handlers[2] = ecore_event_handler_add(ECORE_SDL_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_sdl_event_button_down, NULL);
   ecore_evas_event_handlers[3] = ecore_event_handler_add(ECORE_SDL_EVENT_MOUSE_BUTTON_UP, _ecore_evas_sdl_event_button_up, NULL);
   ecore_evas_event_handlers[4] = ecore_event_handler_add(ECORE_SDL_EVENT_MOUSE_MOVE, _ecore_evas_sdl_event_mouse_move, NULL);
   ecore_evas_event_handlers[5] = ecore_event_handler_add(ECORE_SDL_EVENT_MOUSE_WHEEL, _ecore_evas_sdl_event_mouse_wheel, NULL);
   ecore_evas_event_handlers[6] = ecore_event_handler_add(ECORE_SDL_EVENT_GOT_FOCUS, _ecore_evas_sdl_event_got_focus, NULL);
   ecore_evas_event_handlers[7] = ecore_event_handler_add(ECORE_SDL_EVENT_LOST_FOCUS, _ecore_evas_sdl_event_lost_focus, NULL);
   ecore_evas_event_handlers[8] = ecore_event_handler_add(ECORE_SDL_EVENT_RESIZE, _ecore_evas_sdl_event_video_resize, NULL);
   ecore_evas_event_handlers[9] = ecore_event_handler_add(ECORE_SDL_EVENT_EXPOSE, _ecore_evas_sdl_event_video_expose, NULL);

   return _ecore_evas_init_count;
}

static int
_ecore_evas_sdl_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
	int i;

	while (ecore_evases) _ecore_evas_free(ecore_evases);
        for (i = 0; i < sizeof (ecore_evas_event_handlers) / sizeof (Ecore_Event_Handler*); i++)
	  ecore_event_handler_del(ecore_evas_event_handlers[i]);
	ecore_idle_enterer_del(ecore_evas_idle_enterer);
	ecore_evas_idle_enterer = NULL;
        ecore_timer_del(ecore_evas_event);
        ecore_evas_event = NULL;
#ifndef _WIN32
	if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
#endif /* _WIN32 */
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_sdl_free(Ecore_Evas *ee)
{
   ecore_evases = _ecore_list2_remove(ecore_evases, ee);
   _ecore_evas_sdl_shutdown();
   ecore_sdl_shutdown();
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

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x __UNUSED__, int y __UNUSED__, int w, int h)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;

   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (obj == NULL)
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
}

static const Ecore_Evas_Engine_Func _ecore_sdl_engine_func =
{
   _ecore_evas_sdl_free,
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
   NULL
};

static Ecore_Evas*
_ecore_evas_internal_sdl_new(int rmethod, const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha)
{
   Evas_Engine_Info_SDL *einfo;
   Ecore_Evas           *ee;

   if (!name)
     name = ecore_evas_sdl_default;

   if (ecore_evases) return NULL;

   if (!ecore_sdl_init(name)) return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_sdl_init(w, h);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_sdl_engine_func;

   ee->driver = "sdl";
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
   ee->prop.fullscreen = fullscreen;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);

   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   einfo = (Evas_Engine_Info_SDL*) evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.rotation = 0;
        einfo->info.fullscreen = fullscreen;
        einfo->info.hwsurface = hwsurface;
        einfo->info.noframe = noframe;
        einfo->info.alpha = alpha;
        evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
     }
   evas_key_modifier_add(ee->evas, "Shift");
   evas_key_modifier_add(ee->evas, "Control");
   evas_key_modifier_add(ee->evas, "Alt");
   evas_key_modifier_add(ee->evas, "Meta");
   evas_key_modifier_add(ee->evas, "Hyper");
   evas_key_modifier_add(ee->evas, "Super");
   evas_key_lock_add(ee->evas, "Caps_Lock");
   evas_key_lock_add(ee->evas, "Num_Lock");
   evas_key_lock_add(ee->evas, "Scroll_Lock");

   evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);

   SDL_ShowCursor(SDL_DISABLE);

   ecore_evases = _ecore_list2_prepend(ecore_evases, ee);
   return ee;
}
#endif

EAPI Ecore_Evas*
ecore_evas_sdl_new(const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha)
{
#ifdef BUILD_ECORE_EVAS_SDL
   int                  rmethod;

   rmethod = evas_render_method_lookup("software_sdl");
   if (!rmethod) return NULL;

   return _ecore_evas_internal_sdl_new(rmethod, name, w, h, fullscreen, hwsurface, noframe, alpha);
#else
   fprintf(stderr, "OUTCH !\n");
   return NULL;
#endif
}

EAPI Ecore_Evas*
ecore_evas_sdl16_new(const char* name, int w, int h, int fullscreen, int hwsurface, int noframe, int alpha)
{
#ifdef BUILD_ECORE_EVAS_SDL
   int                  rmethod;

   rmethod = evas_render_method_lookup("software_16_sdl");
   if (!rmethod) return NULL;

   return _ecore_evas_internal_sdl_new(rmethod, name, w, h, fullscreen, hwsurface, noframe, alpha);
#else
   fprintf(stderr, "OUTCH !\n");
   return NULL;
#endif
}
