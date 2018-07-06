#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>

#include "Ecore_Evas.h"
#include "ecore_evas_buffer.h"
#include "ecore_evas_private.h"

static void
_ecore_evas_buffer_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image)
     {
        Ecore_Evas *ee2;

        ee2 = evas_object_data_get(bdata->image, "Ecore_Evas_Parent");
        evas_object_del(bdata->image);
        if (ee2)
          ee2->sub_ecore_evas = eina_list_remove(ee2->sub_ecore_evas, ee);
     }
   else
     {
        bdata->free_func(bdata->data,
			 bdata->pixels);
     }

   free(bdata);
}

static void
_ecore_evas_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image) return;
   ee->x = ee->req.x = x;
   ee->y = ee->req.y = y;
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   int stride = 0;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (bdata->image)
     {
        bdata->pixels = evas_object_image_data_get(bdata->image, 1);
        stride = evas_object_image_stride_get(bdata->image);
     }
   else
     {
        if (bdata->pixels)
          bdata->free_func(bdata->data,
                                      bdata->pixels);
        bdata->pixels = bdata->alloc_func(bdata->data,
					  ee->w * ee->h * sizeof(int));
        stride = ee->w * sizeof(int);
     }

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        if (ee->alpha)
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        else
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
        einfo->info.dest_buffer = bdata->pixels;
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
   if (bdata->image)
      evas_object_image_data_set(bdata->image, bdata->pixels);
   else
     bdata->resized = 1;
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   _ecore_evas_resize(ee, w, h);
}

static void
_ecore_evas_show(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image) return;
   if (ecore_evas_focus_device_get(ee, NULL)) return;
   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
}

static void
_ecore_evas_buffer_title_set(Ecore_Evas *ee, const char *t)
{
   if (eina_streq(ee->prop.title, t)) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (!t) return;
   ee->prop.title = strdup(t);
}

static void
_ecore_evas_buffer_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (!eina_streq(n, ee->prop.name))
     {
       	free(ee->prop.name);
       	ee->prop.name = NULL;
       	if (n) ee->prop.name = strdup(n);
     }
   if (!eina_streq(c, ee->prop.clas))
     {
       	free(ee->prop.clas);
       	ee->prop.clas = NULL;
       	if (c) ee->prop.clas = strdup(c);
     }
}

static Eina_Bool
_ecore_evas_buffer_prepare(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata;

   bdata = ee->engine.data;
   if (bdata->image)
     {
        int w, h;

        evas_object_image_size_get(bdata->image, &w, &h);
        if ((w != ee->w) || (h != ee->h))
           _ecore_evas_resize(ee, w, h);
        bdata->pixels = evas_object_image_data_get(bdata->image, 1);
     }
   else if (bdata->resized)
     {
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
        bdata->resized = 0;
     }

   return EINA_TRUE;
}

static void
_ecore_evas_buffer_update_image(void *data, Evas *e EINA_UNUSED, void *event_info)
{
   Evas_Event_Render_Post *post = event_info;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Eina_Rectangle *r;
   Eina_List *l;

   evas_object_image_data_set(bdata->image, bdata->pixels);
   EINA_LIST_FOREACH(post->updated_area, l, r)
     evas_object_image_data_update_add(bdata->image,
                                       r->x, r->y, r->w, r->h);
}

EAPI int
ecore_evas_buffer_render(Ecore_Evas *ee)
{
   int r;

   r = ecore_evas_render(ee);
   ecore_evas_render_wait(ee);
   return r;
}

// NOTE: if you fix this, consider fixing ecore_evas_ews.c as it is similar!
static void
_ecore_evas_buffer_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Coord xx, yy, ww, hh, fx, fy, fw, fh;

   evas_object_geometry_get(bdata->image, &xx, &yy, &ww, &hh);
   evas_object_image_fill_get(bdata->image, &fx, &fy, &fw, &fh);

   if (fw < 1) fw = 1;
   if (fh < 1) fh = 1;

   if (evas_object_map_get(bdata->image) &&
       evas_object_map_enable_get(bdata->image))
     {
        fx = 0; fy = 0;
        fw = ee->w; fh = ee->h;
        ww = ee->w; hh = ee->h;
     }
   
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
_ecore_evas_buffer_transfer_modifiers_locks(Evas *e, Evas *e2)
{
   const char *mods[] = 
     { "Shift", "Control", "Alt", "Meta", "Hyper", "Super", NULL };
   const char *locks[] = 
     { "Scroll_Lock", "Num_Lock", "Caps_Lock", NULL };
   int i;
   
   for (i = 0; mods[i]; i++)
     {
        if (evas_key_modifier_is_set(evas_key_modifier_get(e), mods[i]))
          evas_key_modifier_on(e2, mods[i]);
        else
          evas_key_modifier_off(e2, mods[i]);
     }
   for (i = 0; locks[i]; i++)
     {
        if (evas_key_lock_is_set(evas_key_lock_get(e), locks[i]))
          evas_key_lock_on(e2, locks[i]);
        else
          evas_key_lock_off(e2, locks[i]);
     }
}

static void
_ecore_evas_buffer_cb_mouse_in(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_In *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_out(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Out *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Down *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_down(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_up(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Up *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_up(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_move(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Move *ev;
   Evas_Coord x, y;

   ee = data;
   ev = event_info;
   x = ev->cur.canvas.x;
   y = ev->cur.canvas.y;
   if (!ee->evas) return;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   _ecore_evas_mouse_move_process(ee, x, y, ev->timestamp);
}

static void
_ecore_evas_buffer_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Wheel *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_wheel(ee->evas, ev->direction, ev->z, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
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
   if (!ee->evas) return;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_multi_down(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_up(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
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
   if (!ee->evas) return;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_multi_up(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_move(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
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
   if (!ee->evas) return;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->cur.canvas.xsub - (double)xx) + (double)x;
   yf = (ev->cur.canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_multi_move(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_free(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee->driver) _ecore_evas_free(ee);
}

static void
_ecore_evas_buffer_cb_key_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Key_Down *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_key_down(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_key_up(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Key_Up *ev;

   ee = data;
   ev = event_info;
   if (!ee->evas) return;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_key_up(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (!ee->evas) return;
   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
}

static void
_ecore_evas_buffer_cb_focus_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (!ee->evas) return;
   _ecore_evas_focus_device_set(ee, NULL, EINA_FALSE);
}

static void
_ecore_evas_buffer_cb_show(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void
_ecore_evas_buffer_cb_hide(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.withdrawn = EINA_TRUE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void
_ecore_evas_buffer_alpha_set(Ecore_Evas *ee, int alpha)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   if (((ee->alpha) && (alpha)) || ((!ee->alpha) && (!alpha))) return;
   ee->alpha = alpha;
   if (bdata->image)
      evas_object_image_alpha_set(bdata->image, ee->alpha);
   else
     {
        Evas_Engine_Info_Buffer *einfo;
        
        einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->alpha)
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
             else
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
}

static void
_ecore_evas_buffer_profile_set(Ecore_Evas *ee, const char *profile)
{
   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   if (profile)
     {
        ee->prop.profile.name = (char *)eina_stringshare_add(profile);

        /* just change ee's state.*/
        if (ee->func.fn_state_change)
          ee->func.fn_state_change(ee);
     }
}

static void
_ecore_evas_buffer_msg_parent_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   Ecore_Evas *parent_ee = NULL;
   parent_ee = ecore_evas_data_get(ee, "parent");

   if (parent_ee)
     {
        if (parent_ee->func.fn_msg_parent_handle)
          parent_ee ->func.fn_msg_parent_handle(parent_ee, msg_domain, msg_id, data, size);
     }
   else
     {
        if (ee->func.fn_msg_parent_handle)
          ee ->func.fn_msg_parent_handle(ee, msg_domain, msg_id, data, size);
     }
}

static void
_ecore_evas_buffer_msg_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   Ecore_Evas *child_ee = NULL;
   child_ee = ecore_evas_data_get(ee, "child");

   if (child_ee)
     {
        if (child_ee->func.fn_msg_handle)
          child_ee->func.fn_msg_handle(child_ee, msg_domain, msg_id, data, size);
     }
   else
     {
        if (ee->func.fn_msg_handle)
          ee->func.fn_msg_handle(ee, msg_domain, msg_id, data, size);
     }
}

static void
_ecore_evas_buffer_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (x) *x = ee->x;
   if (y) *y = ee->y;
   if (w) *w = ee->w;
   if (h) *h = ee->h;
}

static void
_ecore_evas_buffer_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   evas_pointer_canvas_xy_get(ee->evas, x, y);
}

static Eina_Bool
_ecore_evas_buffer_pointer_warp(const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image)
     _ecore_evas_mouse_move_process((Ecore_Evas*)ee, x, y, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff));
   else
     {
        Ecore_Event_Mouse_Move *ev;

        ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
        EINA_SAFETY_ON_NULL_RETURN_VAL(ev, EINA_FALSE);

        ev->window = ee->prop.window;
        ev->event_window = ee->prop.window;
        ev->root_window = ee->prop.window;
        ev->timestamp = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
        ev->same_screen = 1;

        ev->x = x;
        ev->y = y;
        ev->root.x = x;
        ev->root.y = y;

        {
           const char *mods[] = 
             { "Shift", "Control", "Alt", "Super", NULL };
           int modifiers[] =
             { ECORE_EVENT_MODIFIER_SHIFT, ECORE_EVENT_MODIFIER_CTRL, ECORE_EVENT_MODIFIER_ALT,
               ECORE_EVENT_MODIFIER_WIN, 0 };
           int i;
           
           for (i = 0; mods[i]; i++)
             if (evas_key_modifier_is_set(evas_key_modifier_get(ee->evas), mods[i]))
               ev->modifiers |= modifiers[i];
        }

        //FIXME ev->multi.device = ???

        ev->multi.radius = 1;
        ev->multi.radius_x = 1;
        ev->multi.radius_y = 1;
        ev->multi.pressure = 1.0;
        ev->multi.angle = 0.0;
        ev->multi.x = ev->x;
        ev->multi.y = ev->y;
        ev->multi.root.x = ev->x;
        ev->multi.root.y = ev->y;

        ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
     }
   return EINA_TRUE;
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
     _ecore_evas_move,
     NULL,
     _ecore_evas_resize,
     _ecore_evas_move_resize,
     NULL,
     NULL,
     _ecore_evas_show,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_buffer_title_set,
     _ecore_evas_buffer_name_class_set,
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
     NULL, // profiles_set
     _ecore_evas_buffer_profile_set,

     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,

     NULL,
     _ecore_evas_buffer_screen_geometry_get,
     NULL,  // screen_dpi_get
     _ecore_evas_buffer_msg_parent_send,
     _ecore_evas_buffer_msg_send,

     _ecore_evas_buffer_pointer_xy_get, // pointer_xy_get
     _ecore_evas_buffer_pointer_warp, // pointer_warp

     NULL, // wm_rot_preferred_rotation_set
     NULL, // wm_rot_available_rotations_set
     NULL, // wm_rot_manual_rotation_done_set
     NULL, // wm_rot_manual_rotation_done

     NULL, // aux_hints_set

     NULL, // fn_animator_register
     NULL, // fn_animator_unregister

     NULL, // fn_evas_changed
     NULL, //fn_focus_device_set
     NULL, //fn_callback_focus_device_in_set
     NULL, //fn_callback_focus_device_out_set
     NULL, //fn_callback_device_mouse_in_set
     NULL, //fn_callback_device_mouse_out_set
     NULL, //fn_pointer_device_xy_get
     _ecore_evas_buffer_prepare,
};

static void *
_ecore_evas_buffer_pix_alloc(void *data EINA_UNUSED, int size)
{
   return malloc(size);
}

static void
_ecore_evas_buffer_pix_free(void *data EINA_UNUSED, void *pix)
{
   free(pix);
}

EAPI Ecore_Evas *
ecore_evas_buffer_allocfunc_new(int w, int h,
                                void *(*alloc_func) (void *data, int size),
                                void (*free_func) (void *data, void *pix),
                                const void *data)
{
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Ecore_Evas *ee;
   int rmethod;

   EINA_SAFETY_ON_NULL_RETURN_VAL(alloc_func, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(free_func, NULL);

   rmethod = evas_render_method_lookup("buffer");
   EINA_SAFETY_ON_TRUE_RETURN_VAL(rmethod == 0, NULL);

   ee = calloc(1, sizeof(Ecore_Evas));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);

   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;
   ee->engine.data = bdata;
   bdata->alloc_func = alloc_func;
   bdata->free_func = free_func;
   bdata->data = (void *)data;

   ee->driver = "buffer";

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->rotation = 0;
   ee->visible = 1;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1;

   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;
   else
     ee->can_async_render = 1;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.borderless = EINA_TRUE;
   ee->prop.override = EINA_TRUE;
   ee->prop.maximized = EINA_TRUE;
   ee->prop.fullscreen = EINA_FALSE;
   ee->prop.withdrawn = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;

   /* init evas here */
   if (!ecore_evas_evas_new(ee, w, h))
     {
        ERR("Can not create a Canvas.");
        ecore_evas_free(ee);
        return NULL;
     }

   evas_output_method_set(ee->evas, rmethod);

   bdata->pixels = bdata->alloc_func(bdata->data, w * h * sizeof(int));

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
        einfo->info.dest_buffer = bdata->pixels;
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

   if (!_ecore_evas_cursors_init(ee))
     {
        ERR("Could not init the Ecore Evas cursors");
        ecore_evas_free(ee);
        return NULL;
     }
   evas_event_feed_mouse_in(ee->evas, 0, NULL);

   _ecore_evas_register(ee);

   evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);

   return ee;
}

EAPI Ecore_Evas *
ecore_evas_buffer_new(int w, int h)
{
    return ecore_evas_buffer_allocfunc_new
     (w, h, _ecore_evas_buffer_pix_alloc, _ecore_evas_buffer_pix_free, NULL);
}

EAPI const void *
ecore_evas_buffer_pixels_get(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);

   bdata = ee->engine.data;
   ecore_evas_render(ee);
   ecore_evas_render_wait(ee);
   return bdata->pixels;
}

EAPI Ecore_Evas *
ecore_evas_buffer_ecore_evas_parent_get(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);

   bdata = ee->engine.data;
   return evas_object_data_get(bdata->image, "Ecore_Evas_Parent");
}

EAPI Evas_Object *
ecore_evas_object_image_new(Ecore_Evas *ee_target)
{
   Evas_Object *o;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas *ee;
   int rmethod;
   int w = 1, h = 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ee_target, NULL);

   rmethod = evas_render_method_lookup("buffer");
   EINA_SAFETY_ON_TRUE_RETURN_VAL(rmethod == 0, NULL);

   ee = calloc(1, sizeof(Ecore_Evas));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);

   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ee->engine.data = bdata;

   o = evas_object_image_add(ee_target->evas);
   evas_object_image_content_hint_set(o, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_alpha_set(o, 0);
   evas_object_image_size_set(o, w, h);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;

   ee->driver = "buffer";

   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1;
   ee->can_async_render = 0;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.borderless = EINA_TRUE;
   ee->prop.override = EINA_TRUE;
   ee->prop.maximized = EINA_FALSE;
   ee->prop.fullscreen = EINA_FALSE;
   ee->prop.withdrawn = EINA_TRUE;
   ee->prop.sticky = EINA_FALSE;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST, _ecore_evas_buffer_update_image, ee);

   bdata->image = o;
   evas_object_data_set(bdata->image, "Ecore_Evas", ee);
   evas_object_data_set(bdata->image, "Ecore_Evas_Parent", ee_target);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_IN,
                                  _ecore_evas_buffer_cb_mouse_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_OUT,
                                  _ecore_evas_buffer_cb_mouse_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _ecore_evas_buffer_cb_mouse_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_UP,
                                  _ecore_evas_buffer_cb_mouse_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_MOVE,
                                  _ecore_evas_buffer_cb_mouse_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_WHEEL,
                                  _ecore_evas_buffer_cb_mouse_wheel, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_DOWN,
                                  _ecore_evas_buffer_cb_multi_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_UP,
                                  _ecore_evas_buffer_cb_multi_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_MOVE,
                                  _ecore_evas_buffer_cb_multi_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FREE,
                                  _ecore_evas_buffer_cb_free, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_DOWN,
                                  _ecore_evas_buffer_cb_key_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_UP,
                                  _ecore_evas_buffer_cb_key_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_IN,
                                  _ecore_evas_buffer_cb_focus_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_OUT,
                                  _ecore_evas_buffer_cb_focus_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_SHOW,
                                  _ecore_evas_buffer_cb_show, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_HIDE,
                                  _ecore_evas_buffer_cb_hide, ee);
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        bdata->pixels = evas_object_image_data_get(o, 1);
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        einfo->info.dest_buffer = bdata->pixels;
        einfo->info.dest_buffer_row_bytes = evas_object_image_stride_get(o);
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        evas_object_image_data_set(o, bdata->pixels);
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

   if (!_ecore_evas_cursors_init(ee))
     {
        ERR("Could not init the Ecore Evas cursors");
        ecore_evas_free(ee);
        return NULL;
     }

   _ecore_evas_subregister(ee_target, ee);
   return o;
}
