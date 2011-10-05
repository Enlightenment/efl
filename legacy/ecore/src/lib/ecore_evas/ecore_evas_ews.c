#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

EAPI int ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_ADD = 0;
EAPI int ECORE_EVAS_EWS_EVENT_DEL = 0;
EAPI int ECORE_EVAS_EWS_EVENT_RESIZE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_MOVE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_SHOW = 0;
EAPI int ECORE_EVAS_EWS_EVENT_HIDE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_FOCUS = 0;
EAPI int ECORE_EVAS_EWS_EVENT_UNFOCUS = 0;
EAPI int ECORE_EVAS_EWS_EVENT_RAISE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_LOWER = 0;
EAPI int ECORE_EVAS_EWS_EVENT_ACTIVATE = 0;

EAPI int ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_LAYER_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE = 0;

#ifdef BUILD_ECORE_EVAS_EWS
static int _ecore_evas_init_count = 0;

static Ecore_Evas *_ews_ee = NULL;
static Evas_Object *_ews_bg = NULL;
static Eina_List *_ews_children = NULL;
static const void *_ews_manager = NULL;
static char *_ews_engine = NULL;
static char *_ews_options = NULL;
static int _ews_x = 0;
static int _ews_y = 0;
static int _ews_w = 1024;
static int _ews_h = 768;
static Eina_Bool _ews_defaults_engine = EINA_TRUE;
static Eina_Bool _ews_defaults_geo = EINA_TRUE;

static const char EWS_ENGINE_NAME[] = "ews";

static void
_ecore_evas_ews_pre_free(Ecore_Evas *ee __UNUSED__)
{
   DBG("EWS backing store free'd");
   _ews_children = eina_list_free(_ews_children);
   _ews_ee = NULL;
   _ews_bg = NULL;
}

static void
_ecore_evas_ews_del_request(Ecore_Evas *ee __UNUSED__)
{
   INF("EWS backing store deletion is forbidden!");
}

static Ecore_Evas *
_ecore_evas_ews_ee_new(void)
{
   Ecore_Evas *ee = ecore_evas_new(_ews_engine, _ews_x, _ews_y, _ews_w, _ews_h,
                                   _ews_options);
   if (!ee)
     ERR("Failed: ecore_evas_new(%s, %d, %d, %d, %d, %s)",
         _ews_engine, _ews_x, _ews_y, _ews_w, _ews_h, _ews_options);
   else
     {
        ecore_evas_callback_pre_free_set(ee, _ecore_evas_ews_pre_free);
        ecore_evas_callback_delete_request_set(ee, _ecore_evas_ews_del_request);
        ecore_evas_name_class_set(ee, "ecore_evas_ews", "ews");
        ecore_evas_title_set
          (ee, "EWS: Ecore + Evas Single Process Windowing System");
        ecore_evas_show(ee);
     }

   return ee;
}

static void
_ecore_evas_ews_env_setup(void)
{
   const char *env = getenv("ECORE_EVAS_EWS");
   char *p, *n, *tmp;

   if (_ews_defaults_engine)
     {
        free(_ews_engine);
        _ews_engine = NULL;
        free(_ews_options);
        _ews_options = NULL;
     }
   if (_ews_defaults_geo)
     {
        _ews_x = 0;
        _ews_y = 0;
        _ews_w = 1024;
        _ews_h = 768;
     }

   if ((!env) || (!*env)) return;

   p = tmp = strdup(env);
   if (!tmp) return;

   n = strchr(p, ':');
   if (n) *n = '\0';
   if (_ews_defaults_engine) _ews_engine = strdup(p);
   if (!n) goto end;

   p = n + 1;
   n = strchr(p, ':');
   if (!n) goto end;
   *n = '\0';
   if (_ews_defaults_geo) _ews_x = atoi(p);

   p = n + 1;
   n = strchr(p, ':');
   if (!n) goto end;
   *n = '\0';
   if (_ews_defaults_geo) _ews_y = atoi(p);

   p = n + 1;
   n = strchr(p, ':');
   if (!n) goto end;
   *n = '\0';
   if (_ews_defaults_geo) _ews_w = atoi(p);

   p = n + 1;
   n = strchr(p, ':');
   if (n) *n = '\0';
   if (_ews_defaults_geo) _ews_h = atoi(p);
   if (!n) goto end;

   p = n + 1;
   if (_ews_defaults_engine) _ews_options = strdup(p);

 end:
   free(tmp);
}

static void
_ecore_evas_ews_event_free(void *data __UNUSED__, void *ev)
{
   Ecore_Evas *ee = ev;
   _ecore_evas_unref(ee);
}

static void
_ecore_evas_ews_event(Ecore_Evas *ee, int event)
{
   _ecore_evas_ref(ee);
   ecore_event_add(event, ee, _ecore_evas_ews_event_free, NULL);
}

static void
_ecore_evas_ews_event_free_del(void *data __UNUSED__, void *ev __UNUSED__)
{
   _ecore_evas_ews_shutdown();
}

static void
_ecore_evas_ews_free(Ecore_Evas *ee)
{
   evas_object_del(ee->engine.ews.image);
   _ews_ee->sub_ecore_evas = eina_list_remove(_ews_ee->sub_ecore_evas, ee);

   ecore_event_add(ECORE_EVAS_EWS_EVENT_DEL, ee, _ecore_evas_ews_event_free_del, NULL);
}

static void
_ecore_evas_ews_move(Ecore_Evas *ee, int x, int y)
{
   if ((x == ee->x) && (y == ee->y)) return;
   ee->x = x;
   ee->y = y;
   evas_object_move(ee->engine.ews.image, x, y);
   if (ee->func.fn_move) ee->func.fn_move(ee);

   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_MOVE);
}

static void
_ecore_evas_ews_managed_move(Ecore_Evas *ee, int x, int y)
{
   if ((x == ee->x) && (y == ee->y)) return;
   ee->x = x;
   ee->y = y;
   if (ee->func.fn_move) ee->func.fn_move(ee);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_MOVE);
}

static void
_ecore_evas_ews_resize_internal(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;
   void *pixels;
   int stride;

   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);
   evas_damage_rectangle_add(ee->evas, 0, 0, w, h);

   evas_object_image_size_set(ee->engine.ews.image, w, h);
   evas_object_image_fill_set(ee->engine.ews.image, 0, 0, w, h);
   evas_object_resize(ee->engine.ews.image, w, h);

   pixels = evas_object_image_data_get(ee->engine.ews.image, 1);
   evas_object_image_data_set(ee->engine.ews.image, pixels); // refcount
   stride = evas_object_image_stride_get(ee->engine.ews.image);

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   EINA_SAFETY_ON_NULL_RETURN(einfo);

   einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
   einfo->info.dest_buffer = pixels;
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

static void
_ecore_evas_ews_resize(Ecore_Evas *ee, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   _ecore_evas_ews_resize_internal(ee, w, h);
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_RESIZE);
}

static void
_ecore_evas_ews_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   _ecore_evas_ews_move(ee, x, y);
   _ecore_evas_ews_resize(ee, w, h);
}

static void
_ecore_evas_ews_rotation_set(Ecore_Evas *ee, int rot, int resize)
{
   if (ee->rotation == rot) return;
   ee->rotation = rot;

   ERR("TODO: rot=%d, resize=%d", rot, resize);

   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_shaped_set(Ecore_Evas *ee, int val)
{
   if (ee->shaped == val) return;
   ee->shaped = val;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_show(Ecore_Evas *ee)
{
   ee->should_be_visible = EINA_TRUE;
   evas_object_show(ee->engine.ews.image);
   if (ee->prop.fullscreen)
     evas_object_focus_set(ee->engine.ews.image, EINA_TRUE);

   if (ee->func.fn_show) ee->func.fn_show(ee);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_SHOW);
}

static void
_ecore_evas_ews_hide(Ecore_Evas *ee)
{
   ee->should_be_visible = EINA_FALSE;
   evas_object_hide(ee->engine.ews.image);

   if (ee->func.fn_hide) ee->func.fn_hide(ee);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_HIDE);
}

static void
_ecore_evas_ews_raise(Ecore_Evas *ee)
{
   evas_object_raise(ee->engine.ews.image);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_RAISE);
}

static void
_ecore_evas_ews_lower(Ecore_Evas *ee)
{
   evas_object_lower(ee->engine.ews.image);
   evas_object_lower(_ews_bg);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_LOWER);
}

static void
_ecore_evas_ews_activate(Ecore_Evas *ee)
{
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_ACTIVATE);
}

static void
_ecore_evas_ews_title_set(Ecore_Evas *ee, const char *t)
{
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (t) ee->prop.title = strdup(t);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   if (n) ee->prop.name = strdup(n);
   if (c) ee->prop.clas = strdup(c);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
   evas_object_size_hint_min_set(ee->engine.ews.image, w, h);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
   evas_object_size_hint_max_set(ee->engine.ews.image, w, h);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
   evas_object_size_hint_request_set(ee->engine.ews.image, w, h);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_object_cursor_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   ee->prop.cursor.object = NULL;
}

static void
_ecore_evas_ews_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
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

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _ecore_evas_ews_object_cursor_del, ee);

   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_layer_set(Ecore_Evas *ee, int layer)
{
   if (layer < EVAS_LAYER_MIN + 1)
     layer = EVAS_LAYER_MIN + 1;
   else if (layer > EVAS_LAYER_MAX)
     layer = EVAS_LAYER_MAX;

   if (ee->prop.layer == layer) return;
   ee->prop.layer = layer;
   evas_object_layer_set(ee->engine.ews.image, layer);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_LAYER_CHANGE);
}

static void
_ecore_evas_ews_focus_set(Ecore_Evas *ee, int val)
{
   evas_object_focus_set(ee->engine.ews.image, val);
   ee->prop.focused = val;
   if (val)
     {
        evas_focus_in(ee->evas);
        if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
        _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_FOCUS);
     }
   else
     {
        evas_focus_out(ee->evas);
        if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
        _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_UNFOCUS);
     }
}

static void
_ecore_evas_ews_iconified_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.iconified == val) return;
   ee->prop.iconified = val;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE);
}

static void
_ecore_evas_ews_borderless_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.borderless == val) return;
   ee->prop.borderless = val;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_override_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.override == val) return;
   if (ee->visible) evas_object_show(ee->engine.ews.image);
   if (ee->prop.focused) evas_object_focus_set(ee->engine.ews.image, EINA_TRUE);
   ee->prop.override = val;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_maximized_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.maximized == val) return;
   ee->prop.maximized = val;
   if (val) evas_object_show(ee->engine.ews.image);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE);
}

static void
_ecore_evas_ews_fullscreen_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.fullscreen == val) return;
   ee->prop.fullscreen = val;

   if (!val)
     {
        evas_object_move(ee->engine.ews.image, ee->x, ee->y);
        evas_object_resize(ee->engine.ews.image, ee->w, ee->h);
     }
   else
     {
        Evas_Coord w, h;
        ecore_evas_geometry_get(_ews_ee, NULL, NULL, &w, &h);
        evas_object_move(ee->engine.ews.image, 0, 0);
        evas_object_resize(ee->engine.ews.image, w, h);
        evas_object_focus_set(ee->engine.ews.image, EINA_TRUE);
     }

   if (ee->should_be_visible)
     evas_object_show(ee->engine.ews.image);
   else
     evas_object_hide(ee->engine.ews.image);

   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE);
}

static void
_ecore_evas_ews_avoid_damage_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.avoid_damage == val) return;
   ee->prop.avoid_damage = val;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_withdrawn_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.withdrawn == val) return;
   ee->prop.withdrawn = val;
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_sticky_set(Ecore_Evas *ee, int val)
{
   if (ee->prop.sticky == val) return;
   ee->prop.sticky = val;
   if ((val) && (ee->func.fn_sticky)) ee->func.fn_sticky(ee);
   else if ((!val) && (ee->func.fn_unsticky)) ee->func.fn_unsticky(ee);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_ignore_events_set(Ecore_Evas *ee, int val)
{
   if (ee->ignore_events == val) return;
   ee->ignore_events = val;
   evas_object_pass_events_set(ee->engine.ews.image, val);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_alpha_set(Ecore_Evas *ee, int val)
{
   if (ee->alpha == val) return;
   ee->alpha = val;
   evas_object_image_alpha_set(ee->engine.ews.image, val);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static void
_ecore_evas_ews_transparent_set(Ecore_Evas *ee, int val)
{
   if (ee->transparent == val) return;
   ee->transparent = val;
   evas_object_image_alpha_set(ee->engine.ews.image, val);
   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE);
}

static int
_ecore_evas_ews_render(Ecore_Evas *ee)
{
   Eina_List *updates, *l, *ll;
   Ecore_Evas *ee2;
   Eina_Rectangle *r;
   int w, h, rend = 0;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        rend |= _ecore_evas_ews_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   evas_object_image_size_get(ee->engine.ews.image, &w, &h);
   if ((w != ee->w) || (h != ee->h))
     ecore_evas_resize(ee, w, h);

   updates = evas_render_updates(ee->evas);

   EINA_LIST_FOREACH(updates, l, r)
     evas_object_image_data_update_add(ee->engine.ews.image,
                                       r->x, r->y, r->w, r->h);

   if (updates)
     {
        evas_render_updates_free(updates);
        _ecore_evas_idle_timeout_update(ee);
     }

   return updates ? 1 : rend;
}

static void
_ecore_evas_ews_screen_geometry_get(const Ecore_Evas *ee __UNUSED__, int *x, int *y, int *w, int *h)
{
   ecore_evas_geometry_get(_ews_ee, x, y, w, h);
}

static const Ecore_Evas_Engine_Func _ecore_ews_engine_func =
{
     _ecore_evas_ews_free,
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
     _ecore_evas_ews_move,
     _ecore_evas_ews_managed_move,
     _ecore_evas_ews_resize,
     _ecore_evas_ews_move_resize,
     _ecore_evas_ews_rotation_set,
     _ecore_evas_ews_shaped_set,
     _ecore_evas_ews_show,
     _ecore_evas_ews_hide,
     _ecore_evas_ews_raise,
     _ecore_evas_ews_lower,
     _ecore_evas_ews_activate,
     _ecore_evas_ews_title_set,
     _ecore_evas_ews_name_class_set,
     _ecore_evas_ews_size_min_set,
     _ecore_evas_ews_size_max_set,
     _ecore_evas_ews_size_base_set,
     _ecore_evas_ews_size_step_set,
     _ecore_evas_ews_object_cursor_set,
     _ecore_evas_ews_layer_set,
     _ecore_evas_ews_focus_set,
     _ecore_evas_ews_iconified_set,
     _ecore_evas_ews_borderless_set,
     _ecore_evas_ews_override_set,
     _ecore_evas_ews_maximized_set,
     _ecore_evas_ews_fullscreen_set,
     _ecore_evas_ews_avoid_damage_set,
     _ecore_evas_ews_withdrawn_set,
     _ecore_evas_ews_sticky_set,
     _ecore_evas_ews_ignore_events_set,
     _ecore_evas_ews_alpha_set,
     _ecore_evas_ews_transparent_set,
     _ecore_evas_ews_render,
     _ecore_evas_ews_screen_geometry_get
};

void
_ecore_evas_ews_events_init(void)
{
   if (ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE != 0) return;
   ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_ADD = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_DEL = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_RESIZE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_MOVE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_SHOW = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_HIDE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_FOCUS = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_UNFOCUS = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_RAISE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_LOWER = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_ACTIVATE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_LAYER_CHANGE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE = ecore_event_type_new();
   ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE = ecore_event_type_new();
}

static int
_ecore_evas_ews_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

   _ecore_evas_ews_env_setup();

   return _ecore_evas_init_count;
}

int
_ecore_evas_ews_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        if (_ews_ee)
          {
             ecore_evas_free(_ews_ee);
             _ews_ee = NULL;
          }
        if (_ews_children)
          {
             eina_list_free(_ews_children);
             _ews_children = NULL;
          }

        free(_ews_engine);
        _ews_engine = NULL;
        free(_ews_options);
        _ews_options = NULL;
        _ews_defaults_engine = EINA_TRUE;
        _ews_defaults_geo = EINA_TRUE;

     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_ews_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord xx, yy, ww, hh, fx, fy, fw, fh;

   evas_object_geometry_get(ee->engine.ews.image, &xx, &yy, &ww, &hh);
   evas_object_image_fill_get(ee->engine.ews.image, &fx, &fy, &fw, &fh);

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
_ecore_evas_ews_modifiers_apply(Ecore_Evas *ee, const Evas_Modifier *modifier)
{
   Evas *e = ee->evas;

   if (evas_key_modifier_is_set(modifier, "Shift"))
     evas_key_modifier_on(e, "Shift");
   else evas_key_modifier_off(e, "Shift");

   if (evas_key_modifier_is_set(modifier, "Control"))
     evas_key_modifier_on(e, "Control");
   else evas_key_modifier_off(e, "Control");

   if (evas_key_modifier_is_set(modifier, "Alt"))
     evas_key_modifier_on(e, "Alt");
   else evas_key_modifier_off(e, "Alt");

   if (evas_key_modifier_is_set(modifier, "Super"))
     evas_key_modifier_on(e, "Super");
   else evas_key_modifier_off(e, "Super");

   if (evas_key_modifier_is_set(modifier, "Hyper"))
     evas_key_modifier_on(e, "Hyper");
   else evas_key_modifier_off(e, "Hyper");

   if (evas_key_modifier_is_set(modifier, "Scroll_Lock"))
     evas_key_lock_on(e, "Scroll_Lock");
   else evas_key_lock_off(e, "Scroll_Lock");

   if (evas_key_modifier_is_set(modifier, "Num_Lock"))
     evas_key_lock_on(e, "Num_Lock");
   else evas_key_lock_off(e, "Num_Lock");

   if (evas_key_modifier_is_set(modifier, "Caps_Lock"))
     evas_key_lock_on(e, "Caps_Lock");
   else evas_key_lock_off(e, "Caps_Lock");

   if (evas_key_modifier_is_set(modifier, "Shift_Lock"))
     evas_key_lock_on(e, "Shift_Lock");
   else evas_key_lock_off(e, "Shift_Lock");
}

static void
_ecore_evas_ews_cb_mouse_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_In *ev = event_info;
   Evas_Coord x = ev->canvas.x;
   Evas_Coord y = ev->canvas.y;
   _ecore_evas_ews_coord_translate(ee, &x, &y);
   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
   _ecore_evas_mouse_move_process(ee, x, y, ev->timestamp);
}

static void
_ecore_evas_ews_cb_mouse_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Out *ev = event_info;
   Evas_Coord x = ev->canvas.x;
   Evas_Coord y = ev->canvas.y;
   // TODO: consider grab mode in EWS
   _ecore_evas_ews_coord_translate(ee, &x, &y);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
   _ecore_evas_mouse_move_process(ee, x, y, ev->timestamp);
}

static void
_ecore_evas_ews_cb_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Down *ev = event_info;
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_mouse_down(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_ews_cb_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Up *ev = event_info;
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_mouse_up(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_ews_cb_mouse_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x = ev->cur.canvas.x;
   Evas_Coord y = ev->cur.canvas.y;
   _ecore_evas_ews_coord_translate(ee, &x, &y);
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   _ecore_evas_mouse_move_process(ee, x, y, ev->timestamp);
}

static void
_ecore_evas_ews_cb_mouse_wheel(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Wheel *ev = event_info;
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_mouse_wheel(ee->evas, ev->direction, ev->z, ev->timestamp, NULL);
}

static void
_ecore_evas_ews_cb_multi_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Multi_Down *ev = event_info;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   x = ev->canvas.x;
   y = ev->canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_ews_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_multi_down(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_ews_cb_multi_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Multi_Up *ev = event_info;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   x = ev->canvas.x;
   y = ev->canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_ews_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_multi_up(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_ews_cb_multi_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Multi_Move *ev = event_info;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   x = ev->cur.canvas.x;
   y = ev->cur.canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_ews_coord_translate(ee, &x, &y);
   xf = (ev->cur.canvas.xsub - (double)xx) + (double)x;
   yf = (ev->cur.canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_ews_modifiers_apply(ee, ev->modifiers);
   evas_event_feed_multi_move(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->timestamp, NULL);
}

static void
_ecore_evas_ews_cb_free(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   if (ee->driver) _ecore_evas_free(ee);
}

static void
_ecore_evas_ews_cb_key_down(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Key_Down *ev = event_info;

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
_ecore_evas_ews_cb_key_up(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Key_Up *ev = event_info;

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
_ecore_evas_ews_cb_focus_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_focus_set(ee, EINA_TRUE);
}

static void
_ecore_evas_ews_cb_focus_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   if (ee->deleted) return;
   ecore_evas_focus_set(ee, EINA_FALSE);
}

static void
_ecore_evas_ews_cb_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_show(ee);
}

static void
_ecore_evas_ews_cb_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;
   if (ee->deleted) return;
   ecore_evas_hide(ee);
}
#endif

EAPI Ecore_Evas *
ecore_evas_ews_new(int x, int y, int w, int h)
{
// basically a copy of ecore_evas_buffer_new() keep in sync...
#ifdef BUILD_ECORE_EVAS_EWS
   Evas_Object *o;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas *ee;
   int rmethod;

   if (_ecore_evas_ews_init() < 1) return NULL;

   if (!_ews_ee) _ews_ee = _ecore_evas_ews_ee_new();
   if (!_ews_ee)
     {
        ERR("Could not create EWS backing store");
        _ecore_evas_ews_shutdown();
        return NULL;
     }

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   o = evas_object_image_add(_ews_ee->evas);
   evas_object_image_content_hint_set(o, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_ews_engine_func;

   ee->driver = EWS_ENGINE_NAME;

   if (w < 1) w = 1;
   if (h < 1) h = 1;

   ee->x = 0;
   ee->y = 0;
   ee->w = w;
   ee->h = h;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   evas_object_image_fill_set(o, 0, 0, w, h);

   ee->engine.ews.image = o;
   evas_object_data_set(ee->engine.ews.image, "Ecore_Evas", ee);
   evas_object_image_size_set(o, ee->w, ee->h);
   evas_object_image_alpha_set(o, 1);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MOUSE_IN,
                                  _ecore_evas_ews_cb_mouse_in, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MOUSE_OUT,
                                  _ecore_evas_ews_cb_mouse_out, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _ecore_evas_ews_cb_mouse_down, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MOUSE_UP,
                                  _ecore_evas_ews_cb_mouse_up, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MOUSE_MOVE,
                                  _ecore_evas_ews_cb_mouse_move, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MOUSE_WHEEL,
                                  _ecore_evas_ews_cb_mouse_wheel, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MULTI_DOWN,
                                  _ecore_evas_ews_cb_multi_down, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MULTI_UP,
                                  _ecore_evas_ews_cb_multi_up, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_MULTI_MOVE,
                                  _ecore_evas_ews_cb_multi_move, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_FREE,
                                  _ecore_evas_ews_cb_free, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_KEY_DOWN,
                                  _ecore_evas_ews_cb_key_down, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_KEY_UP,
                                  _ecore_evas_ews_cb_key_up, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_FOCUS_IN,
                                  _ecore_evas_ews_cb_focus_in, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_FOCUS_OUT,
                                  _ecore_evas_ews_cb_focus_out, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_SHOW,
                                  _ecore_evas_ews_cb_show, ee);
   evas_object_event_callback_add(ee->engine.ews.image,
                                  EVAS_CALLBACK_HIDE,
                                  _ecore_evas_ews_cb_hide, ee);
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        void *pixels = evas_object_image_data_get(o, 1);
        evas_object_image_data_set(o, pixels); // refcount
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        einfo->info.dest_buffer = pixels;
        einfo->info.dest_buffer_row_bytes = evas_object_image_stride_get(o);
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

   _ews_ee->sub_ecore_evas = eina_list_append(_ews_ee->sub_ecore_evas, ee);
   _ews_children = eina_list_append(_ews_children, ee);

   _ecore_evas_ews_event(ee, ECORE_EVAS_EWS_EVENT_ADD);

   return ee;
#else
   return NULL;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
#endif
}

EAPI Evas_Object *
ecore_evas_ews_backing_store_get(const Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_EWS
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_ews_backing_store_get");
        return NULL;
     }
   return ee->engine.ews.image;
#else
   return NULL;
   (void)ee;
#endif
}

EAPI void
ecore_evas_ews_delete_request(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_EWS
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_ews_delete_request");
        return;
     }
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);
   else ecore_evas_free(ee);
#else
   (void)ee;
#endif
}


EAPI Eina_Bool
ecore_evas_ews_engine_set(const char *engine, const char *options)
{
#ifdef BUILD_ECORE_EVAS_EWS
   if (_ews_ee) return EINA_FALSE;

   free(_ews_engine);
   free(_ews_options);

   _ews_engine = engine ? strdup(engine) : NULL;
   _ews_options = options ? strdup(options) : NULL;

   if ((engine) && (!_ews_engine)) return EINA_FALSE;
   if ((options) && (!_ews_options)) return EINA_FALSE;

   _ews_defaults_engine = EINA_FALSE;
   return EINA_TRUE;
#else
   return EINA_FALSE;
   (void)engine;
   (void)options;
#endif
}

EAPI Eina_Bool
ecore_evas_ews_setup(int x, int y, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_EWS
   Eina_Bool ret = EINA_TRUE;

   _ews_defaults_geo = EINA_FALSE;
   _ews_x = x;
   _ews_y = y;
   _ews_w = w;
   _ews_h = h;

   if (!_ews_ee) return EINA_TRUE;

   /* move-resize is not as implemented as move + resize */
   ecore_evas_move(_ews_ee, x, y);
   ecore_evas_resize(_ews_ee, w, h);

   ecore_evas_geometry_get(_ews_ee, &x, &y, &w, &h);

#define TST(n) if ((n != _ews_##n))                     \
     {                                                  \
        WRN("Asked %d, got %d for "#n, _ews_##n, n);    \
        ret = EINA_FALSE;                               \
     }
   TST(x);
   TST(y);
   TST(w);
   TST(h);
#undef TST
   return ret;
#else
   return EINA_FALSE;
   (void)x;
   (void)y;
   (void)w;
   (void)h;
#endif
}

EAPI Ecore_Evas *
ecore_evas_ews_ecore_evas_get(void)
{
#ifdef BUILD_ECORE_EVAS_EWS
   if (!_ews_ee) _ews_ee = _ecore_evas_ews_ee_new();
   return _ews_ee;
#else
   return NULL;
#endif
}

EAPI Evas *
ecore_evas_ews_evas_get(void)
{
#ifdef BUILD_ECORE_EVAS_EWS
   return ecore_evas_get(ecore_evas_ews_ecore_evas_get());
#else
   return NULL;
#endif
}

EAPI Evas_Object *
ecore_evas_ews_background_get(void)
{
   return _ews_bg;
}

static void
_ecore_evas_ews_background_free(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   _ews_bg = NULL;
   ecore_evas_ews_background_set(NULL);
}

EAPI void
ecore_evas_ews_background_set(Evas_Object *o)
{
   if ((o) && (o == _ews_bg)) return;

   if (_ews_bg)
     {
        evas_object_del(_ews_bg);
        _ews_bg = NULL;
     }

   if ((!o) && (_ews_ee))
     {
        o = evas_object_rectangle_add(ecore_evas_get(_ews_ee));
        evas_object_color_set(o, 0, 0, 0, 255);
     }

   if (_ews_ee)
     {
        Evas_Coord w, h;
        Evas *e = ecore_evas_get(_ews_ee);

        if (e != evas_object_evas_get(o))
          {
             ERR("background not in ecore_evas_ews_evas_get() canvas!");
             return;
          }

        evas_output_viewport_get(e, NULL, NULL, &w, &h);
        evas_object_move(o, 0, 0);
        evas_object_resize(o, w, h);
        evas_object_layer_set(o, EVAS_LAYER_MIN);
        evas_object_lower(o);
        evas_object_show(o);

        evas_object_event_callback_add
          (o, EVAS_CALLBACK_FREE, _ecore_evas_ews_background_free, NULL);
     }

   _ews_bg = o;
}


EAPI const Eina_List *
ecore_evas_ews_children_get(void)
{
#ifdef BUILD_ECORE_EVAS_EWS
   return _ews_children;
#else
   return NULL;
#endif
}

EAPI void
ecore_evas_ews_manager_set(const void *manager)
{
#ifdef BUILD_ECORE_EVAS_EWS
   if (_ews_manager ==  manager) return;
   _ews_manager = manager;
   ecore_event_add(ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE, NULL, NULL, NULL);
#else
   (void)manager;
#endif
}

EAPI const void *
ecore_evas_ews_manager_get(void)
{
#ifdef BUILD_ECORE_EVAS_EWS
   return _ews_manager;
#else
   return NULL;
#endif
}
