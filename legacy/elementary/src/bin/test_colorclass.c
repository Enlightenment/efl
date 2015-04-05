#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static char *
tl_cb(char *str)
{
   return str;
}

void
test_colorclass()
{
   Evas_Object *win, *cc;

   win = elm_win_util_standard_add("colorclass", "Color Classes");
   elm_win_autodel_set(win, EINA_TRUE);

   elm_color_class_translate_cb_set(tl_cb);
   cc = elm_color_class_editor_add(win, 0);
   elm_win_resize_object_add(win, cc);
   evas_object_size_hint_weight_set(cc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(cc);

   evas_object_show(win);
   evas_object_size_hint_min_set(win, 400, 280);
   evas_object_resize(win, 400, 233);
}
