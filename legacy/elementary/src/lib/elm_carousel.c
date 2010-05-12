#include <Elementary.h>
#include "elm_priv.h"

// FIXME: this is NOT the carousel - yet!

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr, *bx;
   Eina_List *items;
   int icon_size;
};

struct _Elm_Carousel_Item
{
   Evas_Object *obj, *base, *icon;
   const char *label;
   Evas_Smart_Cb func;
   const void *data;
   Eina_Bool selected : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_item_show(Elm_Carousel_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Coord x, y, w, h, bx, by;
   if (!wd) return;
   evas_object_geometry_get(wd->bx, &bx, &by, NULL, NULL);
   evas_object_geometry_get(it->base, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x - bx, y - by, w, h);
}

static void
_item_select(Elm_Carousel_Item *it)
{
   Elm_Carousel_Item *it2;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2;
   const Eina_List *l;
   if (!wd) return;
   if (it->selected) return;
   EINA_LIST_FOREACH(wd->items, l, it2)
     {
	if (it2->selected)
	  {
	     it2->selected = EINA_FALSE;
	     edje_object_signal_emit(it2->base, "elm,state,unselected", "elm");
	     break;
	  }
     }
   it->selected = EINA_TRUE;
   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   _item_show(it);
   obj2 = it->obj;
   if (it->func) it->func((void *)(it->data), it->obj, it);
   evas_object_smart_callback_call(obj2, "clicked", it);
}

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
   const Eina_List *l;
   const Elm_Carousel_Item *it;
   if (!wd) return;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        Evas_Coord mw, mh;

	if (it->selected)
	  edje_object_signal_emit(it->base, "elm,state,selected", "elm");
	_elm_theme_object_set(obj, it->base, "carousel", "item", elm_widget_style_get(obj));
	edje_object_scale_set(it->base, elm_widget_scale_get(obj) * _elm_config->scale);
	if (it->icon)
	  {
	     edje_extern_object_min_size_set(it->icon,
					     (double)wd->icon_size * _elm_config->scale,
					     (double)wd->icon_size * _elm_config->scale);
	     edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
	  }
	edje_object_part_text_set(it->base, "elm.text", it->label);
	edje_object_size_min_calc(it->base, &mw, &mh);
	evas_object_size_hint_min_set(it->base, mw, mh);
	evas_object_size_hint_max_set(it->base, 9999, mh);
     }
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord vw = 0, vh = 0;
   if (!wd) return;
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &minw, &minh);
   evas_object_resize(wd->scr, 500, 500);
   evas_object_size_hint_min_get(wd->bx, &minw, &minh);
   evas_object_resize(wd->bx, minw, minh);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   minw = minw + (500 - vw);
   minh = minh + (500 - vh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord mw, mh, vw, vh, w, h;
   const Eina_List *l;
   Elm_Carousel_Item *it;
   if (!wd) return;
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   evas_object_size_hint_min_get(wd->bx, &mw, &mh);
   evas_object_geometry_get(wd->bx, NULL, NULL, &w, &h);
   if (vw >= mw)
     {
	if (w != vw) evas_object_resize(wd->bx, vw, h);
     }
   EINA_LIST_FOREACH(wd->items, l, it)
     {
	if (it->selected)
	  {
	     _item_show(it);
	     break;
	  }
     }

}

static void
_select(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _item_select(data);
}

EAPI Evas_Object *
elm_carousel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "carousel");
   elm_widget_type_set(obj, "carousel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "carousel", "base", "default");
   elm_widget_resize_object_set(obj, wd->scr);
   elm_smart_scroller_policy_set(wd->scr,
				 ELM_SMART_SCROLLER_POLICY_AUTO,
				 ELM_SMART_SCROLLER_POLICY_OFF);

   wd->icon_size = 32;

   wd->bx = evas_object_box_add(e);
   evas_object_box_layout_set(wd->bx,
		   evas_object_box_layout_homogeneous_horizontal, NULL, NULL);
   elm_widget_sub_object_add(obj, wd->bx);
   elm_smart_scroller_child_set(wd->scr, wd->bx);
   evas_object_show(wd->bx);

   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE,
				  _resize, obj);

   _sizing_eval(obj);
   return obj;
}

EAPI Elm_Carousel_Item *
elm_carousel_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Evas_Coord mw, mh;
   Elm_Carousel_Item *it = calloc(1, sizeof(Elm_Carousel_Item));

   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->func = func;
   it->data = data;
   it->base = edje_object_add(evas_object_evas_get(obj));
   _elm_theme_object_set(obj, it->base, "carousel", "item", elm_widget_style_get(obj));
   edje_object_signal_callback_add(it->base, "elm,action,click", "elm",
				   _select, it);
   elm_widget_sub_object_add(obj, it->base);
   if (it->icon)
     {
	edje_extern_object_min_size_set(it->icon,
					(double)wd->icon_size * _elm_config->scale,
					(double)wd->icon_size * _elm_config->scale);
	edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
	evas_object_show(it->icon);
	elm_widget_sub_object_add(obj, it->icon);
     }
   edje_object_part_text_set(it->base, "elm.text", it->label);
   edje_object_size_min_calc(it->base, &mw, &mh);
   evas_object_size_hint_weight_set(it->base, 0.0, 0.0);
   evas_object_size_hint_align_set(it->base, -1.0, -1.0);
   evas_object_size_hint_min_set(it->base, mw, mh);
   evas_object_size_hint_max_set(it->base, 9999, mh);
   evas_object_box_append(wd->bx, it->base);
   evas_object_show(it->base);
   _sizing_eval(obj);
   return it;
}

EAPI void
elm_carousel_item_del(Elm_Carousel_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2 = it->obj;
   if (!wd) return;
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   if (it->icon) evas_object_del(it->icon);
   evas_object_del(it->base);
   free(it);
   _theme_hook(obj2);
}

EAPI void
elm_carousel_item_select(Elm_Carousel_Item *item)
{
   _item_select(item);
}
