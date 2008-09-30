#include <Elementary.h>
#include "elm_priv.h"

static void _elm_win_name_set(Elm_Win *win, const char *name);
static void _elm_win_title_set(Elm_Win *win, const char *title);
static void _elm_win_show(Elm_Win *win);
static void _elm_win_hide(Elm_Win *win);
static void _elm_win_del(Elm_Win *win);

Elm_Win_Class _elm_win_class =
{
   &_elm_obj_class, /* parent */
     ELM_OBJ_WIN,
     _elm_win_name_set,
     _elm_win_title_set
};

static void
_elm_child_eval_job(Elm_Win *win)
{
   Evas_List *l;
   int w, h;
   int expand_x, expand_y;
   
   win->deferred_child_eval_job = NULL;
   _elm_obj_nest_push();
   w = h = 0;
   expand_x = expand_y = 0;
   for (l = win->children; l; l = l->next)
     {
	if (((Elm_Obj *)(l->data))->hastype(l->data, ELM_OBJ_WIDGET))
	  {
	     if (((Elm_Widget *)(l->data))->w > w) w = ((Elm_Widget *)(l->data))->w;
	     if (((Elm_Widget *)(l->data))->h > h) h = ((Elm_Widget *)(l->data))->h;
	     if (((Elm_Widget *)(l->data))->expand_x) expand_x = 1;
	     if (((Elm_Widget *)(l->data))->expand_y) expand_y = 1;
	  }
     }
   ecore_evas_size_min_set(win->ee, w, h);
   if ((!expand_x) && (!expand_y)) ecore_evas_size_max_set(win->ee, w, h);
   else if (!expand_x) ecore_evas_size_max_set(win->ee, w, 32727);
   else if (!expand_y) ecore_evas_size_max_set(win->ee, 32767, h);
   else ecore_evas_size_max_set(win->ee, 0, 0);
   if (w < win->w) w = win->w;
   if (h < win->h) h = win->h;
   if ((w > win->w) || (h > win->h)) ecore_evas_resize(win->ee, w, h);
   if (win->showme)
     {
	win->showme = 0;
	ecore_evas_show(win->ee);
     }
   _elm_obj_nest_pop();
}

static void
_elm_on_child_add(void *data, Elm_Win *win, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   if (win->deferred_child_eval_job) ecore_job_del(win->deferred_child_eval_job);
   win->deferred_child_eval_job = ecore_job_add(_elm_child_eval_job, win);
}

static void
_elm_on_child_del(void *data, Elm_Win *win, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   if (win->deferred_child_eval_job) ecore_job_del(win->deferred_child_eval_job);
   win->deferred_child_eval_job = ecore_job_add(_elm_child_eval_job, win);
}

static void
_elm_win_name_set(Elm_Win *win, const char *name)
{
   if (win->name) evas_stringshare_del(win->name);
   win->name = evas_stringshare_add(name);
   if (win->ee) ecore_evas_name_class_set(win->ee, win->name, _elm_appname);
}

static void
_elm_win_title_set(Elm_Win *win, const char *title)
{
   if (win->title) evas_stringshare_del(win->title);
   win->title = evas_stringshare_add(title);
   if (win->ee) ecore_evas_title_set(win->ee, win->title);
}

static void
_elm_win_show(Elm_Win *win)
{
   if (win->deferred_child_eval_job)
     win->showme = 1;
   else
     ecore_evas_show(win->ee);
}

static void
_elm_win_hide(Elm_Win *win)
{
   win->showme = 0;
   ecore_evas_hide(win->ee);
}

static void
_elm_win_type_set(Elm_Win *win, Elm_Win_Type type)
{
   if (win->win_type == type) return;
   win->win_type = type;
   switch (win->win_type)
     {
      case ELM_WIN_BASIC:
	if (win->xwin) ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_NORMAL);
	// FIXME: if child object is a scroll region, then put its child back
	break;
      case ELM_WIN_DIALOG_BASIC:
	if (win->xwin) ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DIALOG);
	// FIXME: if child object is a scroll region, then put its child back
	break;
      default:
	break;
     }
}

static void
_elm_win_geom_set(Elm_Win *win, int x, int y, int w, int h)
{
   if ((win->w != w) || (win->h != h) || (win->x != x) || (win->y != y))
     {
	win->x = x;
	win->y = y;
	win->w = w;
	win->h = h;
	ecore_evas_move_resize(win->ee, win->x, win->y, win->w, win->h);
     }
}
    
static void
_elm_win_size_alloc(Elm_Win *win, int w, int h)
{
   /* this should never be called */
}

static void
_elm_win_size_req(Elm_Win *win, Elm_Widget *child, int w, int h)
{
   if (child)
     {
	if (win->deferred_child_eval_job) ecore_job_del(win->deferred_child_eval_job);
	win->deferred_child_eval_job = ecore_job_add(_elm_child_eval_job, win);
     }
   else
     {
	if ((w == win->w) && (h == win->h)) return;
	ecore_evas_resize(win->ee, w, h);
     }
}

static void
_elm_win_above(Elm_Win *win, Elm_Widget *above)
{
}

static void
_elm_win_below(Elm_Win *win, Elm_Widget *below)
{
}
    
static void
_elm_win_del(Elm_Win *win)
{
   if (_elm_obj_del_defer(ELM_OBJ(win))) return;
   if (win->ee)
     {
	ecore_evas_free(win->ee);
	evas_stringshare_del(win->title);
	evas_stringshare_del(win->name);
     }
   if (win->deferred_resize_job)
     ecore_job_del(win->deferred_resize_job);
   if (win->deferred_child_eval_job)
     ecore_job_del(win->deferred_child_eval_job);
   ((Elm_Obj_Class *)(((Elm_Win_Class *)(win->clas))->parent))->del(ELM_OBJ(win));
}



static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Elm_Win *win = ecore_evas_data_get(ee, "__Elm");
   if (!win) return;
   _elm_obj_nest_push();
   _elm_cb_call(ELM_OBJ(win), ELM_CB_DEL_REQ, NULL);
   if (win->autodel) win->del(ELM_OBJ(win));
   _elm_obj_nest_pop();
}

static void
_elm_win_resize_job(Elm_Win *win)
{
   Evas_List *l;
   int w, h;
   
   win->deferred_resize_job = NULL;
   ecore_evas_geometry_get(win->ee, NULL, NULL, &w, &h);
   if ((win->w == w) && (win->h == h)) return;
   win->w = w;
   win->h = h;
   /* resize all immediate children if they are widgets or sub-classes */
   _elm_obj_nest_push();
   for (l = win->children; l; l = l->next)
     {
	if (((Elm_Obj *)(l->data))->hastype(l->data, ELM_OBJ_WIDGET))
	  ((Elm_Widget *)(l->data))->geom_set(l->data, 0, 0, win->w, win->h);
     }
   _elm_cb_call(ELM_OBJ(win), ELM_CB_RESIZE, NULL);
   _elm_obj_nest_pop();
}

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win *win = ecore_evas_data_get(ee, "__Elm");
   if (!win) return;
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   win->deferred_resize_job = ecore_job_add(_elm_win_resize_job, win);
}

EAPI Elm_Win *
elm_win_new(void)
{
   Elm_Win *win;
   
   win = ELM_NEW(Elm_Win);
   
   _elm_obj_init(ELM_OBJ(win));
   win->clas = &_elm_win_class;
   win->type = ELM_OBJ_WIN;
   
   win->del = _elm_win_del;

   win->geom_set = _elm_win_geom_set;
   win->show = _elm_win_show;
   win->hide = _elm_win_hide;
   win->size_alloc = _elm_win_size_alloc;
   win->size_req = _elm_win_size_req;
   win->above = _elm_win_above;
   win->below = _elm_win_below;
   
   win->name_set = _elm_win_name_set;
   win->title_set = _elm_win_title_set;
   
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
	win->del(ELM_OBJ(win));
	return NULL;
     }
   win->type = ELM_WIN_BASIC;
   win->name = evas_stringshare_add("default"); 
   win->title = evas_stringshare_add("Elementary Window");

   win->evas = ecore_evas_get(win->ee);
   ecore_evas_title_set(win->ee, win->title);
   ecore_evas_name_class_set(win->ee, win->name, _elm_appname);
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

   win->cb_add(win, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   win->cb_add(win, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   
   return win;
}
