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
   Eina_List *subs;
   const char *style;
};

struct _Subinfo
{
   const char *swallow;
   Evas_Object *obj;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _reval_content(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _hov_show_do(Evas_Object *obj);
static void _hov_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_show(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _hov_hide(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_hover_target_set(obj, NULL);
   elm_hover_parent_set(obj, NULL);
   evas_object_event_callback_del(wd->hov, EVAS_CALLBACK_MOVE, _hov_move);
   evas_object_event_callback_del(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize);
   evas_object_event_callback_del(wd->hov, EVAS_CALLBACK_SHOW, _hov_show);
   evas_object_event_callback_del(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide);
   while (wd->subs)
     {
        Subinfo *si = wd->subs->data;
	wd->subs = eina_list_remove_list(wd->subs, wd->subs);
	evas_stringshare_del(si->swallow);
	free(si);
     }
   if (wd->style) eina_stringshare_del(wd->style);
}

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
   // FIXME: hover contents doesnt seem to propagate resizes properly
   if (wd->style)
     _elm_theme_set(wd->cov, "hover", "base", wd->style);
   else
     _elm_theme_set(wd->cov, "hover", "base", "default");
   edje_object_message_signal_process(wd->cov);
   _reval_content(obj);
   _sizing_eval(obj);
   if (evas_object_visible_get(wd->cov)) _hov_show_do(obj);
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
   edje_object_part_swallow(wd->cov, "elm.swallow.offset", wd->offset);
   evas_object_size_hint_min_set(wd->size, w2, h2);
   edje_object_part_swallow(wd->cov, "elm.swallow.size", wd->size);
}

static void
_reval_content(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
        edje_object_part_swallow(wd->hov, si->swallow, si->obj);
     }
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	if (si->obj == obj)
	  {
	     edje_object_part_swallow(wd->hov, si->swallow, si->obj);
	     break;
	  }
     }
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   Eina_List *l;

   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	if (si->obj == sub)
	  {
	     evas_object_event_callback_del
	       (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	     wd->subs = eina_list_remove_list(wd->subs, l);
	     evas_stringshare_del(si->swallow);
	     free(si);
	     break;
	  }
     }
}    

static void
_hov_show_do(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   if (wd->cov)
     {
	evas_object_show(wd->cov);
	edje_object_signal_emit(wd->cov, "elm,action,show", "elm");
     }
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	char buf[1024];
	if (!strncmp(si->swallow, "elm.swallow.slot.", 17))
	  {
	     snprintf(buf, sizeof(buf), "elm,action,slot,%s,show", 
		      si->swallow + 17);
	     edje_object_signal_emit(wd->cov, buf, "elm");
	  }
     }
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
   _hov_show_do(data);
}

static void
_hov_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   if (wd->cov)
     {
	edje_object_signal_emit(wd->cov, "elm,action,hide", "elm");
	evas_object_hide(wd->cov);
     }
   for (l = wd->subs; l; l = l->next)
     {
	Subinfo *si = l->data;
	char buf[1024];
	if (!strncmp(si->swallow, "elm.swallow.slot.", 17))
	  {
	     snprintf(buf, sizeof(buf), "elm,action,slot,%s,hide", 
		      si->swallow + 17);
	     edje_object_signal_emit(wd->cov, buf, "elm");
	  }
     }
}

static void
_target_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->target = NULL;
}

static void
_signal_dismiss(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_hide(data);
   evas_object_smart_callback_call(data, "clicked", NULL);
}

static void
_parent_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_parent_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_parent_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_parent_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_hide(wd->cov);
}

static void
_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->parent = NULL;
   _sizing_eval(data);
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
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_del_hook_set(obj, _del_hook);

   wd->hov = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->hov, 1);
   evas_object_color_set(wd->hov, 0, 0, 0, 0);
   elm_widget_resize_object_set(obj, wd->hov);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_MOVE, _hov_move, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_RESIZE, _hov_resize, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_SHOW, _hov_show, obj);
   evas_object_event_callback_add(wd->hov, EVAS_CALLBACK_HIDE, _hov_hide, obj);
   
   wd->cov = edje_object_add(e);
   _elm_theme_set(wd->cov, "hover", "base", "default");
   elm_widget_sub_object_add(obj, wd->cov);
   edje_object_signal_callback_add(wd->cov, "elm,action,dismiss", "", _signal_dismiss, obj);
   
   wd->offset = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->offset, 1);
   evas_object_color_set(wd->offset, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->offset);
   
   wd->size = evas_object_rectangle_add(e);
   evas_object_pass_events_set(wd->size, 1);
   evas_object_color_set(wd->size, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->size);
   
   edje_object_part_swallow(wd->cov, "elm.swallow.offset", wd->offset);
   edje_object_part_swallow(wd->cov, "elm.swallow.size", wd->size);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_hover_target_set(Evas_Object *obj, Evas_Object *target)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->target)
     {
	evas_object_event_callback_del(wd->target, EVAS_CALLBACK_DEL, _target_del);
     }
   wd->target = target;
   if (wd->target)
     {
	evas_object_event_callback_add(wd->target, EVAS_CALLBACK_DEL, _target_del, obj);
	elm_widget_hover_object_set(target, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->parent)
     {
	evas_object_event_callback_del(wd->parent, EVAS_CALLBACK_MOVE, _parent_move);
	evas_object_event_callback_del(wd->parent, EVAS_CALLBACK_RESIZE, _parent_resize);
	evas_object_event_callback_del(wd->parent, EVAS_CALLBACK_SHOW, _parent_show);
	evas_object_event_callback_del(wd->parent, EVAS_CALLBACK_HIDE, _parent_hide);
	evas_object_event_callback_del(wd->parent, EVAS_CALLBACK_DEL, _parent_del);
     }
   wd->parent = parent;
   if (wd->parent)
     {
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_MOVE, _parent_move, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE, _parent_resize, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_SHOW, _parent_show, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_HIDE, _parent_hide, obj);
	evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_DEL, _parent_del, obj);
//	elm_widget_sub_object_add(parent, obj);
     }
   _sizing_eval(obj);
}

EAPI void
elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Subinfo *si;
   Eina_List *l;
   char buf[1024];

   snprintf(buf, sizeof(buf), "elm.swallow.slot.%s", swallow);
   for (l = wd->subs; l; l = l->next)
     {
	si = l->data;
	if (!strcmp(buf, si->swallow))
	  {
	     if (content == si->obj) return;
	     elm_widget_sub_object_del(obj, si->obj);
	     break;
	  }
     }
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->cov, buf, content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	si = ELM_NEW(Subinfo);
	si->swallow = evas_stringshare_add(buf);
	si->obj = content;
	wd->subs = eina_list_append(wd->subs, si);
	_sizing_eval(obj);
     }
}

EAPI void
elm_hover_style_set(Evas_Object *obj, const char *style)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->style) eina_stringshare_del(wd->style);
   if (style) wd->style = eina_stringshare_add(style);
   else wd->style = NULL;
   _elm_theme_set(wd->cov, "hover", "base", style);
   _reval_content(obj);
   _sizing_eval(obj);
}

EAPI const char *
elm_hover_best_content_location_get(Evas_Object *obj, Elm_Hover_Axis pref_axis)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x = 0, y = 0, w = 0, h = 0, x2 = 0, y2 = 0, w2 = 0, h2 = 0;
   Evas_Coord spc_l, spc_r, spc_t, spc_b;
     
   if (wd->parent) evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   if (wd->hov) evas_object_geometry_get(wd->hov, &x2, &y2, &w2, &h2);
   spc_l = x2 - x;
   spc_r = (x + w) - (x2 + w2);
   if (spc_l < 0) spc_l = 0;
   if (spc_r < 0) spc_r = 0;
   spc_t = y2 - y;
   spc_b = (y + h) - (y2 + y2);
   if (spc_t < 0) spc_t = 0;
   if (spc_b < 0) spc_b = 0;
   if (pref_axis == ELM_HOVER_AXIS_HORIZONTAL)
     {
        if (spc_l < spc_r) return "right";
        else return "left";
     }
   else if (pref_axis == ELM_HOVER_AXIS_VERTICAL)
     {
        if (spc_t < spc_b) return "bottom";
        else return "top";
     }
   if (spc_l < spc_r)
     {
        if (spc_t > spc_r) return "top";
        else if (spc_b > spc_r) return "bottom";
        return "right";
     }
   if (spc_t > spc_r) return "top";
   else if (spc_b > spc_r) return "bottom";
   return "left";
}
