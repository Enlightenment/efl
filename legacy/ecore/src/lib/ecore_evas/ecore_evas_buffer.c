#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Evas.h"
#include "Ecore_Input.h"

#include "ecore_private.h"
#include "ecore_evas_private.h"

#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
static int _ecore_evas_init_count = 0;

static int _ecore_evas_fps_debug = 0;

static Ecore_Evas *ecore_evases = NULL;

static int
_ecore_evas_buffer_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;
   if (getenv("ECORE_EVAS_FPS_DEBUG")) _ecore_evas_fps_debug = 1;
   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_init();
   return _ecore_evas_init_count;
}

static void
_ecore_evas_buffer_free(Ecore_Evas *ee)
{
   ecore_evases = _ecore_list2_remove(ecore_evases, ee);
   _ecore_evas_buffer_shutdown();
   if (ee->engine.buffer.image)
     {
	Ecore_Evas *ee2;

	ee2 = evas_object_data_get(ee->engine.buffer.image, "Ecore_Evas_Parent");
	evas_object_del(ee->engine.buffer.image);
	ee2->sub_ecore_evas = eina_list_remove(ee2->sub_ecore_evas, ee);
     }
   else
     free(ee->engine.buffer.pixels);
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->engine.buffer.image)
     {
	ee->engine.buffer.pixels = evas_object_image_data_get(ee->engine.buffer.image, 1);
     }
   else
     {
	if (ee->engine.buffer.pixels) free(ee->engine.buffer.pixels);
	ee->engine.buffer.pixels = malloc(ee->w * ee->h * sizeof(int));
     }

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
	einfo->info.dest_buffer = ee->engine.buffer.pixels;
	einfo->info.dest_buffer_row_bytes = ee->w * sizeof(int);
	einfo->info.use_color_key = 0;
	einfo->info.alpha_threshold = 0;
	einfo->info.func.new_update_region = NULL;
	einfo->info.func.free_update_region = NULL;
	evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

int
_ecore_evas_buffer_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
	while (ecore_evases)
	  {
	     _ecore_evas_free((Ecore_Evas *)ecore_evases);
	  }
	if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

void
_ecore_evas_buffer_render(Ecore_Evas *ee)
{
   Eina_List *updates, *l, *ll;
   Ecore_Evas *ee2;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
	if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
	_ecore_evas_buffer_render(ee2);
	if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
   if (ee->engine.buffer.image)
     {
	int w, h;

	evas_object_image_size_get(ee->engine.buffer.image, &w, &h);
	if ((w != ee->w) || (h != ee->h))
	  _ecore_evas_resize(ee, w, h);
     }
   updates = evas_render_updates(ee->evas);
   if (ee->engine.buffer.image)
     {
        Evas_Rectangle *r;

	EINA_LIST_FOREACH(updates, l, r)
	  if (ee->engine.buffer.image)
	    evas_object_image_data_update_add(ee->engine.buffer.image,
					      r->x, r->y, r->w, r->h);
     }
   if (updates)
     {
	evas_render_updates_free(updates);
	_ecore_evas_idle_timeout_update(ee);
     }
}

static void
_ecore_evas_buffer_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord xx, yy, fx, fy, fw, fh;

   evas_object_geometry_get(ee->engine.buffer.image, &xx, &yy, NULL, NULL);
   evas_object_image_fill_get(ee->engine.buffer.image, &fx, &fy, &fw, &fh);

   if (fw < 1) fw = 1;
   xx = (*x - xx) - fx;
   while (xx < 0) xx += fw;
   while (xx > fw) xx -= fw;
   *x = (ee->w * xx) / fw;

   if (fh < 1) fh = 1;
   yy = (*y - yy) - fy;
   while (yy < 0) yy += fh;
   while (yy > fh) yy -= fh;
   *y = (ee->h * yy) / fh;
}

static void
_ecore_evas_buffer_cb_mouse_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_In *ev;

   ee = data;
   ev = event_info;
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Out *ev;

   ee = data;
   ev = event_info;
   evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Down *ev;

   ee = data;
   ev = event_info;
   evas_event_feed_mouse_down(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Up *ev;

   ee = data;
   ev = event_info;
   evas_event_feed_mouse_up(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Move *ev;
   Evas_Coord x, y;

   ee = data;
   ev = event_info;
   x = ev->cur.canvas.x;
   y = ev->cur.canvas.y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   _ecore_evas_mouse_move_process(ee, x, y, ev->timestamp);
}

static void
_ecore_evas_buffer_cb_mouse_wheel(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Wheel *ev;

   ee = data;
   ev = event_info;
   evas_event_feed_mouse_wheel(ee->evas, ev->direction, ev->z, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_free(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee->driver) _ecore_evas_free(ee);
}

static void
_ecore_evas_buffer_cb_key_down(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Key_Down *ev;

   ee = data;
   ev = event_info;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Shift"))
     evas_key_modifier_on(e, "Shift");
   else
     evas_key_modifier_off(e, "Shift");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Control"))
     evas_key_modifier_on(e, "Control");
   else
     evas_key_modifier_off(e, "Control");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Alt"))
     evas_key_modifier_on(e, "Alt");
   else
     evas_key_modifier_off(e, "Alt");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Meta"))
     evas_key_modifier_on(e, "Meta");
   else
     evas_key_modifier_off(e, "Meta");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Hyper"))
     evas_key_modifier_on(e, "Hyper");
   else
     evas_key_modifier_off(e, "Hyper");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Super"))
     evas_key_modifier_on(e, "Super");
   else
     evas_key_modifier_off(e, "Super");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Scroll_Lock"))
     evas_key_lock_on(e, "Scroll_Lock");
   else
     evas_key_lock_off(e, "Scroll_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Num_Lock"))
     evas_key_lock_on(e, "Num_Lock");
   else
     evas_key_lock_off(e, "Num_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Caps_Lock"))
     evas_key_lock_on(e, "Caps_Lock");
   else
     evas_key_lock_off(e, "Caps_Lock");
   evas_event_feed_key_down(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_key_up(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Key_Up *ev;

   ee = data;
   ev = event_info;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Shift"))
     evas_key_modifier_on(e, "Shift");
   else
     evas_key_modifier_off(e, "Shift");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Control"))
     evas_key_modifier_on(e, "Control");
   else
     evas_key_modifier_off(e, "Control");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Alt"))
     evas_key_modifier_on(e, "Alt");
   else
     evas_key_modifier_off(e, "Alt");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Meta"))
     evas_key_modifier_on(e, "Meta");
   else
     evas_key_modifier_off(e, "Meta");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Hyper"))
     evas_key_modifier_on(e, "Hyper");
   else
     evas_key_modifier_off(e, "Hyper");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Super"))
     evas_key_modifier_on(e, "Super");
   else
     evas_key_modifier_off(e, "Super");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Scroll_Lock"))
     evas_key_lock_on(e, "Scroll_Lock");
   else
     evas_key_lock_off(e, "Scroll_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Num_Lock"))
     evas_key_lock_on(e, "Num_Lock");
   else
     evas_key_lock_off(e, "Num_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Caps_Lock"))
     evas_key_lock_on(e, "Caps_Lock");
   else
     evas_key_lock_off(e, "Caps_Lock");
   evas_event_feed_key_up(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_focus_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.focused = 1;
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
}

static void
_ecore_evas_buffer_cb_focus_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.focused = 0;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
}

static void
_ecore_evas_buffer_cb_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void
_ecore_evas_buffer_cb_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static const Ecore_Evas_Engine_Func _ecore_buffer_engine_func =
{
   _ecore_evas_buffer_free,
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
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_Evas *
ecore_evas_buffer_new(int w, int h)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas *ee;
   int rmethod;

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_buffer_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;

   ee->driver = "buffer";

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->rotation = 0;
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
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   ee->engine.buffer.pixels = malloc(w * h * sizeof(int));

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
	einfo->info.dest_buffer = ee->engine.buffer.pixels;
	einfo->info.dest_buffer_row_bytes = ee->w * sizeof(int);
	einfo->info.use_color_key = 0;
	einfo->info.alpha_threshold = 0;
	einfo->info.func.new_update_region = NULL;
	einfo->info.func.free_update_region = NULL;
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

   evas_event_feed_mouse_in(ee->evas, 0, NULL);

   ecore_evases = _ecore_list2_prepend(ecore_evases, ee);
   return ee;
#else
   return NULL;
#endif
}

EAPI const void *
ecore_evas_buffer_pixels_get(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   _ecore_evas_buffer_render(ee);
   return ee->engine.buffer.pixels;
#else
   return NULL;
#endif
}

EAPI Evas_Object *
ecore_evas_object_image_new(Ecore_Evas *ee_target)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Evas_Object *o;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas *ee;
   int rmethod;
   int w, h;

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   o = evas_object_image_add(ee_target->evas);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_buffer_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;

   ee->driver = "buffer";

   w = 1;
   h = 1;
   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 0;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 0;
   ee->prop.fullscreen = 0;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   ee->engine.buffer.image = o;
   evas_object_data_set(ee->engine.buffer.image, "Ecore_Evas", ee);
   evas_object_data_set(ee->engine.buffer.image, "Ecore_Evas_Parent", ee_target);
   evas_object_image_size_set(o, ee->w, ee->h);
   evas_object_image_alpha_set(o, 1);
   ee->engine.buffer.pixels = evas_object_image_data_get(o, 1);
   evas_object_image_data_set(o, ee->engine.buffer.pixels);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_MOUSE_IN,
				  _ecore_evas_buffer_cb_mouse_in, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_MOUSE_OUT,
				  _ecore_evas_buffer_cb_mouse_out, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_MOUSE_DOWN,
				  _ecore_evas_buffer_cb_mouse_down, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_MOUSE_UP,
				  _ecore_evas_buffer_cb_mouse_up, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_MOUSE_MOVE,
				  _ecore_evas_buffer_cb_mouse_move, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_MOUSE_WHEEL,
				  _ecore_evas_buffer_cb_mouse_wheel, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_FREE,
				  _ecore_evas_buffer_cb_free, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_KEY_DOWN,
				  _ecore_evas_buffer_cb_key_down, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_KEY_UP,
				  _ecore_evas_buffer_cb_key_up, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_FOCUS_IN,
				  _ecore_evas_buffer_cb_focus_in, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_FOCUS_OUT,
				  _ecore_evas_buffer_cb_focus_out, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_SHOW,
				  _ecore_evas_buffer_cb_show, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
				  EVAS_CALLBACK_HIDE,
				  _ecore_evas_buffer_cb_hide, ee);
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
	einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
	einfo->info.dest_buffer = ee->engine.buffer.pixels;
	einfo->info.dest_buffer_row_bytes = ee->w * sizeof(int);
	einfo->info.use_color_key = 0;
	einfo->info.alpha_threshold = 0;
	einfo->info.func.new_update_region = NULL;
	einfo->info.func.free_update_region = NULL;
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

   ee_target->sub_ecore_evas = eina_list_append(ee_target->sub_ecore_evas, ee);
   return o;
#else
   return NULL;
#endif
}
