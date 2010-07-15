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
 * The slideshow use 2 callbacks to create and delete the objects displayed. When an item
 * is displayed the function itc->func.get() is called. This function should create the object,
 * for example the object can be an evas_object_image or a photocam. When a object is no more
 * displayed the function itc->func.del() is called, the user can delete or hide the object here.
 * If itc->func.del is NULL the slideshow delete the object with evas_object_del().
 */

typedef struct _Widget_Data Widget_Data;

struct _Elm_Slideshow_Item
{
   Evas_Object *obj;

   Eina_List *l, *l_built, *l_built_o2;

   const void *data;
   const Elm_Slideshow_Item_Class *itc;

   Evas_Object *o, *o_2;
};

struct _Elm_Slideshow_Layout
{
	const char *name;
	int nb_items;
};

struct _Widget_Data
{
   Evas_Object *slideshow;

   // list of Elm_Slideshow_Item*
   Eina_List *items;
   Eina_List *items_built;
   Eina_List *items_built_o2;

   Elm_Slideshow_Item *current;
   Elm_Slideshow_Item *previous;

   Eina_List *transitions;
   const char *transition;

   Ecore_Timer *timer;
   int timeout;
   Eina_Bool loop:1;

   struct {
	   const Elm_Slideshow_Layout *current;
	   const Elm_Slideshow_Layout *next;
	   Eina_List *list; //list of Elm_Slideshow_Layout
   } layout;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool _timer_cb(void *data);

static void
_del_hook(Evas_Object *obj)
{
   Elm_Slideshow_Layout *layout;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_slideshow_clear(obj);
   _elm_stringlist_free(wd->transitions);
   if (wd->timer) ecore_timer_del(wd->timer);
   EINA_LIST_FREE(wd->layout.list, layout)
   {
	   eina_stringshare_del(layout->name);
	   free(layout);
   }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_theme_object_set(obj, wd->slideshow, "slideshow", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->slideshow, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   edje_object_size_min_calc(wd->slideshow, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, minw, minh);
}


static Elm_Slideshow_Item* _item_prev_get(Elm_Slideshow_Item* item)
{
	Widget_Data *wd = elm_widget_data_get(item->obj);
	Elm_Slideshow_Item* prev = eina_list_data_get(eina_list_prev(item->l));
	if(!prev && wd->loop)
		prev = eina_list_data_get(eina_list_last(item->l));
	return prev;
}
static Elm_Slideshow_Item* _item_next_get(Elm_Slideshow_Item* item)
{
	Widget_Data *wd = elm_widget_data_get(item->obj);
	Elm_Slideshow_Item* next = eina_list_data_get(eina_list_next(item->l));
	if(!next && wd->loop)
		next = eina_list_data_get(wd->items);
	return next;
}


static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   ;
}

static void
_item_realize(Elm_Slideshow_Item *item, int num)
{
	Elm_Slideshow_Item *_item;
	Evas_Object *obj = item->obj;
	Widget_Data *wd = elm_widget_data_get(obj);
	Evas_Coord w, h;

	if (!wd) return;

	if(num == 1)
	{
		if (!item->o && item->itc->func.get)
		{
			item->o = item->itc->func.get((void*)item->data, obj);
			evas_object_smart_member_add(item->o, obj);
			item->l_built = eina_list_append(NULL, item);
			wd->items_built = eina_list_merge(wd->items_built, item->l_built);
			evas_object_hide(item->o);
		}
		else if (item->l_built)
			wd->items_built = eina_list_demote_list(wd->items_built, item->l_built);
	}
	else //num == 2
	{
		if (!item->o_2 && item->itc->func.get)
		{
			item->o_2 = item->itc->func.get((void*)item->data, obj);
			evas_object_smart_member_add(item->o_2, obj);
			item->l_built_o2 = eina_list_append(NULL, item);
			wd->items_built_o2 = eina_list_merge(wd->items_built_o2, item->l_built_o2);
			evas_object_hide(item->o_2);
		}
	}

	//delete unused items
	while (eina_list_count(wd->items_built) > 3 + wd->layout.current->nb_items * 2)
	{
		_item = eina_list_data_get(wd->items_built);
		wd->items_built = eina_list_remove_list(wd->items_built, wd->items_built);
		if(item->itc->func.del)
			item->itc->func.del((void*)item->data, _item->o);
		else
			evas_object_del(_item->o);
		_item->o = NULL;
	}
	//delete unused items
	while (eina_list_count(wd->items_built_o2) > 3 + wd->layout.current->nb_items * 2)
	{
		_item = eina_list_data_get(wd->items_built_o2);
		wd->items_built_o2 = eina_list_remove_list(wd->items_built_o2, wd->items_built_o2);
		if(item->itc->func.del)
			item->itc->func.del((void*)item->data, _item->o_2);
		else
			evas_object_del(_item->o_2);
		_item->o_2 = NULL;
	}
}

static void
_layout_end(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
	Elm_Slideshow_Item *item;
	Widget_Data *wd = elm_widget_data_get(data);
	if (!wd) return;

	wd->layout.current = wd->layout.next;
}

static void
_first_last_get(Evas_Object *obj, Elm_Slideshow_Item *current, Elm_Slideshow_Item **_first, Elm_Slideshow_Item **_last)
{
	Elm_Slideshow_Item *first, *last;
	int i;
	Widget_Data *wd = elm_widget_data_get(obj);
	if (!wd) return;

	last = current;
	first = current;

	Eina_Bool loop_1 = EINA_TRUE;
	for(i = 0 - wd->layout.current->nb_items; i < 0 && first; i++)
	{
		if(!loop_1 && first == current)
			break;
		first = _item_prev_get(first);
		loop_1 = EINA_FALSE;
	}

	loop_1 = EINA_TRUE;
	for(i = 0; i <  wd->layout.current->nb_items && last; i++)
	{
		if(!loop_1 && last == current)
			break;
		last = _item_next_get(last);
		loop_1 = EINA_FALSE;
	}

	*_first = first;
	*_last = last;
}

static void
_end(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
	char buf[PATH_MAX];
	int i;
	Elm_Slideshow_Item *item, *first, *last;
	Widget_Data *wd = elm_widget_data_get(data);
	if (!wd) return;

	if(wd->previous)
	{
		_first_last_get(data, wd->previous, &first, &last);
		while(1)
		{
			edje_object_part_unswallow(NULL, first->o);
			evas_object_hide(first->o);
			edje_object_part_unswallow(NULL, first->o_2);
			evas_object_hide(first->o_2);

			if(first == last)
				break;
			first = _item_next_get(first);
			if(!first)
				first = eina_list_data_get(wd->items);
		}
		wd->previous = NULL;
	}


	item = wd->current;
	if (!item || !item->o) return;

	_first_last_get(data, wd->current, &first, &last);
	i = 0 - wd->layout.current->nb_items;
	while(1)
	{
	   _item_realize(first, 1);
		edje_object_part_unswallow(NULL, first->o_2);
		evas_object_hide(first->o_2);
		edje_object_part_unswallow(NULL, first->o);
		evas_object_show(first->o);

		edje_object_signal_emit(wd->slideshow, "anim,end", "slideshow");
		snprintf(buf, sizeof(buf), "elm.swallow.%d.1", i);
		edje_object_part_swallow(wd->slideshow, buf, first->o);

		if(first == last)
			break;
		first = _item_next_get(first);
		if(!first)
			first = eina_list_data_get(wd->items);
		i++;
	}
}


static Eina_Bool
_timer_cb(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ECORE_CALLBACK_CANCEL;
   wd->timer = NULL;
   elm_slideshow_next(obj);
   return ECORE_CALLBACK_CANCEL;
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
   Eina_List *layouts, *l;
   const char *s;
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "slideshow");
   elm_widget_type_set(obj, "slideshow");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->current = NULL;
   wd->previous = NULL;

   wd->slideshow = edje_object_add(e);
   _elm_theme_object_set(obj, wd->slideshow, "slideshow", "base", "default");
   evas_object_smart_member_add(wd->slideshow, obj);
   elm_widget_resize_object_set(obj, wd->slideshow);
   evas_object_show(wd->slideshow);

   wd->transitions = _elm_stringlist_get(edje_object_data_get(wd->slideshow, "transitions"));
   if (eina_list_count(wd->transitions) > 0)
     wd->transition = eina_stringshare_add(eina_list_data_get(wd->transitions));

   layouts = _elm_stringlist_get(edje_object_data_get(wd->slideshow, "layouts"));
   EINA_LIST_FOREACH(layouts, l, s)
   {
	   Elm_Slideshow_Layout* layout = calloc(1, sizeof(Elm_Slideshow_Layout));
	   layout->name = eina_stringshare_add(s);
	   const char *n = edje_object_data_get(wd->slideshow, s);
	   if(n)
	   {
		   int i = atoi(n);
		   layout->nb_items = i;
	   }
	   else
		   layout->nb_items = 0;
	   wd->layout.list = eina_list_append(wd->layout.list, layout);
   }
   _elm_stringlist_free(layouts);
   if (eina_list_count(wd->layout.list) > 0)
       wd->layout.current = eina_list_data_get(wd->layout.list);

   edje_object_signal_callback_add(wd->slideshow, "end", "slideshow", _end, obj);
   edje_object_signal_callback_add(wd->slideshow, "layout,end", "slideshow", _layout_end, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);

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
 *
 * @ingroup Slideshow
 */
EAPI Elm_Slideshow_Item*
elm_slideshow_item_add(Evas_Object *obj, const Elm_Slideshow_Item_Class *itc, const void *data)
{
   Elm_Slideshow_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   item = calloc(1, sizeof(Elm_Slideshow_Item));
   item->data = data;
   item->itc = itc;
   item->obj = obj;
   item->l = eina_list_append(item->l, item);

   wd->items = eina_list_merge(wd->items, item->l);

   if (!wd->current) elm_slideshow_show(item);

   return item;
}

/**
 * Go to the item
 *
 * @param obj The slideshow object
 * @param item The item
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_show(Elm_Slideshow_Item *item)
{
   Widget_Data *wd = elm_widget_data_get(item->obj);
   if (!wd) return;
   if (item == wd->current) return;

   _end(item->obj, item->obj, NULL, NULL);

   wd->previous = wd->current;
   wd->current = item;
   _item_realize(item, 1);

   _end(item->obj, item->obj, NULL, NULL);
}

/**
 * Go to the next item
 *
 * @param obj The slideshow object
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_next(Evas_Object *obj)
{
   char buf[1024];
   int i;
   Elm_Slideshow_Item *next = NULL, *first, *last;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   
   if (wd->current)
	   next = _item_next_get(wd->current);

   if (!next || next == wd->current) return;
   

   _end(obj, obj, NULL, NULL);
   
   if (wd->timer) ecore_timer_del(wd->timer);
   if (wd->timeout > 0)
     wd->timer = ecore_timer_add(wd->timeout, _timer_cb, obj);


   _first_last_get(obj, next, &first, &last);

   i = 0 - wd->layout.current->nb_items;
   while(1)
   {
	   _item_realize(first, 2);

	   snprintf(buf, sizeof(buf), "elm.swallow.%d.2", i);
	   edje_object_part_swallow(wd->slideshow, buf, first->o_2);
	   evas_object_show(first->o_2);


	   if(first == last)
		   break;
	   first = _item_next_get(first);
	   if(!first)
			first = eina_list_data_get(wd->items);
	   i++;
   }

   snprintf(buf, sizeof(buf), "%s,next", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->previous = wd->current;
   wd->current = next;
}

/**
 * Go to the previous item
 *
 * @param obj The slideshow object
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_previous(Evas_Object *obj)
{
   char buf[1024];
   Elm_Slideshow_Item *prev = NULL, *first, *last;
   int i;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (wd->current)
     prev = _item_prev_get(wd->current);

   if (!prev ||  prev == wd->current) return;

   _end(obj, obj, NULL, NULL);

   if (wd->timer) ecore_timer_del(wd->timer);
   if (wd->timeout > 0)
     wd->timer = ecore_timer_add(wd->timeout, _timer_cb, obj);

   _first_last_get(obj, prev, &first, &last);

   i = 0 - wd->layout.current->nb_items;
   while(1)
   {
	   _item_realize(first, 2);

	   snprintf(buf, sizeof(buf), "elm.swallow.%d.2", i);
	   edje_object_part_swallow(wd->slideshow, buf, first->o_2);
	   evas_object_show(first->o_2);


	   if(first == last)
		   break;
	   first = _item_next_get(first);
	   if(!first)
		   first = eina_list_data_get(wd->items);
	   i++;
   }


   snprintf(buf, 1024, "%s,previous", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->previous = wd->current;
   wd->current = prev;
}

/**
 * Returns the list of transitions available.
 *
 * @param obj The slideshow object
 * @return Returns the list of transitions (list of const char*)
 *
 * @ingroup Slideshow
 */
const Eina_List *
elm_slideshow_transitions_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->transitions;
}

/**
 * Returns the list of layouts available.
 *
 * @param obj The slideshow object
 * @return Returns the list of layout (list of const Elm_Slideshow_Layout*)
 *
 * @ingroup Slideshow
 */
const Eina_List *
elm_slideshow_layouts_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->layout.list;
}

/**
 * Set the transition to use
 *
 * @param obj The slideshow object
 * @param transition the new transition
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_transition_set(Evas_Object *obj, const char *transition)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->transition, transition);
}

/**
 * Returns the transition to use
 *
 * @param obj The slideshow object
 * @return the transition set
 *
 * @ingroup Slideshow
 */
EAPI const char *
elm_slideshow_transition_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->transition;
}

/**
 * The slideshow can go to the next item automatically after a few seconds.
 * This method set the timeout to use. A timeout <=0 disable the timer.
 *
 * @param obj The slideshow object
 * @param timeout The new timeout
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_timeout_set(Evas_Object *obj ,int timeout)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->timeout = timeout;
   if (wd->timer) ecore_timer_del(wd->timer);
   wd->timer = NULL;
   if (timeout > 0)
     wd->timer = ecore_timer_add(timeout, _timer_cb, obj);
}

/**
 * Returns the timeout value
 *
 * @param obj The slideshow object
 * @return Returns the timeout
 *
 * @ingroup Slideshow
 */
EAPI int
elm_slideshow_timeout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return -1;
   return wd->timeout;
}

/**
 * Set if the first item should follow the last and vice versa
 *
 * @param obj The slideshow object
 * @param loop if EINA_TRUE, the first item will follow the last and vice versa
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_loop_set(Evas_Object *obj, Eina_Bool loop)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->loop = loop;
}

/**
 * Returns the current layout name
 *
 * @param obj The slideshow object
 * @returns Returns the layout name
 *
 * @ingroup Slideshow
 */
EAPI const Elm_Slideshow_Layout *
elm_slideshow_layout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->layout.current;
}

/**
 * Set the layout
 *
 * @param obj The slideshow object
 * @param layout the new layout
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_layout_set(Evas_Object *obj, const Elm_Slideshow_Layout *layout)
{
   char buf[PATH_MAX];
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   snprintf(buf, sizeof(buf), "layout,%s", layout->name);
   if(wd->layout.current->nb_items > layout->nb_items)
   {
	   wd->layout.next = layout;
	   edje_object_signal_emit(wd->slideshow, buf, "slideshow");
   }
   else
   {
	   wd->layout.current = layout;
   	   edje_object_signal_emit(wd->slideshow, buf, "slideshow");
   }
   _end(obj, NULL, NULL, NULL);
}

/**
 * Return if the first item should follow the last and vice versa
 *
 * @param obj The slideshow object
 * @returns Returns the loop flag
 *
 * @ingroup Slideshow
 */
EAPI Eina_Bool
elm_slideshow_loop_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->loop;
}

/**
 * Delete all the items
 *
 * @param obj The slideshow object
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_clear(Evas_Object *obj)
{
	Elm_Slideshow_Item *item;
	ELM_CHECK_WIDTYPE(obj, widtype);
	Widget_Data *wd = elm_widget_data_get(obj);
	if (!wd) return;
	wd->previous = NULL;
	wd->current = NULL;
	EINA_LIST_FREE(wd->items_built, item)
	{
		if (item->itc->func.del)
			item->itc->func.del((void*)item->data, item->o);
		else
			evas_object_del(item->o);
	}
	EINA_LIST_FREE(wd->items_built_o2, item)
	{
		if (item->itc->func.del)
			item->itc->func.del((void*)item->data, item->o_2);
		else
			evas_object_del(item->o_2);
	}


	EINA_LIST_FREE(wd->items, item)
	{
		free(item);
	}
}


/**
 * Delete the item
 *
 * @param item The slideshow item
 *
 * @ingroup Slideshow
 */
EAPI void
elm_slideshow_item_del(Elm_Slideshow_Item *item)
{
	if (!item) return;
	Widget_Data *wd = elm_widget_data_get(item->obj);
	if (!wd) return;
	if (wd->previous == item) wd->previous = NULL;
	if (wd->current == item)
	{
		Eina_List *l = eina_list_data_find_list(wd->items, item);
		Eina_List *l2 = eina_list_next(l);
		wd->current = NULL;
		if (!l2)
			l2 = eina_list_nth_list(wd->items, eina_list_count(wd->items) - 1);
		if (l2)
			elm_slideshow_show(eina_list_data_get(l2));
	}

	wd->items = eina_list_remove_list(wd->items, item->l);
	wd->items_built = eina_list_remove_list(wd->items_built, item->l_built);
	wd->items_built_o2 = eina_list_remove_list(wd->items_built_o2, item->l_built_o2);

	if (item->o && item->itc->func.del)
	{
		item->itc->func.del((void*)item->data, wd->previous->o);
		item->itc->func.del((void*)item->data, wd->previous->o_2);
	}
	else if (item->o)
	{
		evas_object_del(item->o);evas_object_del(item->o_2);

	}
	free(item);
}

/**
 * Returns the list of items
 * @param obj The slideshow object
 * @return Returns the list of items (list of Elm_Slideshow_Item).
 *
 * @ingroup Slideshow
 */
EAPI const Eina_List *
elm_slideshow_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->items;
}


/**
 * Returns the current item displayed
 *
 * @param obj The slideshow object
 * @return Returns the current item displayed
 *
 * @ingroup Slideshow
 */
EAPI Elm_Slideshow_Item *
elm_slideshow_item_current_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->current;
}

/**
 * Returns the evas object associated to an item
 *
 * @param item The slideshow item
 * @return Returns the evas object associated to this item
 *
 * @ingroup Slideshow
 */
EAPI Evas_Object *
elm_slideshow_item_object_get(Elm_Slideshow_Item * item)
{
   if (!item) return NULL;
   return item->o;
}

/**
 * Returns the data associated to an item
 *
 * @param item The slideshow item
 * @return Returns the data associated to this item
 *
 * @ingroup Slideshow
 */
EAPI void *
elm_slideshow_item_data_get(Elm_Slideshow_Item * item)
{
   if (!item) return NULL;
   return (void *)item->data;
}

/**
 * Returns the name of a layout
 *
 * @param layout the layout item
 * @return Returns the name of the layout
 *
 * @ingroup Slideshow
 */
EAPI const char *
elm_slideshow_layout_name_get(const Elm_Slideshow_Layout *layout)
{
   if (!layout) return NULL;
   return layout->name;
}
