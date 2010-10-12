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
_custom_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;
   Evas_Coord iw = 0, ih = 0;
   Evas_Coord fx = 0, fy = 0, fw = 0, fh = 0;
   Evas_Coord nx = 0, ny = 0, nw = 0, nh = 0;
   const char *p;

   if ((!wd->img) || (!wd->file)) return;
   if (((p = strrchr(wd->file, '.'))) && (!strcasecmp(p, ".edj"))) return;

   /* grab image size */
   evas_object_image_size_get(wd->img, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;

   /* grab base object dimensions */
   evas_object_geometry_get(wd->base, &bx, &by, &bw, &bh);

   /* set some defaults */
   nx = bx;
   ny = by;
   nw = bw;
   nh = bh;

   switch (wd->option) 
     {
      case ELM_BG_OPTION_CENTER:
        fx = fy = 0;
        fw = nw = iw;
        fh = nh = ih;
        nx = ((bw - fw) / 2);
        ny = ((bh - fh) / 2);
        break;
      case ELM_BG_OPTION_SCALE:
        fw = bw;
        fh = ((ih * fw) / iw);
        if (fh < bh)
          {
             fh = bh;
             fw = ((iw * fh) / ih);
          }
        fx = ((bw - fw) / 2);
        fy = ((bh - fh) / 2);
        break;
      case ELM_BG_OPTION_TILE:
        fx = fy = 0;
        fw = iw;
        fh = ih;
        break;
      case ELM_BG_OPTION_STRETCH:
      default:
        fx = nx = 0;
        fy = ny = 0;
        fw = bw;
        fh = bh;
        break;
     }

   evas_object_image_fill_set(wd->img, fx, fy, fw, fh);
   evas_object_move(wd->img, nx, ny);
   evas_object_resize(wd->img, nw, nh);
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

   evas_object_event_callback_add(wd->base, EVAS_CALLBACK_RESIZE, 
                                  _custom_resize, wd);

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
	evas_object_image_file_set(wd->img, file, group);
     }
   evas_object_repeat_events_set(wd->img, EINA_TRUE);
   edje_object_part_swallow(wd->base, "elm.swallow.background", wd->img);
   elm_widget_sub_object_add(obj, wd->img);
   _custom_resize(wd, NULL, NULL, NULL);
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
   _custom_resize(wd, NULL, NULL, NULL);
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
        edje_object_part_swallow(wd->base, "elm.swallow.rectangle", wd->rect);
        elm_widget_sub_object_add(obj, wd->rect);
        _custom_resize(wd, NULL, NULL, NULL);
     }
   evas_object_color_set(wd->rect, r, g, b, 255);
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
   if (!overlay) 
     {
        _custom_resize(wd, NULL, NULL, NULL);
        return;
     }
   wd->overlay = overlay;
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->overlay);
   elm_widget_sub_object_add(obj, wd->overlay);

   _custom_resize(wd, NULL, NULL, NULL);
}
