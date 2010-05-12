#include <string.h>

#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Bg Bg
 *
 * The bg object is used for setting a solid background to a window or packing
 * into any container object.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *img, *custom_img;
   const char  *file, *group;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _custom_resize(void *data, Evas *a, Evas_Object *obj, void *event_info);

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
   _elm_theme_object_set(obj, wd->img, "bg", "base", elm_widget_style_get(obj));
}

static void
_custom_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   int iw = 0, ih = 0;
   Evas_Coord x, y, w, h, ow = 0, oh = 0;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);

   if ((iw < 1) || (ih < 1)) return;
   w = ow;
   h = (ih * w) / iw;
   if (h < oh)
     {
	h = oh;
	w = (iw * h) / ih;
     }
   x = (ow - w) / 2;
   y = (oh - h) / 2;
   evas_object_image_fill_set(obj, x, y, w, h);
}

/**
 * Add a new background to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Bg
 */
EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "bg");
   elm_widget_type_set(obj, "bg");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);

   wd->img = edje_object_add(e);
   _elm_theme_object_set(obj, wd->img, "bg", "base", "default");
   elm_widget_resize_object_set(obj, wd->img);
   return obj;
}

/**
 * Set the file (image or edje) used for the background
 *
 * @param obj The bg object
 * @param file The file path
 * @param group Optional key (group in Edje) within the file
 *
 * This sets the image file used in the background object. The image (or edje)
 * will be stretched (retaining aspect if its an image file) to completely fill
 * the bg object. This may mean some parts arte not visible.
 *
 * @ingroup Bg
 */
EAPI void
elm_bg_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *p;

   if (wd->custom_img)
     {
	evas_object_del(wd->custom_img);
	wd->custom_img = NULL;
     }
   if (!file) return;
   eina_stringshare_replace(&wd->file, file);
   eina_stringshare_replace(&wd->group, group);
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     {
	wd->custom_img = edje_object_add(evas_object_evas_get(wd->img));
	edje_object_file_set(wd->custom_img, file, group);
     }
   else
     {
	wd->custom_img = evas_object_image_add(evas_object_evas_get(wd->img));
	evas_object_event_callback_add(wd->custom_img, EVAS_CALLBACK_RESIZE, 
                                       _custom_resize, wd);
	evas_object_image_file_set(wd->custom_img, file, group);
     }
   elm_widget_sub_object_add(obj, wd->custom_img);
   evas_object_repeat_events_set(wd->custom_img, 1);
   edje_object_part_swallow(wd->img, "elm.swallow.background", wd->custom_img);
   evas_object_show(wd->custom_img);
}
