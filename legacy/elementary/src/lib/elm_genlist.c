#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;
typedef struct _Item_Block Item_Block;
typedef struct _Pan Pan;

struct _Widget_Data
{
   Evas_Object *scr;
   Evas_Object *pan_smart;
   Evas_Object *content;
   Eina_Inlist *items;
   Eina_Inlist *blocks;
   Pan *pan;
   Evas_Coord pan_x, pan_y, minw, minh;
   Evas_Bool min_w : 1;
   Evas_Bool min_h : 1;
};

struct _Item_Block
{
   Eina_Inlist __header;
   int count;
   Widget_Data *wd;
   Evas_Object *obj;
   Eina_List *items;
   Evas_Coord x, y, w, h, minw, minh;
   Evas_Bool realized : 1;
   Evas_Bool changed : 1;
};

struct _Item
{
   Eina_Inlist __header;
   Item_Block *block;
   Eina_Inlist *subblocks; // not done yet
   Eina_Inlist *subitems; // not done yet
   
   Evas_Coord x, y, w, h, minw, minh;
   Evas_Bool realized : 1;
   Evas_Bool selected : 1;
   Evas_Bool expanded : 1; // not done yet
   Evas_Bool disabled : 1;
   Evas_Bool mincalcd : 1;
   
   const Elm_Genlist_Item_Class *itc;
   const void *data;
   Elm_Genlist_Item *parent; // not done yet
   Elm_Genlist_Item_Flags flags;
   struct {
      void (*func) (void *data, Evas_Object *obj, void *event_info);
      const void *data;
   } func;
   
   Evas_Object *base;
};

struct _Pan {
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data *wd;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static Evas_Smart_Class _pan_sc = {NULL};

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

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if (x > ow) x = ow;
   if (y > oh) y = oh;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{    
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow; 
   if (y) *y = oh; 
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = calloc(1, sizeof(Pan));
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   _pan_sc.del(obj);
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
_item_realize(Item *it, int in)
{
   const char *stacking;
   
   if (it->realized) return;
   it->base = edje_object_add(evas_object_evas_get(it->block->obj));
// FIXME: hook callbacks   
//   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_DOWN,
//                                  _mouse_down, it);
//   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_UP,
//                                  _mouse_up, it);
   evas_object_smart_member_add(it->base, it->block->wd->pan_smart);
   elm_widget_sub_object_add(it->block->obj, it->base);
   if (in & 0x1)
     _elm_theme_set(it->base, "list", "item_odd", "default");
   else
     _elm_theme_set(it->base, "list", "item", "default");
   stacking = edje_object_data_get(it->base, "stacking");
   if (stacking)
     {
        if (!strcmp(stacking, "below"))
          evas_object_lower(it->base);
        else if (!strcmp(stacking, "above"))
          evas_object_raise(it->base);
     }
   if (it->itc->func.label_get)
     {
        char *s = it->itc->func.label_get(it->data, "xxx");
        if (s)
          {
             edje_object_part_text_set(it->base, "elm.text", s);
             free(s);
          }
     }
   // FIXME: need to get all labels, all icons and all states and all labels
   evas_object_show(it->base);
   if (!it->mincalcd)
     {
        Evas_Coord mw, mh;
        
        mw = mh = -1;
        elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
        elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        it->minw = mw;
        it->minh = mh;
        it->mincalcd = 1;
     }
   it->realized = 1;
}

static void
_item_unrealize(Item *it)
{
   if (!it->realized) return;
   evas_object_del(it->base);
   it->base = NULL;
   it->realized = 0;
}

static void
_item_block_recalc(Item_Block *itb, int in)
{
   Eina_List *l;
   Evas_Coord minw = 0, minh = 0;
   
   for (l = itb->items; l; l = l->next)
     {
        Item *it = l->data;
        _item_realize(it, in);
        if (!itb->realized) _item_unrealize(it);
        minh += it->minh;
        if (minw < it->minw) minw = it->minw;
        in++;
     }
   itb->minw = minw;
   itb->minh = minh;
   itb->changed = 0;
}

static void
_item_block_realize(Item_Block *itb, int in)
{
   Eina_List *l;
   if (itb->realized) return;
   for (l = itb->items; l; l = l->next)
     {
        Item *it = l->data;
        _item_realize(it, in);
        in++;
     }
   itb->realized = 1;
}

static void
_item_block_unrealize(Item_Block *itb)
{
   Eina_List *l;
   
   if (!itb->realized) return;
   for (l = itb->items; l; l = l->next)
     {
        Item *it = l->data;
        _item_unrealize(it);
     }
   itb->realized = 0;
}

static void
_item_block_position(Item_Block *itb)
{
   Eina_List *l;
   Evas_Coord y = 0, ox, oy;
   
   evas_object_geometry_get(itb->wd->pan_smart, &ox, &oy, NULL, NULL);
   for (l = itb->items; l; l = l->next)
     {
        Item *it = l->data;
        
        it->x = 0;
        it->y = y;
        it->w = itb->w;
        it->h = it->minh;
        evas_object_resize(it->base, it->w, it->h);
        evas_object_move(it->base, 
                         ox + itb->x + it->x - itb->wd->pan_x,
                         oy + itb->y + it->y - itb->wd->pan_y);
        y += it->h;
     }
   itb->realized = 1;
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Eina_Inlist *il;
   Evas_Coord minw = 0, minh = 0, x = 0, y = 0, ow, oh;
   int bn, in;
   
   for (bn = 0, in = 0, il = sd->wd->blocks; il; il = il->next, bn++)
     {
        Item_Block *itb = (Item_Block *)il;
        if (itb->changed)
          _item_block_recalc(itb, in);
        itb->y = y;
        itb->x = 0;
        itb->w = itb->minw;
        itb->h = itb->minh;
        minh += itb->minh;
        if (minw < itb->minw) minw = itb->minw;
        itb->h = itb->minh;
        y += itb->minh;
        in += itb->count;
     }
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if (minw < ow) minw = ow;
   if ((minw != sd->wd->minw) || (minh != sd->wd->minh))
     {
        sd->wd->minw = minw;
        sd->wd->minh = minh;
        evas_object_smart_callback_call(obj, "changed", NULL);
     }
   // FIXME: calcualte new geom
   
   for (bn = 0, in = 0, il = sd->wd->blocks; il; il = il->next, bn++)
     {
        Item_Block *itb = (Item_Block *)il;
        itb->w = minw;
        if (ELM_RECTS_INTERSECT(itb->x - sd->wd->pan_x, 
                                itb->y - sd->wd->pan_y, 
                                itb->w, itb->h,
                                0, 0, ow, oh))
          {
             if (!itb->realized)
               {
                  printf("REALIZE BN # %i [%i %i, %ix%i]\n", bn, itb->x, itb->y, itb->w, itb->h);
                  _item_block_realize(itb, in);
               }
             _item_block_position(itb);
          }
        else
          {
             if (itb->realized)
               {
                  printf("UNREALIZE BN # %i [%i %i, %ix%i]\n", bn, itb->x, itb->y, itb->w, itb->h);
                  _item_block_unrealize(itb);
               }
          }
        in += itb->count;
     }
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
        static Evas_Smart_Class sc;
        evas_object_smart_clipped_smart_set(&_pan_sc);
        sc = _pan_sc;
        sc.name = "Elm_Genlist_Pan";
        sc.version = EVAS_SMART_CLASS_VERSION;
        sc.add = _pan_add;
        sc.del = _pan_del;
        sc.resize = _pan_resize;
        sc.calculate = _pan_calculate;
        smart = evas_smart_class_new(&sc);
     }
   if (smart)
     {
        wd->pan_smart = evas_object_smart_add(e, smart);
        wd->pan = evas_object_smart_data_get(wd->pan_smart);
        wd->pan->wd = wd;
     }
   
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

static void
_item_block_add(Widget_Data *wd, Evas_Object *obj, Item *it, Item *itpar, Item *itrel)
{
   Item_Block *itb;
   
   if (!itrel) /* new block */
     {
        newblock:
        itb = calloc(1, sizeof(Item_Block));
        if (!itb) return;
        wd->blocks = eina_inlist_append(wd->blocks, (Eina_Inlist *)itb);
        itb->obj = obj;
        itb->wd = wd;
     }
   else
     {
        itb = itrel->block;
        if (itb->count > 32) goto newblock;
     }
   itb->items = eina_list_append(itb->items, it);
   itb->count++;
   itb->changed = 1;
   it->block = itb;
   evas_object_smart_changed(wd->pan_smart);
}

EAPI Elm_Genlist_Item *
elm_genlist_item_append(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                        const void *data, Elm_Genlist_Item *parent, 
                        Elm_Genlist_Item_Flags flags,
                        void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *itpar, *itrel;
   Item *it = _item_new(obj, itc, data, parent, flags, func, func_data);
   if (!it) return NULL;
   if (!parent)
     {
        wd->items = eina_inlist_append(wd->items, (Eina_Inlist *)it);
        itrel = (Item *)(((Eina_Inlist *)it)->prev);
        _item_block_add(wd, obj, it, NULL, itrel);
     }
   else
     {
        // FIXME: tree. not done yet
        itpar = (Item *)parent;
        itpar->subitems = eina_inlist_append(itpar->subitems, (Eina_Inlist *)it);
     }
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
