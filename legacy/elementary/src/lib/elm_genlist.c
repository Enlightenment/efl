#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;
typedef struct _Item_Block Item_Block;
typedef struct _Pan Pan;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *scr;
   Evas_Object *pan_smart;
   Eina_Inlist *items;
   Eina_Inlist *blocks;
   Pan *pan;
   Evas_Coord pan_x, pan_y, minw, minh;
   Ecore_Job *calc_job;
   Ecore_Idler *queue_idler;
   Eina_List *queue;
   Eina_List *selected;
   Item *show_item;
   Evas_Bool on_hold : 1;
   Evas_Bool multi : 1;
   Evas_Bool min_w : 1;
   Evas_Bool min_h : 1;
};

struct _Item_Block
{
   Eina_Inlist __header;
   int count;
   Widget_Data *wd;
   Eina_List *items;
   Evas_Coord x, y, w, h, minw, minh;
   Evas_Bool realized : 1;
   Evas_Bool changed : 1;
};

struct _Item
{
   Eina_Inlist __header;
   Widget_Data *wd;
   Item_Block *block;
   Eina_Inlist *subblocks; // FIXME: not done yet
   Eina_Inlist *subitems; // FIXME: not done yet
   Evas_Coord x, y, w, h, minw, minh;
   const Elm_Genlist_Item_Class *itc;
   const void *data;
   Elm_Genlist_Item *parent; // not done yet
   Elm_Genlist_Item_Flags flags;
   struct {
      void (*func) (void *data, Evas_Object *obj, void *event_info);
      const void *data;
   } func;
   
   Evas_Object *base;
   Eina_List *labels, *icons, *states;
   Eina_List *icon_objs;
   
   Evas_Bool realized : 1;
   Evas_Bool selected : 1;
   Evas_Bool expanded : 1; // FIXME: not done yet
   Evas_Bool disabled : 1;
   Evas_Bool mincalcd : 1;
   Evas_Bool queued : 1;
   Evas_Bool showme : 1;
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

static void _item_unrealize(Item *it);
static void _item_block_unrealize(Item_Block *itb);
static void _calc_job(void *data);
    
static Evas_Smart_Class _pan_sc = {NULL};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_genlist_clear(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Inlist *il;
   Eina_List *l;
   elm_smart_scroller_theme_set(wd->scr, "scroller", "base", "default");
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   for (il = wd->blocks; il; il = il->next)
     {
        Item_Block *itb = (Item_Block *)il;
        if (itb->realized) _item_block_unrealize(itb);
        for (l = itb->items; l; l = l->next)
          {
             Item *it = l->data;
             it->mincalcd = 0;
          }
        itb->changed = 1;
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
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

   // fixme - now handle scroll hinting etc.
   /*
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
    */
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static Eina_List *
_stringlist_get(const char *str)
{
   Eina_List *list = NULL;
   const char *s, *b;
   if (!str) return NULL;
   for (b = s = str; 1; s++)
     {
        if ((*s == ' ') || (*s == 0))
          {
             char *t = malloc(s - b + 1);
             if (t)
               {
                  strncpy(t, b, s - b);
                  t[s - b] = 0;
                  list = eina_list_append(list, eina_stringshare_add(t));
                  free(t);
               }
             b = s + 1;
          }
        if (*s == 0) break;
     }
   return list;
}

static void
_stringlist_free(Eina_List *list)
{
   while (list)
     {
        eina_stringshare_del(list->data);
        list = eina_list_remove_list(list, list);
     }
}

static void
_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Item *it = data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = 1;
   else it->wd->on_hold = 0;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(it->wd->obj, "clicked", it);
}

static void
_item_select(Item *it)
{
   const char *selectraise;
   if (it->selected) return;
   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     evas_object_raise(it->base);
   it->selected = 1;
   it->wd->selected = eina_list_append(it->wd->selected, it);
   if (it->func.func) it->func.func((void *)it->func.data, it->wd->obj, it);
   evas_object_smart_callback_call(it->wd->obj, "selected", it);
}

static void
_item_unselect(Item *it)
{
   const char *stacking, *selectraise;
   if (!it->selected) return;
   edje_object_signal_emit(it->base, "elm,state,unselected", "elm");
   stacking = edje_object_data_get(it->base, "stacking");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
        if ((stacking) && (!strcmp(stacking, "below")))
          evas_object_lower(it->base);
     }
   it->selected = 0;
   it->wd->selected = eina_list_remove(it->wd->selected, it);
   evas_object_smart_callback_call(it->wd->obj, "unselected", it);
}

static void
_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_List *l;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = 1;
   else it->wd->on_hold = 0;
   if (it->wd->on_hold)
     {
        it->wd->on_hold = 0;
        return;
     }
   if (it->disabled) return;
   if (it->wd->multi)
     {
        if (!it->selected) _item_select(it);
        else _item_unselect(it);
     }
   else
     {
        for (l = it->wd->selected; l;)
          {
             Item *it2 = l->data;
             l = l->next;
             if ((it2 != it) && (it2->selected)) _item_unselect(it2);
          }
        if (!it->selected) _item_select(it);
     }
}

static void
_item_realize(Item *it, int in, int calc)
{
   const char *stacking;
   char buf[1024];
   
   if (it->realized) return;
   it->base = edje_object_add(evas_object_evas_get(it->wd->obj));
   edje_object_scale_set(it->base, elm_widget_scale_get(it->wd->obj) * _elm_config->scale);
   evas_object_smart_member_add(it->base, it->wd->pan_smart);
   elm_widget_sub_object_add(it->wd->obj, it->base);
   if (in & 0x1)
     snprintf(buf, sizeof(buf), "%s/%s", "item_odd", it->itc->item_style);
   else
     snprintf(buf, sizeof(buf), "%s/%s", "item", it->itc->item_style);
   _elm_theme_set(it->base, "genlist", buf, "default");
   if (!calc)
     {
        stacking = edje_object_data_get(it->base, "stacking");
        if (stacking)
          {
             if (!strcmp(stacking, "below")) evas_object_lower(it->base);
             else if (!strcmp(stacking, "above")) evas_object_raise(it->base);
          }
        evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_DOWN,
                                       _mouse_down, it);
        evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_UP,
                                       _mouse_up, it);
        if (it->selected)
          edje_object_signal_emit(it->base, "elm,state,selected", "elm");
        if (it->disabled)
          edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
     }
   
   if (it->itc->func.label_get)
     {
        Eina_List *l;
        
        it->labels = _stringlist_get(edje_object_data_get(it->base, "labels"));
        for (l = it->labels; l; l = l->next)
          {
             const char *key = l->data;
             char *s = it->itc->func.label_get(it->data, it->wd->obj, l->data);
             if (s)
               {
                  edje_object_part_text_set(it->base, l->data, s);
                  free(s);
               }
          }
     }
   if (it->itc->func.icon_get)
     {
        Eina_List *l;
        
        it->icons = _stringlist_get(edje_object_data_get(it->base, "icons"));
        for (l = it->icons; l; l = l->next)
          {
             const char *key = l->data;
             Evas_Object *ic = it->itc->func.icon_get(it->data, it->wd->obj, l->data);
             if (ic)
               {
                  it->icon_objs = eina_list_append(it->icon_objs, ic);
                  edje_object_part_swallow(it->base, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(it->wd->obj, ic);
               }
          }
     }
   if (it->itc->func.state_get)
     {
        Eina_List *l;
        
        it->states = _stringlist_get(edje_object_data_get(it->base, "states"));
        for (l = it->states; l; l = l->next)
          {
             const char *key = l->data;
             Evas_Bool on = it->itc->func.state_get(it->data, it->wd->obj, l->data);
             if (on)
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
                  edje_object_signal_emit(it->base, buf, "elm");
               }
          }
     }
   if (!it->mincalcd)
     {
        Evas_Coord mw = -1, mh = -1;
        elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
        elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        it->w = it->minw = mw;
        it->h = it->minh = mh;
        it->mincalcd = 1;
     }
   if (!calc) evas_object_show(it->base);
   it->realized = 1;
}

static void
_item_unrealize(Item *it)
{
   if (!it->realized) return;
   evas_object_del(it->base);
   it->base = NULL;
   _stringlist_free(it->labels);
   it->labels = NULL;
   _stringlist_free(it->icons);
   it->icons = NULL;
   _stringlist_free(it->states);
   while (it->icon_objs)
     {
        evas_object_del(it->icon_objs->data);
        it->icon_objs = eina_list_remove_list(it->icon_objs, it->icon_objs);
     }
   it->states = NULL;
   it->realized = 0;
}

static int
_item_block_recalc(Item_Block *itb, int in)
{
   Eina_List *l;
   Evas_Coord minw = 0, minh = 0;
   int showme = 0;
   Evas_Coord y = 0;
   
   for (l = itb->items; l; l = l->next)
     {
        Item *it = l->data;
        showme |= it->showme;
        if (!itb->realized)
          {
             _item_realize(it, in, 1);
             _item_unrealize(it);
          }
        else
          _item_realize(it, in, 0);
        minh += it->minh;
        if (minw < it->minw) minw = it->minw;
        in++;
        it->x = 0;
        it->y = y;
        y += it->minw;
     }
   itb->minw = minw;
   itb->minh = minh;
   itb->changed = 0;
   /* force an evas norender to garbage collect deleted objects */
   evas_norender(evas_object_evas_get(itb->wd->obj));
   return showme;
}

static void
_item_block_realize(Item_Block *itb, int in)
{
   Eina_List *l;
   if (itb->realized) return;
   for (l = itb->items; l; l = l->next)
     {
        Item *it = l->data;
        _item_realize(it, in, 0);
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
        if (it->realized)
          {
             evas_object_resize(it->base, it->w, it->h);
             evas_object_move(it->base, 
                              ox + itb->x + it->x - itb->wd->pan_x,
                              oy + itb->y + it->y - itb->wd->pan_y);
             evas_object_show(it->base);
          }
        y += it->h;
     }
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Eina_Inlist *il;
   Evas_Coord minw = -1, minh = 0, x = 0, y = 0, ow, oh;
   Item_Block *chb = NULL;
   int bn, in;
   int minw_change = 0;
   
   for (bn = 0, in = 0, il = wd->blocks; il; il = il->next, bn++)
     {
        Item_Block *itb = (Item_Block *)il;
        int showme = 0;
        if (chb)
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        if (itb->changed)
          {
             if (itb->realized) _item_block_unrealize(itb);
             showme = _item_block_recalc(itb, in);
             chb = itb;
          }
        itb->y = y;
        itb->x = 0;
        minh += itb->minh;
        if (minw == -1) minw = itb->minw;
        else if (minw < itb->minw)
          {
             minw = itb->minw;
             minw_change = 1;
          }
        itb->w = minw;
        itb->h = itb->minh;
        y += itb->h;
        in += itb->count;
        if (showme)
          {
             wd->show_item->showme = 0;
             elm_smart_scroller_child_region_show(wd->scr, 
                                                  wd->show_item->x + wd->show_item->block->x, 
                                                  wd->show_item->y + wd->show_item->block->y,
                                                  wd->show_item->block->w, 
                                                  wd->show_item->h);
             wd->show_item = NULL;
             showme = 0;
          }
     }
   if (minw_change)
     {
        for (il = wd->blocks; il; il = il->next)
          {
             Item_Block *itb = (Item_Block *)il;
             itb->minw = minw;
             itb->w = itb->minw;
          }
     }
   if ((chb) && (((Eina_Inlist *)(chb))->next))
     {
        for (il = ((Eina_Inlist *)(chb))->next; il; il = il->next)
          {
             Item_Block *itb = (Item_Block *)il;
             if (itb->realized) _item_block_unrealize(itb);
          }
     }
   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &ow, &oh);
   if (minw < ow) minw = ow;
   if ((minw != wd->minw) || (minh != wd->minh))
     {
        wd->minw = minw;
        wd->minh = minh;
        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
     }
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
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
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Eina_Inlist *il;
   Evas_Coord ow, oh;
   int bn, in;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   for (bn = 0, in = 0, il = sd->wd->blocks; il; il = il->next, bn++)
     {
        Item_Block *itb = (Item_Block *)il;
        itb->w = sd->wd->minw;
        if (ELM_RECTS_INTERSECT(itb->x - sd->wd->pan_x, 
                                itb->y - sd->wd->pan_y, 
                                itb->w, itb->h,
                                0, 0, ow, oh))
          {
             if ((!itb->realized) || (itb->changed))
               {
                  _item_block_realize(itb, in);
               }
             _item_block_position(itb);
          }
        else
          {
             if (itb->realized)
               {
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
   
   wd->obj = obj;

   if (!smart)
     {
        static Evas_Smart_Class sc;
        evas_object_smart_clipped_smart_set(&_pan_sc);
        sc = _pan_sc;
        sc.name = "elm_genlist_pan";
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
   
   _sizing_eval(obj);
   return obj;
}

static Item *
_item_new(Widget_Data *wd, const Elm_Genlist_Item_Class *itc, 
          const void *data, Elm_Genlist_Item *parent, 
          Elm_Genlist_Item_Flags flags,
          void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Item *it;
   
   it = calloc(1, sizeof(Item));
   if (!it) return NULL;
   it->wd = wd;
   it->itc = itc;
   it->data = data;
   it->parent = parent;
   it->flags = flags;
   it->func.func = func;
   it->func.data = func_data;
   return it;
}

static void
_item_block_del(Item *it)
{
   Eina_Inlist *il;
   Item_Block *itb = it->block;
   
   itb->items = eina_list_remove(itb->items, it);
   itb->count--;
   itb->changed = 1;
   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
   if (itb->count < 1)
     {
        it->wd->blocks = eina_inlist_remove(it->wd->blocks, (Eina_Inlist *)itb);
        free(itb);
     }
   else
     {
        if (itb->count < 16)
          {
             Item_Block *itbp = (Item_Block *)(((Eina_Inlist *)(itb))->prev);
             Item_Block *itbn = (Item_Block *)(((Eina_Inlist *)(itb))->next);
             if ((itbp) && ((itbp->count + itb->count) < 48))
               {
                  while (itb->items)
                    {
                       Item *it2 = itb->items->data;
                       it2->block = itbp;
                       itbp->items = eina_list_append(itbp->items, it2);
                       itb->items = eina_list_remove_list(itb->items, itb->items);
                       itbp->count++;
                       itbp->changed = 1;
                    }
                  it->wd->blocks = eina_inlist_remove(it->wd->blocks, (Eina_Inlist *)itb);
                  free(itb);
               }
             else if ((itbn) && ((itbn->count + itb->count) < 48))
               {
                  while (itb->items)
                    {
                       Eina_List *last = eina_list_last(itb->items);
                       Item *it2 = last->data;
                       it2->block = itbn;
                       itb->items = eina_list_remove_list(itb->items, last);
                       itbn->items = eina_list_prepend(itbn->items, it2);
                       itbn->count++;
                       itbn->changed = 1;
                    }
                  it->wd->blocks = eina_inlist_remove(it->wd->blocks, (Eina_Inlist *)itb);
                  free(itb);
               }
          }
     }
}

static void
_item_del(Item *it)
{
   if (it->wd->show_item == it) it->wd->show_item = NULL;
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);
   if (it->itc->func.del) it->itc->func.del(it->data, it->wd->obj);
   if (it->realized) _item_unrealize(it);
   if (it->block) _item_block_del(it);
   // FIXME: tree. del it->subblocks
   // FIXME: tree. del it->subitems
   if (it->queued)
     {
        it->wd->queue = eina_list_remove(it->wd->queue, it);
     }
   it->wd->items = eina_inlist_remove(it->wd->items, (Eina_Inlist *)it);
   free(it);
}

static void
_item_block_add(Widget_Data *wd, Item *it, Item *itpar, Item *itrel, int before)
{
   Item_Block *itb;

   if (!itrel)
     {
        newblock:
        itb = calloc(1, sizeof(Item_Block));
        if (!itb) return;
        itb->wd = wd;
        if (!it->parent)
          {
             if (itrel)
               {
                  if (!itrel->block)
                    {
                       wd->blocks = eina_inlist_append(wd->blocks, (Eina_Inlist *)itb);
                       itb->items = eina_list_append(itb->items, it);
                    }
                  else
                    {
                       if (before)
                         {
                            wd->blocks = eina_inlist_prepend_relative(wd->blocks, (Eina_Inlist *)itb, (Eina_Inlist *)(itrel->block));
                            itb->items = eina_list_prepend_relative(itb->items, it, itrel);
                         }
                       else
                         {
                            wd->blocks = eina_inlist_append_relative(wd->blocks, (Eina_Inlist *)itb, (Eina_Inlist *)(itrel->block));
                            itb->items = eina_list_append_relative(itb->items, it, itrel);
                         }
                    }
               }
             else
               {
                  wd->blocks = eina_inlist_append(wd->blocks, (Eina_Inlist *)itb);
                  itb->items = eina_list_append(itb->items, it);
               }
          }
        else
          {
             // FIXME: tree not handled.
          }
     }
   else
     {
        itb = itrel->block;
        if ((!itb) || (itb->count > 32)) goto newblock;
        if (before)
          itb->items = eina_list_prepend_relative(itb->items, it, itrel);
        else
          itb->items = eina_list_append_relative(itb->items, it, itrel);
     }
   itb->count++;
   itb->changed = 1;
   it->block = itb;
   if (itb->wd->calc_job) ecore_job_del(itb->wd->calc_job);
   itb->wd->calc_job = ecore_job_add(_calc_job, itb->wd);
}

static int
_item_idler(void *data)
{
   Widget_Data *wd = data;
   int n;
   
   for (n = 0; (wd->queue) && (n < 16); n++)
     {
        Item *itrel, *it;
        
        it = wd->queue->data;
        wd->queue = eina_list_remove_list(wd->queue, wd->queue);
        it->queued = 0;
        if (!it->parent)
          {
             itrel = (Item *)(((Eina_Inlist *)it)->prev);
             if (itrel) _item_block_add(wd, it, NULL, itrel, 0);
             else
               {
                  itrel = (Item *)(((Eina_Inlist *)it)->next);
                  if (itrel && itrel->queued)
                    _item_block_add(wd, it, NULL, NULL, 0);
                  else
                    _item_block_add(wd, it, NULL, itrel, 1);
               }
          }
        else
          {
             // FIXME: tree. not done yet
          }
     }
   if (n > 0)
     {
        if (wd->calc_job) ecore_job_del(wd->calc_job);
        wd->calc_job = ecore_job_add(_calc_job, wd);
     }
   if (!wd->queue)
     {
        wd->queue_idler = NULL;
        return 0;
     }
   return 1;
}

static void
_item_queue(Widget_Data *wd, Item *it)
{
   if (it->queued) return;
   if (!wd->queue_idler) wd->queue_idler = ecore_idler_add(_item_idler, wd);
   it->queued = 1;
   wd->queue = eina_list_append(wd->queue, it);
}

EAPI Elm_Genlist_Item *
elm_genlist_item_append(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                        const void *data, Elm_Genlist_Item *parent, 
                        Elm_Genlist_Item_Flags flags,
                        void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = _item_new(wd, itc, data, parent, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_append(wd->items, (Eina_Inlist *)it);
   else
     {
        // FIXME: tree. not done yet
     }
   _item_queue(wd, it);
   return (Elm_Genlist_Item *)it;
}

EAPI Elm_Genlist_Item *
elm_genlist_item_prepend(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                         const void *data, Elm_Genlist_Item *parent, 
                         Elm_Genlist_Item_Flags flags,
                         void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = _item_new(wd, itc, data, parent, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_prepend(wd->items, (Eina_Inlist *)it);
   else
     {
        // FIXME: tree. not done yet
     }
   _item_queue(wd, it);
   return (Elm_Genlist_Item *)it;
}

EAPI Elm_Genlist_Item *
elm_genlist_item_insert_before(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                               const void *data, Elm_Genlist_Item *before,
                               Elm_Genlist_Item_Flags flags,
                               void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = _item_new(wd, itc, data, NULL, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_prepend_relative(wd->items, (Eina_Inlist *)it, (Eina_Inlist *)before);
   else
     {
        // FIXME: tree. not done yet
     }
   _item_queue(wd, it);
   return (Elm_Genlist_Item *)it;
}

EAPI Elm_Genlist_Item *
elm_genlist_item_insert_after(Evas_Object *obj, const Elm_Genlist_Item_Class *itc, 
                              const void *data, Elm_Genlist_Item *after,
                              Elm_Genlist_Item_Flags flags,
                              void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = _item_new(wd, itc, data, NULL, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_append_relative(wd->items, (Eina_Inlist *)it, (Eina_Inlist *)after);
   else
     {
        // FIXME: tree. not done yet
     }
   _item_queue(wd, it);
   return (Elm_Genlist_Item *)it;
}

EAPI void
elm_genlist_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   while (wd->items)
     {
        Item *it = (Item *)(wd->items);
        wd->items = eina_inlist_remove(wd->items, wd->items);
        if (it->realized) _item_unrealize(it);
        if (it->itc->func.del) it->itc->func.del(it->data, it->wd->obj);
        free(it);
     }
   while (wd->blocks)
     {
        Item_Block *itb = (Item_Block *)(wd->blocks);
        wd->blocks = eina_inlist_remove(wd->blocks, wd->blocks);
        if (itb->items) eina_list_free(itb->items);
        free(itb);
     }
   if (wd->calc_job)
     {
        ecore_job_del(wd->calc_job);
        wd->calc_job = NULL;
     }
   if (wd->queue_idler)
     {
        ecore_idler_del(wd->queue_idler);
        wd->queue_idler = NULL;
     }
   if (wd->queue)
     {
        eina_list_free(wd->queue);
        wd->queue = NULL;
     }
   if (wd->selected)
     {
        eina_list_free(wd->selected);
        wd->selected = NULL;
     }
   wd->show_item = NULL;
   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;
   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
}

EAPI void
elm_genlist_multi_select_set(Evas_Object *obj, Evas_Bool multi)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->multi = multi;
}

EAPI const Elm_Genlist_Item *
elm_genlist_selected_item_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->selected) return wd->selected->data;
   return NULL;
}

EAPI const Eina_List *
elm_genlist_selected_items_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->selected;
}

EAPI const Elm_Genlist_Item *
elm_genlist_first_item_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return (Elm_Genlist_Item *)(wd->items);
}

EAPI const Elm_Genlist_Item *
elm_genlist_last_item_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd->items) return NULL;
   return (Elm_Genlist_Item*)(wd->items->last);
}

EAPI const Elm_Genlist_Item *
elm_genlist_item_next_get(Elm_Genlist_Item *item)
{
   return (Elm_Genlist_Item *)(((Eina_Inlist *)item)->next);
}

EAPI const Elm_Genlist_Item *
elm_genlist_item_prev_get(Elm_Genlist_Item *item)
{
   return (Elm_Genlist_Item *)(((Eina_Inlist *)item)->prev);
}

EAPI void
elm_genlist_item_selected_set(Elm_Genlist_Item *item, Evas_Bool selected)
{
   Item *it = (Item *)item;
   Widget_Data *wd = elm_widget_data_get(it->wd->obj);
   Eina_List *l;
   if (selected)
     {
        if (!wd->multi)
          {
             for (l = it->wd->selected; l;)
               {
                  Item *it2 = l->data;
                  l = l->next;
                  if ((it2 != it) && (it2->selected)) _item_unselect(it2);
               }
          }
        if (!it->selected) _item_select(it);
     }
   else if (it->selected)
     _item_unselect(it);
}

EAPI Evas_Bool
elm_genlist_item_selected_get(Elm_Genlist_Item *item)
{
   Item *it = (Item *)item;
   return it->selected;
}

EAPI void
elm_genlist_item_expanded_set(Elm_Genlist_Item *item, Evas_Bool expanded)
{
   // FIXME: tree. not done yet
}

EAPI Evas_Bool
elm_genlist_item_expanded_get(Elm_Genlist_Item *item)
{
   // FIXME: tree. not done yet
}
    
EAPI void
elm_genlist_item_disabled_set(Elm_Genlist_Item *item, Evas_Bool disabled)
{
   Item *it = (Item *)item;
   if (it->disabled == disabled) return;
   it->disabled = disabled;
   if (it->realized)
     {
        if (it->disabled)
          edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(it->base, "elm,state,enabled", "elm");
     }
}

EAPI Evas_Bool
elm_genlist_item_disabled_get(Elm_Genlist_Item *item)
{
   Item *it = (Item *)item;
   return it->disabled;
}

EAPI void
elm_genlist_item_show(Elm_Genlist_Item *item)
{
   Item *it = (Item *)item;
   if ((it->queued) || (!it->mincalcd))
     {
        it->wd->show_item = it;
        it->showme = 1;
        return;
     }
   if (it->wd->show_item)
     {
        it->wd->show_item->showme = 0;
        it->wd->show_item = NULL;
     }
   elm_smart_scroller_child_region_show(it->wd->scr,
                                        it->x + it->block->x, 
                                        it->y + it->block->y,
                                        it->block->w, it->h);
}

EAPI void
elm_genlist_item_del(Elm_Genlist_Item *item)
{
   Item *it = (Item *)item;
   if (!it) return;
   _item_del(it);
}

EAPI const void *
elm_genlist_item_data_get(Elm_Genlist_Item *item)
{
   Item *it = (Item *)item;
   return it->data;
}

EAPI void
elm_genlist_item_update(Elm_Genlist_Item *item)
{
   // fixme - now do call all the class get funcs again - re realize if realized.
}
