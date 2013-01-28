#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_evas_wayland_private.h"


static const char *interface_wl_name = "wayland";
static const int   interface_wl_version = 1;

/* local structures */
typedef struct _EE_Wl_Smart_Data EE_Wl_Smart_Data;
struct _EE_Wl_Smart_Data
{
   Evas_Object *frame;
   Evas_Object *text;
   Evas_Coord x, y, w, h;
};

static Evas_Smart *_ecore_evas_wl_common_smart = NULL;

/* local variables */

static int _ecore_evas_wl_init_count = 0;
static Ecore_Event_Handler *_ecore_evas_wl_event_hdls[5];


static Eina_Bool
_ecore_evas_wl_common_cb_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
   _ecore_evas_mouse_move_process(ee, ev->x, ev->y, ev->timestamp);
   ee->in = EINA_TRUE;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Mouse_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->in)
     {
        ecore_event_evas_modifier_lock_update(ee->evas, ev->modifiers);
        evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
        if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
        if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
        ee->in = EINA_FALSE;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   evas_focus_out(ee->evas);
   ee->prop.focused = 0;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_wl_common_cb_window_configure(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Wl_Event_Window_Configure *ev;
   int nw = 0, nh = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = event;
   ee = ecore_event_window_match(ev->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if (ev->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   if (ee->prop.fullscreen)
     {
        _ecore_evas_wl_common_move(ee, ev->x, ev->y);
        ee->engine.func->fn_resize(ee, ev->w, ev->h);

        return ECORE_CALLBACK_PASS_ON;
     }

   if ((ee->x != ev->x) || (ee->y != ev->y))
     {
        ee->req.x = ee->x;
        ee->req.y = ee->y;
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }

   nw = ev->w;
   nh = ev->h;

   if ((ee->prop.maximized) || (!ee->prop.fullscreen))
     {
        int fw = 0, fh = 0;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
        nw = ev->w - fw;
        nh = ev->h - fh;
     }

   if (ee->prop.min.w > nw) nw = ee->prop.min.w;
   else if (nw > ee->prop.max.w) nw = ee->prop.max.w;
   if (ee->prop.min.h > nh) nh = ee->prop.min.h;
   else if (nh > ee->prop.max.h) nh = ee->prop.max.h;

   if ((ee->w != nw) || (ee->h != nh))
     {
        ee->req.w = nw;
        ee->req.h = nh;
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

int
_ecore_evas_wl_common_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_ecore_evas_wl_init_count != 1)
     return _ecore_evas_wl_init_count;

   _ecore_evas_wl_event_hdls[0] =
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_IN,
                             _ecore_evas_wl_common_cb_mouse_in, NULL);
   _ecore_evas_wl_event_hdls[1] =
     ecore_event_handler_add(ECORE_WL_EVENT_MOUSE_OUT,
                             _ecore_evas_wl_common_cb_mouse_out, NULL);
   _ecore_evas_wl_event_hdls[2] =
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_IN,
                             _ecore_evas_wl_common_cb_focus_in, NULL);
   _ecore_evas_wl_event_hdls[3] =
     ecore_event_handler_add(ECORE_WL_EVENT_FOCUS_OUT,
                             _ecore_evas_wl_common_cb_focus_out, NULL);
   _ecore_evas_wl_event_hdls[4] =
     ecore_event_handler_add(ECORE_WL_EVENT_WINDOW_CONFIGURE,
                             _ecore_evas_wl_common_cb_window_configure, NULL);

   ecore_event_evas_init();

   return _ecore_evas_wl_init_count;
}

int
_ecore_evas_wl_common_shutdown(void)
{
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (--_ecore_evas_wl_init_count != 0)
     return _ecore_evas_wl_init_count;

   for (i = 0; i < sizeof(_ecore_evas_wl_event_hdls) / sizeof(Ecore_Event_Handler *); i++)
     {
        if (_ecore_evas_wl_event_hdls[i])
          ecore_event_handler_del(_ecore_evas_wl_event_hdls[i]);
     }

   ecore_event_evas_shutdown();

   return _ecore_evas_wl_init_count;
}

void
_ecore_evas_wl_common_pre_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (wdata->frame) evas_object_del(wdata->frame);
}

void
_ecore_evas_wl_common_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wdata = ee->engine.data;
   if (wdata->win) ecore_wl_window_free(wdata->win);
   wdata->win = NULL;
   free(wdata);

   ecore_event_window_unregister(ee->prop.window);
   ecore_evas_input_event_unregister(ee);

   _ecore_evas_wl_common_shutdown();
   ecore_wl_shutdown();
}

void
_ecore_evas_wl_common_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->req.w = w;
   ee->req.h = h;

   if (!ee->prop.fullscreen)
     {
        int fw = 0, fh = 0;

        if (ee->prop.min.w > w) w = ee->prop.min.w;
        else if (w > ee->prop.max.w) w = ee->prop.max.w;
        if (ee->prop.min.h > h) h = ee->prop.min.h;
        else if (h > ee->prop.max.h) h = ee->prop.max.h;

        evas_output_framespace_get(ee->evas, NULL, NULL, &fw, &fh);
        w += fw;
        h += fh;
     }

   if ((ee->w != w) || (ee->h != h))
     {
        ee->w = w;
        ee->h = h;

        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, h, w);
             evas_output_viewport_set(ee->evas, 0, 0, h, w);
          }
        else
          {
             evas_output_size_set(ee->evas, w, h);
             evas_output_viewport_set(ee->evas, 0, 0, w, h);
          }

        if (ee->prop.avoid_damage)
          {
             int pdam = 0;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }

        if (wdata->frame)
          evas_object_resize(wdata->frame, w, h);
     }
}

void
_ecore_evas_wl_common_callback_resize_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_resize = func;
}

void
_ecore_evas_wl_common_callback_move_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_move = func;
}

void
_ecore_evas_wl_common_callback_delete_request_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_delete_request = func;
}

void
_ecore_evas_wl_common_callback_focus_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_focus_in = func;
}

void
_ecore_evas_wl_common_callback_focus_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_focus_out = func;
}

void
_ecore_evas_wl_common_callback_mouse_in_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_mouse_in = func;
}

void
_ecore_evas_wl_common_callback_mouse_out_set(Ecore_Evas *ee, void (*func)(Ecore_Evas *ee))
{
   if (!ee) return;
   ee->func.fn_mouse_out = func;
}

void
_ecore_evas_wl_common_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;

   wdata = ee->engine.data;
   ee->req.x = x;
   ee->req.y = y;

   if ((ee->x != x) || (ee->y != y))
     {
        ee->x = x;
        ee->y = y;
        if (wdata->win)
          ecore_wl_window_update_location(wdata->win, x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

static void
_ecore_evas_wl_common_smart_add(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;
   Evas *evas;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = calloc(1, sizeof(EE_Wl_Smart_Data)))) return;

   evas = evas_object_evas_get(obj);

   sd->x = 0;
   sd->y = 0;
   sd->w = 1;
   sd->h = 1;

   sd->frame = evas_object_rectangle_add(evas);
   evas_object_color_set(sd->frame, 249, 249, 249, 255);
   evas_object_smart_member_add(sd->frame, obj);

   sd->text = evas_object_text_add(evas);
   evas_object_color_set(sd->text, 0, 0, 0, 255);
   evas_object_text_style_set(sd->text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(sd->text, "Sans", 10);
   evas_object_text_text_set(sd->text, "Smart Test");
   evas_object_smart_member_add(sd->text, obj);

   evas_object_smart_data_set(obj, sd);
}

static void
_ecore_evas_wl_common_smart_del(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_del(sd->text);
   evas_object_del(sd->frame);
   free(sd);
}

static void
_ecore_evas_wl_common_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   evas_object_resize(sd->frame, w, h);
}

static void
_ecore_evas_wl_common_smart_show(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_show(sd->frame);
   evas_object_show(sd->text);
}

static void
_ecore_evas_wl_common_smart_hide(Evas_Object *obj)
{
   EE_Wl_Smart_Data *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_hide(sd->text);
   evas_object_hide(sd->frame);
}

static void
_ecore_evas_wl_common_smart_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_ecore_evas_wl_common_smart) return;
     {
        static const Evas_Smart_Class sc =
          {
             "ecore_evas_wl_frame", EVAS_SMART_CLASS_VERSION,
             _ecore_evas_wl_common_smart_add,
             _ecore_evas_wl_common_smart_del,
             NULL,
             _ecore_evas_wl_common_smart_resize,
             _ecore_evas_wl_common_smart_show,
             _ecore_evas_wl_common_smart_hide,
             NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
          };
        _ecore_evas_wl_common_smart = evas_smart_class_new(&sc);
     }
}

Evas_Object *
_ecore_evas_wl_common_frame_add(Evas *evas)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_evas_wl_common_smart_init();
   return evas_object_smart_add(evas, _ecore_evas_wl_common_smart);
}

void
_ecore_evas_wl_common_raise(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!ee) || (!ee->visible)) return;
   wdata = ee->engine.data;
   ecore_wl_window_raise(wdata->win);
}

void
_ecore_evas_wl_common_title_set(Ecore_Evas *ee, const char *title)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   wdata = ee->engine.data;
   if ((ee->prop.draw_frame) && (wdata->frame))
     {
        EE_Wl_Smart_Data *sd;

        if ((sd = evas_object_smart_data_get(wdata->frame)))
          evas_object_text_text_set(sd->text, ee->prop.title);
     }

   if ((ee->prop.title) && (wdata->win->shell_surface))
     wl_shell_surface_set_title(wdata->win->shell_surface,
                                ee->prop.title);
}

void
_ecore_evas_wl_common_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   if (n) ee->prop.name = strdup(n);
   if (c) ee->prop.clas = strdup(c);

   if ((ee->prop.clas) && (wdata->win->shell_surface))
     wl_shell_surface_set_class(wdata->win->shell_surface,
                                ee->prop.clas);
}

void
_ecore_evas_wl_common_size_min_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
}

void
_ecore_evas_wl_common_size_max_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
}

void
_ecore_evas_wl_common_size_base_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
}

void
_ecore_evas_wl_common_size_step_set(Ecore_Evas *ee, int w, int h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee) ee->prop.cursor.object = NULL;
}

void
_ecore_evas_wl_common_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y, fx, fy;
   Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (!obj)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        ecore_wl_window_cursor_default_restore(wdata->win);
        return;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_wl_window_pointer_set(wdata->win, NULL, 0, 0);

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_output_framespace_get(ee->evas, &fx, &fy, NULL, NULL);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object,
                    x - fx - ee->prop.cursor.hot.x,
                    y - fy - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
}

void
_ecore_evas_wl_common_layer_set(Ecore_Evas *ee, int layer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.layer == layer) return;
   if (layer < 1) layer = 1;
   else if (layer > 255) layer = 255;
   ee->prop.layer = layer;
}

void
_ecore_evas_wl_common_iconified_set(Ecore_Evas *ee, int iconify)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.iconified == iconify) return;
   ee->prop.iconified = iconify;
   /* FIXME: Implement this in Wayland someshow */
}

void
_ecore_evas_wl_common_maximized_set(Ecore_Evas *ee, int max)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   wdata = ee->engine.data;
   if (ee->prop.maximized == max) return;
   ee->prop.maximized = max;
   ecore_wl_window_maximized_set(wdata->win, max);
}

void
_ecore_evas_wl_common_fullscreen_set(Ecore_Evas *ee, int full)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   if (ee->prop.fullscreen == full) return;
   wdata = ee->engine.data;
   ee->prop.fullscreen = full;
   ecore_wl_window_fullscreen_set(wdata->win, full);
}

void
_ecore_evas_wl_common_ignore_events_set(Ecore_Evas *ee, int ignore)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return;
   ee->ignore_events = ignore;
   /* NB: Hmmm, may need to pass this to ecore_wl_window in the future */
}

int
_ecore_evas_wl_common_pre_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *ll = NULL;
   Ecore_Evas *ee2 = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   return rend;
}

int
_ecore_evas_wl_common_render_updates(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *updates = NULL;
   Ecore_Evas_Engine_Wl_Data *wdata = ee->engine.data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((updates = evas_render_updates(ee->evas)))
     {
        Eina_List *l = NULL;
        Eina_Rectangle *r;

        EINA_LIST_FOREACH(updates, l, r)
          ecore_wl_window_damage(wdata->win,
                                 r->x, r->y, r->w, r->h);

        ecore_wl_flush();

        evas_render_updates_free(updates);
        rend = 1;
     }

   return rend;
}

void
_ecore_evas_wl_common_post_render(Ecore_Evas *ee)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_evas_idle_timeout_update(ee);
   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
}

int
_ecore_evas_wl_common_render(Ecore_Evas *ee)
{
   int rend = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ee) return 0;
   if (!ee->visible)
     {
        evas_norender(ee->evas);
        return 0;
     }

   rend = _ecore_evas_wl_common_pre_render(ee);
   rend |= _ecore_evas_wl_common_render_updates(ee);
   _ecore_evas_wl_common_post_render(ee);

   return rend;
}

void
_ecore_evas_wl_common_screen_geometry_get(const Ecore_Evas *ee EINA_UNUSED, int *x, int *y, int *w, int *h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   ecore_wl_screen_size_get(w, h);
}

void
_ecore_evas_wl_common_screen_dpi_get(const Ecore_Evas *ee EINA_UNUSED, int *xdpi, int *ydpi)
{
   int dpi = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (xdpi) *xdpi = 0;
   if (ydpi) *ydpi = 0;
   /* FIXME: Ideally this needs to get the DPI from a specific screen */
   dpi = ecore_wl_dpi_get();
   if (xdpi) *xdpi = dpi;
   if (ydpi) *ydpi = dpi;
}

static void
_ecore_evas_wayland_resize(Ecore_Evas *ee, int location)
{
   if (!ee) return;
   if (!strcmp(ee->driver, "wayland_shm"))
     {
#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
        _ecore_evas_wayland_shm_resize(ee, location);
#endif
     }
   else if (!strcmp(ee->driver, "wayland_egl"))
     {
#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
        _ecore_evas_wayland_egl_resize(ee, location);
#endif
     }
}

static void
_ecore_evas_wayland_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   if (!strncmp(ee->driver, "wayland", 7))
     {
	wdata = ee->engine.data;
        if (wdata->win)
          {
             wdata->win->moving = EINA_TRUE;
             ecore_wl_window_move(wdata->win, x, y);
          }
     }
}

static void
_ecore_evas_wayland_type_set(Ecore_Evas *ee, int type)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!ee) return;
   wdata = ee->engine.data;
   ecore_wl_window_type_set(wdata->win, type);
}

static Ecore_Wl_Window *
_ecore_evas_wayland_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Wl_Data *wdata;

   if (!(!strncmp(ee->driver, "wayland", 7)))
     return NULL;

   wdata = ee->engine.data;
   return wdata->win;
}

static void
_ecore_evas_wayland_pointer_set(Ecore_Evas *ee EINA_UNUSED, int hot_x EINA_UNUSED, int hot_y EINA_UNUSED)
{

}

Ecore_Evas_Interface_Wayland *
_ecore_evas_wl_interface_new(void)
{
   Ecore_Evas_Interface_Wayland *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Wayland));
   if (!iface) return NULL;

   iface->base.name = interface_wl_name;
   iface->base.version = interface_wl_version;

   iface->resize = _ecore_evas_wayland_resize;
   iface->move = _ecore_evas_wayland_move;
   iface->pointer_set = _ecore_evas_wayland_pointer_set;
   iface->type_set = _ecore_evas_wayland_type_set;
   iface->window_get = _ecore_evas_wayland_window_get;

   return iface;
}
