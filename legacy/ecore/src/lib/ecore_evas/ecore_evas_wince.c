/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include "Ecore.h"
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
# include "Ecore_WinCE.h"
# include "ecore_wince_private.h"
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_WINCE */

#include "Ecore_Evas.h"

#include "ecore_private.h"
#include "ecore_evas_private.h"

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE

#define ECORE_EVAS_EVENT_COUNT 12

static int _ecore_evas_init_count = 0;
static int _ecore_evas_fps_debug  = 0;

static Ecore_Event_Handler *ecore_evas_event_handlers[ECORE_EVAS_EVENT_COUNT];
static Ecore_Idle_Enterer  *ecore_evas_idle_enterer = NULL;
static Ecore_Evas          *ecore_evases = NULL;
static Eina_Hash           *ecore_evases_hash = NULL;

static int _ecore_evas_wince_event_key_down(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_key_up(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_mouse_button_down(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_mouse_button_up(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event);

static int _ecore_evas_wince_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event);

/* Private functions */

static void
_ecore_evas_wince_render(Ecore_Evas *ee)
{
   Eina_List *updates = NULL;
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Eina_List *ll;
   Ecore_Evas *ee2;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
	if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
	_ecore_evas_buffer_render(ee2);
	if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
#endif
   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
   if (ee->prop.avoid_damage)
     {
        updates = evas_render_updates(ee->evas);
        if (updates) evas_render_updates_free(updates);
     }
   else if ((ee->visible) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     {
        if (ee->shaped)
          {
             updates = evas_render_updates(ee->evas);
             if (updates) evas_render_updates_free(updates);
          }
        else
          {
             updates = evas_render_updates(ee->evas);
             if (updates) evas_render_updates_free(updates);
          }
     }
   else
     evas_norender(ee->evas);
   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
}

static int
_ecore_evas_wince_idle_enter(void *data __UNUSED__)
{
   Ecore_List2 *l;
   double       t1 = 0.0;
   double       t2 = 0.0;

   if (!ecore_evases) return 1;
   if (_ecore_evas_fps_debug)
     {
	t1 = ecore_time_get();
     }
   for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;

	ee = (Ecore_Evas *)l;
	_ecore_evas_wince_render(ee);
     }
   if (_ecore_evas_fps_debug)
     {
	t2 = ecore_time_get();
	_ecore_evas_fps_debug_rendertime_add(t2 - t1);
     }
   return 1;
}

static int
_ecore_evas_wince_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1)
     return _ecore_evas_init_count;

   if (getenv("ECORE_EVAS_FPS_DEBUG"))
     _ecore_evas_fps_debug = 1;

   ecore_evas_idle_enterer = ecore_idle_enterer_add(_ecore_evas_wince_idle_enter, NULL);

   ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_WINCE_EVENT_KEY_DOWN, _ecore_evas_wince_event_key_down, NULL);
   ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_WINCE_EVENT_KEY_UP, _ecore_evas_wince_event_key_up, NULL);
   ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_wince_event_mouse_button_down, NULL);
   ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_BUTTON_UP, _ecore_evas_wince_event_mouse_button_up, NULL);
   ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_MOVE, _ecore_evas_wince_event_mouse_move, NULL);
   ecore_evas_event_handlers[5]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_IN, _ecore_evas_wince_event_mouse_in, NULL);
   ecore_evas_event_handlers[6]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_OUT, _ecore_evas_wince_event_mouse_out, NULL);
   ecore_evas_event_handlers[7]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_DAMAGE, _ecore_evas_wince_event_window_damage, NULL);
   ecore_evas_event_handlers[8]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_DESTROY, _ecore_evas_wince_event_window_destroy, NULL);
   ecore_evas_event_handlers[9]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_SHOW, _ecore_evas_wince_event_window_show, NULL);
   ecore_evas_event_handlers[10]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_HIDE, _ecore_evas_wince_event_window_hide, NULL);
   ecore_evas_event_handlers[11]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST, _ecore_evas_wince_event_window_delete_request, NULL);

   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_init();

   return _ecore_evas_init_count;
}

int
_ecore_evas_wince_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
	int i;

	while (ecore_evases) _ecore_evas_free(ecore_evases);
	for (i = 0; i < ECORE_EVAS_EVENT_COUNT; i++)
	  ecore_event_handler_del(ecore_evas_event_handlers[i]);
	ecore_idle_enterer_del(ecore_evas_idle_enterer);
	ecore_evas_idle_enterer = NULL;
	if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
     }

   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;

   return _ecore_evas_init_count;
}

static char *
_ecore_evas_wince_winid_str_get(Ecore_WinCE_Window *window)
{
   static char  id[9];
   const char  *vals = "qWeRtYuIoP5-$&<~";
   unsigned int val;

   val = (unsigned int)window;
   id[0] = vals[(val >> 28) & 0xf];
   id[1] = vals[(val >> 24) & 0xf];
   id[2] = vals[(val >> 20) & 0xf];
   id[3] = vals[(val >> 16) & 0xf];
   id[4] = vals[(val >> 12) & 0xf];
   id[5] = vals[(val >>  8) & 0xf];
   id[6] = vals[(val >>  4) & 0xf];
   id[7] = vals[(val      ) & 0xf];
   id[8] = 0;

   return id;
}

static Ecore_Evas *
_ecore_evas_wince_match(Ecore_WinCE_Window *window)
{
   Ecore_Evas *ee;

   ee = eina_hash_find(ecore_evases_hash, _ecore_evas_wince_winid_str_get(window));

   return ee;
}

static void
_ecore_evas_wince_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   ee->mouse.x = x;
   ee->mouse.y = y;
   if (ee->prop.cursor.object)
     {
	evas_object_show(ee->prop.cursor.object);
	if (ee->rotation == 0)
	  evas_object_move(ee->prop.cursor.object,
			   x - ee->prop.cursor.hot.x,
			   y - ee->prop.cursor.hot.y);
	else if (ee->rotation == 90)
	  evas_object_move(ee->prop.cursor.object,
			   ee->h - y - 1 - ee->prop.cursor.hot.x,
			   x - ee->prop.cursor.hot.y);
	else if (ee->rotation == 180)
	  evas_object_move(ee->prop.cursor.object,
			   ee->w - x - 1 - ee->prop.cursor.hot.x,
			   ee->h - y - 1 - ee->prop.cursor.hot.y);
	else if (ee->rotation == 270)
	  evas_object_move(ee->prop.cursor.object,
			   y - ee->prop.cursor.hot.x,
			   ee->w - x - 1 - ee->prop.cursor.hot.y);
     }
   if (ee->rotation == 0)
     evas_event_feed_mouse_move(ee->evas, x, y, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_feed_mouse_move(ee->evas, ee->h - y - 1, x, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_feed_mouse_move(ee->evas, ee->w - x - 1, ee->h - y - 1, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_feed_mouse_move(ee->evas, y, ee->w - x - 1, timestamp, NULL);
}

static int
_ecore_evas_wince_event_key_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                 *ee;
   Ecore_WinCE_Event_Key_Down *e;

   e = event;

   EINA_ERROR_PINFO("key down (%f %s %s %s)\n", e->time, e->keyname, e->keysymbol, e->keycompose);

   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   evas_event_feed_key_down(ee->evas, e->keyname, e->keysymbol, e->keycompose, NULL, e->time, NULL);

   return 1;
}

static int
_ecore_evas_wince_event_key_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas               *ee;
   Ecore_WinCE_Event_Key_Up *e;

   e = event;

   EINA_ERROR_PINFO("key up (%f %s %s %s)\n", e->time, e->keyname, e->keysymbol, e->keycompose);

   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   evas_event_feed_key_up(ee->evas, e->keyname, e->keysymbol, e->keycompose, NULL, e->time, NULL);

   return 1;
}

static int
_ecore_evas_wince_event_mouse_button_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                          *ee;
   Ecore_WinCE_Event_Mouse_Button_Down *e;
   Evas_Button_Flags                    flags = EVAS_BUTTON_NONE;

   e = event;

   EINA_ERROR_PINFO("mouse button down (%f %dx%d)\n", e->time, e->x, e->y);

   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   evas_event_feed_mouse_down(ee->evas, e->button, flags, e->time, NULL);

   return 1;
}

static int
_ecore_evas_wince_event_mouse_button_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                        *ee;
   Ecore_WinCE_Event_Mouse_Button_Up *e;
   Evas_Button_Flags                  flags = EVAS_BUTTON_NONE;

   e = event;

   EINA_ERROR_PINFO("mouse button up (%f %dx%d)\n", e->time, e->x, e->y);

   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   evas_event_feed_mouse_up(ee->evas, e->button, flags, e->time, NULL);

   return 1;
}

static int
_ecore_evas_wince_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                   *ee;
   Ecore_WinCE_Event_Mouse_Move *e;

   EINA_ERROR_PINFO("mouse move\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   _ecore_evas_wince_mouse_move_process(ee, e->x, e->y, e->time);

   return 1;
}

static int
_ecore_evas_wince_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                 *ee;
   Ecore_WinCE_Event_Mouse_In *e;

   EINA_ERROR_PINFO("mouse in\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;

   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   evas_event_feed_mouse_in(ee->evas, e->time, NULL);
   _ecore_evas_wince_mouse_move_process(ee, e->x, e->y, e->time);

   return 1;
}

static int
_ecore_evas_wince_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                  *ee;
   Ecore_WinCE_Event_Mouse_Out *e;

   EINA_ERROR_PINFO("mouse out\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;

   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   _ecore_evas_wince_mouse_move_process(ee, e->x, e->y, e->time);

   evas_event_feed_mouse_out(ee->evas, e->time, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);

   return 1;
}

static int
_ecore_evas_wince_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                      *ee;
   Ecore_WinCE_Event_Window_Damage *e;

   EINA_ERROR_PINFO("window damage\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;

   if (ee->prop.avoid_damage)
     {
#warning [ECORE] [WINCE] No Region code
     }
   else
     {
        if (ee->rotation == 0)
          evas_damage_rectangle_add(ee->evas,
                                    e->x,
                                    e->y,
                                    e->width,
                                    e->height);
        else if (ee->rotation == 90)
          evas_damage_rectangle_add(ee->evas,
                                    ee->h - e->y - e->height,
                                    e->x,
                                    e->height,
                                    e->width);
        else if (ee->rotation == 180)
          evas_damage_rectangle_add(ee->evas,
                                    ee->w - e->x - e->width,
                                    ee->h - e->y - e->height,
                                    e->width,
                                    e->height);
        else if (ee->rotation == 270)
          evas_damage_rectangle_add(ee->evas,
                                    e->y,
                                    ee->w - e->x - e->width,
                                    e->height,
                                    e->width);
     }

   return 1;
}

static int
_ecore_evas_wince_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                       *ee;
   Ecore_WinCE_Event_Window_Destroy *e;

   EINA_ERROR_PINFO("window destroy\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   ecore_evas_free(ee);

   return 1;
}

static int
_ecore_evas_wince_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_WinCE_Event_Window_Show *e;

   EINA_ERROR_PINFO("window show\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   if (ee->visible) return 0; /* dont pass it on */
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);

   return 1;
}

static int
_ecore_evas_wince_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_WinCE_Event_Window_Hide *e;

   EINA_ERROR_PINFO("window hide\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   if (!ee->visible) return 0; /* dont pass it on */
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);

   return 1;
}

static int
_ecore_evas_wince_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                              *ee;
   Ecore_WinCE_Event_Window_Delete_Request *e;

   EINA_ERROR_PINFO("window delete request\n");

   e = event;
   ee = _ecore_evas_wince_match(e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != ee->engine.wince.window) return 1;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);

   return 1;
}


/* Ecore_Evas interface */

static void
_ecore_evas_wince_free(Ecore_Evas *ee)
{
   EINA_ERROR_PINFO("ecore evas free\n");

   ecore_wince_window_del(ee->engine.wince.window);
   eina_hash_del(ecore_evases_hash, _ecore_evas_wince_winid_str_get(ee->engine.wince.window), ee);
   ecore_evases = _ecore_list2_remove(ecore_evases, ee);
   _ecore_evas_wince_shutdown();
   ecore_wince_shutdown();
}

static void
_ecore_evas_wince_callback_delete_request_set(Ecore_Evas *ee,
                                              void (*func) (Ecore_Evas *ee))
{
   ee->func.fn_delete_request = func;
}

static void
_ecore_evas_wince_move(Ecore_Evas *ee, int x, int y)
{
  EINA_ERROR_PINFO("ecore evas move (%dx%d)\n", x, y);

   if ((x != ee->x) || (y != ee->y))
     {
        ee->x = x;
        ee->y = y;
        ecore_wince_window_move(ee->engine.wince.window, x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

static void
_ecore_evas_wince_resize(Ecore_Evas *ee, int width, int height)
{
   EINA_ERROR_PINFO("ecore evas resize (%dx%d)\n", width, height);

   if ((ee->w != width) || (ee->h != height))
     {
        ee->w = width;
        ee->h = height;
        ecore_wince_window_resize(ee->engine.wince.window, width, height);
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        /* FIXME: damage and shape */

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_ecore_evas_wince_move_resize(Ecore_Evas *ee, int x, int y, int width, int height)
{
   EINA_ERROR_PINFO("ecore evas resize (%dx%d %dx%d)\n", x, y, width, height);

   if ((ee->w != width) || (ee->h != height) || (x != ee->x) || (y != ee->y))
     {
        int change_size = 0;
        int change_pos = 0;

        if ((ee->w != width) || (ee->h != height)) change_size = 1;
        if ((x != ee->x) || (y != ee->y)) change_pos = 1;

        ee->x = x;
        ee->y = y;
        ee->w = width;
        ee->h = height;
        ecore_wince_window_move_resize(ee->engine.wince.window, x, y, width, height);
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        /* FIXME: damage and shape */
        if (change_pos)
          {
             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
        if (change_size)
          {
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
          }
     }
}

/* static void */
/* _ecore_evas_wince_rotation_set(Ecore_Evas *ee, int rotation) */
/* { */
/*    int rot_dif; */

/*    if (ee->rotation == rotation) return; */
/*    rot_dif = ee->rotation - rotation; */
/*    if (rot_dif < 0) rot_dif = -rot_dif; */
/*    if (!strcmp(ee->driver, "software_ddraw")) */
/*      { */
/* 	Evas_Engine_Info_Software_16_WinCE *einfo; */

/* 	einfo = (Evas_Engine_Info_Software_16_WinCE *)evas_engine_info_get(ee->evas); */
/* 	if (!einfo) return; */
/* 	if (rot_dif != 180) */
/* 	  { */
/* 	     int minw, minh, maxw, maxh, basew, baseh, stepw, steph; */

/* 	     einfo->info.rotation = rotation; */
/* 	     evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo); */
/* 	     if (!ee->prop.fullscreen) */
/* 	       { */
/* 		  ecore_wince_window_resize(ee->engine.wince.window, ee->h, ee->w); */
/* 		  ee->expecting_resize.w = ee->h; */
/* 		  ee->expecting_resize.h = ee->w; */
/* 	       } */
/* 	     else */
/* 	       { */
/* 		  int w, h; */

/* 		  ecore_wince_window_size_get(ee->engine.wince.window, &w, &h); */
/* 		  ecore_wince_window_resize(ee->engine.wince.window, h, w); */
/* 		  if ((rotation == 0) || (rotation == 180)) */
/* 		    { */
/* 		       evas_output_size_set(ee->evas, ee->w, ee->h); */
/* 		       evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h); */
/* 		    } */
/* 		  else */
/* 		    { */
/* 		       evas_output_size_set(ee->evas, ee->h, ee->w); */
/* 		       evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w); */
/* 		    } */
/* 		  if (ee->func.fn_resize) ee->func.fn_resize(ee); */
/* 	       } */
/* 	     ecore_evas_size_min_get(ee, &minw, &minh); */
/* 	     ecore_evas_size_max_get(ee, &maxw, &maxh); */
/* 	     ecore_evas_size_base_get(ee, &basew, &baseh); */
/* 	     ecore_evas_size_step_get(ee, &stepw, &steph); */
/* 	     ee->rotation = rotation; */
/* 	     ecore_evas_size_min_set(ee, minh, minw); */
/* 	     ecore_evas_size_max_set(ee, maxh, maxw); */
/* 	     ecore_evas_size_base_set(ee, baseh, basew); */
/* 	     ecore_evas_size_step_set(ee, steph, stepw); */
/* 	     _ecore_evas_wince_mouse_move_process(ee, ee->mouse.x, ee->mouse.y, */
/*                                                   ecore_wince_current_time_get()); */
/* 	  } */
/* 	else */
/* 	  { */
/* 	     einfo->info.rotation = rotation; */
/* 	     evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo); */
/* 	     ee->rotation = rotation; */
/* 	     _ecore_evas_wince_mouse_move_process(ee, ee->mouse.x, ee->mouse.y, */
/*                                                   ecore_wince_current_time_get()); */
/* 	     if (ee->func.fn_resize) ee->func.fn_resize(ee); */
/* 	  } */
/* 	if ((ee->rotation == 90) || (ee->rotation == 270)) */
/* 	  evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w); */
/* 	else */
/* 	  evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h); */
/*      } */
/* } */

static void
_ecore_evas_wince_show(Ecore_Evas *ee)
{
   EINA_ERROR_PINFO("ecore evas show\n");

   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_wince_render(ee);
   ecore_wince_window_show(ee->engine.wince.window);
/*    if (ee->prop.fullscreen) */
/*      ecore_wince_window_focus(ee->engine.wince.window); */
}

static void
_ecore_evas_wince_hide(Ecore_Evas *ee)
{
   EINA_ERROR_PINFO("ecore evas hide\n");

   ecore_wince_window_hide(ee->engine.wince.window);
   ee->should_be_visible = 0;
}

/* static void */
/* _ecore_evas_wince_raise(Ecore_Evas *ee) */
/* { */
/*    if (!ee->prop.fullscreen) */
/*      ecore_wince_window_raise(ee->engine.wince.window); */
/*    else */
/*      ecore_wince_window_raise(ee->engine.wince.window); */
/* } */

/* static void */
/* _ecore_evas_wince_lower(Ecore_Evas *ee) */
/* { */
/*    if (!ee->prop.fullscreen) */
/*      ecore_wince_window_lower(ee->engine.wince.window); */
/*    else */
/*      ecore_wince_window_lower(ee->engine.wince.window); */
/* } */

static void
_ecore_evas_wince_title_set(Ecore_Evas *ee, const char *title)
{
   EINA_ERROR_PINFO("ecore evas title set\n");

   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   ecore_wince_window_title_set(ee->engine.wince.window, ee->prop.title);
}

/* static void */
/* _ecore_evas_wince_size_min_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 0) width = 0; */
/*    if (height < 0) height = 0; */
/*    if ((ee->prop.min.w == width) && (ee->prop.min.h == height)) return; */
/*    ee->prop.min.w = width; */
/*    ee->prop.min.h = height; */
/*    ecore_wince_window_size_min_set(ee->engine.wince.window, width, height); */
/* } */

/* static void */
/* _ecore_evas_wince_size_max_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 0) width = 0; */
/*    if (height < 0) height = 0; */
/*    if ((ee->prop.max.w == width) && (ee->prop.max.h == height)) return; */
/*    ee->prop.max.w = width; */
/*    ee->prop.max.h = height; */
/*    ecore_wince_window_size_max_set(ee->engine.wince.window, width, height); */
/* } */

/* static void */
/* _ecore_evas_wince_size_base_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 0) width = 0; */
/*    if (height < 0) height = 0; */
/*    if ((ee->prop.base.w == width) && (ee->prop.base.h == height)) return; */
/*    ee->prop.base.w = width; */
/*    ee->prop.base.h = height; */
/*    ecore_wince_window_size_base_set(ee->engine.wince.window, width, height); */
/* } */

/* static void */
/* _ecore_evas_wince_size_step_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 1) width = 1; */
/*    if (height < 1) height = 1; */
/*    if ((ee->prop.step.w == width) && (ee->prop.step.h == height)) return; */
/*    ee->prop.step.w = width; */
/*    ee->prop.step.h = height; */
/*    ecore_wince_window_size_step_set(ee->engine.wince.window, width, height); */
/* } */

static void
_ecore_evas_wince_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
#if 0
   int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (obj == NULL)
     {
	ee->prop.cursor.object = NULL;
	ee->prop.cursor.layer = 0;
	ee->prop.cursor.hot.x = 0;
	ee->prop.cursor.hot.y = 0;
	ecore_wince_window_cursor_show(ee->engine.wince.window, 1);
	return;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_wince_window_cursor_show(ee->engine.wince.window, 0);

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object,
		    x - ee->prop.cursor.hot.x,
		    y - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);
#endif
}

/* static void */
/* _ecore_evas_wince_focus_set(Ecore_Evas *ee, int on __UNUSED__) */
/* { */
/*    ecore_wince_window_focus_set(ee->engine.wince.window); */
/* } */

/* static void */
/* _ecore_evas_wince_iconified_set(Ecore_Evas *ee, int on) */
/* { */
/* /\*    if (((ee->prop.borderless) && (on)) || *\/ */
/* /\*        ((!ee->prop.borderless) && (!on))) return; *\/ */
/*    ee->prop.iconified = on; */
/*    ecore_wince_window_iconified_set(ee->engine.wince.window, ee->prop.iconified); */
/* } */

/* static void */
/* _ecore_evas_wince_borderless_set(Ecore_Evas *ee, int on) */
/* { */
/*    if (((ee->prop.borderless) && (on)) || */
/*        ((!ee->prop.borderless) && (!on))) return; */
/*    ee->prop.borderless = on; */
/*    ecore_wince_window_borderless_set(ee->engine.wince.window, ee->prop.borderless); */
/* } */

static void
_ecore_evas_wince_fullscreen_set(Ecore_Evas *ee, int on)
{
   Evas_Engine_Info_Software_16_WinCE *einfo;
   struct _Ecore_WinCE_Window         *window;

   EINA_ERROR_PINFO("ecore evas fullscreen set\n");

   if ((ee->engine.wince.state.fullscreen && on) ||
      (!ee->engine.wince.state.fullscreen && !on))
     return;

   ee->engine.wince.state.fullscreen = on;
   ee->prop.fullscreen = on;

   window = (struct _Ecore_WinCE_Window *)ee->engine.wince.window;

   if (on != 0)
   {
/*       ecore_win32_window_shape_set(ee->engine.win32.window, 0, 0, NULL); */
      ecore_wince_window_fullscreen_set(ee->engine.wince.window, on);
      ee->w = GetSystemMetrics(SM_CXSCREEN);
      ee->h = GetSystemMetrics(SM_CYSCREEN);
      evas_output_size_set(ee->evas, ee->w, ee->h);
      evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   }
   else
   {
      int w;
      int h;

      ecore_wince_window_fullscreen_set(ee->engine.wince.window, on);
      ecore_wince_window_size_get(ee->engine.wince.window, &w, &h);
      ee->w = w;
      ee->h = h;
      evas_output_size_set(ee->evas, ee->w, ee->h);
      evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
/*       ecore_win32_window_shape_set(window, */
/*                                    window->shape.width, */
/*                                    window->shape.height, */
/*                                    window->shape.mask); */
   }

   einfo = (Evas_Engine_Info_Software_16_WinCE *)evas_engine_info_get(ecore_evas_get(ee));
   if (einfo != NULL)
     {
        einfo->info.fullscreen = !!on;
/*         einfo->info.layered = window->shape.layered; */
        evas_engine_info_set(ecore_evas_get(ee), (Evas_Engine_Info *)einfo);
     }
}

static void *
_ecore_evas_wince_window_get(const Ecore_Evas *ee)
{
   EINA_ERROR_PINFO("ecore evas window get\n");

   return ee->engine.wince.window;
}

static const Ecore_Evas_Engine_Func _ecore_wince_engine_func =
{
   _ecore_evas_wince_free,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_wince_callback_delete_request_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_wince_move,
   NULL,
   _ecore_evas_wince_resize,
   _ecore_evas_wince_move_resize,
   NULL, //_ecore_evas_wince_rotation_set,
   NULL, /* _ecore_evas_x_shaped_set */
   _ecore_evas_wince_show,
   _ecore_evas_wince_hide,
   NULL, //_ecore_evas_wince_raise,
   NULL, //_ecore_evas_wince_lower,
   NULL, //_ecore_evas_wince_activate,
   _ecore_evas_wince_title_set,
   NULL, /* _ecore_evas_x_name_class_set */
   NULL, //_ecore_evas_wince_size_min_set,
   NULL, //_ecore_evas_wince_size_max_set,
   NULL, //_ecore_evas_wince_size_base_set,
   NULL, //_ecore_evas_wince_size_step_set,
   _ecore_evas_wince_cursor_set,
   NULL, /* _ecore_evas_x_layer_set */
   NULL, //_ecore_evas_wince_focus_set,
   NULL, //_ecore_evas_wince_iconified_set,
   NULL, //_ecore_evas_wince_borderless_set,
   NULL, /* _ecore_evas_x_override_set */
   NULL,
   _ecore_evas_wince_fullscreen_set,
   NULL, /* _ecore_evas_x_avoid_damage_set */
   NULL, /* _ecore_evas_x_withdrawn_set */
   NULL, /* _ecore_evas_x_sticky_set */
   NULL, /* _ecore_evas_x_ignore_events_set */
   NULL, /* _ecore_evas_x_alpha_set */
   _ecore_evas_wince_window_get
};

/* API */

static Ecore_Evas *
ecore_evas_software_wince_new_internal(int                 backend,
                                       Ecore_WinCE_Window *parent,
                                       int                 x,
                                       int                 y,
                                       int                 width,
                                       int                 height,
                                       int                 fullscreen)
{
   Evas_Engine_Info_Software_16_WinCE *einfo;
   Ecore_Evas                         *ee;
   int                                 rmethod;

   rmethod = evas_render_method_lookup("software_16_wince");
   if (!rmethod)
     return NULL;

   if (!ecore_wince_init())
     return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     {
        ecore_wince_shutdown();
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   if (!_ecore_evas_wince_init())
     {
        free(ee);
        ecore_wince_shutdown();
        return NULL;
     }

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_wince_engine_func;

   ee->driver = "software_16_wince";

   if (width < 1) width = 1;
   if (height < 1) height = 1;
   ee->x = x;
   ee->y = y;
   ee->w = width;
   ee->h = height;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   /* FIXME: sticky to add */

   ee->engine.wince.window = ecore_wince_window_new(parent, x, y, width, height);
   if (!ee->engine.wince.window)
     {
        _ecore_evas_wince_shutdown();
        free(ee);
        ecore_wince_shutdown();
        return NULL;
     }

   ecore_wince_window_fullscreen_set(ee->engine.wince.window, fullscreen);

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, width, height);
   evas_output_viewport_set(ee->evas, 0, 0, width, height);

   einfo = (Evas_Engine_Info_Software_16_WinCE *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_WinCE_Window *)ee->engine.wince.window)->window;
        einfo->info.width = width;
        einfo->info.height = height;
        einfo->info.backend = backend;
        einfo->info.rotation = 0;
        einfo->info.fullscreen = fullscreen;
	evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

        ecore_wince_window_backend_set(ee->engine.wince.window, backend);
        ecore_wince_window_suspend_set(ee->engine.wince.window, einfo->func.suspend);
        ecore_wince_window_resume_set(ee->engine.wince.window, einfo->func.resume);
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

   ecore_evases = _ecore_list2_prepend(ecore_evases, ee);
   if (!ecore_evases_hash)
     ecore_evases_hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(ecore_evases_hash, _ecore_evas_wince_winid_str_get(ee->engine.wince.window), ee);

   return ee;
}

#else

static Ecore_Evas *
ecore_evas_software_wince_new_internal(int                 backend __UNUSED__,
                                       Ecore_WinCE_Window *parent __UNUSED__,
                                       int                 x __UNUSED__,
                                       int                 y __UNUSED__,
                                       int                 width __UNUSED__,
                                       int                 height __UNUSED__,
                                       int                 fullscreen __UNUSED__)
{
   return NULL;
}

#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_WINCE */


EAPI Ecore_Evas *
ecore_evas_software_wince_new(Ecore_WinCE_Window *parent,
                              int                 x,
                              int                 y,
                              int                 width,
                              int                 height)
{
   return ecore_evas_software_wince_new_internal(0, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent,
                                 int                 x,
                                 int                 y,
                                 int                 width,
                                 int                 height)
{
   return ecore_evas_software_wince_new_internal(1, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent,
                                   int                 x,
                                   int                 y,
                                   int                 width,
                                   int                 height)
{
   return ecore_evas_software_wince_new_internal(2, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent,
                                    int                 x,
                                    int                 y,
                                    int                 width,
                                    int                 height)
{
   return ecore_evas_software_wince_new_internal(3, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent,
                                  int                 x,
                                  int                 y,
                                  int                 width,
                                  int                 height)
{
   return ecore_evas_software_wince_new_internal(4, parent, x, y, width, height, 0);
}

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee)
{
   return (Ecore_WinCE_Window *) _ecore_evas_wince_window_get(ee);
}

#else

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return NULL;
}

#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_WINCE */
