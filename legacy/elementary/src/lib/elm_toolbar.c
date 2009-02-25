#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr, *bx;
   Eina_List *items;
   int icon_size;
   Evas_Bool scrollable : 1;
};

struct _Elm_Toolbar_Item
{
   Evas_Object *obj;
   Evas_Object *base;
   const char *label;
   Evas_Object *icon;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   const void *data;
   Evas_Bool selected : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_item_show(Elm_Toolbar_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Coord x, y, w, h, bx, by;
   
   evas_object_geometry_get(wd->bx, &bx, &by, NULL, NULL);
   evas_object_geometry_get(it->base, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x - bx, y - by, w, h);
}

static void
_item_select(Elm_Toolbar_Item *it)
{
   Elm_Toolbar_Item *it2;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2;
   const Eina_List *l;
   if (it->selected) return;
   EINA_LIST_FOREACH(wd->items, l, it2)
     {
        if (it2->selected)
          {
             it2->selected = 0;
             edje_object_signal_emit(it2->base, "elm,state,unselected", "elm");
             break;
          }
     }
   it->selected = 1;
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
   Elm_Toolbar_Item *it;
   EINA_LIST_FREE(wd->items, it)
     {
        eina_stringshare_del(it->label);
        if (it->icon) evas_object_del(it->icon);
        evas_object_del(it->base);
        free(it);
     }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Elm_Toolbar_Item *it;
   Evas_Coord mw, mh;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->selected)
          edje_object_signal_emit(it->base, "elm,state,selected", "elm");
        _elm_theme_set(it->base, "toolbar", "item", "default");
        if (it->icon)
          {
             edje_extern_object_min_size_set(it->icon, 
                                             (double)wd->icon_size * _elm_config->scale, 
                                             (double)wd->icon_size * _elm_config->scale);
             edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
          }
        edje_object_part_text_set(it->base, "elm.text", it->label);
        mw = mh = -1;
        elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
        elm_coords_finger_size_adjust(1, &mw, 1, &mh);
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

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &minw, &minh);
   evas_object_resize(wd->scr, 500, 500);
   evas_object_size_hint_min_get(wd->bx, &minw, &minh);
   evas_object_resize(wd->bx, minw, minh);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   if (wd->scrollable)
     {
        minw = 500 - vw;
        minh = minh + (500 - vh);
     }
   else
     {
        minw = minw + (500 - vw);
        minh = minh + (500 - vh);
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord mw, mh, vw, vh, w, h;
   const Eina_List *l;
   Elm_Toolbar_Item *it;
   
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
_select(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   _item_select(data);
}

EAPI Evas_Object *
elm_toolbar_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);
   
   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_theme_set(wd->scr, "toolbar", "base", "default");
   elm_widget_resize_object_set(obj, wd->scr);
   elm_smart_scroller_policy_set(wd->scr, 
                                 ELM_SMART_SCROLLER_POLICY_AUTO,
                                 ELM_SMART_SCROLLER_POLICY_OFF);
   
   wd->icon_size = 32;
   wd->scrollable = 1;
   
   wd->bx = _els_smart_box_add(e);
   _els_smart_box_orientation_set(wd->bx, 1);
   _els_smart_box_homogenous_set(wd->bx, 1);
   elm_widget_sub_object_add(obj, wd->bx);
   elm_smart_scroller_child_set(wd->scr, wd->bx);
   evas_object_show(wd->bx);

   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE,
                                  _resize, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI Elm_Toolbar_Item *
elm_toolbar_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;
   Elm_Toolbar_Item *it = calloc(1, sizeof(Elm_Toolbar_Item));
   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->func = func;
   it->data = data;
   it->base = edje_object_add(evas_object_evas_get(obj));
   _elm_theme_set(it->base, "toolbar", "item", "default");
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
   mw = mh = -1;
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   evas_object_size_hint_weight_set(it->base, 0.0, 0.0);
   evas_object_size_hint_align_set(it->base, -1.0, -1.0);
   evas_object_size_hint_min_set(it->base, mw, mh);
   evas_object_size_hint_max_set(it->base, 9999, mh);
   _els_smart_box_pack_end(wd->bx, it->base);
   evas_object_show(it->base);
   _sizing_eval(obj);
   return it;
}

EAPI void
elm_toolbar_item_del(Elm_Toolbar_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2 = it->obj;
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   if (it->icon) evas_object_del(it->icon);
   evas_object_del(it->base);
   free(it);
   _theme_hook(obj2);
}

EAPI void
elm_toolbar_item_select(Elm_Toolbar_Item *item)
{
   _item_select(item);
}

EAPI void
elm_toolbar_scrollable_set(Evas_Object *obj, Evas_Bool scrollable)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->scrollable = scrollable;
   _sizing_eval(obj);
}
