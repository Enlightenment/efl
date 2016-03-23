#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define EINA_UNUSED
#endif
#include <Elementary.h>

static char *
_grid_text_get(void        *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                const char  *part EINA_UNUSED)
{
   return strdup("label");
}

static Evas_Object *
_grid_content_get(void        *data EINA_UNUSED,
               Evas_Object *obj,
               const char  *part)
{
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *icon = elm_icon_add(obj);
        elm_icon_standard_set(icon, "home");
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1,
                                         1);
        evas_object_show(icon);
        return icon;
     }

   return NULL;
}

#include "widget_preview_tmpl_head.c"

Evas_Object *gengrid = elm_gengrid_add(win);
evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, gengrid);
evas_object_show(gengrid);

Elm_Gengrid_Item_Class gic;
gic.item_style = "default";
gic.func.text_get = _grid_text_get;
gic.func.content_get = _grid_content_get;
gic.func.state_get = NULL;
gic.func.del = NULL;

elm_gengrid_item_append(gengrid, &gic, NULL, NULL, NULL);
elm_gengrid_item_append(gengrid, &gic, NULL, NULL, NULL);
elm_gengrid_item_append(gengrid, &gic, NULL, NULL, NULL);

#include "widget_preview_tmpl_foot.c"
