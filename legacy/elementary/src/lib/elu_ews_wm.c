#include <Elementary.h>
#include "elm_priv.h"

static Eina_Bool _ews_used = EINA_FALSE;
static Eina_List *_ews_ev_handlers = NULL;
static Eina_Hash *_ews_borders = NULL;
static Eina_Hash *_ews_borders_geo = NULL;
static Evas_Object *_ews_bg = NULL;
static Ecore_Animator *_ews_border_mover = NULL;
static Evas_Object *_ews_border_mover_obj = NULL;
static Evas_Point _ews_border_mover_off = {0, 0};

static void
_elm_ews_border_usable_screen_geometry_get(int *x, int *y, int *w, int *h)
{
   Ecore_Evas *ee = ecore_evas_ews_ecore_evas_get();
   ecore_evas_geometry_get(ee, NULL, NULL, w, h);
   if (x) *x = 0;
   if (y) *y = 0;
   // TODO: when add a shelf for iconified, subtract its area here.
}

static void
_elm_ews_wm_border_del(void *data)
{
   Evas_Object *deco = data;
   evas_object_del(deco);

   if (_ews_border_mover_obj == deco)
     {
        if (_ews_border_mover)
          {
             ecore_animator_del(_ews_border_mover);
             _ews_border_mover = NULL;
          }
        _ews_border_mover_obj = NULL;
     }
}

static Evas_Object *
_elm_ews_wm_border_find(const Ecore_Evas *ee)
{
   return eina_hash_find(_ews_borders, &ee);
}

static Eina_Rectangle *
_elm_ews_wm_border_geo_find(const Ecore_Evas *ee)
{
   return eina_hash_find(_ews_borders_geo, &ee);
}

static void
_elm_ews_border_geo_apply(Ecore_Evas *ee, Evas_Object *o)
{
   int x, y, w, h;
   ecore_evas_geometry_get(ee, &x, &y, &w, &h);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
}

static void
_elm_ews_border_focus_apply(Ecore_Evas *ee, Evas_Object *o)
{
   const char *sig;
   if (ecore_evas_focus_get(ee))
     sig = "elm,state,focus,on";
   else
     sig = "elm,state,focus,off";
   edje_object_signal_emit(o, sig, "elm");
}

static void
_elm_ews_border_stack_apply(Ecore_Evas *ee, Evas_Object *o)
{
   Evas_Object *bs_o = ecore_evas_ews_backing_store_get(ee);
   evas_object_stack_below(o, bs_o);
}

static void
_elm_ews_border_iconified_apply(Ecore_Evas *ee, Evas_Object *o)
{
   const char *sig;
   if (ecore_evas_iconified_get(ee))
     sig = "elm,state,iconified,on";
   else
     sig = "elm,state,iconified,off";
   edje_object_signal_emit(o, sig, "elm");

   // TODO: add to some taskbar? and actually hide it?
   DBG("EWS does not implement iconified yet");
}

static void
_elm_ews_border_maximized_apply(Ecore_Evas *ee, Evas_Object *o)
{
   int x, y, w, h;
   if (ecore_evas_maximized_get(ee))
     {
        Eina_Rectangle *r;
        int ex, ey, ew, eh;

        edje_object_signal_emit(o, "elm,state,maximized,on", "elm");
        edje_object_message_signal_process(o);
        ecore_evas_geometry_get(ee, &x, &y, &w, &h);

        r = _elm_ews_wm_border_geo_find(ee);
        if (!r)
          {
             r = malloc(sizeof(Eina_Rectangle));
             eina_hash_add(_ews_borders_geo, &ee, r);
          }

        r->x = x;
        r->y = y;
        r->w = w;
        r->h = h;
        _elm_ews_border_usable_screen_geometry_get(&x, &y, &w, &h);
        edje_object_parts_extends_calc(o, &ex, &ey, &ew, &eh);
        x -= ex;
        y -= ey;
        w -= ew - r->w;
        h -= eh - r->h;
     }
   else
     {
        Eina_Rectangle *r = _elm_ews_wm_border_geo_find(ee);
        edje_object_signal_emit(o, "elm,state,maximized,off", "elm");

        if (!r) ecore_evas_geometry_get(ee, &x, &y, &w, &h);
        else
          {
             x = r->x;
             y = r->y;
             w = r->w;
             h = r->h;
          }
     }

   ecore_evas_move_resize(ee, x, y, w, h);
   _elm_ews_border_geo_apply(ee, o);
}

static void
_elm_ews_border_layer_apply(Ecore_Evas *ee, Evas_Object *o)
{
   Evas_Object *bs_o = ecore_evas_ews_backing_store_get(ee);
   evas_object_layer_set(o, evas_object_layer_get(bs_o));
   _elm_ews_border_stack_apply(ee, o);
}

static void
_elm_ews_border_fullscreen_apply(Ecore_Evas *ee, Evas_Object *o)
{
   const char *sig;
   if (ecore_evas_fullscreen_get(ee))
     sig = "elm,state,fullscreen,on";
   else
     sig = "elm,state,fullscreen,off";
   edje_object_signal_emit(o, sig, "elm");
   _elm_ews_border_geo_apply(ee, o);
}

static void
_elm_ews_border_config_apply(Ecore_Evas *ee, Evas_Object *o, Elm_Theme *th)
{
   const char *title, *name = NULL, *class = NULL, *style = NULL;
   const char *sig;

   if (ecore_evas_borderless_get(ee))
     style = "borderless";

   _elm_theme_set(th, o, "ews", "decoration", style ? style : "default");

   if (ecore_evas_shaped_get(ee) || ecore_evas_alpha_get(ee) ||
       ecore_evas_transparent_get(ee))
     sig = "elm,state,alpha,on";
   else
     sig = "elm,state,alpha,off";
   edje_object_signal_emit(o, sig, "elm");

   title = ecore_evas_title_get(ee);
   ecore_evas_name_class_get(ee, &name, &class);
   edje_object_part_text_set(o, "elm.text.title", title);
   edje_object_part_text_set(o, "elm.text.name", name);
   edje_object_part_text_set(o, "elm.text.class", class);

   _elm_ews_border_geo_apply(ee, o);
   _elm_ews_border_focus_apply(ee, o);
   _elm_ews_border_stack_apply(ee, o);
   _elm_ews_border_iconified_apply(ee, o);
   _elm_ews_border_maximized_apply(ee, o);
   _elm_ews_border_layer_apply(ee, o);
   _elm_ews_border_fullscreen_apply(ee, o);
}

static Eina_Bool
_elm_ews_wm_border_theme_set(Ecore_Evas *ee, Evas_Object *o, Elm_Theme *th)
{
   _elm_ews_border_config_apply(ee, o, th);
   return EINA_TRUE;
}

static void
_elm_ews_border_sig_focus(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_focus_set(ee, EINA_TRUE);
}

static void
_elm_ews_border_sig_iconify(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_iconified_set(ee, EINA_TRUE);
}

static void
_elm_ews_border_sig_maximize(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_maximized_set(ee, EINA_TRUE);
}

static void
_elm_ews_border_sig_fullscreen(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_fullscreen_set(ee, EINA_TRUE);
}

static void
_elm_ews_border_sig_restore(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_iconified_set(ee, EINA_FALSE);
   ecore_evas_maximized_set(ee, EINA_FALSE);
   ecore_evas_fullscreen_set(ee, EINA_FALSE);
}

static void
_elm_ews_border_sig_close(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   ecore_evas_ews_delete_request(ee);
}

static void
_elm_ews_border_sig_menu(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   // TODO: show some menu?
   ERR("EWS does not implement menu yet");
   (void)data;
}

static Eina_Bool
_elm_ews_border_mover(void *data)
{
   Ecore_Evas *ee = data;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   int x, y;

   evas_pointer_output_xy_get(ecore_evas_ews_evas_get(), &x, &y);
   x -= _ews_border_mover_off.x;
   y -= _ews_border_mover_off.y;
   ecore_evas_move(ee, x, y);
   evas_object_move(o, x, y);

   return EINA_TRUE;
}

static void
_elm_ews_border_sig_move_start(void *data, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   Ecore_Evas *ee = data;
   Evas_Object *bs_o = ecore_evas_ews_backing_store_get(ee);
   int x, y, ox, oy;

   if (_ews_border_mover) ecore_animator_del(_ews_border_mover);

   evas_pointer_output_xy_get(evas_object_evas_get(bs_o), &x, &y);
   evas_object_geometry_get(bs_o, &ox, &oy, NULL, NULL);
   _ews_border_mover_off.x = x - ox;
   _ews_border_mover_off.y = y - oy;
   _ews_border_mover_obj = bs_o;
   _ews_border_mover = ecore_animator_add(_elm_ews_border_mover, ee);
}

static void
_elm_ews_border_sig_move_stop(void *data __UNUSED__, Evas_Object *o __UNUSED__, const char *sig __UNUSED__, const char *source __UNUSED__)
{
   if (!_ews_border_mover) return;
   ecore_animator_del(_ews_border_mover);
   _ews_border_mover = NULL;
   _ews_border_mover_obj = NULL;
}

static Eina_Bool
_elm_ews_wm_add_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = edje_object_add(ecore_evas_ews_evas_get());
   Evas_Coord x, y, w, h, sw, sh;

   edje_object_signal_callback_add
     (o, "elm,action,focus", "elm", _elm_ews_border_sig_focus, ee);
   edje_object_signal_callback_add
     (o, "elm,action,iconify", "elm", _elm_ews_border_sig_iconify, ee);
   edje_object_signal_callback_add
     (o, "elm,action,maximize", "elm", _elm_ews_border_sig_maximize, ee);
   edje_object_signal_callback_add
     (o, "elm,action,fullscreen", "elm", _elm_ews_border_sig_fullscreen, ee);
   edje_object_signal_callback_add
     (o, "elm,action,restore", "elm", _elm_ews_border_sig_restore, ee);
   edje_object_signal_callback_add
     (o, "elm,action,close", "elm", _elm_ews_border_sig_close, ee);
   edje_object_signal_callback_add
     (o, "elm,action,menu", "elm", _elm_ews_border_sig_menu, ee);
   edje_object_signal_callback_add
     (o, "elm,action,move,start", "elm", _elm_ews_border_sig_move_start, ee);
   edje_object_signal_callback_add
     (o, "elm,action,move,stop", "elm", _elm_ews_border_sig_move_stop, ee);

   eina_hash_add(_ews_borders, &ee, o);
   _elm_ews_wm_border_theme_set(ee, o, NULL);

   ecore_evas_screen_geometry_get(ee, NULL, NULL, &sw, &sh);
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   x = (sw - w) / 2;
   y = (sh - h) / 2;
   ecore_evas_move(ee, x, y);
   ecore_evas_focus_set(ee, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_del_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   eina_hash_del(_ews_borders, &ee, NULL);
   eina_hash_del(_ews_borders_geo, &ee, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_geo_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_geo_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_show_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   evas_object_show(o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_hide_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   evas_object_hide(o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_focus_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_focus_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_stack_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_stack_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_iconified_change_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_iconified_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_maximized_change_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_maximized_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_layer_change_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_layer_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_fullscreen_change_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_fullscreen_apply(ee, o);
   return EINA_TRUE;
}

static Eina_Bool
_elm_ews_wm_config_change_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Evas *ee = event_info;
   Evas_Object *o = _elm_ews_wm_border_find(ee);
   _elm_ews_border_config_apply(ee, o, NULL);
   return EINA_TRUE;
}

void
_elm_ews_wm_rescale(Elm_Theme *th, Eina_Bool use_theme)
{
   Eina_Iterator *it = eina_hash_iterator_tuple_new(_ews_borders);
   Eina_Hash_Tuple *tp = NULL;

   if (!use_theme)
     {
        EINA_ITERATOR_FOREACH(it, tp)
          _elm_ews_wm_border_theme_set(*(void**)tp->key, tp->data, NULL);

        if (_ews_bg)
          _elm_theme_set(NULL, _ews_bg, "ews", "background", "default");
     }
   else
     {
        EINA_ITERATOR_FOREACH(it, tp)
          _elm_ews_wm_border_theme_set(*(void**)tp->key, tp->data, th);

        if (_ews_bg)
          _elm_theme_set(th, _ews_bg, "ews", "background", "default");
     }

   eina_iterator_free(it);
}

int
_elm_ews_wm_init(void)
{
   Evas *e;
   Evas_Object *o;

   if (strcmp(_elm_config->engine, ELM_EWS) != 0)
     {
        _ews_used = EINA_FALSE;
        return EINA_TRUE;
     }

   e = ecore_evas_ews_evas_get();
   if (!e) return EINA_FALSE;
   o = edje_object_add(e);
   if (!o) return EINA_FALSE;

   if (!_elm_theme_set(NULL, o, "ews", "background", "default"))
     {
        ERR("Could not set background theme, fallback to rectangle");
        evas_object_del(o);
        _ews_bg = o = NULL;
     }
   else
     _ews_bg = o;
   ecore_evas_ews_background_set(o);


#define ADD_EH(ev, cb)                                          \
   _ews_ev_handlers = eina_list_append                          \
     (_ews_ev_handlers, ecore_event_handler_add(ev, cb, NULL))
   ADD_EH(ECORE_EVAS_EWS_EVENT_ADD, _elm_ews_wm_add_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_DEL, _elm_ews_wm_del_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_RESIZE, _elm_ews_wm_geo_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_MOVE, _elm_ews_wm_geo_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_SHOW, _elm_ews_wm_show_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_HIDE, _elm_ews_wm_hide_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_FOCUS, _elm_ews_wm_focus_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_UNFOCUS, _elm_ews_wm_focus_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_RAISE, _elm_ews_wm_stack_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_LOWER, _elm_ews_wm_stack_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE, _elm_ews_wm_iconified_change_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE, _elm_ews_wm_maximized_change_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_LAYER_CHANGE, _elm_ews_wm_layer_change_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE, _elm_ews_wm_fullscreen_change_cb);
   ADD_EH(ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE, _elm_ews_wm_config_change_cb);
#undef ADD_EH

   if (!_ews_borders)
     _ews_borders = eina_hash_pointer_new(_elm_ews_wm_border_del);

   if (!_ews_borders_geo)
     _ews_borders_geo = eina_hash_pointer_new(free);

   _ews_used = EINA_TRUE;
   return EINA_TRUE;
}

void
_elm_ews_wm_shutdown(void)
{
   Ecore_Event_Handler *eh;

   if (_ews_border_mover)
     {
        ecore_animator_del(_ews_border_mover);
        _ews_border_mover = NULL;
     }
   _ews_border_mover_obj = NULL;

   EINA_LIST_FREE(_ews_ev_handlers, eh) ecore_event_handler_del(eh);
   if (_ews_borders)
     {
        eina_hash_free(_ews_borders);
        _ews_borders = NULL;
     }
   if (_ews_borders_geo)
     {
        eina_hash_free(_ews_borders_geo);
        _ews_borders_geo = NULL;
     }
   _ews_bg = NULL;
}
