#include "config.h"
#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "Ecore_Evas.h"
#ifdef BUILD_ECORE_X
#include "Ecore_X.h"
#endif

#ifdef BUILD_ECORE_X
static int _ecore_evas_init_count = 0;

static Ecore_Evas *ecore_evases = NULL;
static Ecore_Event_Handler *ecore_evas_event_handlers[15];
static Ecore_Idle_Enterer *ecore_evas_idle_enterer = NULL;

static void
_ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y)
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
     evas_event_feed_mouse_move(ee->evas, x, y);
   else if (ee->rotation == 90)
     evas_event_feed_mouse_move(ee->evas, ee->h - y - 1, x);
   else if (ee->rotation == 180)
     evas_event_feed_mouse_move(ee->evas, ee->w - x - 1, ee->h - y - 1);
   else if (ee->rotation == 270)
     evas_event_feed_mouse_move(ee->evas, y, ee->w - x - 1);
}

static Ecore_Evas *
_ecore_evas_x_match(Ecore_X_Window win)
{
   Ecore_List *l;
   
   for (l = (Ecore_List *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;
	
	ee = (Ecore_Evas *)l;
	if ((ee->engine.x.win == win) || (ee->engine.x.win_container == win))
	  {
	     ecore_evases = _ecore_list_remove(ecore_evases, ee);
	     ecore_evases = _ecore_list_prepend(ecore_evases, ee);
	     return ee;
	  }
     }
   return NULL;
}

static void
_ecore_evas_modifier_locks_update(Ecore_Evas *ee, int modifiers)
{
   if (modifiers & ECORE_X_MODIFIER_SHIFT)
     evas_key_modifier_on(ee->evas, "Shift");
   else
     evas_key_modifier_off(ee->evas, "Shift");
   if (modifiers & ECORE_X_MODIFIER_CTRL)
     evas_key_modifier_on(ee->evas, "Control");
   else
     evas_key_modifier_off(ee->evas, "Control");
   if (modifiers & ECORE_X_MODIFIER_ALT)
     evas_key_modifier_on(ee->evas, "Alt");
   else
     evas_key_modifier_off(ee->evas, "Alt");
   if (modifiers & ECORE_X_MODIFIER_WIN)
     {
	evas_key_modifier_on(ee->evas, "Super");
	evas_key_modifier_on(ee->evas, "Hyper");
     }
   else
     {
	evas_key_modifier_off(ee->evas, "Super");
	evas_key_modifier_off(ee->evas, "Hyper");
     }
   if (modifiers & ECORE_X_LOCK_SCROLL)
     evas_key_lock_on(ee->evas, "Scroll_Lock");
   else
     evas_key_lock_off(ee->evas, "Scroll_Lock");
   if (modifiers & ECORE_X_LOCK_NUM)
     evas_key_lock_on(ee->evas, "Num_Lock");
   else
     evas_key_lock_off(ee->evas, "Num_Lock");
   if (modifiers & ECORE_X_LOCK_CAPS)
     evas_key_lock_on(ee->evas, "Caps_Lock");
   else
     evas_key_lock_off(ee->evas, "Caps_Lock");   
}

static int
_ecore_evas_event_key_down(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Key_Down *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   _ecore_evas_modifier_locks_update(ee, e->modifiers);
   evas_event_feed_key_down(ee->evas, e->keyname);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_key_up(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Key_Up *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   _ecore_evas_modifier_locks_update(ee, e->modifiers);
   evas_event_feed_key_up(ee->evas, e->keyname);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_button_down(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Button_Down *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   _ecore_evas_modifier_locks_update(ee, e->modifiers);
   _ecore_evas_mouse_move_process(ee, e->x, e->y);
   evas_event_feed_mouse_down(ee->evas, e->button);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_button_up(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Button_Up *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   _ecore_evas_modifier_locks_update(ee, e->modifiers);   
   _ecore_evas_mouse_move_process(ee, e->x, e->y);
   evas_event_feed_mouse_up(ee->evas, e->button);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_wheel(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Wheel *e;

   e = event;
   ee = _ecore_evas_x_match(e->win);

   if (!ee)
      return 1; /* pass on event */

   _ecore_evas_modifier_locks_update(ee, e->modifiers);
   _ecore_evas_mouse_move_process(ee, e->x, e->y);
   evas_event_feed_mouse_wheel(ee->evas, e->direction, e->z);

   return 0; /* don't pass it on */
}

static int
_ecore_evas_event_mouse_move(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Move *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   _ecore_evas_modifier_locks_update(ee, e->modifiers);
   _ecore_evas_mouse_move_process(ee, e->x, e->y);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_in(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_In *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   _ecore_evas_modifier_locks_update(ee, e->modifiers);
   evas_event_feed_mouse_in(ee->evas);
   _ecore_evas_mouse_move_process(ee, e->x, e->y);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_mouse_out(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Out *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   _ecore_evas_modifier_locks_update(ee, e->modifiers);   
   _ecore_evas_mouse_move_process(ee, e->x, e->y);
   evas_event_feed_mouse_out(ee->evas);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_focus_in(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Focus_In *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   ee->prop.focused = 1;
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_focus_out(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Focus_Out *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   ee->prop.focused = 0;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_damage(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Damage *e;

   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if (ee->prop.avoid_damage)
     {
	XRectangle xr;
	Region tmpr;
	
	if (!ee->engine.x.damages) ee->engine.x.damages = XCreateRegion();
	tmpr = XCreateRegion();
	xr.x = e->x;
	xr.y = e->y;
	xr.width = e->w;
	xr.height = e->h;
	XUnionRectWithRegion(&xr, ee->engine.x.damages, tmpr);
	XDestroyRegion(ee->engine.x.damages);
	ee->engine.x.damages = tmpr;
     }
   else
     {
	if (ee->rotation == 0)
	  evas_damage_rectangle_add(ee->evas, 
				    e->x, 
				    e->y, 
				    e->w, e->h);
	else if (ee->rotation == 90)
	  evas_damage_rectangle_add(ee->evas, 
				    ee->h - e->y - e->h, 
				    e->x, 
				    e->h, e->w);
	else if (ee->rotation == 180)
	  evas_damage_rectangle_add(ee->evas, 
				    ee->w - e->x - e->w, 
				    ee->h - e->y - e->h, 
				    e->w, e->h);
	else if (ee->rotation == 270)
	  evas_damage_rectangle_add(ee->evas, 
				    e->y, 
				    ee->w - e->x - e->w, 
				    e->h, e->w);
     }
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_destroy(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Destroy *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   ecore_evas_free(ee);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_configure(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Configure *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if ((ee->prop.fullscreen) && (e->win == ee->engine.x.win_container)) return 0;
   if ((e->from_wm) || (ee->prop.fullscreen) || (ee->prop.override))
     {
	if ((ee->x != e->x) || (ee->y != e->y))
	  {
	     ee->x = e->x;
	     ee->y = e->y;
	     if (ee->func.fn_move) ee->func.fn_move(ee);	     
	  }
     }
   if ((ee->w != e->w) || (ee->h != e->h))
     {
	ee->w = e->w;
	ee->h = e->h;
	if (e->win == ee->engine.x.win_container)
	  ecore_x_window_resize(ee->engine.x.win, ee->w, ee->h);
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
	if (ee->prop.avoid_damage)
	  {
	     ecore_evas_avoid_damage_set(ee, 0);
	     ecore_evas_avoid_damage_set(ee, 1);
	  }
	if (ee->shaped)
	  {
	     ecore_evas_shaped_set(ee, 0);
	     ecore_evas_shaped_set(ee, 1);
	  }
	if ((ee->expecting_resize.w > 0) &&
	    (ee->expecting_resize.h > 0))
	  {
	     if ((ee->expecting_resize.w == ee->w) &&
		 (ee->expecting_resize.h == ee->h))
	       _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y);
	     ee->expecting_resize.w = 0;
	     ee->expecting_resize.h = 0;
	  }
	if (ee->func.fn_resize) ee->func.fn_resize(ee);	
     }
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_delete_request(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Delete_Request *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_show(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Show *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if (ee->visible) return 0; /* dont pass it on */
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_event_window_hide(void *data, int type, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Hide *e;
   
   e = event;
   ee = _ecore_evas_x_match(e->win);
   if (!ee) return 1; /* pass on event */
   if (!ee->visible) return 0; /* dont pass it on */
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
   return 0; /* dont pass it on */
}

static int
_ecore_evas_idle_enter(void *data)
{
   Ecore_List *l;
   
   for (l = (Ecore_List *)ecore_evases; l; l = l->next)
     {
	Ecore_Evas *ee;
	
	ee = (Ecore_Evas *)l;
	if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
	if (ee->prop.avoid_damage)
	  {
	     Evas_List *updates, *l;
	     
	     updates = evas_render_updates(ee->evas);
	     for (l = updates; l; l = l->next)
	       {
		  Evas_Rectangle *r;
		  XRectangle xr;
		  Region tmpr;
		  
		  if (!ee->engine.x.damages)
		    ee->engine.x.damages = XCreateRegion();
		  r = l->data;
		  tmpr = XCreateRegion();
		  if (ee->rotation == 0)
		    {
		       xr.x = r->x;
		       xr.y = r->y;
		       xr.width = r->w;
		       xr.height = r->h;
		    }
		  else if (ee->rotation == 90)
		    {
		       xr.x = r->y;
		       xr.y = ee->h - r->x - r->w;
		       xr.width = r->h;
		       xr.height = r->w;
		    }
		  else if (ee->rotation == 180)
		    {
		       xr.x = ee->w - r->x - r->w;
		       xr.y = ee->h - r->y - r->h;
		       xr.width = r->w;
		       xr.height = r->h;
		    }
		  else if (ee->rotation == 270)
		    {
		       xr.x = ee->w - r->y - r->h;
		       xr.y = r->x;
		       xr.width = r->h;
		       xr.height = r->w;
		    }
		  XUnionRectWithRegion(&xr, ee->engine.x.damages, tmpr);
		  XDestroyRegion(ee->engine.x.damages);
		  ee->engine.x.damages = tmpr;
	       }
	     if (ee->engine.x.damages)
	       {
		  XSetRegion(ecore_x_display_get(), ee->engine.x.gc, ee->engine.x.damages);
		  ecore_x_pixmap_paste(ee->engine.x.pmap, ee->engine.x.win, ee->engine.x.gc,
				       0, 0, ee->w, ee->h, 0, 0);
		  XDestroyRegion(ee->engine.x.damages);
		  ee->engine.x.damages = 0;
		  if ((ee->shaped) && (updates))
		    {
		       if (ee->prop.fullscreen)
			 ecore_x_window_shape_mask_set(ee->engine.x.win, ee->engine.x.mask);
		       else
			 ecore_x_window_shape_mask_set(ee->engine.x.win_container, ee->engine.x.mask);
		    }
		  if (updates) evas_render_updates_free(updates);
	       }
	  }
	else if ((ee->visible) || 
		 ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
		 ((ee->should_be_visible) && (ee->prop.override)))
	  {
	     if (ee->shaped)
	       {
		  Evas_List *updates;
		  
		  updates = evas_render_updates(ee->evas);
		  if (updates)
		    {
		       if (ee->prop.fullscreen)
			 ecore_x_window_shape_mask_set(ee->engine.x.win, ee->engine.x.mask);
		       else
			 ecore_x_window_shape_mask_set(ee->engine.x.win_container, ee->engine.x.mask);
		       evas_render_updates_free(updates);
		    }
	       }
	     else
	       evas_render(ee->evas);
	  }
	if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
     }
   ecore_x_flush();
   return 1;
}

static int
_ecore_evas_x_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;
   ecore_evas_idle_enterer = ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_X_EVENT_KEY_DOWN, _ecore_evas_event_key_down, NULL);
   ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_X_EVENT_KEY_UP, _ecore_evas_event_key_up, NULL);
   ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_event_mouse_button_down, NULL);
   ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, _ecore_evas_event_mouse_button_up, NULL);
   ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_MOVE, _ecore_evas_event_mouse_move, NULL);
   ecore_evas_event_handlers[5]  = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN, _ecore_evas_event_mouse_in, NULL);
   ecore_evas_event_handlers[6]  = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT, _ecore_evas_event_mouse_out, NULL);
   ecore_evas_event_handlers[7]  = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, _ecore_evas_event_window_focus_in, NULL);
   ecore_evas_event_handlers[8]  = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, _ecore_evas_event_window_focus_out, NULL);
   ecore_evas_event_handlers[9]  = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DAMAGE, _ecore_evas_event_window_damage, NULL);
   ecore_evas_event_handlers[10] = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY, _ecore_evas_event_window_destroy, NULL);
   ecore_evas_event_handlers[11] = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE, _ecore_evas_event_window_configure, NULL);
   ecore_evas_event_handlers[12] = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, _ecore_evas_event_window_delete_request, NULL);
   ecore_evas_event_handlers[13] = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHOW, _ecore_evas_event_window_show, NULL);
   ecore_evas_event_handlers[14] = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_HIDE, _ecore_evas_event_window_hide, NULL);
   ecore_evas_event_handlers[15] = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_WHEEL, _ecore_evas_event_mouse_wheel, NULL);
   return _ecore_evas_init_count;
}

static void
_ecore_evas_x_free(Ecore_Evas *ee)
{
   ecore_x_window_del(ee->engine.x.win);
   ecore_x_window_del(ee->engine.x.win_container);
   if (ee->engine.x.pmap) ecore_x_pixmap_del(ee->engine.x.pmap);
   if (ee->engine.x.mask) ecore_x_pixmap_del(ee->engine.x.mask);
   if (ee->engine.x.gc) ecore_x_gc_del(ee->engine.x.gc);
   if (ee->engine.x.damages) XDestroyRegion(ee->engine.x.damages);
   ee->engine.x.pmap = 0;
   ee->engine.x.mask = 0;
   ee->engine.x.gc = 0;
   ee->engine.x.damages = 0;
   ecore_evases = _ecore_list_remove(ecore_evases, ee);
   _ecore_evas_x_shutdown();
   ecore_x_shutdown();
}

static void
_ecore_evas_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (func) ecore_x_window_prop_protocol_set(ee->engine.x.win_container, ECORE_X_WM_PROTOCOL_DELETE_REQUEST,1);
   else ecore_x_window_prop_protocol_set(ee->engine.x.win_container, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, 0);
   ee->func.fn_delete_request = func;
}

static void
_ecore_evas_move(Ecore_Evas *ee, int x, int y)
{
   ecore_x_window_move(ee->engine.x.win_container, x, y);
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   ecore_x_window_resize(ee->engine.x.win_container, w, h);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   ecore_x_window_move_resize(ee->engine.x.win_container, x, y, w, h);
}

static void
_ecore_evas_rotation_set(Ecore_Evas *ee, int rotation)
{
   Evas_Engine_Info_Software_X11 *einfo;
   int rot_dif;
   
   if (ee->rotation == rotation) return;   
   if (!strcmp(ee->driver, "gl_x11")) return;
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (!einfo) return;
   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;
   if (rot_dif != 180)
     {
	int minw, minh, maxw, maxh, basew, baseh, stepw, steph;
	
	einfo->info.rotation = rotation;
	evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	if (!ee->prop.fullscreen)
	  {
	     ecore_x_window_resize(ee->engine.x.win_container, ee->h, ee->w);
	     ee->expecting_resize.w = ee->h;
	     ee->expecting_resize.h = ee->w;
	  }
	else
	  {
	     int w, h;
	     
	     ecore_x_window_size_get(ee->engine.x.win_container, &w, &h);
	     ecore_x_window_resize(ee->engine.x.win_container, h, w);
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
	     if (ee->func.fn_resize) ee->func.fn_resize(ee);	
	  }
	ecore_evas_size_min_get(ee, &minw, &minh);
	ecore_evas_size_max_get(ee, &maxw, &maxh);
	ecore_evas_size_base_get(ee, &basew, &baseh);
	ecore_evas_size_step_get(ee, &stepw, &steph);
	ee->rotation = rotation;
	ecore_evas_size_min_set(ee, minh, minw);
	ecore_evas_size_max_set(ee, maxh, maxw);
	ecore_evas_size_base_set(ee, baseh, basew);
	ecore_evas_size_step_set(ee, steph, stepw);
	_ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y);
     }
   else
     {
	einfo->info.rotation = rotation;
	evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	ee->rotation = rotation;
	_ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y);
	if (ee->func.fn_resize) ee->func.fn_resize(ee);	
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   else
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
}

static void
_ecore_evas_shaped_set(Ecore_Evas *ee, int shaped)
{
   Evas_Engine_Info_Software_X11 *einfo;
   
   if (((ee->shaped) && (shaped)) ||
       ((!ee->shaped) && (!shaped)))
     return;
   if (!strcmp(ee->driver, "gl_x11")) return;
   ee->shaped = shaped;
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	if (ee->shaped)
	  {
	     ee->engine.x.mask = ecore_x_pixmap_new(ee->engine.x.win, ee->w, ee->h, 1);
	     einfo->info.mask = ee->engine.x.mask;
	     evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
	  }
	else
	  {
	     if (ee->engine.x.mask) ecore_x_pixmap_del(ee->engine.x.mask);
	     ee->engine.x.mask = 0;
	     einfo->info.mask = 0;
	     evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	     ecore_x_window_shape_mask_set(ee->engine.x.win, 0);
	     ecore_x_window_shape_mask_set(ee->engine.x.win_container, 0);
	  }
     }
}

static void
_ecore_evas_show(Ecore_Evas *ee)
{
   if (!ee->prop.fullscreen)
     ecore_x_window_show(ee->engine.x.win_container);
   ecore_x_window_show(ee->engine.x.win);
   if (ee->prop.fullscreen)
     ecore_x_window_focus(ee->engine.x.win);
   ee->should_be_visible = 1;
}

static void
_ecore_evas_hide(Ecore_Evas *ee)
{
   if (!ee->prop.fullscreen)
     ecore_x_window_hide(ee->engine.x.win_container);
   ecore_x_window_hide(ee->engine.x.win);
   ee->should_be_visible = 0;
}

static void
_ecore_evas_raise(Ecore_Evas *ee)
{
   if (!ee->prop.fullscreen)
     ecore_x_window_raise(ee->engine.x.win_container);
   else
     ecore_x_window_raise(ee->engine.x.win);
}

static void
_ecore_evas_lower(Ecore_Evas *ee)
{
   if (!ee->prop.fullscreen)
     ecore_x_window_lower(ee->engine.x.win_container);
   else
     ecore_x_window_lower(ee->engine.x.win);
}

static void
_ecore_evas_title_set(Ecore_Evas *ee, const char *t)
{
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (t) ee->prop.title = strdup(t);
   ecore_x_window_prop_title_set(ee->engine.x.win_container, ee->prop.title);
}

static void
_ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   ee->prop.name = strdup(n);
   ee->prop.clas = strdup(c);
   ecore_x_window_prop_name_class_set(ee->engine.x.win_container, ee->prop.name, ee->prop.clas);
}

static void
_ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
   ecore_x_window_prop_min_size_set(ee->engine.x.win_container, ee->prop.min.w, ee->prop.min.h);
}

static void
_ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
   ecore_x_window_prop_max_size_set(ee->engine.x.win_container, ee->prop.max.w, ee->prop.max.h);
}

static void
_ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
   ecore_x_window_prop_base_size_set(ee->engine.x.win_container, ee->prop.base.w, ee->prop.base.h);
}

static void
_ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
   ecore_x_window_prop_step_size_set(ee->engine.x.win_container, ee->prop.step.w, ee->prop.step.h);
}

static void
_ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y)
{
   int x, y;
   
   if (!file)
     {
	if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);
	if (ee->prop.cursor.file) free(ee->prop.cursor.file);
	ee->prop.cursor.object = NULL;
	ee->prop.cursor.file = NULL;
	ee->prop.cursor.layer = 0;
	ee->prop.cursor.hot.x = 0;
	ee->prop.cursor.hot.y = 0;
	ecore_x_window_cursor_show(ee->engine.x.win, 1);
	return;
     }
   ecore_x_window_cursor_show(ee->engine.x.win, 0);
   if (!ee->prop.cursor.object) ee->prop.cursor.object = evas_object_image_add(ee->evas);
   if (ee->prop.cursor.file) free(ee->prop.cursor.file);
   ee->prop.cursor.file = strdup(file);
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;   
   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_color_set(ee->prop.cursor.object, 255, 255, 255, 255);
   evas_object_move(ee->prop.cursor.object, 
		    x - ee->prop.cursor.hot.x,
		    y - ee->prop.cursor.hot.y);
   evas_object_image_file_set(ee->prop.cursor.object, ee->prop.cursor.file, NULL);
   evas_object_image_size_get(ee->prop.cursor.object, &x, &y);
   evas_object_resize(ee->prop.cursor.object, x, y);
   evas_object_image_fill_set(ee->prop.cursor.object, 0, 0, x, y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);
}

static void
_ecore_evas_layer_set(Ecore_Evas *ee, int layer)
{
   if (ee->prop.layer == layer) return;
   ee->prop.layer = layer;
   ecore_x_window_prop_layer_set(ee->engine.x.win_container, ee->prop.layer);
}

static void
_ecore_evas_focus_set(Ecore_Evas *ee, int on)
{
   ecore_x_window_focus(ee->engine.x.win_container);
}

static void
_ecore_evas_borderless_set(Ecore_Evas *ee, int on)
{
   if (((ee->prop.borderless) && (on)) ||
       ((!ee->prop.borderless) && (!on))) return;
   ee->prop.borderless = on;
   ecore_x_window_prop_borderless_set(ee->engine.x.win_container, ee->prop.borderless);
}

static void
_ecore_evas_withdrawn_set(Ecore_Evas *ee, int withdrawn)
{
   if ((ee->prop.withdrawn && withdrawn) ||
      (!ee->prop.withdrawn && !withdrawn)) return;

   ee->prop.withdrawn = withdrawn;
   ecore_x_window_prop_withdrawn_set(ee->engine.x.win_container, ee->prop.withdrawn);
}

static void
_ecore_evas_override_set(Ecore_Evas *ee, int on)
{
   if (((ee->prop.override) && (on)) ||
       ((!ee->prop.override) && (!on))) return;
   ecore_x_window_hide(ee->engine.x.win);
   ecore_x_window_reparent(ee->engine.x.win, 0, 0, 0);	
   ecore_x_window_del(ee->engine.x.win_container);
   if (on)
     ee->engine.x.win_container = ecore_x_window_override_new(0, ee->x, ee->y, ee->w, ee->h);
   else
     {
	ee->engine.x.win_container = ecore_x_window_new(0, ee->x, ee->y, ee->w, ee->h);
	ecore_x_window_prop_title_set(ee->engine.x.win_container, ee->prop.title);
	ecore_x_window_prop_name_class_set(ee->engine.x.win_container, ee->prop.name, ee->prop.clas);
	if (ee->func.fn_delete_request)
	  ecore_x_window_prop_protocol_set(ee->engine.x.win_container, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, 1);
	ecore_x_window_prop_min_size_set(ee->engine.x.win_container, ee->prop.min.w, ee->prop.min.h);
	ecore_x_window_prop_max_size_set(ee->engine.x.win_container, ee->prop.max.w, ee->prop.max.h);
	ecore_x_window_prop_base_size_set(ee->engine.x.win_container, ee->prop.base.w, ee->prop.base.h);
	ecore_x_window_prop_step_size_set(ee->engine.x.win_container, ee->prop.step.w, ee->prop.step.h);
	ecore_x_window_prop_borderless_set(ee->engine.x.win_container, ee->prop.borderless);
	ecore_x_window_prop_layer_set(ee->engine.x.win_container, ee->prop.layer);
	ecore_x_window_prop_withdrawn_set(ee->engine.x.win_container, ee->prop.withdrawn);
     }
   ecore_x_window_reparent(ee->engine.x.win, ee->engine.x.win_container, 0, 0);	
   ecore_x_window_show(ee->engine.x.win);
   if (ee->visible) ecore_x_window_show(ee->engine.x.win_container);
   if (ee->prop.focused) ecore_x_window_focus(ee->engine.x.win_container);
   ee->prop.override = on;
}

static void
_ecore_evas_fullscreen_set(Ecore_Evas *ee, int on)
{
   if (((ee->prop.fullscreen) && (on)) ||
       ((!ee->prop.fullscreen) && (!on))) return;
   if (on)
     {
	int rw, rh;
	
	ecore_x_window_size_get(0, &rw, &rh);
	ecore_x_window_resize(ee->engine.x.win, rw, rh);
	ecore_x_window_reparent(ee->engine.x.win, 0, 0, 0);
	ecore_x_window_raise(ee->engine.x.win);
	ecore_x_window_show(ee->engine.x.win);
	ecore_x_window_focus(ee->engine.x.win);
	ecore_x_window_hide(ee->engine.x.win_container);
	ecore_x_window_shape_mask_set(ee->engine.x.win_container, 0);
	if (ee->should_be_visible)
	  {
	     ecore_x_window_show(ee->engine.x.win);
	     ecore_x_window_focus(ee->engine.x.win);
	  }
	ee->x = 0;
	ee->y = 0;
     }
   else
     {
	int pw, ph;
	
	ecore_x_window_size_get(ee->engine.x.win_container, &pw, &ph);
	ecore_x_window_reparent(ee->engine.x.win, ee->engine.x.win_container, 0, 0);
	ecore_x_window_resize(ee->engine.x.win, pw, ph);
	ecore_x_window_shape_mask_set(ee->engine.x.win, 0);
	if (ee->should_be_visible) ecore_x_window_show(ee->engine.x.win_container);
     }
   ee->prop.fullscreen = on;
}

static void
_ecore_evas_avoid_damage_set(Ecore_Evas *ee, int on)
{
   Evas_Engine_Info_Software_X11 *einfo;

   if (((ee->prop.avoid_damage) && (on)) ||
       ((!ee->prop.avoid_damage) && (!on)))
     return;
   if (!strcmp(ee->driver, "gl_x11")) return;
   ee->prop.avoid_damage = on;   
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	if (ee->prop.avoid_damage)
	  {
	     ee->engine.x.pmap = ecore_x_pixmap_new(ee->engine.x.win, ee->w, ee->h, 0);
	     ee->engine.x.gc = ecore_x_gc_new(ee->engine.x.pmap);
	     einfo->info.drawable = ee->engine.x.pmap;
	     evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	     if ((ee->rotation == 90) || (ee->rotation == 270))
	       evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
	     else
	       evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
	  }
	else
	  {
	     if (ee->engine.x.pmap) ecore_x_pixmap_del(ee->engine.x.pmap);
	     if (ee->engine.x.gc) ecore_x_gc_del(ee->engine.x.gc);
	     ee->engine.x.pmap = 0;
	     ee->engine.x.gc = 0;
	     einfo->info.drawable = ee->engine.x.win;
	     evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
	  }
     }
}

int
_ecore_evas_x_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
	int i;
   
	while (ecore_evases) ecore_evas_free(ecore_evases);
	for (i = 0; i < 15; i++)
	  ecore_event_handler_del(ecore_evas_event_handlers[i]);
	ecore_idle_enterer_del(ecore_evas_idle_enterer);
	ecore_evas_idle_enterer = NULL;
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static const Ecore_Evas_Engine_Func _ecore_x_engine_func =
{
   _ecore_evas_x_free,
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
     _ecore_evas_move,
     _ecore_evas_resize,
     _ecore_evas_move_resize,
     _ecore_evas_rotation_set,
     _ecore_evas_shaped_set,
     _ecore_evas_show,
     _ecore_evas_hide,
     _ecore_evas_raise,
     _ecore_evas_lower,
     _ecore_evas_title_set,
     _ecore_evas_name_class_set,
     _ecore_evas_size_min_set,
     _ecore_evas_size_max_set,
     _ecore_evas_size_base_set,
     _ecore_evas_size_step_set,
     _ecore_evas_cursor_set,
     _ecore_evas_layer_set,
     _ecore_evas_focus_set,
     NULL,
     _ecore_evas_borderless_set,
     _ecore_evas_override_set,
     NULL,
     _ecore_evas_fullscreen_set,
     _ecore_evas_avoid_damage_set,
     _ecore_evas_withdrawn_set
};
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Evas *
ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, 
			    int x, int y, int w, int h)
{
#ifdef BUILD_ECORE_X   
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas *ee;
   int rmethod;

   rmethod = evas_render_method_lookup("software_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   
   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);
   
   _ecore_evas_x_init();
   
   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;
   
   ee->driver = strdup("software_x11");
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   
   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   
   /* init evas here */
   ee->evas = evas_new();
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);
   
   ee->engine.x.win_container = ecore_x_window_new(parent, x, y, w, h);
   ee->engine.x.win = ecore_x_window_override_new(ee->engine.x.win_container, 0, 0, w, h);
   
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	einfo->info.display  = ecore_x_display_get();
	einfo->info.visual   = DefaultVisual(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
	einfo->info.colormap = DefaultColormap(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
	einfo->info.drawable = ee->engine.x.win;
	einfo->info.depth    = DefaultDepth(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
	einfo->info.rotation = 0;
	einfo->info.debug    = 0;
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

   ecore_evases = _ecore_list_prepend(ecore_evases, ee);
   return ee;
#else
   return NULL;
#endif   
}

Ecore_Evas *
ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, 
		      int x, int y, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_GL 
   Evas_Engine_Info_GL_X11 *einfo;
   Ecore_Evas *ee;
   int rmethod;

   rmethod = evas_render_method_lookup("gl_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   
   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);
   
   _ecore_evas_x_init();
   
   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;
   
   ee->driver = strdup("gl_x11");
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   
   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   
   /* init evas here */
   ee->evas = evas_new();
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);
   
   ee->engine.x.win_container = ecore_x_window_new(parent, x, y, w, h);
   einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	XSetWindowAttributes attr;
	
	attr.backing_store = NotUseful;
	attr.colormap = einfo->func.best_colormap_get(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
	attr.border_pixel = 0;
	attr.background_pixmap = None;
	attr.event_mask =
	  ExposureMask | ButtonPressMask | ButtonReleaseMask | 
	  PointerMotionMask | StructureNotifyMask;
	attr.bit_gravity = ForgetGravity;
	
	ee->engine.x.win = 
	  XCreateWindow(ecore_x_display_get(), 
			ee->engine.x.win_container,
			0, 0, 
			w, h, 0,
			einfo->func.best_depth_get(ecore_x_display_get(), DefaultScreen(ecore_x_display_get())),
			InputOutput,
			einfo->func.best_visual_get(ecore_x_display_get(), DefaultScreen(ecore_x_display_get())),
			CWBackingStore | CWColormap |
			CWBackPixmap | CWBorderPixel |
			CWBitGravity | CWEventMask,
			&attr);
	einfo->info.display  = ecore_x_display_get();
	einfo->info.visual   = einfo->func.best_visual_get(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
	einfo->info.colormap = einfo->func.best_colormap_get(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
	einfo->info.drawable = ee->engine.x.win;
	einfo->info.depth    = einfo->func.best_depth_get(ecore_x_display_get(), DefaultScreen(ecore_x_display_get()));
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

   ecore_evases = _ecore_list_prepend(ecore_evases, ee);
   return ee;
#else
   return NULL;
#endif   
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_X_Window
ecore_evas_software_x11_window_get(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_X
   return ee->engine.x.win;
#else   
   return 0;
#endif
}
