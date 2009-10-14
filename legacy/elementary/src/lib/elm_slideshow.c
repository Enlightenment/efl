/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Slideshow slideshow
 *
 * This object display a list of object (generally a list of images) and some actions like
 * next/previous are used to navigate. The animations are defined in the theme,
 * consequently new animations can be added without having to update the
 * applications.
 *
 * The slideshow use 2 callbacks to create and delete the objects displayed. When an item is displayed the function itc->func.get() is called. This function should create the object, for example the object can be an evas_object_image or a photocam. When a object is no more displayed the function itc->func.del() is called, the user can delete or hide the object here. If itc->func.del is NULL the slideshow delete the object with evas_object_del().
 */

typedef struct _Widget_Data Widget_Data;

struct _Elm_Slideshow_Item
{
   Evas_Object *obj;

   Eina_List *l;

   const void *data;
   const Elm_Slideshow_Item_Class *itc;

   Evas_Object *o;
};

struct _Widget_Data
{
   Evas_Object *slideshow;

   // list of Elm_Slideshow_Item*
   Eina_List *items;
   Elm_Slideshow_Item *current;
   Elm_Slideshow_Item *previous;
   int loop;

   Eina_List *transitions;
   const char *transition;

   Ecore_Timer *timer;
   int timeout;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _signal_clicked(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _signal_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int _timer_cb(void *data);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   elm_slideshow_clear(obj);
   _stringlist_free(wd->transitions);
   if (wd->timer) ecore_timer_del(wd->timer);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   _elm_theme_set(wd->slideshow, "slideshow", "base", "default");
   edje_object_scale_set(wd->slideshow, elm_widget_scale_get(obj) * 
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   edje_object_size_min_calc(wd->slideshow, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, minw, minh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
}

static void
_end(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Evas_Object *o;
   Eina_List *l;
   Elm_Slideshow_Item *item;
   const char *part;
   Widget_Data *wd = elm_widget_data_get(data);

   item = wd->previous;
   if(item && item->o)
     {
	edje_object_part_unswallow(NULL, wd->previous->o);
	if(item->o && item->itc->func.del)
	     item->itc->func.del((void*)item->data, wd->previous->o);
	else if(item->o)
	  evas_object_del(item->o);
	item->o = NULL;
	wd->previous = NULL;
     }

   item = wd->current;
   if(!item || !item->o) return;

   edje_object_part_unswallow(NULL, item->o);
   edje_object_part_swallow(wd->slideshow, "elm.swallow.1", item->o);
   if(wd->timeout>0)
     wd->timer = ecore_timer_add(wd->timeout, _timer_cb, data);
}


static int
_timer_cb(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);

   wd->timer = NULL;
   elm_slideshow_next(obj);
   return 0;
}

/**
 * Add a new slideshow to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Slideshow
 */
EAPI Evas_Object *
elm_slideshow_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "slideshow");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->current = NULL;
   wd->previous = NULL;

   wd->slideshow = edje_object_add(e);
   _elm_theme_set(wd->slideshow, "slideshow", "base", "default");
   elm_widget_resize_object_set(obj, wd->slideshow);
   evas_object_show(wd->slideshow);

   wd->transitions = _stringlist_get(edje_object_data_get(wd->slideshow, "transitions"));
   if(eina_list_count(wd->transitions) > 0)
     wd->transition = eina_stringshare_add(eina_list_data_get(wd->transitions));

   edje_object_signal_callback_add(wd->slideshow, "end", "slideshow", _end, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Add a object in the list. The object can be a evas object image or a elm photo for example.
 * 
 * @param obj The slideshow object
 * @aram itc Callbacks used to create/delete the object. If itc->del.del is NULL, the object will be destroyed with evas_object_del()
 * @param data Data used by the user to identified the item
 * @return Returns The slideshow item
 */
EAPI Elm_Slideshow_Item* 
elm_slideshow_item_add(Evas_Object *obj, const Elm_Slideshow_Item_Class *itc, const void *data)
{
   Elm_Slideshow_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;

   item = calloc(1, sizeof(Elm_Slideshow_Item));
   item->data = data;
   item->itc = itc;
   item->obj = obj;
   item->l = eina_list_append(item->l, item);

   wd->items = eina_list_merge(wd->items, item->l);

   if(!wd->current)
     elm_slideshow_show(item);

   return item;
}

/**
 * Go to the item number @pos
 *
 * @param obj The slideshow object
 * @param pos The position of the item
 */
EAPI void
elm_slideshow_show(Elm_Slideshow_Item *item)
{
   Evas_Object *o;
   Eina_List *l;
   const char *part;
   Widget_Data *wd = elm_widget_data_get(item->obj);

   if (!wd) return;
   if(item == wd->current) return;

   _end(item->obj, item->obj, NULL, NULL);

   wd->previous = wd->current;
   wd->current = item;
   if(!item->o && item->itc->func.get)
     {
	 item->o = item->itc->func.get((void*)item->data, item->obj);
	 evas_object_smart_member_add(item->o, item->obj);
     }

   _end(item->obj, item->obj, NULL, NULL);
}

/**
 * Go to the next item
 *
 * @param obj The slideshow object
 */
EAPI void
elm_slideshow_next(Evas_Object *obj)
{
   char buf[1024];
   Eina_List *l;
   const char *part;
   Elm_Slideshow_Item *item;
   Evas_Object *o;
   Elm_Slideshow_Item *next = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   if(wd->current)
     next = eina_list_data_get(eina_list_next(wd->current->l));
   if(wd->current && !next && wd->loop)
     next = eina_list_data_get(wd->items);

   if(!next || next == wd->current) return;

   _end(obj, obj, NULL, NULL);

   if (wd->timer) ecore_timer_del(wd->timer);
   wd->timer = NULL;

   if(!next->o && next->itc->func.get)
     {
	 next->o = next->itc->func.get((void*)next->data, obj);
	 evas_object_smart_member_add(next->o, obj);
     }


   edje_object_part_swallow(wd->slideshow, "elm.swallow.2", next->o);

   snprintf(buf, 1024, "%s,next", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->previous = wd->current;
   wd->current = next;
}

/**
 * Go to the previous item
 *
 * @param obj The slideshow object
 */
   EAPI void
elm_slideshow_previous(Evas_Object *obj)
{
   char buf[1024];
   Eina_List *l;
   const char *part;
   Evas_Object *o;
   Elm_Slideshow_Item *prev;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
  
   if(wd->current)
     prev = eina_list_data_get(eina_list_prev(wd->current->l));
   if(wd->current && !prev && wd->loop)
     prev = eina_list_data_get(eina_list_last(wd->items));

   if(!prev ||  prev == wd->current) return;

   _end(obj, obj, NULL, NULL);

   if (wd->timer) ecore_timer_del(wd->timer);
   wd->timer = NULL;

   if(!prev->o && prev->itc->func.get)
     {
	 prev->o = prev->itc->func.get((void*)prev->data, obj);
	 evas_object_smart_member_add(prev->o, obj);
     }

   edje_object_part_swallow(wd->slideshow, "elm.swallow.2", prev->o);

   snprintf(buf, 1024, "%s,previous", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->previous = wd->current;
   wd->current = prev;
}

/**
 * Returns the list of transitions available.
 *
 * @param obj The slideshow object
 * @return Returns the list of transitions (list of char*)
 */
const Eina_List *
elm_slideshow_transitions_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   return wd->transitions;
}

/**
 * Set the transition to use
 *
 * @param obj The slideshow object
 * @param transition the new transition
 */
EAPI void
elm_slideshow_transition_set(Evas_Object *obj, const char *transition)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   eina_stringshare_del(wd->transition);
   wd->transition = eina_stringshare_add(transition);
}

/**
 * The slideshow can go to the next item automatically after a few seconds.
 * This method set the timeout to use. A timeout <=0 disable the timer.
 *
 * @param obj The slideshow object
 * @param timeout The new timeout
 */
EAPI void
elm_slideshow_timeout_set(Evas_Object *obj ,int timeout)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   wd->timeout = timeout;
   if (wd->timer) ecore_timer_del(wd->timer);
   wd->timer = NULL;
   if (timeout>0)
     wd->timer = ecore_timer_add(timeout, _timer_cb, obj);
}

/**
 * Returns the timeout value
 *
 * @param obj The slideshow object
 * @return Returns the timeout
 */
EAPI int
elm_slideshow_timeout_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   return wd->timeout;
}

/**
 * Set if the first item should follow the last and vice versa
 *
 * @param obj The slideshow object
 * @param loop if 1, the first item will follow the last and vice versa
 */
EAPI void
elm_slideshow_loop_set(Evas_Object *obj, int loop)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   wd->loop = loop;
}

/**
 * Delete all the items
 *
 * @param obj The slideshow object
 */
EAPI void
elm_slideshow_clear(Evas_Object *obj)
{
   Elm_Slideshow_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;

   wd->previous = NULL;
   wd->current = NULL;
   EINA_LIST_FREE(wd->items, item)
     {
	if(item->o && item->itc->func.del)
	     item->itc->func.del((void*)item->data, wd->previous->o);
	else if(item->o)
	  evas_object_del(item->o);

	free(item);
     }
}


/**
 * Delete the item
 *
 * @param item The slideshow item 
 */
   EAPI void	
elm_slideshow_item_del(Elm_Slideshow_Item *item)
{
   if(!item) return;

   Widget_Data *wd = elm_widget_data_get(item->obj);

   if(wd->previous == item) wd->previous = NULL;
   if(wd->current == item)
     {
	wd->current = NULL;
	Eina_List *l = eina_list_data_find_list(wd->items, item);
	Eina_List *l2 = eina_list_next(l);
	if(!l2)
	  l2 = eina_list_nth_list(wd->items, eina_list_count(wd->items)-1);
	if(l2)
	  elm_slideshow_show(eina_list_data_get(l2));
     }

   wd->items = eina_list_remove(wd->items, item);

   if(item->o && item->itc->func.del)
     item->itc->func.del((void*)item->data, wd->previous->o);
   else if(item->o)
     evas_object_del(item->o);

   free(item);
}

/**
 * Returns the current item displayed
 *
 * @param obj The slideshow object
 * @return Returns the current item displayed
 */
EAPI Elm_Slideshow_Item*
elm_slideshow_item_current_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return wd->current;
}

/**
 * Returns the evas object associated to an item
 *
 * @param item The slideshow item
 * @return Returns the evas object associated to this item
 */
EAPI Evas_Object*
elm_slideshow_item_object_get(Elm_Slideshow_Item * item)
{
   if (!item) return NULL;
   return item->o;
}

