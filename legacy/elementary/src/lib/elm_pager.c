#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;

struct _Widget_Data
{
   Eina_List *stack;
   Item *top, *oldtop;
};

struct _Item
{
   Evas_Object *obj, *base, *content;
   Evas_Coord minw, minh;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
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
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     edje_object_scale_set(it->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     {
        if (it->minw > minw) minw = it->minw;
        if (it->minh > minh) minh = it->minh;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   // FIXME: is a child changed size hints
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   // FIXME: handle del
}    

static void
_eval_top(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *ittop;
   if (!wd->stack) return;
   ittop = eina_list_last(wd->stack)->data;
   if (ittop != wd->top)
     {
        Evas_Object *o;
        const char *onshow, *onhide;

        if (wd->top)
          {
             o = wd->top->base;
             edje_object_signal_emit(o, "elm,action,hide", "elm");
             onhide = edje_object_data_get(o, "onhide");
             if (onhide)
               {
                  if (!strcmp(onhide, "raise")) evas_object_raise(o);
                  else if (!strcmp(onhide, "lower")) evas_object_lower(o);
               }
          }
        wd->top = ittop;
        o = wd->top->base;
        evas_object_show(o);
        edje_object_signal_emit(o, "elm,action,show", "elm");
        onshow = edje_object_data_get(o, "onshow");
        if (onshow)
          {
             if (!strcmp(onshow, "raise")) evas_object_raise(o);
             else if (!strcmp(onshow, "lower")) evas_object_lower(o);
          }
#if 0        
        if (wd->oldtop)
          {
             evas_object_hide(wd->oldtop);
             wd->oldtop = NULL;
          }
        printf("old top %p\n", wd->top);
        wd->oldtop = wd->top;
        wd->top = stacktop;
        // FIXME: transition from oldtop to top
        
        printf("hide %i\n", wd->swap);
        o = wd->base[wd->swap];
        edje_object_signal_emit(o, "elm,action,hide", "elm");
        onhide = edje_object_data_get(o, "onhide");
        if (onhide)
          {
             printf("onhide = %s\n", onhide);
             if (!strcmp(onhide, "raise"))
               {
                  printf("raise!\n");
                  evas_object_raise(o);
               }
             else if (!strcmp(onhide, "lower"))
               {
                  printf("lower!\n");
                  evas_object_lower(o);
               }
          }
        
        printf("show %i\n", 1 - wd->swap);
        o = wd->base[1 - wd->swap];
        printf("swallow %p into base %i\n", wd->top, 1 - wd->swap);
        edje_object_part_swallow(o, "elm.swallow.content", wd->top);
        evas_object_show(wd->top);
        edje_object_signal_emit(o, "elm,action,show", "elm");
        onshow = edje_object_data_get(o, "onshow");
        if (onshow)
          {
             printf("onshow = %s\n", onshow);
             if (!strcmp(onshow, "raise"))
               {
                  printf("raise2!\n");
                  evas_object_raise(o);
               }
             else if (!strcmp(onshow, "lower"))
               {
                  printf("lower2!\n");
                  evas_object_lower(o);
               }
          }
        wd->swap = 1 - wd->swap;
        printf("swap = %i\n", wd->swap);
#endif        
     }
}

static void
_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y;
   Eina_List *l;
   Item *it;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   EINA_LIST_FOREACH(wd->stack, l, it)
     evas_object_move(it->base, x, y);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord w, h;
   Eina_List *l;
   Item *it;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(wd->stack, l, it)
     evas_object_resize(it->base, w, h);
}

static void
_signal_show_finished(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   // finished show - don't really care
}

static void
_signal_hide_finished(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Item *it = data;
   evas_object_hide(it->base);
}

EAPI Evas_Object *
elm_pager_add(Evas_Object *parent)
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
   
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_pager_content_push(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = calloc(1, sizeof(Item));
   Evas_Coord x, y, w, h;
   if (!it) return;
   printf("#####------ push %p\n", content);
   it->obj = obj;
   it->content = content;
   it->base = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(it->base, obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(it->base, x, y);
   evas_object_resize(it->base, w, h);
   elm_widget_sub_object_add(obj, it->base); 
   elm_widget_sub_object_add(obj, content); 
   _elm_theme_set(it->base,  "pager", "base", "default");
   edje_object_signal_callback_add(it->base, "elm,action,show,finished", "", _signal_show_finished, it);
   edje_object_signal_callback_add(it->base, "elm,action,hide,finished", "", _signal_hide_finished, it);
   edje_object_part_swallow(it->base, "elm.swallow.content", content);
   evas_object_show(content);
   edje_object_size_min_calc(it->base, &it->minw, &it->minh);
   // FIXME: if child changes size hints...
   wd->stack = eina_list_append(wd->stack, it);
   _eval_top(obj);
   _sizing_eval(obj);
}

EAPI void
elm_pager_content_pop(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   // FIXME: actually make pop animated - promote 2nd last in stack then
   // when anim finished delete 2nd last (which was top).
   Evas_Object *top = elm_pager_content_top_get(obj);
//   if (wd->top) evas_object_del(wd->top);
}

EAPI void
elm_pager_content_promote(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     {
        if (it->content == content)
          {
             wd->stack = eina_list_remove_list(wd->stack, l);
             wd->stack = eina_list_append(wd->stack, it);
             _eval_top(obj);
             return;
          }
     }
}

EAPI Evas_Object *
elm_pager_content_bottom_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   // FIXME: wrong
   if (wd->stack) return wd->stack->data;
   return NULL;
}

EAPI Evas_Object *
elm_pager_content_top_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   // FIXME: wrong
   if (wd->stack) return eina_list_last(wd->stack)->data;
   return NULL;
}

