#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static char *
tl_cb(char *str)
{
   return str;
}

static Eina_List *
name_cb(void)
{
   Eina_List *l, *ret = NULL;
   Eina_Iterator *it;
   Eina_File *f;

   it = edje_file_iterator_new();
   if (!it) return NULL;
   EINA_ITERATOR_FOREACH(it, f)
     {
        l = elm_color_class_util_edje_file_list(f);
        if (l)
          ret = eina_list_merge(ret, l);
     }
   eina_iterator_free(it);
   return ret;
}

void
test_colorclass()
{
   Evas_Object *win, *cc;

   win = elm_win_util_standard_add("colorclass", "Color Classes");
   elm_win_autodel_set(win, EINA_TRUE);

   elm_color_class_translate_cb_set(tl_cb);
   elm_color_class_list_cb_set(name_cb);
   cc = elm_color_class_editor_add(win, 0);
   elm_win_resize_object_add(win, cc);
   evas_object_size_hint_weight_set(cc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(cc);

   evas_object_show(win);
   evas_object_size_hint_min_set(win, 400, 280);
   evas_object_resize(win, 400, 233);
}
