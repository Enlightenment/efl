#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;
typedef struct _Item_Block Item_Block;

struct _Widget_Data
{
   Evas_Object *scr;
   Evas_Object *pan_smart;
   Evas_Object *content;
   Eina_Inlist *items;
   Eina_Inlist *blocks;
   Evas_Bool min_w : 1;
   Evas_Bool min_h : 1;
};

struct _Item_Block
{
   Eina_Inlist __header;
   int count;
   Evas_Object *obj;
   Eina_List *items;
   Evas_Coord x, y, w, h;
   Evas_Bool realized : 1;
};

struct _Item
{
   Eina_Inlist __header;
   Item_Block *block;
   Eina_Inlist *subblocks; // not done yet
   Eina_List *subitems; // not done yet
   
   Evas_Coord x, y, w, h;
   Evas_Bool realized : 1;
   Evas_Bool selected : 1;
   Evas_Bool expanded : 1; // not done yet
   Evas_Bool disabled : 1;
   
   const Elm_Genlist_Item_Class *itc;
   const void *data;
   Elm_Genlist_Item *parent; // not done yet
   Elm_Genlist_Item_Flags flags;
   struct {
      void (*func) (void *data, Evas_Object *obj, void *event_info);
      const void *data;
   } func;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

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
   elm_smart_scroller_theme_set(wd->scr, "scroller", "base", "default");
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord  vw, vh, minw, minh, maxw, maxh, w, h, vmw, vmh;
   double xw, xy;
   
   evas_object_size_hint_min_get(wd->content, &minw, &minh);
   evas_object_size_hint_max_get(wd->content, &maxw, &maxh);
   evas_object_size_hint_weight_get(wd->content, &xw, &xy);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   if (xw > 0.0)
     {
	if ((minw > 0) && (vw < minw)) vw = minw;
	else if ((maxw > 0) && (vw > maxw)) vw = maxw;
     }
   else if (minw > 0) vw = minw;
   if (xy > 0.0)
     {
	if ((minh > 0) && (vh < minh)) vh = minh;
	else if ((maxh > 0) && (vh > maxh)) vh = maxh;
     }
   else if (minh > 0) vh = minh;
   evas_object_resize(wd->content, vw, vh);
   w = -1;
   h = -1;
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);
   if (wd->min_w) w = vmw + minw;
   if (wd->min_h) h = vmh + minh;
   evas_object_size_hint_min_set(obj, w, h);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->content)
     {
	elm_widget_on_show_region_hook_set(wd->content, NULL, NULL);
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->content = NULL;
	_sizing_eval(obj);
     }
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

typedef struct _Pan Pan;

struct _Pan {
   int x;
};

static Evas_Smart_Class _pan_sc = {NULL};

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{    
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{    
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   
   sd = calloc(1, sizeof(Pan));
   if (!sd) return;
   // FIXME: setup sd
   evas_object_smart_data_set(obj, sd);
   _pan_sc.add(obj);
}
   
static void
_pan_del(Evas_Object *obj)
{
   Pan *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   // FIXME: del smart stuff
   free(sd);
}

static void
_pan_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   evas_object_smart_changed(obj);
}
        
static void
_pan_calculate(Evas_Object *obj)
{
   // FIXME: calcualte new geom
}
   
EAPI Evas_Object *
elm_genlist_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord vw, vh, minw, minh;
   static Evas_Smart *smart = NULL;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   
   wd->scr = elm_smart_scroller_add(e);
   elm_widget_resize_object_set(obj, wd->scr);

   if (!smart)
     {
        static Evas_Smart_Class sc = { "Elm_Genlist_Pan", EVAS_SMART_CLASS_VERSION, };
        
        evas_object_smart_clipped_smart_set(&_pan_sc);
        sc.add = _pan_add;
        sc.del = _pan_del;
        sc.move = _pan_sc.move;
        sc.resize = _pan_resize;
        sc.show = _pan_sc.show;
        sc.hide = _pan_sc.hide;
        sc.color_set = _pan_sc.color_set;
        sc.clip_set = _pan_sc.clip_set;
        sc.clip_unset = _pan_sc.clip_unset;
        sc.calculate = _pan_calculate;
        sc.member_add = _pan_sc.member_add;
        sc.member_del = _pan_sc.member_del;
        smart = evas_smart_class_new(&sc);
     }
      
   wd->pan_smart = evas_object_smart_add(e, smart);
   // FIXME: init the pan
   
   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
                                     _pan_set, _pan_get, 
                                     _pan_max_get, _pan_child_size_get);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);

//   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

static Item *
_item_new(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
          const void *data, Elm_Genlist_Item *parent, 
          Elm_Genlist_Item_Flags flags,
          void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Item *it;
   
   it = calloc(1, sizeof(Item));
   if (!it) return NULL;
   it->itc = itc;
   it->data = data;
   it->parent = parent;
   it->flags = flags;
   it->func.func = func;
   it->func.data = func_data;
   return it;
}

EAPI Elm_Genlist_Item *
elm_genlist_item_append(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                        const void *data, Elm_Genlist_Item *parent, 
                        Elm_Genlist_Item_Flags flags,
                        void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = _item_new(obj, itc, data, parent, flags, func, func_data);
   if (!it) return NULL;
   wd->items = eina_inlist_append(wd->items, (Eina_Inlist *)it);
   return (Elm_Genlist_Item *)it;
}

EAPI Elm_Genlist_Item *
elm_genlist_item_prepend(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                         const void *data, Elm_Genlist_Item *parent, 
                         Elm_Genlist_Item_Flags flags,
                         void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
}

EAPI Elm_Genlist_Item *
elm_genlist_item_insert_before(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                               const void *data, Elm_Genlist_Item *before,
                               Elm_Genlist_Item_Flags flags,
                               void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
}

EAPI Elm_Genlist_Item *
elm_genlist_item_insert_after(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                              const void *data, Elm_Genlist_Item *after,
                              Elm_Genlist_Item_Flags flags,
                              void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
}

EAPI void
elm_genlist_multi_select_set(Evas_Object *obj, Evas_Bool multi)
{
}

EAPI const Eina_List *
elm_genlist_items_get(Evas_Object *obj)
{
}

EAPI const Elm_Genlist_Item *
elm_genlist_selected_item_get(Evas_Object *obj)
{
}

EAPI const Eina_List *
elm_genlist_selected_items_get(Evas_Object *obj)
{
}

EAPI const Eina_List *
elm_genlist_item_items_get(Elm_Genlist_Item *item)
{
}

EAPI void
elm_genlist_item_selected_set(Elm_Genlist_Item *item, Evas_Bool selected)
{
}

EAPI void
elm_genlist_item_expanded_set(Elm_Genlist_Item *item, Evas_Bool expanded)
{
 // not done yet
}

EAPI void
elm_genlist_item_disabld_set(Elm_Genlist_Item *item, Evas_Bool disabld)
{
}

EAPI void
elm_genlist_item_show(Elm_Genlist_Item *item)
{
}

EAPI void
elm_genlist_item_del(Elm_Genlist_Item *item)
{
}

EAPI const void *
elm_genlist_item_data_get(Elm_Genlist_Item *item)
{
}
