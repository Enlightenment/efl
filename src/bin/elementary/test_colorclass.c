#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>


void
test_colorclass(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *cc;

   win = elm_win_util_standard_add("colorclass", "Color Classes");
   elm_win_autodel_set(win, EINA_TRUE);

   cc = elm_color_class_editor_add(win);
   elm_win_resize_object_add(win, cc);
   evas_object_size_hint_weight_set(cc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(cc);

   evas_object_show(win);
   evas_object_size_hint_min_set(win, 400 * elm_config_scale_get(),
                                      280 * elm_config_scale_get());
   evas_object_resize(win, 400 * elm_config_scale_get(),
                           280 * elm_config_scale_get());
}
