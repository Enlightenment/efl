#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

/* get our images to make slideshow items */
static Evas_Object *
_get(void        *data,
     Evas_Object *obj)
{
   Evas_Object *photo = elm_photo_add(obj);
   elm_photo_file_set(photo, data);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");

   return photo;
}

#include "widget_preview_tmpl_head.c"

static const char *img9 = PACKAGE_DATA_DIR "/images/logo.png";
static Elm_Slideshow_Item_Class itc;

Evas_Object *o = elm_slideshow_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

itc.func.get = _get;
itc.func.del = NULL;

elm_slideshow_timeout_set(o, 0.01);
elm_slideshow_item_add(o, &itc, img9);

#include "widget_preview_tmpl_foot.c"
