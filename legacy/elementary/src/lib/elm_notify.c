/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Notify notify
 *
 * Display a window in a particular region of the application (top, bottom ...).
 * A timeout can be set to automatically close the window.
 *
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo     Subinfo;

struct _Widget_Data
{
   Evas_Object *notify;
   Evas_Object *content;

   Elm_Notify_Orient orient;

   int timeout;
   Ecore_Timer *timer;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _calc(Evas_Object *obj);
static void _content_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);

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
   if(!wd) return;
   _elm_theme_set(wd->notify, "notify", "base", "default");
   edje_object_scale_set(wd->notify, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

   static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   edje_object_size_min_calc(wd->notify, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

   static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   edje_object_part_swallow(wd->notify, "elm.swallow.content", obj);
   _sizing_eval(data);
}

   static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   evas_object_event_callback_del
      (wd->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
   evas_object_del(wd->content);
   if(wd->timer)
     {
	ecore_timer_del(wd->timer);
	wd->timer = NULL;
     }
}

   static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _calc(obj);
}

static void _calc(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord x,y,w,h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   edje_object_size_min_calc(wd->notify, &minw, &minh);
   if(wd->content)
     {
	int offx = (w - minw)/2;
	int offy = (h - minh)/2;

	switch(wd->orient)
	  {
	   case ELM_NOTIFY_ORIENT_TOP:
	      evas_object_move(wd->notify, x + offx, 0);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_BOTTOM:
	      evas_object_move(wd->notify, x + offx, h - minh);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_LEFT:
	      evas_object_move(wd->notify, 0, y+offy);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_RIGHT:
	      evas_object_move(wd->notify, w - minw, y+offy);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_TOP_LEFT:
	      evas_object_move(wd->notify, 0, 0);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_TOP_RIGHT:
	      evas_object_move(wd->notify, w-minw, 0);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
	      evas_object_move(wd->notify, 0, h - minh);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	   case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
	      evas_object_move(wd->notify, w-minw, h-minh);
	      evas_object_resize(wd->notify, minw, minh);
	      break;
	  }
     }
}


   static int
_timer_cb(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);

   wd->timer = NULL;
   evas_object_hide(obj);

   return 0;
}

   static void
_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_show(wd->notify);

   if(wd->timer)
     {
	ecore_timer_del(wd->timer);
	wd->timer = NULL;
     }
   if(wd->timeout>0)
     wd->timer = ecore_timer_add(wd->timeout, _timer_cb, obj);
}

   static void
_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_hide(wd->notify);

   if(wd->timer)
     {
	ecore_timer_del(wd->timer);
	wd->timer = NULL;
     }
}

/**
 * Add a new notify to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Notify
 */
   EAPI Evas_Object *
elm_notify_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "notify");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->notify = edje_object_add(e);
   elm_notify_orient_set(obj, ELM_NOTIFY_ORIENT_TOP);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _show, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _hide, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the notify content
 *
 * @param obj The notify object
 * @param content The content will be filled in this notify object
 *
 * @ingroup Notify
 */
   EAPI void
elm_notify_content_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;

   if(wd->content)
     elm_widget_sub_object_del(obj, wd->content);

   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->notify, "elm.swallow.content", content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
	      _changed_size_hints, obj);
	wd->content = content;
	_sizing_eval(obj);
     }
   _calc(obj);
}

/**
 * Set the orientation
 *
 * @param obj The notify object
 * @param orient The new orientation
 */
   EAPI void
elm_notify_orient_set(Evas_Object *obj, Elm_Notify_Orient orient)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->orient = orient;
   switch(orient)
     {
      case ELM_NOTIFY_ORIENT_TOP:
	 _elm_theme_set(wd->notify, "notify", "base", "default");
	 break;
      case ELM_NOTIFY_ORIENT_BOTTOM:
	 _elm_theme_set(wd->notify, "notify", "base", "bottom");
	 break;
      case ELM_NOTIFY_ORIENT_LEFT:
	 _elm_theme_set(wd->notify, "notify", "base", "left");
	 break;
      case ELM_NOTIFY_ORIENT_RIGHT:
	 _elm_theme_set(wd->notify, "notify", "base", "right");
	 break;
      case ELM_NOTIFY_ORIENT_TOP_LEFT:
	 _elm_theme_set(wd->notify, "notify", "base", "top_left");
	 break;
      case ELM_NOTIFY_ORIENT_TOP_RIGHT:
	 _elm_theme_set(wd->notify, "notify", "base", "top_right");
	 break;
      case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
	 _elm_theme_set(wd->notify, "notify", "base", "bottom_left");
	 break;
      case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
	 _elm_theme_set(wd->notify, "notify", "base", "bottom_right");
	 break;
     }

   _resize(obj, NULL, obj, NULL);
}

/**
 * Set the time before the notify window is hidden. <br>
 * Set a value < 0 to disable the timer
 *
 * @param obj The notify object
 * @param time the new timeout
 */
   EAPI void
elm_notify_timeout_set(Evas_Object *obj, int timeout)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->timeout = timeout;
}

/**
 * Re-init the timer
 * @param obj The notify object
 */
   EAPI void
elm_notify_timer_init(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if(wd->timer)
     ecore_timer_del(wd->timer);
   if(wd->timeout>0)
     wd->timer = ecore_timer_add(wd->timeout, _timer_cb, obj);
}
