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
   Evas_Object *img1, *img2;

   // list of char*
   Eina_List *images;
   int current;

   Eina_List *transitions;
   const char *transition;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _signal_clicked(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _signal_move(void *data, Evas *e, Evas_Object *obj, void *event_info);


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
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   evas_object_event_callback_del
      (wd->img1, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
   evas_object_del(wd->img1);

   evas_object_event_callback_del
      (wd->img2, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
   evas_object_del(wd->img2);
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

   Widget_Data *wd = elm_widget_data_get(data);

   node = eina_list_nth(wd->images, wd->current);
   if(!node) return;

   o = edje_object_part_object_get(wd->slideshow, "elm.image.1");
   evas_object_image_file_set((Evas_Object *)o, node->file, node->group);
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
   const Evas_Object *o;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   if(pos<0 || pos>=eina_list_count(wd->images))
     return ;

   if(wd->img1)
     evas_object_del(wd->img1);

   wd->current = pos;
   node = eina_list_nth(wd->images, wd->current);

   o = edje_object_part_object_get(wd->slideshow, "elm.image.1");
   evas_object_image_file_set((Evas_Object *)o, node->file, node->group);
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
   Node *node;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   if(wd->current >= eina_list_count(wd->images) - 1)
     return ;

   node = eina_list_nth(wd->images, wd->current + 1);

   _end(obj, obj, NULL, NULL);

   o = edje_object_part_object_get(wd->slideshow, "elm.image.2");
   evas_object_image_file_set((Evas_Object *)o, node->file, node->group);

   snprintf(buf, 1024, "%s,next", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->current++;
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
   Node *node;
   Widget_Data *wd = elm_widget_data_get(obj);
   if(!wd) return;

   if(wd->current <= 0)
     return ;

   node = eina_list_nth(wd->images, wd->current - 1);

   _end(obj, obj, NULL, NULL);

   o = edje_object_part_object_get(wd->slideshow, "elm.image.2");
   evas_object_image_file_set((Evas_Object *)o, node->file, node->group);

   snprintf(buf, 1024, "%s,previous", wd->transition);
   edje_object_signal_emit(wd->slideshow, buf, "slideshow");

   wd->current--;
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

