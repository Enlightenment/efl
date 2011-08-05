#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static char *
_grid_label_get(void        *data __UNUSED__,
                Evas_Object *obj __UNUSED__,
                const char  *part __UNUSED__)
{
   return strdup("label");
}

static Evas_Object *
_grid_icon_get(void        *data __UNUSED__,
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
gic.func.label_get = _grid_label_get;
gic.func.icon_get = _grid_icon_get;
gic.func.state_get = NULL;
gic.func.del = NULL;

elm_gengrid_item_append(gengrid, &gic, NULL, NULL, NULL);
elm_gengrid_item_append(gengrid, &gic, NULL, NULL, NULL);
elm_gengrid_item_append(gengrid, &gic, NULL, NULL, NULL);

#include "widget_preview_tmpl_foot.c"
