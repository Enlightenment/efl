#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo     Subinfo;

struct _Widget_Data
{
   Evas_Object *lay;
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
   while (wd->subs)
     {
        Subinfo *si = wd->subs->data;
	wd->subs = evas_list_remove_list(wd->subs, wd->subs);
	evas_stringshare_del(si->swallow);
	free(si);
     }
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->lay, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_List *l;
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	if (si->obj == obj)
	  {
	     edje_object_part_swallow(wd->lay, si->swallow, obj);
	     _sizing_eval(obj);
	     break;
	  }
     }
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   Evas_List *l;
   
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
}    

EAPI Evas_Object *
elm_layout_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->lay = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->lay);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_layout_file_set(Evas_Object *obj, const char *file, const char *group)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_file_set(wd->lay, file, group);
   _sizing_eval(obj);
}

EAPI void
elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
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
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->lay, swallow, content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	si = ELM_NEW(Subinfo);
	si->swallow = evas_stringshare_add(swallow);
	si->obj = content;
	wd->subs = evas_list_append(wd->subs, si);
	_sizing_eval(obj);
     }
}

EAPI Evas_Object *
elm_layout_edje_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->lay;
}
