#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Win Win
 *
 * The window class of Elementary.  Contains functions to manipulate
 * windows.
 */

typedef struct _Elm_Win Elm_Win;

struct _Elm_Win
{
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *parent, *win_obj;
   Eina_List *subobjs;
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin;
   Ecore_Event_Handler *client_message_handler;
#endif   
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
static void _elm_win_obj_callback_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
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

Eina_List *_elm_win_list = NULL;

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

   if (win->parent)
     {
        evas_object_event_callback_del_full(win->parent, EVAS_CALLBACK_DEL,
                                            _elm_win_obj_callback_parent_del, win);
        win->parent = NULL;
     }
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
#ifdef HAVE_ELEMENTARY_X
   if (win->client_message_handler)
     ecore_event_handler_del(win->client_message_handler);
#endif   
// FIXME: Why are we flushing edje on every window destroy ??
//   evas_image_cache_flush(win->evas);
//   evas_font_cache_flush(win->evas);
//   edje_file_cache_flush();
//   edje_collection_cache_flush();
// FIXME: we are in the del handler for the object and delete the canvas
// that lives under it from the handler... nasty. deferring doesnt help either
   ecore_job_add(_deferred_ecore_evas_free, win->ee);
//   ecore_evas_free(win->ee);
   free(win);

   if ((!_elm_win_list) &&
       (elm_policy_get(ELM_POLICY_QUIT) == ELM_POLICY_QUIT_LAST_WINDOW_CLOSED))
     {
        evas_image_cache_flush(e);
        evas_font_cache_flush(e);
        edje_file_cache_flush();
        edje_collection_cache_flush();
	elm_exit();
     }
}

static void
_elm_win_obj_callback_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Win *win = data;
   if (obj == win->parent) win->parent = NULL;
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

#ifdef HAVE_ELEMENTARY_X
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
#endif

#ifdef HAVE_ELEMENTARY_X
static void
_elm_win_xwin_update(Elm_Win *win)
{
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
}
#endif

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

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool
_elm_win_client_message(void *data, int type __UNUSED__, void *event)
{
   Elm_Win *win = data;
   Ecore_X_Event_Client_Message *e = event;

   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_FLUSH)
     {
        if (e->data.l[0] == win->xwin)
          {
             Evas *evas = evas_object_evas_get(win->win_obj);
             if (evas)
               {
                  edje_file_cache_flush();
                  edje_collection_cache_flush();
                  evas_image_cache_flush(evas);
                  evas_font_cache_flush(evas);
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_DUMP)
     {
        if (e->data.l[0] == win->xwin)
          {
             Evas *evas = evas_object_evas_get(win->win_obj);
             if (evas)
               {
                  edje_file_cache_flush();
                  edje_collection_cache_flush();
                  evas_image_cache_flush(evas);
                  evas_font_cache_flush(evas);
                  evas_render_dump(evas);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}
#endif

/**
 * Adds a window object. If this is the first window created, pass NULL as
 * @p parent.
 *
 * @param parent Parent object to add the window to, or NULL
 * @param name The name of the window
 * @param type The window type, one of the following:
 * ELM_WIN_BASIC
 * ELM_WIN_DIALOG_BASIC
 * ELM_WIN_DESKTOP
 * ELM_WIN_DOCK
 * ELM_WIN_TOOLBAR
 * ELM_WIN_MENU
 * ELM_WIN_UTILITY
 * ELM_WIN_SPLASH
 *
 * @return The created object, or NULL on failure
 *
 * @ingroup Win
 */
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
#ifdef HAVE_ELEMENTARY_X
        win->client_message_handler = ecore_event_handler_add
          (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, win);
#endif        
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
#ifdef HAVE_ELEMENTARY_X
        win->client_message_handler = ecore_event_handler_add
          (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, win);
#endif        
	break;
      case ELM_XRENDER_X11:
	win->ee = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, 1, 1);
        if (!win->ee)
          {
             CRITICAL("XRender engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
#ifdef HAVE_ELEMENTARY_X
        win->client_message_handler = ecore_event_handler_add
          (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, win);
#endif        
	break;
      case ELM_OPENGL_X11:
	win->ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
        if (!win->ee)
          {
             CRITICAL("OpenGL engine create failed. Try software.");
             win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
          }
#ifdef HAVE_ELEMENTARY_X
        win->client_message_handler = ecore_event_handler_add
          (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, win);
#endif        
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
#endif   
   if ((_elm_config->bgpixmap) && (!_elm_config->compositing))
     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
// bg pixmap done by x - has other issues like can be redrawn by x before it
// is filled/ready by app
//     ecore_evas_avoid_damage_set(win->ee, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);

   win->type = type;
   win->parent = parent;
   if (win->parent)
     evas_object_event_callback_add(win->parent, EVAS_CALLBACK_DEL,
                                    _elm_win_obj_callback_parent_del, win);

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

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
   
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

/**
 * Add @p subobj as a resize object of window @p obj.
 *
 * @param obj The window object
 * @param subobj The resize object to add
 *
 * @ingroup Win
 */
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

/**
 * Delete @p subobj as a resize object of window @p obj.
 *
 * @param obj The window object
 * @param subobj The resize object to add
 *
 * @ingroup Win
 */
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

/**
 * Set the title of the window
 *
 * @param obj The window object
 * @param title The title to set
 *
 * @ingroup Win
 */
EAPI void
elm_win_title_set(Evas_Object *obj, const char *title)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_title_set(win->ee, title);
}

/**
 * Set the window's autodel state.
 *
 * @param obj The window object
 * @param autodel If true, the window will automatically delete itself when closed
 *
 * @ingroup Win
 */
EAPI void
elm_win_autodel_set(Evas_Object *obj, Eina_Bool autodel)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   win->autodel = autodel;
}

/**
 * Activate a window object.
 *
 * @param obj The window object
 *
 * @ingroup Win
 */
EAPI void
elm_win_activate(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_activate(win->ee);
}

/**
 * Lower a window object.
 *
 * @param obj The window object
 *
 * @ingroup Win
 */
EAPI void
elm_win_lower(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_lower(win->ee);
}

/**
 * Raise a window object.
 *
 * @param obj The window object
 *
 * @ingroup Win
 */
EAPI void
elm_win_raise(Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_raise(win->ee);
}

/**
 * Set the borderless state of a window.
 *
 * @param obj The window object
 * @param borderless If true, the window is borderless
 *
 * @ingroup Win
 */
EAPI void
elm_win_borderless_set(Evas_Object *obj, Eina_Bool borderless)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_borderless_set(win->ee, borderless);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the borderless state of a window.
 *
 * @param obj The window object
 * @return If true, the window is borderless
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_borderless_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_borderless_get(win->ee);
}

/**
 * Set the shaped state of a window.
 *
 * @param obj The window object
 * @param shaped If true, the window is shaped
 *
 * @ingroup Win
 */
EAPI void
elm_win_shaped_set(Evas_Object *obj, Eina_Bool shaped)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_shaped_set(win->ee, shaped);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the shaped state of a window.
 *
 * @param obj The window object
 * @return If true, the window is shaped
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_shaped_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_shaped_get(win->ee);
}

/**
 * Set the alpha channel state of a window.
 *
 * @param obj The window object
 * @param alpha If true, the window has an alpha channel
 *
 * @ingroup Win
 */
EAPI void
elm_win_alpha_set(Evas_Object *obj, Eina_Bool alpha)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
#ifdef HAVE_ELEMENTARY_X
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
#endif
     ecore_evas_alpha_set(win->ee, alpha);
}

/**
 * Get the alpha channel state of a window.
 *
 * @param obj The window object
 * @return If true, the window has an alpha channel
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_alpha_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_alpha_get(win->ee);
}

/**
 * Set the transparency state of a window.
 *
 * @param obj The window object
 * @param transparent If true, the window is transparent
 *
 * @ingroup Win
 */
EAPI void
elm_win_transparent_set(Evas_Object *obj, Eina_Bool transparent)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     {
       ecore_evas_transparent_set(win->ee, transparent);	  
	_elm_win_xwin_update(win);
     }
   else
#endif
     ecore_evas_transparent_set(win->ee, transparent);
}

/**
 * Get the transparency state of a window.
 *
 * @param obj The window object
 * @return If true, the window is transparent
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_transparent_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;

   return ecore_evas_transparent_get(win->ee);
}

/**
 * Set the override state of a window.
 *
 * @param obj The window object
 * @param override If true, the window is overridden
 *
 * @ingroup Win
 */
EAPI void
elm_win_override_set(Evas_Object *obj, Eina_Bool override)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_override_set(win->ee, override);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the override state of a window.
 *
 * @param obj The window object
 * @return If true, the window is overridden
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_override_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_override_get(win->ee);
}

/**
 * Set the fullscreen state of a window.
 *
 * @param obj The window object
 * @param fullscreen If true, the window is fullscreen
 *
 * @ingroup Win
 */
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
#ifdef HAVE_ELEMENTARY_X
	_elm_win_xwin_update(win);
#endif
	break;
     }
}

/**
 * Get the fullscreen state of a window.
 *
 * @param obj The window object
 * @return If true, the window is fullscreen
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_fullscreen_get(const Evas_Object *obj)
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

/**
 * Set the maximized state of a window.
 *
 * @param obj The window object
 * @param maximized If true, the window is maximized
 *
 * @ingroup Win
 */
EAPI void
elm_win_maximized_set(Evas_Object *obj, Eina_Bool maximized)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_maximized_set(win->ee, maximized);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the maximized state of a window.
 *
 * @param obj The window object
 * @return If true, the window is maximized
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_maximized_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_maximized_get(win->ee);
}

/**
 * Set the iconified state of a window.
 *
 * @param obj The window object
 * @param iconified If true, the window is iconified
 *
 * @ingroup Win
 */
EAPI void
elm_win_iconified_set(Evas_Object *obj, Eina_Bool iconified)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_iconified_set(win->ee, iconified);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the iconified state of a window.
 *
 * @param obj The window object
 * @return If true, the window is iconified
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_iconified_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_iconified_get(win->ee);
}

/**
 * Set the layer of the window.
 *
 * @param obj The window object
 * @param layer The layer of the window
 *
 * @ingroup Win
 */
EAPI void
elm_win_layer_set(Evas_Object *obj, int layer)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_layer_set(win->ee, layer); 
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the layer of the window.
 *
 * @param obj The window object
 * @return The layer of the window
 *
 * @ingroup Win
 */
EAPI int
elm_win_layer_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
   return ecore_evas_layer_get(win->ee);
}

/**
 * Set the rotation of the window.
 *
 * @param obj The window object
 * @param rotation The rotation of the window, in degrees (0-360)
 *
 * @ingroup Win
 */
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Rotates the window and resizes it
 *
 * @param obj The window object
 * @param layer The rotation of the window in degrees (0-360)
 *
 * @ingroup Win
 */
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the rotation of the window.
 *
 * @param obj The window object
 * @return The rotation of the window in degrees (0-360)
 *
 * @ingroup Win
 */
EAPI int
elm_win_rotation_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
   return win->rot;
}

/**
 * Set the sticky state of the window.
 *
 * @param obj The window object
 * @param sticky If true, the window's sticky state is enabled
 *
 * @ingroup Win
 */
EAPI void
elm_win_sticky_set(Evas_Object *obj, Eina_Bool sticky)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   ecore_evas_sticky_set(win->ee, sticky);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(win);
#endif
}

/**
 * Get the sticky state of the window.
 *
 * @param obj The window object
 * @return If true, the window's sticky state is enabled
 *
 * @ingroup Win
 */
EAPI Eina_Bool
elm_win_sticky_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
   return ecore_evas_sticky_get(win->ee);
}

/**
 * Sets the keyboard mode of the window.
 *
 * @param obj The window object
 * @param mode The mode to set; one of:
 * ELM_WIN_KEYBOARD_UNKNOWN
 * ELM_WIN_KEYBOARD_OFF
 * ELM_WIN_KEYBOARD_ON
 * ELM_WIN_KEYBOARD_ALPHA
 * ELM_WIN_KEYBOARD_NUMERIC
 * ELM_WIN_KEYBOARD_PIN
 * ELM_WIN_KEYBOARD_PHONE_NUMBER
 * ELM_WIN_KEYBOARD_HEX
 * ELM_WIN_KEYBOARD_TERMINAL
 * ELM_WIN_KEYBOARD_PASSWORD
 * ELM_WIN_KEYBOARD_IP
 * ELM_WIN_KEYBOARD_HOST
 * ELM_WIN_KEYBOARD_FILE
 * ELM_WIN_KEYBOARD_URL
 * ELM_WIN_KEYBOARD_KEYPAD
 * ELM_WIN_KEYBOARD_J2ME
 *
 * @ingroup Win
 */
EAPI void
elm_win_keyboard_mode_set(Evas_Object *obj, Elm_Win_Keyboard_Mode mode)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
   if (mode == win->kbdmode) return;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
#endif
   win->kbdmode = mode;
#ifdef HAVE_ELEMENTARY_X
   if (win->xwin)
     ecore_x_e_virtual_keyboard_state_set
     (win->xwin, (Ecore_X_Virtual_Keyboard_State)win->kbdmode);
#endif
}

/**
 * Sets whether the window is a keyboard.
 *
 * @param obj The window object
 * @param is_keyboard If true, the window is a virtual keyboard
 *
 * @ingroup Win
 */
EAPI void
elm_win_keyboard_win_set(Evas_Object *obj, Eina_Bool is_keyboard)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype);
   win = elm_widget_data_get(obj);
   if (!win) return;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
   if (win->xwin)
     ecore_x_e_virtual_keyboard_set(win->xwin, is_keyboard);
#endif
}

/**
 * Get the screen position of a window.
 *
 * @param obj The window object
 * @param x The int to store the x coordinate to
 * @param y The int to store the y coordinate to
 *
 * @ingroup Win
 */
EAPI void
elm_win_screen_position_get(const Evas_Object *obj, int *x, int *y)
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
elm_win_conformant_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
elm_win_quickpanel_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   win = elm_widget_data_get(obj);
   if (!win) return EINA_FALSE;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
elm_win_quickpanel_priority_major_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
elm_win_quickpanel_priority_minor_get(const Evas_Object *obj)
{
   Elm_Win *win;
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   win = elm_widget_data_get(obj);
   if (!win) return -1;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(win);
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
   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_object_set(obj, wd->frm, "win", "inwin", elm_widget_style_get(obj));
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

/**
 * @defgroup Inwin Inwin
 *
 * An inwin is a window inside a window that is useful for a quick popup.  It does not hover.
 */
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
   _elm_theme_object_set(obj, wd->frm, "win", "inwin", "default");
   elm_widget_resize_object_set(obj2, wd->frm);

   evas_object_smart_callback_add(obj2, "sub-object-del", _sub_del, obj2);

   _sizing_eval(obj2);
   return obj2;
}

/**
 * Activates an inwin object
 *
 * @param obj The inwin to activate
 *
 * @ingroup Inwin
 */
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

/**
 * Set the content of an inwin object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_win_inwin_content_unset() function.
 *
 * @param obj The inwin object
 * @param content The object to set as content
 *
 * @ingroup Inwin
 */
EAPI void
elm_win_inwin_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype2);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
     }
   _sizing_eval(obj);
}

/**
 * Unset the content of an inwin object.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @param obj The inwin object
 * @return The content that was being used
 *
 * @ingroup Inwin
 */
EAPI Evas_Object *
elm_win_inwin_content_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype2) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   Evas_Object *content = wd->content;
   elm_widget_sub_object_del(obj, wd->content);
   edje_object_part_unswallow(wd->frm, wd->content);
   wd->content = NULL;
   return content;
}

/* windowing spcific calls - shall we do this differently? */
/**
 * Get the Ecore_X_Window of an Evas_Object
 *
 * @param obj The object
 *
 * @return The Ecore_X_Window of @p obj
 *
 * @ingroup Win
 */
EAPI Ecore_X_Window
elm_win_xwindow_get(const Evas_Object *obj)
{
   Ecore_X_Window xwin = 0;
   Ecore_Evas *ee = NULL;
   if (!obj) return 0;
#ifdef HAVE_ELEMENTARY_X
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   if (ee) xwin = (Ecore_X_Window)ecore_evas_window_get(ee);
   return xwin;
#else
   return 0;
#endif
}
