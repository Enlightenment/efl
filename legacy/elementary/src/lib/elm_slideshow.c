/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Slideshow slideshow
 *
 * This object display a list of images and some actions like
 * next/previous are used to naviguate. The animations are defined in the theme,
 * consequently new animations can be added without having to update the
 * applications.
 *
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Subinfo     Subinfo;
typedef struct _Node Node;

struct _Node
{
   const char *file;
   const char *group;
};

struct _Widget_Data
{
   Evas_Object *slideshow;

   // list of char*
   Eina_List *images;
   int current;
   int loop;

   Eina_List *transitions;
   const char *transition;

   Eina_List *img1_parts;
   Eina_List *img2_parts;

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
   if (wd->timer)
     ecore_timer_del(wd->timer);
   free(wd);
}

   static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;
   _elm_theme_set(wd->slideshow, "slideshow", "base", "default");
   edje_object_scale_set(wd->slideshow, elm_widget_scale_get(obj) * _elm_config->scale);
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
   Node *node;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
}

   static void
_signal_clicked(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, "clicked", NULL);
}

   static void
_signal_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, "move", NULL);
}

   static void
_end(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Node *node;
   const Evas_Object *o;
   Eina_List *l;
   const char *part;

   Widget_Data *wd = elm_widget_data_get(data);

   node = eina_list_nth(wd->images, wd->current);
   if(!node) return;

   EINA_LIST_FOREACH(wd->img1_parts, l, part)
     {
	o = edje_object_part_object_get(wd->slideshow, part);
	evas_object_image_file_set((Evas_Object *)o, node->file, node->group);
     }

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

   wd->slideshow = edje_object_add(e);
   _elm_theme_set(wd->slideshow, "slideshow", "base", "default");
   elm_widget_resize_object_set(obj, wd->slideshow);
   evas_object_show(wd->slideshow);

   wd->transitions = _stringlist_get(edje_object_data_get(wd->slideshow, "transitions"));
   if(eina_list_count(wd->transitions) > 0)
     wd->transition = eina_stringshare_add(eina_list_data_get(wd->transitions));
   wd->img1_parts = _stringlist_get(edje_object_data_get(wd->slideshow, "image1"));
   wd->img2_parts = _stringlist_get(edje_object_data_get(wd->slideshow, "image2"));

   edje_object_signal_callback_add(wd->slideshow, "end", "slideshow", _end, obj);

   evas_object_event_callback_add(wd->slideshow, EVAS_CALLBACK_MOUSE_DOWN, _signal_clicked, obj);
   evas_object_event_callback_add(wd->slideshow, EVAS_CALLBACK_MOUSE_MOVE, _signal_move, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Add an image in the list
 *
 * @param obj The slideshow object
 * @param file The image file
 * @param group The edje group if the image is in an eet file
 */
   EAPI void
elm_slideshow_image_add(Evas_Object *obj, const char *file, const char *group)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   Node *node = ELM_NEW(Node);

   node->file = eina_stringshare_add(file);
   if(group)
     node->group = eina_stringshare_add(group);

   wd->images = eina_list_append(wd->images, node);

   if(eina_list_count(wd->images) == 1)
     elm_slideshow_goto(obj, 0);
}

/**
 * Go to the image number @pos
 *
 * @param obj The slideshow object
 * @param pos The position of the image
 */
   EAPI void
elm_slideshow_goto(Evas_Object *obj, int pos)
{
   Node *node;
   Eina_List *l;
   const char *part;
   const Evas_Object *o;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   if(pos<0 || pos>=eina_list_count(wd->images))
     return ;

   wd->current = pos;
   node = eina_list_nth(wd->images, wd->current);

   EINA_LIST_FOREACH(wd->img1_parts, l, part)
     {
	o = edje_object_part_object_get(wd->slideshow, part);
	evas_object_image_file_set((Evas_Object *)o, node->file, node->group);
     }
}

/**
 * Go to the next image
 *
 * @param obj The slideshow object
 */
   EAPI void
elm_slideshow_next(Evas_Object *obj)
{
   char buf[1024];
   const Evas_Object *o;
   Eina_List *l;
   const char *part;
   Node *node;
   int next;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   if(eina_list_count(wd->images) <= 0
	 || (wd->current >= eina_list_count(wd->images) - 1
	    && !wd->loop))
     return ;

   if(wd->current >= eina_list_count(wd->images) - 1)
     next = 0;
   else
     next = wd->current + 1;

   node = eina_list_nth(wd->images, next);

   _end(obj, obj, NULL, NULL);


   EINA_LIST_FOREACH(wd->img2_parts, l, part)
     {
	o = edje_object_part_object_get(wd->slideshow, part);
	evas_object_image_file_set((Evas_Object *)o, node->file, node->group);
     }

   snprintf(buf, 1024, "%s,next", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->current = next;

   if(wd->timer)
     ecore_timer_del(wd->timer);
   wd->timer = NULL;
}

/**
 * Go to the previous image
 *
 * @param obj The slideshow object
 */
   EAPI void
elm_slideshow_previous(Evas_Object *obj)
{
   char buf[1024];
   const Evas_Object *o;
   Eina_List *l;
   const char *part;
   Node *node;
   int previous;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   if(eina_list_count(wd->images) <= 0
	 || (wd->current <= 0
	    && !wd->loop))
     return ;

   if(wd->current <= 0)
     previous = eina_list_count(wd->images) - 1;
   else
     previous = wd->current - 1;

   node = eina_list_nth(wd->images, previous);

   _end(obj, obj, NULL, NULL);

   EINA_LIST_FOREACH(wd->img2_parts, l, part)
     {
	o = edje_object_part_object_get(wd->slideshow, part);
	evas_object_image_file_set((Evas_Object *)o, node->file, node->group);
     }

   snprintf(buf, 1024, "%s,previous", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->current = previous;

   if(wd->timer)
     ecore_timer_del(wd->timer);
   wd->timer = NULL;
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
 * The object can go to the next image automatically after a few seconds.
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
   if(wd->timer)
     ecore_timer_del(wd->timer);
   wd->timer = NULL;
   if(timeout>0)
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
 * Set if the first image should follow the last
 *
 * @param obj The slideshow object
 * @param loop if 1, the first image will follow the last
 */
   EAPI void
elm_slideshow_loop_set(Evas_Object *obj, int loop)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   wd->loop = loop;
}

/**
 * Delete all the images
 *
 * @param obj The slideshow object
 */
   EAPI void
elm_slideshow_clear(Evas_Object *obj)
{
   Node *node;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   EINA_LIST_FREE(wd->images, node)
     {
	 if(node->file) eina_stringshare_del(node->file);
	 if(node->group) eina_stringshare_del(node->group);
     }
}

