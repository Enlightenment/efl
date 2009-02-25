/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <dirent.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "Ecore_Evas.h"
#ifdef BUILD_ECORE_EVAS_FB
#include "Ecore_Fb.h"
#include "ecore_fb_private.h"
#endif

#ifdef BUILD_ECORE_EVAS_FB
static int _ecore_evas_init_count = 0;

static int _ecore_evas_fps_debug = 0;
static char *ecore_evas_default_display = "0";
static Eina_List *ecore_evas_input_devices = NULL;
static Ecore_Evas *ecore_evases = NULL;
static Ecore_Event_Handler *ecore_evas_event_handlers[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
static Ecore_Idle_Enterer *ecore_evas_idle_enterer = NULL;

static void
_ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   int fbw, fbh;
   
   ee->mouse.x = x;
   ee->mouse.y = y;
   ecore_fb_size_get(&fbw, &fbh);
   if (ee->prop.cursor.object)
     {
	evas_object_show(ee->prop.cursor.object);
        if (ee->rotation == 0)
	  evas_object_move(ee->prop.cursor.object,
			   x - ee->prop.cursor.hot.x,
			   y - ee->prop.cursor.hot.y);
	else if (ee->rotation == 90)
	  evas_object_move(ee->prop.cursor.object,
			   (fbh - ee->h) + ee->h - y - 1 - ee->prop.cursor.hot.x,
			   x - ee->prop.cursor.hot.y);
	else if (ee->rotation == 180)
	  evas_object_move(ee->prop.cursor.object,
			   (fbw - ee->w) + ee->w - x - 1 - ee->prop.cursor.hot.x,
			   (fbh - ee->h) + ee->h - y - 1 - ee->prop.cursor.hot.y);
	else if (ee->rotation == 270)
	  evas_object_move(ee->prop.cursor.object,
			   y - ee->prop.cursor.hot.x,
			   (fbw - ee->w) + ee->w - x - 1 - ee->prop.cursor.hot.y);
     }
   if (ee->rotation == 0)
     evas_event_feed_mouse_move(ee->evas, x, y, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_feed_mouse_move(ee->evas, (fbh - ee->h) + ee->h - y - 1, x, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_feed_mouse_move(ee->evas, (fbw - ee->w) + ee->w - x - 1, (fbh - ee->h) + ee->h - y - 1, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_feed_mouse_move(ee->evas, y, (fbw - ee->w) + ee->w - x - 1, timestamp, NULL);   
}

static Ecore_Evas *
_ecore_evas_fb_match(void)
{
   return ecore_evases;
}

static void
_ecore_evas_fb_lose(void *data __UNUSED__)
{
   Ecore_List2 *l;
   Eina_List *ll;
   Ecore_Fb_Input_Device *dev;

   for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;
	
	ee = (Ecore_Evas *)l;
	ee->visible = 0;
     }
   if (ecore_evas_input_devices)
     {
	EINA_LIST_FOREACH(ecore_evas_input_devices, ll, dev)
	  ecore_fb_input_device_listen(dev, 0);
     }
}

static void
_ecore_evas_fb_gain(void *data __UNUSED__)
{
   Ecore_List2 *l;
   Eina_List *l;
   Ecore_Fb_Input_Device *dev;

   for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;
	
	ee = (Ecore_Evas *)l;
	ee->visible = 1;
	if ((ee->rotation == 90) || (ee->rotation == 270))
	  evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
	else
	  evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   if (ecore_evas_input_devices)
     {
	EINA_LIST_FOREACH(ecore_evas_input_devices, ll, dev)
	  ecore_fb_input_device_listen(dev, 1);
     }
}

static int
_ecore_evas_event_key_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Fb_Event_Key_Down *e;
   
   e = event;
   ee = _ecore_evas_fb_match();
   if (!ee) return 1; /* pass on event */
   evas_event_feed_key_down(ee->evas, e->keyname, e->keysymbol, e->key_compose, NULL, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_key_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Fb_Event_Key_Up *e;
   
   e = event;
   ee = _ecore_evas_fb_match();
   if (!ee) return 1; /* pass on event */
   evas_event_feed_key_up(ee->evas, e->keyname, e->keysymbol, e->key_compose, NULL, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_button_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Fb_Event_Mouse_Button_Down *e;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;
   
   e = event;
   ee = _ecore_evas_fb_match();
   if (!ee) return 1; /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff));
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   evas_event_feed_mouse_down(ee->evas, e->button, flags, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_button_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Fb_Event_Mouse_Button_Up *e;
   
   e = event;
   ee = _ecore_evas_fb_match();
   if (!ee) return 1; /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff));
   evas_event_feed_mouse_up(ee->evas, e->button, EVAS_BUTTON_NONE, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Fb_Event_Mouse_Move *e;
   
   e = event;
   ee = _ecore_evas_fb_match();
   if (!ee) return 1; /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff));
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_Fb_Event_Mouse_Wheel *e;
   
   e = event;
   ee = _ecore_evas_fb_match();
   if (!ee) return 1; /* pass on event */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff));
   return 0; /* dont pass it on */
}

static int
_ecore_evas_idle_enter(void *data __UNUSED__)
{
   Ecore_List2 *l;
   double t1 = 0.;
   double t2 = 0.;

   if (_ecore_evas_fps_debug)
     {
	t1 = ecore_time_get();
     }
   for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;
	
	ee = (Ecore_Evas *)l;
	if (ee->visible)
	  {
	     Eina_List *updates;
	     
#ifdef BUILD_ECORE_EVAS_BUFFER
	     Eina_List *ll;
	     Ecore_Evas *ee2;
#endif
	     if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
#ifdef BUILD_ECORE_EVAS_BUFFER
	     EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
	       {
		  if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
		  _ecore_evas_buffer_render(ee2);
		  if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
	       }
#endif	     
 	     updates = evas_render_updates(ee->evas);
	     if (updates)
	       {
		  evas_render_updates_free(updates);
		  _ecore_evas_idle_timeout_update(ee);
	       }
	     if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
	  }
	else
	  evas_norender(ee->evas);
     }
   if (_ecore_evas_fps_debug)
     {
	t2 = ecore_time_get();
	_ecore_evas_fps_debug_rendertime_add(t2 - t1);
     }
   return 1;
}

static int
_ecore_evas_fb_init(int w, int h)
{
   Ecore_Fb_Input_Device *device;
   Ecore_Fb_Input_Device_Cap caps;
   int mouse_handled = 0;
   int keyboard_handled = 0;
   
   DIR *input_dir;
   struct dirent *input_entry;
   
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;
   
   if (getenv("ECORE_EVAS_FPS_DEBUG")) _ecore_evas_fps_debug = 1;
   ecore_evas_idle_enterer = ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_init();
   /* register all input devices */
   input_dir = opendir("/dev/input/");
   if (!input_dir) return _ecore_evas_init_count;
   
   while ((input_entry = readdir(input_dir)))
     {
	char device_path[256];
	
	if (strncmp(input_entry->d_name, "event", 5) != 0)
	  continue;
	
	snprintf(device_path, 256, "/dev/input/%s", input_entry->d_name);
	if (!(device = ecore_fb_input_device_open(device_path)))
	  continue;
	
	caps = ecore_fb_input_device_cap_get(device);

	if (ecore_evas_input_devices)
	  {
	     /* Mouse */
	     if (caps & ECORE_FB_INPUT_DEVICE_CAP_RELATIVE)
	       {
		  ecore_fb_input_device_axis_size_set(device, w, h);
		  ecore_fb_input_device_listen(device,1);
		  ecore_evas_input_devices = eina_list_append(ecore_evas_input_devices, device);
		  if (!mouse_handled)
		    {
		       ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_event_mouse_button_down, NULL);
		       ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_BUTTON_UP, _ecore_evas_event_mouse_button_up, NULL);
		       ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_MOVE, _ecore_evas_event_mouse_move, NULL);
		       ecore_evas_event_handlers[5]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_WHEEL, _ecore_evas_event_mouse_wheel, NULL);
		       mouse_handled = 1;
		    }
	       }
	     /* Keyboard */
	     else if ((caps & ECORE_FB_INPUT_DEVICE_CAP_KEYS_OR_BUTTONS) && !(caps & ECORE_FB_INPUT_DEVICE_CAP_ABSOLUTE))
	       {
		  ecore_fb_input_device_listen(device,1);
		  ecore_evas_input_devices = eina_list_append(ecore_evas_input_devices, device);
		  if (!keyboard_handled)
		    {
		       ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_FB_EVENT_KEY_DOWN, _ecore_evas_event_key_down, NULL);
		       ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_FB_EVENT_KEY_UP, _ecore_evas_event_key_up, NULL);
		       keyboard_handled = 1;
		    }
	       }
	  }
     }
   if (!mouse_handled)
     {
	if (ecore_fb_ts_init())
	  {
	     ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_event_mouse_button_down, NULL);
	     ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_BUTTON_UP, _ecore_evas_event_mouse_button_up, NULL);
	     ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_MOVE, _ecore_evas_event_mouse_move, NULL);
	     mouse_handled = 1;
	  }
     }
   return _ecore_evas_init_count;
}

static void
_ecore_evas_fb_free(Ecore_Evas *ee)
{
   ecore_evases = _ecore_list2_remove(ecore_evases, ee);   
   _ecore_evas_fb_shutdown();
   ecore_fb_shutdown();
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
       evas_output_size_set(ee->evas, ee->h, ee->w);
       evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
   else
     {
       evas_output_size_set(ee->evas, ee->w, ee->h);
       evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);	
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x __UNUSED__, int y __UNUSED__, int w, int h)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
       evas_output_size_set(ee->evas, ee->h, ee->w);
       evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
   else
     {
       evas_output_size_set(ee->evas, ee->w, ee->h);
       evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);	
}

static void
_ecore_evas_rotation_set(Ecore_Evas *ee, int rotation)
{
   Evas_Engine_Info_FB *einfo;
   int rot_dif;
   
   if (ee->rotation == rotation) return;
   einfo = (Evas_Engine_Info_FB *)evas_engine_info_get(ee->evas);
   if (!einfo) return;
   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;
   if (rot_dif != 180)
     {
	
	einfo->info.rotation = rotation;
	evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	if (!ee->prop.fullscreen)
	  {
	     int tmp;
	     
	     tmp = ee->w;
	     ee->w = ee->h;
	     ee->h = tmp;
	  }
	else
	  {
	     if ((rotation == 0) || (rotation == 180))
	       {
		  evas_output_size_set(ee->evas, ee->w, ee->h);
		  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
	       }
	     else
	       {
		  evas_output_size_set(ee->evas, ee->h, ee->w);
		  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
	       }
	  }
	ee->rotation = rotation;
     }
   else
     {
	einfo->info.rotation = rotation;
	evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	ee->rotation = rotation;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   else
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff));
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
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

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
}

static void
_ecore_evas_fullscreen_set(Ecore_Evas *ee, int on)
{
   Eina_List *l;
   int resized = 0;
   
   if (((ee->prop.fullscreen) && (on)) ||
       ((!ee->prop.fullscreen) && (!on))) return;
   if (on)
     {
	int w, h;
	
	ee->engine.fb.real_w = ee->w;
	ee->engine.fb.real_h = ee->h;
	w = ee->w;
	h = ee->h;
	ecore_fb_size_get(&w, &h);
	if ((w == 0) && (h == 0))
	  {
	     w = ee->w;
	     h = ee->h;
	  }
	if ((w != ee->w) || (h != ee->h)) resized = 1;
	ee->w = w;
	ee->h = h;
	evas_output_size_set(ee->evas, ee->w, ee->h);
	evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
	evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   else
     {
	if ((ee->engine.fb.real_w != ee->w) || (ee->engine.fb.real_h != ee->h)) resized = 1;
	ee->w = ee->engine.fb.real_w;
	ee->h = ee->engine.fb.real_h;
	evas_output_size_set(ee->evas, ee->w, ee->h);
	evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
	evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   ee->prop.fullscreen = on;
   /* rescale the input device area */
   if (ecore_evas_input_devices)
     {
	Ecore_Fb_Input_Device *dev;
	
	EINA_LIST_FOREACH(ecore_evas_input_devices, l, dev)
	   ecore_fb_input_device_axis_size_set(dev, ee->wn ee->h);
     }
   if (resized)
     {
	if (ee->func.fn_resize) ee->func.fn_resize(ee); 
     }
}
    
int
_ecore_evas_fb_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
	int i;
   
	while (ecore_evases) _ecore_evas_free(ecore_evases);
	for (i = 0; i < 5; i++)
	  ecore_event_handler_del(ecore_evas_event_handlers[i]);
	ecore_idle_enterer_del(ecore_evas_idle_enterer);
	ecore_evas_idle_enterer = NULL;
	if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
	ecore_fb_ts_shutdown();
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static const Ecore_Evas_Engine_Func _ecore_fb_engine_func =
{
   _ecore_evas_fb_free,
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
     _ecore_evas_rotation_set,
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
     _ecore_evas_fullscreen_set,
     NULL,
     NULL,
     NULL,
     NULL
};
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_Evas *
ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_FB
   Evas_Engine_Info_FB *einfo;
   Ecore_Evas *ee;

   int rmethod;

   if (!disp_name)
   disp_name = ecore_evas_default_display;

   rmethod = evas_render_method_lookup("fb");
   if (!rmethod) return NULL;
   
   if (!ecore_fb_init(disp_name)) return NULL;
   ecore_fb_callback_gain_set(_ecore_evas_fb_gain, NULL);
   ecore_fb_callback_lose_set(_ecore_evas_fb_lose, NULL);
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);
   
   _ecore_evas_fb_init(w, h);
   
   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_fb_engine_func;
   
   ee->driver = "fb";
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->rotation = rotation;
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
   
   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);

   if ((rotation == 90) || (rotation == 270))
     {
       evas_output_size_set(ee->evas, h, w);
       evas_output_viewport_set(ee->evas, 0, 0, h, w);
     }
   else
     {
       evas_output_size_set(ee->evas, w, h);
       evas_output_viewport_set(ee->evas, 0, 0, w, h);
     }

   einfo = (Evas_Engine_Info_FB *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	einfo->info.virtual_terminal = 0;
	einfo->info.device_number = strtol(disp_name, NULL, 10);
	einfo->info.device_number = 0;
	einfo->info.refresh = 0;
	einfo->info.rotation = ee->rotation;
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

   ecore_evases = _ecore_list2_prepend(ecore_evases, ee);
   return ee;
#else
   disp_name = NULL;
   rotation = w = h = 0;
   return NULL;
#endif   
}
