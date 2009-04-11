#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Elm_Win Elm_Win;

struct _Elm_Win
{
   Ecore_Evas     *ee;
   Evas           *evas;
   Evas_Object    *parent;
   Evas_Object    *win_obj;
   Eina_List      *subobjs;
   Ecore_X_Window  xwin;
   Ecore_Job      *deferred_resize_job;
   Ecore_Job      *deferred_child_eval_job;
   
   Elm_Win_Type          type;
   Elm_Win_Keyboard_Mode kbdmode;
   Evas_Bool             autodel : 1;
   int                   *autodel_clear;
};

static void _elm_win_obj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_win_resize(Ecore_Evas *ee);
static void _elm_win_delete_request(Ecore_Evas *ee);
static void _elm_win_resize_job(void *data);
#ifdef HAVE_ELEMENTARY_X   
static void _elm_win_xwin_update(Elm_Win *win);
#endif
static void _elm_win_eval_subobjs(Evas_Object *obj);
static void _elm_win_subobj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_win_subobj_callback_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static Eina_List *_elm_win_list = NULL;

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win *win = elm_widget_data_get(ecore_evas_object_associate_get(ee));
   if (!win) return;
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   win->deferred_resize_job = ecore_job_add(_elm_win_resize_job, win);
}

static void
_elm_win_focus_in(Ecore_Evas *ee)
{
   Elm_Win *win = elm_widget_data_get(ecore_evas_object_associate_get(ee));
   if (!win) return;
   evas_object_smart_callback_call(win->win_obj, "focus-in", NULL);
}

static void
_elm_win_focus_out(Ecore_Evas *ee)
{
   Elm_Win *win = elm_widget_data_get(ecore_evas_object_associate_get(ee));
   if (!win) return;
   evas_object_smart_callback_call(win->win_obj, "focus-out", NULL);
}

static void
_deferred_ecore_evas_free(void *data)
{
   ecore_evas_free(data);
}

static void
_elm_win_obj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Elm_Win *win = data;

   if (win->autodel_clear) *(win->autodel_clear) = -1;
   _elm_win_list = eina_list_remove(_elm_win_list, win->win_obj);
   while (win->subobjs) elm_win_resize_object_del(obj, win->subobjs->data);
   ecore_evas_callback_delete_request_set(win->ee, NULL);
   ecore_evas_callback_resize_set(win->ee, NULL);
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   if (win->deferred_child_eval_job) ecore_job_del(win->deferred_child_eval_job);
   while (evas_object_bottom_get(win->evas) &&
	  (evas_object_bottom_get(win->evas) != obj))
     {
        evas_object_del(evas_object_bottom_get(win->evas));
     }
   while (evas_object_top_get(win->evas) &&
	  (evas_object_top_get(win->evas) != obj))
     {
        evas_object_del(evas_object_top_get(win->evas));
     }
// FIXME: we are in the del handler for the object and delete the canvas
// that lives under it from the handler... nasty. deferring doesnt help either
   ecore_job_add(_deferred_ecore_evas_free, win->ee);
//   ecore_evas_free(win->ee);
   free(win);
}

static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Elm_Win *win = elm_widget_data_get(ecore_evas_object_associate_get(ee));
   if (!win) return;
   int autodel = win->autodel;
   win->autodel_clear = &autodel;
   evas_object_smart_callback_call(win->win_obj, "delete-request", NULL);
   // FIXME: if above callback deletes - then the below will be invalid
   if (autodel == 1) evas_object_del(win->win_obj);
   else if (autodel == 0) win->autodel_clear = NULL;
}

static void
_elm_win_resize_job(void *data)
{
   Elm_Win *win = data;
   const Eina_List *l;
   Evas_Object *obj;
   int w, h;
   
   win->deferred_resize_job = NULL;
   ecore_evas_geometry_get(win->ee, NULL, NULL, &w, &h);
   evas_object_resize(win->win_obj, w, h);
   EINA_LIST_FOREACH(win->subobjs, l, obj)
     {
	evas_object_move(obj, 0, 0);
	evas_object_resize(obj, w, h);
     }
}

static void
_elm_win_xwindow_get(Elm_Win *win)
{
   win->xwin = 0;
   switch (_elm_config->engine)
     {
      case ELM_SOFTWARE_X11:
	if (win->ee) win->xwin = ecore_evas_software_x11_window_get(win->ee);
	break;
      case ELM_SOFTWARE_FB:
      case ELM_SOFTWARE_16_WINCE:
	break;
      case ELM_SOFTWARE_16_X11:
	if (win->ee) win->xwin = ecore_evas_software_x11_16_window_get(win->ee);
	break;
      case ELM_XRENDER_X11:
	if (win->ee) win->xwin = ecore_evas_xrender_x11_window_get(win->ee);
	break;
      case ELM_OPENGL_X11:
	if (win->ee) win->xwin = ecore_evas_gl_x11_window_get(win->ee);
	break;
      case ELM_SOFTWARE_WIN32:
	if (win->ee) win->xwin = (long)ecore_evas_win32_window_get(win->ee);
	break;
      default:
	break;
     }
}

static void
_elm_win_xwin_update(Elm_Win *win)
{
#ifdef HAVE_ELEMENTARY_X   
   _elm_win_xwindow_get(win);
   if (win->parent)
     {
	Elm_Win *win2;
	
	win2 = elm_widget_data_get(win->parent);
	if (win2)
	  {
	     if (win->xwin)
	       ecore_x_icccm_transient_for_set(win->xwin, win2->xwin);
	  }
     }
   
   if (win->type == ELM_WIN_BASIC)
     {
	switch (win->type)
          {
	   case ELM_WIN_BASIC:
	     if (win->xwin) 
	       ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_NORMAL);
	     break;
	   case ELM_WIN_DIALOG_BASIC:
	     if (win->xwin) 
	       ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DIALOG);
	     break;
	   case ELM_WIN_DESKTOP:
	      if (win->xwin)
		ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DESKTOP);
	      break;
	   case ELM_WIN_DOCK:
	      if (win->xwin)
		ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DOCK);
	      break;
	   case ELM_WIN_TOOLBAR:
	      if (win->xwin)
		ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_TOOLBAR);
	      break;
	   case ELM_WIN_MENU:
	      if (win->xwin)
		ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_MENU);
	      break;
	   case ELM_WIN_UTILITY:
	      if (win->xwin)
		ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_UTILITY);
	      break;
	   case ELM_WIN_SPLASH:
	      if (win->xwin)
		ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_SPLASH);
	      break;
	   default:
	     break;
	  }
     }
   if (win->xwin)
     ecore_x_e_virtual_keyboard_state_set
     (win->xwin, (Ecore_X_Virtual_Keyboard_State)win->kbdmode);
#endif
}

static void
_elm_win_eval_subobjs(Evas_Object *obj)
{
   const Eina_List *l;
   const Evas_Object *child;
   Elm_Win *win = elm_widget_data_get(obj);
   Evas_Coord w, h, minw = -1, minh = -1, maxw = -1, maxh = -1;
   int xx = 1, xy = 1;
   double wx, wy;

   EINA_LIST_FOREACH(win->subobjs, l, child)
     {
	evas_object_size_hint_weight_get(child, &wx, &wy);
	if (wx == 0.0) xx = 0;
	if (wy == 0.0) xy = 0;
	
	evas_object_size_hint_min_get(child, &w, &h);
	if (w < 1) w = -1;
	if (h < 1) h = -1;
	if (w > minw) minw = w;
	if (h > minh) minh = h;
	
	evas_object_size_hint_max_get(child, &w, &h);
	if (w < 1) w = -1;
	if (h < 1) h = -1;
	if (maxw == -1) maxw = w;
	else if ((w > 0) && (w < maxw)) maxw = w;
	if (maxh == -1) maxh = h;
	else if ((h > 0) && (h < maxh)) maxh = h;
     }
   if ((maxw >= 0) && (maxw < minw)) maxw = minw;
   if ((maxh >= 0) && (maxh < minh)) maxh = minh;
   if (!xx) maxw = minw;
   else maxw = 32767;
   if (!xy) maxh = minh;
   else maxh = 32767;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_elm_win_subobj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_win_resize_object_del(data, obj);
}

static void
_elm_win_subobj_callback_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _elm_win_eval_subobjs(data);
}

void
_elm_win_shutdown(void)
{
   while (_elm_win_list) evas_object_del(_elm_win_list->data);
}

void
_elm_win_rescale(void)
{
   const Eina_List *l;
   Evas_Object *obj;
   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     elm_widget_theme(obj);
}

EAPI Evas_Object *
elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type)
{
   Elm_Win *win;
   const Eina_List *l;
   const char *fontpath;
   
   win = ELM_NEW(Elm_Win);
   switch (_elm_config->engine)
     {
      case ELM_SOFTWARE_X11:
	win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
	break;
      case ELM_SOFTWARE_FB:
	win->ee = ecore_evas_fb_new(NULL, 0, 1, 1);
	break;
      case ELM_SOFTWARE_16_X11:
	win->ee = ecore_evas_software_x11_16_new(NULL, 0, 0, 0, 1, 1);
	break;
      case ELM_XRENDER_X11:
	win->ee = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, 1, 1);
	break;
      case ELM_OPENGL_X11:
	win->ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
	break;
      case ELM_SOFTWARE_WIN32:
	win->ee = ecore_evas_software_ddraw_new(NULL, 0, 0, 1, 1);
	break;
      case ELM_SOFTWARE_16_WINCE:
	win->ee = ecore_evas_software_wince_gdi_new(NULL, 0, 0, 1, 1);
	break;
      default:
	break;
     }
   if (!win->ee)
     {
	EINA_ERROR_PERR("elementary: ERROR. Cannot create window.\n");
	free(win);
	return NULL;
     }
   _elm_win_xwindow_get(win);
   if (_elm_config->bgpixmap && !_elm_config->compositing)
     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
// bg pixmap done by x - has other issues like can be redrawn by x before it
// is filled/ready by app   
//     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);
   
   win->type = type;
   win->parent = parent;
   
   win->evas = ecore_evas_get(win->ee);
   win->win_obj = elm_widget_add(win->evas);
   elm_widget_data_set(win->win_obj, win);
   evas_object_color_set(win->win_obj, 0, 0, 0, 0);
   evas_object_move(win->win_obj, 0, 0);
   evas_object_resize(win->win_obj, 1, 1);
   evas_object_layer_set(win->win_obj, 50);
   evas_object_pass_events_set(win->win_obj, 1);

   ecore_evas_object_associate(win->ee, win->win_obj,
                               ECORE_EVAS_OBJECT_ASSOCIATE_BASE |
                               ECORE_EVAS_OBJECT_ASSOCIATE_STACK |
                               ECORE_EVAS_OBJECT_ASSOCIATE_LAYER);
   evas_object_event_callback_add(win->win_obj, EVAS_CALLBACK_DEL,
                                  _elm_win_obj_callback_del, win);
   
   ecore_evas_name_class_set(win->ee, name, _elm_appname);
   ecore_evas_callback_delete_request_set(win->ee, _elm_win_delete_request);
   ecore_evas_callback_resize_set(win->ee, _elm_win_resize);
   ecore_evas_callback_focus_in_set(win->ee, _elm_win_focus_in);
   ecore_evas_callback_focus_out_set(win->ee, _elm_win_focus_out);
   evas_image_cache_set(win->evas, _elm_config->image_cache * 1024);
   evas_font_cache_set(win->evas, _elm_config->font_cache * 1024);
   EINA_LIST_FOREACH(_elm_config->font_dirs, l, fontpath)
     evas_font_path_append(win->evas, fontpath);
   if (_elm_config->font_hinting == 0)
     evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_NONE);
   else if (_elm_config->font_hinting == 1)
     evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_AUTO);
   else if (_elm_config->font_hinting == 2)
     evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_BYTECODE);
   edje_frametime_set(1.0 / 60.0);
   edje_scale_set(_elm_config->scale);

   _elm_win_xwin_update(win);

   _elm_win_list = eina_list_append(_elm_win_list, win->win_obj);
   
   switch (_elm_config->engine)
     {
      case ELM_SOFTWARE_16_WINCE:
      case ELM_SOFTWARE_FB:
        ecore_evas_fullscreen_set(win->ee, 1);
	break;
      case ELM_SOFTWARE_X11:
      case ELM_SOFTWARE_16_X11:
      case ELM_XRENDER_X11:
      case ELM_OPENGL_X11:
      case ELM_SOFTWARE_WIN32:
      default:
	break;
     }
   return win->win_obj;
}

EAPI void
elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj)
{
   Elm_Win *win = elm_widget_data_get(obj);
   Evas_Coord w, h;
   if (!win) return;
   win->subobjs = eina_list_append(win->subobjs, subobj);
   elm_widget_sub_object_add(obj, subobj);
   evas_object_event_callback_add(subobj, EVAS_CALLBACK_DEL, _elm_win_subobj_callback_del, obj);
   evas_object_event_callback_add(subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _elm_win_subobj_callback_changed_size_hints, obj);
   ecore_evas_geometry_get(win->ee, NULL, NULL, &w, &h);
   evas_object_move(subobj, 0, 0);
   evas_object_resize(subobj, w, h);
   _elm_win_eval_subobjs(obj);
}

EAPI void
elm_win_resize_object_del(Evas_Object *obj, Evas_Object *subobj)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   evas_object_event_callback_del(subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _elm_win_subobj_callback_changed_size_hints);
   evas_object_event_callback_del(subobj, EVAS_CALLBACK_DEL, _elm_win_subobj_callback_del);
   win->subobjs = eina_list_remove(win->subobjs, subobj);
   elm_widget_sub_object_del(obj, subobj);
   _elm_win_eval_subobjs(obj);
}

EAPI void
elm_win_title_set(Evas_Object *obj, const char *title)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_title_set(win->ee, title);
}
  
EAPI void
elm_win_autodel_set(Evas_Object *obj, Evas_Bool autodel)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   win->autodel = autodel;
}

EAPI void
elm_win_activate(Evas_Object *obj)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_activate(win->ee);
}

EAPI void
elm_win_lower(Evas_Object *obj)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_lower(win->ee);
}

EAPI void
elm_win_raise(Evas_Object *obj)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_raise(win->ee);
}

EAPI void
elm_win_borderless_set(Evas_Object *obj, Evas_Bool borderless)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_borderless_set(win->ee, borderless);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_shaped_set(Evas_Object *obj, Evas_Bool shaped)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_shaped_set(win->ee, shaped);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_alpha_set(Evas_Object *obj, Evas_Bool alpha)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   if (win->xwin)
     {
	if (alpha)
	  {
	     if (!_elm_config->compositing)
	       elm_win_shaped_set(obj, alpha);
	     else
	       ecore_evas_alpha_set(win->ee, alpha);
	  }
	else
	  ecore_evas_alpha_set(win->ee, alpha);
	_elm_win_xwin_update(win);
     }
   else
     ecore_evas_alpha_set(win->ee, alpha);
}

EAPI void
elm_win_override_set(Evas_Object *obj, Evas_Bool override)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_override_set(win->ee, override);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_fullscreen_set(Evas_Object *obj, Evas_Bool fullscreen)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   switch (_elm_config->engine)
     {
     case ELM_SOFTWARE_16_WINCE:
     case ELM_SOFTWARE_FB:
        // these engines... can ONLY be fullscreen
        break;
     default:
        ecore_evas_fullscreen_set(win->ee, fullscreen);
        _elm_win_xwin_update(win);
        break;
     }
}

EAPI void
elm_win_maximized_set(Evas_Object *obj, Evas_Bool maximized)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_maximized_set(win->ee, maximized);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_iconified_set(Evas_Object *obj, Evas_Bool iconified)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_iconified_set(win->ee, iconified);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_layer_set(Evas_Object *obj, int layer)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_layer_set(win->ee, layer);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_rotation_set(Evas_Object *obj, int rotation)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_rotation_set(win->ee, rotation);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_sticky_set(Evas_Object *obj, Eina_Bool sticky)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_sticky_set(win->ee, sticky);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_keyboard_mode_set(Evas_Object *obj, Elm_Win_Keyboard_Mode mode)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   if (mode == win->kbdmode) return;
   _elm_win_xwindow_get(win);
   win->kbdmode = mode;
#ifdef HAVE_ELEMENTARY_X   
   if (win->xwin)
     ecore_x_e_virtual_keyboard_state_set
     (win->xwin, (Ecore_X_Virtual_Keyboard_State)win->kbdmode);
#endif   
}

EAPI void
elm_win_keyboard_win_set(Evas_Object *obj, Evas_Bool is_keyboard)
{
   Elm_Win *win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X   
   if (win->xwin)
     ecore_x_e_virtual_keyboard_set
     (win->xwin, is_keyboard);
#endif   
}

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *content;
   const char *style;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->style) eina_stringshare_del(wd->style);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->style) _elm_theme_set(wd->frm, "win", "inwin", wd->style);
   else _elm_theme_set(wd->frm, "win", "inwin", "default");
   if (wd->content)
     edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   evas_object_size_hint_min_get(wd->content, &minw, &minh);
   edje_object_size_min_calc(wd->frm, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", obj);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->content)
     {
        evas_object_event_callback_del
          (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
        wd->content = NULL;
        _sizing_eval(obj);
     }
}

EAPI Evas_Object *
elm_win_inwin_add(Evas_Object *obj)
{
   Elm_Win *win = elm_widget_data_get(obj);
   Evas_Object *obj2;
   Widget_Data *wd;
   if (!win) return NULL;
   wd = ELM_NEW(Widget_Data);
   obj2 = elm_widget_add(win->evas);
   evas_object_size_hint_weight_set(obj2, 1.0, 1.0);
   evas_object_size_hint_align_set(obj2, -1.0, -1.0);
   elm_win_resize_object_add(obj, obj2);
   
   elm_widget_data_set(obj2, wd);
   elm_widget_del_hook_set(obj2, _del_hook);
   elm_widget_theme_hook_set(obj2, _theme_hook);
   
   wd->frm = edje_object_add(win->evas);
   _elm_theme_set(wd->frm, "win", "inwin", "default");
   elm_widget_resize_object_set(obj2, wd->frm);
   
   evas_object_smart_callback_add(obj2, "sub-object-del", _sub_del, obj2);
   
   _sizing_eval(obj2);
   return obj2;
}

EAPI void
elm_win_inwin_style_set(Evas_Object *obj, const char *style)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->style) eina_stringshare_del(wd->style);
   if (style) wd->style = eina_stringshare_add(style);
   else wd->style = NULL;
   _theme_hook(obj);
}

EAPI void
elm_win_inwin_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_raise(obj);
   evas_object_show(obj);
   edje_object_signal_emit(wd->frm, "elm,action,show", "elm");
   elm_widget_focused_object_clear(elm_widget_parent_get(obj));
}

EAPI void
elm_win_inwin_content_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
        evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        _sizing_eval(obj);
     }
}

/* windowing spcific calls - shall we do this differently? */
EAPI Ecore_X_Window
elm_win_xwindow_get(const Evas_Object *obj)
{
   Elm_Win *win = (Elm_Win *)elm_widget_data_get(obj);
   if (!win) return 0;
   _elm_win_xwindow_get(win);
   return win->xwin;
}
