#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define EINA_UNUSED
#endif

#include <Elementary.h>

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   return strdup(buf);
}

unsigned char _func(void *data);

#include "widget_preview_tmpl_head.c"

Evas_Object *combobox = elm_combobox_add(win);
evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
elm_object_part_text_set(combobox, "guide", "A long list");
evas_object_resize(combobox, 300, 28);
evas_object_show(combobox);

Elm_Genlist_Item_Class *itc;
itc = elm_genlist_item_class_new();
itc->item_style = "default";
itc->func.text_get = gl_text_get;
itc->func.content_get = NULL;
itc->func.state_get = NULL;
itc->func.filter_get = NULL;
itc->func.del = NULL;
for (int i = 0; i < 1000; i++)
   elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                           NULL, ELM_GENLIST_ITEM_NONE, NULL,
                           (void*)(uintptr_t)(i * 10));
ecore_timer_add(0.05, _func, combobox);

#include "widget_preview_tmpl_foot.c"

unsigned char _func(void *data)
{
   elm_combobox_hover_begin(data);
   return 0;
}
