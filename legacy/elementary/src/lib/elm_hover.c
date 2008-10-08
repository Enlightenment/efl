#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo     Subinfo;

struct _Widget_Data
{
   Evas_Object *hov;
   Evas_Object *cov;
   Evas_Object *offset, *size;
   Evas_Object *parent, *target;
   Evas_List *subs;
};

struct _Subinfo
{
   const char *swallow;
   Evas_Object *obj;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
/*   
   while (wd->subs)
     {
        Subinfo *si = wd->subs->data;
	wd->subs = evas_list_remove_list(wd->subs, wd->subs);
	evas_stringshare_del(si->swallow);
	free(si);
     }
 */
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;
   
   if (wd->parent) evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   if (wd->hov) evas_object_geometry_get(wd->hov, &x2, &y2, &w2, &h2);
   evas_object_move(wd->cov, x, y);
   evas_object_resize(wd->cov, w, h);
   evas_object_size_hint_min_set(wd->offset, x2 - x, y2 - y);
   edje_object_part_swallow(wd->hov, "elm.swallow.offset", wd->offset);
   evas_object_size_hint_min_set(wd->size, w2, h2);
   edje_object_part_swallow(wd->hov, "elm.swallow.size", wd->size);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_List *l;
/*   
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	if (si->obj == obj)
	  {
	     edje_object_part_swallow(wd->hov, si->swallow, obj);
	     _sizing_eval(obj);
	     break;
	  }
     }
 */
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   Evas_List *l;

/*   
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	if (si->obj == sub)
	  {
	     evas_object_event_callback_del
	       (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	     wd->subs = evas_list_remove_list(wd->subs, l);
	     evas_stringshare_del(si->swallow);
	     free(si);
	     break;
	  }
     }
 */
}    

static void
_hov_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hov_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hov_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_show(wd->cov);
}

static void
_hov_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_hide(wd->cov);
}

static void
_cov_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_smart_callback_call(data, "clicked", NULL);
   evas_object_hide(data);
}

EAPI Evas_Object *
elm_hover_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);

   wd->hov = evas_object_rectangle_add(e);
   evas_object_color_set(wd->hov, 0, 0, 0, 0);
   elm_widget_resize_object_set(obj, wd->hov);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_MOVE, _hov_move, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_SHOW, _hov_show, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide, obj);
   
   wd->cov = edje_object_add(e);
   _elm_theme_set(wd->cov, "hover", "hover");
   elm_widget_sub_object_add(obj, wd->cov);
   evas_object_event_callback_add(wd->cov, EVAS_CALLBACK_MOUSE_DOWN, _cov_down, obj);
   
   wd->offset = evas_object_rectangle_add(e);
   evas_object_color_set(wd->offset, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->offset);
   
   wd->size = evas_object_rectangle_add(e);
   evas_object_color_set(wd->size, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->size);
   
   edje_object_part_swallow(wd->hov, "elm.swallow.offset", wd->offset);
   edje_object_part_swallow(wd->hov, "elm.swallow.size", wd->size);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_hover_target_set(Evas_Object *obj, Evas_Object *target)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->target = target;
   elm_widget_hover_object_set(target, obj);
   _sizing_eval(obj);
}

EAPI void
elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->parent = parent;
   elm_widget_sub_object_add(parent, obj);
   _sizing_eval(obj);
}

/*
EAPI void
elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   Evas_List *l;
   
   for (l = wd->subs; l; l = l->next)
     {
	si = l->data;
	if (!strcmp(swallow, si->swallow))
	  {
	     if (content == si->obj) return;
	     elm_widget_sub_object_del(obj, si->obj);
	     break;
	  }
     }
   if (content)
     {
	edje_object_part_swallow(wd->hov, swallow, content);
	elm_widget_sub_object_add(obj, content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	si = ELM_NEW(Subinfo);
	si->swallow = evas_stringshare_add(swallow);
	si->obj = content;
	wd->subs = evas_list_append(wd->subs, si);
	_sizing_eval(obj);
     }
}
*/
