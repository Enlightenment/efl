#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// NOTE: if you fix this, consider fixing ecore_evas_ews.c as it is similar!

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
static int _ecore_evas_init_count = 0;

static int
_ecore_evas_buffer_init(void)
{
   _ecore_evas_init_count++;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_buffer_free(Ecore_Evas *ee)
{
   if (ee->engine.buffer.image)
     {
        Ecore_Evas *ee2;

        ee2 = evas_object_data_get(ee->engine.buffer.image, "Ecore_Evas_Parent");
        evas_object_del(ee->engine.buffer.image);
        ee2->sub_ecore_evas = eina_list_remove(ee2->sub_ecore_evas, ee);
     }
   else
     {
        ee->engine.buffer.free_func(ee->engine.buffer.data,
                                    ee->engine.buffer.pixels);
     }
   _ecore_evas_buffer_shutdown();
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;
   int stride = 0;

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
        stride = evas_object_image_stride_get(ee->engine.buffer.image);
     }
   else
     {
        if (ee->engine.buffer.pixels)
          ee->engine.buffer.free_func(ee->engine.buffer.data,
                                      ee->engine.buffer.pixels);
        ee->engine.buffer.pixels =
          ee->engine.buffer.alloc_func(ee->engine.buffer.data,
                                       ee->w * ee->h * sizeof(int));
        stride = ee->w * sizeof(int);
     }

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        einfo->info.dest_buffer = ee->engine.buffer.pixels;
        einfo->info.dest_buffer_row_bytes = stride;
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
   if (ee->engine.buffer.image)
      evas_object_image_data_set(ee->engine.buffer.image, ee->engine.buffer.pixels);
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

int
_ecore_evas_buffer_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

int
_ecore_evas_buffer_render(Ecore_Evas *ee)
{
   Eina_List *updates, *l, *ll;
   Ecore_Evas *ee2;
   int rend = 0;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
           rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
   if (ee->engine.buffer.image)
     {
        int w, h;

        evas_object_image_size_get(ee->engine.buffer.image, &w, &h);
        if ((w != ee->w) || (h != ee->h))
           _ecore_evas_resize(ee, w, h);
        ee->engine.buffer.pixels = evas_object_image_data_get(ee->engine.buffer.image, 1);
     }
   updates = evas_render_updates(ee->evas);
   if (ee->engine.buffer.image)
     {
        Eina_Rectangle *r;

        evas_object_image_data_set(ee->engine.buffer.image, ee->engine.buffer.pixels);
        EINA_LIST_FOREACH(updates, l, r)
           evas_object_image_data_update_add(ee->engine.buffer.image,
                                             r->x, r->y, r->w, r->h);
     }
   if (updates)
     {
        evas_render_updates_free(updates);
        _ecore_evas_idle_timeout_update(ee);
     }

   return updates ? 1 : rend;
}

// NOTE: if you fix this, consider fixing ecore_evas_ews.c as it is similar!
static void
_ecore_evas_buffer_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord xx, yy, ww, hh, fx, fy, fw, fh;

   evas_object_geometry_get(ee->engine.buffer.image, &xx, &yy, &ww, &hh);
   evas_object_image_fill_get(ee->engine.buffer.image, &fx, &fy, &fw, &fh);

   if (fw < 1) fw = 1;
   if (fh < 1) fh = 1;

   if ((fx == 0) && (fy == 0) && (fw == ww) && (fh == hh))
     {
        *x = (ee->w * (*x - xx)) / fw;
        *y = (ee->h * (*y - yy)) / fh;
     }
   else
     {
        xx = (*x - xx) - fx;
        while (xx < 0) xx += fw;
        while (xx > fw) xx -= fw;
        *x = (ee->w * xx) / fw;

        yy = (*y - yy) - fy;
        while (yy < 0) yy += fh;
        while (yy > fh) yy -= fh;
        *y = (ee->h * yy) / fh;
     }
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
_ecore_evas_buffer_cb_multi_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Multi_Down *ev;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   ee = data;
   ev = event_info;
   x = ev->canvas.x;
   y = ev->canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   evas_event_feed_multi_down(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Multi_Up *ev;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   ee = data;
   ev = event_info;
   x = ev->canvas.x;
   y = ev->canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   evas_event_feed_multi_up(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Multi_Move *ev;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   ee = data;
   ev = event_info;
   x = ev->cur.canvas.x;
   y = ev->cur.canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->cur.canvas.xsub - (double)xx) + (double)x;
   yf = (ev->cur.canvas.ysub - (double)yy) + (double)y;
   evas_event_feed_multi_move(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->timestamp, NULL);
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
     evas_key_modifier_on(ee->evas, "Shift");
   else
     evas_key_modifier_off(ee->evas, "Shift");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Control"))
     evas_key_modifier_on(ee->evas, "Control");
   else
     evas_key_modifier_off(ee->evas, "Control");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Alt"))
     evas_key_modifier_on(ee->evas, "Alt");
   else
     evas_key_modifier_off(ee->evas, "Alt");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Meta"))
     evas_key_modifier_on(ee->evas, "Meta");
   else
     evas_key_modifier_off(ee->evas, "Meta");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Hyper"))
     evas_key_modifier_on(ee->evas, "Hyper");
   else
     evas_key_modifier_off(ee->evas, "Hyper");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Super"))
     evas_key_modifier_on(ee->evas, "Super");
   else
     evas_key_modifier_off(ee->evas, "Super");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Scroll_Lock"))
     evas_key_lock_on(ee->evas, "Scroll_Lock");
   else
     evas_key_lock_off(ee->evas, "Scroll_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Num_Lock"))
     evas_key_lock_on(ee->evas, "Num_Lock");
   else
     evas_key_lock_off(ee->evas, "Num_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Caps_Lock"))
     evas_key_lock_on(ee->evas, "Caps_Lock");
   else
     evas_key_lock_off(ee->evas, "Caps_Lock");
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
     evas_key_modifier_on(ee->evas, "Shift");
   else
     evas_key_modifier_off(ee->evas, "Shift");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Control"))
     evas_key_modifier_on(ee->evas, "Control");
   else
     evas_key_modifier_off(ee->evas, "Control");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Alt"))
     evas_key_modifier_on(ee->evas, "Alt");
   else
     evas_key_modifier_off(ee->evas, "Alt");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Meta"))
     evas_key_modifier_on(ee->evas, "Meta");
   else
     evas_key_modifier_off(ee->evas, "Meta");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Hyper"))
     evas_key_modifier_on(ee->evas, "Hyper");
   else
     evas_key_modifier_off(ee->evas, "Hyper");
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Super"))
     evas_key_modifier_on(ee->evas, "Super");
   else
     evas_key_modifier_off(ee->evas, "Super");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Scroll_Lock"))
     evas_key_lock_on(ee->evas, "Scroll_Lock");
   else
     evas_key_lock_off(ee->evas, "Scroll_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Num_Lock"))
     evas_key_lock_on(ee->evas, "Num_Lock");
   else
     evas_key_lock_off(ee->evas, "Num_Lock");
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Caps_Lock"))
     evas_key_lock_on(ee->evas, "Caps_Lock");
   else
     evas_key_lock_off(ee->evas, "Caps_Lock");
   evas_event_feed_key_up(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_focus_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
}

static void
_ecore_evas_buffer_cb_focus_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.focused = 0;
   evas_focus_out(ee->evas);
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

static void
_ecore_evas_buffer_alpha_set(Ecore_Evas *ee, int alpha)
{
   if (((ee->alpha) && (alpha)) || ((!ee->alpha) && (!alpha))) return;
   ee->alpha = alpha;
   if (ee->engine.buffer.image)
      evas_object_image_alpha_set(ee->engine.buffer.image, ee->alpha);
}

static Ecore_Evas_Engine_Func _ecore_buffer_engine_func =
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
     _ecore_evas_buffer_alpha_set,
     NULL, //transparent

     NULL, // render
     NULL  // screen_geometry_get
};
#endif

static void *
_ecore_evas_buffer_pix_alloc(void *data __UNUSED__, int size)
{
   return malloc(size);
}

static void
_ecore_evas_buffer_pix_free(void *data __UNUSED__, void *pix)
{
   free(pix);
}

EAPI Ecore_Evas *
ecore_evas_buffer_new(int w, int h)
{
    return ecore_evas_buffer_allocfunc_new
     (w, h, _ecore_evas_buffer_pix_alloc, _ecore_evas_buffer_pix_free, NULL);
}

EAPI Ecore_Evas *
ecore_evas_buffer_allocfunc_new(int w, int h, void *(*alloc_func) (void *data, int size), void (*free_func) (void *data, void *pix), const void *data)
{
// NOTE: if you fix this, consider fixing ecore_evas_ews.c as it is similar!
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas *ee;
   int rmethod;

   if ((!alloc_func) || (!free_func)) return NULL;
   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_buffer_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;
   ee->engine.buffer.alloc_func = alloc_func;
   ee->engine.buffer.free_func = free_func;
   ee->engine.buffer.data = (void *)data;

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

   ee->engine.buffer.pixels =
     ee->engine.buffer.alloc_func
     (ee->engine.buffer.data, w * h * sizeof(int));

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

   ee->engine.func->fn_render = _ecore_evas_buffer_render;
   _ecore_evas_register(ee);

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

EAPI Evas *
ecore_evas_object_evas_get(Evas_Object *obj)
{
   Ecore_Evas *ee;

   ee = evas_object_data_get(obj, "Ecore_Evas");
   if (!ee) return NULL;

   return ecore_evas_get(ee);
}

EAPI Ecore_Evas *
ecore_evas_object_ecore_evas_get(Evas_Object *obj)
{
   return evas_object_data_get(obj, "Ecore_Evas");
}

EAPI Evas_Object *
ecore_evas_object_image_new(Ecore_Evas *ee_target)
{
// NOTE: if you fix this, consider fixing ecore_evas_ews.c as it is similar!
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Evas_Object *o;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas *ee;
   int rmethod;
   int w = 1, h = 1;

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   o = evas_object_image_add(ee_target->evas);
   evas_object_image_content_hint_set(o, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_alpha_set(o, 0);
   evas_object_image_size_set(o, w, h);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_buffer_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;

   ee->driver = "buffer";

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
                                  EVAS_CALLBACK_MULTI_DOWN,
                                  _ecore_evas_buffer_cb_multi_down, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
                                  EVAS_CALLBACK_MULTI_UP,
                                  _ecore_evas_buffer_cb_multi_up, ee);
   evas_object_event_callback_add(ee->engine.buffer.image,
                                  EVAS_CALLBACK_MULTI_MOVE,
                                  _ecore_evas_buffer_cb_multi_move, ee);
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
        ee->engine.buffer.pixels = evas_object_image_data_get(o, 1);
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        einfo->info.dest_buffer = ee->engine.buffer.pixels;
        einfo->info.dest_buffer_row_bytes = evas_object_image_stride_get(o);
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        evas_object_image_data_set(o, ee->engine.buffer.pixels);
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
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

   ee->engine.func->fn_render = _ecore_evas_buffer_render;

   return o;
#else
   return NULL;
#endif
}
