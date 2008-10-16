#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Elm_Win Elm_Win;

struct _Elm_Win
{
   Ecore_Evas     *ee;
   Evas           *evas;
   Evas_Object    *parent;
   Evas_Object    *win_obj;
   Evas_List      *subobjs;
   Ecore_X_Window  xwin;
   Ecore_Job      *deferred_resize_job;
   Ecore_Job      *deferred_child_eval_job;
   Elm_Win_Type    type;
   Evas_Bool       autodel : 1;
};

static void _elm_win_resize(Ecore_Evas *ee);
static void _elm_win_obj_intercept_show(void *data, Evas_Object *obj);
static void _elm_win_obj_intercept_hide(void *data, Evas_Object *obj);
static void _elm_win_obj_intercept_move(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _elm_win_obj_intercept_resize(void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _elm_win_obj_intercept_raise(void *data, Evas_Object *obj);
static void _elm_win_obj_intercept_lower(void *data, Evas_Object *obj);
static void _elm_win_obj_intercept_stack_above(void *data, Evas_Object *obj, Evas_Object *above);
static void _elm_win_obj_intercept_stack_below(void *data, Evas_Object *obj, Evas_Object *below);
static void _elm_win_obj_intercept_layer_set(void *data, Evas_Object *obj, int l);
static void _elm_win_obj_intercept_color_set(void *data, Evas_Object *obj, int r, int g, int b, int a);
static void _elm_win_obj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_win_obj_callback_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_win_delete_request(Ecore_Evas *ee);
static void _elm_win_resize_job(void *data);
static void _elm_win_xwin_update(Elm_Win *win);
static void _elm_win_eval_subobjs(Evas_Object *obj);
static void _elm_win_subobj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_win_subobj_callback_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static Evas_List *_elm_win_list = NULL;

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win *win = ecore_evas_data_get(ee, "__Elm");
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   win->deferred_resize_job = ecore_job_add(_elm_win_resize_job, win);
}

static void
_elm_win_obj_intercept_show(void *data, Evas_Object *obj)
{
   Elm_Win *win = data;
   ecore_evas_show(win->ee);
   evas_object_show(obj);
}

static void
_elm_win_obj_intercept_hide(void *data, Evas_Object *obj)
{
   Elm_Win *win = data;
   ecore_evas_hide(win->ee);
   evas_object_hide(obj);
}

static void
_elm_win_obj_intercept_move(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Elm_Win *win = data;
   // FIXME: account for frame
   ecore_evas_move(win->ee, x, y);
}

static void
_elm_win_obj_intercept_resize(void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Elm_Win *win = data;
   ecore_evas_resize(win->ee, w, h);
}

static void
_elm_win_obj_intercept_raise(void *data, Evas_Object *obj)
{
   Elm_Win *win = data;
   ecore_evas_raise(win->ee);
}

static void
_elm_win_obj_intercept_lower(void *data, Evas_Object *obj)
{
   Elm_Win *win = data;
   ecore_evas_lower(win->ee);
}

static void
_elm_win_obj_intercept_stack_above(void *data, Evas_Object *obj, Evas_Object *above)
{
   if (above)
     {
	Elm_Win *win = evas_object_data_get(above, "__Elm");
	if (!win) evas_object_raise(obj);
	// FIXME: find window id of win and stack abive
	return;
     }
   evas_object_raise(obj);
   return;
}

static void
_elm_win_obj_intercept_stack_below(void *data, Evas_Object *obj, Evas_Object *below)
{
   if (below)
     {
	Elm_Win *win = evas_object_data_get(below, "__Elm");
	if (!win) evas_object_raise(obj);
	// FIXME: find window id of win and stack below
	return;
     }
   evas_object_lower(obj);
   return;
}

static void
_elm_win_obj_intercept_layer_set(void *data, Evas_Object *obj, int l)
{
   Elm_Win *win = data;
   // FIXME: use netwm above/below hints
   ecore_evas_layer_set(win->ee, l);
}

static void
_elm_win_obj_intercept_color_set(void *data, Evas_Object *obj, int r, int g, int b, int a)
{
   return;
}

static void
_elm_win_obj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Elm_Win *win = data;

   _elm_win_list = evas_list_remove(_elm_win_list, win->win_obj);
   while (win->subobjs) elm_win_resize_object_del(obj, win->subobjs->data);
   evas_object_intercept_show_callback_del(win->win_obj, _elm_win_obj_intercept_show);
   evas_object_intercept_hide_callback_del(win->win_obj, _elm_win_obj_intercept_hide);
   evas_object_intercept_move_callback_del(win->win_obj, _elm_win_obj_intercept_move);
   evas_object_intercept_resize_callback_del(win->win_obj, _elm_win_obj_intercept_resize);
   evas_object_intercept_raise_callback_del(win->win_obj, _elm_win_obj_intercept_raise);
   evas_object_intercept_lower_callback_del(win->win_obj, _elm_win_obj_intercept_lower);
   evas_object_intercept_stack_above_callback_del(win->win_obj, _elm_win_obj_intercept_stack_above);
   evas_object_intercept_stack_below_callback_del(win->win_obj, _elm_win_obj_intercept_stack_below);
   evas_object_intercept_layer_set_callback_del(win->win_obj, _elm_win_obj_intercept_layer_set);
   evas_object_intercept_color_set_callback_del(win->win_obj, _elm_win_obj_intercept_color_set);
   evas_object_event_callback_del(win->win_obj, EVAS_CALLBACK_DEL, _elm_win_obj_callback_del);
   evas_object_event_callback_del(win->win_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _elm_win_obj_callback_changed_size_hints);
   ecore_evas_free(win->ee);
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   if (win->deferred_child_eval_job) ecore_job_del(win->deferred_child_eval_job);
   free(win);
}

static void
_elm_win_obj_callback_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Elm_Win *win = data;
   Evas_Coord w, h;
   
   evas_object_size_hint_min_get(obj, &w, &h);
   ecore_evas_size_min_set(win->ee, w, h);
   evas_object_size_hint_max_get(obj, &w, &h);
   if (w < 1) w = -1;
   if (h < 1) h = -1;
   ecore_evas_size_max_set(win->ee, w, h);
}

static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Elm_Win *win = ecore_evas_data_get(ee, "__Elm");
   evas_object_smart_callback_call(win->win_obj, "delete-request", NULL);
   if (win->autodel) evas_object_del(win->win_obj);
}

static void
_elm_win_resize_job(void *data)
{
   Elm_Win *win = data;
   Evas_List *l;
   int w, h;
   
   win->deferred_resize_job = NULL;
   ecore_evas_geometry_get(win->ee, NULL, NULL, &w, &h);
   evas_object_intercept_resize_callback_del(win->win_obj, _elm_win_obj_intercept_resize);
   evas_object_resize(win->win_obj, w, h);
   evas_object_intercept_resize_callback_add(win->win_obj, _elm_win_obj_intercept_resize, win);
   for (l = win->subobjs; l; l = l->next)
     {
	evas_object_move(l->data, 0, 0);
	evas_object_resize(l->data, w, h);
     }
}

static void
_elm_win_xwin_update(Elm_Win *win)
{
   win->xwin = 0;
   switch (_elm_config->engine)
     {
      case ELM_SOFTWARE_X11:
	if (win->ee) win->xwin = ecore_evas_software_x11_window_get(win->ee);
	break;
      case ELM_SOFTWARE_FB:
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
      default:
	break;
     }
   if (win->parent)
     {
	Elm_Win *win2;
	
	win2 = evas_object_data_get(win->parent, "__Elm");
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
	   default:
	     break;
	  }
     }

}

static void
_elm_win_eval_subobjs(Evas_Object *obj)
{
   Evas_List *l;
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   Evas_Coord w, h, minw = -1, minh = -1, maxw = -1, maxh = -1;
   int xx = 1, xy = 1;
   double wx, wy;

   for (l = win->subobjs; l; l = l->next)
     {
	evas_object_size_hint_weight_get(l->data, &wx, &wy);
	if (wx == 0.0) xx = 0;
	if (wy == 0.0) xy = 0;
	
	evas_object_size_hint_min_get(l->data, &w, &h);
	if (w < 1) w = -1;
	if (h < 1) h = -1;
	if (w > minw) minw = w;
	if (h > minh) minh = h;
	
	evas_object_size_hint_max_get(l->data, &w, &h);
	if (w < 1) w = -1;
	if (h < 1) h = -1;
	if (maxw == -1) maxw = w;
	else if (w < maxw) maxw = w;
	if (maxh == -1) maxh = h;
	else if (h < maxh) maxh = h;
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

EAPI Evas_Object *
elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type)
{
   Elm_Win *win;
   
   win = ELM_NEW(Elm_Win);
   switch (_elm_config->engine)
     {
      case ELM_SOFTWARE_X11:
	win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
	if (win->ee) win->xwin = ecore_evas_software_x11_window_get(win->ee);
	break;
      case ELM_SOFTWARE_FB:
	win->ee = ecore_evas_fb_new(NULL, 0, 1, 1);
        ecore_evas_fullscreen_set(win->ee, 1);
	break;
      case ELM_SOFTWARE_16_X11:
	win->ee = ecore_evas_software_x11_16_new(NULL, 0, 0, 0, 1, 1);
	if (win->ee) win->xwin = ecore_evas_software_x11_16_window_get(win->ee);
	break;
      case ELM_XRENDER_X11:
	win->ee = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, 1, 1);
	if (win->ee) win->xwin = ecore_evas_xrender_x11_window_get(win->ee);
	break;
      case ELM_OPENGL_X11:
	win->ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
	if (win->ee) win->xwin = ecore_evas_gl_x11_window_get(win->ee);
	break;
      default:
	break;
     }
   if (!win->ee)
     {
	printf("ELEMENTARY: Error. Cannot create window.\n");
	free(win);
	return NULL;
     }
   if (_elm_config->bgpixmap && !_elm_config->compositing)
     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
// bg pixmap done by x - has other issues like can be redrawn by x before it
// is filled/ready by app   
//     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);
   
   win->type = type;
   win->parent = parent;
   
   win->evas = ecore_evas_get(win->ee);
   win->win_obj = elm_widget_add(win->evas);
   evas_object_color_set(win->win_obj, 0, 0, 0, 0);
   evas_object_move(win->win_obj, 0, 0);
   evas_object_resize(win->win_obj, 1, 1);
   evas_object_layer_set(win->win_obj, 50);
   evas_object_pass_events_set(win->win_obj, 1);
   evas_object_data_set(win->win_obj, "__Elm", win);

   evas_object_intercept_show_callback_add(win->win_obj, _elm_win_obj_intercept_show, win);
   evas_object_intercept_hide_callback_add(win->win_obj, _elm_win_obj_intercept_hide, win);
   evas_object_intercept_move_callback_add(win->win_obj, _elm_win_obj_intercept_move, win);
   evas_object_intercept_resize_callback_add(win->win_obj, _elm_win_obj_intercept_resize, win);
   evas_object_intercept_raise_callback_add(win->win_obj, _elm_win_obj_intercept_raise, win);
   evas_object_intercept_lower_callback_add(win->win_obj, _elm_win_obj_intercept_lower, win);
   evas_object_intercept_stack_above_callback_add(win->win_obj, _elm_win_obj_intercept_stack_above, win);
   evas_object_intercept_stack_below_callback_add(win->win_obj, _elm_win_obj_intercept_stack_below, win);
   evas_object_intercept_layer_set_callback_add(win->win_obj, _elm_win_obj_intercept_layer_set, win);
   evas_object_intercept_color_set_callback_add(win->win_obj, _elm_win_obj_intercept_color_set, win);
   evas_object_event_callback_add(win->win_obj, EVAS_CALLBACK_DEL, _elm_win_obj_callback_del, win);
   evas_object_event_callback_add(win->win_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _elm_win_obj_callback_changed_size_hints, win);
   
   ecore_evas_name_class_set(win->ee, name, _elm_appname);
   ecore_evas_data_set(win->ee, "__Elm", win);
   ecore_evas_callback_delete_request_set(win->ee, _elm_win_delete_request);
   ecore_evas_callback_resize_set(win->ee, _elm_win_resize);
   // FIXME: use elm config for this
   evas_image_cache_set(win->evas, 4096 * 1024);
   evas_font_cache_set(win->evas, 512 * 1024);
   evas_font_path_append(win->evas, "fonts");
//   evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_NONE);
//   evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_AUTO);
//   evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_BYTECODE);
   edje_frametime_set(1.0 / 30.0);
   edje_scale_set(_elm_config->scale);

   _elm_win_xwin_update(win);

   _elm_win_list = evas_list_append(_elm_win_list, win->win_obj);
   
   return win->win_obj;
}

EAPI void
elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   win->subobjs = evas_list_append(win->subobjs, subobj);
   elm_widget_sub_object_add(obj, subobj);
   evas_object_event_callback_add(subobj, EVAS_CALLBACK_DEL, _elm_win_subobj_callback_del, obj);
   evas_object_event_callback_add(subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _elm_win_subobj_callback_changed_size_hints, obj);
   _elm_win_eval_subobjs(obj);
}

EAPI void
elm_win_resize_object_del(Evas_Object *obj, Evas_Object *subobj)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   evas_object_event_callback_del(subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _elm_win_subobj_callback_changed_size_hints);
   evas_object_event_callback_del(subobj, EVAS_CALLBACK_DEL, _elm_win_subobj_callback_del);
   win->subobjs = evas_list_remove(win->subobjs, subobj);
   elm_widget_sub_object_del(obj, subobj);
   _elm_win_eval_subobjs(obj);
}

EAPI void
elm_win_title_set(Evas_Object *obj, const char *title)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   ecore_evas_title_set(win->ee, title);
}
  
EAPI void
elm_win_autodel_set(Evas_Object *obj, Evas_Bool autodel)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   win->autodel = autodel;
}

EAPI void
elm_win_activate(Evas_Object *obj)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   ecore_evas_activate(win->ee);
}

EAPI void
elm_win_borderless_set(Evas_Object *obj, Evas_Bool borderless)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   ecore_evas_borderless_set(win->ee, borderless);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_shaped_set(Evas_Object *obj, Evas_Bool shaped)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   ecore_evas_shaped_set(win->ee, shaped);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_alpha_set(Evas_Object *obj, Evas_Bool alpha)
{
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
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
   Elm_Win *win = evas_object_data_get(obj, "__Elm");
   if (!win) return;
   ecore_evas_override_set(win->ee, override);
   _elm_win_xwin_update(win);
}
