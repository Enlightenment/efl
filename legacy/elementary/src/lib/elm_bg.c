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
   Evas_Object *base, *rect, *img, *overlay;
   const char  *file, *group;
   Elm_Bg_Option option;
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
   Evas_Coord w, h;

   _elm_theme_object_set(obj, wd->base, "bg", "base", 
                         elm_widget_style_get(obj));

   if (wd->rect)
     edje_object_part_swallow(wd->base, "elm.swallow.rectangle", wd->rect);
   if (wd->img)
     edje_object_part_swallow(wd->base, "elm.swallow.background", wd->img);
   if (wd->overlay)
     edje_object_part_swallow(wd->base, "elm.swallow.content", wd->overlay);

// FIXME: if i don't do this, bg doesnt calc correctly. why?   
   evas_object_geometry_get(wd->base, NULL, NULL, &w, &h);
   evas_object_resize(wd->base, w, h);
}

static void
_custom_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   int iw = 0, ih = 0;
   Evas_Coord x = 0, y = 0, w = 0, h = 0, ow = 0, oh = 0;
   Widget_Data *wd = data;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);

   if ((iw < 1) || (ih < 1)) return;
   if (wd->option == ELM_BG_OPTION_SCALE) 
     {
        w = ow;
        h = (ih * w) / iw;
        if (h < oh)
          {
             h = oh;
             w = (iw * h) / ih;
          }
        x = (ow - w) / 2;
        y = (oh - h) / 2;
     }
   else if (wd->option == ELM_BG_OPTION_TILE) 
     {
        x = y = 0;
        w = iw;
        h = ih;
     }
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
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "bg", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);

   wd->option = ELM_BG_OPTION_SCALE;
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
 * the bg object. This may mean some parts are not visible.
 *
 * @ingroup Bg
 */
EAPI void
elm_bg_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *p;

   if (wd->img)
     {
	evas_object_del(wd->img);
	wd->img = NULL;
     }
   if (!file) return;
   eina_stringshare_replace(&wd->file, file);
   eina_stringshare_replace(&wd->group, group);
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     {
	wd->img = edje_object_add(evas_object_evas_get(wd->base));
	edje_object_file_set(wd->img, file, group);
     }
   else
     {
	wd->img = evas_object_image_add(evas_object_evas_get(wd->base));
	evas_object_event_callback_add(wd->img, EVAS_CALLBACK_RESIZE, 
                                       _custom_resize, wd);
	evas_object_image_file_set(wd->img, file, group);
     }
   elm_widget_sub_object_add(obj, wd->img);
   evas_object_repeat_events_set(wd->img, EINA_TRUE);
   edje_object_part_swallow(wd->base, "elm.swallow.background", wd->img);
   evas_object_show(wd->img);
}

/**
 * Set the option used for the background image
 *
 * @param obj The bg object
 * @param option The desired background option (TILE, SCALE)
 *
 * This sets the option used for manipulating the display of the background 
 * image. The image can be tiled or scaled.
 *
 * @ingroup Bg
 */
EAPI void 
elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option) 
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   wd->option = option;
}

/**
 * Set the option used for the background color
 *
 * @param obj The bg object
 * @param r
 * @param g
 * @param b
 *
 * This sets the color used for the background rectangle.
 *
 * @ingroup Bg
 */
EAPI void 
elm_bg_color_set(Evas_Object *obj, int r, int g, int b) 
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd->rect)
     {
        wd->rect = evas_object_rectangle_add(evas_object_evas_get(wd->base));
        elm_widget_sub_object_add(obj, wd->rect);
        edje_object_part_swallow(wd->base, "elm.swallow.rectangle", wd->rect);
     }
   evas_object_color_set(wd->rect, r, g, b, 255);
   evas_object_show(wd->rect);
}

/**
 * Set the overlay object used for the background object.
 *
 * @param obj The bg object
 * @param overlay The overlay object
 *
 * This provides a way for elm_bg to have an 'overlay' (such as animated fog)
 * 
 * @ingroup Bg
 */
EAPI void 
elm_bg_overlay_set(Evas_Object *obj, Evas_Object *overlay)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (wd->overlay)
     {
	evas_object_del(wd->overlay);
	wd->overlay = NULL;
     }
   if (!overlay) return;
   wd->overlay = overlay;
   elm_widget_sub_object_add(obj, wd->overlay);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->overlay);
}
