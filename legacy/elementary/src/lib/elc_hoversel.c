#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *pop, *hover;
   Evas_Object *hover_parent;
   const char *hover_style;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   elm_hoversel_hover_end(obj);
   elm_hoversel_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj, void *event_info)
{
   elm_hoversel_hover_end(data);
}

static void
_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->hover_parent = NULL;
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   
//   wd->scroller = elm_scroller_add(parent);
//   elm_widget_resize_object_set(obj, wd->scroller);
   
//   evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
//				  _changed_size_hints, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_parent)
     evas_object_event_callback_del(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

EAPI void
elm_hoversel_hover_style_set(Evas_Object *obj, const char *style)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   wd->hover_style = NULL;
   if (style) wd->hover_style = eina_stringshare_add(style);
}  

EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
}
