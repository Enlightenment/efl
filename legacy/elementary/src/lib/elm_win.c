#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Win Win
 *
 */

typedef struct _Elm_Win Elm_Win;

struct _Elm_Win
{
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *parent, *win_obj;
   Eina_List *subobjs;
   Ecore_X_Window xwin;
   Ecore_Job *deferred_resize_job;
   Ecore_Job *deferred_child_eval_job;

   Elm_Win_Type type;
   Elm_Win_Keyboard_Mode kbdmode;
   Eina_Bool autodel : 1;
   int *autodel_clear, rot;
   struct {
      int x, y;
   } screen;
};

static const char *widtype = NULL;
static void _elm_win_obj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _elm_win_obj_intercept_show(void *data, Evas_Object *obj);
static void _elm_win_move(Ecore_Evas *ee);
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
_elm_win_move(Ecore_Evas *ee)
{
   Evas_Object *obj = ecore_evas_object_associate_get(ee);
   Elm_Win *win;
   int x, y;
   
   if (!obj) return;
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   win->screen.x = x;
   win->screen.y = y;
   evas_object_smart_callback_call(win->win_obj, "moved", NULL);
}

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Evas_Object *obj = ecore_evas_object_associate_get(ee);
   Elm_Win *win;
   
   if (!obj) return;
   win = elm_widget_data_get(obj);
   if (!win) return;
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   win->deferred_resize_job = ecore_job_add(_elm_win_resize_job, win);
}

static void
_elm_win_focus_in(Ecore_Evas *ee)
{
   Evas_Object *obj = ecore_evas_object_associate_get(ee);
   Elm_Win *win;
   
   if (!obj) return;
   win = elm_widget_data_get(obj);
   if (!win) return;
   /*NB: Why two different "focus signals" here ??? */
   evas_object_smart_callback_call(win->win_obj, "focus-in", NULL); // FIXME: remove me
   evas_object_smart_callback_call(win->win_obj, "focus,in", NULL);
}

static void
_elm_win_focus_out(Ecore_Evas *ee)
{
   Evas_Object *obj = ecore_evas_object_associate_get(ee);
   Elm_Win *win;
   
   if (!obj) return;
   win = elm_widget_data_get(obj);
   if (!win) return;
   evas_object_smart_callback_call(win->win_obj, "focus-out", NULL); // FIXME: remove me
   evas_object_smart_callback_call(win->win_obj, "focus,out", NULL);
}

static void
_deferred_ecore_evas_free(void *data)
{
   ecore_evas_free(data);
}

static void
_elm_win_obj_callback_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Win *win = data;
   Evas_Object *child;

   if (win->autodel_clear) *(win->autodel_clear) = -1;
   _elm_win_list = eina_list_remove(_elm_win_list, win->win_obj);
   while (win->subobjs) elm_win_resize_object_del(obj, win->subobjs->data);
   ecore_evas_callback_delete_request_set(win->ee, NULL);
   ecore_evas_callback_resize_set(win->ee, NULL);
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   if (win->deferred_child_eval_job) ecore_job_del(win->deferred_child_eval_job);
   while (((child = evas_object_bottom_get(win->evas)) != NULL) &&
	  (child != obj))
     {
	evas_object_del(child);
     }
   while (((child = evas_object_top_get(win->evas)) != NULL) &&
	  (child != obj))
     {
	evas_object_del(child);
     }
   evas_image_cache_flush(win->evas);
   evas_font_cache_flush(win->evas);
// FIXME: Why are we flushing edje on every window destroy ??
   edje_file_cache_flush();
   edje_collection_cache_flush();
// FIXME: we are in the del handler for the object and delete the canvas
// that lives under it from the handler... nasty. deferring doesnt help either
   ecore_job_add(_deferred_ecore_evas_free, win->ee);
//   ecore_evas_free(win->ee);
   free(win);

   if ((!_elm_win_list) &&
       (elm_policy_get(ELM_POLICY_QUIT) == ELM_POLICY_QUIT_LAST_WINDOW_CLOSED))
     {
	elm_exit();
     }
}

static void
_elm_win_obj_intercept_show(void *data __UNUSED__, Evas_Object *obj)
{
   // this is called to make sure all smart containers have calculated their
   // sizes BEFORE we show the window to make sure it initially appears at
   // our desired size (ie min size is known first)
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   evas_object_show(obj);
}

static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Evas_Object *obj = ecore_evas_object_associate_get(ee);
   Elm_Win *win;
   if (strcmp(elm_widget_type_get(obj), "win")) return;
   
   win = elm_widget_data_get(obj);
   if (!win) return;
   int autodel = win->autodel;
   win->autodel_clear = &autodel;
   evas_object_smart_callback_call(win->win_obj, "delete-request", NULL); // FIXME: remove me
   evas_object_smart_callback_call(win->win_obj, "delete,request", NULL);
   // FIXME: if above callback deletes - then the below will be invalid
   if (autodel) evas_object_del(win->win_obj);
   else win->autodel_clear = NULL;
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
      case ELM_SOFTWARE_DIRECTFB:
      case ELM_SOFTWARE_16_WINCE:
      case ELM_SOFTWARE_SDL:
      case ELM_SOFTWARE_16_SDL:
      case ELM_OPENGL_SDL:
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

   if (!win->xwin) return; /* nothing more to do */

   switch (win->type)
     {
      case ELM_WIN_BASIC:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_NORMAL);
	 break;
      case ELM_WIN_DIALOG_BASIC:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DIALOG);
	 break;
      case ELM_WIN_DESKTOP:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DESKTOP);
	 break;
      case ELM_WIN_DOCK:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DOCK);
	 break;
      case ELM_WIN_TOOLBAR:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_TOOLBAR);
	 break;
      case ELM_WIN_MENU:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_MENU);
	 break;
      case ELM_WIN_UTILITY:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_UTILITY);
	 break;
      case ELM_WIN_SPLASH:
	 ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_SPLASH);
	 break;
      default:
	 break;
     }
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
_elm_win_subobj_callback_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_win_resize_object_del(data, obj);
}

static void
_elm_win_subobj_callback_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _elm_win_eval_subobjs(data);
}

void
_elm_win_shutdown(void)
{
   while (_elm_win_list)
     evas_object_del(_elm_win_list->data);
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
      case ELM_SOFTWARE_DIRECTFB:
        win->ee = ecore_evas_directfb_new(NULL, 1, 0, 0, 1, 1);
	break;
      case ELM_SOFTWARE_16_X11:
	win->ee = ecore_evas_software_x11_16_new(NULL, 0, 0, 0, 1, 1);
        if (!win->ee)
          {
             CRITICAL("Software-16 engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
	break;
      case ELM_XRENDER_X11:
	win->ee = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, 1, 1);
        if (!win->ee)
          {
             CRITICAL("XRender engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
	break;
      case ELM_OPENGL_X11:
	win->ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
        if (!win->ee)
          {
             CRITICAL("OpenGL engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
	break;
      case ELM_SOFTWARE_WIN32:
	win->ee = ecore_evas_software_gdi_new(NULL, 0, 0, 1, 1);
	break;
      case ELM_SOFTWARE_16_WINCE:
	win->ee = ecore_evas_software_wince_gdi_new(NULL, 0, 0, 1, 1);
	break;
      case ELM_SOFTWARE_SDL:
	win->ee = ecore_evas_sdl_new(NULL, 0, 0, 0, 0, 0, 1);
        if (!win->ee)
          {
             CRITICAL("Software SDL engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
	break;
      case ELM_SOFTWARE_16_SDL:
	win->ee = ecore_evas_sdl16_new(NULL, 0, 0, 0, 0, 0, 1);
        if (!win->ee)
          {
             CRITICAL("Sofware-16-SDL engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
	break;
      case ELM_OPENGL_SDL:
	win->ee = ecore_evas_gl_sdl_new(NULL, 1, 1, 0, 0);
        if (!win->ee)
          {
             CRITICAL("OpenGL SDL engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
	break;
      default:
	break;
     }
   if (!win->ee)
     {
	ERR("Cannot create window.");
	free(win);
	return NULL;
     }
   _elm_win_xwindow_get(win);
   if ((_elm_config->bgpixmap) && (!_elm_config->compositing))
     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
// bg pixmap done by x - has other issues like can be redrawn by x before it
// is filled/ready by app
//     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);

   win->type = type;
   win->parent = parent;

   win->evas = ecore_evas_get(win->ee);
   win->win_obj = elm_widget_add(win->evas);
   elm_widget_type_set(win->win_obj, "win");
   ELM_SET_WIDTYPE(widtype, "win");
   elm_widget_data_set(win->win_obj, win);
   evas_object_color_set(win->win_obj, 0, 0, 0, 0);
   evas_object_move(win->win_obj, 0, 0);
   evas_object_resize(win->win_obj, 1, 1);
   evas_object_layer_set(win->win_obj, 50);
   evas_object_pass_events_set(win->win_obj, 1);

   evas_object_intercept_show_callback_add(win->win_obj,
                                           _elm_win_obj_intercept_show, win);
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
   ecore_evas_callback_move_set(win->ee, _elm_win_move);
   evas_image_cache_set(win->evas, (_elm_config->image_cache * 1024));
   evas_font_cache_set(win->evas, (_elm_config->font_cache * 1024));
   EINA_LIST_FOREACH(_elm_config->font_dirs, l, fontpath)
     evas_font_path_append(win->evas, fontpath);
   if (_elm_config->font_hinting == 0)
     evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_NONE);
   else if (_elm_config->font_hinting == 1)
     evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_AUTO);
   else if (_elm_config->font_hinting == 2)
     evas_font_hinting_set(win->evas, EVAS_FONT_HINTING_BYTECODE);

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
      case ELM_SOFTWARE_SDL:
      case ELM_SOFTWARE_16_SDL:
      case ELM_OPENGL_SDL:
      default:
	break;
     }
   return win->win_obj;
}

EAPI void
elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj)
{
   Evas_Coord w, h;
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   win->subobjs = eina_list_append(win->subobjs, subobj);
   elm_widget_sub_object_add(obj, subobj);
   evas_object_event_callback_add(subobj, EVAS_CALLBACK_DEL,
                                  _elm_win_subobj_callback_del, obj);
   evas_object_event_callback_add(subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _elm_win_subobj_callback_changed_size_hints,
                                  obj);
   ecore_evas_geometry_get(win->ee, NULL, NULL, &w, &h);
   evas_object_move(subobj, 0, 0);
   evas_object_resize(subobj, w, h);
   _elm_win_eval_subobjs(obj);
}

EAPI void
elm_win_resize_object_del(Evas_Object *obj, Evas_Object *subobj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   evas_object_event_callback_del_full(subobj,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _elm_win_subobj_callback_changed_size_hints,
                                       obj);
   evas_object_event_callback_del_full(subobj, EVAS_CALLBACK_DEL,
                                       _elm_win_subobj_callback_del, obj);
   win->subobjs = eina_list_remove(win->subobjs, subobj);
   elm_widget_sub_object_del(obj, subobj);
   _elm_win_eval_subobjs(obj);
}

EAPI void
elm_win_title_set(Evas_Object *obj, const char *title)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_title_set(win->ee, title);
}

EAPI void
elm_win_autodel_set(Evas_Object *obj, Eina_Bool autodel)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   win->autodel = autodel;
}

EAPI void
elm_win_activate(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_activate(win->ee);
}

EAPI void
elm_win_lower(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_lower(win->ee);
}

EAPI void
elm_win_raise(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_raise(win->ee);
}

EAPI void
elm_win_borderless_set(Evas_Object *obj, Eina_Bool borderless)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_borderless_set(win->ee, borderless);
   _elm_win_xwin_update(win);
}

EAPI Eina_Bool
elm_win_borderless_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_borderless_get(win->ee);
}

EAPI void
elm_win_shaped_set(Evas_Object *obj, Eina_Bool shaped)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_shaped_set(win->ee, shaped);
   _elm_win_xwin_update(win);
}

EAPI Eina_Bool
elm_win_shaped_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_shaped_get(win->ee);
}

EAPI void
elm_win_alpha_set(Evas_Object *obj, Eina_Bool alpha)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
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

EAPI Eina_Bool
elm_win_alpha_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_alpha_get(win->ee);
}

EAPI void
elm_win_transparent_set(Evas_Object *obj, Eina_Bool transparent)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   
   if (win->xwin)
     {
       ecore_evas_transparent_set(win->ee, transparent);	  
	_elm_win_xwin_update(win);
     }
   else
     ecore_evas_transparent_set(win->ee, transparent);
}

EAPI Eina_Bool
elm_win_transparent_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;

   return ecore_evas_transparent_get(win->ee);
}

EAPI void
elm_win_override_set(Evas_Object *obj, Eina_Bool override)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_override_set(win->ee, override);
   _elm_win_xwin_update(win);
}

EAPI Eina_Bool
elm_win_override_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_override_get(win->ee);
}

EAPI void
elm_win_fullscreen_set(Evas_Object *obj, Eina_Bool fullscreen)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
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

EAPI Eina_Bool
elm_win_fullscreen_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   switch (_elm_config->engine)
     {
     case ELM_SOFTWARE_16_WINCE:
     case ELM_SOFTWARE_FB:
	// these engines... can ONLY be fullscreen
	return EINA_TRUE;
	break;
     default:
	return ecore_evas_fullscreen_get(win->ee);
	break;
     }
   return EINA_FALSE;
}

EAPI void
elm_win_maximized_set(Evas_Object *obj, Eina_Bool maximized)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_maximized_set(win->ee, maximized);
   _elm_win_xwin_update(win);
}

EAPI Eina_Bool
elm_win_maximized_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_maximized_get(win->ee);
}

EAPI void
elm_win_iconified_set(Evas_Object *obj, Eina_Bool iconified)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_iconified_set(win->ee, iconified);
   _elm_win_xwin_update(win);
}

EAPI Eina_Bool
elm_win_iconified_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_iconified_get(win->ee);
}

EAPI void
elm_win_layer_set(Evas_Object *obj, int layer)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_layer_set(win->ee, layer);
   _elm_win_xwin_update(win);
}

EAPI int
elm_win_layer_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
   return ecore_evas_layer_get(win->ee);
}

EAPI void
elm_win_rotation_set(Evas_Object *obj, int rotation)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   if (win->rot == rotation) return;
   win->rot = rotation;
   ecore_evas_rotation_set(win->ee, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_eval_subobjs(obj);
   _elm_win_xwin_update(win);
}

EAPI void
elm_win_rotation_with_resize_set(Evas_Object *obj, int rotation)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   if (win->rot == rotation) return;
   win->rot = rotation;
   ecore_evas_rotation_with_resize_set(win->ee, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_eval_subobjs(obj);
   _elm_win_xwin_update(win);
}

EAPI int
elm_win_rotation_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
   return win->rot;
}

EAPI void
elm_win_sticky_set(Evas_Object *obj, Eina_Bool sticky)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_sticky_set(win->ee, sticky);
   _elm_win_xwin_update(win);
}

EAPI Eina_Bool
elm_win_sticky_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_sticky_get(win->ee);
}

EAPI void
elm_win_keyboard_mode_set(Evas_Object *obj, Elm_Win_Keyboard_Mode mode)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
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
elm_win_keyboard_win_set(Evas_Object *obj, Eina_Bool is_keyboard)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     ecore_x_e_virtual_keyboard_set(win->xwin, is_keyboard);
#endif
}

EAPI void
elm_win_screen_position_get(Evas_Object *obj, int *x, int *y)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   if (x) *x = win->screen.x;
   if (y) *y = win->screen.y;
}

/**
 * Set if this window is an illume conformant window
 *
 * @param obj The window object
 * @param conformant The conformant flag (1 = conformant, 0 = non-conformant)
 *
 * @ingroup Win
 */
EAPI void
elm_win_conformant_set(Evas_Object *obj, Eina_Bool conformant)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     ecore_x_e_illume_conformant_set(win->xwin, conformant);
#endif
}

/**
 * Get if this window is an illume conformant window
 *
 * @param obj The window object
 * @return A boolean if this window is illume conformant or not
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_conformant_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     return ecore_x_e_illume_conformant_get(win->xwin);
#endif
   return EINA_FALSE;
}

/**
 * Set a window to be an illume quickpanel window
 *
 * By default window objects are not quickpanel windows.
 *
 * @param obj The window object
 * @param quickpanel The quickpanel flag (1 = quickpanel, 0 = normal window)
 *
 * @ingroup Win
 */
EAPI void
elm_win_quickpanel_set(Evas_Object *obj, Eina_Bool quickpanel)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     {
        ecore_x_e_illume_quickpanel_set(win->xwin, quickpanel);
        if (quickpanel)
          {
             Ecore_X_Window_State states[2];

             states[0] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
             states[1] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
             ecore_x_netwm_window_state_set(win->xwin, states, 2);
             ecore_x_icccm_hints_set(win->xwin, 0, 0, 0, 0, 0, 0, 0);
          }
     }
#endif
}

/**
 * Get if this window is a quickpanel or not
 *
 * @param obj The window object
 * @return A boolean if this window is a quickpanel or not
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_quickpanel_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     return ecore_x_e_illume_quickpanel_get(win->xwin);
#endif
   return EINA_FALSE;
}

/**
 * Set the major priority of a quickpanel window
 *
 * @param obj The window object
 * @param priority The major priority for this quickpanel
 *
 * @ingroup Win
 */
EAPI void
elm_win_quickpanel_priority_major_set(Evas_Object *obj, int priority)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     ecore_x_e_illume_quickpanel_priority_major_set(win->xwin, priority);
#endif
}

/**
 * Get the major priority of a quickpanel window
 *
 * @param obj The window object
 * @return The major priority of this quickpanel
 *
 * @ingroup Win
 */
EAPI int
elm_win_quickpanel_priority_major_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     return ecore_x_e_illume_quickpanel_priority_major_get(win->xwin);
#endif
   return -1;
}

/**
 * Set the minor priority of a quickpanel window
 *
 * @param obj The window object
 * @param priority The minor priority for this quickpanel
 *
 * @ingroup Win
 */
EAPI void
elm_win_quickpanel_priority_minor_set(Evas_Object *obj, int priority)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     ecore_x_e_illume_quickpanel_priority_minor_set(win->xwin, priority);
#endif
}

/**
 * Get the minor priority of a quickpanel window
 *
 * @param obj The window object
 * @return The minor priority of this quickpanel
 *
 * @ingroup Win
 */
EAPI int
elm_win_quickpanel_priority_minor_get(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     return ecore_x_e_illume_quickpanel_priority_minor_get(win->xwin);
#endif
   return -1;
}

/**
 * Set which zone this quickpanel should appear in
 *
 * @param obj The window object
 * @param zone The requested zone for this quickpanel
 *
 * @ingroup Win
 */
EAPI void
elm_win_quickpanel_zone_set(Evas_Object *obj, int zone)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   _elm_win_xwindow_get(win);
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     ecore_x_e_illume_quickpanel_zone_set(win->xwin, zone);
#endif
}

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *content;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static const char *widtype2 = NULL;

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_set(wd->frm, "win", "inwin", elm_widget_style_get(obj));
   if (wd->content)
     edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   evas_object_size_hint_min_get(wd->content, &minw, &minh);
   edje_object_size_min_calc(wd->frm, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->content)
     {
	evas_object_event_callback_del_full
          (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
	wd->content = NULL;
	_sizing_eval(obj);
     }
}

EAPI Evas_Object *
elm_win_inwin_add(Evas_Object *obj)
{
   Evas_Object *obj2;
   Widget_Data *wd;
   Elm_Win *win;

   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   win = elm_widget_data_get(obj);
   if (!win) return NULL;
   wd = ELM_NEW(Widget_Data);
   obj2 = elm_widget_add(win->evas);
   elm_widget_type_set(obj2, "inwin");
   ELM_SET_WIDTYPE(widtype2, "inwin");
   elm_widget_sub_object_add(obj, obj2);
   evas_object_size_hint_weight_set(obj2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj2, EVAS_HINT_FILL, EVAS_HINT_FILL);
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

/** DEPRECATED. use elm_object_style_set() instead */
EAPI void
elm_win_inwin_style_set(Evas_Object *obj, const char *style)
{
   elm_widget_style_set(obj, style);
}

EAPI void
elm_win_inwin_activate(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype2);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_raise(obj);
   evas_object_show(obj);
   edje_object_signal_emit(wd->frm, "elm,action,show", "elm");
   elm_widget_focused_object_clear(elm_widget_parent_get(obj));
}

EAPI void
elm_win_inwin_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype2);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->content) && (wd->content != content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
	_sizing_eval(obj);
     }
}

/* windowing spcific calls - shall we do this differently? */
EAPI Ecore_X_Window
elm_win_xwindow_get(const Evas_Object *obj)
{
   Ecore_X_Window xwin = 0;
   Ecore_Evas *ee = NULL;
   
   if (!obj) return 0;
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   if (ee) xwin = (Ecore_X_Window)ecore_evas_window_get(ee);
   return xwin;
}
