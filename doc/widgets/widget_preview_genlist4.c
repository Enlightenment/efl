#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define EINA_UNUSED
#endif
#include <Elementary.h>

static char *
_text_get(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           const char  *part)
{
   return strdup(part);
}

static Evas_Object *
_content_get(void        *data EINA_UNUSED,
          Evas_Object *obj,
          const char  *part EINA_UNUSED)
{
    Evas_Object *icon = elm_icon_add(obj);
    elm_icon_standard_set(icon, "home");
    evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1,
                                     1);
    evas_object_show(icon);
    return icon;
}

#include "widget_preview_tmpl_head.c"

Evas_Object *genlist = elm_genlist_add(win);
evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, genlist);
evas_object_show(genlist);

Elm_Genlist_Item_Class gic = { 0 };
gic.item_style = "group_index";
gic.func.text_get = _text_get;
gic.func.content_get = _content_get;
gic.func.state_get = NULL;
gic.func.del = NULL;

elm_genlist_item_append(genlist, &gic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
elm_genlist_item_append(genlist, &gic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
elm_genlist_item_append(genlist, &gic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

#include "widget_preview_tmpl_foot.c"
